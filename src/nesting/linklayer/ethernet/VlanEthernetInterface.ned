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

import inet.linklayer.ethernet.EtherMacFullDuplex;
import inet.linklayer.ethernet.EtherEncap;
import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.InterfaceEntry;
import nesting.linklayer.ethernet.VLANEncap;

//
// Ethernet network interface, used in Host modules in nesting project.
//
module VlanEthernetInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        @class(inet::InterfaceEntry);
        string interfaceTableModule;
        string address = default("auto"); // MAC address
        string fcsMode @enum("declared","computed") = default("declared");
        @display("i=block/ifcard;bgl=2");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        mac: EtherMacFullDuplex {
            @display("p=141,242");
        }
        etherEncap: EtherEncap {
            parameters:
                @display("p=141,152");
        }
        vlanEncap: VLANEncap {
            @display("p=141,68");
        }
    connections:
        mac.phys <--> { @display("m=s"); } <--> phys;

        mac.upperLayerOut --> etherEncap.lowerLayerIn;
        mac.upperLayerIn <-- etherEncap.lowerLayerOut;

        etherEncap.upperLayerOut --> { @display("m=n"); } --> vlanEncap.lowerLayerIn;
        etherEncap.upperLayerIn <-- { @display("m=n"); } <-- vlanEncap.lowerLayerOut;

        vlanEncap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        vlanEncap.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn;
}
