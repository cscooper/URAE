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

#include "TraCIScenarioManager.h"
#include "CORNERModel.h"

#include "BaseWorldUtility.h"
#include "AirFrame_m.h"
#include "ChannelAccess.h"

#include <queue>
#include <fstream>





DimensionSet CORNERMapping::dimensions(Dimension::time,Dimension::frequency);


using namespace VectorMath;
using namespace Urae;

CORNERMapping::CORNERMapping( Coord tPos, Coord rPos, double k, Urae::UraeData::Classification c, const Argument& start, const Argument& interval, const Argument& end) :
					   SimpleConstMapping( dimensions, start, end, interval ), txPos( tPos ), rxPos( rPos ), kFactor( k ), mClassification( c ) {
}


double CORNERMapping::getValue( const Argument& pos ) const {

	TraCIScenarioManager *pManager = TraCIScenarioManagerAccess().get();
	TraCIScenarioManager::TraCICoord posT = pManager->omnet2traci( txPos );
	TraCIScenarioManager::TraCICoord posR = pManager->omnet2traci( rxPos );

    Vector2D vTx = Vector2D(posT.x, posT.y);
    Vector2D vRx = Vector2D(posR.x, posR.y);

    // get classification...
    Classifier classifier(mClassification);
    double PL = classifier.CalculatePathloss( vTx, vRx ) / UraeData::GetSingleton()->GetSystemLoss();
    double fading = Fading::GetSingleton()->CalculateFading( mClassification.mClassification, kFactor );

    return PL * fading;

}


CORNERModel::CORNERModel( simtime_t_cref i ) : interval( i ) {
	// TODO Auto-generated constructor stub
}

CORNERModel::~CORNERModel() {
	// TODO Auto-generated destructor stub
}




void CORNERModel::filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos ) {

	Signal& signal = frame->getSignal();
	double kFactor = 0;

	TraCIScenarioManager *pManager = TraCIScenarioManagerAccess().get();

	TraCIMobility *pMobTx = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	TraCIMobility *pMobRx = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());

	int txIndex, rxIndex;
	bool txHasMapping = UraeData::GetSingleton()->LinkHasMapping( pMobTx->getRoadId(), &txIndex );
	bool rxHasMapping = UraeData::GetSingleton()->LinkHasMapping( pMobRx->getRoadId(), &rxIndex );
	UraeData::Classification c;

	TraCIScenarioManager::TraCICoord posR = pManager->omnet2traci( receiverPos );
	TraCIScenarioManager::TraCICoord posT = pManager->omnet2traci( sendersPos );
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
					sendersPos,
					receiverPos,
					kFactor,
					c,
					Argument(signal.getReceptionStart()),
					Argument(0.1),
					Argument(signal.getReceptionEnd())
					)
	);

}




