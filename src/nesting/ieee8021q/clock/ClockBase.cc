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

#include "ClockBase.h"

namespace nesting {

void ClockBase::initialize() {
    if (simTime().getScaleExp() > SIMTIME_NS) {
        throw cRuntimeError(
                "The simulation time precision has to be at least nanoseconds.");
    }

    lastLocalTickTimestamp = simTime();
    lastGlobalTickTimestamp = simTime();
    lastGlobalNonTickTimestamp = simTime();
    clockRate = simTime().parse(par("clockRate"));

    WATCH(lastGlobalTickTimestamp);
    WATCH(lastGlobalNonTickTimestamp);
    WATCH(lastLocalTickTimestamp);

    tick = false;
}

ClockBase::~ClockBase() {
    for (auto entry : scheduledTicks) {
        deleteScheduledTickEntry(entry);
    }
    scheduledTicks.clear();
}

void ClockBase::handleMessage(cMessage* message) {
    if (message->isSelfMessage()) {

        assert(!scheduledTicks.empty());
        assert(message == scheduledTicks.front()->tickMessage);

        tick = true;

        updateTimeOnScheduledTick();
        notifyNextTick();
        deleteScheduledTickEntry(scheduledTicks.front());
        scheduledTicks.pop_front();

        tick = false;
    } else {
        throw cRuntimeError("Received invalid message. ");
    }
}

void ClockBase::addScheduledTickListener(unsigned int idleTicks,
        IClockListener* listener) {

    Enter_Method_Silent();

    // Binary search to find scheduled entry with equal or more idle ticks.
    std::list<ScheduledTickEntry*>::iterator it = lower_bound(
    scheduledTicks.begin(),
    scheduledTicks.end(),
    idleTicks,
    [](ScheduledTickEntry*& elem, unsigned int idleTicks) {
        return elem->scheduledTick.ticks < idleTicks;
    }
    );

    // Schedule tick if it was not already scheduled by previous subscription.
    ScheduledTickEntry* scheduledTickEntry;
    if (it != scheduledTicks.end() && (*it)->scheduledTick.ticks == idleTicks) {
        scheduledTickEntry = *it;
    }
    else {
        // Insert scheduled tick into datastructure.
        scheduledTickEntry = new ScheduledTickEntry();
        scheduledTickEntry->scheduledTick.ticks = idleTicks;
        scheduledTickEntry->scheduledTick.timestamp = scheduleTick(idleTicks);
        scheduledTickEntry->tickMessage = new cMessage();

        it = scheduledTicks.insert(it, scheduledTickEntry);

        // Send self-message
        scheduleAt(
        scheduledTickEntry->scheduledTick.timestamp,
        scheduledTickEntry->tickMessage
        );
        if(par("verbose")) {
            EV_DEBUG << getFullPath()<<": Scheduled new tick event in t-" << idleTicks
            << ". Scheduled ticks: ";
            for (auto entry : scheduledTicks) {
                EV_DEBUG << entry->scheduledTick.ticks << ", ";
            }
            EV_DEBUG << endl;
        }}

    // Add listener if he was not already added
    bool listenerNotAdded = find(
    scheduledTickEntry->listeners.begin(),
    scheduledTickEntry->listeners.end(),
    listener
    ) == scheduledTickEntry->listeners.end();

    if (listenerNotAdded) {
        scheduledTickEntry->listeners.push_back(listener);
    }
}

void ClockBase::updateTimeOnScheduledTick() {
    assert(!scheduledTicks.empty() && tick);

    ScheduledTickEntry* nextScheduledTickListeners = scheduledTicks.front();
    int elapsedTicks = nextScheduledTickListeners->scheduledTick.ticks;
    incrementTime(elapsedTicks);

    EV_DEBUG << getFullPath() << ": "
                    << scheduledTicks.front()->scheduledTick.ticks
                    << " tick(s) elapsed. " << "Local time: "
                    << lastLocalTickTimestamp << "." << endl;

    decrementScheduledTickCountdowns(elapsedTicks);
}

void ClockBase::updateTime() {
    assert(!tick);

    ScheduledTick lastScheduledTick = lastTick();

    // Invariant: No ticks elapsed since last scheduled tick
    // => last scheduled time-stamps are equal
    assert(
            lastScheduledTick.ticks != 0
                    || lastScheduledTick.timestamp == lastGlobalTickTimestamp);

    incrementTime(lastScheduledTick);
    decrementScheduledTickCountdowns(lastScheduledTick.ticks);
}

void ClockBase::incrementTime(unsigned int ticks) {
    if (ticks == 0) {
        lastGlobalNonTickTimestamp = simTime();
    } else {
        lastGlobalTickTimestamp = simTime();
        lastGlobalNonTickTimestamp = simTime();
        lastLocalTickTimestamp += clockRate * ticks;
    }
}

void ClockBase::incrementTime(ScheduledTick scheduledTick) {
    if (scheduledTick.ticks == 0) {
        lastGlobalNonTickTimestamp = simTime();
    } else {
        lastGlobalTickTimestamp = scheduledTick.timestamp;
        lastGlobalNonTickTimestamp = scheduledTick.timestamp;
        lastLocalTickTimestamp += clockRate * scheduledTick.ticks;
    }
}

void ClockBase::decrementScheduledTickCountdowns(unsigned int ticks) {
    for (ScheduledTickEntry* scheduledTickListeners : scheduledTicks) {
        scheduledTickListeners->scheduledTick.ticks -= ticks;
    }
}

void ClockBase::deleteScheduledTickEntry(ScheduledTickEntry* entry) {
    cancelAndDelete(entry->tickMessage);
    delete entry;
}

void ClockBase::notifyNextTick() {
    assert(!scheduledTicks.empty() && tick);

    // Remove the next tick so that notified listeners are able to schedule a new
    // tick event for t-0 ticks without interfering with the current next tick
    // that also happens at t-0.
    ScheduledTickEntry* nextScheduledTickListeners = scheduledTicks.front();
    scheduledTicks.pop_front();

    // Notify listeners that subscribed current tick
    for (IClockListener* listener : nextScheduledTickListeners->listeners) {
        listener->tick(this);
    }

    // Insert the next tick back into the datastructure. After this, it should be
    // possible that two ticks are scheduled for t-0. One currently processed by
    // the clock, another one scheduled.
    scheduledTicks.push_front(nextScheduledTickListeners);
}

simtime_t ClockBase::getTime() {
    if (!tick) {
        updateTime();
    }
    return lastLocalTickTimestamp;
}

simtime_t ClockBase::getClockRate() {
    return clockRate;
}

void ClockBase::subscribeTick(IClockListener* listener,
        unsigned int idleTicks) {
    Enter_Method_Silent();
    if (!tick) {
        updateTime();
    }
    addScheduledTickListener(idleTicks, listener);
}

void ClockBase::unsubscribeTicks(IClockListener* listener) {
    for (auto entry : scheduledTicks) {
        entry->listeners.erase(
                remove(entry->listeners.begin(), entry->listeners.end(),
                        listener), entry->listeners.end());
    }
}

} /* namespace nesting */
