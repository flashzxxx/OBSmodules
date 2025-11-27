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
        int satId;  // 卫星编号 1-9
        int numISLPorts;  // 星间链路端口数（2-4，取决于位置）
        int lambdasPerISL = default(4);
        
        @display("i=device/satellite;bgb=300,400");
        
    gates:
        // 星间链路门（连接其他卫星）
        input islIn[];
        output islOut[];
        
    submodules:
        // 用户终端
        host: StandardHost {
            parameters:
                routingFile = "config/H" + string(satId) + ".irt";
                @display("p=150,50");
        }
        
        // 边缘路由器（OBS接入）
        edgeRouter: OBS_EdgeNode {
            parameters:
                numInLambdas = lambdasPerISL;
                numOutLambdas = lambdasPerISL;
                routingTable.routingFile = "config/Edge" + string(satId) + ".irt";
                @display("p=150,150");
            gates:
                obsIn[lambdasPerISL+1];
                obsOut[lambdasPerISL+1];
        }
        
        // 核心交换机（星间流量转发）
        coreSwitch: OBS_CoreNode {
            parameters:
                numPorts = numISLPorts + 1;  // ISL端口 + 1个本地edgeRouter
                ControlUnit.RoutingTable.routeTableFile = "config/core" + string(satId) + "Route.dat";
                @display("p=150,300;i=abstract/opticalswitch");
        }
        
    connections:
        // 用户终端 <-> 边缘路由器 (100GbE - Standard Maximum Ethernet Speed)
        host.ethg++ <--> {datarate=100Gbps; delay=0.1ms;} <--> edgeRouter.ethIO++;
        
        // 边缘路由器 <-> 核心交换机（本地连接）
        for i=0..lambdasPerISL {
            edgeRouter.obsOut[i] --> {datarate=400Gbps; delay=0.01ms;} --> coreSwitch.in++;
            coreSwitch.out++ --> {datarate=400Gbps; delay=0.01ms;} --> edgeRouter.obsIn[i];
        }
        
        // 核心交换机 <-> 星间链路（对外接口）
        for i=0..sizeof(islIn)-1 {
            islIn[i] --> coreSwitch.in++;
            coreSwitch.out++ --> islOut[i];
        }
}
