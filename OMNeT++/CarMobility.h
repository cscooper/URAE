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

#ifndef __RAYTRACEREXPERIMENT_CARMOBILITY_H_
#define __RAYTRACEREXPERIMENT_CARMOBILITY_H_

#include <omnetpp.h>

/**
 * TODO - Generated class
 */
class CarMobility : public TraCIMobility
{

public:
    double GetLength();
    double GetWidth();
    double GetHeight();

    void SetLength(double l);
    void SetWidth(double w);
    void SetHeight(double h);

protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    double mLength, mWidth, mHeight;
};

#endif
