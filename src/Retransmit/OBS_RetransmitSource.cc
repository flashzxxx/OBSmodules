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
#include <cstring>

#include "IPvXAddressResolver.h"
#include "UDPControlInfo_m.h"

Define_Module(OBS_RetransmitSource);

simsignal_t OBS_RetransmitSource::requestCompletedSignal = registerSignal("requestCompleted");
simsignal_t OBS_RetransmitSource::windowDeferredSignal = registerSignal("windowDeferred");

OBS_RetransmitSource::OBS_RetransmitSource(){
   sendTimer = NULL;
   // pendingSend is false until the send timer finds the window full; the window itself needs no
   // message of its own, a deferred send is the send timer rescheduled earlier
   pendingSend = false;
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

      // The window bounds the requests that are simultaneously unacknowledged. 0 disables it,
      // which is the open-loop arm whose results already exist; every config that does not ask
      // for a window therefore behaves exactly as it did before this parameter was added.
      windowSize = par("windowSize");
      if(windowSize < 0) error("windowSize must not be negative (0 = unlimited)");
      retryInterval = par("retryInterval");
      if(retryInterval <= 0) error("retryInterval must be positive");

      warmupPeriod = simulation.getWarmupPeriod();

      nextSeq = 0;
      requestsSent = 0;
      retransmissionsSent = 0;
      requestsAcked = 0;
      requestsAbandoned = 0;
      duplicateAcks = 0;
      // Deferral statistics. They are all zero while windowSize = 0.
      windowDefers = 0;
      retransmitDefers = 0;
      pendingSends = 0;
      requestsOutstandingMax = 0;
      pendingSendTimeSum = SIMTIME_ZERO;
      pendingSendTimePeak = SIMTIME_ZERO;
      pendingSendTimeFloor = SIMTIME_ZERO;
      pendingSince = SIMTIME_ZERO;
      pendingSend = false;

      WATCH(requestsSent);
      WATCH(retransmissionsSent);
      WATCH(requestsAcked);
      WATCH(requestsAbandoned);
      WATCH(windowDefers);
      WATCH(retransmitDefers);
      WATCH(pendingSends);
      WATCH(requestsOutstandingMax);
      WATCH(pendingSend);

      // Guard: initialize() can run more than once (the init stages), and the timer must not be
      // replaced or the scheduled event would be lost.
      if(sendTimer == NULL) sendTimer = new cMessage("sendTimer");
   }
}

// The send timer is rescheduled earlier than its interval while it waits for a free window slot;
// the retransmission timer of a request is re-armed with retryInterval for the same reason. Both
// survive on the sender side only - no packet is emitted until a slot is free - so a deferred
// event is the sender throttling itself, not a new retransmission and not a loss.
void OBS_RetransmitSource::handleMessageWhenUp(cMessage *msg){
   if(msg == sendTimer){
      // The event that arrives here is a scheduled request, whether it is the periodic one or a
      // repeat while the window was full; pendingSend is what turns it into a request.
      if(stopTime >= SIMTIME_ZERO && simTime() >= stopTime){
         // Stop offering, and emit nothing: the periodic stream is over even if the event that
         // carried us here was a deferral.
         return;
      }

      if(windowFull()){
         // Defer: come back later, keep the request in hand. The window is what bounds the
         // in-flight count, so a lost retransmission cannot grow into unbounded offered load.
         if(!pendingSend){
            pendingSend = true;
            pendingSince = simTime();
            windowDefers++;
         }
         scheduleAt(simTime() + retryInterval,sendTimer);
         return;
      }

      sendFreshRequest();
      simtime_t next = simTime() + par("sendInterval").doubleValue();
      if(stopTime < SIMTIME_ZERO || next < stopTime){
         scheduleAt(next,sendTimer);
      }
      return;
   }

   if(msg->isSelfMessage()){
      if(strcmp(msg->getName(),"retransmitTimerDeferred") == 0){
         // A retransmission waiting for a free slot. Re-arm and poll again; its idle period
         // consumes no retransmissions, so deferring cannot turn into extra attempts.
         scheduleAt(simTime() + retryInterval,msg);
         return;
      }
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

// One packet of the offered stream, plus its retransmission timer. Called only when the window
// has room for it.
void OBS_RetransmitSource::sendFreshRequest(){
   if(destAddresses.empty()) return;

   if(pendingSend){
      // The request was due earlier and waited; report how long.
      simtime_t waited = simTime() - pendingSince;
      pendingSends++;
      pendingSendTimeSum += waited;
      if(pendingSendTimePeak == SIMTIME_ZERO || waited > pendingSendTimePeak) pendingSendTimePeak = waited;
      if(pendingSendTimeFloor == SIMTIME_ZERO || waited < pendingSendTimeFloor) pendingSendTimeFloor = waited;
      emit(windowDeferredSignal,SIMTIME_DBL(waited));
   }
   pendingSend = false;

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
   if((long)outstanding.size() > requestsOutstandingMax) requestsOutstandingMax = outstanding.size();

   socket.sendTo(request,state.destAddr,state.destPort);
   // The timer is armed unconditionally, even when maxRetransmissions is 0. Skipping it there
   // looks like a harmless saving, but it is not: the timer would have fired and taken the
   // "retry cap reached" branch, incrementing requestsAbandoned, which a config with
   // maxRetransmissions = 0 can observe (fdl_tests.ini uses it). Keeping the arm preserves the
   // exact behaviour of the open-loop arm, which is what the window parameter must not disturb.
   scheduleAt(simTime() + retransmitTimeout,timer);
}

// One repeat of a request that is already in flight. The completion delay origin stays at the
// request's first transmission, so a retransmission does not restart it.
void OBS_RetransmitSource::sendRetransmission(RequestState& state){
   OBS_RetransmitPacket *request = new OBS_RetransmitPacket("RetransmitRequest");
   request->setSeq(state.seq);
   request->setByteLength(messageLength);
   socket.sendTo(request,state.destAddr,state.destPort);

   state.transmissions++;
   if(simTime() >= warmupPeriod) retransmissionsSent++;
   scheduleAt(simTime() + retransmitTimeout,state.timer);
}

// A window slot was just freed by an acknowledgement or an abandonment. If a fresh request is
// waiting, send it now; otherwise the send timer is already counting down. Deferred
// retransmissions need no action here: they poll the slot themselves.
void OBS_RetransmitSource::onWindowSlotFreed(){
   if(!pendingSend || windowFull()){
      return;
   }
   if(sendTimer != NULL && sendTimer->isScheduled()){
      cancelEvent(sendTimer);
   }
   sendFreshRequest();
   simtime_t next = simTime() + par("sendInterval").doubleValue();
   if(stopTime < SIMTIME_ZERO || next < stopTime){
      scheduleAt(next,sendTimer);
   }
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
   // The retry cap is checked first on purpose: a request that has exhausted its retries is
   // abandoned here rather than parked in the window, so a full window cannot keep it alive
   // forever. With maxRetransmissions = 0 this branch is what a request hits on its first timer.
   if(state.transmissions > maxRetransmissions){
      if(simTime() >= warmupPeriod) requestsAbandoned++;
      delete timer;
      outstanding.erase(it);
      onWindowSlotFreed();
      return;
   }

   if(windowFull()){
      // A retransmission occupies a window slot - it is another transmission of a request that
      // is already in flight. If the window is full, retry after retryInterval instead of
      // putting more traffic on a network that is already behind. Marked by name so the poll is
      // not mistaken for another attempt.
      timer->setName("retransmitTimerDeferred");
      retransmitDefers++;
      scheduleAt(simTime() + retryInterval,timer);
      return;
   }

   sendRetransmission(state);
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

   // The acknowledgement freed a window slot.
   onWindowSlotFreed();
}

void OBS_RetransmitSource::finish(){
   recordScalar("requestsSent",requestsSent);
   recordScalar("retransmissionsSent",retransmissionsSent);
   recordScalar("transmissionsTotal",requestsSent + retransmissionsSent);
   recordScalar("requestsAcked",requestsAcked);
   recordScalar("requestsAbandoned",requestsAbandoned);
   recordScalar("requestsOutstanding",(long)outstanding.size());
   recordScalar("duplicateAcks",duplicateAcks);

   // Window evidence. With windowSize = 0 all of these stay 0 and nothing about the open-loop
   // arm changes; with a window set, requestsOutstandingMax must never exceed windowSize, which
   // is the invariant the acceptance gate checks.
   //
   // The parameter itself is recorded too. It is a plain NED parameter, so OMNeT++ records it
   // only when a config happens to iterate over it - and the per-window configs deliberately set
   // it as a literal, which left the results with no record of which window produced them. The
   // only trace was the high-water mark, and a reader had to infer the window from it. This
   // scalar removes that trap; it costs one value per source.
   recordScalar("windowSize",windowSize);
   recordScalar("windowDefers",windowDefers);
   recordScalar("retransmitDefers",retransmitDefers);
   recordScalar("pendingSends",pendingSends);
   recordScalar("requestsOutstandingMax",requestsOutstandingMax);
   if(pendingSends > 0){
      recordScalar("pendingSendTimeMean",SIMTIME_DBL(pendingSendTimeSum) / (double)pendingSends);
      recordScalar("pendingSendTimeMax",SIMTIME_DBL(pendingSendTimePeak));
      recordScalar("pendingSendTimeMin",SIMTIME_DBL(pendingSendTimeFloor));
   }

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
