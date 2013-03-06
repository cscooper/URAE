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
#include "Singleton.h"
#include "VectorMath.h"
#include <list>
#include <map>

#pragma once

namespace Urae {

	/*
	 * Name: UraeData
	 * Inherits: Singleton
	 * Description: Container for data on road network. This includes:
	 * 					-> Nodes and links
	 * 					-> Building Edges
	 * 					-> Intersections
	 * 					-> Edge Index Buckets for easy look-up
	 * 					-> Classifications for CORNER
	 */
	class UraeData : public Singleton<UraeData> {
		
	public:

		/*
		 * Name: Link
		 * Description: Contains the data for one stretch of road
		 */
		struct Link {
			int index;			// Link index
			int nodeAindex;		// index of the start node
			int nodeBindex;		// index of the end node
			int NumberOfLanes;	// number of lanes on this road
			double flow;		// flow rate of traffic			(as yet unused)
			double speed;		// speed of cars on the road	(as yet unused)
		};

		typedef std::vector<int> LinkIndexSet;

		/*
		 * Name: Node
		 * Description: Contains the data for one intersection
		 */
		struct Node {
			int index;						// index of intersection
			VectorMath::Vector2D position;	// position of the intersection
			LinkIndexSet mConnectedLinks;	// set of links which connect to this node.
							// Note: This indexes the summed link set, NOT the other link set.
			VectorMath::Real mSize;			// size of the intersection (treated as a circle).
		};

		typedef std::vector< VectorMath::Vector2D > VectorSet;
		typedef std::vector< VectorMath::LineSegment > LineSet;
                
		struct Classification {
			int mClassification;					// the classification
			int mNodeSet[2];						// the set of junctions in this classification
			int mFullNodeCount;						// the full number of nodes that had to be traversed to get this classification
			VectorMath::Real mMainStreetLaneCount;	// number of lanes in the sidestreet for NLOS1/2 calculations.
			VectorMath::Real mSideStreetLaneCount;	// number of lanes in the sidestreet for NLOS1/2 calculations.
			VectorMath::Real mParaStreetLaneCount;	// number of lanes in the sidestreet for NLOS2 calculations.
		};

		struct Building {
			long mId;
			LineSet mEdgeSet;
			VectorMath::Real mPermitivity;
			VectorMath::Real mMaxHeight;
			VectorMath::Real mHeightStdDev;
		};

		// typedefs
		typedef std::vector<Link> LinkSet;
		typedef std::vector<Node> NodeSet;
		typedef std::map< std::pair<int,int>, Classification > ClassificationMap;
		typedef std::vector< Building > BuildingSet;
		typedef std::vector< long > Bucket;

		
		/*
		 * Name: Grid
		 * Description: Contains the rectangle representing the current grid and the list of links in the grid
		 *
		 */
		struct Grid {
			VectorMath::Rect gridRect;
			LinkIndexSet linkList;
		};

		// getters
		VectorMath::Real GetWavelength();
		VectorMath::Real GetLamdaBy4PiSq();
		VectorMath::Real GetTransmitPower();
		VectorMath::Real GetSystemLoss();
		VectorMath::Real GetReceiverSensitivity();
		VectorMath::Real GetFreeSpaceRange();
		VectorMath::Real GetLaneWidth();
		VectorMath::Real GetLossPerReflection();

		// blank constructor, giving empty UraeData
		UraeData( VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid );

		/*
		* Constructor Arguments:
		* 		1. linksFile - file name of the CORNER links file
		* 		2. nodesFile - file name of the CORNER nodes file
		* 		3. classFile - file name of the CORNER class file
		* 		4. buildingFile - file name of the CORNER building file
		* 		5. laneWidth - width of one lane in metres
		* 		6. lambda - wavelength of the carrier signal
		* 		7. txPower - transmission power of the signal
		* 		8. L - losses due to the system (signal processing, etc) not related to propagation
		* 		9. sensitivity - the sensitivity of the receiver
		* 		10. lpr - The loss per reflection
		*/
				
		UraeData( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile, VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid );

		~UraeData();

		/*
		 * Method: void AddBuilding( Building newBuilding );
		 * Description: Manually add a building to the network
		 */
		void AddBuilding( Building newBuilding );
		
		/*
		 * Method: void CalculateMapRectangle();
		 * Description: Calculate map bounds after manually adding buildings.
		 */
		void CalculateMapRectangle();
		
		/*
		 * Method: VectorMath::Rect GetMapRect();
		 * Description: Gets the bounds of the road network.
		 */
		VectorMath::Rect GetMapRect();

		/*
		 * Method: Link *GetLink( int index );
		 * Description: Gets a pointer to a link of the given index.
		 */
		Link *GetLink( int index );

		/*
		 * Method: Node *GetNode( int index );
		 * Description: Gets a pointer to a node of the given index.
		 */
		Node *GetNode( int index );

		/*
		 * Method: Link *GetSummedLink( int index );
		 * Description: Gets a pointer to a summed link of the given index.
		 */
		Link *GetSummedLink( int index );

		/*
		 * Method: int GetSummedLinkCount();
		 * Description: Return the number of summed links.
		 */
		int GetSummedLinkCount() { return mSummedLinkSet.size(); }

		/*
		 * Method: void GetGrid(Vector2D position) {
		 * Description: Gets the grid associated with the specified position
		 */
		Grid* GetGrid(VectorMath::Vector2D position);

		/*
		 * Method: Classification GetClassification( int l1, int l2 );
		 * Description: Get the CORNER classification between the given links.
		 */
		Classification GetClassification( int l1, int l2 );
		
		/*
		 * Method: Building *GetBuilding( int index );
		 * Description: Get the building at the given index.
		 */
		Building *GetBuilding( int index );
		
		/*
		 * Method: int GetBuildingCount();
		 * Description: Get the number of buildings.
		 */
		int GetBuildingCount() { return mBuildingSet.size(); }
		
		void CollectBucketsInRange( VectorMath::Real r, VectorMath::Vector2D p, Bucket* );

		/*
		 * Method: void LoadNetwork( char* linksFile, char* nodesFile, const char* classFile );
		 * Description: Loads the data from the links, nodes, and classification files.
		 */
		void LoadNetwork( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile );
		
		/*
		 * Method: void ComputeSummedLinkSet();
		 * Description: Takes the link set from the file, and calculates a reduced set
		 * 				comprised of links that are the only connections between node pairs.
		 */
		void ComputeSummedLinkSet();

		/*
		 * Method: void ComputeBuckets();
		 * Description: Fills the buckets with indices of building edges.
		 */
		void ComputeBuckets();


	protected:

		Bucket **m_ppBuckets;
		unsigned int mBucketX;
		unsigned int mBucketY;
		VectorMath::Vector2D mCentroid;
		VectorMath::Real mBucketSize;

		LinkSet mLinkSet;									// set of links loaded from file
		NodeSet mNodeSet;									// set of nodes loaded from file
		LinkSet mSummedLinkSet;								// link set calculated by summing lane counts of links sharing nodes

		VectorMath::Real mWavelength;						// wavelength of carrier signal
		VectorMath::Real mTransmitPower;					// transmission power
		VectorMath::Real mSystemLoss;						// sum of system losses (L)
		VectorMath::Real mSensitivity;						// receiver sensitivity

		VectorMath::Real mLossPerReflection;				// Loss of power per reflection

		VectorMath::Real mFreeSpaceRange;					// Free Space Transmission Range

		VectorMath::Vector2D mFirstCentroid;				// the centroid of the first bucket
		
		VectorMath::Real mLambdaBy4PiSq;					// pre-calculated lambda/4pi^2 since it is used a lot in classifer

		VectorMath::Real mGridSize;							//preferred size of the grid (assuming square)
		Grid **mGridList;									//list of grids in the map
		unsigned int mGridRowCount;
		unsigned int mGridColumnCount;

		VectorMath::Real mLaneWidth;						// Width of each lane
		VectorMath::Rect mMapRect;							// Rectangle showing bounds of the map network

		ClassificationMap mClassificationMap;				// classifications
		BuildingSet mBuildingSet;


	};


};
