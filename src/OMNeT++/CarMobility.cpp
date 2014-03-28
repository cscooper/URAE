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


#include "CarMobility.h"
#include "UraeScenarioManager.h"



Define_Module(CarMobility);


CarMobility::CarMobility() {

	mGridCell.x = -1;
	mGridCell.y = -1;

}


CarMobility::~CarMobility() {

	UraeScenarioManagerAccess().get()->updateModuleGrid( this, mGridCell, Coord(-1,-1,0) );

}


const Coord CarMobility::getGridCell() const {

	return mGridCell;

}


const std::string& CarMobility::getCarType() const {

	return mCarType;

}



const VectorMath::Vector3D& CarMobility::getCarDimensions() const {

	return mCarDimensions;

}



void CarMobility::nextPosition(const Coord& position, std::string road_id, double speed, double angle, TraCIScenarioManager::VehicleSignal signals ) {

	if ( this->road_id != road_id )
		updateLane();

	TraCIMobility::nextPosition( position, road_id, speed, angle, signals );

}



void CarMobility::changePosition() {

	TraCIMobility::changePosition();
	UraeScenarioManager *pManager = UraeScenarioManagerAccess().get();

	double dim = pManager->getGridSize();
	Coord lastPos = mGridCell;
	mGridCell.x = (int)(fabs(move.getStartPos().x) / dim);
	mGridCell.y = (int)(fabs(move.getStartPos().y) / dim);

	if ( mGridCell.x >= pManager->getGridWidth() )
		mGridCell.x = pManager->getGridWidth()-1;

	if ( mGridCell.y >= pManager->getGridHeight() )
		mGridCell.y = pManager->getGridHeight()-1;

	if ( lastPos != mGridCell )
		UraeScenarioManagerAccess().get()->updateModuleGrid( this, lastPos, mGridCell );
	else
		mGridCell = lastPos;

}



void CarMobility::initialize( int stage ) {

	TraCIMobility::initialize( stage );

	if ( stage == 0 ) {

		mCarType = UraeScenarioManagerAccess().get()->commandGetVehicleType( getExternalId() );
		mCarDimensions = Urae::UraeData::GetSingleton()->GetVehicleTypeDimensions( mCarType );

	}

}




/** Fetch the ID of the lane the car is in. */
void CarMobility::updateLane() {

	mLaneID = UraeScenarioManagerAccess().get()->commandGetVehicleType( getExternalId() );

}





