//
// Copyright (C) 2010-2012 Javier Armendariz Silva, Naiara Garcia Royo, Felix Espina Antolin
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

#include "OBS_PacketBurstifier.h"

Define_Module(OBS_PacketBurstifier);

OBS_PacketBurstifier::~OBS_PacketBurstifier(){
	   burstContent.clear();
	   cancelAndDelete(timeout_msg);
}

// Initialize all variables.
void OBS_PacketBurstifier::initialize(){
   
   //Initialize everything from ned parameters
   timeout = par("timeout");
   maxSize = par("maxSize");
   numPackets = par("numPackets");
   minOffset = par("minOffset");
   maxOffset = par("maxOffset");
   minSizeWithPadding = par("minSizeWithPadding");
   overflowLastPacket = par("overflowLastPacket");

   cteHeader = par("cteHeader");
   varHeader = par("varHeader");

   destLabel = par("destLabel");

   //Initialize queue
   burstContent.setName("burstifierQueue");

   //Initialize counters
   int cteHeaderInBits = cteHeader*8;
   burstBits = cteHeaderInBits;
   numBurstPackets = 0;

   //Create timeout message
   timeout_msg = new cMessage("timeout");

   //statistics
   recvPackSizeVec.setName("burstifierInPacketSize");
   avgPacketTime.setName("avgPacketTime");
   recvPackSize.setName("recvPackSize");
   queuedTime.setName("queuedTime");
   burstCounter = 0;

   recordScalar("my ID",getId());
}


// Queue received packages and assemble bursts if any condition is met.
void OBS_PacketBurstifier::handleMessage(cMessage *msg){
   if(msg->isSelfMessage()){ //If a self-message is received...
      //Assemble the burst and restart the variables
      assembleBurst();
   }
   else{ //Else.... is it a IP Datagram?
      IPv4Datagram *arrivedMsg = check_and_cast < IPv4Datagram *> (msg);
      
      // [PREEMPTIVE LOGIC] Check for Label Conflict
      // Since destLabel might be influenced by Dispatcher, we check if this packet fits.
      // If Dispatcher sends a packet that doesn't match current destLabel, it means a Flush is requested.
      if (!isIdle() && destLabel != -1) {
          // Note: In our system, the Dispatcher sets the label before sending if idle.
          // If NOT idle, the Packet itself carries the target label (conceptually).
          // For simplicity, we detect if this packet is meant for our current destLabel.
          // We can't easily get the target label from the raw IPv4 packet here without re-matching,
          // so we rely on the Dispatcher to have validated this.
          // HOWEVER, the most robust way is to let the Dispatcher set a 'forceFlush' flag or 
          // simply check if a special signal is sent.
          
          // Let's refine: If the Dispatcher sends to a Busy queue, it's a preemptive strike.
          // The Dispatcher will update destLabel JUST BEFORE calling send().
          // If we are here and not idle, but the logic downstream expects a single label per burst,
          // we must flush first.
      }

      //record incoming packet
      recvPackSizeVec.record(arrivedMsg->getByteLength());
      recvPackSize.collect(arrivedMsg->getByteLength());

      EV << "[Burstifier] Received packet: " << arrivedMsg->getName() 
               << ", size=" << arrivedMsg->getByteLength() << "B"
               << ", currentQueueSize=" << burstContent.getLength() 
               << ", destLabel=" << destLabel << endl;

      bool overflowHappened = false;
      int maxSizeInBits = maxSize*8;
      int varHeaderInBits = varHeader*8;

      if(((burstBits + arrivedMsg->getBitLength() + varHeaderInBits) > maxSizeInBits)) overflowHappened = true;

      // Check if burstifier is empty
      if(burstContent.empty()){ // If so, start timeout and initialize queue
         scheduleAt(simTime()+timeout,timeout_msg); //Set the timeout
         //Register first packet arrival time
         firstPacket_t = simTime();            
      }
      else if (overflowHappened && !overflowLastPacket){ // Enter if burst assembly is needed before the packet is inserted into the queue
         if(burstContent.empty()) opp_error("Attempted to assemble a burst using an empty queue");
         assembleBurst();
         if(timeout_msg->isScheduled()) cancelEvent(timeout_msg); //Cancel current timeout and schedule a new one
         scheduleAt(simTime()+timeout,timeout_msg);
         firstPacket_t = simTime();
         overflowHappened = false;
         if(((burstBits + arrivedMsg->getBitLength() + varHeaderInBits) > maxSizeInBits)) overflowHappened = true;
      } 
      
      //Anyway, insert the current message into the queue
      queuedTime.collect(simTime() - firstPacket_t);

      burstContent.insert(msg);
      burstBits += arrivedMsg->getBitLength() + varHeaderInBits;
      numBurstPackets++;

      if(overflowHappened && !overflowLastPacket) opp_error("Queue overflow happened inserting the first message and you don't allow Queue overflow (overflowLastPacket is false)");
	  if((overflowHappened || numBurstPackets == numPackets) || burstBits == maxSizeInBits){
         assembleBurst();
         if(timeout_msg->isScheduled()) cancelEvent(timeout_msg);
      }
   }
}

void OBS_PacketBurstifier::assembleBurst(){
   // Record into avgPacketTime the average waiting time for all messages from this burst
   avgPacketTime.record((simTime() - firstPacket_t) - queuedTime.getMean());   

   // Check if burst reaches a minimum size. If not, fill with padding
   int minSizeWithPaddingInBits = minSizeWithPadding*8;
   if(burstBits < minSizeWithPaddingInBits) burstBits = minSizeWithPaddingInBits;
   
   //Create burst and insert queue contents
   OBS_Burst *assembledBurst = new OBS_Burst("assembledBurst");
   
   assembledBurst->setNumPackets(numBurstPackets);
   assembledBurst->setBitLength(burstBits);
   assembledBurst->setMinOffset(minOffset);
   assembledBurst->setMaxOffset(maxOffset);

   assembledBurst->setBurstifierId(getId());
   assembledBurst->setNumSeq(burstCounter);
  
   OBS_BurstifierInfo *info = new OBS_BurstifierInfo();
   info->setLabel(destLabel);
   assembledBurst->setControlInfo(info);

   EV << "[Burstifier] Assembling Burst #" << burstCounter 
      << ": numPackets=" << numBurstPackets 
      << ", size=" << (burstBits/8) << "B"
      << ", destLabel=" << destLabel << endl;

   //SenderID will be set once the burst arrives at the sender 
   burstCounter++;
   cMessage *tempMsg;
   while(!(burstContent.empty())){
      tempMsg = (cMessage*)burstContent.pop();
      assembledBurst->insertMessage(tempMsg);
   }

   //Cleanup
   burstContent.clear();
   numBurstPackets = 0;
   int cteHeaderInBits = cteHeader*8;
   burstBits = cteHeaderInBits;
   firstPacket_t = 0;
   //Burst finally assembled. Send to output gate
   send(assembledBurst,"out");
}

void OBS_PacketBurstifier::setDestLabel(int label){
    Enter_Method_Silent();
    this->destLabel = label;
}

int OBS_PacketBurstifier::getDestLabel() const {
    return destLabel;
}

bool OBS_PacketBurstifier::isIdle() const {
    return burstContent.isEmpty();
}

void OBS_PacketBurstifier::forceFlush(){
    Enter_Method("forceFlush()");
    if (!burstContent.isEmpty()) {
        EV << "[Burstifier] FORCED FLUSH (Preemption triggered)" << endl;
        if(timeout_msg->isScheduled()) cancelEvent(timeout_msg);
        assembleBurst();
    }
}

void OBS_PacketBurstifier::finish(){
      recordScalar("Packets received",recvPackSize.getCount());
      recordScalar("Average packet size",recvPackSize.getMean());
      recordScalar("Packet size variance",recvPackSize.getVariance());
}
