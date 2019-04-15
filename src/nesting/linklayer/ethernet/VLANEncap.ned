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

//
// This module performs encapsulation/decapsulation of VLAN Tags. This module
// should be used in combination with the EtherEncap modul of inet.
//
// When a frame is received from the lower layer, it is expected to have an
// Ieee802Ctrl control information attached. There are two cases for processing
// packets from the lower layer:
//
// (1)	A frame is of type Ethernet1QTag (the frame "is" a VLAN Tag) it is
//		decapsulated and the VLAN Tag fields are added to the frame within a
//		Ieee8021QCtrl control information.
//
// (2)	If a frame is not of type Ethernet1QType (the frame "is" no VLAN Tag)
//		an Ieee8021QCtrl information with default values for PCP, DEI, VID
//		as specified in this NED file is added.
//
// Analog in the other direction, when a frame is received from the higher
// layer, the Frame is possibly encapsulated into a Ethernet1QTag (VLAN Tag is 
// added) when the Ieee8021QCtrl control information the packet must have,
// indicates it.
//
// Alternatively the tagUntaggedFrames flag can be set. Then every frame is
// encapsulated in an default VLAN Tag. Of course this would alter the packet
// size.
//
// @see ~EtherEncap, ~Ieee802Ctrl, ~Ieee8021QCtrl, ~Ethernet1QTag
//
import inet.linklayer.ethernet.IEtherEncap;

simple VLANEncap like IEtherEncap
{
    parameters:
        @display("i=block/square");
        @class(VLANEncap);
        bool tagUntaggedFrames = default(false);	// If true, the encap process will add a VLAN Tag to untagged frames. This alters the frame size.
        int pvid = default(1);						// Default port-VID inserted in control info of (vlan-)untagged frames. 
        bool registerProtocol = default(false);
        @signal[encapPk](type=cPacket);
        @signal[decapPk](type=cPacket);
        @statistic[encapPk](title="packets encapsulated"; source=encapPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[decapPk](title="packets decapsulated"; source=decapPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        bool verbose = default(false);
    gates:
        input upperLayerIn @labels(Ieee8021QCtrl/down);
        output upperLayerOut @labels(Ieee802Ctrl/up);
        input lowerLayerIn @labels(Ieee8021QCtrl/up);
        output lowerLayerOut @labels(Ieee802Ctrl/down);
}
