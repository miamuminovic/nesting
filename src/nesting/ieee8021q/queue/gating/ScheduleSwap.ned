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

package nesting.ieee8021q.queue.gating;

//
// This module is used to initiate schedule changes in ~GateController,
// ~FilteringDatabase and ~VlanEtherTrafGenSched modules according to an own
// configurable schedule.
//
// @see ~GateController, ~FilteringDatabase, ~VlanEtherTrafGenSched, ~IClock
//
simple ScheduleSwap
{
    parameters:
        string clockModule = default("^.clock"); // Path to the ~IClock module
        string switchModule = default("^"); // Path to the ~VlanEtherSwitch module
        string filteringDatabaseModule = default("^.filteringDatabase"); // Path to the ~FilteringDatabase module
        string gateControllerModules = default("^.eth[%d].queuing.gateController"); // Wildcard path that is used to determine the paths to ~GateController modules
        string tsnGenModule = default("^.trafGenSchedApp"); // Path to the ~VlanEtherTrafGenSched module that generates TSN traffic
        xml schedule = default(xml("<schedules></schedules>")); // Internal micro schedule that manages the micro schedules
        bool usedInHost = default(false);
        bool verbose = default(false);
        @display("i=block/switch");
        @class(ScheduleSwap);
}
