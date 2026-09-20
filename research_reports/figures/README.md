# 图表产物

`Examples/RingFdlOBS/results/` 下的原始仿真输出（`.sca` / `.vec`，本机约 1.1 GB）只保留在本地，不入版本库。
本目录只存放已经进入分析流程的成图与表格，均为对应仿真输出的逐字节副本（SHA256 与来源文件一致）。

| 文件 | 本地来源目录 | 生成脚本 | 批次日期 | 内容 |
|---|---|---|---|---|
| `expA_tau_sweep.png` | `results/ExpA-TauSweep/` | `tools/plot_expA_tausweep.py` | 2026-09-18 | 实验 A：丢包率与 FDL 使用率随 τ/T_burst 的变化（5 τ × 2 负载 × repeat 5） |
| `phase1_four_modes.png` | `results/task1.3-2026-09-18/` | `tools/plot_phase1_modes.py` | 2026-09-18 | 第一阶段四种调度模式（Static / NoPreemption / Dynamic / RoundRobin）对比 |
| `hotspot_table.csv` | `results/FDL-Hotspot-calib-2026-09-18/` | `tools/analyze_hotspot.py` | 2026-09-18 | 热点负载标定表：发包间隔 → ρ、最忙节点与端口、丢包率 |
| `fdl_loop_probe.png` | `results/_loop-probe/` | 本地临时脚本（未入库） | 2026-09-18 | `maxFdlLoopsPerBurst` 配对探针（同种子、只改该参数） |

生成脚本属于本机工作文件，保存在 `Examples/RingFdlOBS/tools/`，不在版本库内。重新生成图表需要对应的
实验配置与原始 `.sca` 结果文件。
