# RingFdlOBS - 10-Satellite OBS Network Simulation

## 📋 Overview

This simulation models a **10-satellite constellation** network using Optical Burst Switching (OBS) technology with Inter-Satellite Links (ISL). The network features a custom topology with 3 rows of satellites connected by bidirectional ISLs, and is used to evaluate the performance of **multi-mode dynamic queue scheduling** at the OBS edge node.

### Network Topology

```
    1 --- 2 --- 3
    |     |     |
    4 --- 5 --- 6 --- 7
          |     |     |
          8 --- 9 --- 10
```

- **10 Satellites** with `OBS_SatelliteNode` compound modules (Host + EdgeRouter + CoreSwitch)
- **13 ISL Links**, each with 5ms delay (~1000km inter-satellite distance)
- **Data Rate**: 10 Gbps per wavelength
- **Ethernet**: 100 GbE (host-to-edge)

## 🗂️ Project Structure

```
RingFdlOBS/
├── omnetpp.ini              # Main entry - includes all .ini files
├── params.ini               # General & network parameters (topology, queues, burst assembly)
├── tests.ini                # Functional test configurations (5 tests)
├── experiments.ini          # Performance comparison experiments (4 experiments)
├── DispatcherRules.dat      # Packet dispatcher rules (IP → Label mapping)
├── RingFdlOBS.ned           # Network topology definition
├── plot_comprehensive.R     # R script for 2×2 performance figures
├── run_all_flowscaling.bat  # Batch runner for FlowScaling experiment
├── run_all_flowscaling.sh   # Shell runner for FlowScaling experiment
├── run_tcp_experiment.sh    # Shell runner for TCP experiment
├── config/                  # Routing configuration files
│   ├── H*.irt              # Host routing tables (10 files)
│   ├── Edge*.irt           # Edge router routing tables (10 files)
│   └── core*Route.dat      # Core switch routing tables (10 files)
└── results/                 # Simulation output (.sca, .vec files)
```

## � Core Design: Multi-Mode Dispatcher

The key innovation is in `OBS_PacketDispatcher`, which supports **4 dispatch modes** controlled by the `dispatchMode` parameter:

| Mode | Name | Strategy | Behavior |
|------|------|----------|----------|
| 0 | **Dynamic** | P1→P2→P3→P4 (LRU Preemption) | Zero packet loss, adaptive queue reuse |
| 1 | **NoPreemption** | P1→P2→P3→Drop | No forced flush, drops when full |
| 2 | **RoundRobin** | Rotate queues sequentially | Simple but high signaling overhead |
| 3 | **Static** | label → fixed queue (modulo) | Simplest, drops on label collision |

**Four-level priority dispatch logic (P1–P4)**:
- **P1** (Busy Match): Queue is assembling AND label matches → append directly
- **P2** (Idle Reuse): Queue is idle AND label matches → reuse
- **P3** (Fresh Idle): Queue is idle → assign new label
- **P4** (LRU Preemption): All queues busy → evict least-recently-used queue (Mode 0 only)

### Default Configuration

- **8 physical queues** (`numPacketBurstifiers = 8`) serving **10 destinations**
- DispatcherRules maps destination IP subnets to labels 1–10
- Burst assembly: 2ms timeout, 1 packet per burst, 50B min padding

## 🚀 Quick Start

### Running a Specific Test

```cmd
# From the RingFdlOBS directory
..\..\obsmodules.exe -c ICMPTest -u Cmdenv omnetpp.ini
..\..\obsmodules.exe -c UDPTest -u Cmdenv omnetpp.ini
..\..\obsmodules.exe -c TCPTest -u Cmdenv omnetpp.ini
```

### Running Performance Experiments

```cmd
# FlowScaling: 4 modes × 6 flow counts = 24 runs
..\..\obsmodules.exe -c FlowScaling -u Cmdenv omnetpp.ini

# Or use batch script for all FlowScaling runs
run_all_flowscaling.bat
```

### Running with GUI (Tkenv)

```cmd
..\..\obsmodules.exe -c ICMPTest -u Tkenv omnetpp.ini
```

## 🧪 Functional Tests (tests.ini)

These tests validate correctness of the OBS protocol stack and queue scheduling:

| Test | Description | Traffic Pattern |
|------|-------------|-----------------|
| **ICMPTest** | ICMP Ping: Sat1→Sat5, Sat1→Sat10 | 2 ping flows, 10 pings each |
| **UDPTest** | UDP communication: Sat1→Sat5, Sat1→Sat10 | 1 UDP flow, 1000B packets |
| **TCPTest** | TCP multi-destination: Sat1↔Sat5, Sat1↔Sat10 | 2 TCP client-server sessions |
| **TCPUDPMixedTest** | Mixed traffic: TCP Sat1→Sat5, UDP Sat1→Sat10 | TCP + UDP coexistence |
| **LRUPreemptionTest** | Stress test: 10 ping flows → 8 queues | Forces P4 LRU preemption |

The **LRUPreemptionTest** is particularly important — it creates 10 simultaneous ping flows targeting all destinations, exceeding the 8-queue capacity to verify LRU preemption behavior.

## 📊 Performance Experiments (experiments.ini)

All experiments sweep across **4 dispatch modes** (mode=0,1,2,3) for direct comparison:

| Experiment | Variable | Range | Fixed | Purpose |
|-----------|----------|-------|-------|---------|
| **FlowScaling** | Concurrent flows | 2, 4, 6, 8, 10, 12 | High-freq UDP (2ms interval) | Queue contention under flow overload |
| **LoadIntensity** | Send interval | 0.1s, 0.02s, 0.005s, 0.002s, 0.001s | 10 UDP flows | Impact of traffic rate on scheduling |
| **QueueSensitivity** | Queue count | 2, 4, 6, 8 | 10 flows, 2ms interval | Queue-to-flow ratio effect |
| **TCPThroughput** | — | — | 12 TCP flows, 5000B requests | Reliable transport under heavy load |

### Key Metrics Recorded

Each experiment records per-dispatcher:
- `Packets received` / `Dropped Packets` / `Drop Rate (%)`
- `P1 Hits (Busy Match)` / `P2 Hits (Idle Reuse)` / `P3 Hits (Fresh Idle)` / `P4 Hits (LRU Preemption)`
- `Force Flush Count` (queue label switch cost)

## 📈 Data Analysis

### R Visualization

```r
# Generate 2×2 performance comparison figures (requires ggplot2)
Rscript plot_comprehensive.R

# Outputs:
#   results/fig_ppt_a_flowscaling_pb.png    - Drop rate vs flow count
#   results/fig_ppt_b_flowscaling_so.png    - Signaling overhead vs flow count
#   results/fig_ppt_c_loadintensity_pb.png  - Drop rate vs send rate
#   results/fig_ppt_d_loadintensity_so.png  - Signaling overhead vs send rate
```

## 🌐 Network Architecture

### Satellite Node Compound Module (`OBS_SatelliteNode`)

Each satellite encapsulates three layers:
1. **`StandardHost`** — User terminal (IP traffic source/sink)
2. **`OBS_EdgeNode`** — Edge router with burst assembly pipeline:
   - `OBS_PacketDispatcher` → `OBS_PacketBurstifier[8]` → `OBS_BurstSender`
3. **`OBS_CoreNode`** — Optical core switch for ISL forwarding

### Satellite Port Configuration

| Type | Satellites | ISL Neighbors | Core Ports |
|------|-----------|---------------|------------|
| Leaf-2 | sat1, 3, 4, 7, 8, 10 | 2 | 3 (1 edge + 2 ISL) |
| Branch-3 | sat2, 9 | 3 | 4 (1 edge + 3 ISL) |
| Hub-4 | sat5, 6 | 4 | 5 (1 edge + 4 ISL) |

### IP Addressing

Each satellite host is assigned a `/24` subnet: `10.0.<satId>.0/24`

| Sat | Subnet | Host IP |
|-----|--------|---------|
| sat1 | 10.0.1.0/24 | 10.0.1.x |
| sat2 | 10.0.2.0/24 | 10.0.2.x |
| ... | ... | ... |
| sat10 | 10.0.10.0/24 | 10.0.10.x |

## 🔧 Configuration Reference

### Key Parameters (params.ini)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `numPacketBurstifiers` | 8 | Number of physical burst assembly queues |
| `dispatchMode` | 0 (Dynamic) | Dispatcher scheduling strategy (0–3) |
| `packetBurstifier[*].timeout` | 0.002s | Burst assembly timeout |
| `packetBurstifier[*].numPackets` | 1 | Max packets per burst |
| `packetBurstifier[*].minSizeWithPadding` | 50B | Minimum burst size with padding |
| `sender.dataRate` | 10Gbps | OBS data channel rate |
| `sim-time-limit` | 12s | Default simulation duration |

### Dispatcher Rules (DispatcherRules.dat)

Maps destination IP subnet to a label ID (1–10):

```
destAddr 10.0.1.0 mask 255.255.255.0 destLabel 1
destAddr 10.0.2.0 mask 255.255.255.0 destLabel 2
...
destAddr 10.0.10.0 mask 255.255.255.0 destLabel 10
```

## 🐛 Troubleshooting

**Issue**: "Unassigned Parameter" errors
- **Fix**: Ensure all `lambdasEdge*` and `lambdasCore*` parameters are defined in `params.ini`

**Issue**: TCP test hangs or produces no output
- **Fix**: Increase `sim-time-limit`; TCP requires time for handshake + RTT over ISL

**Issue**: All packets dropped in Mode 3 (Static)
- **Expected**: When `labelId > numQueues`, static mapping `(label-1) % numQueues` causes collisions. This is by-design for comparison.

**Issue**: PowerShell script execution blocked
- **Fix**: `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`

## 📚 References

- OBSModules framework for OMNeT++ (Universidad Publica de Navarra)
- INET-2.0.0 framework for IP/TCP/UDP protocol stack
- Satellite network parameters based on LEO constellation research

---

**Last Updated**: 2026-03-06  
**Author**: OBS Satellite Network Team
