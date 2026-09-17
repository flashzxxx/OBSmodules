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

#include <omnetpp.h>
#include "OBS_CoreRoutingTable.h"
#include "OBS_CoreOutputHorizon.h"
#include "OBS_ControlUnitInfo_m.h"
#include "OBS_BurstControlPacket_m.h"
#include "OBS_BCPControlInfo_m.h"
#include "OBS_OpticalCrossConnect.h"
#include "OBS_CoreInput.h"
#include "OBS_CoreOutput.h"

#define OBS_SCHEDULE_OXC 1
#define OBS_UNSCHEDULE_OXC 2

//! Creates a lightpath based on the incoming BCP info.
class OBS_CoreControlLogic : public cSimpleModule{
   private:
     OBS_CoreRoutingTable *routingTable; //!< Pointer to routing table.
     OBS_CoreOutputHorizon *gatesHorizon; //!< Pointer to output horizon.
     OBS_CoreInput *coreInput; //!< Input module pointer.
     OBS_CoreOutput *coreOutput; //!< Output module pointer.
     OBS_OpticalCrossConnect *oxc; //!< OXC pointer.

     simtime_t processingTime; //!< Control unit processing time for each BCP.
     simtime_t guardTime; //!< Offset between burst arrival and channel setting order.
     double dataRate; //!< Optical channel data rate.

     int dropCounter; //!< Dropped bursts counter.
     int *recvBurstCounter; //!< Received burst counter.
     int *schedBurstCounter; //!< Scheduled burst counter.

     int numInPorts; //!< Number of input fibers.
     int numOutPorts; //!< Number of output fibers.
     //! Accumulated occupancy of each output fiber, i.e. the sum of the burst durations reserved on it.
     //! Divided by (simulated time x data channels of the port) it yields the measured channel load, which
     //! is the quantity the offered-load calibration of experiments A/B/C is verified against.
     simtime_t *portBusyTime;
     double *portCarriedBytes; //!< Payload bytes reserved on each output fiber. double avoids overflow on long runs.

     bool useFDL; //!< Whether to use FDL loopback.
     simtime_t tau; //!< FDL delay time.
     int numPorts; //!< Number of ports.
     simtime_t switchReconfigTime; //!< Physical lower bound on tau when FDL is on.
     bool fdlOffsetIsSO; //!< Stay-Offset: hold the BCP by tau so downstream offset matches cut-through.
     int maxFdlLoopsPerBurst; //!< -1 unlimited; >=0 is the maximum fdlLoopCount allowed before refusing another loop.
     simtime_t warmupPeriod; //!< Copied from simulation.getWarmupPeriod(). Statistics use burstArrival.

     int fdlUsageCountCounter; //!< Usage counter for FDL.
     simtime_t busyTime; //!< Accumulated FDL entry occupancy (burstDuration).
     simtime_t busyTimeInFlight; //!< Accumulated FDL in-fibre occupancy (burstDuration+tau).
     int burstLossContentionCounter; //!< Counter for contention drops.
     int burstsLoopedOnceCounter; //!< Loopbacks that raised fdlLoopCount to 1.
     int burstsLoopedMultipleCounter; //!< Loopbacks that raised fdlLoopCount above 1.
     double outgoingOffsetSum; //!< Sum of forwarded BCP offsets, for the mean scalar.
     long outgoingOffsetSamples;
     simtime_t outgoingOffsetMin;

     simsignal_t fdlUsageCountSignal; //!< OMNeT++ signal for FDL usage count.
     simsignal_t fdlUtilizationSignal; //!< OMNeT++ signal for FDL utilization.
     simsignal_t burstLossContentionSignal; //!< OMNeT++ signal for contention drops.
     simsignal_t burstLossTotalSignal; //!< OMNeT++ signal for total drops.
     simsignal_t burstNodeDelaySignal; //!< Extra delay introduced at this node (0 or tau).

   protected:

     FILE *data_f; //!< Output file descriptor.
     
	 virtual void initialize();
     virtual void finish();
     virtual void handleMessage(cMessage *msg);
   public:
     OBS_CoreControlLogic();
     virtual ~OBS_CoreControlLogic();
};
