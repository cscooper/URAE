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
#include <queue>

#include "TraCIScenarioManager.h"
#include "CarShadowModel.h"
#include "BaseWorldUtility.h"
#include "AirFrame_m.h"
#include "ChannelAccess.h"

#include "UraeData.h"


DimensionSet CarShadowMapping::dimensions(Dimension::time,Dimension::frequency);


using namespace VectorMath;
using namespace Urae;

CarShadowMapping::CarShadowMapping( double v, const Argument& start, const Argument& interval, const Argument& end) :
					   SimpleConstMapping( dimensions, start, end, interval ), mValue(v) {
}


double CarShadowMapping::getValue( const Argument& pos ) const {

	if ( mValue < -1 )
		return 1;

	double PL = 0;

	/*
	 *	These are approximations derived by W.C.Y. Lee
	 *	See Rappaport, Wireless Communications Principles and Practice, Chapter 4, p.131
	 **/

	if ( -1 <= mValue && mValue <= 0 )
		PL = 0.5-0.62*mValue;
	else if ( 0 < mValue && mValue <= 1 )
		PL = 0.5*exp(-0.95*mValue);
	else if ( 1 < mValue && mValue <= 2.4 )
		PL = 0.4 - sqrt( 0.1184 - ( 0.38 - 0.1 * mValue ) * ( 0.38 - 0.1 * mValue ) );
	else if ( mValue > 2.4 )
		PL = 0.225 / mValue;

	return PL;

}


CarShadowModel::CarShadowModel( simtime_t_cref i, double mLambda ) : interval( i ), mWavelength(mLambda) {
	// TODO Auto-generated constructor stub
}

CarShadowModel::~CarShadowModel() {
	// TODO Auto-generated destructor stub
}




void CarShadowModel::filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos ) {

	Signal& signal = frame->getSignal();

	TraCIScenarioManager *pManager = TraCIScenarioManagerAccess().get();

	LineSegment l;
	l.mStart = Vector2D(  sendersPos.x,  sendersPos.y );
	l.mEnd   = Vector2D( receiverPos.x, receiverPos.y );
	Rect lineRect(l);


	TraCIMobility *pSenderMob = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	TraCIMobility *pReceiverMob = dynamic_cast<TraCIMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());

	if ( !pSenderMob )
		return;

	double txHeight = UraeData::GetSingleton()->GetVehicleClassDimensions( pSenderMob->commandGetVehicleClass() ).z;

	bool bFound = false;

	// find the vehicles on the line between Tx and Rx
	std::map<std::string, cModule*> *pCars = pManager->getManagedHostsPtr();
	std::map<std::string, cModule*>::iterator it;
	CarEntry vTx[3], vRx[3];
	double Dtr = l.GetDistance();
	vTx[0].mDistance = vTx[1].mDistance = vTx[2].mDistance = vRx[0].mDistance = vRx[1].mDistance = vRx[2].mDistance = Dtr;
	vTx[0].mHeight = vRx[0].mHeight = 0;
	vTx[1].mHeight = vRx[1].mHeight = 0;
	vTx[2].mHeight = vRx[2].mHeight = 0;

	for ( AllInVector( it, (*pCars) ) ) {

		TraCIMobility *pMob = dynamic_cast<TraCIMobility*>(it->second->getSubmodule("mobility",-1));

		Coord vPos = pMob->getCurrentPosition();

		if ( pMob == pSenderMob || pMob == pReceiverMob )
			continue;

		double angle = pMob->getAngleRad();

		Vector2D v1, v2, p, heading1, heading2;
		p = Vector2D( vPos.x, vPos.y );
		heading1 = Vector2D( cos(angle), sin(angle) );
		heading2 = Vector2D( heading1.y, -heading1.x );
		LineSegment d1, d2;

		Vector3D currDims = UraeData::GetSingleton()->GetVehicleClassDimensions( pMob->commandGetVehicleClass() );
		d1 = LineSegment( p+heading1*currDims.y/2+heading2*currDims.x/2, p-heading1*currDims.y/2-heading2*currDims.x/2 );
		d2 = LineSegment( p+heading1*currDims.y/2-heading2*currDims.x/2, p-heading1*currDims.y/2+heading2*currDims.x/2 );

		bool d1Int = d1.IntersectLine( l, &v1 ), d2Int = d2.IntersectLine( l, &v2 );

		if ( d1Int || d2Int ) {

			double h1 = currDims.z - txHeight, h2, h3, dTx, dRx;
			if ( d1Int ) {
				h2 = (d1.mStart-v1).Magnitude();
				h3 = (  d1.mEnd-v1).Magnitude();
				dTx = (l.mStart-v1).Magnitude();
				dRx = (  l.mEnd-v1).Magnitude();
			} else if ( d2Int ) {
				h2 = (d2.mStart-v2).Magnitude();
				h3 = (  d2.mEnd-v2).Magnitude();
				dTx = (l.mStart-v2).Magnitude();
				dRx = (  l.mEnd-v2).Magnitude();
			}


			if ( h1/dTx > vTx[0].mHeight/vTx[0].mDistance ) {
				vTx[0].mHeight = h1;
				vTx[0].mDistance = dTx;
			}
			if ( h2/dTx > vTx[1].mHeight/vTx[1].mDistance ) {
				vTx[1].mHeight = h2;
				vTx[1].mDistance = dTx;
			}
			if ( h3/dTx > vTx[2].mHeight/vTx[2].mDistance ) {
				vTx[2].mHeight = h3;
				vTx[2].mDistance = dTx;
			}

			if ( h1/dRx > vRx[0].mHeight/vRx[0].mDistance ) {
				vRx[0].mHeight = h1;
				vRx[0].mDistance = dTx;
			}
			if ( h2/dRx > vRx[1].mHeight/vRx[1].mDistance ) {
				vRx[1].mHeight = h2;
				vRx[1].mDistance = dTx;
			}
			if ( h3/dRx > vRx[2].mHeight/vRx[2].mDistance ) {
				vRx[2].mHeight = h3;
				vRx[2].mDistance = dTx;
			}

			bFound = true;

		} else {

			continue;

		}

	}


	if ( !bFound )
		return;

	bool cond1, cond2;
	double aTx, aRx, gamma, d1, d2, h, v;

	for ( int i = 0; i < 3; i++ ) {

		aTx = atan(vTx[i].mHeight/vTx[i].mDistance);
		aRx = atan(vRx[i].mHeight/vRx[i].mDistance);
		gamma = tan(aRx)/tan(aTx);
		cond1 = (Dtr-vTx[i].mDistance)*tan(aRx) > vTx[i].mHeight;
		cond2 = (Dtr-vRx[i].mDistance)*tan(aTx) > vRx[i].mHeight;

		if ( cond1 && cond2 ) {

			d2 = Dtr / ( 1 + gamma );
			d1 = gamma * d2;
			h = d1 * tan( aTx );

		} else if ( !cond1 && cond2 ) {

			d1 = vTx[i].mDistance;
			d2 = Dtr - vRx[i].mDistance;
			h = vTx[i].mHeight;

		} else if ( cond1 && !cond2 ) {

			d1 = Dtr - vTx[i].mDistance;
			d2 = vRx[i].mDistance;
			h = vRx[i].mHeight;

		} else {
			continue;
		}

		v = h * sqrt( 2 * (d1 + d2) / ( mWavelength * d1 * d2 ) );

		signal.addAttenuation(
				new CarShadowMapping(
						v,
						Argument(signal.getReceptionStart()),
						Argument(0.1),
						Argument(signal.getReceptionEnd())
						)
		);

	}

}




