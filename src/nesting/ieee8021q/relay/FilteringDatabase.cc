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

#include "FilteringDatabase.h"

namespace nesting {

Define_Module(FilteringDatabase);

FilteringDatabase::FilteringDatabase() {
    this->agingActive = false;
    this->agingThreshold = 0;
}

FilteringDatabase::FilteringDatabase(bool agingActive,
        simtime_t agingThreshold) {
    this->agingActive = agingActive;
    this->agingThreshold = agingThreshold;
}

FilteringDatabase::~FilteringDatabase() {
}

void FilteringDatabase::clearAdminFdb() {
    adminFdb.clear();
}
void FilteringDatabase::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        cXMLElement* fdb = par("database");
        cXMLElement* cycleXml = par("cycle");
        cycle = atoi(cycleXml->getFirstChildWithTag("cycle")->getNodeValue());
        loadDatabase(fdb, cycle);

        cModule* clockModule = getModuleByPath(par("clockModule"));
        clock = check_and_cast<IClock*>(clockModule);

//    WATCH_MAP(fdb)
    } else if (stage == INITSTAGE_LINK_LAYER) {
        clock->subscribeTick(this, 0);
    }
}

int FilteringDatabase::numInitStages() const {
    return INITSTAGE_LINK_LAYER + 1;
}

void FilteringDatabase::loadDatabase(cXMLElement* xml, int cycle) {
    newCycle = cycle;

    std::string switchName =
            this->getModuleByPath(par("switchModule"))->getFullName();
    cXMLElement* fdb;
    //TODO this bool can probably be refactored to a nullptr check
    bool databaseFound = false;
    //try to extract the part of the filteringDatabase xml belonging to this module
    for (cXMLElement* host : xml->getChildren()) {
        if (host->hasAttributes() && host->getAttribute("id") == switchName) {
            fdb = host;
            databaseFound = true;
            break;
        }
    }

    //only continue if a filtering database was found for this switch
    if (!databaseFound) {
        return;
    }

    // Get static rules from XML file
    cXMLElement* staticRules = fdb->getFirstChildWithTag("static");

    if (staticRules != nullptr) {
        clearAdminFdb();

        cXMLElement* forwardingXml = staticRules->getFirstChildWithTag(
                "forward");
        if (forwardingXml != nullptr) {
            this->parseEntries(forwardingXml);
        }

        changeDatabase = true;
    }

}

void FilteringDatabase::parseEntries(cXMLElement* xml) {
    // If present get rules from XML file
    if (xml == nullptr) {
        throw new cRuntimeError("Illegal xml input");
    }
    // Rules for individual addresses
    cXMLElementList individualAddresses = xml->getChildrenByTagName(
            "individualAddress");

    for (auto individualAddress : individualAddresses) {

        std::string macAddressStr = std::string(
                individualAddress->getAttribute("macAddress"));
        if (macAddressStr.empty()) {
            throw cRuntimeError(
                    "individualAddress tag in forwarding database XML must have an "
                            "macAddress attribute");
        }

        if (!individualAddress->getAttribute("port")) {
            throw cRuntimeError(
                    "individualAddress tag in forwarding database XML must have an "
                            "port attribute");
        }

        std::vector<int> port;
        port.insert(port.begin(), 1,
                atoi(individualAddress->getAttribute("port")));

        uint8_t vid = 0;
        if (individualAddress->getAttribute("vid"))
            vid = static_cast<uint8_t>(atoi(
                    individualAddress->getAttribute("vid")));

        // Create and insert entry for different individual address types
        if (vid == 0) {
            MacAddress macAddress;
            if (!macAddress.tryParse(macAddressStr.c_str())) {
                throw new cRuntimeError("Cannot parse invalid Mac address.");
            }
            adminFdb.insert( { macAddress,
                    std::pair<simtime_t, std::vector<int>>(0, port) });
        } else {
            // TODO
            throw cRuntimeError(
                    "Individual address rules with VIDs aren't supported yet");
        }
    }

    // Rules for multicastAddresses
    cXMLElementList multicastAddresses = xml->getChildrenByTagName(
            "multicastAddress");
    for (auto multicastAddress : multicastAddresses) {
        std::string macAddressStr = std::string(
                multicastAddress->getAttribute("macAddress"));
        if (macAddressStr.empty()) {
            throw cRuntimeError(
                    "multicastAddress tag in forwarding database XML must have an "
                            "macAddress attribute");
        }

        if (!multicastAddress->getAttribute("ports")) {
            throw cRuntimeError(
                    "multicastAddress tag in forwarding database XML must have an "
                            "ports attribute");
        }
        std::string portsString = multicastAddress->getAttribute("ports");
        std::vector<int> port;
        unsigned int i = 0;
        while (i <= portsString.length()) {
            port.push_back(portsString[i] - '0');
            i += 2;
        }

        uint8_t vid = 0;
        if (multicastAddress->getAttribute("vid"))
            vid = static_cast<uint8_t>(atoi(
                    multicastAddress->getAttribute("vid")));

        // Create and insert entry for different individual address types
        if (vid == 0) {
            MacAddress macAddress;
            if (!macAddress.tryParse(macAddressStr.c_str())) {
                throw new cRuntimeError("Cannot parse invalid Mac address.");
            }
            if (!macAddress.isMulticast()) {
                throw new cRuntimeError(
                        "Mac address is not a Multicast address.");
            }
            adminFdb.insert( { macAddress,
                    std::pair<simtime_t, std::vector<int>>(0, port) });
        } else {
            // TODO
            throw cRuntimeError(
                    "Multicast address rules with VIDs aren't supported yet");
        }
    }
}

void FilteringDatabase::tick(IClock *clock) {
    if (changeDatabase) {
        operFdb.swap(adminFdb);
        cycle = newCycle;
        clearAdminFdb();

        EV_INFO << getFullPath() << ": Loading filtering database at time "
                       << clock->getTime().inUnit(SIMTIME_US) << endl;

        changeDatabase = false;
    }
    clock->subscribeTick(this, cycle);
}

void FilteringDatabase::handleMessage(cMessage *msg) {
    throw cRuntimeError("Must not receive messages.");
}

void FilteringDatabase::insert(MacAddress macAddress, simtime_t curTS,
        int port) {
    std::vector<int> tmp;
    tmp.insert(tmp.begin(), 1, port);
    operFdb[macAddress] = std::pair<simtime_t, std::vector<int>>(curTS, tmp);
}

int FilteringDatabase::getPort(MacAddress macAddress, simtime_t curTS) {
    simtime_t ts;
    std::vector<int> port;

    auto it = operFdb.find(macAddress);

    //is element available?
    if (it != operFdb.end()) {
        ts = it->second.first;
        port = it->second.second;
        // return if mac address belongs to multicast
        if (port.size() != 1) {
            return -1;
        }
        // static entries (ts == 0) do not age
        if (!agingActive || (ts == 0 || curTS - ts < agingThreshold)) {
            operFdb[macAddress] = std::pair<simtime_t, std::vector<int>>(curTS,
                    port);
            return port.at(0);
        } else {
            operFdb.erase(macAddress);
        }
    }

    return -1;
}

std::vector<int> FilteringDatabase::getPorts(MacAddress macAddress,
        simtime_t curTS) {
    simtime_t ts;
    std::vector<int> ports;

    if (!macAddress.isMulticast()) {
        ports.push_back(-1);
        return ports;
    }

    auto it = operFdb.find(macAddress);

    //is element available?
    if (it != operFdb.end()) {
        ts = it->second.first;
        ports = it->second.second;
        // static entries (ts == 0) do not age
        if (!agingActive || (ts == 0 || curTS - ts < agingThreshold)) {
            operFdb[macAddress] = std::pair<simtime_t, std::vector<int>>(curTS,
                    ports);
            return ports;
        } else {
            operFdb.erase(macAddress);
        }

    }

    ports.push_back(-1);
    return ports;
}

} // namespace nesting
