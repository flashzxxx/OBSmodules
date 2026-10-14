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

#include "OBS_CoreOutputHorizon.h"

Define_Module(OBS_CoreOutputHorizon);

OBS_CoreOutputHorizon::OBS_CoreOutputHorizon(){
   // initialize() allocates these, but it can opp_error before it reaches the
   // horizon allocation (a malformed lambdasPerPort string). Null-init them so
   // the destructor does not walk uninitialized pointers on that path - that
   // used to free garbage and abort with an access violation instead of
   // reporting the model error.
   horizon = NULL;
   portLambdas = NULL;
}

OBS_CoreOutputHorizon::~OBS_CoreOutputHorizon(){
	if(horizon != NULL){
		int i;
		int numPorts = par("numPorts");
		for(i=0;i<=numPorts;i++)
		   free(horizon[i]);
		free(horizon);
	}
	free(portLambdas); // free(NULL) is a no-op
}

void OBS_CoreOutputHorizon::initialize(){
   int numPorts = par("numPorts");
   int i=0;
   int j = 0;
   portLambdas= (int*)calloc(numPorts + 1,sizeof(int));
   
   cStringTokenizer tokenizer(par("lambdasPerPort").stringValue());
   while(tokenizer.hasMoreTokens()){
      if(i >= numPorts){
         opp_error("lambdasPerPort has more tokens than numPorts (%d)", numPorts);
      }
      portLambdas[i] = atoi(tokenizer.nextToken());
      i++;
   }
   if(i != numPorts){
      opp_error("lambdasPerPort has %d tokens, expected numPorts=%d", i, numPorts);
   }
   portLambdas[numPorts] = 1; // FDL loopback port has 1 wavelength channel

   horizon = (simtime_t**)calloc(numPorts + 1,sizeof(simtime_t*));

   for(i=0;i<=numPorts;i++){
      horizon[i] = (simtime_t*)calloc(portLambdas[i],sizeof(simtime_t)); 
      // Fill horizon matrix with zeros
      for(j=0;j<portLambdas[i];j++){
         horizon[i][j] = 0;
	 WATCH(horizon[i][j]);
      }
   }

}

//Find a lambda which horizon value is nearest (or even equal) to given arrival time. 
int OBS_CoreOutputHorizon::findNearestLambda(int port,simtime_t arrivalTime){
   Enter_Method_Silent();

   int i;
   int min = 0;

   simtime_t minDiff = -1; //Initial value -1 because 0 is a valid time difference

   for(i=0;i<portLambdas[port];i++){
      // A simple minimum algorithm
      if(arrivalTime > horizon[port][i]){ 
         if(minDiff == -1){ //No minimum encountered already. For now this value is fine.
	    min = i;
            minDiff = arrivalTime - horizon[port][i];
         }else if((arrivalTime - horizon[port][i]) < minDiff){ //Return first minimum value encountered
            min = i;
            minDiff = arrivalTime - horizon[port][i];
         }
      }
   }
   //If minDiff is -1 at this point, no minimum found
   if(minDiff == -1) return -1;

   return min;
}

// Enter_Method_Silent rather than Enter_Method: this is a plain getter with no side effects, and it is
// also called from finish(), where triggering the method-call animation of Enter_Method is not wanted.
int OBS_CoreOutputHorizon::getPortLambdas(int port){
   Enter_Method_Silent();
   int numPorts = par("numPorts");
   if(portLambdas == NULL || port < 0 || port > numPorts) return 0;
   return portLambdas[port];
}

void OBS_CoreOutputHorizon::updateHorizon(int port, int lambda, simtime_t newTime){
   Enter_Method_Silent();
   horizon[port][lambda] = newTime;
}

simtime_t OBS_CoreOutputHorizon::getHorizon(int port,int lambda){
   Enter_Method_Silent();
   if(lambda == -1) return -1; //Just in case lambda = -1
   return horizon[port][lambda];
}
