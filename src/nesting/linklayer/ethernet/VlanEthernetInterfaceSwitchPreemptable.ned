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

package nesting.linklayer.ethernet;

@namespace();

import nesting.ieee8021q.queue.Queuing;
import nesting.linklayer.ethernet.VLANEncap;
import nesting.linklayer.framePreemption.FrameForward;
import nesting.linklayer.framePreemption.EtherMACFullDuplexPreemptable;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.IEtherEncap;
import inet.networklayer.common.InterfaceEntry;

//
// This module implements an ethernet interface that supports frame preemption.
//
module VlanEthernetInterfaceSwitchPreemptable extends InterfaceEntry like IEthernetInterface
{
    parameters:
        @class(inet::InterfaceEntry);
        string interfaceTableModule;
        string address = default("auto"); // MAC address
        string fcsMode @enum("declared","computed") = default("declared");
        @display("i=block/ifcard;bgl=2");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
        bool framePreemptionDisabled = default(true);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        // mac supporting frame preemption
        mac: EtherMACFullDuplexPreemptable {
            @display("p=140,511");
        }
        queuing: Queuing {
            parameters:
                @display("p=202,61;q=l2queue");
        }
        vlanEncapE: VLANEncap {
            @display("p=90,199");
        }
        etherEncapE: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=90,333");
        }
        vlanEncapP: VLANEncap {
            @display("p=202,199");
        }
        etherEncapP: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=202,333");
        }
        frameForward: FrameForward {
            @display("p=90,61");
        }
    connections:
        upperLayerIn --> queuing.in;
        frameForward.upperLayerOut --> upperLayerOut;

        vlanEncapE.upperLayerOut --> frameForward.lowerLayerInE;
        vlanEncapP.upperLayerOut --> frameForward.lowerLayerInP;

        queuing.eOut --> vlanEncapE.upperLayerIn;
        queuing.pOut --> vlanEncapP.upperLayerIn;

        etherEncapE.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> etherEncapE.lowerLayerIn;

        etherEncapP.lowerLayerOut --> mac.upperLayerPreemptableIn;
        mac.upperLayerPreemptableOut --> etherEncapP.lowerLayerIn;

        vlanEncapE.lowerLayerOut --> etherEncapE.upperLayerIn;
        etherEncapE.upperLayerOut --> vlanEncapE.lowerLayerIn;

        vlanEncapP.lowerLayerOut --> etherEncapP.upperLayerIn;
        etherEncapP.upperLayerOut --> vlanEncapP.lowerLayerIn;

        mac.phys <--> { @display("m=s"); } <--> phys;
}
