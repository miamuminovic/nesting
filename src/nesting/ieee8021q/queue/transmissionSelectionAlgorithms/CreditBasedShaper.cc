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

#include "CreditBasedShaper.h"

namespace nesting {

Define_Module(CreditBasedShaper);

CreditBasedShaper::~CreditBasedShaper() {
    cancelEvent(&endSpendingCreditMessage);
    cancelEvent(&reachedZeroCreditMessage);
}

void CreditBasedShaper::initialize() {
    TSAlgorithm::initialize();

    // Initialize credit value
    credit = 0;
    WATCH(credit);

    // Initialize idle slope value
    idleSlopeFactor = par("idleSlopeFactor");
    WATCH(idleSlopeFactor);
    if (idleSlopeFactor <= 0 || idleSlopeFactor >= 1) {
        throw cRuntimeError(
                "Value of idleSlope for credit-based-shaper must be in the range (0,1)");
    }

    // Initialize state
    updateState(kIdle);
}

void CreditBasedShaper::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (msg == &endSpendingCreditMessage) {
            handleEndSpendingCreditEvent();
        } else if (msg == &reachedZeroCreditMessage) {
            handleZeroCreditReachedEvent();
        } else {
            TSAlgorithm::handleMessage(msg);
        }
    } else {
        Packet* packet = check_and_cast<Packet*>(msg);
        handleSendPacketEvent(packet);
        send(packet, "out");
    }
}

void CreditBasedShaper::refreshDisplay() const {
    char buf[80];
    sprintf(buf, "credit-based\ncredit: %d", static_cast<int>(credit));
    getDisplayString().setTagArg("t", 0, buf);
}

double CreditBasedShaper::getIdleSlope() {
    return idleSlopeFactor * getPortTransmitRate();
}

double CreditBasedShaper::getSendSlope() {
    return getPortTransmitRate() - getIdleSlope();
}

double CreditBasedShaper::getPortTransmitRate() {
    return mac->getTxRate();
}

double CreditBasedShaper::creditsForTime(double creditPerSecond,
        simtime_t time) {
    assert(creditPerSecond > 0);
    assert(time >= SimTime::ZERO);

    simtime_t secondsPerCredit = SimTime(1, SIMTIME_S) / creditPerSecond;
    return time / secondsPerCredit;
}

simtime_t CreditBasedShaper::timeForCredits(double creditPerSecond,
        double credit) {
    assert(creditPerSecond > 0);
    assert(credit >= 0);

    double seconds = credit / creditPerSecond;
    return seconds * SimTime(1, SIMTIME_S);
}

simtime_t CreditBasedShaper::idleTimeToZeroCredit() {
    assert(credit < 0);
    return simTime() + timeForCredits(getIdleSlope(), 0 - credit);
}

simtime_t CreditBasedShaper::transmissionTime(Packet* packet) {
    simtime_t transmissionTime = timeForCredits(getPortTransmitRate(),
            Ieee8021q::getFinalEthernet2FrameBitLength(packet));
    return transmissionTime;
}

void CreditBasedShaper::updateState(State newState) {
    state = newState;
    lastEventTimestamp = simTime();

    EV_DEBUG << getFullPath() << ": New state: [state=";
    switch (state) {
    case kIdle:
        EV_INFO << "idle";
        break;
    case kSpendCredit:
        EV_INFO << "spendCredit";
        break;
    case kEarnCredit:
        EV_INFO << "earnCredit";
        break;
    }
    EV_DEBUG << ",credit=" << credit << "]" << endl;
}

void CreditBasedShaper::spendCredit(Packet* packet) {
    double spendCredit = creditsForTime(getSendSlope(),
            transmissionTime(packet));
    credit -= spendCredit;

    EV_DEBUG << getFullPath() << ": Spending " << spendCredit
                    << " credit to transmit "
                    << Ieee8021q::getFinalEthernet2FrameBitLength(packet)
                    << "bits (" << packet->getBitLength()
                    << "bits payload without headers)." << endl;
}

void CreditBasedShaper::earnCredits(simtime_t time) {
    double earnedCredit = creditsForTime(getIdleSlope(), time);
    credit += earnedCredit;

    EV_DEBUG << getFullPath() << ": Earned " << earnedCredit << " credit."
                    << endl;
}

void CreditBasedShaper::resetCredit() {
    credit = 0;

    EV_DEBUG << getFullPath() << ": Resetted credit." << endl;
}

bool CreditBasedShaper::isCreditPositive() {
    return static_cast<int>(credit) >= 0;
}

bool CreditBasedShaper::isPacketReadyForTransmission() {
    uint64_t mtuSize = kEthernet2MaximumTransmissionUnitBitLength.get();
    return !queue->isEmpty(mtuSize);
}

void CreditBasedShaper::handleGateStateChangedEvent() {
    if (transmissionGate->isGateOpen()) {
        EV_TRACE << getFullPath() << ": Handle gate opened event." << endl;
        if (state == kIdle && isPacketReadyForTransmission()) {
            updateState(kEarnCredit);
            if (!isCreditPositive()) {
                scheduleAt(idleTimeToZeroCredit(), &reachedZeroCreditMessage);
            }
        }
    } else {
        EV_TRACE << getFullPath() << ": Handle gate closed event." << endl;
        if (state == kEarnCredit) {
            cancelEvent(&reachedZeroCreditMessage);
            earnCredits(simTime() - lastEventTimestamp);
            updateState(kIdle);
        }
    }
}

void CreditBasedShaper::handlePacketEnqueuedEvent() {
    assert(isPacketReadyForTransmission());

    EV_TRACE << getFullPath() << ": Handle packet enqueued event." << endl;

    if (state == kIdle && transmissionGate->isGateOpen()) {
        updateState(kEarnCredit);
    } else if (state == kEarnCredit) {
        cancelEvent(&reachedZeroCreditMessage);
        earnCredits(simTime() - lastEventTimestamp);
        updateState(kEarnCredit);
    }

    // If new state is earning credits then maybe send self message to signal
    // when zero credits are reached.
    if (state == kEarnCredit && !isCreditPositive()) {
        scheduleAt(idleTimeToZeroCredit(), &reachedZeroCreditMessage);
    }

    // If positive amount of credit is available pass packetEnqueued event to
    // subsequent queues.
    if (isCreditPositive()) {
        transmissionGate->packetEnqueued();
    }
}

void CreditBasedShaper::handleSendPacketEvent(Packet* packet) {
    assert(state != kSpendCredit);
    assert(isCreditPositive());
    assert(!reachedZeroCreditMessage.isScheduled());

    EV_TRACE << getFullPath() << ": Handle send packet event." << endl;

    if (state == kEarnCredit) {
        earnCredits(simTime() - lastEventTimestamp);
    }

    spendCredit(packet);

    scheduleAt(simTime() + transmissionTime(packet), &endSpendingCreditMessage);
    updateState(kSpendCredit);
}

void CreditBasedShaper::handleEndSpendingCreditEvent() {
    assert(state == kSpendCredit);

    EV_TRACE << getFullPath() << ": Handle end spending credit event." << endl;

    if (!isPacketReadyForTransmission() && isCreditPositive()) {
        resetCredit();
        updateState(kIdle);
    } else if (isPacketReadyForTransmission()
            && transmissionGate->isGateOpen()) {
        updateState(kEarnCredit);
        if (credit < 0) {
            scheduleAt(idleTimeToZeroCredit(), &reachedZeroCreditMessage);
        }
    } else {
        updateState(kIdle);
    }
}

void CreditBasedShaper::handleZeroCreditReachedEvent() {
    assert(state == kEarnCredit);
    assert(transmissionGate->isGateOpen());

    EV_TRACE << getFullPath() << ": Handle zero credit reached event." << endl;

    earnCredits(simTime() - lastEventTimestamp);
    updateState(kEarnCredit);
    assert(isCreditPositive());

    if (isCreditPositive() && isPacketReadyForTransmission()) {
        transmissionGate->packetEnqueued();
    }
}

bool CreditBasedShaper::isEmpty(uint64_t maxBits) {
    return !isCreditPositive() || queue->isEmpty(maxBits);
}

} // namespace nesting
