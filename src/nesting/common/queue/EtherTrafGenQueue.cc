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

#include "EtherTrafGenQueue.h"

#include <omnetpp/ccomponent.h>
#include <omnetpp/cexception.h>
#include <omnetpp/clog.h>
#include <omnetpp/cmessage.h>
#include <omnetpp/cnamedobject.h>
#include <omnetpp/cobjectfactory.h>
#include <omnetpp/cpacket.h>
#include <omnetpp/cpar.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/cwatch.h>
#include <omnetpp/regmacros.h>
#include <omnetpp/simutil.h>
#include <cstdio>
#include <iostream>
#include <string>

#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace nesting {

Define_Module(EtherTrafGenQueue);

void EtherTrafGenQueue::initialize() {
    // Signals
    sentPkSignal = registerSignal("sentPk");

    // Initialize sequence-number for generated packets
    seqNum = 0;

    // NED parameters
    etherType = &par("etherType");
    vlanTagEnabled = &par("vlanTagEnabled");
    pcp = &par("pcp");
    dei = &par("dei");
    vid = &par("vid");
    packetLength = &par("packetLength");
    const char *destAddress = par("destAddress");
    if (!destMacAddress.tryParse(destAddress)) {
        throw new cRuntimeError("Invalid MAC Address");
    }

    // Statistics
    packetsSent = 0;
    WATCH(packetsSent);

}

void EtherTrafGenQueue::handleMessage(cMessage *msg) {
    throw cRuntimeError("cannot handle messages.");
}

Packet* EtherTrafGenQueue::generatePacket() {
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
            &Protocol::ipv4);
    // TODO check if protocol is correct
    auto sapTag = datapacket->addTagIfAbsent<Ieee802SapReq>();
    sapTag->setSsap(ssap);
    sapTag->setDsap(dsap);

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
    return datapacket;
}

void EtherTrafGenQueue::requestPacket() {
    Enter_Method("requestPacket(...)");

    /*
    if(doNotSendFirstInitPacket) {
        doNotSendFirstInitPacket = false;
        return;
    }
    */

    Packet* packet = generatePacket();

    if(par("verbose")) {
        auto macTag = packet->findTag<MacAddressReq>();
        auto ieee8021qTag = packet->findTag<VLANTagReq>();
        EV_TRACE << getFullPath() << ": Send packet `" << packet->getName() << "' dest=" << macTag->getDestAddress()
        << " length=" << packet->getBitLength() << "B type= empty" << " vlan-tagged=false";
        if(ieee8021qTag) {
            EV_TRACE << " vlan-tagged=true" << " pcp=" << ieee8021qTag->getPcp()
            << " dei=" << ieee8021qTag->getDe() << " vid=" << ieee8021qTag->getVID();
        }
        EV_TRACE << endl;
    }
    emit(sentPkSignal, packet);
    send(packet, "out");
    packetsSent++;
}

int EtherTrafGenQueue::getNumPendingRequests() {
    // Requests are always served immediately,
    // therefore no pending requests exist.
    return 0;
}

bool EtherTrafGenQueue::isEmpty() {
    // Queue is never empty
    return false;
}

cMessage* EtherTrafGenQueue::pop() {
    return generatePacket();
}

} // namespace nesting
