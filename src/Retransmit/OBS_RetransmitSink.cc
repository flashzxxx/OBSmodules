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

#include "OBS_RetransmitSink.h"

#include "UDPControlInfo_m.h"

Define_Module(OBS_RetransmitSink);

OBS_RetransmitSink::OBS_RetransmitSink(){
}

OBS_RetransmitSink::~OBS_RetransmitSink(){
}

void OBS_RetransmitSink::initialize(int stage){
   ApplicationBase::initialize(stage);

   if(stage == 0){
      localPort = par("localPort");
      ackLength = par("ackLength").longValue();
      if(ackLength <= 0) error("ackLength must be positive");

      warmupPeriod = simulation.getWarmupPeriod();

      requestsReceived = 0;
      acksSent = 0;
      WATCH(requestsReceived);
      WATCH(acksSent);
   }
}

void OBS_RetransmitSink::handleMessageWhenUp(cMessage *msg){
   if(msg->getKind() == UDP_I_DATA){
      OBS_RetransmitPacket *request = check_and_cast<OBS_RetransmitPacket*>(PK(msg));

      // Where the request came from. The echo has to reach that exact socket, because the
      // source keeps its outstanding requests indexed by sequence number.
      UDPDataIndication *ctrl = check_and_cast<UDPDataIndication*>(request->removeControlInfo());
      IPvXAddress srcAddress = ctrl->getSrcAddr();
      int srcPort = ctrl->getSrcPort();
      delete ctrl;

      if(simTime() >= warmupPeriod) requestsReceived++;

      OBS_RetransmitPacket *ack = new OBS_RetransmitPacket("RetransmitAck");
      ack->setSeq(request->getSeq());
      ack->setByteLength(ackLength);
      socket.sendTo(ack,srcAddress,srcPort);
      acksSent++;

      delete request;
   } else if (msg->getKind() == UDP_I_ERROR){
      EV << "Ignoring UDP error report\n";
      delete msg;
   } else {
      error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
   }
}

void OBS_RetransmitSink::finish(){
   recordScalar("requestsReceived",requestsReceived);
   recordScalar("acksSent",acksSent);
   ApplicationBase::finish();
}

bool OBS_RetransmitSink::handleNodeStart(IDoneCallback *doneCallback){
   socket.setOutputGate(gate("udpOut"));
   socket.bind(localPort);
   return true;
}

bool OBS_RetransmitSink::handleNodeShutdown(IDoneCallback *doneCallback){
   return true;
}

void OBS_RetransmitSink::handleNodeCrash(){
}
