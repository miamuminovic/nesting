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

package nesting.common.queue;

//
// This module provides a queue containing an endless amount of ethernet
// packets and implements the IPassiveQueue Interface of INET.
//
// The queue is never empty and everytime a packet is requested the queue 
// generates one and sends it out.
//
simple EtherTrafGenQueue
{
    parameters:
        @display("i=block/queue");
        @class(EtherTrafGenQueue);
        string destAddress = default("");		// destination MAC address, or module path name of destination station; empty means off
        int etherType = default(0);				// etherType to set on outgoing packets
        bool vlanTagEnabled = default(true);	// When true, the application's packets will be VLAN encapsulated by the VLANEncap module, otherwise the pcp, dei, vid value is ignored
        int pcp = default(0);					// PCP (Priority Code Point) value that is added to the packets control information
        bool dei = default(false);				// DEI (Drop Eligible Indicator) that is added to the packets control information
        int vid = default(1);					// VID (VLAN Identifier) value that is added to the packets control information
        volatile int packetLength @unit(B);		// length of packets to send
        @signal[sentPk](type=cPacket);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        bool verbose = default(false);
    gates:
        output out @labels(Ieee8021QCtrl/down);
}
