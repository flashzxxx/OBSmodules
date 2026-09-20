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
// Sink and acknowledger of the bufferless retransmission baseline (experiment R).
//
// It answers every request with an ackLength acknowledgement carrying the request's sequence
// number back to its sender. The acknowledgement is small on purpose: it travels the same
// loaded constellation as the data, so it must not double the offered load, but its cost is
// real (one more burst, one more OXC window per request) and is left visible in the measured
// channel utilization of the arm.
//
simple OBS_RetransmitSink like IUDPApp
{
    parameters:
        int localPort;                             // local port to listen on
        volatile int ackLength @unit(B);           // acknowledgement size, in bytes
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
