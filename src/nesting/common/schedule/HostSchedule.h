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

#ifndef NESTING_COMMON_SCHEDULE_HOSTSCHEDULE_H_
#define NESTING_COMMON_SCHEDULE_HOSTSCHEDULE_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;

namespace nesting {

/**
 * A host-schedule is an array of entries, that consist of a timestamp, a size in bytes and an Ieee8021QCtrl-info containing values
 * such as destination address and PCP.
 */
template<typename T>
class HostSchedule {
protected:
    /**
     * Schedule entries, that consist of a length in abstract time units and
     * a scheduled object.
     */
    std::vector<std::tuple<int, int, T>> entries;

    /**
     * Total cycletime of this schedule.
     */
    int cycle = 0;

public:
    HostSchedule() {
    }

    virtual ~HostSchedule() {
    }
    ;

    /** Returns the number of entries of the schedule. */
    virtual unsigned int size() const {
        return entries.size();
    }

    /** Sets the Cycletime of this schedule. */
    virtual void setCycle(unsigned int cycleLength) {
        cycle = cycleLength;
    }

    /** Returns the number of entries of the schedule. */
    virtual unsigned int getCycle() const {
        return cycle;
    }

    /** Returns the scheduled object at a given index. */
    virtual T getScheduledObject(unsigned int index) const {
        return std::get < 2 > (entries[index]);
    }

    /**
     * Returns the time when the scheduled object is supposed to be sent.
     */
    virtual unsigned int getTime(unsigned int index) const {
        return std::get < 0 > (entries[index]);
    }

    /**
     *  Return the size of the scheduled object at a given index.
     */
    virtual unsigned int getSize(unsigned int index) const {
        return std::get < 1 > (entries[index]);
    }

    /** Returns true if the schedule contains no entries. Otherwise false. */
    virtual bool isEmpty() const {
        return (entries.size() == 0);
    }

    /**
     * Adds a new entry add the end of the schedule.
     *
     * @param time          The length of the schedule entry in abstract
     *                        time units.
     * @param size          The size of the scheduled entry in bytes.
     *
     * @param scheduledObject The schedule objects associated with the
     *                        scheduled entry.
     */
    virtual void addEntry(int time, int size, T scheduledObject) {
        entries.push_back(std::make_tuple(time, size, scheduledObject));
    }
};

} // namespace nesting

#endif /* NESTING_COMMON_SCHEDULE_HOSTSCHEDULE_H_ */
