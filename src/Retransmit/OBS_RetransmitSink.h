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

#ifndef __OBS_RETRANSMITSINK_H_
#define __OBS_RETRANSMITSINK_H_

#include "INETDefs.h"
#include "applications/common/ApplicationBase.h"
#include "transport/contract/UDPSocket.h"
#include "OBS_RetransmitPacket_m.h"

//! Host-side sink and acknowledger of the bufferless retransmission baseline.
//!
//! It answers every request with a short acknowledgement carrying the request's sequence
//! number back to the sender, which is what lets OBS_RetransmitSource tell a lost burst from
//! a delivered one. The acknowledgement is deliberately smaller than the request (ackLength
//! against the request's messageLength): it has to travel back through the same loaded
//! constellation, but it is control traffic and is not meant to double the offered load.
//! Its real cost is still there - one more burst, one more OXC window - and shows up in the
//! measured channel utilization of the arm rather than being assumed away.
//!
//! Duplicate requests (a retransmission sent because the acknowledgement itself was lost) are
//! answered again rather than filtered here: the source is the only place that can count them
//! unambiguously, because sequence numbers are per source and several sources feed one sink.
class OBS_RetransmitSink : public ApplicationBase {
   protected:
      UDPSocket socket;
      int localPort;
      long ackLength; //!< Size of the acknowledgement, in bytes.

      long requestsReceived;
      long acksSent;
      simtime_t warmupPeriod;

   public:
      OBS_RetransmitSink();
      virtual ~OBS_RetransmitSink();

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
