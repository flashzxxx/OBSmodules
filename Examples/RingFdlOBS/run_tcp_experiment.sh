#!/bin/bash
# Run TCPThroughput experiment: 4 modes (0-3), 1 run each = 4 total
OMSIM="../../out/gcc-debug/src/obs-modules"
NET="obsmodules.Examples.RingFdlOBS.RingFdlOBS"

for run in 0 1 2 3; do
    echo "=== TCPThroughput run $run ==="
    $OMSIM -u Cmdenv -f omnetpp.ini -f experiments.ini -c TCPThroughput -r $run -n "../../src;." --result-dir=results/TCPThroughput
done

echo "All TCPThroughput runs complete."
