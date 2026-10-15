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

package obsmodules.src.Retransmit;

import inet.applications.IUDPApp;

//
// Source of the bufferless retransmission baseline (experiment R, paper section 5.2).
//
// It offers exactly the stream UDPBasicApp offers - one messageLength packet every
// sendInterval, to a destination drawn uniformly from destAddresses - and additionally keeps
// every unacknowledged request until the peer answers. When retransmitTimeout expires without
// an acknowledgement the request is transmitted again, at most maxRetransmissions times; after
// that it counts as lost.
//
// This is the minimum comparable version of the "no optical buffer, let the source recover"
// route that Zhao 2022 and Roethig/Mouammar 2026 take on engineering grounds: the core still
// drops the burst on contention, but the loss is not necessarily fatal. Without this arm the
// No-FDL reference is a straw man, because plain No-FDL never retries anything.
//
// retransmitTimeout must exceed the round trip of the longest path in the scenario. A timeout
// shorter than the RTT makes every request retransmit spuriously and the arm would then be
// measuring its own impatience instead of the network's loss.
//
// windowSize bounds the requests that are simultaneously unacknowledged. It defaults to 0,
// i.e. unlimited, which is the open-loop arm that was measured up to week 14: that arm has no
// flow control at all, so the retransmissions provoked by loss add to the load that caused
// them, and above a certain offered load the backlog grows without bound. Bounding it at W
// makes the arm a window-limited ARQ and lets it be measured at loads where the open-loop
// variant diverges, which is what section 5.2 needs in order to stop resting on a single
// comparable load point.
//
// A retransmission consumes a window slot too, because it is another transmission of a
// request that is already in flight - a limit that only counted fresh requests would still
// let a request be retransmitted without bound. When the window is full, both a fresh request
// and a retransmission are deferred and retried after retryInterval. Nothing is dropped for
// lack of a slot: deferring throttles the source, dropping would add a loss mechanism and
// change what the arm's delivery ratio means.
//
// The send timer is a fixed interval, not a random one: par("sendInterval") is evaluated once
// at initialization, so the offered stream is strictly periodic and the window changes only
// how much of it is deferred. completionDelay keeps its meaning (first transmission to
// acknowledgement); the waiting time is reported separately as pendingSendDelay, so that the
// delay comparison with the loopback arm stays on the same footing as before.
//
// It is NOT part of the protected EdgeNode baseline: nothing in src/EdgeNode/ is touched, and
// the module is only instantiated by configs that ask for it.
//
simple OBS_RetransmitSource like IUDPApp
{
    parameters:
        int localPort = default(-1);                 // local port (-1: use ephemeral port)
        string destAddresses = default("");          // list of addresses, separated by spaces ("" : don't send)
        int destPort;                                // destination port
        volatile int messageLength @unit(B);         // request size, in bytes
        double startTime @unit(s) = default(0s);     // start of the first request
        double stopTime @unit(s) = default(-1s);     // stop offering new requests, -1s means forever
        volatile double sendInterval @unit(s);       // interval between fresh requests, evaluated once at initialization
        double retransmitTimeout @unit(s);           // ARQ timeout, measured from the last transmission
        int maxRetransmissions = default(3);         // retry cap; 0 = send and forget, no timer is armed for the request at all
        int windowSize = default(0);                 // max unacknowledged requests in flight; 0 = unlimited
        double retryInterval @unit(s) = default(1ms); // how often to re-check for a free window slot while full
        @display("i=block/app");
        @signal[requestCompleted](type="double");
        // record=mean,min,max,count (not "stats"): the stats recorder writes a single
        // <name>:stats statistic line with no fields in this OMNet++ version, which no analysis
        // script can read. These recorders produce ordinary scalars, the same way
        // OBS_CoreControlLogic records burstNodeDelay. pendingSendDelay is deliberately separate
        // from completionDelay: one is network delay, the other is the sender waiting for a slot.
        @statistic[completionDelay](title="request completion delay (first transmission to acknowledgement)"; source="requestCompleted"; record=mean,min,max,count);
        @statistic[pendingSendDelay](title="time a fresh request waited for a free window slot"; source="windowDeferred"; record=mean,min,max,count);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
