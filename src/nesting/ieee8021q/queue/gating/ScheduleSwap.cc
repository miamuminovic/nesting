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

#include "../../queue/gating/ScheduleSwap.h"

namespace nesting {

Define_Module(ScheduleSwap);

void ScheduleSwap::initialize(int stage) {
    //initialize clock in first stage
    if (stage == INITSTAGE_LOCAL) {
        // Keep reference to clock module
        cModule* clockModule = getModuleFromPar<cModule>(par("clockModule"),
                this);
        clock = check_and_cast<IClock*>(clockModule);

        WATCH(scheduleIndex);
    }
    //initialize schedule xml in second stage when clock is initialized
    else if (stage == INITSTAGE_LINK_LAYER) {
        scheduleIndex = 0;
        scheduleXml = par("schedule").xmlValue();
        clock->subscribeTick(this, 0);
    }
}

void ScheduleSwap::handleMessage(cMessage *msg) {
    throw cRuntimeError("cannot handle messages");
}

int ScheduleSwap::numInitStages() const {
    return INITSTAGE_LINK_LAYER + 1;
}

void ScheduleSwap::tick(IClock *clock) {
    Enter_Method("tick()");

    //only act if there are entries and the index points to an entry in range
    if (scheduleIndex<scheduleXml->getChildrenByTagName("entry").size()) {
        cXMLElement* entry = scheduleXml->getChildrenByTagName("entry")[scheduleIndex];
        //If the entry defines a new schedule, apply it
        if(!entry->getChildrenByTagName("schedule").empty()) {
            cXMLElement* newScheduleXml = entry->getFirstChildWithTag("schedule")->getFirstChildWithTag("schedule");
            //TODO check if valid schedule
            if(!par("usedInHost").boolValue()) {
                cModule* switchModule = this->getModuleByPath(par("switchModule"));
                if (switchModule != nullptr && !entry->getChildrenByTagName("schedule").empty()) {
                    //find every gateController module in this switch
                    for (int i = 0; i < switchModule->gateSize("ethg"); i++) {
                        const char* gateControllerModulesPath = par("gateControllerModules");
                        //TODO calculate array length
                        char gateControllerModulePath[1000];
                        sprintf(gateControllerModulePath, gateControllerModulesPath, i);
                        EV_INFO << getFullPath() << ": Changing switch schedule at " << gateControllerModulePath << endl;
                        cModule* module = this->getModuleByPath(gateControllerModulePath);
                        if (module != nullptr) {
                            //try to apply the schedule
                            GateController* gateControllerModule = check_and_cast< GateController*>(module);
                            gateControllerModule->loadScheduleOrDefault( newScheduleXml );
                        }
                        else {
                            EV_ERROR << getFullPath() << ": Parent module (gateController) not found" << endl;
                        }

                    }
                }
                cModule* filteringDatabaseModule = this->getModuleByPath(par("filteringDatabaseModule"));
                if(filteringDatabaseModule != nullptr && !entry->getChildrenByTagName("routing").empty()) {
                    cXMLElement* newRouting = entry->getFirstChildWithTag("routing")->getFirstChildWithTag("filteringDatabases");
                    FilteringDatabase* routingModule = check_and_cast< FilteringDatabase*>(filteringDatabaseModule);
                    routingModule->loadDatabase( newRouting , atoi(newScheduleXml->getFirstChildWithTag("cycle")->getNodeValue()));
                }
            }
            else {
                const char* tsnGenPath = par("tsnGenModule");
                cModule* module = this->getModuleByPath(tsnGenPath);
                if(module !=nullptr) {
                    EV_INFO << getFullPath() << ": Changing host schedule at " << tsnGenPath << endl;
                    VlanEtherTrafGenSched* tsnModule = check_and_cast<VlanEtherTrafGenSched*>(module);
                    tsnModule->loadScheduleOrDefault(newScheduleXml);
                }
                else {
                    EV_ERROR << getFullPath()<<": Parent module (host) not found" << endl;
                }

            }
        }
        scheduleIndex += 1;
        if (scheduleIndex<scheduleXml->getChildrenByTagName("entry").size()) {
            clock->subscribeTick(this, atoi(entry->getFirstChildWithTag("length")->getNodeValue()));
            EV_INFO << getFullPath()<<": Next schedule swap subscribed at " << entry->getFirstChildWithTag("length")->getNodeValue() << "." << endl;
        }
        else {
            EV_INFO <<getFullPath()<< ": Last schedule swap was executed (" << scheduleXml->getChildrenByTagName("entry").size() <<" entries)." << endl;
        }

    }
}
}
