# 实验平台重建与计划修订 walkthrough

> 日期：2026-08-14
> 范围：CoreNode 统计instrumentation、FDL 实验配置体系、分析工具、研究计划与状态文档
> 前一次 walkthrough（第 1 周方向确认与阶段基线）内容已被本文件替换，历史记录见 `research_reports/2026-08-week1-baseline/`

---

## 一、本次要解决的问题

对文档、代码和实验配置做了一轮实地核对，发现三类问题。第一类是架构判断错误，第二类是会让阶段三实验跑出空结果的配置缺陷，第三类是计划本身的覆盖缺口。

### 架构判断错误：回环 FDL 不能证明 offset→0

原研究叙事是"offset=0 时 burst 无提前通知，必须光域暂存，所以需要 FDL"。这个论证本身成立，但它指向的不是当前实现的 FDL。

现有实现中 burst 进入 FDL 的前提，是控制逻辑在 `burstArrival - guardTime/2` 时刻已经把入口门连到 FDL 出口门：

```287:305:src/CoreNode/OBS_CoreControlLogic.cc
      // Scenario B: FDL Loopback
      int fdlOutGate = oxc->gateSize("out") - 1;
      int fdlInGate = oxc->gateSize("in") - 1;

      // 1st OXC reservation: inGate -> fdlOutGate
      simtime_t fdlConnectTime = burstArrival - guardTime/2;
```

这个预约要求 BCP 提前到达，即 offset > 0。offset = 0 时 OXC 尚未配置，burst 会在 `OBS_OpticalCrossConnect` 中因 `schedulingTable[inGate] == -1` 被直接删除，回环 FDL 无从介入。

支撑 offset→0 的是另一个器件：位于每个输入端口、OXC 之前、无条件生效的**输入 FDL**，用于吸收标签读取与 OXC 配置时间。两者位置、数量、触发条件均不同。据此把研究目标口径改为"边缘节点发出时 offset = 0，节点内部处理时间由输入 FDL 吸收"，严格 `offset = 0` 因物理不可达而不再作为目标。

### 配置缺陷：实验 A/B 会跑出空结果

| 缺陷 | 实测 | 后果 |
|---|---|---|
| W=1 被破坏 | `params.ini:34` 设 `lambdasCore1to2 = 3` | 全网最拥塞链路恰是唯一违反 W=1 的链路 |
| 负载约 0.24% | FlowScaling 为 12 流 × 500B/2ms = 24 Mbps 对 10 Gbps 信道 | 核心节点几乎无竞争，`fdlUsageCount` 恒为 0，τ 扫描得平直线 |
| 仿真确定性 | `UDPBasicApp` 固定 `sendInterval` 与固定 `startTime` | 换种子结果相同，重复次数与置信区间无意义 |

第 1 周 smoke 测试中"核心丢弃合计为 0"已经是第二条的直接表现。

### 一处澄清：FDL horizon 未加 τ 不是缺陷

`newFDLHorizon = burstArrival + burstDuration + 3g/4`（`OBS_CoreControlLogic.cc:294`）看似漏掉了 τ，但 FDL 是延迟线不是缓存：多个 burst 只要入口时间不重叠就可以同时在光纤中传播，定长延迟保证出口顺序与间隔不变。horizon 建模的是 FDL **入口占用**，是正确的。相应地 `fdlUtilization` 度量的是入口占用率而非光纤占空比，这一定义必须写进论文，否则容易被误读。

---

## 二、代码变更

变更限于 CoreNode，均为新增统计，不改变任何调度判断，因此 `useFDL=false` 的向后兼容性不受影响。`src/EdgeNode/` 未触碰。

### `OBS_CoreOutputHorizon.h/.cc`

新增 `getPortLambdas(int port)`，用于把每端口的累计占用换算成利用率。原先 `portLambdas` 是 protected 且无访问接口。

### `OBS_CoreControlLogic.h/.cc`

新增两个按输出端口索引的累加器：

```cc
simtime_t *portBusyTime;      // 每条输出光纤上被预约的 burst 时长之和
double *portCarriedBytes;     // 每条输出光纤承载的字节数，用 double 避免长时间仿真溢出
```

累加点放在直通与回环两个分支的汇合处，两条路径都会经过，避免重复代码：

```cc
   // Both the direct and the FDL branch end up reserving burstDuration on (outPort,lambda), so the
   // channel occupancy is accumulated once here for either path.
   portBusyTime[outPort] += burstDuration;
   portCarriedBytes[outPort] += (double)burstLength;
```

`finish()` 中新增标量：

| 标量 | 含义 | 用途 |
|---|---|---|
| `burstsReceived` / `burstsScheduled` | 到达与成功调度的 burst 数 | 丢包率的分子分母来源 |
| `burstLossRate` | 丢弃数 / 到达数 | 实验 A/B 的主指标，原先只有计数没有比率 |
| `channelUtilization[p]` | 端口 p 的实测利用率 | **负载标定的地面真值** |
| `carriedBursts[p]` / `carriedBytes[p]` | 端口 p 的承载 burst 数与字节数 | 两者之比即平均 burst 长度，用于实验 B 的公平性检查 |
| `maxChannelUtilization` | 该节点最忙输出光纤的利用率 | 标定脚本直接读取 |

`channelUtilization` 是本次的关键：没有它就无法验证"这组实验确实跑在 ρ=0.5"，而这正是原计划缺失的一环。

---

## 三、新增实验配置

三个新 ini 全部只定义 `[Config]` 段、不写 `[General]`，因此第一阶段的 `params.ini`／`tests.ini`／`experiments.ini` 行为完全不变，旧实验仍可复现。已在 `omnetpp.ini` 末尾 include。

### `fdl_params.ini` — `[Config FDL-Scenario]`

场景基线，纠正上述三个缺陷：

- **W=1**：`lambdasCore1to2 = 1`，并同步修正 sat1/sat2 的 `lambdasPerInPort/OutPort`。NED 中每条链路建 `lambdasCoreXtoY + 1` 条并行光纤，其中 +1 是承载 BCP 的控制信道，所以 1 恰为一个数据波长。
- **多源流量**：10 个卫星各跑一个 `UDPBasicApp`，`destAddresses` 列出其余 9 个节点，INET 每包随机选目的地，由此得到均匀全互联流量。第一阶段全部流量源自 sat1，拓扑退化为单源星形，中间节点从不承受多上游汇聚——而这正是 FDL 存在的意义所在。
- **随机化**：发送过程改为 `exponential()`，启动时刻 `uniform(0s, 1ms)` 抖动，重复实验才有意义。
- **非零处理时延**：`BCPProcessingDelay = 1us`。原为 `0s`，offset 沿路径永不消耗，会让实验 C 免费"证明" offset=0 可行。

### `fdl_tests.ini` — Task 2.3

确定性冲突构造：sat1 与 sat5 同时发往 sat3，两条路径按 `core1Route.dat`／`core5Route.dat` 在 sat2 汇聚于同一输出端口，传播时延同为 5 ms，sat5 延后 5 µs 启动以避免恰好相等（`findNearestLambda` 用严格 `>`，到达时刻等于 horizon 会被判为忙，正好相等时结果不易解读）。

单包成帧使 burst 固定为 1437 B，1 Gbps 下 T_burst = 11.5 µs，第二个 burst 的 waitTime ≈ 6.5 µs，于是 τ 直接选择分支：

| 测试 | τ | 预期分支 | 判据 |
|---|---|---|---|
| Test-FDL-Basic | 20 µs | 回环 | `fdlUsageCount > 0`，两次预约间隔恰为 τ，BCP offset 增加恰为 τ |
| Test-FDL-TooShort | 2 µs | 丢弃 | `fdlUsageCount == 0`，`burstLossContention > 0` |
| Test-FDL-Off | — | 丢弃 | 所有标量与 TooShort 一致 |
| Test-FDL-Compatibility | — | — | 与 pre-FDL 提交 `.sca` 逐标量一致 |

Test-FDL-TooShort 为本次新增。缺了它，Test-FDL-Basic 通过也可能是"FDL 无条件接收一切"造成的假阳性。Test-FDL-Compatibility 刻意不继承 `FDL-Scenario`，而是继承第一阶段的 `ICMPTest`，否则流量都变了就失去回归意义。

### `fdl_experiments.ini`

| 配置 | 用途 |
|---|---|
| `FDL-Calibrate` | 负载标定扫描，FDL 关闭 |
| `ExpA-TauSweep` / `ExpA-NoFDL` | 实验 A 及其无 FDL 参照 |
| `ExpB-Joint` | 实验 B 的 2×2 × 多负载 |
| `FDL-RateCheck` | 验证 1 Gbps 归一化成立 |

---

## 四、线速率与光纤长度：本次最主要的设计发现

约束链是这样闭合的。光在光纤中速度约 2.04×10⁸ m/s，所以延迟 τ 需要长度 `L = τ × 2.04e8`。单次回环只能救回等待不超过 τ 的 burst，而一个 burst 占用信道造成的等待量级就是一个 burst 时长，因此有效区间在 τ ≈ T_burst 附近，而 `T_burst = burstBytes × 8 / C`。合并得：

```
L ≈ (burstBytes × 8 / C) × 2.04e8
```

**FDL 的光纤长度预算反过来约束了线速率与 burst 大小的组合。** 降低线速率会拉长 burst 时长，从而要求更长的光纤——这与"卫星用低速率更现实"的直觉相反。`tools/fdl_design.py` 输出（4295 B burst、1 km 预算、10 Gbps 设计点）：

| τ/T_burst | 物理 τ | 光纤长度 | 传输损耗 | 1 km 预算内 |
|---|---|---|---|---|
| 0.25 | 0.86 µs | 175 m | 0.04 dB | 是 |
| 0.5 | 1.72 µs | 351 m | 0.07 dB | 是 |
| 1.0 | 3.44 µs | 701 m | 0.14 dB | 是 |
| 2.0 | 6.87 µs | 1402 m | 0.28 dB | 否 |
| 4.0 | 13.74 µs | 2805 m | 0.56 dB | 否 |

即 1 km 预算下，10 Gbps 信道最多支持约 6 kB 的 burst。这张表本身就是可以写进论文的工程结论，也说明原计划把 τ 扫到 100 µs（约 20 km 光纤）没有意义。

### 为什么仿真跑在 1 Gbps

10 Gbps 下达到 ρ=0.5 需要约 270 万包/秒穿过 INET 协议栈，按每包数十个事件计约需数亿事件，一个扫描点就要数分钟到数小时，而实验 A/B 有上百个点。仓库中不存在 burst 级流量源（只有 INET 应用 + 边缘成帧器），短期内无法绕开这个瓶颈。

FDL 动力学只依赖无量纲比值（ρ、τ/T_burst、offset/T_burst），因此扫描在 1 Gbps 下进行、结果以 τ/T_burst 报告，再换算到 10 Gbps 设计点。`FDL-RateCheck` 保持所有无量纲比值不变、把 τ 与发送间隔同时除以 10，在 10 Gbps 下复现同一工作点用于验证。若该验证不通过，实验 A 必须改在物理速率下重跑，代价是需要先开发 burst 级流量源。

---

## 五、分析工具

### `tools/calibrate_load.py`

把目标负载换算成发送间隔。模型没有归一化负载旋钮，而 ρ 与发送间隔的映射依赖路由表和哪条 ISL 恰好最忙，无法解析写死，只能实测。饱和前 ρ ∝ 1/meanInterval，因此一次标定扫描即可定出常数：

```
python tools/calibrate_load.py results/FDL-Calibrate
```

脚本读取各 run 的 `channelUtilization[]`，默认排除 port 0（本地边缘落地口）只统计 ISL 口，丢包超过 1% 的点视为已饱和、不参与拟合，最后打印各目标 ρ 对应的 ini 行。拓扑、路由、burst 大小、信道速率任一变化都必须重新标定。

`fdl_params.ini` 中当前的 42.6 µs 只是解析估算的起点（假设均匀全互联、平均 2.2 跳、26 条有向数据信道、最忙信道为均值 1.8 倍），必须被实测值替换。

### `tools/fdl_design.py`

τ ↔ 光纤长度 ↔ 损耗预算换算，输出即上表。

两个脚本均已在本机运行验证：`fdl_design.py` 输出与配置注释中的数值一致；`calibrate_load.py` 对现有 `week1-smoke` 结果能正确解析并在缺少新标量时给出明确提示。

---

## 六、文档变更

| 文件 | 变更 |
|---|---|
| `research_reports/00-project-management/codex_phase2_tasks.md` | 12 周扩为 16 周：插入第 5 周实验平台重建、第 6 周文献定位、第 7 周 Task 1.3；实验 C 拆为三周并前置输入 FDL 时序模型；新增论文并行线；修正 FDL 架构定位与目标口径；实验 A 改为按 τ/T_burst 扫描；实验 B 增加 burst 长度公平性要求 |
| `research_status.md` | 同步周表；新增"回环 FDL 不能证明 offset→0"与目标口径修正；新增实验平台三缺陷表；更新已知/待证明表与关键文件表；关闭原两个待确认问题并提出三个新的 |

新增的三个待确认问题：仿真线速率归一化是否被接受；FDL 光纤长度预算的实际数值；BCP 电子处理时延的实际取值。

---

## 七、未执行事项

按 `AGENTS.md`"编译和仿真验证由用户执行"，本次未运行 `make`，也未运行任何 OMNeT++ 仿真。以下需要用户验证：

1. **编译**：新增了 `getPortLambdas` 与 `finish()` 中的标量记录。`sprintf` 所需的 stdio 已由文件中既有的 `fopen`／`fprintf` 引入。release 链接的 `-linet` 问题是既有的工具链问题，与本次变更无关。
2. **`fdl_tests.ini` 三个确定性测试**：sat1/sat5 在 sat2 汇聚的推断来自 `core1Route.dat` 与 `core5Route.dat` 的静态阅读，需要用 EV 日志确认实际输出端口一致；5 µs 的错开量是否落在预期的 waitTime 区间也需实测确认。
3. **`FDL-Calibrate` 标定**：k 值尚未测得，`fdl_params.ini` 与 `ExpB-Joint` 中的发送间隔目前都是解析估算值，不能当作已达到标称负载。
4. **`FDL-RateCheck`**：1 Gbps 归一化的有效性尚未验证。若不成立，实验 A 的设计需要重做。
5. **`ExpB-Joint` 的 `${edgeMode}` 与 `${fdl}`** 是否与继承自 `FDL-Scenario` 的普通赋值正确叠加，需要用 `-a` 列举 run 数确认（预期 2×2×4 = 16 组合 × 5 次重复 = 80 run）。

本次未创建 feature branch，也未提交任何改动，等待用户确认后再按 `task/2.3-fdl-platform` 建立分支。
