@echo off
echo ============================================
echo  FlowScaling Batch Runner (24 runs: 0..23)
echo ============================================

set OMNET_ROOT=D:\Download\omnetpp-4.6-src-windows\omnetpp-4.6
set WD=%OMNET_ROOT%\tools\win32\usr\bin\
set MSYSTEM=MINGW32
set PATH=%OMNET_ROOT%\bin;%OMNET_ROOT%\tools\win32\mingw32\bin;%OMNET_ROOT%\tools\win32\usr\bin;%PATH%
set HOME=%OMNET_ROOT%

cd /d d:\01work\project\OBSmodules\Examples\RingFdlOBS

"%WD%bash.exe" --login -c "cd /d/01work/project/OBSmodules/Examples/RingFdlOBS && source /d/Download/omnetpp-4.6-src-windows/omnetpp-4.6/setenv && for i in $(seq 0 23); do echo '=== Run '$i' / 23 ==='; ../../out/gcc-debug/obsmodules.exe -u Cmdenv -c FlowScaling -r $i -n '../../src;.'; done && echo 'ALL 24 RUNS COMPLETED'"

pause
