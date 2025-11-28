# RingFdlOBS - 9-Satellite OBS Network Simulation

## 📋 Overview

This simulation models a 9-satellite constellation network (3×3 grid) using Optical Burst Switching (OBS) technology with Inter-Satellite Links (ISL).

## 🗂️ Project Structure

```
RingFdlOBS/
├── omnetpp.ini              # Main configuration (general parameters)
├── scenarios.ini            # Wavelength scenarios (1λ, 4λ, 10λ)
├── experiments.ini          # Optimization experiments
├── DispatcherRules.dat      # Packet dispatcher rules
├── RingFdlOBS.ned          # Network topology definition
├── config/                  # Routing configuration files
│   ├── H*.irt              # Host routing tables
│   ├── Edge*.irt           # Edge router routing tables
│   └── core*Route.dat      # Core switch routing tables
└── scripts/                 # Helper scripts (see below)
```

## 🚀 Quick Start

### Method 1: Using Quick Launch Scripts (Windows)

```cmd
# Run standard 4-wavelength scenario
run_standard.bat

# Run high-capacity 10wavelength scenario
run_highcapacity.bat
```

### Method 2: Using Command Line

```cmd
# Run specific configuration
..\..\obsmodules.exe -c Standard4Lambdas -u Cmdenv omnetpp.ini

# Available configurations (see scenarios.ini and experiments.ini):
# - Standard4Lambdas
# - HighCapacity10Lambdas
# - LowLatency1Lambda
# - Baseline
# - LargeOffset
# - SmallBurst
# - HighLoad
# - HighCapacityHighLoad
```

### Method 3: With Performance Monitoring (PowerShell)

```powershell
# Run with real-time performance monitoring
.\run_with_monitor.ps1 -Config Standard4Lambdas -ResultsDir results

# Monitor: Memory usage, CPU time, execution duration
# Generates detailed logs and performance reports
```

## 📊 Available Scenarios

### Base Scenarios (scenarios.ini)

| Scenario | Wavelengths | Total Capacity | Use Case |
|----------|-------------|----------------|----------|
| **LowLatency1Lambda** | 1 | 400 Gbps | Minimal complexity, low latency |
| **Standard4Lambdas** | 4 | 1.6 Tbps | Standard operations |
| **HighCapacity10Lambdas** | 10 | 4 Tbps | High bandwidth requirements |

### Optimization Experiments (experiments.ini)

| Experiment | Base | Modification | Purpose |
|-----------|------|--------------|---------|
| **Baseline** | Standard4Lambdas | None | Reference benchmark |
| **LargeOffset** | Standard4Lambdas | ↑ Offset time | Reduce contention |
| **SmallBurst** | Standard4Lambdas | ↓ Burst size | Lower latency |
| **HighLoad** | Standard4Lambdas | ↑ Traffic rate | Stress test |
| **HighCapacityHighLoad** | HighCapacity10Lambdas | ↑ Traffic rate | Maximum throughput test |
| **HighCapacityLargeOffset** | HighCapacity10Lambdas | ↑ Offset time | High capacity optimization |
| **LowLatencyHighLoad** | LowLatency1Lambda | ↑ Traffic rate | Single-lambda stress test |

## 🧪 Running All Tests

### Batch Test Runner

```cmd
# Run all scenarios automatically
runAllTests.bat

# Results saved in: results_<timestamp>/
# Each scenario generates separate output files
```

## 📈 Configuration Details

### Wavelength Design Pattern

Following TreeTopologyOBS design:
- `lambdasPerISL` = maximum index value
- `lambdasPerISL = 3` → creates 4 wavelengths (indices 0,1,2,3)
- `lambdasPerISL = 9` → creates 10 wavelengths (indices 0-9)

### Network Topology

- **9 Satellites** arranged in 3×3 grid
- **ISL Delay**: 5ms (~1000km distance)
- **Data Rate**: 400 Gbps per wavelength
- **Ethernet**: 100 GbE (host-to-edge)

### Satellite Port Configuration

| Type | Satellites | Neighbors | Total Ports |
|------|-----------|-----------|-------------|
| Corner | sat1, 3, 7, 9 | 2 | 10 |
| Edge | sat2, 4, 6, 8 | 3 | 15 |
| Center | sat5 | 4 | 20 |

## 📝 Helper Scripts

### Windows Batch Scripts

- **`run_standard.bat`** - Quick launch standard scenario
- **`run_highcapacity.bat`** - Quick launch high-capacity scenario
- **`runAllTests.bat`** - Run all test scenarios sequentially

### PowerShell Scripts

- **`run_with_monitor.ps1`** - Advanced runner with performance monitoring
  - Real-time memory and CPU tracking
  - Detailed performance logs
  - Automatic result organization

## 📤 Output Files

Results are saved in `results_<timestamp>/`:
- `<Config>_output.txt` - Simulation console output
- `<Config>_stdout.txt` - Standard output
- `<Config>_stderr.txt` - Error output
- `<Config>_<timestamp>.log` - Performance summary (PowerShell only)

## 🔧 Customization

### Adding New Scenarios

1. Edit `scenarios.ini` to add new wavelength configurations
2. Update `lambdasPerISL` and port configurations
3. Save and run with `-c YourNewConfig`

### Adding New Experiments

1. Edit `experiments.ini`
2. Use `extends = BaseScenario` to inherit settings
3. Override specific parameters for your experiment

## 📚 References

- Design pattern inspired by TreeTopologyOBS example
- OBS protocol implementation from OBSModules framework
- Satellite network parameters based on LEO constellation research

## 💡 Tips

- Start with `Baseline` to understand default behavior
- Use `run_with_monitor.ps1` for performance analysis
- Check `results/` directory for scalar and vector outputs
- Compare different scenarios using the same traffic pattern

## 🐛 Troubleshooting

**Issue**: "Unassigned Parameter" errors
- **Solution**: Ensure `lambdasPerISL` is defined in your Config section

**Issue**: Memory allocation errors
- **Solution**: Check that loop ranges match wavelength counts (for i=0..lambdasPerISL)

**Issue**: Unable to run scripts
- **Solution**: For PowerShell, you may need to enable script execution:
  ```powershell
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```

---

**Last Updated**: 2025-11-27  
**Author**: OBS Satellite Network Team
