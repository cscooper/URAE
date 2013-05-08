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

#ifndef CORNERMODEL_H_
#define CORNERMODEL_H_

#include "MiXiMDefs.h"
#include "AnalogueModel.h"
#include "Mapping.h"

#include "Urae.h"



class MIXIM_API CORNERModel: public AnalogueModel {

protected:
	static DimensionSet dimensions;
	simtime_t interval;

public:
	CORNERModel( simtime_t i );
	virtual ~CORNERModel();

	virtual void filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos );
};



class MIXIM_API CORNERMapping : public SimpleConstMapping {

protected:
	static DimensionSet dimensions;
	Coord txPos;
	Coord rxPos;
	Urae::UraeData::Classification mClassification;
	double kFactor;

public:
	CORNERMapping( Coord tPos,
				   Coord rPos,
				   Urae::UraeData::Classification c,
				   double k,
				   const Argument& start,
				   const Argument& interval,
				   const Argument& end);

	virtual double getValue( const Argument& pos ) const;
	double GetK() const { return kFactor; }

	ConstMapping* constClone() const {
		return new CORNERMapping( *this );
	}



};


#endif /* CORNERMODEL_H_ */
