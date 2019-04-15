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

#ifndef __MAIN_IDEALCLOCK_H_
#define __MAIN_IDEALCLOCK_H_

#include <omnetpp.h>
#include <tuple>

#include "ClockBase.h"

using namespace omnetpp;

namespace nesting {

/**
 * See the NED file for a detailed description
 */
class IdealClock: public ClockBase {
protected:
    /** @copydoc ScheduleTick ClockBase::lastTick() */
    virtual ScheduledTick lastTick() override;

    /** @copydoc simtime_t ClockBase::scheduleTick(unsigned int) */
    virtual simtime_t scheduleTick(unsigned int idleTicks) override;
public:
    virtual ~IdealClock() {
    }
    ;
};

} // namespace nesting

#endif
