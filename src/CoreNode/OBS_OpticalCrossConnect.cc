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

#include "OBS_OpticalCrossConnect.h"

Define_Module(OBS_OpticalCrossConnect);

OBS_OpticalCrossConnect::OBS_OpticalCrossConnect(){
   // initialize() allocates both tables; null-init so the destructor is safe if
   // initialize() never runs (e.g. the network fails to build elsewhere).
   schedulingTable = NULL;
   outputOwner = NULL;
}

OBS_OpticalCrossConnect::~OBS_OpticalCrossConnect(){
	free(schedulingTable);
	free(outputOwner);
}

void OBS_OpticalCrossConnect::initialize(){
   schedulingTable = (int*)calloc(gateSize("in"),sizeof(int));
   outputOwner = (int*)calloc(gateSize("out"),sizeof(int));
   oxcDropCount = 0;
   int i;
   for(i=0;i<gateSize("in");i++){
      schedulingTable[i] = -1;
      WATCH(schedulingTable[i]);
   }
   for(i=0;i<gateSize("out");i++){
      outputOwner[i] = -1;
      WATCH(outputOwner[i]);
   }
   WATCH(oxcDropCount);
}

void OBS_OpticalCrossConnect::handleMessage(cMessage *msg){
   cGate *gate = msg->getArrivalGate();

   if(schedulingTable[gate->getIndex()] == -1){
      EV << "[OXC] Dropping burst on unprogrammed inGate=" << gate->getIndex()
         << " at t=" << simTime() << endl;
      oxcDropCount++;
      delete msg;
   }
   else
      send(msg,"out",schedulingTable[gate->getIndex()]);
}

void OBS_OpticalCrossConnect::setGate(int inGate,int outGate){
   Enter_Method("programming gate connection %d -> %d",inGate,outGate);

   if(schedulingTable[inGate] != -1) opp_error("Attempting to schedule an already scheduled input channel. Channel id: %d",inGate);
   if(outGate < 0 || outGate >= gateSize("out")) opp_error("OXC output gate %d is out of range", outGate);
   if(outputOwner[outGate] != -1 && outputOwner[outGate] != inGate){
      opp_error("Attempting to connect inGate %d -> outGate %d, but outGate %d is already occupied by inGate %d",
                inGate, outGate, outGate, outputOwner[outGate]);
   }
   schedulingTable[inGate] = outGate;
   outputOwner[outGate] = inGate;
}

void OBS_OpticalCrossConnect::unsetGate(int inGate){
   Enter_Method("unprogramming gate %d",inGate);
   int outGate = schedulingTable[inGate];
   if(outGate >= 0 && outGate < gateSize("out") && outputOwner[outGate] == inGate){
      outputOwner[outGate] = -1;
   }
   schedulingTable[inGate] = -1;
}

void OBS_OpticalCrossConnect::finish(){
   recordScalar("oxcBurstDropped", oxcDropCount);
}
