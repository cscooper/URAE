/*
 *  UraeData.h - Contains the building geometry and CORNER classifications for a sumo map.
 *  Copyright (C) 2012  C. S. Cooper, A. Mukunthan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact Details: Cooper - andor734@gmail.com
 */

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cfloat>
#include <string>
#include <list>
#include <map>
#include <climits>

#include "Singleton.h"
#include "VectorMath.h"
#include "UraeData.h"
#include "Classifier.h"

using namespace std;
using namespace VectorMath;
using namespace Urae;


DECLARE_SINGLETON( UraeData );






Real UraeData::GetWavelength() {
	return mWavelength;
}

Real UraeData::GetLamdaBy4PiSq() {
	return mLambdaBy4PiSq;
}

Real UraeData::GetTransmitPower() {
	return mTransmitPower;
}


Real UraeData::GetSystemLoss() {
	return mSystemLoss;
}


Real UraeData::GetReceiverSensitivity() {
	return mSensitivity;
}


Real UraeData::GetFreeSpaceRange() {
	return mFreeSpaceRange;
}


Real UraeData::GetLaneWidth() {
	return mLaneWidth;
}


Real UraeData::GetLossPerReflection() {
	return mLossPerReflection;
}





UraeData::UraeData( VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid ) {

	mLaneWidth = laneWidth;
	mWavelength = lambda;
	mTransmitPower = txPower;
	mSystemLoss = L;
	mSensitivity = sensitivity; 
	mLossPerReflection = lpr;
	mGridSize = grid;
	mBucketSize = grid;
	mLambdaBy4PiSq = pow( mWavelength / (4 * M_PI), 2 );
	mFreeSpaceRange = ( mWavelength / ( 4 * M_PI ) ) * sqrt( mTransmitPower / ( mSystemLoss * mSensitivity ) );

}


/*
 * Constructor Arguments:
 * 		1. linksFile - file name of the CORNER links file
 * 		2. nodesFile - file name of the CORNER nodes file
 * 		3. classFile - file name of the CORNER class file
 * 		4. buildingFile - file name of the CORNER building file
 * 		5. linkMapFile - file name of the CORNER link mapping file
 * 		6. laneWidth - width of one lane in metres
 * 		7. lambda - wavelength of the carrier signal
 * 		8. txPower - transmission power of the signal
 * 		9. L - losses due to the system (signal processing, etc) not related to propagation
 * 		10. sensitivity - the sensitivity of the receiver
 * 		11. lpr - The loss per reflection
 */
UraeData::UraeData(
		const char* linksFile,
		const char* nodesFile,
		const char* classFile,
		const char* buildingFile,
		const char* linkMapFile, 
		VectorMath::Real laneWidth, 
		VectorMath::Real lambda, 
		VectorMath::Real txPower, 
		VectorMath::Real L, 
		VectorMath::Real sensitivity, 
		VectorMath::Real lpr, 
		VectorMath::Real grid ) { 

	mLaneWidth = laneWidth;
	mWavelength = lambda;
	mTransmitPower = txPower;
	mSystemLoss = L; 
	mSensitivity = sensitivity; 
	mLossPerReflection = lpr; 
	mGridSize = grid; 
	mBucketSize = grid; 
	mLambdaBy4PiSq = pow( mWavelength / (4 * M_PI), 2 );
	mFreeSpaceRange = sqrt( mLambdaBy4PiSq * mTransmitPower / ( mSystemLoss * mSensitivity ) );

	LoadNetwork( linksFile, nodesFile, classFile, buildingFile, linkMapFile, NULL );
	ComputeSummedLinkSet();
	ComputeBuckets();

}



/*
 * Constructor Arguments:
 * 		1. linksFile - file name of the CORNER links file
 * 		2. nodesFile - file name of the CORNER nodes file
 * 		3. classFile - file name of the CORNER class file
 * 		4. buildingFile - file name of the CORNER building file
 * 		5. linkMapFile - file name of the CORNER link mapping file
 * 		6. riceDataFile - file name of the pre-computed K-factor data
 * 		7. laneWidth - width of one lane in metres
 * 		8. lambda - wavelength of the carrier signal
 * 		9. txPower - transmission power of the signal
 * 		10. L - losses due to the system (signal processing, etc) not related to propagation
 * 		11. sensitivity - the sensitivity of the receiver
 * 		12. lpr - The loss per reflection
 */
UraeData::UraeData(
		const char* linksFile,
		const char* nodesFile,
		const char* classFile,
		const char* buildingFile,
		const char* linkMapFile, 
		const char* riceDataFile, 
		VectorMath::Real laneWidth, 
		VectorMath::Real lambda, 
		VectorMath::Real txPower, 
		VectorMath::Real L, 
		VectorMath::Real sensitivity, 
		VectorMath::Real lpr, 
		VectorMath::Real grid ) { 

	mLaneWidth = laneWidth;
	mWavelength = lambda;
	mTransmitPower = txPower;
	mSystemLoss = L; 
	mSensitivity = sensitivity; 
	mLossPerReflection = lpr; 
	mGridSize = grid; 
	mBucketSize = grid; 
	mLambdaBy4PiSq = pow( mWavelength / (4 * M_PI), 2 );
	mFreeSpaceRange = sqrt( mLambdaBy4PiSq * mTransmitPower / ( mSystemLoss * mSensitivity ) );

	LoadNetwork( linksFile, nodesFile, classFile, NULL, linkMapFile, riceDataFile );
	ComputeSummedLinkSet();
	ComputeBuckets();

}


UraeData::~UraeData() {

	mNodeSet.clear();
	mLinkSet.clear();
	mSummedLinkSet.clear();
	mClassificationMap.clear();
	mBuildingSet.clear();

}


/*
 * Method: void AddBuilding( Building newBuilding );
 * Description: Manually add a building to the network
 */
void UraeData::AddBuilding( UraeData::Building newBuilding ) {
	mBuildingSet.push_back( newBuilding );
}



/*
 * Method: void CalculateMapRectangle();
 * Description: Calculate map bounds after manually adding buildings.
 */
void UraeData::CalculateMapRectangle() {

	Vector2D topLeft, bottomRight, v1, v2;
	topLeft = Vector2D(DBL_MAX,DBL_MAX);
	bottomRight = Vector2D(DBL_MIN,DBL_MIN) * -1;
	std::vector<Real> dX, dY;

	for ( BuildingSet::iterator it = mBuildingSet.begin(); it != mBuildingSet.end(); it++ ) {

		for ( LineSet::iterator lineIt = it->mEdgeSet.begin(); lineIt != it->mEdgeSet.end(); lineIt++ ) {

			dX.push_back( lineIt->mStart.x );
			dY.push_back( lineIt->mStart.y );
			dX.push_back(   lineIt->mEnd.x );
			dY.push_back(   lineIt->mEnd.y );

		}

		v1.x = *std::min_element( dX.begin(), dX.end() );
		v1.y = *std::min_element( dY.begin(), dY.end() );
		v2.x = *std::max_element( dX.begin(), dX.end() );
		v2.y = *std::max_element( dY.begin(), dY.end() );
		dX.clear();
		dY.clear();

		topLeft.x     = std::min( v1.x, topLeft.x );
		topLeft.y     = std::min( v1.y, topLeft.y );
		bottomRight.x = std::max( v2.x, topLeft.x );
		bottomRight.y = std::max( v2.y, topLeft.y );

	}

	mMapRect = Rect( topLeft, bottomRight - topLeft );

}


/*
 * Method: Rect GetMapRect();
 * Description: Gets the bounds of the road network.
 */
Rect UraeData::GetMapRect() {
	return mMapRect;
}


/*
 * Method: Link *GetLink( int index );
 * Description: Gets a pointer to a link of the given index.
 */
UraeData::Link *UraeData::GetLink( int index ) {
	return &mLinkSet[ index ];
}




/*
 * Method: Node *GetNode( int index );
 * Description: Gets a pointer to a node of the given index.
 */
UraeData::Node *UraeData::GetNode( int index ) {
	return &mNodeSet[ index ];
}




/*
 * Method: Link *GetSummedLink( int index );
 * Description: Gets a pointer to a summed link of the given index.
 */
UraeData::Link *UraeData::GetSummedLink( int index ) {
	return &mSummedLinkSet[ index ];
}




/*
 * Method: Classification GetClassification( int l1, int l2 );
 * Description: Get the CORNER classification between the given links.
 */
UraeData::Classification UraeData::GetClassification( int l1, int l2 ) {
	std::pair<int,int> linkPair;
	
	if (l1 < l2) {
		linkPair.first = l1;
		linkPair.second = l2;
	} else {
		linkPair.first = l2;
		linkPair.second = l1;
	}

	ClassificationMap::iterator cm = mClassificationMap.find(linkPair);
	if ( cm != mClassificationMap.end() ) {
		return cm->second;
	} else {
		Classification c;
		c.mClassification = Classifier::OutOfRange;
		c.mFullNodeCount = INT_MAX;
		return c;
	}
}


/*
 * Method: Classification GetClassification( std::string link1, std::string link2 );
 * Description: Get the CORNER classification between the given links (by name).
 */
UraeData::Classification UraeData::GetClassification( std::string link1, std::string link2 ) {

	return GetClassification( mLinkIndexMap[link1], mLinkIndexMap[link2] );

}



/*
 * Method: VectorMath::Real GetK( LinkPair p, Vector2D srcPos, Vector2D destPos );
 * Description: Get the pre-computed k-factor between the given source and destination.
 */
Real UraeData::GetK( UraeData::LinkPair p, Vector2D srcPos, Vector2D destPos ) {

	Real k = 0;
	Real sMin = DBL_MAX, dMin = DBL_MAX;
	RiceFactorData::iterator riceIt;
	for ( AllInVector( riceIt, mRiceFactorData[p] ) ) {

		Real sDiff = (     riceIt->mSource- srcPos).MagnitudeSq();
		Real dDiff = (riceIt->mDestination-destPos).MagnitudeSq();

		if ( sMin >= sDiff && dMin >= dDiff ) {
			sMin = sDiff;
			dMin = dDiff;
			k = riceIt->mKfactor;
		}

	}

	return k;

}




/*
 * Method: bool LinkHasMapping( std::string linkName, int *pMapping );
 * Description: Returns true if the given link name is mapped to an index.
 */
bool UraeData::LinkHasMapping( std::string linkName, int *pMapping ) {

	bool hasMapping = mLinkIndexMap.find(linkName) != mLinkIndexMap.end();
	if ( hasMapping && pMapping )
		*pMapping = mLinkIndexMap[linkName];
	return hasMapping;

}


/*
 * Method: Building *GetBuilding( int index );
 * Description: Get the building at the given index.
 */
UraeData::Building *UraeData::GetBuilding( int index ) {
	return &mBuildingSet[index];
}


/*
 * Method: void GetGrid(Vector2D position) {
 * Description: Gets the grid associated with the specified position
 */
UraeData::Grid* UraeData::GetGrid(Vector2D position) {
	return( &(mGridList[int(position.y/mGridSize)][int(position.x/mGridSize)]) );
	
}

/*
 * Method: void LoadNetwork( char* linksFile, char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* riceDataFile );
 * Description: Loads the data from the links and nodes files.
 */
void UraeData::LoadNetwork( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* riceDataFile ) {
	ifstream stream;
	char buffer[20];

	int numNodesInFile, numLinksInFile, numClassInFile, numBuildingsInFile, numLinkMappings, numRice;
	Vector2D topLeft, bottomRight;
	UraeData::Node tempNode;
	UraeData::Link tempLink;

	// read the nodes file
	if ( nodesFile ) { 

		stream.open( nodesFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open nodes file: %s", nodesFile );
		}

		stream >> dec >> numNodesInFile;

		mNodeSet.reserve(numNodesInFile);

		for(int n = 0; n < numNodesInFile; n++) {
			stream >> tempNode.index >> tempNode.position.x >> tempNode.position.y;
			if ( topLeft.x > tempNode.position.x )
				topLeft.x = tempNode.position.x;
			if ( bottomRight.x < tempNode.position.x )
				bottomRight.x = tempNode.position.x;
			if ( topLeft.y > tempNode.position.y )
				topLeft.y = tempNode.position.y;
			if ( bottomRight.y < tempNode.position.y )
				bottomRight.y = tempNode.position.y;
			mNodeSet.push_back(tempNode);
		}
	
		stream.close();

	}

	// read the links file
	if ( linksFile ) {

		stream.open( linksFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open links file: %s", linksFile );
		}

		stream >> dec >> numLinksInFile;

		mLinkSet.reserve(numLinksInFile);
		for(int l = 0; l < numLinksInFile; l++)
		{
			stream >> tempLink.index
				>> tempLink.nodeAindex
				>> tempLink.nodeBindex
				>> tempLink.NumberOfLanes
				>> buffer // Boarder segment is not in use
				>> tempLink.flow
				>> tempLink.speed;
			mLinkSet.push_back(tempLink);
		}

		stream.close();

	}

	// read the classification files
	if ( classFile ) {

		stream.open( classFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open classification file: %s", classFile );
		}

		stream >> dec >> numClassInFile;
		pair<int,int> linkPair;
		int link1, link2;

		Classification tempClass;
		for(int c = 0; c < numClassInFile; c++ ) {

			stream >> link1 >> link2 >> tempClass.mClassification >> tempClass.mFullNodeCount;

			if ( tempClass.mClassification == Classifier::NLOS1 || tempClass.mClassification == Classifier::NLOS2 ) {
				stream >> tempClass.mMainStreetLaneCount;
				stream >> tempClass.mSideStreetLaneCount;
				if ( tempClass.mClassification == Classifier::NLOS2 )
					stream >> tempClass.mParaStreetLaneCount;
			}

			if ( tempClass.mClassification != Classifier::LOS ) {

				for ( int n = 0; n < tempClass.mClassification; n++ ) {

					stream >> tempClass.mNodeSet[ n ];

				}

			}

			linkPair.first = link1;
			linkPair.second = link2;
			mClassificationMap[ linkPair ] = tempClass;

		}

		stream.close();

	}

	// read the building files
	if ( buildingFile ) {

		stream.open( buildingFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open building file: %s", buildingFile );
		}

		stream >> dec >> numBuildingsInFile;
		int vertexCount, tmp;
		Vector2D v1, v2;

		Building tempBuilding;
		for(int c = 0; c < numBuildingsInFile; c++ ) {

			tempBuilding.mId = c;
			stream >> tmp >> tempBuilding.mPermitivity >> tempBuilding.mMaxHeight >> tempBuilding.mHeightStdDev >> vertexCount >> v1.x >> v1.y;
			for ( int v = 0; v < vertexCount-1; v++ ) {

				if ( topLeft.x > tempNode.position.x )
					topLeft.x = tempNode.position.x;
				if ( bottomRight.x < tempNode.position.x )
					bottomRight.x = tempNode.position.x;
				if ( topLeft.y > tempNode.position.y )
					topLeft.y = tempNode.position.y;
				if ( bottomRight.y < tempNode.position.y )
					bottomRight.y = tempNode.position.y;

				stream >> v2.x >> v2.y;
				tempBuilding.mEdgeSet.push_back( LineSegment( v1, v2 ) );
				v1 = v2;

			}

			mBuildingSet.push_back( tempBuilding );
			tempBuilding.mEdgeSet.clear();

		}

		stream.close();

	}

	// read the link mappings
	if ( linkMapFile ) {

		stream.open( linkMapFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open link mapping file: %s", linkMapFile );
		}

		stream >> dec >> numLinkMappings;
		for ( int c = 0; c < numLinkMappings; c++ ) {

			std::string strTmp;
			int index;
			stream >> strTmp >> index;
			mLinkIndexMap[ strTmp ] = index;

		}

		stream.close();

	}

	// read the pre-computed K-factors
	if ( riceDataFile ) {

		stream.open( riceDataFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open Rice datafile: %s", riceDataFile );
		}

		stream >> dec >> numRice;
		for ( int c = 0; c < numRice; c++ ) {

			LinkPair p;
			int nPoints = 0;
			stream >> p.first >> p.second >> nPoints;
			mRiceFactorData[p] = RiceFactorData();
			for ( int n = 0; n < nPoints; n++ ) {

				RiceFactorEntry e;
				stream >> e.mSource.x >> e.mSource.y >> e.mDestination.x >> e.mDestination.y >> e.mKfactor;
				mRiceFactorData[p].push_back( e );

			}

		}

		stream.close();

	}

	mMapRect.location = topLeft;
	mMapRect.size = bottomRight - topLeft;

}





/*
 * Method: void ComputeSummedLinkSet();
 * Description: Takes the link set from the file, and calculates a reduced set
 * 				comprised of links that are the only connections between node pairs.
 */
void UraeData::ComputeSummedLinkSet() {

	// Note: Not sure if this is necessary now, since the Corner python class performs the link reduction.
	
	LinkSet::iterator linkIt;
	std::map< std::pair<int,int>, int > nodePairMapLinkIndex;
	std::pair<int,int> n1, n2;
	int lIndex = 0;
	Link newLink;

	for ( AllInVector( linkIt, mLinkSet ) ) {

		n1.first = n2.second = linkIt->nodeAindex;
		n2.first = n1.second = linkIt->nodeBindex;

		if ( nodePairMapLinkIndex.find( n1 ) != nodePairMapLinkIndex.end() ) {
			mSummedLinkSet[ nodePairMapLinkIndex[ n1 ] ].NumberOfLanes += linkIt->NumberOfLanes;
		} else if ( nodePairMapLinkIndex.find( n2 ) != nodePairMapLinkIndex.end() ) {
			mSummedLinkSet[ nodePairMapLinkIndex[ n2 ] ].NumberOfLanes += linkIt->NumberOfLanes;
		} else {
			newLink.index = lIndex;
			newLink.NumberOfLanes = linkIt->NumberOfLanes;
			newLink.nodeAindex = linkIt->nodeAindex;
			newLink.nodeBindex = linkIt->nodeBindex;
			mSummedLinkSet.push_back( newLink );
			mNodeSet[ n1.first  ].mConnectedLinks.push_back( lIndex );
			mNodeSet[ n1.second ].mConnectedLinks.push_back( lIndex );
			nodePairMapLinkIndex[ n1 ] = lIndex;
			lIndex++;
		}

	}

	nodePairMapLinkIndex.clear();

}



/*
 * Method: void ComputeBuckets();
 * Description: Fills the buckets with indices of building edges.
 */
void UraeData::ComputeBuckets() {

	unsigned int i, j;

	if ( mMapRect.size.x > mBucketSize * SINCOS45 )
		mBucketX = ceil( mMapRect.size.x / mBucketSize - SINCOS45 );
	else
		mBucketX = 1;

	if ( mMapRect.size.y > mBucketSize * SINCOS45 )
		mBucketY = ceil( mMapRect.size.y / mBucketSize - SINCOS45 );
	else
		mBucketY = 1;

	// compute buckets
	m_ppBuckets = new Bucket*[mBucketX];
	for ( i = 0; i < mBucketX; i++ )
		m_ppBuckets[i] = new Bucket[mBucketY];

	mCentroid = ( mMapRect.size - Vector2D( mBucketX-1, mBucketY-1 ) * mBucketSize ) * 0.5;

	for ( i = 0; i < mBucketX; i++ ) {
		for ( j = 0; j < mBucketY; j++ ) {
			Vector2D c = mCentroid + Vector2D( i, j ) * mBucketSize;
			for ( BuildingSet::iterator it = mBuildingSet.begin(); it != mBuildingSet.end(); it++ ) {
				for ( LineSet::iterator edgeIt = it->mEdgeSet.begin(); edgeIt != it->mEdgeSet.end(); edgeIt++ ) {
					if ( edgeIt->IntersectCircle( c, mBucketSize ) ) {
						m_ppBuckets[i][j].push_back( it->mId );
						break;
					}
				}
			}
		}
	}

	Real temp = mMapRect.size.y / mGridSize;
	if (temp <= 0) {
		mGridRowCount = 1;
	} else {
		mGridRowCount = (unsigned int)ceil(temp);
	}
	
	temp = mMapRect.size.x / mGridSize;
	if (temp <= 0) {
		mGridColumnCount = 1;
	} else {
		mGridColumnCount = (unsigned int)ceil(temp);
	}
	
	mGridList = new Grid*[mGridRowCount];
	if (!mGridList) {
		THROW_EXCEPTION ("Could not allocate memory for microBuckets!");
	}
	for (i=0; i<mGridRowCount; i++) {
		mGridList[i] = new Grid[mGridColumnCount];
		for (j=0; j<mGridColumnCount; j++) {
			mGridList[i][j].gridRect = Rect(j*mGridSize, i*mGridSize, mGridSize, mGridSize);
		}
	}
	
	LinkSet::iterator linkIt;
	VectorMath::Rect largerRect;
	for(i=0; i<mGridRowCount; i++) {
		for (j=0; j<mGridColumnCount; j++) {
			for (AllInVector(linkIt, mSummedLinkSet)) {
				//if the link lies within the grid rectangle, add it to the grid
				if (mGridList[i][j].gridRect.LineSegmentWithin(LineSegment(mNodeSet[linkIt->nodeAindex].position, mNodeSet[linkIt->nodeBindex].position))) {
					mGridList[i][j].linkList.push_back(linkIt->index);
				}
			}
		}
	}
}



/*
 * Method: double GetVehicleClassHeight( std::string );
 * Description: Get the height of vehicles of the given class.
 */
double UraeData::GetVehicleClassHeight( std::string strName ) {

	// TODO: GET HEIGHTS FOR THE VEHICLE CLASSES
	return 2;

}




void UraeData::CollectBucketsInRange( VectorMath::Real r, VectorMath::Vector2D p, Bucket *pBucket ) {

	unsigned int i, j;
	for ( i = 0; i < mBucketX; i++ ) {
		for ( j = 0; j < mBucketY; j++ ) {
			Vector2D c = mCentroid + Vector2D( i, j ) * mBucketSize;
			if ( ( p - c ).Magnitude() < r )
				pBucket->insert( pBucket->end(), m_ppBuckets[i][j].begin(), m_ppBuckets[i][j].end() );
		}
	}

}





