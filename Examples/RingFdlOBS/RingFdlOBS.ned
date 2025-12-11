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
        int lambdasEdge1;
        int lambdasEdge2;
        int lambdasEdge3;
        int lambdasEdge4;
        int lambdasEdge5;
        int lambdasEdge6;
        int lambdasEdge7;
        int lambdasEdge8;
        int lambdasEdge9;
        int lambdasCore1to2;
        int lambdasCore1to4;
        int lambdasCore2to3;
        int lambdasCore2to5;
        int lambdasCore3to6;
        int lambdasCore4to5;
        int lambdasCore4to7;
        int lambdasCore5to6;
        int lambdasCore5to8;
        int lambdasCore6to9;
        int lambdasCore7to8;
        int lambdasCore8to9;

        @display("bgb=801,560;bgi=background/terrain");

    submodules:
        // ===== 9 Satellite Nodes =====

        // Row 1
        sat1: OBS_SatelliteNode {
            parameters:
                satId = 1;
                numISLPorts = 2;
                lambdasEdge = lambdasEdge1;
                @display("p=100,85");
            gates:
                // Connections: Sat2(right) + Sat4(down)
                islIn[(lambdasCore1to2+1) + (lambdasCore1to4+1)];
                islOut[(lambdasCore1to2+1) + (lambdasCore1to4+1)];
        }

        sat2: OBS_SatelliteNode {
            parameters:
                satId = 2;
                numISLPorts = 3;
                lambdasEdge = lambdasEdge2;
                @display("p=400,85");
            gates:
                // Connections: Sat1(left) + Sat3(right) + Sat5(down)
                islIn[(lambdasCore1to2+1) + (lambdasCore2to3+1) + (lambdasCore2to5+1)];
                islOut[(lambdasCore1to2+1) + (lambdasCore2to3+1) + (lambdasCore2to5+1)];
        }

        sat3: OBS_SatelliteNode {
            parameters:
                satId = 3;
                numISLPorts = 2;
                lambdasEdge = lambdasEdge3;
                @display("p=700,85");
            gates:
                // Connections: Sat2(left) + Sat6(down)
                islIn[(lambdasCore2to3+1) + (lambdasCore3to6+1)];
                islOut[(lambdasCore2to3+1) + (lambdasCore3to6+1)];
        }

        // Row 2
        sat4: OBS_SatelliteNode {
            parameters:
                satId = 4;
                numISLPorts = 3;
                lambdasEdge = lambdasEdge4;
                @display("p=61,305");
            gates:
                // Connections: Sat1(up) + Sat5(right) + Sat7(down)
                islIn[(lambdasCore1to4+1) + (lambdasCore4to5+1) + (lambdasCore4to7+1)];
                islOut[(lambdasCore1to4+1) + (lambdasCore4to5+1) + (lambdasCore4to7+1)];
        }

        sat5: OBS_SatelliteNode {
            parameters:
                satId = 5;
                numISLPorts = 4;
                lambdasEdge = lambdasEdge5;
                @display("p=361,305");
            gates:
                // Connections: Sat2(up) + Sat4(left) + Sat6(right) + Sat8(down)
                islIn[(lambdasCore2to5+1) + (lambdasCore4to5+1) + (lambdasCore5to6+1) + (lambdasCore5to8+1)];
                islOut[(lambdasCore2to5+1) + (lambdasCore4to5+1) + (lambdasCore5to6+1) + (lambdasCore5to8+1)];
        }

        sat6: OBS_SatelliteNode {
            parameters:
                satId = 6;
                numISLPorts = 3;
                lambdasEdge = lambdasEdge6;
                @display("p=661,305");
            gates:
                // Connections: Sat3(up) + Sat5(left) + Sat9(down)
                islIn[(lambdasCore3to6+1) + (lambdasCore5to6+1) + (lambdasCore6to9+1)];
                islOut[(lambdasCore3to6+1) + (lambdasCore5to6+1) + (lambdasCore6to9+1)];
        }

        // Row 3
        sat7: OBS_SatelliteNode {
            parameters:
                satId = 7;
                numISLPorts = 2;
                lambdasEdge = lambdasEdge7;
                @display("p=119,513");
            gates:
                // Connections: Sat4(up) + Sat8(right)
                islIn[(lambdasCore4to7+1) + (lambdasCore7to8+1)];
                islOut[(lambdasCore4to7+1) + (lambdasCore7to8+1)];
        }

        sat8: OBS_SatelliteNode {
            parameters:
                satId = 8;
                numISLPorts = 3;
                lambdasEdge = lambdasEdge8;
                @display("p=419,513");
            gates:
                // Connections: Sat5(up) + Sat7(left) + Sat9(right)
                islIn[(lambdasCore5to8+1) + (lambdasCore7to8+1) + (lambdasCore8to9+1)];
                islOut[(lambdasCore5to8+1) + (lambdasCore7to8+1) + (lambdasCore8to9+1)];
        }

        sat9: OBS_SatelliteNode {
            parameters:
                satId = 9;
                numISLPorts = 2;
                lambdasEdge = lambdasEdge9;
                @display("p=719,513");
            gates:
                // Connections: Sat6(up) + Sat8(left)
                islIn[(lambdasCore6to9+1) + (lambdasCore8to9+1)];
                islOut[(lambdasCore6to9+1) + (lambdasCore8to9+1)];
        }

    connections:
        // ===== Inter-Satellite Links (ISL, 5ms delay) =====
        // Note: No datarate set - OBS model requires zero transmission delay
        // to preserve timing between iniBurst and endBurst

        // Row 1 Horizontal ISL
        // Sat1 <-> Sat2
        for i=0..lambdasCore1to2 {
            sat1.islOut++ --> {  delay = 5ms; } --> sat2.islIn++;
            sat2.islOut++ --> {  delay = 5ms; } --> sat1.islIn++;
        }
        // Sat2 <-> Sat3
        for i=0..lambdasCore2to3 {
            sat2.islOut++ --> {  delay = 5ms; } --> sat3.islIn++;
            sat3.islOut++ --> {  delay = 5ms; } --> sat2.islIn++;
        }

        // Row 2 Horizontal ISL
        // Sat4 <-> Sat5
        for i=0..lambdasCore4to5 {
            sat4.islOut++ --> {  delay = 5ms; } --> sat5.islIn++;
            sat5.islOut++ --> {  delay = 5ms; } --> sat4.islIn++;
        }
        // Sat5 <-> Sat6
        for i=0..lambdasCore5to6 {
            sat5.islOut++ --> {  delay = 5ms; } --> sat6.islIn++;
            sat6.islOut++ --> {  delay = 5ms; } --> sat5.islIn++;
        }

        // Row 3 Horizontal ISL
        // Sat7 <-> Sat8
        for i=0..lambdasCore7to8 {
            sat7.islOut++ --> {  delay = 5ms; } --> sat8.islIn++;
            sat8.islOut++ --> {  delay = 5ms; } --> sat7.islIn++;
        }
        // Sat8 <-> Sat9
        for i=0..lambdasCore8to9 {
            sat8.islOut++ --> {  delay = 5ms; } --> sat9.islIn++;
            sat9.islOut++ --> {  delay = 5ms; } --> sat8.islIn++;
        }

        // Column 1 Vertical ISL
        // Sat1 <-> Sat4
        for i=0..lambdasCore1to4 {
            sat1.islOut++ --> {  delay = 5ms; } --> sat4.islIn++;
            sat4.islOut++ --> {  delay = 5ms; } --> sat1.islIn++;
        }
        // Sat4 <-> Sat7
        for i=0..lambdasCore4to7 {
            sat4.islOut++ --> {  delay = 5ms; } --> sat7.islIn++;
            sat7.islOut++ --> {  delay = 5ms; } --> sat4.islIn++;
        }

        // Column 2 Vertical ISL
        // Sat2 <-> Sat5
        for i=0..lambdasCore2to5 {
            sat2.islOut++ --> {  delay = 5ms; } --> sat5.islIn++;
            sat5.islOut++ --> {  delay = 5ms; } --> sat2.islIn++;
        }
        // Sat5 <-> Sat8
        for i=0..lambdasCore5to8 {
            sat5.islOut++ --> {  delay = 5ms; } --> sat8.islIn++;
            sat8.islOut++ --> {  delay = 5ms; } --> sat5.islIn++;
        }

        // Column 3 Vertical ISL
        // Sat3 <-> Sat6
        for i=0..lambdasCore3to6 {
            sat3.islOut++ --> {  delay = 5ms; } --> sat6.islIn++;
            sat6.islOut++ --> {  delay = 5ms; } --> sat3.islIn++;
        }
        // Sat6 <-> Sat9
        for i=0..lambdasCore6to9 {
            sat6.islOut++ --> {  delay = 5ms; } --> sat9.islIn++;
            sat9.islOut++ --> {  delay = 5ms; } --> sat6.islIn++;
        }
}
