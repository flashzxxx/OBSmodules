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
#include "OBS_ChannelCalendar.h"
#include "OBS_ControlUnitInfo_m.h"
#include "OBS_BurstControlPacket_m.h"
#include "OBS_BCPControlInfo_m.h"
#include "OBS_OpticalCrossConnect.h"
#include "OBS_CoreInput.h"
#include "OBS_CoreOutput.h"

#define OBS_SCHEDULE_OXC 1
#define OBS_UNSCHEDULE_OXC 2

//! Why a contention drop happened, for the rejection-reason breakdown of paper figure 4.
//! The three reasons below are mutually exclusive and exhaustive, and they partition
//! burstLossContention exactly:
//!   burstLossNoLoopbackTried + burstLossWaitTooLong + burstLossFdlBusy == burstLossContention
//! The distinction matters because it separates "the delay line was too short for this
//! conflict" (waitTooLong: a longer tau would have helped) from "the delay line was already
//! occupied by another burst" (fdlBusy: a longer tau adds nothing, the buffer itself is now a
//! queue). Week 11 could only infer the second case from fdlInFlightOccupancy > 1.
enum OBS_DropReason {
    //! No loopback was attempted at all. NOT "the channel was busy": in an FDL-on cell this is
    //! reached only when the burst has already spent its loop budget - fdlAllowed is false when
    //! getFdlLoopCount() >= maxFdlLoopsPerBurst, and the red-line config sets that to 1 - so
    //! with the delay line on this counter IS the network-level cost of the single-loopback
    //! limit: bursts that already looped once upstream and have nothing left to spend here.
    //! With useFDL = false it degenerates to "the delay line is disabled". Both cases are
    //! direct attempts that failed, so the label must say "no loopback tried", never
    //! "channel busy".
    OBS_DROP_NO_LOOPBACK_TRIED = 0,
    OBS_DROP_WAIT_TOO_LONG = 1, //!< FDL was free, but the target channel is still busy after tau.
    OBS_DROP_FDL_BUSY = 2 //!< FDL entry occupied by an earlier loopback, so no wait was possible.
};

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
     bool enableVoidFilling; //!< Horizon + void filling scheduler option (comparison arm, default off).
     OBS_ChannelCalendar calendar; //!< Channel reservation calendar; only maintained when enableVoidFilling.
     simtime_t tau; //!< FDL delay time.
     int numPorts; //!< Number of ports.
     simtime_t switchReconfigTime; //!< Physical lower bound on tau when FDL is on.
     bool fdlOffsetIsSO; //!< Stay-Offset: hold the BCP by tau so downstream offset matches cut-through.
     int maxFdlLoopsPerBurst; //!< -1 unlimited; >=0 is the maximum fdlLoopCount allowed before refusing another loop.
     simtime_t warmupPeriod; //!< Copied from simulation.getWarmupPeriod(). Statistics use burstArrival.

     int fdlUsageCountCounter; //!< Usage counter for FDL.
     long voidFilledBursts; //!< Bursts admitted into a hole the append-only horizon rule would have refused.
     simtime_t busyTime; //!< Accumulated FDL entry occupancy (burstDuration).
     simtime_t busyTimeInFlight; //!< Accumulated FDL in-fibre occupancy (burstDuration+tau).
     int burstLossContentionCounter; //!< Counter for contention drops.
     //! Contention drops split by cause (only maintained where burstLossContentionCounter is,
     //! i.e. under countStats). Their sum must equal burstLossContentionCounter.
     //! In an FDL-on cell the first one counts bursts that had already looped once upstream -
     //! i.e. the network-level cost of the single-loopback limit - not "channel busy".
     int burstLossNoLoopbackTriedCounter; //!< Drops where no loopback was attempted.
     int burstLossWaitTooLongCounter; //!< Drops where the channel was still busy after tau.
     int burstLossFdlBusyCounter; //!< Drops where the FDL entry was occupied.
     int burstsLoopedOnceCounter; //!< Loopbacks that raised fdlLoopCount to 1.
     int burstsLoopedMultipleCounter; //!< Loopbacks that raised fdlLoopCount above 1.
     double outgoingOffsetSum; //!< Sum of forwarded BCP offsets, for the mean scalar.
     long outgoingOffsetSamples;
     simtime_t outgoingOffsetMin;

     simsignal_t fdlUsageCountSignal; //!< OMNeT++ signal for FDL usage count.
     simsignal_t fdlUtilizationSignal; //!< OMNeT++ signal for FDL utilization.
     simsignal_t burstLossContentionSignal; //!< OMNeT++ signal for contention drops.
     simsignal_t burstLossNoLoopbackTriedSignal; //!< Signal for drops where no loopback was tried.
     simsignal_t burstLossWaitTooLongSignal; //!< Signal for drops still conflicting after tau.
     simsignal_t burstLossFdlBusySignal; //!< Signal for drops refused because the FDL was occupied.
     simsignal_t burstLossTotalSignal; //!< OMNeT++ signal for total drops.
     simsignal_t burstNodeDelaySignal; //!< Extra delay introduced at this node (0 or tau).

   protected:

     FILE *data_f; //!< Output file descriptor.

     //! Window the burst would hold on (port,lambda): the exact OXC reservation interval.
     simtime_t windowStart(simtime_t burstArrival) { return burstArrival - guardTime/2; }
     simtime_t windowEnd(simtime_t burstArrival,simtime_t burstDuration) { return burstArrival + burstDuration + guardTime/4; }

     //! Can (port,lambda) hold a burst arriving at burstArrival? Horizon rule, or the void
     //! filling calendar when enableVoidFilling is set. The two agree when there is no hole.
     bool channelAccepts(int port,int lambda,simtime_t burstArrival,simtime_t burstDuration);
     //! Best lambda of port for that burst, or -1. Horizon scheduling or void filling.
     int selectLambda(int port,simtime_t burstArrival,simtime_t burstDuration);
     //! Update the horizon and, when void filling is on, record the window in the calendar.
     void reserveChannel(int port,int lambda,simtime_t burstArrival,simtime_t burstDuration,simtime_t newHorizon);

	 virtual void initialize();
     virtual void finish();
     virtual void handleMessage(cMessage *msg);
   public:
     OBS_CoreControlLogic();
     virtual ~OBS_CoreControlLogic();
};
