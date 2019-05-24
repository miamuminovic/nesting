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

package nesting.simulations.simpleModelqbuqbv2;

import inet.examples.ospfv2.fulltest.C;
import nesting.node.ethernet.VlanEtherHostFullLoad;
import nesting.node.ethernet.VlanEtherHostQ;
import nesting.node.ethernet.VlanEtherHostSched;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;


//
// TODO auto-generated type
//
network SimpleNetwork
{
    submodules:
        BE: VlanEtherHostFullLoad {
            @display("p=56,255");
        }
        TT: VlanEtherHostSched {
            @display("p=56,168");
        }
        ET: VlanEtherHostSched {
            @display("p=56,83");
        }
        switch: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=199,167");
            gates:
                ethg[4];

        }
        Sink: VlanEtherHostQ {
            @display("p=342,168");
        }
    connections:
        ET.ethg <--> C <--> switch.ethg[0];
        TT.ethg <--> C <--> switch.ethg[1];
        BE.ethg <--> C <--> switch.ethg[2];
        switch.ethg[3] <--> C <--> Sink.ethg;
}
