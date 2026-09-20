# FDL 功能代码与实验配置审核清单

> 日期：2026-09-15
> 审核范围：`src/CoreNode/` 全部 FDL 相关实现与 NED、`Examples/RingFdlOBS/` 的 `fdl_params.ini` / `fdl_tests.ini` / `fdl_experiments.ini`，以及 `params.ini` 中被继承的相关项
> 审核方式：静态阅读与时序验算，未编译、未运行仿真
> 用途：派发给执行模型；每一项都给出位置、问题、要求的改动和验收判据
> 约束：编译与仿真一律由用户执行；执行模型不运行 `make`、不启动 OMNeT++

---

## 零、执行前必读

1. **不得修改 `src/EdgeNode/`。** 本清单所有条目都不需要触碰边缘节点。
2. **不得改变 `useFDL=false` 的行为。** 每一项改完后，`Test-FDL-Off` 与 `Test-FDL-Compatibility` 必须仍然与既有基线逐标量一致（新增标量除外）。这是硬验收条件。
3. **不得新增调度策略。** 本清单只涉及计数器、断言、参数校验、统计定义与实验配置；`useFDL` 仍然只有 false/true 两种模式。
4. **分支命名**：`task/code-review-2026-09`；提交前不合并、不打标签。
5. **完成后产出 `walkthrough.md`**，列明每条的改动、未执行事项和需要用户验证的命令。
6. 下面第五节列出了**已复核确认正确、禁止"顺手清理"**的代码，改动时请避让。
7. **Agent 不运行 `make` / OMNeT++。** 凡验收写「跑某某 config」的，Agent 只改代码与 ini、在 `walkthrough.md` 列出用户命令；不得把「用户尚未跑的仿真」写成已通过。
8. 2026-09-15 可执行性复核见下一节。原文有几条按字面执行会改错语义或验收必然失败，已在对应条目中改成可执行口径。

---

## 〇、可执行性核对（2026-09-15）

结论：**不能整份原样派发。** 第 1 批（B1/B2/B3）可以立刻做；其余必须按下面改过的口径，否则会引入比原问题更大的缺陷。

| ID | 能否直接执行 | 原因 |
|---|---|---|
| B1 | 能 | OXC 目前无 `finish()`，须同时改 `.h` 声明。同刻交接（unschedule 优先于 schedule）必须仍成立，反向占用表不得把合法交接判成冲突 |
| B2 | 能（反向验证改法见条目） | 临时改 ini 再删容易误提交。改为增加「预期失败」config，默认不跑 |
| B3 | 能，但须给测试开豁免 | `FDL-TestBase` 必须 `switchReconfigTime = 0s`，否则日后把该值接到 2 µs 时，`Test-FDL-TooShort` 的 τ=2 µs 会在 `initialize()` 直接 `opp_error` |
| B4 | 代码能做；「重跑标定 / ρ 必升高」不能作为 Agent 验收 | 计数门槛须用 **burstArrival** 而非 BCP 的 `simTime()`。与 H1 合并后 ρ **不一定**高于第 5 周旧值。标定由用户跑 |
| B5 | **原文不能执行** | 原文 SO（只改 offset、BCP 仍立刻转发）会让下游按未延迟时刻配 OXC，突发晚到 τ，等于人为制造 case-1 丢包。正确 SO 是 **BCP 也多等 τ**。不要把 SO 立刻乘进 `ExpA-TauSweep` |
| H1 | 能，须收口选择 | 「或加敏感性维度或另跑一组」不是可执行指令。本批只在 `FDL-Scenario` 写死 `guardTime = 2us`（assumed），**不**加入 ExpA 迭代 |
| H2 | 能 | 优先双写旧名 `fdlUtilization`，避免分析脚本与第 3/4 周对照失明 |
| H3 | 代码能做；验收原文错误 | `calibrate_load.py` **已经**用 `busiest(skip_edge_port=True)` 跳过 port0，不必改判据。Hotspot 下 sat2 的 `maxChannelUtilization` 落在 port0 是预期现象，不是失败 |
| H4 | 能（已选定方案一） | 「二选一」已锁死为方案一 |
| H5 | **不在 Agent 范围** | 必须先有用户跑的 pilot `.sca`。Agent 只准备一个 `ExpA-Pilot` config |
| H6 | 仅 H6.1 能做 | 拆帧器在 `src/EdgeNode/OBS_BurstDisassembler.cc`。H6.2 触碰受保护基线，本批不做 |
| H7 | 能，改参数声明 | `@unit()` 不能加在无量纲 int 上。用 `int maxFdlLoopsPerBurst = default(-1)` |
| H8 | 能 | 删的是 `@statistic` 的 count/vector 别名，手写 `recordScalar` 保留。Compatibility 相对 pre-FDL 的「多 235 条」计数会变，只要求旧行为标量不变 |
| L1 | **本批禁止** | 路由含 `*` 通配，且匹配顺序是队列从后往前。改哈希表极易改掉命中结果。推迟到有纯回归预算时另写方案 |
| L2 | 能 | OMNeT++ 4.x 的 `Enter_Method_Silent()` 无格式化参数 |
| L3 | 能（循环条件已改正） | 原文 `i <= numPorts` 会允许写到 FDL 槽。应为 token 数必须 **等于** `numPorts`，循环 `i < numPorts` |
| L4 | 能 | 收窄路径已写明，勿猜 |

**Agent 本轮允许的范围**：第 1 批 B1/B2/B3 + 第 2 批中的 ini/代码（H1 的 guardTime、B4 的 warmup 实现）+ 第 3 批中不乘进 ExpA 的部分（B5 参数与测试 config、H2 双写、H3 新标量、H8）。**禁止**：L1、H6.2、H5 的统计分析、把 SO/guardTime 乘进 `ExpA-TauSweep`、自行重跑标定并改写 ρ 数字。

---


## 一、阻塞项：实验 A 开跑之前必须全部完成

### B1 OXC 静默丢包与输出冲突无检测

**位置**：`src/CoreNode/OBS_OpticalCrossConnect.cc:39-46`（`handleMessage`）、`:48-53`（`setGate`）

**问题**：输入门未配置时 `delete msg`，不记标量、不发信号、不打 EV。加 FDL 后每个回环突发有两个预约窗口，时序错配机会翻倍，而丢失的突发无任何痕迹，导致 `burstLossTotal` 与端到端实收数对不上且差额不可解释。此外 `schedulingTable` 只按输入门去重，两个不同输入门可同时映射到同一输出门，两个突发都会被 `send` 出去并在同一波长重叠，无报错无计数——任何调度 bug 会产生偏乐观的错误结果而非崩溃。

**要求**：
1. 类目前没有 `finish()`，须在 `OBS_OpticalCrossConnect.h` 声明并实现。新增计数器 `oxcDropCount`，在未配置分支自增，`finish()` 中 `recordScalar("oxcBurstDropped", ...)`；同时打一条 EV，包含到达门号与 `simTime()`。
2. `setGate(inGate, outGate)` 增加输出门占用检查：维护输出门反向占用表（建议 `int* outputOwner`，大小 `gateSize("out")`，初值 -1）。若目标输出门已被**另一个**输入门占用则 `opp_error`，错误信息带两个输入门号与输出门号。同一输入门重复 `setGate` 仍走现有 `opp_error`。
3. `unsetGate` 同步把该输入门对应的输出门恢复为 -1。
4. **合法同刻交接不得误报**：`destDisconnectTime` 与下一突发的 `destConnectTime` 经常相等，靠 `setSchedulingPriority`（unschedule=1 先于 schedule=2）先拆后接。反向表必须在 `unsetGate` 之后才清空，这样同刻事件是「先释放再占用」，不是冲突。
5. 析构时 `free` 新表，与现有 `schedulingTable` 对称。

**验收判据**：
- `Test-FDL-Basic`：`oxcBurstDropped == 0`（全节点）。
- `Test-FDL-TooShort` / `Test-FDL-Off`：`oxcBurstDropped == 0`。
- `Test-FDL-Compatibility`：`oxcBurstDropped == 0`，且其余旧标量与 pre-FDL 基线一致。
- 任一测试触发新增 `opp_error` 即视为发现真实缺陷，须先定位再继续，不得放宽断言。

---

### B2 τ 双份配置可无声失配

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:61-63`（读 `coreNode->par("fdlDelayTime")`）、`src/CoreNode/OBS_FiberDelayLine.cc:26`（读自身 `delayTime`）、`src/CoreNode/OBS_CoreNode.ned:77-81`（NED 绑定）

**问题**：调度器用的 τ 与光纤实际延迟是两个独立参数读取点，NED 默认绑定一致，但 `fdl_params.ini:111` 已存在 `**.sat*.coreSwitch.**.dataRate` 这类 `coreSwitch.**` 通配写法；一旦有人照此风格写 `**.coreSwitch.**.delayTime`，调度器按 τ_logic 预约出口而光纤按 τ_fdl 延迟，突发到达时 OXC 未配置，被 B1 的静默丢包吃掉，表面上看不出任何异常。

**要求**：在 `OBS_CoreControlLogic::initialize()` 中取 `getParentModule()->getParentModule()->getSubmodule("fdl")` 的 `delayTime`，与自身 `tau` 比较，不等则 `opp_error`，打印两个值。`useFDL=false` 时也检查（模块始终存在，避免以后误配）。

**反向验证（不要改完再删 ini）**：在 `fdl_tests.ini` 增加 `[Config Test-FDL-TauMismatch]`，`extends = Test-FDL-Basic`，并设 `**.sat*.coreSwitch.fdl.delayTime = 7us`。文件头注释写明：**预期 initialize 即 `opp_error`，不列入常规回归。** 不要把该 config 写进任何批量脚本。

**验收判据**：
- 三个常规功能测试正常通过。
- `walkthrough.md` 写明用户验证命令：跑 `Test-FDL-TauMismatch` 必须无法完成 initialize。

---

### B3 τ 参数缺乏合法性校验

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:62-63`、`src/CoreNode/OBS_CoreNode.ned:34-35`

**问题**：`useFDL=true` 时 τ 没有任何下界检查。τ=0 会让第二次预约与第一次同时刻发生，可能与活动预约冲突；τ 小于 OXC 开关重构时间在物理上无意义，而"载荷开关参数是否允许这一级缓存"正是论文主张的核心。

**要求**：
1. `useFDL=true` 且 `tau <= 0` 时 `opp_error`。
2. 在 `OBS_CoreNode.ned` 新增 `double switchReconfigTime @unit(s) = default(0s)`。`useFDL=true` 且 `tau < switchReconfigTime` 时 `opp_error`（用严格 `<`，等于时通过）。默认 0 保证旧配置与第 1 批行为不变。
3. **`FDL-TestBase` 必须显式 `**.sat*.coreSwitch.switchReconfigTime = 0s`**，与 H1 的 2 µs 解耦。否则 `Test-FDL-TooShort` 的 τ=2 µs 会在以后把开关时间接到 2 µs 时无法启动。
4. 比较在 `OBS_CoreControlLogic::initialize()` 读取 CoreNode 参数，不要在 FDL 子模块重复读。

**验收判据**：第 1 批默认配置下三个功能测试通过；`useFDL=false` 时即便 τ 非法也不报错。

---

### B4 标定与实验的测量窗口不一致，且无 warm-up 排除

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:382-430`（`finish()` 中除以 `simTime()`）、`Examples/RingFdlOBS/fdl_params.ini:196-201`（`sim-time-limit = 2s`）、`fdl_experiments.ini:57`（Calibrate `0.5s`）、`:182`（Hotspot `0.5s`）

**问题**：`channelUtilization[i] = portBusyTime[i] / (simTime() × lambdas)`，分母含全部瞬态。每跳 5 ms 传播使最长路径瞬态约 25 ms：在 2 s 窗口占约 1%，在 0.5 s 窗口占约 5%。也就是说 interval→ρ 的拟合是在 5% 偏置下完成的，却被用到 1% 偏置的实验里，映射不可迁移；这也是 ρ=0.8 上不去的原因之一。

**要求**（Agent 做代码与 ini；标定由用户跑完再回写数字）：
1. 在 `FDL-Scenario` 设 `warmup-period = 50ms`。**不要**写进 `omnetpp.ini` 的 `[General]`，否则 `Test-FDL-Compatibility` / 第一阶段配置会被误伤。OMNeT++ 4.x 读取用 `simulation.getWarmupPeriod()`。
2. 是否计入测量窗口，用 **`burstArrival >= warmupPeriod`**，禁止用 BCP 到达时刻 `simTime()`。BCP 比突发早一个 offset（实验里 0.5–1 ms），用错门槛会把窗口边界上的突发算漏。
3. `finish()` 利用率分母为 `simTime() - warmupPeriod`（若差 ≤ 0 则记 0 并 `EV_WARN`）。`portBusyTime` / `busyTime` / `portCarriedBytes` 以及 `recvBurstCounter` / `schedBurstCounter` / `dropCounter` / `fdlUsageCount` / `burstLossContention` 一律只统计窗口内突发。`recvBurstCounter` 目前在 offset 检查之前就自增，改的时候不要漏。
4. `FDL-Calibrate`、`FDL-Hotspot`、`ExpA-*`、`ExpB-Joint`、`ExpR-Retransmit` 的 `sim-time-limit` 统一为 `2s`（Calibrate/Hotspot 现为 0.5s，须改）。`FDL-RateCheck` 可保留较短时长，注释写明其 ρ 不可与 2s 配置直接比。
5. `finish()` 记录标量 `warmupPeriod`、`measurementWindow`。
6. `Test-FDL-Compatibility` 不继承 `FDL-Scenario`，无需再关 warmup。若将来有人把 `warmup-period` 放进 `[General]`，在该 config 显式 `warmup-period = 0s`。

**验收判据（Agent）**：三个 `Test-FDL-*` 的突发在 0.1s，大于 50 ms，判据应仍成立（用户跑）。**不要**把「ρ 必高于第 5 周」写进通过条件：与 H1 同时改 guardTime 后，ρ 可能升也可能降。用户重跑 `FDL-Calibrate` 后，由用户（或后续任务）回写 k；Agent 本批只在 `fdl_params.ini` 注释标明「待 warm-up/guardTime 后重标定，35.9 µs 数字暂保留为第 5 周测量」。

---

### B5 τ 扫描中的 offset 混淆（实验设计，必须在实验 A 前定口径）

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:358`（`bcp->setBurstArrivalDelta(arrivalDelta + tau - processingTime)`）、`fdl_experiments.ini:90-99`（`ExpA-TauSweep`）

**问题**：回环后 BCP 的 offset 增加 τ，这是 ChinaCom 2010 意义上的 DO 模式，实现正确。但副作用是回环过的突发到下一跳时 offset 变宽，因而**在下游更容易被调度成功**。τ=4·T_burst 时每次回环给下游多约 137 µs offset。于是 τ 越大曲线越好看，其中一部分收益与"光域暂存缓解竞争"无关。ChinaCom 2010 明确比较过 DO 与 SO 并发现差异巨大（负载 0.8 时 70% vs 23%），只报 DO 会被直接指出 τ 效应与 offset 效应未分离。

**要求**（纠正 2026-09-15：原文实现是错的）：
1. 在 `OBS_CoreNode.ned` 新增 `string fdlOffsetMode = default("DO");` 合法值仅 `"DO"` / `"SO"`，其它值 `opp_error`。默认 DO = 当前行为（BCP offset `+ τ`，BCP 仍只 `sendDelayed(..., processingTime)`）。
2. **正确的 SO（ChinaCom Stay-Offset）**：突发在 FDL 里多走 τ 的同时，**BCP 也多等 τ** 再转发，下游看到的 offset 与直通相同。
   - 回环分支：`bcp->setBurstArrivalDelta(arrivalDelta - processingTime);`（与直通相同）
   - 回环分支 BCP 转发延迟为 `processingTime + tau`
   - 本节点两次 OXC 预约仍按 `burstArrival` 与 `burstArrival+tau`，与现在相同
   - **实现注意**：现在 `sendDelayed(bcp, processingTime, "out")` 在两个分支之后共用（约第 370 行）。SO 不能只改 delta。用局部变量 `bcpFwdDelay`（直通/`DO` 回环 = `processingTime`；`SO` 回环 = `processingTime + tau`），最后仍只调用一次 `sendDelayed`。
3. **禁止**只改 offset 却仍 `sendDelayed(..., processingTime)`：那样突发晚到 τ，下游 OXC 窗口对不上，会大量走 case-1 丢包或 OXC 静默丢包。
4. **不要**把 `offsetMode` 乘进 `ExpA-TauSweep`（会把 run 数立刻翻倍）。另加 `[Config Test-FDL-SO]`：`extends = Test-FDL-Basic`，`**.sat*.coreSwitch.fdlOffsetMode = "SO"`。
5. 对成功转发出去的 BCP 记录 `outgoingOffsetMean`、`outgoingOffsetMin`（累加在 `sendDelayed` 之前读取更新后的 delta）。

**验收判据**：
- 默认 DO：三个原功能测试标量与现网一致（用户跑）。
- `Test-FDL-SO`：`fdlUsageCount > 0` 且 `burstLossContention == 0`；EV 中 BCP 的 offset 字段相对直通公式、不 `+τ`；BCP 离开本节点的时刻比 DO 晚 τ。用户核对 EV。
- 若 SO 下 sat3 少收包或 `oxcBurstDropped>0`，视为实现错误，不是去加大 `minOffset` 掩盖。

---

## 二、高优先：实验 A 之前最好一并完成

### H1 `guardTime = 1ns` 物理上站不住

**位置**：`Examples/RingFdlOBS/params.ini:54`（`**.sat*.coreSwitch.**.guardTime = 0.000000001s`）

**问题**：真实 OXC 的保护间隔不应小于开关重构时间（Zhao 2022 用 T_G=5 µs、开关配置约 2.3 µs；Mouammar 2026 器件表中电光开关 0.1 µs、MEMS 毫秒级）。1 ns 保护带把可达信道利用率抬到物理不可达水平。一篇论证载荷开关可行性的论文若使用 1 ns 保护带，审稿人会立刻发现。

**要求**（选择已锁死，不要再「或」）：
1. 不修改 `params.ini`。在 `FDL-Scenario` 写：
   `**.sat*.coreSwitch.**.guardTime = 2us`
   `**.sat*.coreSwitch.switchReconfigTime = 2us`
   注释标明 assumed，来源 Zhao 2022 开关配置量级，待 D1 修正。
2. **本批不要**把 guardTime 加进 `ExpA-TauSweep` 迭代。敏感性对照另开以后的 `[Config ExpA-GuardSweep]`，现在只写注释占位即可。
3. `FDL-TestBase` 保持 `switchReconfigTime = 0s`（见 B3）。测试可继承 2 µs guardTime：waitTime 从约 6.5 µs 变为约 8 µs，τ=20 µs / 2 µs 仍分别走回环/丢弃。更新 `fdl_tests.ini` 注释中的 waitTime 推导。
4. 第 1 批不要改 guardTime（否则与「不改数值行为」冲突）。guardTime 只在第 2 批改。

**验收判据**：
- Compatibility 不继承 `FDL-Scenario`，guardTime 仍为 `params.ini` 的 1 ns。
- 更新 `fdl_tests.ini` 注释：guardTime=2 µs 时 waitTime ≈ 8 µs；τ=20/2 µs 分支不变则不必改 τ。
- 标定由用户在第 2 批之后重跑，Agent 不回写数字。

**副作用提示**：提高 guardTime 会降低可达吞吐、增加竞争，**有助于把瓶颈 ρ 推到 0.8**，与 H3 目标一致。

---

### H2 `fdlUtilization` 定义与名称不符

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:352`（`busyTime += burstDuration`）、`:382-389`（`finish()`）

**问题**：当前量为"FDL 入口被预约的时长占仿真时长的比例"，而物理上突发在延迟线内飞行 τ + burstDuration。名为 utilization 会在论文中被质疑。

**要求**：
1. 新增 `fdlEntryOccupancy` 与 `fdlInFlightOccupancy`（后者累加 `burstDuration + tau`）。两者都受 B4 窗口约束。
2. **保留**手写 `recordScalar("fdlUtilization", ...)`，值与 `fdlEntryOccupancy` 相同，避免 `calibrate_load.py` 以外的旧对照失明。`walkthrough.md` 写明三名对应关系。
3. `.ned` 的 `@statistic[fdlUtilization]` 按 H8 处理，不要只改名却留两套 count。

**验收判据**：`Test-FDL-Basic` 中 `fdlEntryOccupancy` 与 `fdlInFlightOccupancy` 均 > 0，且后者严格大于前者；`Test-FDL-Off` 中两者均为 0。

---

### H3 瓶颈口归属未校验，`FDL-Hotspot` 可能测错对象

**位置**：`fdl_experiments.ini:167-185`（`FDL-Hotspot`）、`src/CoreNode/OBS_CoreControlLogic.cc:408-430`（`maxChannelUtilization`）

**问题**：九个源打向 sat2，但 sat2 入向 ISL 只有三条（port1=sat1、port2=sat3、port3=sat5），真正的竞争发生在**上游** sat3、sat5 争往 sat2 的链路上，起作用的 FDL 是它们的而不是 sat2 的。更严重的是 `maxChannelUtilization` 遍历所有输出口，sat2 的 port0 是通向本地 host 的边缘口，很可能成为最大值——那是一个没有物理意义的 ρ。

**要求**：
1. `finish()` 中 port0 = 本地边缘口，其余 = ISL。新增 `maxIslChannelUtilization`（ISL 口最大值）与 `maxIslUtilPort`（取到该最大值的端口号；若所有 ISL 口利用率为 0，记 -1，不要记 0，以免和「瓶颈在 port0」混淆）。保留 `maxChannelUtilization`。
2. **不要改** `calibrate_load.py` 的主判据。该脚本已经用 `Run.busiest(skip_edge_port=True)` 在 `channelUtilization[]` 上跳过 port0；`max_channel_util` 字段读了但未用于拟合。可选：报告里多打印一行 C++ 的 `maxIslChannelUtilization` 作交叉校验。

**验收判据**：
- 代码与标量名落地即可。Hotspot **预期** sat2 的 `maxChannelUtilization` 在 port0（九源汇聚到 sat2.host）；真正的 ISL 瓶颈应出现在上游节点（sat3/sat5 等）的 `maxIslChannelUtilization`。
- **删除**原文「所有节点 `maxIslUtilPort != 0`」——空闲节点全 0 时会把默认端口 0 误判为失败。

---

### H4 `FDL-RateCheck` 的无量纲点未真正保持不变

**位置**：`fdl_experiments.ini:202-215`

**问题**：τ 与发送间隔都除以 10（正确），但 `BCPProcessingDelay = 1us`（`fdl_params.ini:118`）与 `guardTime` 未随之缩放。10 Gbps 下 T_burst = 3.44 µs，1 µs 处理时延从占突发时长 3% 变为 29%。该检查会因与归一化无关的原因失败，D2 拿回一个模糊答案。

**要求**（已选定方案一，不要再选方案二）：
在 `FDL-RateCheck` 中同步缩放：
- `BCPProcessingDelay = 0.1us`（1 µs / 10）
- `guardTime = 0.2us`（若 H1 取 2 µs）
- `switchReconfigTime` 若已在 Scenario 设为 2 µs，此处改为 `0.2us`，以免 τ=3.44 µs 反而小于未缩放的 2 µs 触发 B3
注释列出无量纲量：ρ、τ/T_burst、offset/T_burst、guard/T_burst、T_proc/T_burst，全部标注「已缩放」。

**验收判据**：ini 注释清单完整。本批不跑 RateCheck。

---

### H5 `repeat = 5` 可能不足以分辨 FDL 效应

**位置**：`fdl_experiments.ini:98`、`:108`、`:141`、`:156`

**问题**：ρ=0.4、W=1 条件下丢包事件不密集，种子间标准差可能与待测效应同量级。若 5 次重复的置信区间互相重叠，实验 A 的主图整幅不可用。

**要求（Agent 只准备配置，不跑、不统计）**：
在 `fdl_experiments.ini` 增加 `[Config ExpA-Pilot]`：`extends = ExpA-TauSweep`，但只保留一个 τ（建议 `34.4us`）与现有两个 `meanInterval`，`repeat = 10`。`walkthrough.md` 写用户命令。**未完成用户 pilot 之前不得把 `ExpA-TauSweep` 的 `repeat` 改掉，也不得开始实验 A 批量。**

**验收判据**：config 存在且不会因未实现的 VF 参数启动失败。统计分析不在本任务。

---

### H6 缺突发级时延统计

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc` 全局；突发拆帧侧模块

**问题**：实验 A 的指标清单含端到端时延，但目前只能从 INET sink 取，该量被边缘组装时延主导，τ 的影响会被埋掉。"时延 vs τ"这张图按现状不可信。

**要求**：
1. **本批只做 H6.1**：在 `OBS_CoreControlLogic` 对每个成功调度的突发 emit `burstNodeDelay`（直通 0，回环 τ）。`.ned` 增加 `@signal` / `@statistic record=mean,max,count`。受 B4 窗口约束。
2. **H6.2 本批禁止。** 突发级端到端时延要在 `src/EdgeNode/OBS_BurstDisassembler.cc` 打点，触碰受保护基线。`OBS_Burst.msg` / `OBS_BurstControlPacket.msg` 目前没有创建时间戳。需要时另开任务并先经用户批准改 EdgeNode。

**验收判据**：`Test-FDL-Basic` 的 `burstNodeDelay:mean` 接近 τ×（回环突发/本节点成功调度突发）；`Test-FDL-Off` 为 0。允许因只有 1 个回环突发而就是 τ 本身。

---

### H7 "单次回环"红线在代码中只是"每节点单次"

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:193-204`、`:226-234`

**问题**：在 sat2 回环过的突发到 sat3 后仍可再回环一次，且没有任何统计量记录一个突发沿路径回环了几次，连报告都无法产出。若论文声称"每突发单次回环"，当前实现与统计都无法支撑。

**要求**：
1. 在 `src/messages/OBS_BurstControlPacket.msg` **末尾追加** `int fdlLoopCount = 0;`，不改既有字段顺序。OMNeT++ 会重生成 `*_m.cc/h`。
2. 每次决定走回环之后、转发之前自增。
3. CoreNode 参数用 `int maxFdlLoopsPerBurst = default(-1);`（**不要**写 `@unit()`）。`-1` = 不限制；`1` 表示 `fdlLoopCount >= 1` 则禁止再次回环，走丢弃。
4. 标量 `burstsLoopedOnce`、`burstsLoopedMultiple`：在本节点回环时，若自增后计数 == 1 则前者 +1，若 > 1 则后者 +1。
5. 实验 A 正式配置仍默认 -1，本批不改 `ExpA-TauSweep`。

**验收判据**：默认 -1 时原功能测试标量不变。`Test-FDL-Basic` 中 sat2 `burstsLoopedMultiple == 0`。

**验收判据**：
- `maxFdlLoopsPerBurst = -1`（默认）时，全部现有功能测试标量不变。
- `Test-FDL-Basic` 中 `burstsLoopedMultiple == 0`（单跳场景不应出现多次回环），用以确认字段传递正确。
- 实验 A 跑完后必须报告 `burstsLoopedMultiple` 的实际值；若非 0，论文口径须改为"每节点单次回环"，或把 `maxFdlLoopsPerBurst = 1` 作为正式配置。

---

### H8 统计量重复记录与死配置

**位置**：`src/CoreNode/OBS_CoreControlLogic.cc:79-82`、`:350-351`、`:388-391`；`src/CoreNode/OBS_CoreControlLogic.ned:62-72`；`fdl_params.ini:202`

**问题**：`fdlUsageCount` 既通过 `@statistic record=count,vector` 记录，又手写 `recordScalar("fdlUsageCount", ...)`，`.sca` 中会出现两个名字近似的标量，分析脚本若误相加会得到双倍值。`burstLossContention`、`burstLossTotal` 同样。此外 `**.vector-recording = false` 使所有 `@statistic` 的 `vector` 部分成为死配置。

**要求**：
1. 保留手写 `recordScalar`。把 `.ned` 里 `record=count,vector` 改为不记录标量别名：建议 `record=vector` 或删掉 `@statistic` 只留 `@signal`（H6.1 的 `burstNodeDelay` 除外，它需要 mean）。目标是 `.sca` 里不要同时出现 `fdlUsageCount` 与 `fdlUsageCount:count`。
2. `**.vector-recording = false` 已在 `FDL-Scenario`，不必再为死 vector 改 ini。

**验收判据**：`walkthrough.md` 列出改动前后标量名。旧行为标量不变；FDL 相关「多出来的 count 别名」减少是预期，不是回归失败。

---

## 三、低优先：可在实验 A 之后处理

### L1 路由表查询是每 BCP 一次全表线性扫描加一次堆分配

**位置**：`src/CoreNode/OBS_CoreRoutingTable.cc:85-95`、`src/CoreNode/OBS_CoreControlLogic.cc:168-181`

**问题**：`getEntry()` 线性遍历整表并 `dup()` 出副本。core2Route 约 30 条。这是每 BCP 主要成本，但含通配与顺序语义。

**本批禁止实施。** 表项含 `*`（内部 -9），匹配顺序是 `cQueue::Iterator(..., 1)` 从后往前，即文件中后出现的条目优先。改成「按 (inPort, colour, label) 哈希」会改变命中哪一条。若以后做，必须：先按原顺序建「精确键 + 通配列表」，查找时精确命中与通配扫描的优先级与现在完全一致，并保留 `dup()` 语义或同步改所有调用方。需要单独设计与纯回归，不塞进本任务。

---

### L2 热路径上的 `Enter_Method` 开销

**位置**：`src/CoreNode/OBS_CoreOutputHorizon.cc:62`、`:97`、`:102`

**问题**：`findNearestLambda`、`updateHorizon`、`getHorizon` 使用 `Enter_Method`（带方法调用动画记账），每 BCP 调用数次；`getPortLambdas` 已正确使用 `Enter_Method_Silent`。

**要求**：改为 `Enter_Method_Silent`。若确认功能测试需要观察这些调用，可保留 `findNearestLambda` 一处并加注释说明。

**验收判据**：标量不变；`Test-FDL-Basic` 的 EV 中仍能看到两次 OXC 预约（那些日志来自 `OBS_CoreControlLogic`，不受本改动影响）。

---

### L3 `lambdasPerPort` 解析无边界检查

**位置**：`src/CoreNode/OBS_CoreOutputHorizon.cc:40-45`

**问题**：tokenizer 循环无上界保护。若 `lambdasPerOutPort` 的 token 数多于 `numPorts`，会覆写 `portLambdas[numPorts]`（FDL 行）甚至越界，造成堆破坏且难以定位。

**要求**：循环条件为 **`i < numPorts`**（不是 `i <= numPorts`）。循环结束后若 `i != numPorts` 则 `opp_error`，打印两者。然后再执行现有的 `portLambdas[numPorts] = 1`。反向验证：另加 `[Config Test-FDL-PortStringOverflow]`（预期失败），不要改完删 ini。`OBS_CoreOutput.cc` 的同类 tokenizer 不在本批范围，但若改 horizon 却不改 output，只保证 horizon 不会越界即可。

**验收判据**：正常配置下行为不变；人为把某节点的端口串多写一个 `1` 做一次反向验证，必须报错退出；验证后还原。

---

### L4 `**.dispatchMode` 通配范围过宽

**位置**：`fdl_params.ini:153`、`fdl_experiments.ini:138`

**问题**：`**.dispatchMode` 会匹配任何拥有同名参数的模块。当前无冲突，但语义不清。

**要求**：只改 FDL 的 ini，不改 `src/EdgeNode/`、不改 `experiments.ini`。把
`**.dispatchMode` 收窄为：
`**.sat*.edgeRouter.obs.assembler.dispatcher.dispatchMode`
（模块链：`OBS_SatelliteNode.edgeRouter` → `OBS_EdgeNode.obs` → `OBS_EdgeInterface.assembler` → `OBS_BurstAssembler.dispatcher`）。
`fdl_params.ini` 与 `ExpB-Joint` 两处都改。

**验收判据**：`walkthrough.md` 注明用户可用 `-c ExpB-Joint -r 0` 看参数回显，dispatcher 为 3 或 0。Agent 不跑。

---

## 四、验收总表

| ID | 严重度 | 主题 | 是否阻塞实验 A | 需用户跑仿真验证 |
|---|---|---|---|---|
| B1 | 阻塞 | OXC 丢包计数与输出冲突断言 | 是 | 是（三个功能测试 + 兼容性） |
| B2 | 阻塞 | τ 双份配置一致性检查 | 是 | 是（含一次反向验证） |
| B3 | 阻塞 | τ 合法性与开关时间下界 | 是 | 是 |
| B4 | 阻塞 | warm-up 与统一测量窗口；重做标定 | 是 | 是（须重跑 `FDL-Calibrate`） |
| B5 | 阻塞 | DO/SO 双模式与 offset 分布统计 | 是 | 是（含 EV 人工核对） |
| H1 | 高 | `guardTime` 接开关重构时间 | 建议是 | 是（须与 B4 合并重做标定） |
| H2 | 高 | FDL 占用率定义与命名 | 建议是 | 是 |
| H3 | 高 | ISL 瓶颈口归属与 hotspot 校验 | 建议是 | 是（`FDL-Hotspot`） |
| H4 | 高 | RateCheck 无量纲口径 | 否（但阻塞 D2） | 是 |
| H5 | 高 | pilot 定重复次数 | 是 | 是（pilot 运行） |
| H6 | 高 | 突发级时延统计 | 否（但阻塞时延图） | 是 |
| H7 | 高 | 每突发回环次数字段与上限 | 否（但阻塞论文口径） | 是 |
| H8 | 高 | 统计量去重与死配置清理 | 否 | 是 |
| L1 | 低 | 路由表哈希化 | 否 | **本批禁止** |
| L2 | 低 | 热路径 `Enter_Method_Silent` | 否 | 是（纯回归） |
| L3 | 低 | 端口串解析边界检查 | 否 | 是（含预期失败 config） |
| L4 | 低 | `dispatchMode` 通配收窄 | 否 | 否（参数回显即可） |

**Agent 本轮执行顺序**：B1 → B2 → B3（第 1 批，用户先编译跑三个功能测试 + Compatibility）→ H1+B4（第 2 批，用户再跑功能测试；标定等用户）→ B5（按纠正后的 SO）+ H2 + H3 + H8 + H6.1 + H7 + H4 ini + H5 的 ExpA-Pilot config + L2 + L3 + L4。

**批次划分**：
- 第 1 批（B1/B2/B3）：只加计数器、断言、参数校验，不改 guardTime/warmup。验证：三个功能测试与 Compatibility 旧行为标量不变。
- 第 2 批（H1/B4）：改 guardTime 与测量窗口。**会改变 ρ。** Agent 不回写 35.9 µs；只加「待重标定」注释。用户跑 `FDL-Calibrate` 后再改数字。
- 第 3 批（B5 纠正版 / H2 / H3 / H8 / H6.1 / H7）：新参数默认不改变 DO 行为。
- 第 4 批（H4 ini、H5 Pilot config）：只改配置。
- 第 5 批（L2/L3/L4）：L1 不做。

---

## 五、已复核确认正确，禁止改动

以下内容经逐条验算确认无误。改动其他条目时请避让，不要"顺手清理"。

| 位置 | 结论 |
|---|---|
| `OBS_CoreControlLogic.cc:272`、`:279`、`:311`、`:316`、`:336`、`:341` 的 `setSchedulingPriority(2)` / `(1)` | **承重设计。** unschedule 优先级 1 先于 schedule 优先级 2 执行，这是相邻预约在同一时刻交接时不触发 `opp_error` 的唯一保障。删除或改动会导致随机崩溃 |
| FDL horizon 只记入口占用、不加 τ（`:195`、`:228`、`:303`） | **正确。** 已验算：突发 B 的第二次预约 connect 时刻 ≥ 突发 A 的第二次预约 disconnect 时刻，恰好在边界重合并由上述优先级化解。定长延迟线只需守入口 |
| `:363-364` 把 `portBusyTime` / `portCarriedBytes` 的累加放在两个分支之外 | 正确。直通与回环最终都在 `(outPort, lambda)` 上占用一个 `burstDuration`，只应累加一次 |
| `OBS_CoreOutputHorizon.cc:38`、`:45`、`:47-56` 的 `numPorts + 1` 分配与 `portLambdas[numPorts] = 1` | 正确。FDL 行索引 `numPorts` 与控制逻辑中 `getHorizon(numPorts, 0)` 一致；析构 `i <= numPorts` 与分配匹配 |
| `OBS_CoreNode.ned:74-75`、`:109-111` 的 OXC 门数 `+1` 与回环连线 | 正确。FDL 占用最后一对门，与 `oxc->gateSize("in") - 1` / `gateSize("out") - 1` 一致 |
| `fdl_params.ini:71-105` 的 W=1 冻结（13 条 ISL + 10 个节点端口串全部显式写 1） | 做得好。不依赖 `params.ini` 保持"大部分是 1"，并通过 `portLambdas[]` 落盘支持事后否决跑错的 run。不要精简 |
| `fdl_params.ini:138-144` 关于 `maxOffset` 必须严格大于 `minOffset` 的注释与取值 | 正确且是踩过的坑，保留注释 |
| `fdl_params.ini:173-176` 关于 `stopTime = -1s` 的注释 | 正确且是踩过的坑，保留 |
| `fdl_tests.ini:113-117`（`Test-FDL-TooShort`）与 `:128-132`（`Test-FDL-Off`）标量全同的判据 | **整个测试套中最有力的证据**，证明开关语义干净。保留，不要合并这两个 config |
| `fdl_experiments.ini:123-130`（`ExpB-Joint` 的公平性警告） | 判断正确：dispatchMode 改变突发长度分布，而突发长度通过 waitTime 直接进入 FDL 准入判据。保留，并在论文中落实 |
| `fdl_experiments.ini:148-149`（`ExpR-Retransmit` 标注为骨架、不得当作 ExpR 报告） | 正确的自我设限，保留 |
| `params.ini` / `experiments.ini` / `tests.ini` | 第一阶段受保护基线，本次不得修改 |
| `src/EdgeNode/` | 受保护基线，本次不得修改 |

---

## 六、需要同步更新的文档

第 2 批之后 ρ 数字会变，但 **Agent 不得在用户重跑 `FDL-Calibrate` 之前改掉 35.9 µs**。本批只加「待重标定」注释。用户跑完后再改：

| 文档 | 何时改 | 内容 |
|---|---|---|
| `fdl_params.ini` 标定注释 | 本批 | 加一行：35.9 µs 为第 5 周测量，warm-up/guardTime 变更后须重跑 |
| `fdl_params.ini:177-180` 间隔取值 | 用户重标定后 | 新的 ρ=0.4 间隔 |
| `fdl_experiments.ini` 三处 ρ 注释 | 用户重标定后 | 同上 |
| `research_status.md` | 用户重标定后 | 第 5 周记录保留；加后记说明已被取代 |
| `research_reports/2026-09-week5-calibrate/` | 用户重标定后 | 后记，原 `.sca` 不改 |
| `论文可投验收表.md` | 本批可改 | M1 增加 `oxcBurstDropped == 0` |

原第 5 周测量数据不得删除或手改。这是「数据真实」红线。
