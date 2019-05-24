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

package nesting.simulations.simpleModelqbu;

import inet.examples.ospfv2.fulltest.C;
import nesting.node.ethernet.VlanEtherHostFullLoad;
import nesting.node.ethernet.VlanEtherHostQ;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;


//
// TODO auto-generated type
//
network SimpleNetworkQbu
{
    submodules:
        AV: VlanEtherHostFullLoad {
            @display("p=71,62");
        }
        BE: VlanEtherHostFullLoad {
            @display("p=71,147");
        }
        Sink: VlanEtherHostQ {
            @display("p=306,101");
        }
        switch: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=185,100");
            gates:
                ethg[3];
        }
    connections:
        AV.ethg <--> C <--> switch.ethg[0];
        BE.ethg <--> C <--> switch.ethg[1];
        switch.ethg[2] <--> C <--> Sink.ethg;
}
