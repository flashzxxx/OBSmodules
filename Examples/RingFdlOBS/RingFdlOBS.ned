//
// 10-Satellite Constellation OBS Network (Custom Topology)
// Topology:
//     1 --- 2 --- 3
//     |     |     |
//     4 --- 5 --- 6 --- 7
//           |     |     |
//           8 --- 9 --- 10
//
// ISL delay: 5ms (~1000km inter-satellite distance)
//

package obsmodules.Examples.RingFdlOBS;

import obsmodules.src.SatelliteNode.OBS_SatelliteNode;

network RingFdlOBS
{
    parameters:
        // Edge lambdas for each satellite
        int lambdasEdge1;
        int lambdasEdge2;
        int lambdasEdge3;
        int lambdasEdge4;
        int lambdasEdge5;
        int lambdasEdge6;
        int lambdasEdge7;
        int lambdasEdge8;
        int lambdasEdge9;
        int lambdasEdge10;

        // ISL lambdas (named by connected nodes)
        int lambdasCore1to2;  // Row 1
        int lambdasCore2to3;
        int lambdasCore4to5;  // Row 2
        int lambdasCore5to6;
        int lambdasCore6to7;
        int lambdasCore8to9;  // Row 3
        int lambdasCore9to10;
        int lambdasCore1to4;  // Vertical
        int lambdasCore2to5;
        int lambdasCore3to6;
        int lambdasCore5to8;
        int lambdasCore6to9;
        int lambdasCore7to10;

        @display("bgb=799,559;bgi=background/terrain");

    submodules:
        // ===== Row 1: Satellites 1, 2, 3 =====
        sat1: OBS_SatelliteNode {
            parameters:
                satId = 1;
                numISLPorts = 2;  // Right=Sat2, Down=Sat4
                lambdasEdge = lambdasEdge1;
                @display("p=100,80");
            gates:
                islIn[(lambdasCore1to2+1) + (lambdasCore1to4+1)];
                islOut[(lambdasCore1to2+1) + (lambdasCore1to4+1)];
        }

        sat2: OBS_SatelliteNode {
            parameters:
                satId = 2;
                numISLPorts = 3;  // Left=Sat1, Right=Sat3, Down=Sat5
                lambdasEdge = lambdasEdge2;
                @display("p=300,80");
            gates:
                islIn[(lambdasCore1to2+1) + (lambdasCore2to3+1) + (lambdasCore2to5+1)];
                islOut[(lambdasCore1to2+1) + (lambdasCore2to3+1) + (lambdasCore2to5+1)];
        }

        sat3: OBS_SatelliteNode {
            parameters:
                satId = 3;
                numISLPorts = 2;  // Left=Sat2, Down=Sat6
                lambdasEdge = lambdasEdge3;
                @display("p=500,80");
            gates:
                islIn[(lambdasCore2to3+1) + (lambdasCore3to6+1)];
                islOut[(lambdasCore2to3+1) + (lambdasCore3to6+1)];
        }

        // ===== Row 2: Satellites 4, 5, 6, 7 =====
        sat4: OBS_SatelliteNode {
            parameters:
                satId = 4;
                numISLPorts = 2;  // Up=Sat1, Right=Sat5
                lambdasEdge = lambdasEdge4;
                @display("p=100,280");
            gates:
                islIn[(lambdasCore1to4+1) + (lambdasCore4to5+1)];
                islOut[(lambdasCore1to4+1) + (lambdasCore4to5+1)];
        }

        sat5: OBS_SatelliteNode {
            parameters:
                satId = 5;
                numISLPorts = 4;  // Up=Sat2, Left=Sat4, Right=Sat6, Down=Sat8
                lambdasEdge = lambdasEdge5;
                @display("p=300,280");
            gates:
                islIn[(lambdasCore2to5+1) + (lambdasCore4to5+1) + (lambdasCore5to6+1) + (lambdasCore5to8+1)];
                islOut[(lambdasCore2to5+1) + (lambdasCore4to5+1) + (lambdasCore5to6+1) + (lambdasCore5to8+1)];
        }

        sat6: OBS_SatelliteNode {
            parameters:
                satId = 6;
                numISLPorts = 4;  // Up=Sat3, Left=Sat5, Right=Sat7, Down=Sat9
                lambdasEdge = lambdasEdge6;
                @display("p=500,280");
            gates:
                islIn[(lambdasCore3to6+1) + (lambdasCore5to6+1) + (lambdasCore6to7+1) + (lambdasCore6to9+1)];
                islOut[(lambdasCore3to6+1) + (lambdasCore5to6+1) + (lambdasCore6to7+1) + (lambdasCore6to9+1)];
        }

        sat7: OBS_SatelliteNode {
            parameters:
                satId = 7;
                numISLPorts = 2;  // Left=Sat6, Down=Sat10
                lambdasEdge = lambdasEdge7;
                @display("p=700,280");
            gates:
                islIn[(lambdasCore6to7+1) + (lambdasCore7to10+1)];
                islOut[(lambdasCore6to7+1) + (lambdasCore7to10+1)];
        }

        // ===== Row 3: Satellites 8, 9, 10 =====
        sat8: OBS_SatelliteNode {
            parameters:
                satId = 8;
                numISLPorts = 2;  // Up=Sat5, Right=Sat9
                lambdasEdge = lambdasEdge8;
                @display("p=300,480");
            gates:
                islIn[(lambdasCore5to8+1) + (lambdasCore8to9+1)];
                islOut[(lambdasCore5to8+1) + (lambdasCore8to9+1)];
        }

        sat9: OBS_SatelliteNode {
            parameters:
                satId = 9;
                numISLPorts = 3;  // Up=Sat6, Left=Sat8, Right=Sat10
                lambdasEdge = lambdasEdge9;
                @display("p=500,480");
            gates:
                islIn[(lambdasCore6to9+1) + (lambdasCore8to9+1) + (lambdasCore9to10+1)];
                islOut[(lambdasCore6to9+1) + (lambdasCore8to9+1) + (lambdasCore9to10+1)];
        }

        sat10: OBS_SatelliteNode {
            parameters:
                satId = 10;
                numISLPorts = 2;  // Up=Sat7, Left=Sat9
                lambdasEdge = lambdasEdge10;
                @display("p=700,480");
            gates:
                islIn[(lambdasCore7to10+1) + (lambdasCore9to10+1)];
                islOut[(lambdasCore7to10+1) + (lambdasCore9to10+1)];
        }

    connections:
        // ===== Row 1 Horizontal ISL =====
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

        // ===== Row 2 Horizontal ISL =====
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
        // Sat6 <-> Sat7
        for i=0..lambdasCore6to7 {
            sat6.islOut++ --> {  delay = 5ms; } --> sat7.islIn++;
            sat7.islOut++ --> {  delay = 5ms; } --> sat6.islIn++;
        }

        // ===== Row 3 Horizontal ISL =====
        // Sat8 <-> Sat9
        for i=0..lambdasCore8to9 {
            sat8.islOut++ --> {  delay = 5ms; } --> sat9.islIn++;
            sat9.islOut++ --> {  delay = 5ms; } --> sat8.islIn++;
        }
        // Sat9 <-> Sat10
        for i=0..lambdasCore9to10 {
            sat9.islOut++ --> {  delay = 5ms; } --> sat10.islIn++;
            sat10.islOut++ --> {  delay = 5ms; } --> sat9.islIn++;
        }

        // ===== Vertical ISL =====
        // Sat1 <-> Sat4
        for i=0..lambdasCore1to4 {
            sat1.islOut++ --> {  delay = 5ms; } --> sat4.islIn++;
            sat4.islOut++ --> {  delay = 5ms; } --> sat1.islIn++;
        }
        // Sat2 <-> Sat5
        for i=0..lambdasCore2to5 {
            sat2.islOut++ --> {  delay = 5ms; } --> sat5.islIn++;
            sat5.islOut++ --> {  delay = 5ms; } --> sat2.islIn++;
        }
        // Sat3 <-> Sat6
        for i=0..lambdasCore3to6 {
            sat3.islOut++ --> {  delay = 5ms; } --> sat6.islIn++;
            sat6.islOut++ --> {  delay = 5ms; } --> sat3.islIn++;
        }
        // Sat5 <-> Sat8
        for i=0..lambdasCore5to8 {
            sat5.islOut++ --> {  delay = 5ms; } --> sat8.islIn++;
            sat8.islOut++ --> {  delay = 5ms; } --> sat5.islIn++;
        }
        // Sat6 <-> Sat9
        for i=0..lambdasCore6to9 {
            sat6.islOut++ --> {  delay = 5ms; } --> sat9.islIn++;
            sat9.islOut++ --> {  delay = 5ms; } --> sat6.islIn++;
        }
        // Sat7 <-> Sat10
        for i=0..lambdasCore7to10 {
            sat7.islOut++ --> {  delay = 5ms; } --> sat10.islIn++;
            sat10.islOut++ --> {  delay = 5ms; } --> sat7.islIn++;
        }
}
