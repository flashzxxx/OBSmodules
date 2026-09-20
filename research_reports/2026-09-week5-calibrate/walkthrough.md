# 第 5 周 walkthrough：实验平台标定（`FDL-Calibrate`）

> 日期：2026-09-04；2026-09-09 修正 `UDPBasicApp` 的 `stopTime`
> 范围：W=1 全量冻结、标定配置与脚本、`portLambdas` 运行时核对；仿真由用户执行
> 第 4 周 ICMP 对照见 `research_reports/2026-08-week4-compatibility/`
> 对照源码把机制和标定读懂：`research_reports/2026-09-week5-calibrate/FDL机制与实验平台说明.md`
> 根目录 `walkthrough.md` 已改为第 6 周文献定位；本文件保留第 5 周原文。

---

## 一、本次要解决的问题

实验 A/B 的负载标签目前是解析估算。`fdl_params.ini` 里的 `42.6us` 不能当成已经达到 ρ=0.4。第 5 周要把发送间隔和实测最忙 ISL 口利用率对齐，并核对三件平台条件：W=1 真正生效、流量是多源全互联、换种子结果确有差异。

## 二、代码与配置

未改 `src/EdgeNode/`。调度判断未改：`useFDL=false` 路径不变。`portLambdas[]` 只在 `finish()` 里多记一个标量，供标定脚本拒绝“仍继承了 `params.ini` 的 3 波长”的运行。

| 文件 | 变更 |
|---|---|
| `src/CoreNode/OBS_CoreControlLogic.cc` | `finish()` 增加 `portLambdas[p]` |
| `Examples/RingFdlOBS/fdl_params.ini` | 13 条 ISL 与 10 个节点的 `lambdasPerInPort/OutPort` 全部冻结为 1；`stopTime` 改为 `-1s`（INET 表示一直发到仿真结束） |
| `Examples/RingFdlOBS/fdl_experiments.ini` | `FDL-Calibrate` 增加 `repeat=2`、`seed-set=${runnumber}`（7×2=14 run） |
| `Examples/RingFdlOBS/tools/calibrate_load.py` | 拟合 k；同时检查 W=1 / 多源 / 随机化 / FDL 关闭 |

原先 `FDL-Scenario` 只改 sat1/sat2。`params.ini` 的 `lambdasCore1to2 = 3` 正是最忙那条链路；其余节点虽然已经是 1，但不写进 `FDL-Scenario` 就会继续吃 General。本周把全部波长参数写死在 FDL 场景里。

Agent 未编译、未跑仿真。新增 `portLambdas[]` 后必须先 `make MODE=release`，再跑标定。

2026-09-09：用户首次启动 `FDL-Calibrate` 时，`UDPBasicApp` 在初始化阶段报 `Invalid startTime/stopTime parameters`。原因是本场景把 `stopTime` 写成了 `0s`，INET 把 0 当成“在 0 秒停止”，而 `startTime = uniform(0s, 1ms)` 几乎总是大于 0。已改为 `-1s`（该模块 NED 默认值，表示发到仿真结束）。这只改 ini，不必为这一处再编译。

## 三、静态核对（不依赖仿真）

| 条件 | 静态证据 | 运行时如何确认 |
|---|---|---|
| W=1 | `fdl_params.ini` 全部 `lambdasCore*=1`，全部端口串为 `"1 … 1"` | `.sca` 里每个 `portLambdas[p]==1` |
| 多源 | 10 星各有 UDPBasicApp，`destAddresses` 列为其余 9 星 | 10 星 `burst sent>0`，且每个核心 `burstsReceived>0` |
| 随机化 | `exponential(...)` + `startTime=uniform(0s,1ms)` | 同一 `meanInterval` 的两个 seed，ρ / 发帧 / 收帧不全相同 |
| FDL 关闭 | `FDL-Calibrate` 设 `useFDL=false` | 全网 `fdlUsageCount=0` |

明细：`research_reports/2026-09-week5-calibrate/分析.md`。

## 四、用户需要执行的命令

在仓库根目录编译（本周改了 `OBS_CoreControlLogic.cc`）：

```text
make MODE=release
```

在 `Examples/RingFdlOBS/` 下跑 14 个标定 run，然后拟合：

```text
..\..\out\gcc-release\obsmodules.exe -u Cmdenv -f omnetpp.ini -c FDL-Calibrate -n "../..;.;D:/inet/src"
python tools/calibrate_load.py results/FDL-Calibrate --report ..\..\research_reports\2026-09-week5-calibrate\calibrate_report.txt
```

预期：`results/FDL-Calibrate/` 下出现 `FDL-Calibrate-0.sca` … `FDL-Calibrate-13.sca`。把脚本打印的 ρ=0.4 那一行发给我，或直接把该目录放进仓库，我再回写 `fdl_params.ini` 和 `ExpB-Joint` 的间隔列表。

## 五、仿真结果（2026-09-09）

命令行一次跑完 14 个 run（#0–#13），约 2 分钟。原始数据：`Examples/RingFdlOBS/results/FDL-Calibrate/`。

| 检查 | 结果 |
|---|---|
| 14 个 `.sca` | 齐全 |
| W=1 `portLambdas[]` | 36 条全部为 1 |
| 10 星均发出突发 | 通过 |
| 双种子结果有差异 | 通过 |
| 全网 `fdlUsageCount=0` | 通过 |
| ρ=0.4 发送间隔 | 实测曲线插值 **35.9 µs**（原估算 42.6 µs 只到 ρ≈0.35） |
| ρ=0.8 | 本扫描达不到；最重点 21.3 µs 只有 ρ≈0.59 |

枢纽节点在信道利用率只有 4% 时就会有百分之几的突发竞争丢包，这是 W=1 的特性，不是扫描失败。已把 `fdl_params.ini` 的默认间隔改成 35.9 µs。

## 六、未执行事项

1. 未提交。
2. 未跑实验 A。ρ=0.6/0.8 需要更短间隔或接受 ρ≈0.59 为最重点。
