//
// 9-Satellite Constellation OBS Network (3x3 Grid)
// Using OBS_SatelliteNode compound module
// ISL delay: 5ms (~1000km inter-satellite distance)
// All satellites configured with 4 ISL ports (max neighbors) for flexibility
//

package obsmodules.Examples.RingFdlOBS;

import obsmodules.src.SatelliteNode.OBS_SatelliteNode;

network RingFdlOBS
{
    parameters:
        int lambdasPerISL = default(4);
        
        @display("bgb=900,800");
        
    submodules:
        // ===== 9个卫星节点=====
        
        // 第一行
        sat1: OBS_SatelliteNode {
            parameters:
                satId = 1;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=100,200");
            gates:
                islIn[10];  // 角卫星: 2个邻居 × 5通道
                islOut[10];
        }
        
        sat2: OBS_SatelliteNode {
            parameters:
                satId = 2;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=400,200");
            gates:
                islIn[15];  // 边卫星: 3个邻居 × 5通道
                islOut[15];
        }
        
        sat3: OBS_SatelliteNode {
            parameters:
                satId = 3;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=700,200");
            gates:
                islIn[10];  // 角卫星: 2个邻居 × 5通道
                islOut[10];
        }
        
        // 第二行
        sat4: OBS_SatelliteNode {
            parameters:
                satId = 4;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=100,450");
            gates:
                islIn[15];  // 边卫星: 3个邻居 × 5通道
                islOut[15];
        }
        
        sat5: OBS_SatelliteNode {
            parameters:
                satId = 5;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=400,450");
            gates:
                islIn[20];  // 中心卫星: 4个邻居 × 5通道
                islOut[20];
        }
        
        sat6: OBS_SatelliteNode {
            parameters:
                satId = 6;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=700,450");
            gates:
                islIn[15];  // 边卫星: 3个邻居 × 5通道
                islOut[15];
        }
        
        // 第三行
        sat7: OBS_SatelliteNode {
            parameters:
                satId = 7;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=100,700");
            gates:
                islIn[10];  // 角卫星: 2个邻居 × 5通道
                islOut[10];
        }
        
        sat8: OBS_SatelliteNode {
            parameters:
                satId = 8;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=400,700");
            gates:
                islIn[15];  // 边卫星: 3个邻居 × 5通道
                islOut[15];
        }
        
        sat9: OBS_SatelliteNode {
            parameters:
                satId = 9;
                numISLPorts = 4;  
                lambdasPerISL = lambdasPerISL;
                @display("p=700,700");
            gates:
                islIn[10];  // 角卫星: 2个邻居 × 5通道
                islOut[10];
        }
        
    connections:
        // ===== 星间链路（ISL, 5ms delay）=====
        
        // 第一行水平ISL
        for i=0..lambdasPerISL {
            sat1.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat2.islIn++;
            sat2.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat1.islIn++;
        }
        for i=0..lambdasPerISL {
            sat2.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat3.islIn++;
            sat3.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat2.islIn++;
        }
        
        // 第二行水平ISL
        for i=0..lambdasPerISL {
            sat4.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat5.islIn++;
            sat5.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat4.islIn++;
        }
        for i=0..lambdasPerISL {
            sat5.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat6.islIn++;
            sat6.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat5.islIn++;
        }
        
        // 第三行水平ISL
        for i=0..lambdasPerISL {
            sat7.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat8.islIn++;
            sat8.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat7.islIn++;
        }
        for i=0..lambdasPerISL {
            sat8.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat9.islIn++;
            sat9.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat8.islIn++;
        }
        
        // 第一列垂直ISL
        for i=0..lambdasPerISL {
            sat1.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat4.islIn++;
            sat4.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat1.islIn++;
        }
        for i=0..lambdasPerISL {
            sat4.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat7.islIn++;
            sat7.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat4.islIn++;
        }
        
        // 第二列垂直ISL
        for i=0..lambdasPerISL {
            sat2.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat5.islIn++;
            sat5.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat2.islIn++;
        }
        for i=0..lambdasPerISL {
            sat5.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat8.islIn++;
            sat8.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat5.islIn++;
        }
        
        // 第三列垂直ISL
        for i=0..lambdasPerISL {
            sat3.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat6.islIn++;
            sat6.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat3.islIn++;
        }
        for i=0..lambdasPerISL {
            sat6.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat9.islIn++;
            sat9.islOut++ --> {datarate=400Gbps; delay=5ms;} --> sat6.islIn++;
        }
}
