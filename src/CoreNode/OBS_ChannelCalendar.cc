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

// WHY A SEPARATE CLASS AND WHY THE HORIZON TEST AND THE VOID TEST AGREE
// ---------------------------------------------------------------------
// OBS_CoreOutputHorizon keeps one scalar per channel: the time the channel becomes free
// again. A burst is admitted when
//
//     horizon <= burstArrival                                        (H)
//
// which is an *append-only* rule: every admitted burst is placed at or after the point
// where the channel has become free, so no admitted window is ever followed by a hole it
// could have filled.
//
// This class keeps the reservations themselves instead, and admits a burst when its OXC
// window [A - g/2, A + D + g/4] overlaps none of them:
//
//     for every reservation r:  A - g/2 >= r.end  or  A + D + g/4 <= r.start   (V)
//
// The two rules are not merely similar, they coincide whenever the channel has no holes.
// If the last reservation is [A0 - g/2, A0 + D0 + g/4] then the horizon the scalar model
// stores after it is
//
//     horizon = A0 + D0 + 3g/4 = (A0 + D0 + g/4) + g/2 = r.end + g/2,
//
// so (H) reads A >= r.end + g/2, i.e. A - g/2 >= r.end, which is exactly the first branch
// of (V). Void filling therefore only ever *adds* admissions - the ones that fit in a hole
// that the append-only rule had to refuse - and a run in which no hole is ever created
// produces bit-identical decisions to the horizon scheduler. The comparison arm is a
// generalisation of the baseline, not a different baseline.

#include "OBS_ChannelCalendar.h"

// Only bother with the O(n) front erase once a channel has enough reservations that the
// memmove is worth it. Below the threshold the calendar holds a few dozen entries anyway.
static const int OBS_CALENDAR_PRUNE_THRESHOLD = 64;

OBS_ChannelCalendar::OBS_ChannelCalendar(){
   pruneMargin = 0;
   scanCount = 0;
   maxReservations = 0;
}

OBS_ChannelCalendar::~OBS_ChannelCalendar(){
}

void OBS_ChannelCalendar::setChannelCounts(int numPorts,const std::vector<int> &lambdasPerPort){
   calendars.clear();
   calendars.resize(numPorts + 1); // +1: the FDL loopback pseudo-channel
   for(int port=0;port<=numPorts;port++){
      // The FDL loopback port carries a single channel, as in OBS_CoreOutputHorizon.
      int lambdas = (port < numPorts && port < (int)lambdasPerPort.size()) ? lambdasPerPort[port] : 1;
      // A data port with no channel would make every later query read out of bounds and, worse,
      // silently refuse every burst. The window is modelled per channel, so this cannot be
      // worked around: refuse the configuration instead of mis-scheduling the whole run.
      if(lambdas < 1){
         opp_error("OBS_ChannelCalendar: output port %d has %d channels; void filling needs at least one channel per port",
                   port, lambdas);
      }
      calendars[port].resize(lambdas);
   }
}

void OBS_ChannelCalendar::checkChannel(int port,int lambda) const {
   if(port < 0 || port >= (int)calendars.size()){
      opp_error("OBS_ChannelCalendar: port %d is outside the calendar (%d ports)", port, (int)calendars.size());
   }
   if(lambda < 0 || lambda >= (int)calendars[port].size()){
      opp_error("OBS_ChannelCalendar: channel %d of port %d is outside the calendar (%d channels)",
                lambda, port, (int)calendars[port].size());
   }
}

int OBS_ChannelCalendar::getReservationCount(int port,int lambda) const {
   if(port < 0 || port >= (int)calendars.size()) return 0;
   if(lambda < 0 || lambda >= (int)calendars[port].size()) return 0;
   return (int)calendars[port][lambda].size();
}

void OBS_ChannelCalendar::pruneChannel(std::vector<OBS_ChannelReservation> &channel){
   if((int)channel.size() < OBS_CALENDAR_PRUNE_THRESHOLD) return;

   simtime_t cutoff = simTime() - pruneMargin;
   size_t dropped = 0;
   while(dropped < channel.size() && channel[dropped].end < cutoff) dropped++;
   if(dropped > 0) channel.erase(channel.begin(),channel.begin() + dropped);
}

bool OBS_ChannelCalendar::fits(int port,int lambda,simtime_t start,simtime_t end){
   checkChannel(port,lambda);
   std::vector<OBS_ChannelReservation> &channel = calendars[port][lambda];
   std::vector<OBS_ChannelReservation>::const_iterator it;
   for(it=channel.begin();it!=channel.end();++it){
      scanCount++;
      // Sorted by start, so once a reservation begins at or after our end, no later one can
      // overlap either. This early exit is what keeps the cost proportional to the number of
      // voids that actually precede the arrival time, which is the LAUC-VF property the
      // satellite compute budget has to be checked against.
      if(end <= it->start) return true;
      if(start >= it->end) continue;
      return false;
   }
   return true;
}

int OBS_ChannelCalendar::findFittingLambda(int port,simtime_t start,simtime_t end){
   checkChannel(port,0);
   for(int lambda=0;lambda<(int)calendars[port].size();lambda++){
      if(fits(port,lambda,start,end)) return lambda;
   }
   return -1;
}

void OBS_ChannelCalendar::reserve(int port,int lambda,simtime_t start,simtime_t end){
   checkChannel(port,lambda);
   std::vector<OBS_ChannelReservation> &channel = calendars[port][lambda];

   OBS_ChannelReservation reservation;
   reservation.start = start;
   reservation.end = end;

   // Reservations are almost always appended (arrival times increase), so the linear search
   // from the front is O(1) in the common case and only walks when a void is being filled.
   std::vector<OBS_ChannelReservation>::iterator it = channel.begin();
   while(it != channel.end() && it->start < start) ++it;
   channel.insert(it,reservation);

   // Measure the live set, not the pre-prune peak: this number is the "how many voids does
   // W=1 actually have to look at" evidence for the compute-budget discussion.
   pruneChannel(channel);
   if((int)channel.size() > maxReservations) maxReservations = (int)channel.size();
}
