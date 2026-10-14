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

// Request and acknowledgement of the bufferless retransmission baseline (paper section 5.2).
//
// The baseline recovers contention loss in the time domain instead of the optical domain: the
// core drops the burst, the source sees no acknowledgement within retransmitTimeout and sends
// the packet again, up to maxRetransmissions times. The sequence number is what pairs an
// acknowledgement with the request it answers.
//
// It is a field of the packet rather than bytes inside a payload because the edge burstifier
// wraps whole packets: a payload would have to be re-parsed byte-wise on reception, and the
// acknowledgement is a different size from the request anyway.
packet OBS_RetransmitPacket
{
    int seq = -1; //Request sequence number. The acknowledgement echoes the number it answers.
}
