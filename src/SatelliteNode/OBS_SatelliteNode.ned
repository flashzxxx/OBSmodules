//
// Satellite Node Compound Module
// Encapsulates: Host (user terminal) + Edge (access) + Core (ISL switching)
//

package obsmodules.src.SatelliteNode;

import obsmodules.src.EdgeNode.OBS_EdgeNode;
import obsmodules.src.CoreNode.OBS_CoreNode;
import inet.nodes.inet.StandardHost;

module OBS_SatelliteNode
{
    parameters:
        int satId;  // Satellite ID: 1-9
        int numISLPorts;  // Number of ISL ports (2-4, depends on position)
        int lambdasEdge;
        
        @display("i=device/satellite;bgb=300,400");
        
    gates:
        // Inter-Satellite Link gates (connect to other satellites)
        input islIn[];
        output islOut[];
        
    submodules:
        // User terminal
        host: StandardHost {
            parameters:
                routingFile = "config/H" + string(satId) + ".irt";
                @display("p=150,50");
        }
        
        // Edge router (OBS access)
        edgeRouter: OBS_EdgeNode {
            parameters:
                numInLambdas = lambdasEdge;
                numOutLambdas = lambdasEdge;
                routingTable.routingFile = "config/Edge" + string(satId) + ".irt";
                @display("p=150,150");
            gates:
                obsIn[lambdasEdge+1];
                obsOut[lambdasEdge+1];
        }
        
        // Core switch (ISL traffic forwarding)
        coreSwitch: OBS_CoreNode {
            parameters:
                numPorts = numISLPorts + 1;  // ISL ports + 1 local edgeRouter
                ControlUnit.RoutingTable.routeTableFile = "config/core" + string(satId) + "Route.dat";
                @display("p=150,300;i=abstract/opticalswitch");
        }
        
    connections:
        // User terminal <-> Edge router (100GbE - Standard Maximum Ethernet Speed)
        host.ethg++ <--> {datarate=100Gbps;} <--> edgeRouter.ethIO++;
        
        // Edge router <-> Core switch (local connection)
        for i=0..lambdasEdge {
            edgeRouter.obsOut[i] --> {datarate=400Gbps;} --> coreSwitch.in++;
            coreSwitch.out++ --> {datarate=400Gbps;} --> edgeRouter.obsIn[i];
        }
        
        // Core switch <-> Inter-Satellite Links (external interfaces)
        for i=0..sizeof(islIn)-1 {
            islIn[i] --> coreSwitch.in++;
            coreSwitch.out++ --> islOut[i];
        }
}
