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

#ifndef __OBS_RETRANSMITSOURCE_H_
#define __OBS_RETRANSMITSOURCE_H_

#include <map>
#include <vector>

#include "INETDefs.h"
#include "applications/common/ApplicationBase.h"
#include "transport/contract/UDPSocket.h"
#include "OBS_RetransmitPacket_m.h"

//! Host-side source of the bufferless retransmission baseline (experiment R).
//!
//! It offers the same packet stream as INET's UDPBasicApp - one packet every sendInterval,
//! messageLength bytes each, random destination out of destAddresses - but keeps every
//! unacknowledged request and sends it again when retransmitTimeout expires, up to
//! maxRetransmissions times. After that the request counts as lost.
//!
//! Why a request/acknowledgement pair and not just UDP: in this network a burst lost in core
//! contention is simply gone, and nothing tells the source. The alternative to optical
//! buffering that the literature compares against is exactly this - recover in the time
//! domain - and it needs a return path, which is why the arm carries an acknowledgement per
//! request. Its cost is real and is reported rather than hidden: an acknowledgement is a
//! packet, so it is burstified and switched like any other traffic, and the measured channel
//! utilization of this arm is higher than its fresh offered load.
class OBS_RetransmitSource : public ApplicationBase {
   protected:
      //! One request that has been sent and not yet acknowledged.
      struct RequestState {
         int seq;              //!< Sequence number, also the kind of its retransmission timer.
         simtime_t firstSent;  //!< Time of the first transmission, the delay metric's origin.
         int transmissions;    //!< 1 after the first send, 2 after one retransmission, ...
         IPvXAddress destAddr;
         int destPort;
         cMessage *timer;      //!< Retransmission timer of this request.
      };

      UDPSocket socket;
      int localPort;
      int destPort;
      long messageLength;
      simtime_t startTime;
      simtime_t stopTime;
      simtime_t retransmitTimeout; //!< ARQ timeout. Must exceed the path round trip, otherwise every request is retransmitted spuriously.
      int maxRetransmissions;      //!< Retry cap. 0 turns the arm into send-and-forget with loss observation.

      cMessage *sendTimer;         //!< Generates the fresh request stream.
      std::map<int,RequestState> outstanding; //!< Requests awaiting an acknowledgement.
      std::vector<IPvXAddress> destAddresses; //!< Resolved destination addresses.

      int nextSeq;
      // Statistics. All of them are gated on the measurement window, so a warmup period
      // behaves the same way here as it does in OBS_CoreControlLogic.
      long requestsSent;        //!< Fresh requests offered.
      long retransmissionsSent; //!< Extra transmissions caused by missing acknowledgements.
      long requestsAcked;       //!< Requests eventually acknowledged.
      long requestsAbandoned;   //!< Requests that hit the retry cap.
      long duplicateAcks;       //!< Acknowledgements for requests that were already completed.
      simtime_t warmupPeriod;

      static simsignal_t requestCompletedSignal;

      virtual IPvXAddress chooseDestAddr();
      virtual void sendRequest();
      virtual void processAck(cPacket *packet);
      virtual void processRetransmitTimer(cMessage *timer);

   public:
      OBS_RetransmitSource();
      virtual ~OBS_RetransmitSource();

   protected:
      virtual int numInitStages() const { return 4; }
      virtual void initialize(int stage);
      virtual void finish();
      virtual void handleMessageWhenUp(cMessage *msg);

      virtual bool handleNodeStart(IDoneCallback *doneCallback);
      virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
      virtual void handleNodeCrash();
};

#endif
