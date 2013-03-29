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

#ifndef URAESCENARIOMANAGER_H_
#define URAESCENARIOMANAGER_H_

#include <TraCIScenarioManagerLaunchd.h>

#include "Urae.h"


class UraeScenarioManager: public TraCIScenarioManagerLaunchd {

protected:
	Urae::UraeData *mUraeData;
	Urae::Fading *mFading;

public:
	UraeScenarioManager();
	virtual ~UraeScenarioManager();

	virtual void initialize(int stage);
	virtual void finish();

	std::string commandGetVehicleType(std::string vehicleId);
	bool commandCreateRoute(std::string routeId,std::list<std::string> edgeList);

	std::map<std::string, cModule*> *getManagedHostsPtr();
	Coord ConvertCoords( Coord p, bool fromTraci = false );
	double ConvertAngle( double a, bool fromTraci = false );


};



class UraeScenarioManagerAccess
{
	public:
	UraeScenarioManager* get() {
			return FindModule<UraeScenarioManager*>::findGlobalModule();
		};
};




#endif /* DATACOLLECTORSCENARIOMANAGER_H_ */
