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
import nesting.node.ethernet.VlanEtherHostSched;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;


//
// TODO auto-generated type
//
network SimpleNetworkQbu
{
    @display("bgb=396,225");
    submodules:
        switch: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=197,109");
            gates:
                ethg[3];

        }
        BE: VlanEtherHostFullLoad {
            @display("p=94,156");
        }
        Sink: VlanEtherHostQ {
            @display("p=315,110");
        }
        AV: VlanEtherHostSched {
            @display("p=93,53");
        }
    connections:
        AV.ethg <--> C <--> switch.ethg[0];
        BE.ethg <--> C <--> switch.ethg[1];
        switch.ethg[2] <--> C <--> Sink.ethg;
}
