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

package raytracerexperiment.CORNER;
import org.mixim.modules.phy.PhyLayer;

//
// TODO auto-generated module
//
simple CORNERPhyLayer extends PhyLayer
{
    parameters:
        @class("CORNERPhyLayer");
        bool useRaytracing = default(false);	// If false, use constant K factor
        double constantK = default(0);
        bool useVehicles = default(false);
        int rayCount = default(128);
        double gain = default(1);
        int threadCount = default(2);
		double carLPF = default(5);
		double raytraceDistance @unit(m) = default(1m);
		bool recordData = default(false);
//		double raytraceLifetime @unit(s) = default(1s);
}
