# 第二阶段代码改动完全说明

> 日期：2026-08-14
> 用途：解释第二阶段（FDL）到目前为止的**全部**代码改动——每个文件改了什么、每个功能怎么工作、每个测试怎么构造、每个统计量是什么含义。
> 读者假设：了解 OBS 基本概念（BCP、offset、burst），但没有逐行看过这批代码。

---

## 一、一分钟总览

核心节点里一个 burst 的正常旅程：

```
BCP（控制包，提前 offset 到达）          burst（光信号，晚 offset 到达）
      │                                        │
      ▼                                        ▼
  Input ──控制信道──> ControlUnit          Input ──数据信道──> OXC ──> Output
                          │                                    ▲
                          │ 提前算好：几点接通、几点断开          │
                          └────────── scheduleAt() ─────────────┘
```

BCP 先到，控制逻辑读出"burst 什么时候到、多长、去哪"，然后**预约** OXC：在 burst 到达前一刻把输入门接到目标输出门，burst 过完后断开。burst 本身始终是光信号，从不进电域。

第二阶段加的东西只有一件：当目标输出信道**忙**的时候，原来只能丢弃，现在多了一条退路——让 burst 绕进一段光纤（FDL）转一圈，延迟 τ 秒后再从目标信道出去。于是调度变成三个分支：

| 分支 | 条件 | 动作 |
|---|---|---|
| A 直通 | 目标信道在 burst 到达时空闲 | 1 次 OXC 预约，照常转发 |
| B 回环 | 信道忙，但等待时间 ≤ τ，且 FDL 入口空闲，且 `useFDL=true` | 2 次 OXC 预约：先进 FDL，τ 后再出目标信道 |
| C 丢弃 | 以上都不满足 | 丢 BCP，计数 |

`useFDL=false`（默认）时 B 分支整体跳过，行为与第二阶段开发前完全一致——这就是向后兼容红线的实现方式。

---

## 二、改动台账

| 文件 | 属于 | 状态 | 一句话 |
|---|---|---|---|
| `src/CoreNode/OBS_FiberDelayLine.ned/.cc/.h` | Task 2.1 | 已提交（`1d45cf4`） | FDL 器件本体：收到什么就延迟 τ 发出什么 |
| `src/CoreNode/OBS_CoreNode.ned` | Task 2.1 | 已提交 | 挂上 fdl 子模块，OXC 加一对回环门 |
| `src/CoreNode/OBS_CoreOutputHorizon.cc/.h` | Task 2.1 + 平台 | 部分未提交 | horizon 表加 FDL 端口；新增 `getPortLambdas()` |
| `src/CoreNode/OBS_CoreControlLogic.cc/.h/.ned` | Task 2.2 + 平台 | 未提交 | 三分支调度 + FDL 统计 + 信道利用率统计 |
| `Examples/RingFdlOBS/fdl_params.ini` | 平台 | 未提交（新文件） | FDL 实验场景基线（W=1、全互联流量、负载旋钮） |
| `Examples/RingFdlOBS/fdl_tests.ini` | Task 2.3 | 未提交（新文件） | 四个确定性功能测试 |
| `Examples/RingFdlOBS/fdl_experiments.ini` | 阶段三 | 未提交（新文件） | 负载标定 + 实验 A/B + 归一化校验 |
| `Examples/RingFdlOBS/tools/calibrate_load.py` | 平台 | 未提交（新文件） | 目标负载 → 发送间隔换算 |
| `Examples/RingFdlOBS/tools/fdl_design.py` | 平台 | 未提交（新文件） | τ → 光纤长度/延迟器参数换算 |

"平台"指 2026-08-14 的实验平台重建（详见 `walkthrough.md`）。`src/EdgeNode/` 全程未动。

---

## 三、硬件结构：Task 2.1 建了什么

### 3.1 FDL 器件本体：`OBS_FiberDelayLine`

整个模块本质上只有一行逻辑：

```31:34:src/CoreNode/OBS_FiberDelayLine.cc
void OBS_FiberDelayLine::handleMessage(cMessage *msg) {
    fdlUsageCount++;
    sendDelayed(msg, delayTime, "out");
}
```

进什么、出什么，只是晚 `delayTime`（即 τ）秒。它自己不做任何判断——**该不该进 FDL 是控制逻辑的事，FDL 只负责延迟**。这正是真实光纤的行为：一段光纤没有智能，光进去只能等它走完。

`finish()` 记录 `"FDL usage count"` 标量，作为"burst 确实物理经过了 FDL"的独立证据（控制逻辑那边还有一个自己的计数，两者应该相等，不等就说明预约和实际光路脱节了）。

### 3.2 接线：`OBS_CoreNode.ned`

三处改动。

**参数**（两个开关，全部实验靠它们）：

```34:35:src/CoreNode/OBS_CoreNode.ned
        bool useFDL = default(false);
        double fdlDelayTime @unit(s) = default(10us);
```

**OXC 门数 +1**。OXC 原来的数据门数量是"总门数减控制信道数"；现在各加一个，专门给回环用：

```74:75:src/CoreNode/OBS_CoreNode.ned
                in[sizeof(in)-numPorts + 1];
                out[sizeof(out)-numPorts + 1];
```

**回环连线**。OXC 的最后一个输出门 → FDL → OXC 的最后一个输入门：

```109:111:src/CoreNode/OBS_CoreNode.ned
        //Connect the fiber delay line loopback
        OXC.out[sizeof(out)-numPorts] --> fdl.in;
        fdl.out --> OXC.in[sizeof(in)-numPorts];
```

所以"进 FDL"在 OXC 眼里就是"切到最后一个输出门"，"从 FDL 回来"就是"信号从最后一个输入门进来"。控制逻辑里的 `fdlOutGate = gateSize("out") - 1`、`fdlInGate = gateSize("in") - 1` 对应的就是这两个门。**既有门的编号一个都没动**，这是兼容性的物理保证。

### 3.3 记账表：`OBS_CoreOutputHorizon`

horizon 表记录"每条输出信道几点之前是忙的"。调度就是查这张表、更新这张表。Task 2.1 把表加大一行，给 FDL 当独立的预约状态：

```45:45:src/CoreNode/OBS_CoreOutputHorizon.cc
   portLambdas[numPorts] = 1; // FDL loopback port has 1 wavelength channel
```

约定：**端口号 `numPorts`、波长 0 就是 FDL**。控制逻辑查"FDL 现在能不能进"，就是 `getHorizon(numPorts, 0) <= burstArrival`。

2026-08-14 又加了一个只读接口 `getPortLambdas(int port)`，供统计代码把"端口累计占用时长"换算成利用率（占用 ÷ 仿真时长 ÷ 该端口波长数）。它用 `Enter_Method_Silent`，因为会在 `finish()` 里被跨模块调用，不能触发动画。

---

## 四、调度逻辑：Task 2.2 怎么做决定

全部改动集中在 `OBS_CoreControlLogic::handleMessage()`。收到 BCP 后的流程：

**第 1 步：算出 burst 什么时候到。**

```147:147:src/CoreNode/OBS_CoreControlLogic.cc
   simtime_t burstArrival = simTime() + arrivalDelta;
```

`arrivalDelta` 就是 BCP 里携带的剩余 offset。如果算出来是过去时刻（offset 在路上被耗尽了），直接丢弃——这是既有逻辑，编号"丢弃原因 1"。

**第 2 步：查路由表**，得到输出端口 `outPort` 和输出波长。本项目所有路由表（`config/core*Route.dat`）的波长都写 `*`（通配），所以实际走的都是下面的"通配分支"。

**第 3 步：三分支判定。** 通配分支的完整代码：

```187:204:src/CoreNode/OBS_CoreControlLogic.cc
   if(outColour == -9){ // * option. Choose the lambda with closest horizon
      	// Choose the best channel
        lambda = gatesHorizon->findNearestLambda(outPort,burstArrival);        

	if(lambda != -1){
            scheduled = true;
      	} else if (useFDL) {
            // Scenario B: FDL Loopback
            simtime_t fdlHorizon = gatesHorizon->getHorizon(numPorts, 0);
            if (fdlHorizon <= burstArrival) {
                // Look for free lambda at burstArrival + tau
                lambda = gatesHorizon->findNearestLambda(outPort, burstArrival + tau);
                if (lambda != -1) {
                    scheduled = true;
                    usedFDLForThisBurst = true;
                }
            }
        }
```

读法：
1. `findNearestLambda(outPort, burstArrival)`：burst 到达时有没有空闲波长？有 → **A 直通**。
2. 没有，且 `useFDL` 开着 → 查 FDL 入口是否空闲（`fdlHorizon <= burstArrival`），再查"延迟 τ 之后"有没有空闲波长（`findNearestLambda(outPort, burstArrival + tau)`）。两个都过 → **B 回环**。
3. 否则 → **C 丢弃**（"丢弃原因 2"），计入竞争丢包。

固定波长分支（W>1 带 colour 时才走）逻辑等价，只是判据写成 `waitTime <= tau && fdlHorizon <= burstArrival`（第 229-230 行）：等待时间不超过 τ，意味着信道在 `burstArrival + tau` 时一定已空闲，数学上和通配分支的显式复查是一回事。

**重要推论**：`AGENTS.md` 和文档多处引用的 FDL 判据表述 `waitTime <= τ && FDL 空闲` 对应的是第 230 行的固定波长（颜色）分支。但本项目所有路由表波长均为 `*`，**该分支是死代码**。实际生效的是上面通配分支的 `findNearestLambda(outPort, burstArrival+τ)`（第 198 行）。两者数学等价，但读者如果对着第 230 行去解读实验结果会对不上实际执行路径。`fdl_tests.ini` 的测试在通配分支下依然成立：τ=20 µs 时 horizon ≈ burstArrival+6.5 µs < burstArrival+20 µs → 回环；τ=2 µs 时 horizon ≈ burstArrival+6.5 µs > burstArrival+2 µs → 丢弃。通配分支和颜色分支还有一个微小的边界差异（`>` vs `>=`），在连续随机流量下是零概率事件，在确定性测试里的数值也远离边界，已确认接受此差异。

**第 4 步 A（直通）**：一次 OXC 预约（第 254 行起，既有逻辑没动）。三个时刻：

| 事件 | 时刻 |
|---|---|
| OXC 接通 `inGate → 目标门` | `burstArrival − guard/2` |
| OXC 断开 | `burstArrival + burstDuration + guard/4` |
| 目标信道 horizon 更新为 | `burstArrival + burstDuration + 3·guard/4` |

guard 是保护间隔（当前 1 ns），前后各留半个，保证切换不会切到 burst 本体。

**第 4 步 B（回环）**：两次预约，时间轴如下（第 294-352 行）：

```
时刻 ──────────────────────────────────────────────────────────────>
      burstArrival        burstArrival+D          burstArrival+τ        +τ+D
           │                    │                       │                 │
预约1：  inGate→FDL门 接通 ──── 断开
                （burst 全部流进 FDL）
预约2：                                          FDL门→目标门 接通 ────── 断开
                                                 （burst 从 FDL 流出，进目标信道）
horizon： FDL入口 记忙到 burstArrival+D+3g/4
          目标信道 记忙到 burstArrival+τ+D+3g/4
```

（D = burst 时长，g = guard。）两次预约用的是**不同的 OXC 输入门**（第一次是 burst 原输入门，第二次是 FDL 回环输入门），所以互不冲突。

**第 4 步收尾（两分支共用）**：更新 BCP 再转发给下一跳。回环时 offset 要加 τ，否则下一跳会以为 burst 比实际早到：

```358:358:src/CoreNode/OBS_CoreControlLogic.cc
      bcp->setBurstArrivalDelta(arrivalDelta + tau - processingTime);
```

（直通版本是 `arrivalDelta - processingTime`，第 292 行。减 `processingTime` 是因为 BCP 本身还要在本节点排队 `processingTime` 才发出，两相抵消后下游看到的净变化恰好是 +τ。）

### 两个容易误解的设计点

**FDL 的 horizon 为什么只记到 `burstArrival + D` 而不是 `+τ+D`？**
因为 FDL 是延迟线不是缓存。光纤里可以同时跑多个 burst，只要**入口**不重叠；定长延迟保证它们出口也不重叠、顺序不变。horizon 记的是入口占用。推论：`fdlUtilization` 度量的也是入口占用率，不是"光纤里有没有光"，写论文时要注明。

**burst 会不会在同一节点绕两圈？**
不会，而且是结构上不会：burst 出 FDL 后由预约 2 直接送到输出端口，不再经过控制逻辑，没有任何代码路径能让它第二次进 FDL。单次回环红线不靠标志位，靠拓扑。（跨节点则每一跳都可以独立回环一次，这是设计允许的。）

---

## 五、统计量对照表（.sca 里每个数是什么）

全部由 `OBS_CoreControlLogic::finish()` 写出（除注明外），模块路径形如 `RingFdlOBS.satN.coreSwitch.ControlUnit.ControlLogic`。

### Task 2.2 加的（FDL 与丢包）

| 标量 | 含义 | 用途 |
|---|---|---|
| `fdlUsageCount` | 本节点回环成功次数（控制逻辑视角） | 实验 A 主证据；恒为 0 说明 FDL 没被触发 |
| `FDL usage count` | 同上，但由 FDL 模块自己数（物理视角） | 与上面互相印证，不等即时序有 bug |
| `fdlUtilization` | FDL 入口占用率 = Σburst时长 ÷ 仿真时长 | FDL 是否成为瓶颈 |
| `burstLossContention` | 因竞争丢弃数（原因 2/3） | 实验 A/B 分析竞争专用 |
| `burstLossTotal` | 全部丢弃数（含原因 1：burst 先于 BCP） | 总丢包 |

### 平台重建加的（2026-08-14，负载测量）

| 标量 | 含义 | 用途 |
|---|---|---|
| `burstsReceived` / `burstsScheduled` | 到达/成功调度的 burst 数 | 丢包率分母/分子 |
| `burstLossRate` | `dropCounter / burstsReceived`（total 口径，含迟到丢弃） | **实验 A/B 总体指标**；更纯净的竞争丢包率应用 `burstLossContention / burstsReceived` |
| `channelUtilization[p]` | 端口 p 实测利用率 | **负载标定的地面真值**——"这次实验真的跑在 ρ=0.5 吗"由它回答 |
| `carriedBursts[p]` / `carriedBytes[p]` | 端口 p 承载的 burst 数/字节数 | 两者相除 = 平均 burst 长度，实验 B 公平性检查用 |
| `maxChannelUtilization` | 本节点最忙端口的利用率 | 标定脚本直接读它 |

端口号约定：**p=0 是通往本地边缘节点的落地口，p≥1 是星间链路**。看拥塞要看 p≥1。

### 丢弃统计口径说明

代码中有两种丢弃原因，计入不同的计数器：

| 丢弃原因 | 触发条件 | 计入 `burstLossTotal` | 计入 `burstLossContention` |
|---|---|---|---|
| 原因 1：BCP 迟到 | `burstArrival < simTime()`，即 burst 已先于 BCP 到达 | 是 | 否 |
| 原因 2/3：竞争丢弃 | 直通和回环条件均不满足 | 是 | 是 |

因此 `burstLossTotal ≥ burstLossContention`。实验 A/B 的**主指标应使用 `burstLossContention / burstsReceived`**（纯竞争丢包率），而 `burstLossRate`（= `burstLossTotal / burstsReceived`）作为总体参考。在实验 A/B 的正常 offset（500 µs）下迟到丢弃应为 0，两者相等；但分析脚本和论文中仍应注明口径，避免实验 C（offset 缩减）的结果被误用。

---

## 六、测试与实验配置：每一个怎么工作

三个新 ini 通过 `omnetpp.ini` 末尾 include 进来，只含 `[Config]` 段，所以第一阶段的所有旧配置行为不变。

### 6.1 `fdl_params.ini`：场景基线 `FDL-Scenario`

所有 FDL 测试/实验的公共底座，修了三个会让实验空跑的问题：

| 修正 | 原状 → 现状 | 不修的后果 |
|---|---|---|
| W=1 | `lambdasCore1to2` 3 → 1（连带 sat1/sat2 端口波长表） | 全网最挤的链路恰好违反 W=1 红线，结论作废 |
| 负载 | 单源 24 Mbps → 全互联、按目标 ρ 反解发送间隔 | 10 Gbps 信道上 0.24% 负载，永无竞争，FDL 永不触发 |
| 随机性 | 固定间隔/起点 → 指数间隔 + 起点抖动 | 换种子结果一模一样，置信区间无意义 |

流量做法：每颗卫星一个 `UDPBasicApp`，`destAddresses` 列全其余 9 颗，INET 每个包随机挑目的地——一个 app 就实现了均匀全互联。另设 `BCPProcessingDelay = 1µs`（原来是 0，offset 永不消耗，实验 C 会白得"offset=0 可行"的假结论）。

### 6.2 `fdl_tests.ini`：Task 2.3 功能测试

**核心构造**：sat1 和 sat5 同时给 sat3 发包。查路由表可知两条路都在 sat2 汇聚、走同一个输出端口（sat2→sat3），且两条来路传播时延同为 5 ms——所以**冲突是必然的、可预测的**。sat5 故意晚 5 µs 启动，让第二个 burst 稳稳落在第一个的占用区间里（不搞成精确同时，是因为 `findNearestLambda` 判"到达==horizon"为忙，正好打平的结果不好解释）。

每个 burst 固定 1 包 = 1437 B，在 1 Gbps 下时长 11.5 µs，于是第二个 burst 的等待时间 ≈ 11.5 − 5 = 6.5 µs。**τ 变成了选择开关**：

| 测试 | τ | 6.5µs 和 τ 的关系 | 走哪个分支 | 通过判据 |
|---|---|---|---|---|
| `Test-FDL-Basic` | 20 µs | waitTime < τ | B 回环 | sat2 的 `fdlUsageCount > 0`、`burstLossContention == 0`、sat3 收齐全部包；EV 日志里两次预约的接通时刻差恰为 τ |
| `Test-FDL-TooShort` | 2 µs | waitTime > τ | C 丢弃 | `fdlUsageCount == 0`、`burstLossContention > 0` |
| `Test-FDL-Off` | —（useFDL=false） | 不看 τ | C 丢弃 | 所有标量与 TooShort 完全一致 |
| `Test-FDL-Compatibility` | —（useFDL=false） | — | 走老代码 | 与 pre-FDL 提交跑出的 `.sca` 逐标量一致（新增标量除外；`my ID` 因 CoreNode 多了 `fdl` 后编号后移，排除后已核对无行为差） |

为什么要有 TooShort：只有 Basic 的话，"FDL 无条件放行一切"的错误实现也能通过。Basic+TooShort 合起来才证明**条件判断本身**是对的。

为什么 Compatibility 不继承 `FDL-Scenario`：它继承第一阶段的 `ICMPTest`。回归测试的意义是"老场景跑出老结果"，把流量换掉就没有可比性了。

运行方式（在 `Examples/RingFdlOBS/` 下）：

```bash
../../out/gcc-release/obsmodules.exe -u Cmdenv -f omnetpp.ini -c Test-FDL-Basic -n "../..;.;D:/inet/src"
```

前三个测试关闭了 express 模式并打开 sat2 核心节点的 EV 输出，日志里能直接看到每次预约的门号和时刻。

### 6.3 `fdl_experiments.ini`：阶段三实验

| 配置 | 回答什么 | 怎么做 |
|---|---|---|
| `FDL-Calibrate` | "发送间隔设多少才是 ρ=0.4？" | FDL 关闭，扫 7 档间隔 × 2 个种子（14 run），跑完用脚本拟合 ρ = k/间隔，并检查 W=1 / 多源 / 随机化 |
| `ExpA-TauSweep` | 实验 A：τ 的有效区间 | τ 按 T_burst 的 0.25/0.5/1/2/4 倍扫，每点 5 个种子 |
| `ExpA-NoFDL` | 实验 A 的对照线 | 同负载、FDL 关，没有它 τ 曲线无从解读 |
| `ExpB-Joint` | 实验 B：边缘调度 × FDL 有无联合收益 | 2×2×4 档负载×5 种子 = 80 run |
| `FDL-RateCheck` | "1 Gbps 上跑的结果能代表 10 Gbps 吗？" | 所有无量纲比值不变，整体搬到 10 Gbps 复跑一个点 |

**执行顺序有讲究**：先 `FDL-Calibrate`（不然负载标签是假的）→ 把实测 k 写回 ini → 再跑 A/B。`ExpB-Joint` 开跑前先 `-a` 确认枚举出 80 个 run。

### 6.4 工具脚本

**`tools/calibrate_load.py`** — 读 `FDL-Calibrate` 的 `.sca`，取每个 run 全网最忙 ISL 口的 `channelUtilization`，拟合 ρ = k/间隔，打印各目标 ρ 对应的 ini 行。自动剔除丢包 >1% 的点（已饱和，不在线性段）。同一脚本检查 `portLambdas[]` 是否全为 1、10 星是否都发出突发、双种子是否有差异、FDL 计数是否为 0。拓扑/路由/burst 大小/信道速率任何一个变了都要重新标定。

**`tools/fdl_design.py`** — 纯换算，不读仿真数据：τ ↔ 等效光纤长度 ↔ 传输损耗。默认模式给出光纤参考值与 `--max-length` 预算判定；加 `--step` 与 `--switch-loss` 后进入延迟器模式，按二进制编码计算所需开关级数和总损耗（光纤段损耗 + 级数 × 每级插损）。实验 A 出结果后，用它把 τ 曲线翻译成延迟器设计参数。

---

## 七、建议的验证顺序（编译已通过之后）

1. `Test-FDL-Basic` → 看 sat2 的 `fdlUsageCount` 和 EV 日志里两次预约的时刻差。
2. `Test-FDL-TooShort`、`Test-FDL-Off` → 确认丢弃分支和开关，且两者标量一致。
3. `Test-FDL-Compatibility` → 2026-08-20 已与 8-13 的 `ICMPTest` `.sca` 对照：1422 条共有标量全同，多 145 条新统计。2026-09-04 已补干净树 `3530c2f` 的 pre-FDL `ICMPTest`：排除 `my ID` 后 1252 条旧标量全同；72 条 `my ID` 差来自 CoreNode 多了 `fdl` 后的模块编号后移。计划原文对照为有条件通过（仅 `my ID` 不同）。
4. `FDL-Calibrate`（须先 `make MODE=release`，因 `portLambdas[]` 是本周新增标量）+ `python tools/calibrate_load.py results/FDL-Calibrate` → 把实测间隔写回 `fdl_params.ini`。预期 14 个 run。
5. 到这一步，阶段二可以冻结，进入实验 A。

对应研究周：第 3 周（步骤 1-2）、第 4 周（步骤 3）、第 5 周（步骤 4）。

---

## 八、代码复核记录

以下为 2026-08-14 交叉复核发现的边界细节，已逐条对照源码确认。

| 编号 | 发现 | 判定 | 说明 |
|---|---|---|---|
| 3.2.1 | 通配分支 `findNearestLambda` 用严格 `>`，颜色分支用 `<=` | 已确认，接受 | 颜色分支在当前实验中是死代码（路由表波长全为 `*`）；恰好相等是零概率事件 |
| 3.2.2 | 颜色分支"通道空闲"判据含等号（`<=`），`findNearestLambda` 不含 | 已确认，接受 | 同上，与 3.2.1 是同一根源 |
| 3.2.3 | `burstArrival < simTime()` 严格小于，`arrivalDelta=0` 时放行但 `scheduleAt` 到过去时刻会崩溃 | 列为实验 C 前置任务 | 实验 A/B 不受影响；实验 C 和第四阶段 `offset=0` 必然触发崩溃；启动前须 smoke 复现并修复 |
| 3.2.4 | FDL 入口空闲只在决策时查一次 | 已确认，无问题 | OMNeT++ 单线程事件处理，无并发竞态 |
| 3.2.5 | 迟到丢弃只进 total 不进 contention | 已确认，补文档 | 本节第五章已补充口径说明表 |

**关于死代码推论**（3.2.1/3.2.2 补充）：`AGENTS.md` 引用的 `waitTime <= τ && FDL 空闲` 判据对应 `cc:230`，但该行位于颜色分支内。实际生效路径是通配分支的 `findNearestLambda(outPort, burstArrival+τ)`（`cc:198`），详见第四章。`fdl_tests.ini` 的测试在通配分支下结果一致（已推演验证），测试设计不受影响。
