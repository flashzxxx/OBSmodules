//
// Copyright (C) 2010-2012 Javier Armendariz Silva, Naiara Garcia Royo
// Copyright (C) 2010-2012 Universidad Publica de Navarra
//
// This file is part of OBSModules.
//
// OBSModules is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// OBSModules is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with OBSModules.  If not, see <http://www.gnu.org/licenses/>.
//

#include "OBS_PacketDispatcher.h"
#include "OBS_PacketBurstifier.h"

Define_Module(OBS_PacketDispatcher);

OBS_PacketDispatcher::~OBS_PacketDispatcher(){
    if (numQueues != 0){
        delete[] burstifiers;
        delete[] lastAccessTimes;
        ruleList.clear();
    }
}

// Open dispatcherRules file, read all rules and create DispatcherRule objects for each one.
void OBS_PacketDispatcher::initialize(){

   numQueues = par("numQueues");

   droppedPacket = 0;
   WATCH(droppedPacket);

   //initialize the statistic variables
   recvPackSizeVec.setName("recvPacketSize");
   recvPackSize.setName("recvPackSize");
   //end of initialization

    if (numQueues != 0){
        burstifiers = new OBS_PacketBurstifier*[numQueues];
        lastAccessTimes = new simtime_t[numQueues];
        
        for(int j=0; j<numQueues; j++){
            char name[32];
            sprintf(name, "packetBurstifier[%d]", j);
            burstifiers[j] = check_and_cast<OBS_PacketBurstifier*>(getParentModule()->getSubmodule("packetBurstifier", j));
            lastAccessTimes[j] = 0; // Initialize with 0
        }

	   //read one line at a time and create the associated rule for every queue
	   char *line = (char*)calloc(1500,sizeof(char)); 
	   const char* rulesFile = par("dispatcherRules");

	   if(strlen(rulesFile) == 0){
		   opp_error("Rules file not defined");
	   }

	   FILE *ruleFile = fopen(rulesFile,"r");
	 
	   if(ruleFile != NULL){
		  while(fgets(line,1500,ruleFile) != NULL){
		     if(strcmp(line,"\n") != 0 && line[0] != '#'){ 
		        ruleList.push_back(OBS_DispatcherRule(line));
		     }
		  }
	   }
	   else{
		   opp_error("Cannot open rules file %s", rulesFile);
	   }

	   fclose(ruleFile);
	   free(line);
   }
}

// When a packet arrives, it is compared to all rules. If it match to any rule, send it to the corresponding output gate. If not, discard it.
void OBS_PacketDispatcher::handleMessage(cMessage *msg){
   int i;
   
   cPacket *pkt = check_and_cast<cPacket *> (msg);

   // Register incoming packet
   recvPackSizeVec.record(pkt->getByteLength());
   recvPackSize.collect(pkt->getByteLength());

   // 1. Find the target label for this packet
   int targetLabel = -1;
   for(size_t r=0; r < ruleList.size(); r++){
       if(ruleList[r].match(msg)){
           targetLabel = ruleList[r].getDestLabel();
           break;
       }
   }

   if(targetLabel == -1){
       EV_WARN << "No matching rule for packet " << pkt->getName() << "! Dropping." << endl;
       delete msg;
       droppedPacket++;
       return;
   }

   // 2. Find a queue for this label using priority-based selection
   int selectedQueue = -1;
   
   EV << "[Dispatcher] Looking for queue for Label " << targetLabel << ":" << endl;
   
   // Log all queue states
   for(i=0; i < numQueues; i++){
       EV << "  Queue[" << i << "]: idle=" << (burstifiers[i]->isIdle() ? "yes" : "no")
          << ", destLabel=" << burstifiers[i]->getDestLabel() << endl;
   }
   
   // Priority 1: Find a BUSY queue already assigned to this label
   for(i=0; i < numQueues; i++){
       if(!burstifiers[i]->isIdle() && burstifiers[i]->getDestLabel() == targetLabel){
           selectedQueue = i;
           EV << "  -> [Priority1] Found BUSY queue with matching Label: Queue[" << i << "]" << endl;
           break;
       }
   }

   // Priority 2: Find an IDLE queue that was previously assigned to this label (reuse)
   if(selectedQueue == -1){
       for(i=0; i < numQueues; i++){
           if(burstifiers[i]->isIdle() && burstifiers[i]->getDestLabel() == targetLabel){
               selectedQueue = i;
               EV << "  -> [Priority2] Reusing IDLE queue with matching Label: Queue[" << i << "]" << endl;
               break;
           }
       }
   }

   // Priority 3: Find any IDLE queue and assign the new label
   if(selectedQueue == -1){
       for(i=0; i < numQueues; i++){
           if(burstifiers[i]->isIdle()){
               selectedQueue = i;
               burstifiers[i]->setDestLabel(targetLabel);
               EV << "  -> [Priority3] Assigned Label " << targetLabel << " to new IDLE Queue[" << i << "]" << endl;
               break;
           }
       }
   }

   // Priority 4: LRU Preemption (With Force Flush)
   // If all queues are busy with OTHER labels, pick the one that was accessed longest ago.
   if(selectedQueue == -1){
       EV << "  -> [Priority4] No idle queues! Initiating LRU Preemption." << endl;
       int victim = 0;
       simtime_t minTime = lastAccessTimes[0];
       
       for(i=1; i < numQueues; i++){
           if(lastAccessTimes[i] < minTime){
               minTime = lastAccessTimes[i];
               victim = i;
           }
       }
       
       selectedQueue = victim;
       EV << "  -> [Priority4] Victim selected: Queue[" << victim 
          << "] (Last access: " << minTime << "). Forcing Label: " << targetLabel << endl;
          
       // NOTE: Before changing the label, we MUST flush the existing contents 
       // to avoid mixing packets of different destinations.
       burstifiers[selectedQueue]->forceFlush();
       burstifiers[selectedQueue]->setDestLabel(targetLabel);
   }

   // 3. Send and Update Activity
   if(selectedQueue != -1){
       EV << "Matched Label " << targetLabel << ". Sending to out[" << selectedQueue << "]" << endl;
       lastAccessTimes[selectedQueue] = simTime(); // Update LRU timer
       send(msg, "out", selectedQueue);
   }
   else{
       // This should theoretically be unreachable now due to LRU
       EV_WARN << "ALL DISPATCH ATTEMPTS FAILED for Label " << targetLabel << endl;
       delete msg;
       droppedPacket++;
   }
}

void OBS_PacketDispatcher::finish(){
  recordScalar("Packets received",recvPackSize.getCount());
  recordScalar("Average packet size",recvPackSize.getMean());
  recordScalar("Packet size variance",recvPackSize.getVariance());
  recordScalar("Dropped Packets",droppedPacket);
}
