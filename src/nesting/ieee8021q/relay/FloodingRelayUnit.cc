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

#include "FloodingRelayUnit.h"

namespace nesting {

Define_Module(FloodingRelayUnit);

void FloodingRelayUnit::initialize() {
    if (gateSize("in") != gateSize("out")) {
        throw cRuntimeError(
                "The sizes of in[] and out[] vector gates must be equal.");
    }
}

void FloodingRelayUnit::handleMessage(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++) {
        cGate *outputGate = gate("out", i);
        if (!msg->arrivedOn("in", i)) {
            cMessage* dupMsg = msg->dup();
            send(dupMsg, outputGate);
        }
    }

    delete msg;
}

} // namespace nesting
