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

#include "../../queue/gating/GateController.h"
#define COMPILETIME_LOGLEVEL omnetpp::LOGLEVEL_TRACE

namespace nesting {

Define_Module(GateController);

GateController::~GateController() {
    transmissionGates.clear();
    currentSchedule.reset();
    nextSchedule.reset();
}

void GateController::initialize(int stage) {
    //initialize clock and gate references in first stage
    if (stage == INITSTAGE_LOCAL) {
        scheduleIndex = 0;
        // Keep reference to clock module
        cModule* clockModule = getModuleFromPar<cModule>(par("clockModule"),
                this);
        clock = check_and_cast<IClock*>(clockModule);

        // Iterate through transmission gates an keep them as references
        TransmissionGate* transmissionGateVectorModule = getModuleFromPar<
                TransmissionGate>(par("transmissionGateVectorModule"), this);
        cModule::SubmoduleIterator it = cModule::SubmoduleIterator(
                transmissionGateVectorModule->getParentModule());
        for (; !it.end(); it++) {
            cModule* subModule = *it;
            if (subModule->isName(transmissionGateVectorModule->getName())) {
                TransmissionGate* transmissionGate = check_and_cast<
                        TransmissionGate*>(subModule);
                transmissionGates.push_back(transmissionGate);
            }
        }

        cModule* macMod = getModuleFromPar<cModule>(par("macModule"), this);

        EtherMACFullDuplexPreemptable* macTmp =
                dynamic_cast<EtherMACFullDuplexPreemptable*>(macMod);
        if (macTmp != nullptr) {
            preemptMacModule = check_and_cast<EtherMACFullDuplexPreemptable*>(
                    macMod);
            macModule = nullptr;
        } else {
            macModule = check_and_cast<inet::EtherMacFullDuplex*>(macMod);
            preemptMacModule = nullptr;
        }

        WATCH(scheduleIndex);
    }
    //initialize schedule in second stage when clock is initialized
    else if (stage == INITSTAGE_LINK_LAYER) {

        switchString =
                this->getModuleByPath(par("switchModule"))->getFullName();
        portString = std::to_string(
                this->getModuleByPath(par("networkInterfaceModule"))->getIndex());

        lastChange = simTime();
        currentSchedule = std::unique_ptr < Schedule
                < GateBitvector >> (new Schedule<GateBitvector>());
        cXMLElement* xml = par("initialSchedule").xmlValue();
        loadScheduleOrDefault(xml);
        if (par("enableHoldAndRelease")) {
            //Schedule hold for the first entry if needed.
            //This is needed because hold is only always requested for the following entry,
            //but not for the current one. Therefore the first entry would not be held.
            for (TransmissionGate* transmissionGate : transmissionGates) {
                if ((!currentSchedule->isEmpty()
                        && currentSchedule->getScheduledObject(0).test(
                                transmissionGate->getIndex()))
                        && transmissionGate->isExpressQueue()) {
                    preemptMacModule->hold(SIMTIME_ZERO);
                    break;
                }
            }
        }
        clock->subscribeTick(this, 0);
    }
}

int GateController::numInitStages() const {
    return INITSTAGE_LINK_LAYER + 1;
}

void GateController::handleMessage(cMessage *msg) {
    throw cRuntimeError("cannot handle messages");
}

void GateController::tick(IClock *clock) {
    Enter_Method("tick()");

// When the current schedule index is 0, this means that the current
// schedule's cycle was not started or was just finished. Therefore in this
// case a new schedule is loaded if available.
    if (scheduleIndex == 0 && nextSchedule) {
        // Print warning if the feature is used in combination with frame preemption
        if(preemptMacModule != nullptr) {
            if( preemptMacModule->isFramePreemptionEnabled() && par("enableHoldAndRelease")) {
                EV_WARN << "Using schedule swap in combination with Hold&Release (Frame Preemption) can lead to wrong hold periods."<<endl;
            }
        }
        // Load new schedule and delete the old one.
        currentSchedule = move(nextSchedule);
        nextSchedule.reset();

        // If an empty schedule was loaded, all gates are opened and there is no
        // need to subscribe to clock ticks
        if (currentSchedule->isEmpty()) {
            openAllGates();
            return;
        }
    }

    // Get next gatestate bitvector
    GateBitvector bitvector = currentSchedule->getScheduledObject(scheduleIndex);
    bool releaseNeeded = false;
    if(par("enableHoldAndRelease")) {
        //Check whether some express gate is open
        bool someExpressGateOpen=false;
        for (TransmissionGate* transmissionGate : transmissionGates) {
            if(bitvector.test(transmissionGate->getIndex()) && transmissionGate->isExpressQueue()) {
                someExpressGateOpen=true;
                break;
            }
        }
        //If the Mac component was on hold and no express gate is opened, release it
        releaseNeeded = !someExpressGateOpen && currentlyOnHold();
        if(releaseNeeded) {
            if(preemptMacModule != nullptr) {
                preemptMacModule->release();
            }
        }
    }
    //Set gate states for every gate
    setGateStates(bitvector, releaseNeeded);

    EV_DEBUG << getFullPath() << ": Got Tick. Setting gates to "<< bitvector << " at time "<< clock->getTime().inUnit(SIMTIME_US) << endl;
    std::stringstream ss;
    for (TransmissionGate* transmissionGate : transmissionGates) {
        if(transmissionGate->isGateOpen()) {
            ss << "1";
        }
        else {
            ss << "0";
        }
    }
    EV_DEBUG << getFullPath() << ": Actual gate states: " << ss.str() << " at time "<< clock->getTime().inUnit(SIMTIME_US) << endl;

    // Subscribe to the tick, on which a new schedule entry is loaded.
    clock->subscribeTick(this, currentSchedule->getLength(scheduleIndex));
    lastChange = clock->getTime();

    if(par("enableHoldAndRelease")) {
        //Get following bitvector to be able to schedule hold with advance
        GateBitvector nextVector;
        if(nextSchedule && scheduleIndex == currentSchedule->size()-1) {
            //If we are at the last entry of the current schedule, look at the first entry of the next one
            nextVector = nextSchedule->getScheduledObject(0);
        } else {
            nextVector = currentSchedule->getScheduledObject((scheduleIndex + 1) % currentSchedule->size());
        }

        for (TransmissionGate* transmissionGate : transmissionGates) {
            //Schedule hold if any express gate is open in the next schdule state
            if(nextVector.test(transmissionGate->getIndex()) && transmissionGate->isExpressQueue()) {
                if(preemptMacModule!=nullptr) {
                    preemptMacModule->hold((currentSchedule->getLength(scheduleIndex))*clock->getClockRate()-preemptMacModule->getHoldAdvance());
                    break;
                }
            }
        }
    }

    // Switch to next schedule entry
    scheduleIndex = (scheduleIndex + 1) % currentSchedule->size();
}

unsigned int GateController::calculateMaxBit(int gateIndex) {
    double transmitRate;
    if (preemptMacModule != nullptr) {
        transmitRate = preemptMacModule->getTxRate();
    } else {
        transmitRate = macModule->getTxRate();
    }
    if (transmitRate <= 0) {
        return 0;
    }
    simtime_t clockRate = clock->getClockRate();
    simtime_t timeSinceLastChange = clock->getTime() - lastChange;

    double bits = 0;
    //Has the lookahead already touched the next Schedule
    bool touchedNextSchedule = false;
    unsigned int currentIndex = (scheduleIndex + currentSchedule->size() - 1)
            % currentSchedule->size();

    GateBitvector bitvector = currentSchedule->getScheduledObject(currentIndex);
    while (bits < kEthernet2MaximumTransmissionUnitBitLength.get()) {
        //if the bitvector is now closed, return all bit summed up until now
        if (!bitvector.test(gateIndex)) {
            return static_cast<int>(bits);
        }
        //only look in current schedule if there is no nextSchedule
        if (!nextSchedule) {
            bits = bits
                    + ((currentSchedule->getLength(currentIndex) * clockRate)
                            - timeSinceLastChange)
                            / (SimTime(1, SIMTIME_S) / transmitRate);
            timeSinceLastChange = SIMTIME_ZERO;
            currentIndex = (currentIndex + 1) % currentSchedule->size();
            bitvector = currentSchedule->getScheduledObject(currentIndex);
        } else {
            //if there is a nextSchedule but it is not yet being looked at
            if (!touchedNextSchedule) {
                bits =
                        bits
                                + ((currentSchedule->getLength(currentIndex)
                                        * clockRate) - timeSinceLastChange)
                                        / (SimTime(1, SIMTIME_S) / transmitRate);
                timeSinceLastChange = SIMTIME_ZERO;
                //if currentIndex is not the last index in currentSchedule
                if (currentIndex < currentSchedule->size() - 1) {
                    currentIndex = (currentIndex + 1) % currentSchedule->size();
                    bitvector = currentSchedule->getScheduledObject(
                            currentIndex);
                } else {
                    //if it is the last index in currentSchedule, look into nextSchedule from now on
                    touchedNextSchedule = true;
                    currentIndex = 0;
                    bitvector = nextSchedule->getScheduledObject(currentIndex);
                }
            } else {
                //if the nextSchedule is being looked at
                bits = bits
                        + ((nextSchedule->getLength(currentIndex) * clockRate)
                                - timeSinceLastChange)
                                / (SimTime(1, SIMTIME_S) / transmitRate);
                timeSinceLastChange = SIMTIME_ZERO;
                currentIndex = (currentIndex + 1) % nextSchedule->size();
                bitvector = nextSchedule->getScheduledObject(currentIndex);
            }
        }
    }
    //otherwise, return the full MTU bits
    return kEthernet2MaximumTransmissionUnitBitLength.get();

}

void GateController::loadScheduleOrDefault(cXMLElement* xml) {
    Schedule<GateBitvector> *schedule;
    bool realScheduleFound = false;

    //try to extract the part of the schedule belonging to this switch and port
    if (xml != nullptr && xml->hasChildren()) {
        for (cXMLElement* host : xml->getChildren()) {
            if (strcmp(host->getTagName(), "cycle") != 0
                    && host->getAttribute("name") == switchString) {
                for (cXMLElement* port : host->getChildrenByTagName("port")) {
                    if (port->getAttribute("id") == portString) {
                        schedule = ScheduleBuilder::createGateBitvectorSchedule(
                                port);
                        realScheduleFound = true;
                        break;
                    }
                }
                break;
            }
        }
//if the schedule xml does not contain scheduling information for this port,
//create a schedule that has the same cycle as the others, but opens all gates the entire time
        if (!realScheduleFound) {
            schedule = ScheduleBuilder::createDefaultBitvectorSchedule(xml);
        }
    } else {
//use the default xml that has no entry, but a default cycle defined
        cXMLElement* defaultXml = par("emptySchedule").xmlValue();
        schedule = ScheduleBuilder::createGateBitvectorSchedule(defaultXml);
    }

    EV_DEBUG << getFullPath() << ": Loading schedule. Cycle is "
                    << schedule->getLength() << ". Entry count is "
                    << schedule->size() << ". Time is "
                    << clock->getTime().inUnit(SIMTIME_US) << endl;

    std::unique_ptr<Schedule<GateBitvector>> schedulePtr(schedule);
    nextSchedule = move(schedulePtr);
}

void GateController::setGateStates(GateBitvector bitvector, bool release) {
    for (TransmissionGate* transmissionGate : transmissionGates) {
        transmissionGate->setGateState(
                bitvector.test(transmissionGate->getIndex()), release);
    }
}

void GateController::openAllGates() {
    GateBitvector bitvectorAllGatesOpen;
    bitvectorAllGatesOpen.set();
    setGateStates(bitvectorAllGatesOpen, true);
}
bool GateController::currentlyOnHold() {
    if (preemptMacModule != nullptr) {
        return preemptMacModule->isOnHold();
    } else {
        return false;
    }
}
}
// namespace nesting
