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
            @display("p=139,478");
        }
        queuing: Queuing {
            parameters:
                @display("p=202,61;q=l2queue");
        }
        vlanEncapE: VLANEncap {
            @display("p=90,333");
        }
        etherEncapE: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=90,185");
        }
        vlanEncapP: VLANEncap {
            @display("p=202,333");
        }
        etherEncapP: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=202,185");
        }
        frameForward: FrameForward {
            @display("p=90,61");
        }
    connections:
        upperLayerIn --> queuing.in;
        frameForward.upperLayerOut --> upperLayerOut;

        etherEncapE.upperLayerOut --> frameForward.lowerLayerInE;
        etherEncapP.upperLayerOut --> frameForward.lowerLayerInP;

        queuing.eOut --> etherEncapE.upperLayerIn;
        queuing.pOut --> etherEncapP.upperLayerIn;

        vlanEncapE.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> vlanEncapE.lowerLayerIn;

        vlanEncapP.lowerLayerOut --> mac.upperLayerPreemptableIn;
        mac.upperLayerPreemptableOut --> vlanEncapP.lowerLayerIn;

        etherEncapE.lowerLayerOut --> vlanEncapE.upperLayerIn;
        vlanEncapE.upperLayerOut --> etherEncapE.lowerLayerIn;

        etherEncapP.lowerLayerOut --> vlanEncapP.upperLayerIn;
        vlanEncapP.upperLayerOut --> etherEncapP.lowerLayerIn;

        mac.phys <--> { @display("m=s"); } <--> phys;
}
