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
import inet.common.queue.Sink;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.IEtherEncap;
import inet.linklayer.ethernet.EtherMacFullDuplex;
import inet.networklayer.common.InterfaceEntry;

//
// This module implements an ethernet interface with queueing capabilities of Ieee802.1Q. Frame Preemption is not supported.
//
module VlanEthernetInterfaceSwitch extends InterfaceEntry like IEthernetInterface
{
    parameters:
        @class(inet::InterfaceEntry);
        string interfaceTableModule;
        string address = default("auto"); // MAC address
        string fcsMode @enum("declared","computed") = default("declared");
        @display("i=block/ifcard;bgl=2;bgb=296,407");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
        bool framePreemptionDisabled = default(true);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        mac: EtherMacFullDuplex {
            @display("p=113,354");
        }
        queuing: Queuing {
            parameters:
                @display("p=202,51;q=l2queue");
        }
        vlanEncapE: VLANEncap {
            @display("p=113,108");
        }
        etherEncapE: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=113,240");
        }
        sink: Sink {
            @display("p=202,181");
        }
    connections:
        upperLayerIn --> queuing.in;
        vlanEncapE.upperLayerOut --> upperLayerOut;

        queuing.eOut --> vlanEncapE.upperLayerIn;
        queuing.pOut --> sink.in++;

        vlanEncapE.lowerLayerOut --> etherEncapE.upperLayerIn;
        etherEncapE.upperLayerOut --> vlanEncapE.lowerLayerIn;

        mac.upperLayerOut --> etherEncapE.lowerLayerIn;
        etherEncapE.lowerLayerOut --> mac.upperLayerIn;

        mac.phys <--> { @display("m=s"); } <--> phys;
}
