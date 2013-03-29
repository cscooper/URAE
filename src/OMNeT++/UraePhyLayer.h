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

#ifndef URAEPHYLAYER_H_
#define URAEPHYLAYER_H_

#include <queue>
#include "PhyLayer80211p.h"

class UraePhyLayer: public PhyLayer {
public:
	UraePhyLayer();
	virtual ~UraePhyLayer();

protected:
	virtual AnalogueModel* getAnalogueModelFromName( std::string name, ParameterMap& params );

};

#endif /* CORNERPHYLAYER_H_ */
