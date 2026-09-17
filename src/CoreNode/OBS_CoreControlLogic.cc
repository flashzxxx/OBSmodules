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

#include "OBS_CoreControlLogic.h"

Define_Module(OBS_CoreControlLogic);

OBS_CoreControlLogic::OBS_CoreControlLogic(){
   // initialize() validates tau / fdlOffsetMode / switchReconfigTime before it
   // allocates these counters, so an opp_error there would otherwise reach the
   // destructor with uninitialized pointers. free(NULL) is a no-op.
   recvBurstCounter = NULL;
   schedBurstCounter = NULL;
   portBusyTime = NULL;
   portCarriedBytes = NULL;
}

OBS_CoreControlLogic::~OBS_CoreControlLogic(){
	   free(recvBurstCounter);
	   free(schedBurstCounter);
	   free(portBusyTime);
	   free(portCarriedBytes);
}

void OBS_CoreControlLogic::initialize(){
   dropCounter = 0;
   WATCH(dropCounter);

   guardTime = par("guardTime");
   processingTime = par("BCPProcessingDelay");
   cModule *parent = getParentModule();
   dataRate = par("dataRate");

   if (strcmp(par("reportFile"), "") == 0) {
      data_f = NULL;
   }
   else {
      data_f = fopen(par("reportFile"),"w");
   }

   //IMPORTANT: USE check_and_cast WHEN LINKING WITH ANOTHER SUBMODULE. OTHERWISE WE WILL GET A NULL POINTER ERROR

   //TODO: Module names should not be hard-coded. Modify this when possible.
   // One possible way to do this consist in a loop that searches for all neighbor modules and assigns the object reference depending on the module type (check if the module belongs to OBS_RoutingTable,
   // if not check if it belongs to Horizon Table type, etc...)
   // As long as there are no module arrays, it shouldn't give any problem.
   routingTable = check_and_cast<OBS_CoreRoutingTable*>(parent->getSubmodule("RoutingTable"));
   gatesHorizon = check_and_cast<OBS_CoreOutputHorizon*>(parent->getSubmodule("GatesHorizon"));
   coreInput = check_and_cast<OBS_CoreInput*>((parent->getParentModule())->getSubmodule("Input"));
   coreOutput = check_and_cast<OBS_CoreOutput*>((parent->getParentModule())->getSubmodule("Output"));
   oxc = check_and_cast<OBS_OpticalCrossConnect*>((parent->getParentModule())->getSubmodule("OXC"));

   // Read FDL parameters from CoreNode
   cModule *coreNode = getParentModule()->getParentModule();
   useFDL = coreNode->par("useFDL").boolValue();
   tau = coreNode->par("fdlDelayTime");
   switchReconfigTime = coreNode->par("switchReconfigTime");
   maxFdlLoopsPerBurst = coreNode->par("maxFdlLoopsPerBurst");

   // OBS_CoreNode.ned binds this submodule parameter as `delayTime = fdlDelayTime`,
   // and an ini entry cannot override a NED submodule-block assignment. The two
   // values are therefore equal by construction: this is an invariant assertion
   // that would catch a future NED edit breaking that binding, not a user-input
   // validation. It is deliberately so - a single source of truth for tau avoids
   // the "two knobs that must be kept in sync" footgun. Consequence: no ini
   // config can exercise this branch, so there is no expected-fail test for it.
   cModule *fdlMod = coreNode->getSubmodule("fdl");
   if(fdlMod == NULL) opp_error("CoreNode is missing the fdl submodule");
   simtime_t fdlModuleDelay = fdlMod->par("delayTime");
   if(fdlModuleDelay != tau){
      opp_error("fdlDelayTime (%s) disagrees with fdl.delayTime (%s); check the fdl submodule delayTime binding in OBS_CoreNode.ned", tau.str().c_str(), fdlModuleDelay.str().c_str());
   }

   const char *offsetMode = coreNode->par("fdlOffsetMode").stringValue();
   if(strcmp(offsetMode, "DO") == 0){
      fdlOffsetIsSO = false;
   }else if(strcmp(offsetMode, "SO") == 0){
      fdlOffsetIsSO = true;
   }else{
      opp_error("fdlOffsetMode must be \"DO\" or \"SO\", got \"%s\"", offsetMode);
   }

   if(useFDL){
      if(tau <= 0) opp_error("useFDL=true requires fdlDelayTime > 0 (got %s)", tau.str().c_str());
      if(tau < switchReconfigTime){
         opp_error("useFDL=true: fdlDelayTime %s is shorter than switchReconfigTime %s",
                   tau.str().c_str(), switchReconfigTime.str().c_str());
      }
   }

   warmupPeriod = simulation.getWarmupPeriod();

   // Initialize FDL statistics
   numPorts = coreOutput->par("numPorts");
   fdlUsageCountCounter = 0;
   busyTime = 0.0;
   busyTimeInFlight = 0.0;
   burstLossContentionCounter = 0;
   burstsLoopedOnceCounter = 0;
   burstsLoopedMultipleCounter = 0;
   outgoingOffsetSum = 0.0;
   outgoingOffsetSamples = 0;
   outgoingOffsetMin = -1;

   WATCH(useFDL);
   WATCH(tau);
   WATCH(numPorts);
   WATCH(fdlUsageCountCounter);
   WATCH(busyTime);
   WATCH(burstLossContentionCounter);

   // Register signals
   fdlUsageCountSignal = registerSignal("fdlUsageCount");
   fdlUtilizationSignal = registerSignal("fdlUtilization");
   burstLossContentionSignal = registerSignal("burstLossContention");
   burstLossTotalSignal = registerSignal("burstLossTotal");
   burstNodeDelaySignal = registerSignal("burstNodeDelay");

   //Initialize all statistics
   numInPorts = coreInput->par("numPorts");
   numOutPorts = coreOutput->par("numPorts");
   recvBurstCounter= (int*)calloc(numInPorts,sizeof(int));
   schedBurstCounter = (int*)calloc(numOutPorts,sizeof(int));
   portBusyTime = (simtime_t*)calloc(numOutPorts,sizeof(simtime_t));
   portCarriedBytes = (double*)calloc(numOutPorts,sizeof(double));

   int i=0;
   for(i=0;i<numInPorts;i++){
      recvBurstCounter[i] = 0;
      WATCH(recvBurstCounter[i]);
   }
   for(i=0;i<numOutPorts;i++){
      schedBurstCounter[i] = 0;
      portBusyTime[i] = 0;
      portCarriedBytes[i] = 0.0;
      WATCH(schedBurstCounter[i]);
      WATCH(portBusyTime[i]);
   }
}

// Assume this module input is connected to the OE Converter, so we only receive electrical BCPs
void OBS_CoreControlLogic::handleMessage(cMessage *msg){
   //OXC Programming automessage. If msg has kind = OBS_SCHEDULE_OXC
   if(msg->getKind() == OBS_SCHEDULE_OXC){
      OBS_ControlUnitInfo *info = check_and_cast <OBS_ControlUnitInfo*>(msg);
      oxc->setGate(info->getInGate(),info->getOutGate());
      delete msg; 
      return;
   }
   else if(msg->getKind() == OBS_UNSCHEDULE_OXC){ //OXC Unscheduling auto message
      OBS_ControlUnitInfo *info = check_and_cast <OBS_ControlUnitInfo*>(msg);
      oxc->unsetGate(info->getInGate());
      delete msg;
      return;
   }
   //Step 1 - BCP received
   OBS_BurstControlPacket *bcp = check_and_cast <OBS_BurstControlPacket*>(msg);

   //extract BCP control info
   OBS_BCPControlInfo *info = (OBS_BCPControlInfo*)bcp->getControlInfo();

   //Step 2 - Extract all necessary BCP info
   int burstColour = bcp->getBurstColour(); //Burst colour
   int arrivalPort = info->getPort(); //Arrival port
   simtime_t arrivalDelta = bcp->getBurstArrivalDelta(); //Time offset between BCP and Burst
   int burstLength = bcp->getBurstSize(); //Burst length
   int destLabel = bcp->getLabel(); //Burst destination label

   int burstifierId = bcp->getBurstifierId();
   int numSeq = bcp->getNumSeq();

   int inGate = coreInput->getOXCGate(arrivalPort, coreInput->getLambdaByColour(arrivalPort,burstColour)); //Request OXC input gate for incoming burst

   EV << "[CoreControl] Received BCP: inPort=" << arrivalPort 
      << ", label=" << destLabel 
      << ", burstColour=" << burstColour 
      << ", burstSize=" << burstLength << "B"
      << ", offset=" << arrivalDelta << endl;

   simtime_t burstArrival = simTime() + arrivalDelta;
   // Measurement window follows burst arrival, not BCP arrival (offset later).
   bool countStats = (burstArrival >= warmupPeriod);

   if(countStats){
      recvBurstCounter[arrivalPort]++;
   }

   // Check if burst is scheduled to arrive in the future. If everything works OK burst should behave this way. But, it's possible that BCP and burst offset was very small and in some point of the path, burst enters the Core Node before it's BCP. In this case, discard this message (Burst was discarded already)
   if(burstArrival < simTime()){
	//BCP discarded. Burst was here and discarded too.
	delete msg;
	if(countStats){
	   dropCounter++;
	   emit(burstLossTotalSignal, 1);
	}

    //Burst dropped (case 1)
    if (data_f != NULL){
    	fprintf(data_f,"%d %d %s %s 1 1\n", burstifierId, numSeq, simTime().str().c_str(), burstArrival.str().c_str());
    }

	return;
   }

   int burstLengthInBits = burstLength*8; //Burst length in bits
   simtime_t burstDuration = (double)burstLengthInBits / dataRate;

   //Query Core Node routing table asking for output port,colour and label
   OBS_CoreRoutingTableEntry *result = routingTable->getEntry(arrivalPort,burstColour,destLabel);
   if(result == NULL) opp_error("Error in routing table query (Control Unit id: %d)",getId());

   int outPort = result->getOutPort();
   int outColour = result->getOutColour();
   int outLabel = result->getOutLabel();

   EV << "[CoreControl] Routing lookup: inPort=" << arrivalPort 
            << ", label=" << destLabel 
            << " -> outPort=" << outPort 
            << ", outColour=" << outColour 
            << ", outLabel=" << outLabel << endl;

   delete result; //Once the query data is stored, I must clean it.

   int lambda = -1;
   bool scheduled = false;
   bool usedFDLForThisBurst = false;

   bool fdlAllowed = useFDL;
   if(fdlAllowed && maxFdlLoopsPerBurst >= 0 && bcp->getFdlLoopCount() >= maxFdlLoopsPerBurst){
      fdlAllowed = false;
   }

   if(outColour == -9){ // * option. Choose the lambda with closest horizon
      	// Choose the best channel
        lambda = gatesHorizon->findNearestLambda(outPort,burstArrival);        

	if(lambda != -1){
            scheduled = true;
	} else if (fdlAllowed) {
            // Scenario B: FDL Loopback
            simtime_t fdlHorizon = gatesHorizon->getHorizon(numPorts, 0);
            if (fdlHorizon <= burstArrival) {
                // Look for free lambda at burstArrival + tau
                lambda = gatesHorizon->findNearestLambda(outPort, burstArrival + tau);
                if (lambda != -1) {
                    scheduled = true;
                    usedFDLForThisBurst = true;
                }
            }
        }

        if (!scheduled) {
            delete msg;
            if(countStats){
               dropCounter++;
               burstLossContentionCounter++;
               emit(burstLossTotalSignal, 1);
               emit(burstLossContentionSignal, 1);
            }

            //Burst dropped (case 2)
            if (data_f != NULL){
            	fprintf(data_f,"%d %d %s %s 1 2\n", burstifierId, numSeq, simTime().str().c_str(), burstArrival.str().c_str());
            }

         	return;
        }
   }else{//Not *. Use the assigned channel
	    //Check if channel is free at the burst arrival moment. (outColour is actually a colour, that's why I convert it using getLambdaByColour method)
      	lambda = coreOutput->getLambdaByColour(outPort,outColour);

	if(gatesHorizon->getHorizon(outPort,lambda) <= burstArrival ){
            scheduled = true;
	} else if (fdlAllowed) {
            // Scenario B: FDL Loopback
            simtime_t fdlHorizon = gatesHorizon->getHorizon(numPorts, 0);
            simtime_t waitTime = gatesHorizon->getHorizon(outPort, lambda) - burstArrival;
            if (waitTime <= tau && fdlHorizon <= burstArrival) {
                scheduled = true;
                usedFDLForThisBurst = true;
            }
        }

        if (!scheduled) {
            //Drop this BCP if the channel is not available at the time of Burst arrival
            delete msg;
            if(countStats){
               dropCounter++;
               burstLossContentionCounter++;
               emit(burstLossTotalSignal, 1);
               emit(burstLossContentionSignal, 1);
            }

            //Burst dropped (case 3)
            if (data_f != NULL){
            	fprintf(data_f,"%d %d %s %s 1 3\n", burstifierId, numSeq, simTime().str().c_str(), burstArrival.str().c_str());
            }
            
            return;
        }
   }

   simtime_t bcpFwdDelay = processingTime;

   if (!usedFDLForThisBurst) {
      // Scenario A: Direct Forwarding
      simtime_t OXCConnectTime = burstArrival - guardTime/2;
      simtime_t OXCDisconnectTime = burstArrival + burstDuration + guardTime/4;
      simtime_t newHorizon = burstArrival + burstDuration + (3*guardTime)/4;

      if(countStats){
         schedBurstCounter[outPort]++;
      }

      //Update horizon array
      gatesHorizon->updateHorizon(outPort,lambda, newHorizon);

      OBS_ControlUnitInfo *controlInfo = new OBS_ControlUnitInfo();
      OBS_ControlUnitInfo *controlInfo1 = new OBS_ControlUnitInfo();

      // Schedule OXC Programming (set an automessage)
      controlInfo->setInGate(inGate);
      controlInfo->setOutGate(coreOutput->getOXCGate(outPort,lambda));
      controlInfo->setKind(OBS_SCHEDULE_OXC);
      controlInfo->setSchedulingPriority(2);

      scheduleAt(OXCConnectTime,controlInfo);

      // Schedule OXC Unprogramming (set an automessage)
      controlInfo1->setInGate(inGate);
      controlInfo1->setKind(OBS_UNSCHEDULE_OXC);
      controlInfo1->setSchedulingPriority(1);

      scheduleAt(OXCDisconnectTime,controlInfo1);

      EV << "[CoreControl] OXC Reserved (Direct): inGate=" << inGate
         << " -> outPort=" << outPort << ", lambda=" << lambda
         << ", connectTime=" << OXCConnectTime
         << ", disconnectTime=" << OXCDisconnectTime << endl;

      // Update BCP
      info->setPort(outPort);

      bcp->setBurstColour(coreOutput->getColourByLambda(outPort,lambda));
      bcp->setBurstArrivalDelta(arrivalDelta - processingTime);

      if(countStats){
         emit(burstNodeDelaySignal, 0.0);
      }
   } else {
      // Scenario B: FDL Loopback
      int fdlOutGate = oxc->gateSize("out") - 1;
      int fdlInGate = oxc->gateSize("in") - 1;

      int newLoopCount = bcp->getFdlLoopCount() + 1;
      bcp->setFdlLoopCount(newLoopCount);

      // 1st OXC reservation: inGate -> fdlOutGate
      simtime_t fdlConnectTime = burstArrival - guardTime/2;
      simtime_t fdlDisconnectTime = burstArrival + burstDuration + guardTime/4;
      simtime_t newFDLHorizon = burstArrival + burstDuration + (3*guardTime)/4;

      gatesHorizon->updateHorizon(numPorts, 0, newFDLHorizon);

      OBS_ControlUnitInfo *fdlControlInfo = new OBS_ControlUnitInfo();
      OBS_ControlUnitInfo *fdlControlInfo1 = new OBS_ControlUnitInfo();

      fdlControlInfo->setInGate(inGate);
      fdlControlInfo->setOutGate(fdlOutGate);
      fdlControlInfo->setKind(OBS_SCHEDULE_OXC);
      fdlControlInfo->setSchedulingPriority(2);
      scheduleAt(fdlConnectTime, fdlControlInfo);

      fdlControlInfo1->setInGate(inGate);
      fdlControlInfo1->setKind(OBS_UNSCHEDULE_OXC);
      fdlControlInfo1->setSchedulingPriority(1);
      scheduleAt(fdlDisconnectTime, fdlControlInfo1);

      // 2nd OXC reservation: fdlInGate -> dest
      simtime_t delayedArrival = burstArrival + tau;
      simtime_t destConnectTime = delayedArrival - guardTime/2;
      simtime_t destDisconnectTime = delayedArrival + burstDuration + guardTime/4;
      simtime_t newDestHorizon = delayedArrival + burstDuration + (3*guardTime)/4;

      if(countStats){
         schedBurstCounter[outPort]++;
      }

      gatesHorizon->updateHorizon(outPort, lambda, newDestHorizon);

      OBS_ControlUnitInfo *destControlInfo = new OBS_ControlUnitInfo();
      OBS_ControlUnitInfo *destControlInfo1 = new OBS_ControlUnitInfo();

      destControlInfo->setInGate(fdlInGate);
      destControlInfo->setOutGate(coreOutput->getOXCGate(outPort, lambda));
      destControlInfo->setKind(OBS_SCHEDULE_OXC);
      destControlInfo->setSchedulingPriority(2);
      scheduleAt(destConnectTime, destControlInfo);

      destControlInfo1->setInGate(fdlInGate);
      destControlInfo1->setKind(OBS_UNSCHEDULE_OXC);
      destControlInfo1->setSchedulingPriority(1);
      scheduleAt(destDisconnectTime, destControlInfo1);

      EV << "[CoreControl] OXC Reserved (FDL): 1st " << inGate << "->" << fdlOutGate
         << " (conn=" << fdlConnectTime << ", disconn=" << fdlDisconnectTime << "); 2nd "
         << fdlInGate << "->" << coreOutput->getOXCGate(outPort, lambda)
         << " (conn=" << destConnectTime << ", disconn=" << destDisconnectTime << ")" << endl;

      if(countStats){
         fdlUsageCountCounter++;
         emit(fdlUsageCountSignal, 1);
         busyTime += burstDuration;
         busyTimeInFlight += burstDuration + tau;
         emit(burstNodeDelaySignal, SIMTIME_DBL(tau));
         if(newLoopCount == 1) burstsLoopedOnceCounter++;
         else burstsLoopedMultipleCounter++;
      }

      // Update BCP
      info->setPort(outPort);

      bcp->setBurstColour(coreOutput->getColourByLambda(outPort, lambda));
      if(fdlOffsetIsSO){
         bcp->setBurstArrivalDelta(arrivalDelta - processingTime);
         bcpFwdDelay = processingTime + tau;
      }else{
         bcp->setBurstArrivalDelta(arrivalDelta + tau - processingTime);
      }
   }

   // Both the direct and the FDL branch end up reserving burstDuration on (outPort,lambda), so the
   // channel occupancy is accumulated once here for either path.
   if(countStats){
      portBusyTime[outPort] += burstDuration;
      portCarriedBytes[outPort] += (double)burstLength;
      simtime_t outDelta = bcp->getBurstArrivalDelta();
      outgoingOffsetSum += SIMTIME_DBL(outDelta);
      outgoingOffsetSamples++;
      if(outgoingOffsetMin < 0 || outDelta < outgoingOffsetMin) outgoingOffsetMin = outDelta;
   }

   if(outLabel == -9) bcp->setLabel(destLabel);
   else bcp->setLabel(outLabel);
   
   // Wait the Control Logic processing time (and tau in Stay-Offset loopback) and then forward this message
   sendDelayed(bcp,bcpFwdDelay,"out");

   //Burst sent
   if (data_f != NULL){
	   fprintf(data_f,"%d %d %s %s 0 0\n", burstifierId, numSeq, simTime().str().c_str(), burstArrival.str().c_str());
   }
}

void OBS_CoreControlLogic::finish(){
   // Register dropped Bursts
   recordScalar("Burst dropped in Core Node",dropCounter);

   simtime_t window = simTime() - warmupPeriod;
   if(window <= 0){
      EV << "[CoreControl] measurement window is not positive (simTime=" << simTime()
         << ", warmup=" << warmupPeriod << ")" << endl;
      window = 0;
   }

   double utilization = 0.0;
   double inFlightOccupancy = 0.0;
   if (window > 0) {
       utilization = SIMTIME_DBL(busyTime) / SIMTIME_DBL(window);
       inFlightOccupancy = SIMTIME_DBL(busyTimeInFlight) / SIMTIME_DBL(window);
   }
   emit(fdlUtilizationSignal, utilization);

   recordScalar("fdlUsageCount", fdlUsageCountCounter);
   recordScalar("fdlUtilization", utilization);
   recordScalar("fdlEntryOccupancy", utilization);
   recordScalar("fdlInFlightOccupancy", inFlightOccupancy);
   recordScalar("burstLossContention", burstLossContentionCounter);
   recordScalar("burstLossTotal", dropCounter);
   recordScalar("burstsLoopedOnce", burstsLoopedOnceCounter);
   recordScalar("burstsLoopedMultiple", burstsLoopedMultipleCounter);
   recordScalar("warmupPeriod", warmupPeriod);
   recordScalar("measurementWindow", window);

   if(outgoingOffsetSamples > 0){
      recordScalar("outgoingOffsetMean", outgoingOffsetSum / (double)outgoingOffsetSamples);
      recordScalar("outgoingOffsetMin", outgoingOffsetMin);
   }

   // Offered vs carried bursts. burstLossRate is the primary metric of experiments A and B; recording it
   // here avoids every analysis script having to re-derive it from raw counters.
   int i;
   long recvTotal = 0;
   long schedTotal = 0;
   for(i=0;i<numInPorts;i++) recvTotal += recvBurstCounter[i];
   for(i=0;i<numOutPorts;i++) schedTotal += schedBurstCounter[i];

   recordScalar("burstsReceived", recvTotal);
   recordScalar("burstsScheduled", schedTotal);
   if(recvTotal > 0) recordScalar("burstLossRate", (double)dropCounter / (double)recvTotal);

   // Measured load of every output fiber. This is the ground truth the offered-load calibration is
   // checked against: the target rho of an experiment must match maxChannelUtilization of the busiest
   // core-to-core fiber, not the nominal rate configured in the ini file.
   double maxUtilization = 0.0;
   double maxIslUtilization = 0.0;
   int maxIslUtilPort = -1;
   char scalarName[64];
   for(i=0;i<numOutPorts;i++){
      int lambdas = gatesHorizon->getPortLambdas(i);
      double portUtilization = 0.0;
      if(lambdas > 0 && window > 0){
         portUtilization = SIMTIME_DBL(portBusyTime[i]) / (SIMTIME_DBL(window) * (double)lambdas);
      }
      if(portUtilization > maxUtilization) maxUtilization = portUtilization;
      if(i > 0 && portUtilization > maxIslUtilization){
         maxIslUtilization = portUtilization;
         maxIslUtilPort = i;
      }

      sprintf(scalarName,"channelUtilization[%d]",i);
      recordScalar(scalarName, portUtilization);
      // Number of data wavelengths on this output fibre. Experiments A/B require W=1;
      // recording it lets the calibration script reject a run that inherited
      // params.ini's lambdasCore1to2=3 instead of the FDL-Scenario freeze.
      sprintf(scalarName,"portLambdas[%d]",i);
      recordScalar(scalarName, lambdas);
      sprintf(scalarName,"carriedBursts[%d]",i);
      recordScalar(scalarName, schedBurstCounter[i]);
      sprintf(scalarName,"carriedBytes[%d]",i);
      recordScalar(scalarName, portCarriedBytes[i]);
   }
   recordScalar("maxChannelUtilization", maxUtilization);
   recordScalar("maxIslChannelUtilization", maxIslUtilization);
   recordScalar("maxIslUtilPort", maxIslUtilPort);

   if (data_f != NULL){
      fclose(data_f);
   }
}
