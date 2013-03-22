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

typedef std::pair<int,int> LinkPair;
typedef std::pair<Vector2D,Vector2D> SrcDestPair;

struct RiceFactorEntry {

	SrcDestPair mSrcDestPair;
	Real mKfactor;

};

typedef std::vector<RiceFactorEntry> RiceFactorData;
typedef std::map<LinkPair,RiceFactorData> RiceFactorMap;

void ParseArgs( int argc, char *pArgv[] ) {

	bool haveBasename = false;
	string basename;
	int raycount = 256;
	Real increment = 10;
	int cores = 2;
	Real rxGain = 1;
	int areaCount = 1;
	string configFilename("config");

	for ( int a = 2; a < argc; a++ ) {

		char arg = pArgv[a][1];
		a++;

		if ( a == argc ) {
			cout << "Unexpected end of input.\n";
			return;
		}

		switch( arg ) {

			case 'b':
				basename = pArgv[a];
				haveBasename = true;
				break;

			case 'r':
				raycount = atoi(pArgv[a]);
				break;

			case 'i':
				increment = atof(pArgv[a]);
				break;

			case 'c':
				cores = atoi(pArgv[a]);
				break;

			case 'G':
				rxGain = atof(pArgv[a]);
				break;

			case 'N':
				areaCount = atoi(pArgv[a]);
				break;

			case 'F':
				configFilename = pArgv[a];
				break;

			default:
				cout << "Unknown argument: -" << arg << "\n";
				return;

		};

	}

	if ( !haveBasename ) {
		cout << "Require a basename for the corner files!\n";
		return;
	}

	ofstream cfg;
	cfg.open( configFilename.c_str() );

	cfg << "basename " << basename << "\n";
	cfg << "raycount " << raycount << "\n";
	cfg << "increment " << increment << "\n";
	cfg << "cores " << cores << "\n";
	cfg << "rxGain " << rxGain << "\n";

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

	for ( int run = 0; run < areaCount; run++ ) {

		int x = run % gridX;
		int y = run / gridX;

		Vector2D p( s.x*x, s.y*y );

		cfg << "run " << run << "\n";
		cfg << "area " << p.x << "," << p.y << "," << s.x << "," << s.y << "\n";

	}

	cfg.close();
	cout << "Written configuration to " << configFilename << "\n";

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
	while ( !configInput.eof() ) {

		configInput >> varName >> varValue;
		if ( varName == "run" ) {
			runConfigs.push_back( globalConfigs );
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
		ForPointsOnLine( srcPos, srcPath, increment, _l ) {

			if ( bSmallArea && !area.PointWithin( srcPos ) )
				continue;

			Raytracer *rt = new Raytracer( srcPos, raycount, cores );
			rt->Execute();

			// now cycle through the maps a second time
			for ( int i = 0; i < linkCount; i++ ) {

				UraeData::Link *pLinkDest = pUrae->GetSummedLink( i );

				UraeData::Classification cls = pUrae->GetClassification( linkIndex, i );
				if ( cls.mClassification != Classifier::LOS )
					continue;

				RiceFactorData kData;
				
				// this link is LOS
				LineSegment destPath( pUrae->GetNode( pLinkDest->nodeAindex )->position, pUrae->GetNode( pLinkDest->nodeBindex )->position );
				ForPointsOnLine( destPos, destPath, increment, _r ) {

					if ( (destPos-srcPos).MagnitudeSq() >= rangeSq )
						continue;

					RiceFactorEntry kEnt;
					kEnt.mKfactor = rt->ComputeK( destPos, rxGain );
					if ( kEnt.mKfactor == 0 )
						continue;
					kEnt.mSrcDestPair = SrcDestPair( srcPos, destPos );
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

			delete rt;

		}

	}

	log << "Complete.\n";
	delete pUrae;

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

			outputFile << dataIt->mSrcDestPair.first << " "  << dataIt->mSrcDestPair.second << " ";
			if ( dataIt->mKfactor == DBL_MAX )
				outputFile << "inf\n";
			else
				outputFile << dataIt->mKfactor << "\n";

		}

	}

	outputFile.close();

	return 0;

}







