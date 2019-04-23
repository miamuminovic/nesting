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

package nesting.simulations.test;

import ned.DatarateChannel;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;
import nesting.node.ethernet.VlanEtherHostSched;
import nesting.node.ethernet.VlanEtherHostFullLoad;
import nesting.node.ethernet.VlanEtherHostQ;


network TestNetwork
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 0.1Gbps;
        }
    submodules:
        workstation1: VlanEtherHostFullLoad {
            @display("p=79,62");
        }
        robotController: VlanEtherHostSched {
            @display("p=79,202");
        }
        sink: VlanEtherHostQ {
            @display("p=290,62");
        }
        sink1: VlanEtherHostQ {
            @display("p=290,202");
        }
        switchA: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=184,140");
            gates:
                ethg[4];
        }
    connections:
        workstation1.ethg <--> C <--> switchA.ethg[0];
        robotController.ethg <--> C <--> switchA.ethg[1];
        switchA.ethg[3] <--> C <--> sink1.ethg;
        switchA.ethg[2] <--> C <--> sink.ethg;
}
