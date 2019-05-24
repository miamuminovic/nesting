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
    @display("bgb=1007.5,477.1");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        switchA: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=227.5,184.59999");
            gates:
                ethg[8];
        }
        switchB: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=682.5,184.59999");
            gates:
                ethg[8];
        }
        Control3: VlanEtherHostSched {
            @display("p=139.09999,274.3");
        }
        HeadUnit: VlanEtherHostQ {
            @display("p=643.5,87.1");
        }
        AV_sink: VlanEtherHostQ {
            @display("p=304.19998,80.6");
        }
        Control4: VlanEtherHostSched {
            @display("p=227.5,274.3");
        }
        Camera3: VlanEtherHostSched {
            @display("p=84.5,185.9");
        }
        Video: VlanEtherHostSched {
            @display("p=227.5,80.6");
        }
        Audio: VlanEtherHostSched {
            @display("p=145.59999,80.6");
        }
        Bulk: VlanEtherHostSched {
            @display("p=304.19998,274.3");
        }
        PUCAM: VlanEtherHostQ {
            @display("p=721.5,87.1");
        }
        Camera1: VlanEtherHostSched {
            @display("p=527.8,274.3");
        }
        Camera2: VlanEtherHostSched {
            @display("p=612.3,274.3");
        }
        Control1: VlanEtherHostSched {
            @display("p=751.39996,274.3");
        }
        Control2: VlanEtherHostSched {
            @display("p=824.2,274.3");
        }
        FCAM: VlanEtherHostSched {
            @display("p=682.5,274.3");
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
