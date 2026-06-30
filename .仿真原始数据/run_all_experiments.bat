@echo off
REM ========================================
REM  OBSmodules - Batch Experiment Runner
REM  全光交换仿真平台批量实验脚本
REM ========================================

set OMNET_ROOT=D:\Download\omnetpp-4.6-src-windows\omnetpp-4.6
set PATH=%OMNET_ROOT%\bin;%PATH%
set SIM_EXE=..\obsmodules.exe

echo ========== Running FlowScaling (24 runs) ==========
mkdir results\FlowScaling 2>nul
for /L %%i in (0,1,23) do (
    echo   Run #%%i / 23...
    %SIM_EXE% -u Cmdenv -f experiments.ini -c FlowScaling -r %%i
)
echo FlowScaling completed.

echo ========== Running LoadIntensity (20 runs) ==========
mkdir results\LoadIntensity 2>nul
for /L %%i in (0,1,19) do (
    echo   Run #%%i / 19...
    %SIM_EXE% -u Cmdenv -f experiments.ini -c LoadIntensity -r %%i
)
echo LoadIntensity completed.

echo ========== Running QueueSensitivity (16 runs) ==========
mkdir results\QueueSensitivity 2>nul
for /L %%i in (0,1,15) do (
    echo   Run #%%i / 15...
    %SIM_EXE% -u Cmdenv -f experiments.ini -c QueueSensitivity -r %%i
)
echo QueueSensitivity completed.

echo ========== Running TCPThroughput (4 runs) ==========
mkdir results\TCPThroughput 2>nul
for /L %%i in (0,1,3) do (
    echo   Run #%%i / 3...
    %SIM_EXE% -u Cmdenv -f experiments.ini -c TCPThroughput -r %%i
)
echo TCPThroughput completed.

echo ========== All experiments completed ==========
pause
