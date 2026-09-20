# walkthrough：FDL 代码与实验配置审核落地

> 日期：2026-09-15 实现；2026-09-17 用户确认编译通过；同日 Agent 跑完三分支功能测试  
> 分支：`task/code-review-2026-09`  
> 依据：`research_reports/2026-09-code-review/FDL代码与实验审核清单.md`（含 09-15 可执行性纠正）  
> 未提交

---

## 一、做了什么

未改 `src/EdgeNode/`、`params.ini`、`experiments.ini`、`tests.ini`。未改第 5 周标定数字 35.9 µs。

| ID | 落地 |
|---|---|
| B1 | `OBS_OpticalCrossConnect`：`oxcDropCount` / `oxcBurstDropped`；`outputOwner` 反向占用；`finish()` |
| B2 | ControlLogic 比较 `fdlDelayTime` 与 `fdl.delayTime`；`Test-FDL-TauMismatch`（预期失败） |
| B3 | `switchReconfigTime`；`useFDL=true` 时 τ>0 且 τ≥开关时间；`FDL-TestBase` 钉成 `0s` |
| B4 | `warmup-period=50ms` 仅在 `FDL-Scenario`；统计按 **burstArrival**；Calibrate/Hotspot 时长改为 2s |
| B5 | `fdlOffsetMode` DO/SO；SO 用 `bcpFwdDelay = processingTime+tau`；`Test-FDL-SO`；未乘进 ExpA |
| H1 | `FDL-Scenario` 的 `guardTime=2us`（assumed）；测试注释按 waitTime≈8 µs 更新 |
| H2 | `fdlEntryOccupancy` / `fdlInFlightOccupancy`；保留 `fdlUtilization` 作为入口占用的别名 |
| H3 | `maxIslChannelUtilization`、`maxIslUtilPort`（全 0 时为 -1）；未改 `calibrate_load.py` 主判据 |
| H4 | `FDL-RateCheck` 同步缩放处理时延、guard、switchReconfigTime |
| H5 | 仅增加 `ExpA-Pilot` config |
| H6.1 | `burstNodeDelay` 信号（直通 0，回环 τ） |
| H6.2 | **未做**（拆帧器在 EdgeNode） |
| H7 | BCP 追加 `fdlLoopCount`；`maxFdlLoopsPerBurst` 默认 -1 |
| H8 | 去掉 `fdlUsageCount` 等 `@statistic` 的 count/vector 别名；手写 `recordScalar` 保留 |
| L1 | **未做** |
| L2 | horizon 热路径改为 `Enter_Method_Silent` |
| L3 | `lambdasPerPort` token 数必须等于 `numPorts`；`Test-FDL-PortStringOverflow` |
| L4 | `dispatchMode` 收窄为 `**.sat*.edgeRouter.obs.assembler.dispatcher.dispatchMode` |

禁止改动的承重代码未动：`setSchedulingPriority(2)/(1)`、FDL horizon 不加 τ、W=1 冻结、TooShort 与 Off 两个 config。

---

## 二、标量名映射（H8 / H2）

`.sca` 里仍用手写名：

- 仍有：`fdlUsageCount`、`fdlUtilization`、`burstLossContention`、`burstLossTotal`
- 新增：`oxcBurstDropped`、`fdlEntryOccupancy`（与 `fdlUtilization` 同值）、`fdlInFlightOccupancy`、`maxIslChannelUtilization`、`maxIslUtilPort`、`burstsLoopedOnce`、`burstsLoopedMultiple`、`warmupPeriod`、`measurementWindow`、`outgoingOffsetMean`、`outgoingOffsetMin`、`burstNodeDelay:mean` 等
- 去掉：`fdlUsageCount:count`、`burstLossContention:count`、`burstLossTotal:count`、`fdlUtilization:last`（若原先由 `@statistic` 写出）

Compatibility 相对 pre-FDL 的「多出来的 FDL 统计条数」会变；旧行为标量应仍一致。

---

## 三、三分支功能测试（2026-09-17 已跑）

`out/gcc-release/obsmodules.exe` 当时不存在；使用当天 17:09 的 `out/gcc-debug/obsmodules.exe`。PATH 需含 `D:\omnetpp-4.6-src-windows\omnetpp-4.6\bin`、mingw32、以及 `D:\inet\out\gcc-debug\src`。三条均 exit 0。原始 `.sca` / EV 在 `Examples/RingFdlOBS/results/Test-FDL-{Basic,TooShort,Off}/`。

| 检查 | Basic | TooShort | Off |
|---|---|---|---|
| sat2 `fdlUsageCount` | 1 | 0 | 0 |
| sat2 `burstLossContention` | 0 | 1 | 1 |
| sat3 `udpApp[0] rcvdPk:count` | 2 | 1 | 1 |
| sat2 `oxcBurstDropped` | 0 | 2 | 2 |
| sat2 `burstsLoopedOnce` / `burstsLoopedMultiple` | 1 / 0 | 0 / 0 | 0 / 0 |
| sat2 `burstNodeDelay:mean` | 10 µs（直通 0 + 回环 20 µs 的均值） | 0 | 0 |
| TooShort 与 Off 的 1701 条标量值 | — | 完全相同 | 完全相同 |

Basic 的 sat2 EV：

- `OXC Reserved (FDL): 1st 3->4 (conn=0.10600412784); 2nd 4->2 (conn=0.10602412784)`，间隔恰好 τ=20 µs
- sat3 收到的两条 BCP offset：`0.000997616` 与 `0.001017616`，差恰好 20 µs

TooShort / Off 在 sat2 拒绝回环后，未编程的 `inGate=3` 上各丢 2 个光消息（突发头/尾）。这是 B1 计数后的预期，不是控制面多丢了一个突发。模块标量 `"FDL usage count"=2` 同样是头/尾各过一次 FDL，对应一次回环调度。

未跑：`Test-FDL-Compatibility`、`Test-FDL-SO`、以及预期失败的 `TauMismatch` / `PortStringOverflow`。

第 2 批改变了 guardTime 与测量窗口。重跑 `FDL-Calibrate` 之前不要把 35.9 µs 当成新的 ρ=0.4 工作点。

---

## 四、未执行

- L1 路由表哈希化
- H6.2 拆帧端到端时延（EdgeNode）
- H5 的统计分析与冻结 `repeat`
- 重跑标定并回写 k
- git commit（等功能测试通过后再提交）

---

## 五、2026-09-17：M1 复跑暴露的两个缺陷及修复

> 触发：复跑 M1 全量配置（此前只跑过 Basic/TooShort/Off）。
> 分支 `task/code-review-2026-09`；编译 `make MODE=debug` 通过；未提交。

### 5.1 缺陷 A：`Test-FDL-TauMismatch` 不可能失败（B2 的守卫不可达）

B2 加入的守卫比较 `fdlDelayTime`（CoreNode）与 `fdl.delayTime`（fdl 子模块）。但
`OBS_CoreNode.ned` 在子模块块里把它绑定为 `delayTime = fdlDelayTime`，而 **ini 覆盖不了
NED 子模块块赋值**，所以两者恒等、守卫永不触发。

三组探针（临时 `_probe_tau.ini`，已删除）：

| 探针 | ini 设置 | 结果 |
|---|---|---|
| 通配符 | `**.sat*.coreSwitch.fdl.delayTime = 7us` | 与 Basic 完全相同 → 无效 |
| 精确路径 | `RingFdlOBS.sat2.coreSwitch.fdl.delayTime = 7us` | 与 Basic 完全相同 → 无效 |
| 改父参数 | `**.sat*.coreSwitch.fdlDelayTime = 7us` | 238 条标量变化 → 有效 |

后果：`Test-FDL-TauMismatch` 正常跑完并写出标量，与 `Test-FDL-Basic` **逐条相同**（1701 条），
是一个假阳性用例。

**处理**：保留守卫（它防的是将来有人改断 NED 绑定），但改写注释说明它是**不变量断言**而非
用户输入校验；删除 `Test-FDL-TauMismatch`；补两个**真正可达**的负向用例：

| 新配置 | 触发条件 | 报错模块 |
|---|---|---|
| `Test-FDL-TauBelowSwitchTime` | τ(20µs) < `switchReconfigTime`(30µs) | `OBS_CoreControlLogic` |
| `Test-FDL-TauZero` | `useFDL=true` 且 τ=0 | `OBS_CoreControlLogic` |

### 5.2 缺陷 B：预期失败配置以 `0xC0000005` 崩溃退出

gdb 追出**三个独立根因**，都不是同一个。

**B-1 堆缓冲区溢出（真正的内存破坏）**

`OBS_CoreOutput::initialize()` 与 `OBS_CoreInput::initialize()` 把 `lambdasPerPort`
逐个 token 写进 `numPorts` 大小的数组，**循环无边界检查**。5 个 token 写进 4 个 int →
堆块被写过界；错误在后续模块才报出，析构时 `free()` 踩坏堆才崩。
gdb 证据：`warning: Heap block at 089685B8 modified at 089685D0 past requested size of 10`。

**处理**：照 `OBS_CoreOutputHorizon` 已有的写法补上界校验与 token 计数校验。报错点因此
提前到**越界写之前**（`Test-FDL-PortStringOverflow` 的报错模块由 GatesHorizon 变成
`OBS_CoreOutput`，即正确的第一现场）。

**B-2 析构函数释放未初始化指针**

任何模块 `initialize()` 未运行或中途 `opp_error` 时，其后（以及后续节点）的模块析构函数
照样执行，却去 `free()`/`delete` 未初始化成员。逐模块补构造函数置 NULL（或置 0 作循环上界）：

| 目录 | 模块 | 危险操作 |
|---|---|---|
| CoreNode | `OBS_CoreOutputHorizon` | `free(horizon[i])` 循环 |
| CoreNode | `OBS_CoreControlLogic` | 4 个计数器 `free()` |
| CoreNode | `OBS_CoreInput` | 3 个数组 `free()` |
| CoreNode | `OBS_CoreOutput` | `gate2Colour` 行循环 + 3 个数组 |
| CoreNode | `OBS_OpticalCrossConnect` | 2 个表 `free()` |
| CoreNode | `OBS_EOConverter` | **`for(i=0;i<numPorts;i++) delete BCPqueues[i]`** |
| EdgeNode | `OBS_PacketBurstifier` | `cancelAndDelete(timeout_msg)` |
| EdgeNode | `OBS_BurstSender` | `free(horizon/colour)` + `numLambdas` 次 `delete` |
| EdgeNode | `OBS_PacketDispatcher` | `if (numQueues != 0) delete[] …` |

`OBS_EOConverter` 是最后一块拼图：它的类定义内联在 `.cc` 里（无独立 `.h`），三个成员全无
初始化。修它之前，负向用例**约 1/6 概率崩溃**（`gdb` 14 次未复现，改用 20 次重复才暴露）；
修完后 20/20 稳定。OMNeT++ 4.6 的 `cancelAndDelete()` 会判空，`free(NULL)`/`delete[] NULL`
安全，故析构体本身未改语义。

**B-3 析构循环边界用了「已赋值但数组尚未分配」的计数**

`OBS_CoreOutput::initialize()` 先执行 `numPorts = par("numPorts")`，之后才分配
`gate2Colour`；B-1 的新校验在这两者**之间**报错，于是 `numPorts` 已置位而 `gate2Colour`
仍为 NULL，析构的 `for(i=0;i<numPorts;i++) free(gate2Colour[i])` 解引用 NULL。
**处理**：守卫改为看数组指针 `if(gate2Colour != NULL)`，不看计数。

### 5.3 EdgeNode 触碰说明（受专利保护基线）

按用户 2026-09-17 明确批准修 3 个模块。改动**仅为构造函数把成员置 NULL/0**：
不涉及四种调度模式语义、不改默认行为、不改任何调度判定。下述正常配置零差异回归即为证据。

### 5.4 验证结果

正常配置 —— 与修复前归档基线 `results/M1-2026-09-17/` 逐标量对照：

| 配置 | 退出码 | 共有标量 | 值差异 | 仅新增 | 仅基线有 |
|---|---|---|---|---|---|
| `Test-FDL-Basic` | 0 | 1701 | **0** | 0 | 0 |
| `Test-FDL-TooShort` | 0 | 1701 | **0** | 0 | 0 |
| `Test-FDL-Off` | 0 | 1701 | **0** | 0 | 0 |
| `Test-FDL-SO` | 0 | 1701 | **0** | 0 | 0 |

- `TooShort` 与 `Off` 互相对照：1701 条，**0 差异**
- `Test-FDL-Compatibility` 对 `pre-fdl-baseline/ICMPTest-0.sca`：共有 1332，值差异 72 条
  **全部是 `"my ID"`**（加 `fdl` 子模块后模块编号后移），非 `my ID` 差异 **0**，
  仅在 pre-FDL 有 **0** 条
- 负向用例：3 个配置 × 20 次重复 = **60/60 干净退出 exit=1**，报错信息与模块均正确

附带改善：失败运行此前留下 **0 字节 `.sca` 残file**，现在写出有效的小 `.sca`
（1068–1765 B，含 run 头与错误信息）。

### 5.5 运行环境（供后续复现）

编译需要 MSYS 工具链在 PATH 上：
`omnetpp-4.6\tools\win32\usr\bin`（`make`、`sh`）+ `…\tools\win32\mingw32\bin` + `omnetpp-4.6\bin`。
仿真另需 `D:\inet\out\gcc-debug\src`（`libinet.dll`）。已加两个包装脚本免去手工设 PATH：
`Examples/RingFdlOBS/tools/run-sim.cmd` 与 `run-sim.ps1`（`.ps1` 需
`powershell -ExecutionPolicy Bypass`，本机无 `pwsh`）。三个目录也已追加到用户级 PATH，
但**只对新启动的进程生效**。

### 5.6 本次未执行

- L1、H6.2、H5 仍如上节未做
- 未跑 release 构建（本次只编 `MODE=debug`）
- `colours` 参数的两个 tokenizer 循环同样无边界检查，但所有配置里 `inputColours` /
  `outputColours` / `outColours` 均为 `""`，该分支**休眠**，未修（已记录）
- 未提交 git
