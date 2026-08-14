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
   dispatchMode = par("dispatchMode");
   rrCounter = 0;

   droppedPacket = 0;
   p1Hits = 0;
   p2Hits = 0;
   p3Hits = 0;
   p4Hits = 0;
   forceFlushCount = 0;
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

   EV << "[Dispatcher] Initialized with dispatchMode=" << dispatchMode
      << " (" << (dispatchMode==0 ? "Dynamic" : dispatchMode==1 ? "NoPreemption" : dispatchMode==2 ? "RoundRobin" : "Static")
      << "), numQueues=" << numQueues << endl;
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
       EV << "No matching rule for packet " << pkt->getName() << "! Dropping." << endl;
       delete msg;
       droppedPacket++;
       return;
   }

   int selectedQueue = -1;

   // ========== Mode 0: Dynamic (P1→P2→P3→P4 with LRU Preemption) ==========
   if(dispatchMode == 0){
       int idleMatchQueue = -1;
       int firstEmptyQueue = -1;
       
       for(i=0; i < numQueues; i++){
          bool isIdle = burstifiers[i]->isIdle();
          int currentLabel = burstifiers[i]->getDestLabel();

          // P1: Busy queue, label matches → add to existing burst
          if(!isIdle && currentLabel == targetLabel){
             selectedQueue = i;
             p1Hits++;
             break;
          }
          // P2: Idle queue, label matches → reuse
          if(isIdle && idleMatchQueue == -1 && currentLabel == targetLabel){
             idleMatchQueue = i;
          }
          // P3: Any idle queue → assign new label  
          if(isIdle && firstEmptyQueue == -1){
             firstEmptyQueue = i;
          }
       }

       if(selectedQueue == -1){
          if(idleMatchQueue != -1){
             selectedQueue = idleMatchQueue;
             p2Hits++;
          }
          else if(firstEmptyQueue != -1){
             selectedQueue = firstEmptyQueue;
             burstifiers[selectedQueue]->setDestLabel(targetLabel);
             p3Hits++;
          }
       }

       // P4: LRU Preemption
       if(selectedQueue == -1){
           p4Hits++;
           int victim = 0;
           simtime_t minTime = lastAccessTimes[0];
           for(i=1; i < numQueues; i++){
               if(lastAccessTimes[i] < minTime){
                   minTime = lastAccessTimes[i];
                   victim = i;
               }
           }
           selectedQueue = victim;
           burstifiers[selectedQueue]->forceFlush();
           forceFlushCount++;
           burstifiers[selectedQueue]->setDestLabel(targetLabel);
       }
   }
   // ========== Mode 1: No-Preemption (P1→P2→P3, drop if full) ==========
   else if(dispatchMode == 1){
       int idleMatchQueue = -1;
       int firstEmptyQueue = -1;
       
       for(i=0; i < numQueues; i++){
          bool isIdle = burstifiers[i]->isIdle();
          int currentLabel = burstifiers[i]->getDestLabel();

          if(!isIdle && currentLabel == targetLabel){
             selectedQueue = i;
             p1Hits++;
             break;
          }
          if(isIdle && idleMatchQueue == -1 && currentLabel == targetLabel){
             idleMatchQueue = i;
          }
          if(isIdle && firstEmptyQueue == -1){
             firstEmptyQueue = i;
          }
       }

       if(selectedQueue == -1){
          if(idleMatchQueue != -1){
             selectedQueue = idleMatchQueue;
             p2Hits++;
          }
          else if(firstEmptyQueue != -1){
             selectedQueue = firstEmptyQueue;
             burstifiers[selectedQueue]->setDestLabel(targetLabel);
             p3Hits++;
          }
       }
       // No P4: if selectedQueue is still -1, packet will be dropped below
   }
   // ========== Mode 2: Round-Robin (ignore labels, rotate queues) ==========
   else if(dispatchMode == 2){
       selectedQueue = rrCounter % numQueues;
       rrCounter++;
       // Force label change so burst goes to correct destination
       if(burstifiers[selectedQueue]->getDestLabel() != targetLabel){
           if(!burstifiers[selectedQueue]->isIdle()){
               burstifiers[selectedQueue]->forceFlush();
               forceFlushCount++;
           }
           burstifiers[selectedQueue]->setDestLabel(targetLabel);
           p3Hits++; // Count as fresh label assignment
       } else {
           p2Hits++; // Count as label reuse
       }
   }
   // ========== Mode 3: Static (label → fixed queue, drop on collision) ==========
   else if(dispatchMode == 3){
       int fixedQueue = (targetLabel - 1) % numQueues; // label 1-10 → queue 0-7
       int currentLabel = burstifiers[fixedQueue]->getDestLabel();
       bool isIdle = burstifiers[fixedQueue]->isIdle();
       
       if(currentLabel == targetLabel){
           selectedQueue = fixedQueue;
           if(isIdle) p2Hits++; else p1Hits++;
       }
       else if(isIdle){
           selectedQueue = fixedQueue;
           burstifiers[selectedQueue]->setDestLabel(targetLabel);
           p3Hits++;
       }
       // else: queue busy with different label → drop (selectedQueue stays -1)
   }

   // 3. Send and Update Activity
   if(selectedQueue != -1){
       lastAccessTimes[selectedQueue] = simTime();
       send(msg, "out", selectedQueue);
   }
   else{
       EV << "[Dispatcher] Mode=" << dispatchMode << " ALL DISPATCH FAILED for Label " << targetLabel << ". Dropping." << endl;
       delete msg;
       droppedPacket++;
   }
}

void OBS_PacketDispatcher::finish(){
  int totalDispatched = recvPackSize.getCount() - droppedPacket;
  recordScalar("Dispatch Mode",dispatchMode);
  recordScalar("Packets received",recvPackSize.getCount());
  recordScalar("Average packet size",recvPackSize.getMean());
  recordScalar("Packet size variance",recvPackSize.getVariance());
  recordScalar("Dropped Packets",droppedPacket);
  recordScalar("Total Dispatched",totalDispatched);
  recordScalar("P1 Hits (Busy Match)",p1Hits);
  recordScalar("P2 Hits (Idle Reuse)",p2Hits);
  recordScalar("P3 Hits (Fresh Idle)",p3Hits);
  recordScalar("P4 Hits (LRU Preemption)",p4Hits);
  recordScalar("Force Flush Count",forceFlushCount);
  if(totalDispatched > 0){
    recordScalar("Drop Rate (%)", 100.0 * droppedPacket / recvPackSize.getCount());
  } else {
    recordScalar("Drop Rate (%)", 0);
  }
}
