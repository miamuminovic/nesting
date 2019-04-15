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

#ifndef NESTING_COMMON_SCHEDULE_SCHEDULEBUILDER_H_
#define NESTING_COMMON_SCHEDULE_SCHEDULEBUILDER_H_

#include <omnetpp.h>
#include <algorithm>
#include "Schedule.h"
#include "../../ieee8021q/Ieee8021q.h"

using namespace omnetpp;

namespace nesting {

/**
 * Utility class to build schedules from e.g. XML files.
 */
class ScheduleBuilder final {
public:
    /**
     * Creates a schedule for containing bit vectors for transmission gates
     * from an XML file.
     *
     * TODO link to XML file specification
     */
    static Schedule<GateBitvector>* createGateBitvectorSchedule(
            cXMLElement *xml);

    /**
     * Creates a schedule containing one entry that opens all gates for the whole cycle duration
     */
    static Schedule<GateBitvector>* createDefaultBitvectorSchedule(
            cXMLElement *xml);
};

} // namespace nesting

#endif /* NESTING_COMMON_SCHEDULE_SCHEDULEBUILDER_H_ */
