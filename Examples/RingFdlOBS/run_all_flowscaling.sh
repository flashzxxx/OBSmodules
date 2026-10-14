#!/bin/bash
# ============================================
#  FlowScaling Batch Runner (24 runs: 0..23)
#  4 modes x 6 flow counts
#  Run this from OMNeT++ MinGW shell!
# ============================================

cd /d/01work/project/OBSmodules/Examples/RingFdlOBS

echo "Starting batch run..."
echo ""

for i in $(seq 0 23); do
    echo "=== Run $i / 23 ==="
    ../../out/gcc-debug/obsmodules.exe -u Cmdenv -c FlowScaling -r $i -n "../../src;."
    echo "=== Run $i completed ==="
    echo ""
done

echo "============================================"
echo " All 24 runs completed!"
echo "============================================"
