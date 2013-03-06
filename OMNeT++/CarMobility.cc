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

#include <TraCIMobility.h>
#include "CarMobility.h"

Define_Module(CarMobility);


double CarMobility::GetLength() { return mLength; }
double CarMobility::GetWidth() { return mWidth; }
double CarMobility::GetHeight() { return mHeight; }


void CarMobility::SetLength(double l) { mLength = l; }
void CarMobility::SetWidth(double w) { mWidth = w; }
void CarMobility::SetHeight(double h) { mHeight = h; }

void CarMobility::initialize(int stage)
{

	TraCIMobility::initialize( stage );

	double r = (double)rand() / (double)RAND_MAX;
	if ( r <= 0.75 ) {
		mLength = 3;
		mWidth = 1.5;
		mHeight = 1.2;
	} else if ( r-0.75 < 0.2 ) {
		mLength = 3.66;
		mWidth = 1.83;
		mHeight = 1.83;
	} else if ( r-0.95 < 0.05 ) {
		mLength = 12.2;
		mWidth = 1.8;
		mHeight = 3.35;
	}

}

void CarMobility::handleMessage(cMessage *msg)
{
    TraCIMobility::handleMessage(msg);
}





