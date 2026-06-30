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

#ifndef __OBS_FIBERDELAYLINE_H_
#define __OBS_FIBERDELAYLINE_H_

#include <omnetpp.h>

//! Fiber Delay Line (FDL) module. Delays arriving optical burst messages.
class OBS_FiberDelayLine : public cSimpleModule {
   private:
      simtime_t delayTime; //!< Delay time for the optical fiber delay line.
      long fdlUsageCount;  //!< Usage counter for statistics.

   protected:
      virtual void initialize();
      virtual void handleMessage(cMessage *msg);
      virtual void finish();
};

#endif
