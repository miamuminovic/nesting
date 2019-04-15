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

#ifndef __MAIN_ETHERTRAFGENQUEUE_H_
#define __MAIN_ETHERTRAFGENQUEUE_H_

#include <omnetpp.h>
#include <list>

#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/Protocol.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TimeTag_m.h"
#include "../../linklayer/common/VLANTag_m.h"

using namespace omnetpp;
using namespace inet;
using namespace std;

namespace nesting {

/** See the NED file for a detailed description */
class EtherTrafGenQueue: public cSimpleModule, public IPassiveQueue {
protected:
    /** Sequence number for generated packets. */
    long seqNum;

    /** EtherMacBase requests a packet twice during init, tmp fix so that this module only delivers one packet */
    bool doNotSendFirstInitPacket = true;

    /** Destination MAC address of generated packets. */
    MacAddress destMacAddress;

    // Parameters from NED file
    cPar* etherType;
    cPar* vlanTagEnabled;
    cPar* pcp;
    cPar* dei;
    cPar* vid;
    int ssap = -1;
    int dsap = -1;

    cPar* packetLength;

    /** Amount of packets sent for statistic. */
    long packetsSent;

    // signals
    simsignal_t sentPkSignal;
protected:
    virtual void initialize() override;

    virtual void handleMessage(cMessage *msg) override;
    virtual Packet* generatePacket();
public:
    virtual void requestPacket() override;
    virtual int getNumPendingRequests() override;
    virtual bool isEmpty() override;
    virtual void clear() {
    }
    ;
    virtual cMessage *pop() override;
    virtual void addListener(IPassiveQueueListener *listener) {
    }
    ;
    virtual void removeListener(IPassiveQueueListener *listener) {
    }
    ;
};

} // namespace nesting

#endif
