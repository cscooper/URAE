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

#ifndef __URAE_CARMOBILITY_H_
#define __URAE_CARMOBILITY_H_

#include <omnetpp.h>
#include "TraCIMobility.h"

/**
 * CarMobility.
 * This class keeps track of the grid cell the car is located in.
 * This helps the shadowing model exclude vehicles not directly located
 * between sender and receiver.
 */
class CarMobility : public TraCIMobility
{

public:
	CarMobility();
	virtual ~CarMobility();

	Coord getGridCell();

	virtual void changePosition();

protected:

    Coord mGridCell;	/**< The grid cell in which this car is located. */

};

#endif
