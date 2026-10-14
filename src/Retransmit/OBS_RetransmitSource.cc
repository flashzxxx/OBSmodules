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

#include "OBS_RetransmitSource.h"

#include <algorithm>

#include "IPvXAddressResolver.h"
#include "UDPControlInfo_m.h"

Define_Module(OBS_RetransmitSource);

simsignal_t OBS_RetransmitSource::requestCompletedSignal = registerSignal("requestCompleted");

OBS_RetransmitSource::OBS_RetransmitSource(){
   sendTimer = NULL;
}

OBS_RetransmitSource::~OBS_RetransmitSource(){
   cancelAndDelete(sendTimer);
   // Any request still waiting for its acknowledgement owns a scheduled timer. The simulation
   // can end mid-flight (the last requests of a run never complete), so these must be released
   // here or the module would leak self-messages.
   std::map<int,RequestState>::iterator it;
   for(it=outstanding.begin();it!=outstanding.end();++it){
      cancelAndDelete(it->second.timer);
   }
}

void OBS_RetransmitSource::initialize(int stage){
   ApplicationBase::initialize(stage);

   if(stage == 0){
      localPort = par("localPort");
      destPort = par("destPort");
      messageLength = par("messageLength").longValue();
      startTime = par("startTime").doubleValue();
      stopTime = par("stopTime").doubleValue();
      if(stopTime >= SIMTIME_ZERO && stopTime < startTime) error("Invalid startTime/stopTime parameters");

      // The timeout has to clear the round trip of the longest path, otherwise every request is
      // retransmitted while its first copy is still in flight and the arm measures its own
      // impatience rather than the network's loss.
      retransmitTimeout = par("retransmitTimeout");
      if(retransmitTimeout <= 0) error("retransmitTimeout must be positive");
      maxRetransmissions = par("maxRetransmissions");
      if(maxRetransmissions < 0) error("maxRetransmissions must not be negative");

      warmupPeriod = simulation.getWarmupPeriod();

      nextSeq = 0;
      requestsSent = 0;
      retransmissionsSent = 0;
      requestsAcked = 0;
      requestsAbandoned = 0;
      duplicateAcks = 0;

      WATCH(requestsSent);
      WATCH(retransmissionsSent);
      WATCH(requestsAcked);
      WATCH(requestsAbandoned);

      sendTimer = new cMessage("sendTimer");
   }
}

// A self-message that is not sendTimer is the retransmission timer of one request, and its
// kind carries that request's sequence number. Using the kind instead of a second map keeps
// the pairing obvious and cannot collide with INET: these messages never leave the module.
void OBS_RetransmitSource::handleMessageWhenUp(cMessage *msg){
   if(msg == sendTimer){
      sendRequest();
      simtime_t next = simTime() + par("sendInterval").doubleValue();
      if(stopTime < SIMTIME_ZERO || next < stopTime){
         scheduleAt(next,sendTimer);
      }
      return;
   }

   if(msg->isSelfMessage()){
      processRetransmitTimer(msg);
      return;
   }

   if(msg->getKind() == UDP_I_DATA){
      processAck(PK(msg));
   } else if (msg->getKind() == UDP_I_ERROR){
      EV << "Ignoring UDP error report\n";
      delete msg;
   } else {
      error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
   }
}

IPvXAddress OBS_RetransmitSource::chooseDestAddr(){
   int k = intrand(destAddresses.size());
   return destAddresses[k];
}

// One packet of the offered stream, plus its retransmission timer.
void OBS_RetransmitSource::sendRequest(){
   if(destAddresses.empty()) return;

   int seq = nextSeq++;
   OBS_RetransmitPacket *request = new OBS_RetransmitPacket("RetransmitRequest");
   request->setSeq(seq);
   request->setByteLength(messageLength);

   RequestState state;
   state.seq = seq;
   state.firstSent = simTime();
   state.transmissions = 1;
   state.destAddr = chooseDestAddr();
   state.destPort = destPort;

   cMessage *timer = new cMessage("retransmitTimer");
   timer->setKind(seq);
   state.timer = timer;
   outstanding[seq] = state;

   if(state.firstSent >= warmupPeriod) requestsSent++;

   socket.sendTo(request,state.destAddr,state.destPort);
   scheduleAt(simTime() + retransmitTimeout,timer);
}

// The retry cap counts retransmissions, so a request is given up on when its transmission
// count has already reached 1 + maxRetransmissions.
void OBS_RetransmitSource::processRetransmitTimer(cMessage *timer){
   int seq = timer->getKind();
   std::map<int,RequestState>::iterator it = outstanding.find(seq);
   if(it == outstanding.end()){
      // The acknowledgement arrived and should have cancelled this timer; nothing to do.
      delete timer;
      return;
   }

   RequestState &state = it->second;
   if(state.transmissions > maxRetransmissions){
      if(simTime() >= warmupPeriod) requestsAbandoned++;
      delete timer;
      outstanding.erase(it);
      return;
   }

   OBS_RetransmitPacket *request = new OBS_RetransmitPacket("RetransmitRequest");
   request->setSeq(seq);
   request->setByteLength(messageLength);
   socket.sendTo(request,state.destAddr,state.destPort);

   state.transmissions++;
   if(simTime() >= warmupPeriod) retransmissionsSent++;
   scheduleAt(simTime() + retransmitTimeout,timer);
}

void OBS_RetransmitSource::processAck(cPacket *packet){
   OBS_RetransmitPacket *ack = check_and_cast<OBS_RetransmitPacket*>(packet);
   int seq = ack->getSeq();

   std::map<int,RequestState>::iterator it = outstanding.find(seq);
   if(it == outstanding.end()){
      // Late or duplicated acknowledgement: the request was already completed or given up.
      duplicateAcks++;
      delete ack;
      return;
   }

   RequestState &state = it->second;
   if(state.firstSent >= warmupPeriod){
      requestsAcked++;
      // Completion delay is recorded as a framework statistic (see the NED declaration of
      // completionDelay), which gives count, mean, min and max from the same series.
      emit(requestCompletedSignal,SIMTIME_DBL(simTime() - state.firstSent));
   }

   cancelAndDelete(state.timer);
   outstanding.erase(it);
   delete ack;
}

void OBS_RetransmitSource::finish(){
   recordScalar("requestsSent",requestsSent);
   recordScalar("retransmissionsSent",retransmissionsSent);
   recordScalar("transmissionsTotal",requestsSent + retransmissionsSent);
   recordScalar("requestsAcked",requestsAcked);
   recordScalar("requestsAbandoned",requestsAbandoned);
   recordScalar("requestsOutstanding",(long)outstanding.size());
   recordScalar("duplicateAcks",duplicateAcks);

   // Delivery ratio is the primary metric of experiment R: what fraction of the offered
   // requests is eventually delivered, once retransmission has had its chance. Its complement
   // for the FDL arm is the burst loss rate, which is why both arms report against the same
   // measured channel load.
   if(requestsSent > 0) recordScalar("deliveryRatio",(double)requestsAcked/(double)requestsSent);

   // Goodput uses the same window convention as OBS_CoreControlLogic's utilization scalars
   // (simTime() minus warmup), so the two arms are directly comparable.
   simtime_t window = simTime() - warmupPeriod;
   if(window > 0){
      recordScalar("goodputBps",(double)requestsAcked * (double)messageLength * 8.0 / SIMTIME_DBL(window));
   }

   ApplicationBase::finish();
}

bool OBS_RetransmitSource::handleNodeStart(IDoneCallback *doneCallback){
   socket.setOutputGate(gate("udpOut"));
   socket.bind(localPort);

   const char *destAddrs = par("destAddresses");
   cStringTokenizer tokenizer(destAddrs);
   const char *token;
   while((token = tokenizer.nextToken()) != NULL){
      IPvXAddress result;
      IPvXAddressResolver().tryResolve(token,result);
      if(result.isUnspecified()) EV << "cannot resolve destination address: " << token << endl;
      else destAddresses.push_back(result);
   }

   if(!destAddresses.empty()){
      simtime_t start = std::max(startTime,simTime());
      if(stopTime < SIMTIME_ZERO || start < stopTime) scheduleAt(start,sendTimer);
   }
   return true;
}

bool OBS_RetransmitSource::handleNodeShutdown(IDoneCallback *doneCallback){
   if(sendTimer) cancelEvent(sendTimer);
   return true;
}

void OBS_RetransmitSource::handleNodeCrash(){
   if(sendTimer) cancelEvent(sendTimer);
}
