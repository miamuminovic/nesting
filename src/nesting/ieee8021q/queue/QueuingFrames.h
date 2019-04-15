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

#ifndef __STUPRONESTINGPROJECT_QUEUINGFRAMES_H_
#define __STUPRONESTINGPROJECT_QUEUINGFRAMES_H_

#include <omnetpp.h>
#include <array>

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "../Ieee8021q.h"
#include "../../linklayer/common/VLANTag_m.h"

using namespace omnetpp;

namespace nesting {

/** See NED file for a detailed description */
class QueuingFrames: public cSimpleModule {
private:
    /** Number of queues per port */
    int numberOfQueues;

    /**
     * A static implementation of the traffic class mapping from the standard
     */
    int standardTrafficClassMapping[kNumberOfPCPValues][kNumberOfPCPValues] =
        {
              { 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 1, 1, 1, 1 },
              { 0, 0, 0, 0, 1, 1, 2, 2 },
              { 0, 0, 1, 1, 2, 2, 3, 3 },
              { 0, 0, 1, 1, 2, 2, 3, 4 },
              { 1, 0, 2, 2, 3, 3, 4, 5 },
              { 1, 0, 2, 3, 4, 4, 5, 6 },
              { 1, 0, 2, 3, 4, 5, 6, 7 }
          };


    int getFramePriority(int numberOfQueues);
protected:
    virtual void initialize();

    virtual void handleMessage(cMessage *msg);
};

} // namespace nesting

#endif
