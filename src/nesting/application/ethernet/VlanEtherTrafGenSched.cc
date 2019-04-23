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

#include "VlanEtherTrafGenSched.h"

#define COMPILETIME_LOGLEVEL omnetpp::LOGLEVEL_TRACE

namespace nesting {

Define_Module(VlanEtherTrafGenSched);

void VlanEtherTrafGenSched::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        // Signals
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");

        seqNum = 0;
        //WATCH(seqNum);

        // statistics
        TSNpacketsSent = packetsReceived = 0;
        WATCH(TSNpacketsSent);
        WATCH(packetsReceived);

        cModule* clockModule = getModuleFromPar<cModule>(par("clockModule"),
                this);
        clock = check_and_cast<IClock*>(clockModule);

        llcSocket.setOutputGate(gate("out"));
    } else if (stage == INITSTAGE_LINK_LAYER) {
        //clock module reference from ned parameter

        currentSchedule = std::unique_ptr < HostSchedule
                < Ieee8021QCtrl >> (new HostSchedule<Ieee8021QCtrl>());
        cXMLElement* xml = par("initialSchedule").xmlValue();
        loadScheduleOrDefault(xml);

        currentSchedule = move(nextSchedule);
        nextSchedule.reset();

        clock->subscribeTick(this, scheduleNextTickEvent());

        llcSocket.open(-1, ssap);
    }
}

int VlanEtherTrafGenSched::numInitStages() const {
    return INITSTAGE_LINK_LAYER + 1;
}

void VlanEtherTrafGenSched::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        //disregard for now
    } else {
        receivePacket(check_and_cast<Packet *>(msg));
    }
}

void VlanEtherTrafGenSched::sendPacket() {

    char msgname[40];
    sprintf(msgname, "pk-%d-%d", getId(), seqNum);

    // create new packet
    Packet *datapacket = new Packet(msgname, IEEE802CTRL_DATA);
    long len = currentSchedule->getSize(index);
    const auto& payload = makeShared<ByteCountChunk>(B(len));
    // set creation time
    auto timeTag = payload->addTag<CreationTimeTag>();
    timeTag->setCreationTime(simTime());

    datapacket->insertAtBack(payload);
    datapacket->removeTagIfPresent<PacketProtocolTag>();
    datapacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(
            &Protocol::ethernetMac);
    // TODO check if protocol is correct
    auto sapTag = datapacket->addTagIfAbsent<Ieee802SapReq>();
    sapTag->setSsap(ssap);
    sapTag->setDsap(dsap);

    seqNum++;

    // get scheduled control data
    Ieee8021QCtrl header = currentSchedule->getScheduledObject(index);
    // create mac control info
    auto macTag = datapacket->addTag<MacAddressReq>();
    macTag->setDestAddress(header.macTag.getDestAddress());
    // create VLAN control info
    auto ieee8021q = datapacket->addTag<VLANTagReq>();
    ieee8021q->setPcp(header.q1Tag.getPcp());
    ieee8021q->setDe(header.q1Tag.getDe());
    ieee8021q->setVID(header.q1Tag.getVID());

    EV_TRACE << getFullPath() << ": Send TSN packet '" << datapacket->getName()
                    << "' at time " << clock->getTime().inUnit(SIMTIME_US)
                    << endl;

    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    TSNpacketsSent++;
}

void VlanEtherTrafGenSched::receivePacket(Packet *msg) {
    EV_TRACE << getFullPath() << ": Received packet '" << msg->getName()
                    << "' with length " << msg->getByteLength() << "B at time "
                    << clock->getTime().inUnit(SIMTIME_US) << endl;

    packetsReceived++;
    emit(rcvdPkSignal, msg);

    delete msg;
}

void VlanEtherTrafGenSched::tick(IClock *clock) {
    Enter_Method("tick()");
    // When the current schedule index is 0, this means that the current
    // schedule's cycle was not started or was just finished. Therefore in this
    // case a new schedule is loaded if available.
    if (index == currentSchedule->size() ) {
        // Load new schedule and delete the old one.
        if (nextSchedule) {
            currentSchedule = move(nextSchedule);
            nextSchedule.reset();
        }
        index = 0;
        clock->subscribeTick(this, scheduleNextTickEvent());

    }
    else {
        sendPacket();
        index++;
        clock->subscribeTick(this, scheduleNextTickEvent());
    }
}

/* This method returns the timeinterval between
 * the last sent frame and the frame to be sent next */
int VlanEtherTrafGenSched::scheduleNextTickEvent() {
    if (currentSchedule->size() == 0) {
        return currentSchedule->getCycle();
    } else if (index == currentSchedule->size()) {
        return currentSchedule->getCycle() - currentSchedule->getTime(index - 1);
    } else if (index % currentSchedule->size() == 0) {
        return currentSchedule->getTime(index);
    } else {
        return currentSchedule->getTime(index % currentSchedule->size())
                - currentSchedule->getTime(index % currentSchedule->size() - 1);
    }
}

void VlanEtherTrafGenSched::loadScheduleOrDefault(cXMLElement* xml) {
    std::string hostName =
            this->getModuleByPath(par("hostModule"))->getFullName();
    HostSchedule<Ieee8021QCtrl>* schedule;
    bool realScheduleFound = false;
    //try to extract the part of the schedule belonging to this host
    for (cXMLElement* hostxml : xml->getChildren()) {
        if (strcmp(hostxml->getTagName(), "cycle") != 0
                && hostxml->getAttribute("name") == hostName) {
            schedule = HostScheduleBuilder::createHostScheduleFromXML(hostxml,
                    xml);

            EV_DEBUG << getFullPath() << ": Found schedule for name "
                            << hostName << endl;

            realScheduleFound = true;
            break;
        }
    }
    //load empty schedule if there is no part that affects this host in the schedule xml
    if (!realScheduleFound) {
        cXMLElement* defaultXml = par("emptySchedule").xmlValue();
        schedule = HostScheduleBuilder::createHostScheduleFromXML(defaultXml,
                xml);
    }
    std::unique_ptr < HostSchedule < Ieee8021QCtrl >> schedulePtr(schedule);

    nextSchedule.reset();
    nextSchedule = move(schedulePtr);

}

} // namespace nesting
