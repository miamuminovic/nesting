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

#ifndef NESTING_IEEE8021Q_CLOCK_CLOCKBASE_H_
#define NESTING_IEEE8021Q_CLOCK_CLOCKBASE_H_

#include <omnetpp.h>
#include <tuple>
#include <list>
#include <algorithm>
#include <functional>

#include "IClock.h"

using namespace omnetpp;

namespace nesting {

/**
 * Abstract base class for clock implementations using the IClock interface.
 *
 * The clock component owns a local time value that is increment in discrete
 * tick events. For optimization purposes, not every tick is simulated but only
 * the subscribed ones. Therefore the clock also uses the global simulation
 * time to calculate e.g. how many discrete ticks have elapsed since the last
 * simulated tick.
 */
class ClockBase: public IClock, public cSimpleModule {
public:
    /**
     * A scheduled tick is a tuple consisting of a time-stamp and the number
     * of ticks that will be elapsed from now until the time-stamp (global
     * simulation time).
     */
    struct ScheduledTick {
        /** Number of ticks to elapse. */
        unsigned int ticks;

        /** Global point in time when the associated ticks will be elapsed. */
        simtime_t timestamp;
    };

    /**
     * Scheduled tick entries contain the relevant data to manage scheduled
     * ticks within the clock component.
     */
    struct ScheduledTickEntry {
        /**
         * Information about when the tick is scheduled and how many ticks will
         * be elapse until that point.
         */
        ScheduledTick scheduledTick;

        /**
         * Message that is used as self message to signal that the tick was
         * triggered.
         */
        cMessage *tickMessage;

        /** Listeners that have subscribed themselves for the tick event. */
        std::list<IClockListener*> listeners;
    };
protected:
    /** Global time-stamp when the last tick event happened. */
    simtime_t lastGlobalTickTimestamp;

    /**
     * Global time-stamp that describes a time interval since the last global
     * tick event in which for sure no tick event has happened. This means that
     * in the interval from lastGlobalTimestamp until
     * lastGlobalNonTickTimestamp no tick has been elapsed.
     *
     * This variable is useful for implementing stochastic clocks. In this case
     * this value could be used to calculate conditional probabilities.
     */
    simtime_t lastGlobalNonTickTimestamp;

    /** Local time-stamp when the last tick event happened. */
    simtime_t lastLocalTickTimestamp;

    /**
     * The clock rate is the value by which the clock's time is incremented
     * every tick.
     */
    simtime_t clockRate;

    /** Flag to tell if currently a discrete tick is happening. */
    bool tick;

    /**
     * This list contains the scheduled ticks of the clock.
     *
     * Keep in mind, that once a tick is scheduled for e.g. 10 ticks in the
     * future, the tick will stay scheduled even if all listeners unsubscribe
     * themselves from the event. Also a tick never get rescheduled to another
     * point in time.
     *
     * For implementing a stochastic clock it is important, that the scheduled
     * ticks are conditional values that influence probabilities of future
     * ticks.
     */
    std::list<ScheduledTickEntry*> scheduledTicks;

protected:
    /** @copydoc cSimpleModule::initialize() */
    virtual void initialize() override;

    /** @copydoc cSimpleModule::handleMessage(cMessage*) */
    virtual void handleMessage(cMessage* message) override;

    /**
     * This method schedules a tick event and subscribes a listener to that
     * tick event.
     *
     * Calling this method requires that the clocks time values are up to date.
     *
     * If a tick event is already scheduled for a given number of ticks to
     * elapse. The already scheduled tick event is used to subscribe to instead
     * of rescheduling the event or generating a new one.
     *
     * If the given listener is already subscribed to that tick event he is not
     * added again.
     *
     * @param idleTicks Number of ticks to elapse until the scheduled event.
     * @param listener The listener to subscribe to the tick event.
     */
    virtual void addScheduledTickListener(unsigned int idleTicks,
            IClockListener* listener);

    /**
     * Updates the clocks internal time values within a discrete tick event of
     * the clock, respectively on a scheduled tick event.
     *
     * It is required to call this method only within a discrete tick event.
     * This means that tick = true.
     */
    virtual void updateTimeOnScheduledTick();

    /**
     * This method is used to update the clocks internal time values between
     * scheduled tick events.
     */
    virtual void updateTime();

    /**
     * Increments the clock's internal time values by a given number of ticks.
     */
    virtual void incrementTime(unsigned int ticks);

    /**
     * Increments the clock's internal time values by a scheduled tick. This
     * means the local time value is incremented by a given number of ticks
     * from the scheduled tick and the global time value is set to the
     * scheduled tick's time-stamp.
     */
    virtual void incrementTime(ScheduledTick scheduledTick);

    /** Decrements the tick countdowns of all scheduled ticks. */
    virtual void decrementScheduledTickCountdowns(unsigned int ticks);

    /** Notifies the next scheduled tick's listeners. */
    virtual void notifyNextTick();

    /** Deletes a scheduled tick entry and it's subcomponents from memory. */
    virtual void deleteScheduledTickEntry(ScheduledTickEntry* entry);

    /**
     * From the point of the current state (when the clock was updated the
     * last time) this method has to return the number of elapsed ticks until
     * now, as well as the global time stamp when the last tick event was
     * triggered.
     *
     * For implementing a stochastic clock it is important to consider that
     * the clock state is conditional to calculate tick events.
     */
    virtual ScheduledTick lastTick() = 0;

    /**
     * This method must return the global time-stamp when a given number of
     * tick events will be elapsed from the point of view of the clock's
     * current time.
     *
     * For implementing a stochastic clock it is important to consider that
     * the clock state is conditional to calculate tick events.
     *
     * This also means, the result of this method must not violate
     * before-and-after relations regarding other already scheduled tick
     * events.
     *
     * @param idleTicks The number of ticks to elapse.
     * @result          The global time-stamp when the given number of ticks
     *                  will be elapsed.
     */
    virtual simtime_t scheduleTick(unsigned int idleTicks) = 0;

public:
    virtual ~ClockBase();

    /**
     * Returns the clock's internal time value. This method can trigger a time
     * update if not called within a discrete tick event.
     */
    virtual simtime_t getTime() override;

    /** Returns the clock's clock rate. */
    virtual simtime_t getClockRate() override;

    /**
     * Subscribes a listener to receive a tick event of the clock.
     *
     * @param listener  The listener to receive a notification about the clock
     *                  event.
     * @param idleTicks The number of ticks to elapse until the listener gets
     *                  notified.
     */
    virtual void subscribeTick(IClockListener* listener, unsigned int idleTicks)
            override;

    /**
     * Unsubscribes a listener from all ticks he is subscribed to.
     * Unsubscribing from clock ticks won't cause ticks to get unscheduled.
     */
    virtual void unsubscribeTicks(IClockListener* listener);
};

} /* namespace nesting */

#endif /* NESTING_IEEE8021Q_CLOCK_CLOCKBASE_H_ */
