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
}




AnalogueModel* UraePhyLayer::getAnalogueModelFromName( std::string name, ParameterMap& params ) {

	if ( name == "CORNER" ) {

		simtime_t interval = params["interval"].doubleValue();
		double k = -1;
		if ( params.find("k") != params.end() )
			k = params["k"].doubleValue();

		return new CORNERModel( interval, k );

	} else if ( name == "CarShadow" ) {

		simtime_t interval = params["interval"].doubleValue();
		double wavelength = params["wavelength"].doubleValue();

		return new CarShadowModel( interval, wavelength );

	}


	return PhyLayer::getAnalogueModelFromName( name, params );

}












