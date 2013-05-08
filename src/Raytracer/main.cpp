/*
 *  main.cpp - Raytracer-based K Factor calculation
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
#include <cfloat>

#include "Urae.h"
#include "Raytracer.h"

using namespace std;
using namespace Urae;
using namespace VectorMath;


#ifdef USE_VISUALISER
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

ALLEGRO_DISPLAY *gDisplay;
ALLEGRO_BITMAP *gMapBuffer;
Rect gArea, gMapDisplay;
Vector2D gScale;

/** Initialise the visualiser. */
void InitVisualiser() {

	al_init();
	al_install_keyboard();
	al_init_primitives_addon();
	gDisplay = al_create_display( 640, 480 );

}

/** Draw the map inside the given rect. This goes onto a bitmap that is drawn in StartPass. */
void PrepareMap( Rect& area ) {

	gMapBuffer = al_create_bitmap( area.size.x, area.size.y );
	if ( !gMapBuffer )
		throw Exception( "Could not create a bitmap of size %dx%d!", area.size.x, area.size.y );

	al_set_target_bitmap( gMapBuffer );
	al_clear_to_color( al_map_rgb(255,255,255) );

	UraeData *pUrae = UraeData::GetSingleton();
	int linkCount = pUrae->GetSummedLinkCount();
	for ( int link = 0; link < linkCount; link++ ) {

		UraeData::Link *pLink = pUrae->GetSummedLink( link );
		UraeData::Node *pNode1, *pNode2;
		pNode1 = pUrae->GetNode( pLink->nodeAindex );
		pNode2 = pUrae->GetNode( pLink->nodeBindex );

		bool bIn1 = area.PointWithin( pNode1->position );
		bool bIn2 = area.PointWithin( pNode2->position );
		if ( !bIn1 && !bIn2 )
			continue;

		Vector2D start = pNode1->position - area.location;
		Vector2D   end = pNode2->position - area.location;
		al_draw_line( start.x, area.size.y-start.y, end.x, area.size.y-end.y, al_map_rgb(0,0,0), 5 );

	}

	al_set_target_backbuffer( gDisplay );

	gArea = area;
	int w = al_get_bitmap_width(gMapBuffer);
	int h = al_get_bitmap_height(gMapBuffer);
	int newWidth = w;
	int newHeight = h;
	int x, y;
	if ( w < h ) {
		newHeight = 480;
		newWidth = w * (Real)newHeight / (Real)h;
		x = 320 - newWidth/2;
		y = 0;
	} else {
		newWidth = 640;
		newHeight = h * (Real)newWidth / (Real)w;
		x = 0;
		y = 240 - newHeight/2;
	}
	gMapDisplay = Rect( x, y, newWidth, newHeight );
	gScale = Vector2D( newWidth / (Real)w, newHeight / (Real)h );

}

/** Draw the map bitmap. */
void StartPass() {

	al_clear_to_color( al_map_rgb(0,0,0) );

	int w = al_get_bitmap_width(gMapBuffer);
	int h = al_get_bitmap_height(gMapBuffer);

	al_draw_scaled_bitmap( gMapBuffer, 0, 0, w, h, gMapDisplay.location.x, gMapDisplay.location.y, gMapDisplay.size.x, gMapDisplay.size.y, 0 );

}

/** Draw a circular marker with the given colour at the given position. */
void DrawMarker( Vector2D pos, Vector3D col ) {

	al_draw_filled_circle( gMapDisplay.location.x + gScale.x * (pos.x-gArea.location.x), gMapDisplay.location.y + gScale.y * (gArea.size.y-pos.y+gArea.location.y), 5, al_map_rgb_f(col.x,col.y,col.z) );

}

/** Flip the backbuffer. */
void Present() {

	al_flip_display();

}


/** Shutdown the visualiser. */
void Shutdown() {

	al_destroy_bitmap( gMapBuffer );
	al_destroy_display( gDisplay );

}


#endif // #ifdef USE_VISUALISER


typedef std::pair<int,int> LinkPair;
typedef std::pair<Vector2D,Vector2D> SrcDestPair;

struct RiceFactorEntry {

	SrcDestPair mSrcDestPair;
	Real mKfactor;

};

typedef std::vector<RiceFactorEntry> RiceFactorData;
typedef std::map<LinkPair,RiceFactorData> RiceFactorMap;


struct RsuDef {
	std::string mName;
	std::string mRoadId;
	Vector2D mPosition;
};


void ParseArgs( int argc, char *pArgv[] ) {

	bool haveBasename = false;
	string basename;
	int raycount = 256;
	Real increment = 10;
	int cores = 2;
	Real rxGain = 1;
	int areaCount = 1;
	string configFilename("config");
	string rsuDefFile("none");
#ifdef USE_VISUALISER
	bool useVisualiser = false;
#endif // #ifdef USE_VISUALISER

	for ( int a = 2; a < argc; a++ ) {

		char arg = pArgv[a][1];

		switch( arg ) {

			case 'b':
				a++;
				basename = pArgv[a];
				haveBasename = true;
				break;

			case 'r':
				a++;
				raycount = atoi(pArgv[a]);
				break;

			case 'i':
				a++;
				increment = atof(pArgv[a]);
				break;

			case 'c':
				a++;
				cores = atoi(pArgv[a]);
				break;

			case 'G':
				a++;
				rxGain = atof(pArgv[a]);
				break;

			case 'N':
				a++;
				areaCount = atoi(pArgv[a]);
				break;

			case 'F':
				a++;
				configFilename = pArgv[a];
				break;

			case 'R':
				a++;
				rsuDefFile = pArgv[a];
				break;

#ifdef USE_VISUALISER
			case 'V':
				useVisualiser = true;
				break;
#endif // #ifdef USE_VISUALISER

			default:
				cout << "Unknown argument at position " << a << ": -" << arg << "\n";
				return;

		};

	}

	if ( !haveBasename ) {
		cout << "Require a basename for the corner files!\n";
		return;
	}

	ofstream cfg;
	cfg.open( configFilename.c_str() );

	cfg.precision(12);

	cfg << "basename " << basename << "\n";
	cfg << "raycount " << raycount << "\n";
	cfg << "increment " << increment << "\n";
	cfg << "cores " << cores << "\n";
	cfg << "rxGain " << rxGain << "\n";
#ifdef USE_VISUALISER
	cfg << "useVisualiser " << ( useVisualiser ? "true" : "false" ) << "\n";
#endif // #ifdef USE_VISUALISER

	UraeData *pUrae = new UraeData(
		(basename+".corner.lnk").c_str(),
		(basename+".corner.int").c_str(),
		(basename+".corner.cls").c_str(),
		(basename+".corner.bld").c_str(),
		(basename+".corner.lnm").c_str(),
		15, 0.124378109, 400, 1142.9, pow(10,-11), 0.25, 200
	);

	Rect mapRect = pUrae->GetMapRect();

	delete pUrae;

	int gridX = (int)sqrt( mapRect.size.x * areaCount / mapRect.size.y );
	int gridY = (int)sqrt( mapRect.size.y * areaCount / mapRect.size.x );

	if ( gridX == 0 )
		gridX = 1;

	Vector2D s( mapRect.size.x / gridX, mapRect.size.y / gridY );

	// load RSU file
	std::vector<RsuDef> rsuSet;
	if ( rsuDefFile != "none" ) {

		ifstream rsuIn;
		rsuIn.open( rsuDefFile.c_str() );
		int count;
		rsuIn >> count;
		for ( int i = 0; i < count; i++ ) {
			RsuDef r;
			Real h;
			rsuIn >> r.mName >> r.mPosition.x >> r.mPosition.y >> r.mRoadId >> h;
			rsuSet.push_back( r );
		}
		rsuIn.close();

	}

	for ( int run = 0; run < areaCount; run++ ) {

		int x = run % gridX;
		int y = run / gridX;

		Vector2D p( s.x*x, s.y*y );

		cfg << "run " << run << "\n";
		cfg << "area " << p.x << "," << p.y << "," << s.x << "," << s.y << "\n";
		std::vector<RsuDef>::iterator rsuIt;
		for ( AllInVector( rsuIt, rsuSet ) ) {
			if ( Rect( p, s ).PointWithin( rsuIt->mPosition ) ) {
				cfg << "rsu " << rsuIt->mName << "," << rsuIt->mPosition.x << "," << rsuIt->mPosition.y << "," << rsuIt->mRoadId << "\n";
			}
		}

	}

	cfg.close();
	cout << "Written configuration to " << configFilename << "\n";

}



RsuDef ParseRsuDef( string strRsuDef ) {

	RsuDef r;
	int p = -1, plast;

	plast = p+1;
	p = strRsuDef.find( ",", plast );
	r.mName = strRsuDef.substr( plast, p );

	plast = p+1;
	p = strRsuDef.find( ",", plast );
	r.mPosition.x = atof( strRsuDef.substr( plast, p ).c_str() );

	plast = p+1;
	p = strRsuDef.find( ",", plast );
	r.mPosition.y = atof( strRsuDef.substr( plast, p ).c_str() );

	plast = p+1;
	p = strRsuDef.find( ",", plast );
	r.mRoadId = strRsuDef.substr( plast, p );

	return r;

}



Rect ParseRect( string strRect ) {

	Rect r;
	int p = -1, plast;

	plast = p+1;
	p = strRect.find( ",", plast );
	r.location.x = atof( strRect.substr( plast, p ).c_str() );

	plast = p+1;
	p = strRect.find( ",", plast );
	r.location.y = atof( strRect.substr( plast, p ).c_str() );

	plast = p+1;
	p = strRect.find( ",", plast );
	r.size.x = atof( strRect.substr( plast, p ).c_str() );

	plast = p+1;
	p = strRect.find( ",", plast );
	r.size.y = atof( strRect.substr( plast, p ).c_str() );

	return r;

}

#define ISEVEN(x) (x%2)==0

int main( int argc, char *pArgv[] ) {

	if ( argc == 1 ) {
		cout << "Require a configuration file and a run number.\nExecute './raytracer -g' to generate config files.\n";
		return -1;
	} 
	
	if ( pArgv[1][0] == '-' && pArgv[1][1] == 'g' ) {

		ParseArgs( argc, pArgv );
		return 0;

	}


	ofstream log;
	char strLog[200];
	sprintf( strLog, "logs/%s-%s.log", pArgv[1], pArgv[2] );
	log.open( strLog );

	log << "Started raytracer with configuration file '" << pArgv[1] << "' with run number " << pArgv[2] << "\n";


	// read configuration
	ifstream configInput;
	configInput.open( pArgv[1] );
	if ( configInput.fail() ) {
		log << "Couldn't load file '" << pArgv[1] << "'\n";
		return -1;
	}

	string varName;
	string varValue;
	map<string,string> globalConfigs;
	vector< map<string,string> > runConfigs;
	vector< vector< RsuDef > > rsuDefinitions;
	while ( !configInput.eof() ) {

		configInput >> varName >> varValue;
		if ( varName == "run" ) {
			runConfigs.push_back( globalConfigs );
			rsuDefinitions.push_back( vector< RsuDef >() );
		} else if ( varName == "rsu" ) {
			rsuDefinitions.back().push_back( ParseRsuDef( varValue ) );
		} else {
			if ( runConfigs.empty() )
				globalConfigs[varName] = varValue;
			else
				runConfigs.back()[varName] = varValue;
		}

	}

	configInput.close();

	int runNumber = atoi( pArgv[2] );

	string basename = runConfigs[runNumber]["basename"];
	int raycount = atoi( runConfigs[runNumber]["raycount"].c_str() );
	Real range = atof( runConfigs[runNumber]["raycount"].c_str() );
	Real rangeSq = range*range;
	Real increment = atof( runConfigs[runNumber]["increment"].c_str() );
	int cores = atoi( runConfigs[runNumber]["cores"].c_str() );
	Real rxGain = atof( runConfigs[runNumber]["rxGain"].c_str() );
	Rect area = ParseRect( runConfigs[runNumber]["area"] );
	Real laneWidth = atof( runConfigs[runNumber]["laneWidth"].c_str() );
#ifdef USE_VISUALISER
	bool useVisualiser = ( runConfigs[runNumber]["useVisualiser"] == "true" );
	if ( useVisualiser )
		InitVisualiser();
#endif // #ifdef USE_VISUALISER

	runConfigs.clear();
	globalConfigs.clear();

	log << "Initialising Urae...\n";

	UraeData *pUrae;

	try
	{
		pUrae = new UraeData(
			(basename+".corner.lnk").c_str(),
			(basename+".corner.int").c_str(),
			(basename+".corner.cls").c_str(),
			(basename+".corner.bld").c_str(),
			(basename+".corner.lnm").c_str(),
			15, 0.124378109, 400, 1142.9, pow(10,-11), 0.25, 200
		);

	} catch( Exception &e ) {

		log << "Could not initialise URAE. " << e.What() << "\n";
		return -1;

	}

#ifdef USE_VISUALISER
	if ( useVisualiser )
		PrepareMap(area);
#endif // #ifdef USE_VISUALISER


	bool bSmallArea = ( area.size.x != 0 );

	RiceFactorMap riceData;

	int linkCount = pUrae->GetSummedLinkCount();
	log << "Processing " << basename << " with " << linkCount << " links.\n";
	for ( int linkIndex = 0; linkIndex < linkCount; linkIndex++ ) {

		UraeData::Link *pLink = pUrae->GetSummedLink( linkIndex );
		UraeData::Node *pNode1, *pNode2;
		pNode1 = pUrae->GetNode( pLink->nodeAindex );
		pNode2 = pUrae->GetNode( pLink->nodeBindex );

		bool bIn1 = area.PointWithin( pNode1->position );
		bool bIn2 = area.PointWithin( pNode2->position );

		if ( bSmallArea && !bIn1 && !bIn2 )
			continue;

		LineSegment srcPath( pNode1->position, pNode2->position );
		for ( Real srcT = 0; srcT <= 1; srcT += increment/srcPath.GetDistance() ) {

			Vector2D srcLinkPos = srcPath.mStart + srcPath.GetVector() * srcT;
			Vector2D srcLinkNorm( -srcLinkPos.y, srcLinkPos.x );
			for ( int srcLane = 0; srcLane < pLink->NumberOfLanes; srcLane++ ) {

				Vector2D srcPos;
				if ( ISEVEN( pLink->NumberOfLanes ) )
					srcPos = srcLinkPos + srcLinkNorm * ( srcLane - pLink->NumberOfLanes / 2 ) * laneWidth / 2;
				else
					srcPos = srcLinkPos + srcLinkNorm * ( srcLane - ( pLink->NumberOfLanes - 1 ) / 2 ) * laneWidth;

				if ( bSmallArea && !area.PointWithin( srcPos ) )
					continue;

				Raytracer *rt = new Raytracer( srcPos, raycount, cores );
				rt->Execute();

				// now cycle through the maps a second time
				for ( int i = 0; i < linkCount; i++ ) {

					UraeData::Classification cls = pUrae->GetClassification( linkIndex, i );
					if ( cls.mClassification != Classifier::LOS )
						continue;

					RiceFactorData kData;
					UraeData::Link *pLinkDest = pUrae->GetSummedLink( i );
					std::cout << "Source Link: " << linkIndex << " Destination Link: " << i << std::endl;

					// this link is LOS
					LineSegment destPath( pUrae->GetNode( pLinkDest->nodeAindex )->position, pUrae->GetNode( pLinkDest->nodeBindex )->position );
					for ( Real destT = 0; destT <= 1; destT += increment/destPath.GetDistance() ) {

						Vector2D destLinkPos = destPath.mStart + destPath.GetVector() * destT;
						Vector2D destLinkNorm( -destLinkPos.y, destLinkPos.x );
						for ( int destLane = 0; destLane < pLinkDest->NumberOfLanes; destLane++ ) {

							Vector2D destPos;
							if ( ISEVEN( pLinkDest->NumberOfLanes ) )
								destPos = destLinkPos + destLinkNorm * ( destLane - pLinkDest->NumberOfLanes / 2 ) * laneWidth / 2;
							else
								destPos = destLinkPos + destLinkNorm * ( destLane - ( pLinkDest->NumberOfLanes - 1 ) / 2 ) * laneWidth;

							if ( (destPos-srcPos).MagnitudeSq() >= rangeSq )
								continue;

#ifdef USE_VISUALISER
							if ( useVisualiser ) {
								StartPass();
								DrawMarker(  srcPos, Vector3D(1,0,0) );
								DrawMarker( destPos, Vector3D(0,1,0) );
								Present();
							}
#endif // #ifdef USE_VISUALISER

							RiceFactorEntry kEnt;
							kEnt.mKfactor = rt->ComputeK( destPos, rxGain ).mFactorK;
							kEnt.mSrcDestPair = SrcDestPair( srcPos, destPos );
							kData.push_back( kEnt );

						}

					}

					if ( kData.empty() )
						continue;
					
					LinkPair linkPair( linkIndex, i );
					if ( riceData.find( linkPair ) == riceData.end() )
						riceData[linkPair] = kData;
					else 
						riceData[linkPair].insert( riceData[linkPair].begin(), kData.begin(), kData.end() );

				}

				vector< vector< RsuDef > >::iterator rsuDefSetIt;
				vector< RsuDef >::iterator rsuDefIt;
				for ( AllInVector( rsuDefSetIt, rsuDefinitions ) ) {
					
					for ( AllInVector( rsuDefIt, (*rsuDefSetIt) ) ) {

						if ( (rsuDefIt->mPosition-srcPos).MagnitudeSq() >= rangeSq )
							continue;

						int rsuLinkIndex;
						if ( !UraeData::GetSingleton()->LinkHasMapping( rsuDefIt->mRoadId, &rsuLinkIndex ) ) {
							log << "ERROR: RSU '" << rsuDefIt->mName << "' located on link '" << rsuDefIt->mRoadId << "' has no mapping to a road index! Skipping.\n";
							continue;
						}

						UraeData::Classification cls = pUrae->GetClassification( linkIndex, rsuLinkIndex );
						if ( cls.mClassification != Classifier::LOS )
							continue;

#ifdef USE_VISUALISER
						if ( useVisualiser ) {
							StartPass();
							DrawMarker(              srcPos, Vector3D(1,0,0) );
							DrawMarker( rsuDefIt->mPosition, Vector3D(0,1,0) );
							Present();
						}
#endif // #ifdef USE_VISUALISER

						LinkPair linkPair( linkIndex, rsuLinkIndex );

						RiceFactorEntry kEnt;
						kEnt.mKfactor = rt->ComputeK( rsuDefIt->mPosition, rxGain ).mFactorK;
						kEnt.mSrcDestPair = SrcDestPair( srcPos, rsuDefIt->mPosition );

						riceData[linkPair].push_back( kEnt );

						
					}
					
				}

				delete rt;

			}

		}

	}

	log << "Road calculations complete.\n";

	if ( !rsuDefinitions[runNumber].empty() ) {

		log << "Calculating RSUs...\n";
		int processed = 0;
		int skipped = 0;
		vector<RsuDef>::iterator rsuIt;
		for ( AllInVector( rsuIt, rsuDefinitions[runNumber] ) ) {

			int linkIndex;
			if ( !UraeData::GetSingleton()->LinkHasMapping( rsuIt->mRoadId, &linkIndex ) ) {
				log << "ERROR: RSU '" << rsuIt->mName << "' located on link '" << rsuIt->mRoadId << "' has no mapping to a road index! Skipping.\n";
				skipped++;
				continue;
			}

			Raytracer *rt = new Raytracer( rsuIt->mPosition, raycount, cores );
			rt->Execute();

			// now go through all the links in the network.
			for ( int i = 0; i < linkCount; i++ ) {

				UraeData::Classification cls = pUrae->GetClassification( linkIndex, i );
				if ( cls.mClassification != Classifier::LOS )
					continue;

				UraeData::Link *pLinkDest = pUrae->GetSummedLink( i );

				RiceFactorData kData;
				
				// this link is LOS
				LineSegment destPath( pUrae->GetNode( pLinkDest->nodeAindex )->position, pUrae->GetNode( pLinkDest->nodeBindex )->position );
				ForPointsOnLine( destPos, destPath, increment, _r ) {

					if ( (destPos-rsuIt->mPosition).MagnitudeSq() >= rangeSq )
						continue;

#ifdef USE_VISUALISER
					if ( useVisualiser ) {
						StartPass();
						DrawMarker( rsuIt->mPosition, Vector3D(1,0,0) );
						DrawMarker(          destPos, Vector3D(0,1,0) );
						Present();
					}
#endif // #ifdef USE_VISUALISER

					RiceFactorEntry kEnt;
					kEnt.mKfactor = rt->ComputeK( destPos, rxGain ).mFactorK;
					kEnt.mSrcDestPair = SrcDestPair( rsuIt->mPosition, destPos );
					kData.push_back( kEnt );

				}

				if ( kData.empty() )
					continue;
				
				LinkPair linkPair( linkIndex, i );
				if ( riceData.find( linkPair ) == riceData.end() )
					riceData[linkPair] = kData;
				else 
					riceData[linkPair].insert( riceData[linkPair].begin(), kData.begin(), kData.end() );

			}

			processed++;
			delete rt;

		}

		log << "Complete. " << processed << " RSUs processed, " << skipped << " skipped.\n";
	}

	log << "Complete.\n";
	delete pUrae;

#ifdef USE_VISUALISER
	Shutdown();
#endif // #ifdef USE_VISUALISER

	// Now save to a file
	ofstream outputFile;
	char strF[200];
	sprintf( strF, "%s-%d.urae.k", basename.c_str(), runNumber );
	outputFile.open( strF );
	outputFile << riceData.size() << "\n";

	RiceFactorMap::iterator mapIt;
	for ( AllInVector( mapIt, riceData ) ) {

		outputFile << mapIt->first.first << " " << mapIt->first.second << " " << mapIt->second.size() << "\n";

		RiceFactorData::iterator dataIt;
		for ( AllInVector( dataIt, mapIt->second ) ) {

			outputFile << dataIt->mSrcDestPair.first.x << " " << dataIt->mSrcDestPair.first.y << " " << dataIt->mSrcDestPair.second.x << " " << dataIt->mSrcDestPair.second.y << " ";
			if ( dataIt->mKfactor == DBL_MAX )
				outputFile << "inf\n";
			else
				outputFile << dataIt->mKfactor << "\n";

		}

	}

	outputFile.close();

	return 0;

}







