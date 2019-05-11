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

import inet.applications.ethernet.EtherAppServer;
import inet.applications.tcpapp.TcpGenericServerApp;
import inet.examples.inet.ipv4hook.MyHost;
import inet.examples.wireless.hosttohost.ThroughputServer;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherHost2;
import inet.node.ipv6.StandardHost6;
import inet.node.packetdrill.PacketDrillHost;
import ned.DatarateChannel;
import nesting.node.ethernet.VlanEtherHost;
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
            datarate = 100Mbps;
        }
    submodules:
        switchA: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=188,127");
            gates:
                ethg[8];
        }
        switchB: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=643,127");
            gates:
                ethg[8];
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
            @display("p=487.5,215.8");
        }
        Camera2: VlanEtherHostSched {
            @display("p=572,217");
        }
        Control1: VlanEtherHostSched {
            @display("p=712,217");
        }
        Control2: VlanEtherHostSched {
            @display("p=784,217");
        }
        FCAM: VlanEtherHostSched {
            @display("p=642.2,215.8");
        }
    connections:
        switchA.ethg[0] <--> C <--> switchB.ethg[0];
        switchA.ethg[1] <--> C <--> AV_sink.ethg;
        Video.ethg <--> C <--> switchA.ethg[2];
        Audio.ethg <--> C <--> switchA.ethg[3];
        Camera3.ethg <--> C <--> switchA.ethg[4];
        Control3.ethg <--> C <--> switchA.ethg[5];
        Control4.ethg <--> C <--> switchA.ethg[6];
        Bulk.ethg <--> C <--> switchA.ethg[7];
        switchB.ethg[6] <--> C <--> PUCAM.ethg;
        switchB.ethg[7] <--> C <--> HeadUnit.ethg;
        Camera1.ethg <--> C <--> switchB.ethg[1];
        Camera2.ethg <--> C <--> switchB.ethg[2];
        Control1.ethg <--> C <--> switchB.ethg[4];
        Control2.ethg <--> C <--> switchB.ethg[5];
        FCAM.ethg <--> C <--> switchB.ethg[3];
}
