//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <cfloat>
#include <list>
#include <map>
#include <TraCIMobility.h>

#include <queue>

#include <time.h>

#include "Urae.h"

#include "ExperimentRig/ExperimentScenarioManager.h"

#include "UraePhyLayer.h"
#include "CORNERModel.h"
#include "CarShadowModel.h"


Define_Module( UraePhyLayer );

UraePhyLayer::UraePhyLayer() {
	// TODO Auto-generated constructor stub

    m_pCurrentRaytrace = NULL;
    m_pLastRaytrace = NULL;

}

UraePhyLayer::~UraePhyLayer() {
	// TODO Auto-generated destructor stub
    delete m_pCurrentRaytrace;
    delete m_pLastRaytrace;
}



void UraePhyLayer::initialize(int stage) {

	PhyLayer::initialize( stage );
	if ( stage == 0 ) {

		mConstantK = par("constantK").doubleValue();
		mUseRaytracer = par("useRaytracing").boolValue();
		mRayCount = par("rayCount").longValue();
		mThreadCount = par("threadCount").longValue();
		mRaytraceDistance = par("raytraceDistance").doubleValue();

		this->recordScalar( "rayCount", mRayCount, NULL );
		this->recordScalar( "threadCount", mThreadCount, NULL );
		this->recordScalar( "raytraceDistance", mRaytraceDistance, "m" );
		this->recordScalar( "useRaytracer", mUseRaytracer ? 1 : 0, NULL );
		this->recordScalar( "constantK", mConstantK ? 1 : 0, NULL );

	}

}



AnalogueModel* UraePhyLayer::getAnalogueModelFromName( std::string name, ParameterMap& params ) {

	if ( name == "CORNER" ) {

		simtime_t interval = params["interval"].doubleValue();

		return new CORNERModel( interval );

	} else if ( name == "CarShadow" ) {

		simtime_t interval = params["interval"].doubleValue();
		double wavelength = params["wavelength"].doubleValue();

		return new CarShadowModel( interval, wavelength );

	}


	return PhyLayer::getAnalogueModelFromName( name, params );

}




void UraePhyLayer::handleUpperMessage(cMessage* msg) {

	// check if Radio is in TX state
	if (radio->getCurrentState() != Radio::TX)
	{
        delete msg;
        msg = 0;
		opp_error("Error: message for sending received, but radio not in state TX");
	}

	// check if not already sending
	if(txOverTimer->isScheduled())
	{
        delete msg;
        msg = 0;
		opp_error("Error: message for sending received, but radio already sending");
	}

	// build the AirFrame to send
	assert(dynamic_cast<cPacket*>(msg) != 0);

	AirFrame* frame = encapsMsg(static_cast<cPacket*>(msg));

	if ( mUseRaytracer ) {

		if ( Corner::BuildingSolver::GetSingleton() == NULL )
			opp_error( "Raytracer requires initialised Building Solver! Found none." );

		ExperimentScenarioManager *pManager = ExperimentScenarioManagerAccess().get();
		BaseMobility *mob = dynamic_cast<BaseMobility*>(this->getParentModule()->getParentModule()->getSubmodule("mobility",-1));

		Coord pos = pManager->ConvertCoords( mob->getCurrentPosition() );
		VectorMath::Vector2D vPos = VectorMath::Vector2D(pos.x, pos.y);
		// check if there is an existing raytrace
		if ( m_pCurrentRaytrace ) {
			// if there is, how far have we traveled since the most recent trace?
			if ( ( vPos - m_pCurrentRaytrace->GetTransmitterPosition() ).MagnitudeSq() >= mRaytraceDistance ) {
				// We've traveled further than the cut-off distance, so delete the last trace if there is one
				delete m_pLastRaytrace;
				// make the current trace the last one
				m_pLastRaytrace = m_pCurrentRaytrace;
				// and set the current raytrace pointer to NULL
				m_pCurrentRaytrace = NULL;
			}
		}

		// If there is no existing raytrace *OR* we just set it to NULL
		if ( !m_pCurrentRaytrace ) {

			// DO A NEW RAYTRACE!
			try {
				m_pCurrentRaytrace = new Corner::Raytracer( vPos, mRayCount, mThreadCount );
				m_pCurrentRaytrace->Execute();
			} catch( Exception &e ) {
				opp_error( e.What().c_str() );
			}

		}

		frame->addPar( "rayTrace" ).setPointerValue( m_pCurrentRaytrace );

	} else {

		frame->addPar( "riceanK" ).setDoubleValue( mConstantK );

	}

	// make sure there is no self message of kind TX_OVER scheduled
	// and schedule the actual one
	assert (!txOverTimer->isScheduled());
	sendSelfMessage(txOverTimer, simTime() + frame->getDuration());

	sendMessageDown(frame);
}











