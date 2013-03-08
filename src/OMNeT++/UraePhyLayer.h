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

#ifndef CORNERPHYLAYER_H_
#define CORNERPHYLAYER_H_

#include <queue>
#include "PhyLayer80211p.h"

class UraePhyLayer: public PhyLayer {
public:
	UraePhyLayer();
	virtual ~UraePhyLayer();

	virtual void initialize(int stage);

protected:
	virtual AnalogueModel* getAnalogueModelFromName( std::string name, ParameterMap& params );
	virtual void handleUpperMessage(cMessage* msg);


    Corner::Raytracer *m_pCurrentRaytrace;
    Corner::Raytracer *m_pLastRaytrace;

    bool mUseRaytracer;
    double mConstantK;
    int mRayCount;
    int mThreadCount;
    double mRaytraceDistance;		// distance before running new raytrace

};

#endif /* CORNERPHYLAYER_H_ */
