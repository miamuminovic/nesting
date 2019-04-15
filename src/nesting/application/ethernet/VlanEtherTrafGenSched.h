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

#ifndef __NESTING_VETHERTRAFGENSCHED_H
#define __NESTING_VETHERTRAFGENSCHED_H

#include <omnetpp.h>
#include <memory>

#include "inet/applications/ethernet/EtherTrafGen.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/InitStages.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/common/TimeTag_m.h"
#include <omnetpp/cxmlelement.h>
#include <vector>
#include "../../common/schedule/HostSchedule.h"
#include "../../common/schedule/HostScheduleBuilder.h"
#include "../../ieee8021q/clock/IClock.h"

using namespace omnetpp;
using namespace inet;

namespace nesting {

/**
 * See the NED file for a detailed description
 */
class VlanEtherTrafGenSched: public cSimpleModule, public IClockListener {
private:

    /** Current schedule. Is never null. */
    std::unique_ptr<HostSchedule<Ieee8021QCtrl>> currentSchedule;

    /**
     * Next schedule to load after the current schedule finishes it's cycle.
     * Can be null.
     */
    std::unique_ptr<HostSchedule<Ieee8021QCtrl>> nextSchedule;

    /** Index for the current entry in the schedule. */
    long int index = 0;

    IClock *clock;

protected:

    // receive statistics
    long TSNpacketsSent = 0;
    long packetsReceived = 0;
    simsignal_t sentPkSignal;
    simsignal_t rcvdPkSignal;
    int ssap = -1;
    int dsap = -1;

    int seqNum = 0;

    virtual void initialize(int stage) override;
    virtual void sendPacket();
    virtual void receivePacket(Packet *msg);
    virtual void handleMessage(cMessage *msg) override;

    virtual int numInitStages() const override;
    virtual int scheduleNextTickEvent();
public:
    virtual void tick(IClock *clock) override;

    /** Loads a new schedule into the gate controller. */
    virtual void loadScheduleOrDefault(cXMLElement* xml);
};

} // namespace nesting

#endif /* __NESTING_VETHERTRAFGENSCHED_H */
