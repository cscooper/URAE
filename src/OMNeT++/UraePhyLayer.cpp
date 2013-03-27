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
#include "UraePhyLayer.h"
#include "CORNERModel.h"
#include "CarShadowModel.h"


Define_Module( UraePhyLayer );

UraePhyLayer::UraePhyLayer() {
	// TODO Auto-generated constructor stub


}

UraePhyLayer::~UraePhyLayer() {
	// TODO Auto-generated destructor stub

	if ( Urae::UraeData::GetSingleton() )
		delete Urae::UraeData::GetSingleton();
	if ( Urae::Fading::GetSingleton() )
		delete Urae::Fading::GetSingleton();

}



void UraePhyLayer::initialize(int stage) {

	PhyLayer::initialize( stage );
	if ( stage == 0 ) {

		try {
			new Urae::UraeData( par("linksFile").stringValue(),
								par("nodesFile").stringValue(),
								par("classFile").stringValue(),
								NULL,
								par("linkMapFile").stringValue(),
								par("riceFile").stringValue(),
								par("carDefFile").stringValue(),
								par("laneWidth").doubleValue(),
								par("waveLength").doubleValue(),
								par("txPower").doubleValue(),
								par("systemLoss").doubleValue(),
								FWMath::dBm2mW( par("sensitivity").doubleValue() ),
								par("lossPerReflection").doubleValue(), 200 );
		} catch (Exception &e) {
			opp_error(e.What().c_str());
		}

		if ( Urae::UraeData::GetSingleton() == NULL )
		    opp_error("Urae Initialization failed for some reason.");

		try {
			new Urae::Fading( par("componentFile").stringValue(), par("randSeed").longValue() );
		} catch (Exception &e) {
			opp_error(e.What().c_str());
		}

		if ( Urae::Fading::GetSingleton() == NULL )
		    opp_error("Urae Fader failed to initialize for some reason.");

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












