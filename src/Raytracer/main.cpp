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

#include "Urae.h"
#include "Raytracer.h"


using namespace std;
using namespace Urae;
using namespace VectorMath;


int main( int argc, char *pArgv[] ) {

	if ( argc != 6 ) {
		cout << "Require a base filename, a ray count, transmission range, increment, and core count in that order.\n";
		return -1;
	}

	string basename( pArgv[1] );
	int raycount = atoi( pArgv[2] );
	Real range = atof( pArgv[3] );
	Real increment = atof( pArgv[4] );
	int cores = atoi( pArgv[5] );

	UraeData *pUrae = new UraeData(
		(basename+".corner.lnk").c_str(),
		(basename+".corner.int").c_str(),
		(basename+".corner.cls").c_str(),
		(basename+".corner.bld").c_str(),
		(basename+".corner.lnm").c_str(),
		0, 4*M_PI*range, 1, 1, 1, 1, 200
	);

	int linkCount = pUrae->GetSummedLinkCount();
	for ( int linkIndex = 0; linkIndex < linkCount; linkIndex++ ) {

		UraeData::Link *pLink = pUrae->GetSummedLink( linkIndex );
		UraeData::Node *pNode1, *pNode2;
		pNode1 = pUrae->GetNode( pLink->nodeAindex );
		pNode2 = pUrae->GetNode( pLink->nodeBindex );

		LineSegment path( pNode1->position, pNode2->position );
		Real t = 0, tIncr = increment / path.GetDistance();
		for ( ; t <= 1; t += tIncr ) {
			Vector2D pos = path.mStart + ( path.mEnd - path.mStart ) * t;
			Raytracer *rt = new Raytracer( pos, raycount, cores );
			rt->Execute();
			
		}

	}

	return 0;

}







