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

	std::string txRoadId, rxRoadId;
	TraCIMobility *pMobTx = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	TraCIMobility *pMobRx = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
	RsuMobility *pRsuTx = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	RsuMobility *pRsuRx = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
	UraeData::Classification c;

	if ( pMobTx )
		txRoadId = pMobTx->getRoadId();
	else if ( pRsuTx )
		txRoadId = pRsuTx->getRoadId();
	Coord posT = pManager->ConvertCoords( sendersPos );

	if ( pMobRx )
		rxRoadId = pMobRx->getRoadId();
	else if ( pRsuRx )
		rxRoadId = pRsuRx->getRoadId();
	Coord posR = pManager->ConvertCoords( receiverPos );

	c = UraeData::GetSingleton()->GetClassification( txRoadId, rxRoadId, Vector2D(posT.x,posT.y), Vector2D(posR.x,posR.y) );
	if ( c.mClassification == Classifier::LOS )
		kFactor = UraeData::GetSingleton()->GetK( c.mLinkPair, Vector2D(posT.x,posT.y), Vector2D(posR.x,posR.y) );

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




