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

package nesting.simulations.examples;

import ned.DatarateChannel;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;
import nesting.node.ethernet.VlanEtherHostSched;
import nesting.node.ethernet.VlanEtherHostFullLoad;
import nesting.node.ethernet.VlanEtherHostQ;


network TestScenario
{
    @display("bgb=840,314");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 1Gbps;
        }
    submodules:
        switchA: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=188,127");
            gates:
                ethg[4];
        }
        switchB: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=643,127");
            gates:
                ethg[3];
        }
        Control3: VlanEtherHostSched {
            @display("p=100,217");
        }
        HeadUnit: VlanEtherHostQ {
            @display("p=604,29");
        }
        AV_sink: VlanEtherHostQ {
            @display("p=265,23");
        }
        Control4: VlanEtherHostSched {
            @display("p=188,217");
        }
        Camera3: VlanEtherHostSched {
            @display("p=45,128");
        }
        Video: VlanEtherHostSched {
            @display("p=188,23");
        }
        Audio: VlanEtherHostSched {
            @display("p=106,23");
        }
        Bulk: VlanEtherHostSched {
            @display("p=265,217");
        }
        PUCAM: VlanEtherHostQ {
            @display("p=682,29");
        }
        Camera1: VlanEtherHostSched {
            @display("p=501,217");
        }
        Camera2: VlanEtherHostSched {
            @display("p=572,217");
        }
        FCAM: VlanEtherHostSched {
            @display("p=643,217");
        }
        Control1: VlanEtherHostSched {
            @display("p=712,217");
        }
        Control2: VlanEtherHostSched {
            @display("p=784,217");
        }
    connections:
        Control3.ethg <--> C <--> switchA.ethg[0];
        HeadUnit.ethg <--> C <--> switchB.ethg[0];
        switchA.ethg[3] <--> C <--> switchB.ethg[2];
        AV_sink.ethg <--> C <--> switchA.ethg[0];
        Control4.ethg <--> C <--> switchA.ethg[0];
        Camera3.ethg <--> C <--> switchA.ethg[0];
        Video.ethg <--> C <--> switchA.ethg[0];
        Audio.ethg <--> C <--> switchA.ethg[0];
        Bulk.ethg <--> C <--> switchA.ethg[0];
        switchB.ethg[0] <--> C <--> PUCAM.ethg;
        Camera1.ethg <--> C <--> switchB.ethg[0];
        Camera2.ethg <--> C <--> switchB.ethg[0];
        FCAM.ethg <--> C <--> switchB.ethg[0];
        Control1.ethg <--> C <--> switchB.ethg[0];
        Control2.ethg <--> C <--> switchB.ethg[0];
}
