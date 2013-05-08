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

#include "UraeScenarioManager.h"
#include "RsuMobility.h"
#include "CORNERModel.h"

#include "BaseWorldUtility.h"
#include "AirFrame_m.h"
#include "ChannelAccess.h"

#include <queue>
#include <fstream>



DimensionSet CORNERModel::dimensions(Dimension::time,Dimension::frequency);
DimensionSet CORNERMapping::dimensions(Dimension::time,Dimension::frequency);


using namespace VectorMath;
using namespace Urae;

CORNERMapping::CORNERMapping( Coord tPos, Coord rPos, Urae::UraeData::Classification c, double k, const Argument& start, const Argument& interval, const Argument& end) :
					   SimpleConstMapping( dimensions, start, end, interval ), txPos( tPos ), rxPos( rPos ), mClassification( c ), kFactor(k) {
}


double CORNERMapping::getValue( const Argument& pos ) const {
    Classifier classifier(mClassification);
    return classifier.CalculatePathloss( Vector2D( txPos.x, txPos.y ), Vector2D( rxPos.x, rxPos.y ) ) / UraeData::GetSingleton()->GetSystemLoss();
}


CORNERModel::CORNERModel( simtime_t i ) {
	interval = i;
}

CORNERModel::~CORNERModel() {
	// TODO Auto-generated destructor stub
}




void CORNERModel::filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos ) {

	Signal& signal = frame->getSignal();
	double kFactor = 0;

	UraeScenarioManager *pManager = UraeScenarioManagerAccess().get();

	bool txHasMapping = false, rxHasMapping = false;
	TraCIMobility *pMobTx = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	TraCIMobility *pMobRx = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
	RsuMobility *pRsuTx = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	RsuMobility *pRsuRx = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
	UraeData::Classification c;
	int txIndex, rxIndex;

	if ( pMobTx )
		txHasMapping = UraeData::GetSingleton()->LinkHasMapping( pMobTx->getRoadId(), &txIndex );
	else if ( pRsuTx )
		txHasMapping = UraeData::GetSingleton()->LinkHasMapping( pRsuTx->getRoadId(), &txIndex );

	if ( pMobRx )
		rxHasMapping = UraeData::GetSingleton()->LinkHasMapping( pMobRx->getRoadId(), &rxIndex );
	else if ( pRsuRx )
		rxHasMapping = UraeData::GetSingleton()->LinkHasMapping( pRsuRx->getRoadId(), &rxIndex );


	Coord posR = pManager->ConvertCoords( receiverPos );
	Coord posT = pManager->ConvertCoords( sendersPos );
	Vector2D vTx = Vector2D(posT.x, posT.y);
	Vector2D vRx = Vector2D(posR.x, posR.y);

	// Get the CORNER classification
	if ( txHasMapping && rxHasMapping ) {

		// the source and destination are on edges for which we have a mapping
		c = UraeData::GetSingleton()->GetClassification( txIndex, rxIndex );

	} else {

		// Either the source or destination are on an internal edge.
		// Currently we have no mappings to a classification for internal
		// edges, and so must manually calculate one
		Classifier cls;

		cls.CalculatePathloss( vTx, vRx );
		c = cls.GetClassification();

		txIndex = cls.GetSourceLink();
		rxIndex = cls.GetDestinationLink();

	}

	// get the pre-computed K-factor for this position
	kFactor = UraeData::GetSingleton()->GetK( UraeData::LinkPair(txIndex,rxIndex), vTx, vRx );

	signal.addAttenuation(
			new CORNERMapping(
					posT,
					posR,
					c,
					kFactor,
					Argument(signal.getReceptionStart()),
					Argument(interval),
					Argument(signal.getReceptionEnd())
					)
	);


	if ( kFactor != DBL_MAX ) {

		// we have fading at this point
		Mapping *att = MappingUtils::createMapping( dimensions, Mapping::LINEAR );
		Argument pos;

		for ( simtime_t t = signal.getReceptionStart(); t <= signal.getReceptionEnd(); t += interval ) {

			pos.setTime( t );
			double f = Fading::GetSingleton()->CalculateFading( c.mClassification, kFactor );
			att->appendValue( pos, f );

		}
		signal.addAttenuation( att );

	}

}




