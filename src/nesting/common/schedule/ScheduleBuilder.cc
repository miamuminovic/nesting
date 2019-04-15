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

#include "ScheduleBuilder.h"

namespace nesting {

Schedule<GateBitvector>* ScheduleBuilder::createGateBitvectorSchedule(
        cXMLElement *xml) {
    Schedule<GateBitvector>* schedule = new Schedule<GateBitvector>();

    std::vector<cXMLElement*> entries = xml->getChildrenByTagName("entry");
    for (cXMLElement* entry : entries) {
        // Get length
        const char* lengthCString =
                entry->getFirstChildWithTag("length")->getNodeValue();
        unsigned int length = atoi(lengthCString);

        // Get bitvector
        const char* bitvectorCString =
                entry->getFirstChildWithTag("bitvector")->getNodeValue();
        std::string originalVector = std::string(bitvectorCString);
        reverse(originalVector.begin(), originalVector.end());
        GateBitvector bitvector = GateBitvector(originalVector);

        schedule->addEntry(length, bitvector);
    }

    return schedule;
}

Schedule<GateBitvector>* ScheduleBuilder::createDefaultBitvectorSchedule(
        cXMLElement *xml) {
    Schedule<GateBitvector>* schedule = new Schedule<GateBitvector>();
    const char* lengthCString =
            xml->getFirstChildWithTag("cycle")->getNodeValue();
    unsigned int length = atoi(lengthCString);
    std::string gateString(kMaxSupportedQueues, '1');
    GateBitvector bitvector = GateBitvector(gateString);
    schedule->addEntry(length, bitvector);
    return schedule;
}
} // namespace nesting
