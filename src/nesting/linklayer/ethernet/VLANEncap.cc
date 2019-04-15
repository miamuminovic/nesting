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

#include "VLANEncap.h"
#define COMPILETIME_LOGLEVEL omnetpp::LOGLEVEL_TRACE

namespace nesting {

Define_Module(VLANEncap);

void VLANEncap::initialize(int stage) {
    if (stage == inet::INITSTAGE_LOCAL) {
        // Signals
        encapPkSignal = registerSignal("encapPk");
        decapPkSignal = registerSignal("decapPk");

        verbose = par("verbose");
        tagUntaggedFrames = par("tagUntaggedFrames");
        pvid = par("pvid");

        totalFromHigherLayer = 0;
        WATCH(totalFromHigherLayer);

        totalFromLowerLayer = 0;
        WATCH(totalFromLowerLayer);

        totalEncap = 0;
        WATCH(totalEncap);

        totalDecap = 0;
        WATCH(totalDecap);
    } else if (stage == inet::INITSTAGE_LINK_LAYER) {
        if (par("registerProtocol").boolValue()) {
            registerService(inet::Protocol::ipv4, gate("upperLayerIn"),
            nullptr);
            registerProtocol(inet::Protocol::ipv4, nullptr,
                    gate("upperLayerOut"));
            registerService(inet::Protocol::ipv4, gate("lowerLayerIn"),
            nullptr);
            registerProtocol(inet::Protocol::ipv4, nullptr,
                    gate("lowerLayerOut"));
            // TODO check if protocol is correct
        }
    }
}

void VLANEncap::handleMessage(cMessage* msg) {
    if (dynamic_cast<inet::Packet *>(msg) == nullptr){delete msg; return;}
    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);

    if (packet->arrivedOn("lowerLayerIn")) {
        processPacketFromLowerLevel(packet);
    } else {
        processPacketFromHigherLevel(packet);
    }
}

void VLANEncap::processPacketFromHigherLevel(inet::Packet *packet) {
    EV_INFO << getFullPath() << ": Received " << packet << " from upper layer."
                   << endl;

    totalFromHigherLayer++;

    // Encapsulate VLAN Header
    if (packet->findTag<VLANTagReq>()) {
        auto vlanTag = packet->getTag<VLANTagReq>();
        const auto& vlanHeader = inet::makeShared<inet::Ieee8021qHeader>();
        vlanHeader->setPcp(vlanTag->getPcp());
        vlanHeader->setDe(vlanTag->getDe());
        vlanHeader->setVid(vlanTag->getVID());
        packet->insertAtFront(vlanHeader);
        delete packet->removeTagIfPresent<VLANTagReq>();
        // Statistics and logging
        EV_INFO << getFullPath() << ":Encapsulating higher layer packet `"
                       << packet->getName() << "' into VLAN tag" << endl;
        totalEncap++;
        emit(encapPkSignal, packet);
    }

    EV_TRACE << getFullPath() << ": Packet-length is "
                    << packet->getByteLength() << " and Destination is "
                    << packet->getTag<inet::MacAddressReq>()->getDestAddress()
                    << " before sending packet to lower layer" << endl;

    send(packet, "lowerLayerOut");
}

void VLANEncap::processPacketFromLowerLevel(inet::Packet *packet) {
    EV_INFO << getFullPath() << ": Received " << packet << " from lower layer."
                   << endl;

    totalFromLowerLayer++;

    // Decapsulate packet if it is a VLAN Tag, otherwise just insert default
    // values into the control information
    EV_TRACE << getFullPath() << ": Packet-length is "
                    << packet->getByteLength() << ", destination is "
                    << packet->getTag<inet::MacAddressInd>()->getDestAddress();
    if (packet->hasAtFront<inet::Ieee8021qHeader>()) {
        auto vlanHeader = packet->popAtFront<inet::Ieee8021qHeader>();
        auto vlanTag = packet->addTagIfAbsent<VLANTagInd>();
        vlanTag->setPcp(vlanHeader->getPcp());
        vlanTag->setDe(vlanHeader->getDe());
        EV_TRACE << ", PCP Value is " << (int) vlanTag->getPcp() << " ";
        short vid = vlanHeader->getVid();
        if (vid < kMinValidVID || vid > kMaxValidVID) {
            vid = pvid;
        }
        vlanTag->setVID(vid);
        EV_TRACE << getFullPath() << ": Decapsulating packet and `"
                        << "' passing up contained packet `"
                        << packet->getName() << "' to higher layer" << endl;

        totalDecap++;
        emit(decapPkSignal, packet);
    } else {
        auto vlanTag = packet->addTagIfAbsent<VLANTagInd>();
        vlanTag->setPcp(kDefaultPCPValue);
        vlanTag->setDe(kDefaultDEIValue);
        vlanTag->setVID(pvid);
    }

    EV_TRACE << " before sending packet up" << endl;

    // Send packet to upper layer
    send(packet, "upperLayerOut");
}

void VLANEncap::refreshDisplay() const {
    char buf[80];
    sprintf(buf, "up/decap: %ld/%ld\ndown/encap: %ld/%ld", totalFromLowerLayer,
            totalDecap, totalFromHigherLayer, totalEncap);
    getDisplayString().setTagArg("t", 0, buf);
}

int VLANEncap::getPVID() {
    return pvid;
}

} // namespace nesting
