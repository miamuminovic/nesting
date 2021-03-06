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

#include "VlanEtherTrafGen.h"

namespace nesting {

Define_Module(VlanEtherTrafGen);

void VlanEtherTrafGen::initialize(int stage) {
    EtherTrafGen::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        vlanTagEnabled = &par("vlanTagEnabled");
        pcp = &par("pcp");
        dei = &par("dei");
        vid = &par("vid");
    }
}

void VlanEtherTrafGen::sendBurstPackets() {
    int n = numPacketsPerBurst->intValue();
    for (int i = 0; i < n; i++) {
        seqNum++;

        char msgname[40];
        sprintf(msgname, "pk-%d-%ld", getId(), seqNum);

        // create new packet
        Packet *datapacket = new Packet(msgname, IEEE802CTRL_DATA);
        long len = packetLength->intValue();
        const auto& payload = makeShared<ByteCountChunk>(B(len));
        // set creation time
        auto timeTag = payload->addTag<CreationTimeTag>();
        timeTag->setCreationTime(simTime());

        datapacket->insertAtBack(payload);
        datapacket->removeTagIfPresent<PacketProtocolTag>();
        datapacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(
                &Protocol::ethernetMac);
        // TODO check which protocol to insert
        auto sapTag = datapacket->addTagIfAbsent<Ieee802SapReq>();
        sapTag->setSsap(ssap);
        sapTag->setDsap(dsap);

        // create control info for encap modules
        auto macTag = datapacket->addTag<MacAddressReq>();
        macTag->setDestAddress(destMacAddress);

        uint8_t PCP;
        bool de;
        short VID;
        // create VLAN control info
        if (vlanTagEnabled->boolValue()) {
            auto ieee8021q = datapacket->addTag<VLANTagReq>();
            PCP = pcp->intValue();
            de = dei->boolValue();
            VID = vid->intValue();
            ieee8021q->setPcp(PCP);
            ieee8021q->setDe(de);
            ieee8021q->setVID(VID);
        }

        EV_TRACE << getFullPath() << ": Send packet `" << datapacket->getName()
                        << "' dest=" << macTag->getDestAddress() << " length="
                        << datapacket->getBitLength() << "B type="
                        << IEEE802CTRL_DATA << " vlan-tagged="
                        << vlanTagEnabled->boolValue();
        if (vlanTagEnabled->boolValue()) {
            EV_TRACE << " pcp=" << PCP << " dei=" << de << " vid=" << VID;
        }
        EV_TRACE << endl;

        emit(packetSentSignal, datapacket);
        send(datapacket, "out");
        packetsSent++;
    }
}

} // namespace nesting
