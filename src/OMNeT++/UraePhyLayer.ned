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

package Urae.UraePhyLayer;
import org.mixim.modules.phy.PhyLayer;

//
// TODO auto-generated module
//
simple UraePhyLayer extends PhyLayer
{
    parameters:
        @class("UraePhyLayer");
		string linksFile = default("");
		string nodesFile = default("");
		string classFile = default("");
		string linkMapFile = default("");
		string riceFile = default("");
		string carDefFile = default("");
		double laneWidth @unit("m") = default(5m);
		double waveLength @unit("m") = default(0.125m);
		double txPower @unit("mW") = default(80mW);
		double systemLoss = default(1142.9);
		double sensitivity @unit("dBm") = default(-110dBm);
		double lossPerReflection = default(0.75);
		string componentFile = default("default.fading");
		int randSeed = default(1234);

}
