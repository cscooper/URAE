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


#include <fstream>
#include <queue>
#include <algorithm>
#include "UraeScenarioManager.h"

#include "Urae.h"
#include "TraCIConstants.h"


using namespace VectorMath;
using namespace Urae;


Define_Module(UraeScenarioManager);

UraeScenarioManager::UraeScenarioManager() {
	// TODO Auto-generated constructor stub
}

UraeScenarioManager::~UraeScenarioManager() {
	// TODO Auto-generated destructor stub
}


void UraeScenarioManager::initialize(int stage) {

	TraCIScenarioManagerLaunchd::initialize( stage );

	if( stage == 0 ) {

		try {
			mUraeData = new Urae::UraeData( par("linksFile").stringValue(),
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
		    opp_error("Urae::UraeData Initialization failed for some reason.");

		try {
			mFading = new Urae::Fading( par("componentFile").stringValue(), par("randSeed").longValue() );
		} catch (Exception &e) {
			opp_error(e.What().c_str());
		}

		// set up the grid space
		mGridSize = par("gridSize").longValue();
		VectorMath::Rect r = mUraeData->GetMapRect();
		mGridWidth = ceil( r.size.x / mGridSize );
		mGridHeight = ceil( r.size.y / mGridSize );
		mGridLookup = new GridCell* [mGridWidth];
		for ( int i = 0; i < mGridWidth; i++ )
			mGridLookup[i] = new GridCell[mGridHeight];

	}

}


void UraeScenarioManager::finish() {

	if ( mUraeData )
		delete mUraeData;
	if ( mFading )
		delete mFading;

	TraCIScenarioManagerLaunchd::finish();

}


int UraeScenarioManager::getGridSize() const {

	return mGridSize;

}



void UraeScenarioManager::updateModuleGrid( CarMobility *pMod, Coord prev, Coord next ) {

	if ( prev.x > 0 && prev.y > 0 )
		mGridLookup[(int)prev.x][(int)prev.y].erase( std::remove( mGridLookup[(int)prev.x][(int)prev.y].begin(), mGridLookup[(int)prev.x][(int)prev.y].end(), pMod ), mGridLookup[(int)prev.x][(int)prev.y].end() );

	if ( next.x > 0 && next.y > 0 )
		mGridLookup[(int)next.x][(int)next.y].push_back( pMod );

}


const UraeScenarioManager::GridCell& UraeScenarioManager::getGridCell( int x, int y ) const {
	return mGridLookup[x][y];
}



std::string UraeScenarioManager::commandGetVehicleType( std::string vehicleId ) {

	return genericGetString( CMD_GET_VEHICLE_VARIABLE, vehicleId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE );

}


bool UraeScenarioManager::commandCreateRoute( std::string routeId, std::list<std::string> edgeList ) {

	bool success = false;
	std::list<std::string>::iterator strIt;
	TraCIBuffer buffer, retBuf;

	buffer << ADD << routeId << TYPE_STRINGLIST;
	for ( AllInVector( strIt, edgeList ) )
		buffer << (*strIt);

	retBuf = queryTraCIOptional( CMD_SET_ROUTE_VARIABLE, buffer, success );
	ASSERT(retBuf.eof());
	return success;

}


std::map<std::string, cModule*> *UraeScenarioManager::getManagedHostsPtr() {
	return &hosts;
}


Coord UraeScenarioManager::ConvertCoords( Coord p, bool fromTraci ) {

	TraCICoord t;
	Coord r;

	if ( fromTraci ) {

		t.x = p.x;
		t.y = p.y;
		r = traci2omnet( t );

	} else {

		t = omnet2traci( p );
		r.x = t.x;
		r.y = t.y;

	}

	return r;

}





double UraeScenarioManager::ConvertAngle( double a, bool fromTraci ) {

    double r;

    if ( fromTraci ) {

        r = this->traci2omnetAngle( a );

    } else {

        r = this->omnet2traciAngle( a );

    }

    return r;

}




