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


Coord CarMobility::getGridCell() {

	return mGridCell;

}


void CarMobility::changePosition() {

	TraCIMobility::changePosition();

	double dim = UraeScenarioManagerAccess().get()->getGridSize();
	Coord lastPos = mGridCell;
	mGridCell.x = (int)(move.getStartPos().x / dim);
	mGridCell.y = (int)(move.getStartPos().y / dim);
	if ( lastPos != mGridCell )
		UraeScenarioManagerAccess().get()->updateModuleGrid( this, lastPos, mGridCell );
	else
		mGridCell = lastPos;

}



