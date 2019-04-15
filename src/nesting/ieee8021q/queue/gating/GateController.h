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

#ifndef __MAIN_GATECONTROLLER_H_
#define __MAIN_GATECONTROLLER_H_

#include <omnetpp/simtime_t.h>
#include <memory>
#include <string>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/InitStages.h"
#include "inet/linklayer/ethernet/EtherMacFullDuplex.h"

#include "../TransmissionSelection.h"
#include "../../clock/IClock.h"
#include "../../../common/schedule/Schedule.h"
#include "../../clock/IClockListener.h"
#include "../../Ieee8021q.h"
#include "TransmissionGate.h"
#include "../../../common/schedule/ScheduleBuilder.h"
#include "../../../linklayer/framePreemption/EtherMACFullDuplexPreemptable.h"

using namespace omnetpp;

class TransmissionGate;
class EtherMACFullDuplexPreemptable;

namespace nesting {

/**
 * See the NED file for a detailed description
 */
class GateController: public cSimpleModule, public IClockListener {
private:
    /** Current schedule. Is never null. */
    std::unique_ptr<Schedule<GateBitvector>> currentSchedule;

    /**
     * Next schedule to load after the current schedule finishes it's cycle.
     * Can be null.
     */
    std::unique_ptr<Schedule<GateBitvector>> nextSchedule;

    /** Index for the current entry in the schedule. */
    unsigned int scheduleIndex;

    /**
     * Clock reference, needed to get the current time and subscribe
     * clock events.
     */
    IClock* clock;

    /** Reference to transmission gate vector module */
    std::vector<TransmissionGate*> transmissionGates;

    EtherMACFullDuplexPreemptable* preemptMacModule;
    inet::EtherMacFullDuplex* macModule;
    std::string switchString;
    std::string portString;
    simtime_t lastChange;
protected:
    /** @see cSimpleModule::initialize(int) */
    virtual void initialize(int stage) override;

    /** @see cSimpleModule::numInitStages() */
    virtual int numInitStages() const override;

    /** @see cSimpleModule::handleMessage(cMessage*) */
    virtual void handleMessage(cMessage *msg) override;

    /** Opens all transmission gates. */
    virtual void openAllGates(); // TODO use setGateStates internal

    virtual void setGateStates(GateBitvector bitvector, bool release);

public:
    virtual ~GateController();

    /** @see IClockListener::tick(IClock*) */
    virtual void tick(IClock *clock) override;

    /** Calculate the maximum bit size that can be transmitted until the next gate state change.
     *  Returns an unsigned integer value.
     **/
    virtual unsigned int calculateMaxBit(int gateIndex);

    /** extracts and loads the correct schedule from xml file, or an empty one if none is defined */
    virtual void loadScheduleOrDefault(cXMLElement* xml);

    virtual bool currentlyOnHold();

};

} // namespace nesting

#endif
