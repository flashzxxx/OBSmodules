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

#ifndef __OBS_CHANNELCALENDAR_H_
#define __OBS_CHANNELCALENDAR_H_

#include <omnetpp.h>
#include <vector>

//! One reserved interval of one output channel: the OXC window the burst holds.
//!
//! The window is the same one OBS_CoreControlLogic programs into the cross-connect,
//!   [burstArrival - guardTime/2, burstArrival + burstDuration + guardTime/4],
//! so two windows may touch but must never overlap. This is exactly the invariant the
//! scalar horizon already enforces (see the header of OBS_ChannelCalendar.cc), which is
//! what makes the void-filling admission test a strict generalisation of the horizon
//! test rather than a second, differently-behaving scheduler.
struct OBS_ChannelReservation {
   simtime_t start; //!< Window start (OXC connect time).
   simtime_t end;   //!< Window end (OXC disconnect time).
};

//! Reservation calendar of every output channel of one core node.
//!
//! Used only when the void-filling scheduler option is enabled. The horizon module is
//! deliberately left untouched: this class is the "separate implementation" that the
//! comparison arm needs, and keeping it out of OBS_CoreOutputHorizon is what makes the
//! default path provably unaffected (no allocation, no extra work when the option is off).
class OBS_ChannelCalendar {
   public:
      OBS_ChannelCalendar();
      ~OBS_ChannelCalendar();

      //! Size the calendar. numPorts data ports plus one pseudo-channel for the FDL loopback.
      void setChannelCounts(int numPorts,const std::vector<int> &lambdasPerPort);
      //! Reservations ending before simTime()-margin are dropped; they can never overlap a
      //! future query window because burstArrival >= simTime() (late bursts are dropped) and
      //! the query window starts at burstArrival - guardTime/2 <= margin.
      void setPruneMargin(simtime_t margin) { pruneMargin = margin; }

      //! True if [start,end) overlaps no reservation on (port,lambda).
      bool fits(int port,int lambda,simtime_t start,simtime_t end);
      //! First lambda of the port that can hold [start,end), or -1.
      int findFittingLambda(int port,simtime_t start,simtime_t end);
      //! Record a reservation.
      void reserve(int port,int lambda,simtime_t start,simtime_t end);

      long getScanCount() const { return scanCount; }               //!< Interval comparisons, i.e. the LAUC-VF cost proxy.
      int getMaxReservationCount() const { return maxReservations; } //!< Largest calendar seen, i.e. the void count.
      int getReservationCount(int port,int lambda) const;
      bool isEnabled() const { return !calendars.empty(); }

   private:
      void pruneChannel(std::vector<OBS_ChannelReservation> &channel);
      //! Fail loudly on an out-of-range (port,lambda) instead of mis-scheduling the run.
      void checkChannel(int port,int lambda) const;

      //! calendars[port][lambda] = reservations of that channel, sorted by start and disjoint.
      //! Port numPorts is the FDL loopback pseudo-channel.
      std::vector<std::vector<std::vector<OBS_ChannelReservation> > > calendars;
      simtime_t pruneMargin; //!< Conservative margin for the front pruning.
      long scanCount;        //!< Total interval comparisons, for the complexity statement.
      int maxReservations;   //!< Largest number of reservations on one channel.
};

#endif
