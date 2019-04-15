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

#ifndef NESTING_IEEE8021Q_QUEUE_GATING_SCHEDULE_H_
#define NESTING_IEEE8021Q_QUEUE_GATING_SCHEDULE_H_

#include <omnetpp.h>
#include <bitset>
#include <vector>

using namespace omnetpp;

namespace nesting {

/**
 * A schedule is an array of entries, that consist of a bitvector representing
 * gate states and an time value.
 */
template<typename T>
class Schedule {
protected:
    /**
     * Schedule entries, that consist of a length in abstract time units and
     * a scheduled object.
     */
    std::vector<std::tuple<int, T>> entries;

    /**
     * Total length of all schedule entries combined in abstract time units.
     */
    int totalLength = 0;
public:
    Schedule() {
    }

    virtual ~Schedule() {
    }
    ;

    /** Returns the number of entries of the schedule. */
    virtual unsigned int size() const {
        return entries.size();
    }

    /** Returns the scheduled object at a given index. */
    virtual T getScheduledObject(unsigned int index) const {
        return std::get < 1 > (entries[index]);
    }

    /**
     * Returns the time unit, how long an object at a given index is
     * scheduled.
     */
    virtual unsigned int getLength(unsigned int index) const {
        return std::get < 0 > (entries[index]);
    }

    /** Returns the total length of the schedule in abstract time units. */
    virtual unsigned int getLength() const {
        return totalLength;
    }

    /** Returns true if the schedule contains no entries. Otherwise false. */
    virtual bool isEmpty() const {
        return entries.empty();
    }

    /**
     * Adds a new entry add the end of the schedule.
     *
     * @param length          The length of the schedule entry in abstract
     *                        time units.
     * @param scheduledObject The schedule objects associated with the
     *                        scheduled entry.
     */
    virtual void addEntry(int length, T scheduledObject) {
        totalLength += length;
        entries.push_back(make_tuple(length, scheduledObject));
    }
};

} // namespace nesting

#endif /* NESTING_IEEE8021Q_QUEUE_GATING_SCHEDULE_H_ */
