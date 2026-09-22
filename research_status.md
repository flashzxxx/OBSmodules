# OBSmodules 项目研究状态

> 项目名称：卫星星间全光交换仿真
> 技术栈：OMNeT++ 4.x / C++ / NED
> 最终目标：实现卫星星间链路中的全光交换，数据 burst 在传输过程中不经过 OEO
> 当前重点：有条件 Go、主叙事收缩已冻结。**第 13 周已完成（2026-09-22）：实验 B 联合 2×2 与「边缘是否移动交叉点」**（`research_reports/2026-09-week13/分析.md`；图 `research_reports/figures/fig7_edge_core_2x2.png`）。三条结论：① **交叉点不移动**——回环 vs 无缓存重传在两种边缘模式下都是 **2.33×（Static）/ 2.24×（Dynamic）**（2241.2 vs 961.9、2519.7 vs 1125.7 Mbit/s），重传把 ρ 堆到 0.40–0.42 靠 24–33 万次重传副本，有效载荷只有回环的 43% / 45% ⇒ 边缘调度模式**只平移绝对损失水平，不改变相对排序**，按第 12 周验收标准不把边缘当第二个主贡献；② **边缘 Dispatcher 有一个核心计数器看不见的恒定丢弃**——Static 在 3 档负载、FDL 开/关下恒丢 **14.21–14.27%**（波动 ≤0.06 pp），Dynamic 恒 **0.00%**，据此**更正第 7 周 Task 1.3**：按核心 burst loss 读得「Static 好约 1.1 pp」，端到端口径反过来，Dynamic 好 **5.0–12.6 pp**（核心 burst loss 只能当核心内部诊断量，评价边缘或联合效果必须看端到端送达）；③ **FDL 的端到端收益 11.2–15.1 pp 是核心读数（4.5–6.2 pp）的 1.9–2.9 倍**，换算已被六组数据核验（吻合 ≤0.05 pp）：一个 burst ≈3 个包（×3），再乘边缘存活比例（Static ×0.858、Dynamic ×1），即 `端到端 = 边缘丢弃 + (1−边缘丢弃) × 网络丢弃`。四臂 burst 长度公平性已先报：Static 恒 4295.0 B（`minSizeWithPadding` 下限）、Dynamic 4206.5–4210.3 B，差 2.1%，不足以解释端到端差距。**第 12 周：图 6 载荷可行性三区图 + D2 关闭（GO）**（`research_reports/2026-09-week12/分析.md`；图 `research_reports/figures/fig6_feasibility.png`；归一化表 `ratecheck-output.txt`）。两条结论：① **三区已量化**——有用区 τ/T ∈ [0.5, 2]（峰值在 1，两个负载收益 +6.1 / +5.2 pp），< 0.5 无增益，> 2 收益衰减且 ρ≈0.59 下 τ/T=4 **为 −0.9 pp（比不加更差）**；**MEMS/光束转向被不等式一定量排除**（需 τ/T ≥ 2328 / 7276，高 3 个数量级）。② **代价决定结论**——性能最优档（τ/T=1）在成熟商用**电光**开关下要付 **10.64 dB**（3 级 × 3.5 dB），实验阶段的 **InP SOA 只要 0.14 dB**，光纤本身仅 0.14 dB ⇒ **"有用"与"付得起"在成熟器件上不重叠**，问题是"成熟度 vs 链路预算"而非"能否装载"（质量体积预算仍为假设 + 4/6/8 级敏感性）。**D2 GO**：1 Gbps 孪生点 vs 10 Gbps，丢包偏差 0.10%、最忙信道利用率 0.21%、按 T_burst 归一的事件率 ≤0.06%；两个计数量原始偏差 23% 已被测量窗口比 **1.3000**（warmup 是真实秒、时长按速率缩放）完全解释 ⇒ **扫描可按 τ/T_burst 归一化报告并外推**（仅覆盖该工作点；ρ 不可跨 0.2 s / 2 s 配置比较）。**第 11 周（2026-09-22）：225 run 批量 + 图 3–5**：同步 τ 最优 τ/T=1（6.78% / 13.51%，与第 8 周逐位一致）、异步最优 τ/T=2（15.29% / 23.22%）；回环 FDL 端到端送达载荷 **+23% / +21%**，异步下 **VF 贡献（+32% / +40%）大于延迟线本身**；热点 ρ≈0.81 上 FDL 失效（34.8–35.4% 与 τ 无关）；第 8 周判定为 **debug** 构建（release 重跑逐位复现关键值）。**实验 R**：修掉确认包被 `minSizeWithPadding=500B` 放大 2.4 倍的假象（未修不收敛），开环重传可用区间只到新发 ρ≈0.2，同实测负载下送达载荷仅约回环一半。**第 10 周**：ExpA 网格冻结。**第 9 周（2026-09-20）**：VF 调度器选项（`enableVoidFilling`，默认关，Horizon 的严格推广）与 host 侧重传基线（`src/Retransmit/`，不改 `src/EdgeNode/`）；`useFDL=false` 回归**逐标量全同**；同步偏移下 VF 补 **0 个** burst（附证明，D4 据此裁决 A+B）。第 8 周：τ 最优 τ/T≈1（仅同步模型），热点 19.5 µs → ρ=0.8074。第一篇论文骨架见 `research_reports/2026-09-week6-novelty/论文骨架.md`。**D1 已闭合（2026-09-20）**。第 7 周遗留的两篇单波长排队论文已于第 9 周取得**全文**——**τ/T_burst≈1 属已知粒度结论，不得当新发现**。不得把第 5 周标定写成回环性能。
> 当前研究周：第 13 周已完成（2026-09-22）：**实验 B 联合 2×2**（`ExpB-Joint-release`，60 run）与**重传 × 边缘配对臂**（`ExpR-EdgePair-release`，10 run），产出图 7（`research_reports/figures/fig7_edge_core_2x2.png`）与三张表（2×2 全量、逐臂 FDL 收益、回环 vs 重传）。**答案：交叉点不移动**（2.33× / 2.24×）。下一周为第 14 周：边缘 Dispatcher 队列阈值敏感性小批量（确认 14.2% 常数不是单一阈值巧合，它是本周结论的骨架）+ 论文骨架 §5.4 定稿（写入图 7 与三张表，并把第 7 周 Task 1.3 的更正落到骨架对应段落）；若时间允许，给重传臂加固定窗口把 ρ 推到 0.6 以上，验证交叉点在全负载区间是否仍然不动。
> 更新时间：2026-09-22（第 13 周收口：实验 B 联合 2×2 + 5.4 节交叉点答案）

---

## 一、文档用途

本文件用于保存跨 Codex 窗口的长期研究上下文。后续进入本项目时，优先读取本文件，再读取：

1. `research_reports/00-project-management/codex_phase2_tasks.md`：当前确认的逐周执行计划、任务边界和验收标准
2. `.agents/skills/obs-satellite-project/references/research_guide.md`：详细技术约束、任务提示词和验收标准
3. `walkthrough.md`：最近一次代码变更、验证过程和遗留问题

本文件只保留已经形成的研究判断、当前证据和下一步决策，不替代代码注释，也不替代实验原始数据。

### 新聊天启动方式

在新的 Codex 聊天中打开 `D:\01work\project\OBSmodules` 工作区，然后发送：

> 继续 OBS 卫星全光交换研究。请先完整读取项目根目录下的 `AGENTS.md`、`research_status.md`、`walkthrough.md` 和 `research_reports/00-project-management/codex_phase2_tasks.md`，并按 `obs-satellite-project` Skill 执行；再检查 Git 和实际代码状态，确认文档记录与仓库一致。以 `research_status.md` 的当前研究周为起点，完成本周任务并更新状态文档和 `walkthrough.md`。保护 `src/EdgeNode/` 的第一阶段基线；新功能确有需要时可受控扩展，但须保持原模式和旧实验可复现。不要把尚未验证的实现写成已完成结论。编译和仿真由用户执行；Agent 不运行 `make`。`-linet` 已解决。

如果只想讨论方案、不希望修改代码，在末尾增加：

> 本次只做研究分析和方案讨论，不修改代码、不运行仿真。

如果希望直接推进当前周任务，在末尾增加：

> 请直接推进当前研究周，完成能够自主完成的代码、配置、核对和周会材料，不停留在计划描述。编译和仿真由用户执行；Agent 不运行 make。release 链接（`-linet`）已解决，不要再当作阻塞。

---

## 二、研究目标与基本判断

### 最终研究目标

实现卫星星间链路中的全光交换：数据 burst 从一个卫星节点进入网络，到离开网络的过程中始终保持光信号形式，不在核心节点进行 O/E/O 转换。

### 当前问题的根源

现有 OBS 架构中，数据 burst 可以在光域转发，但 BCP 控制包仍需在核心节点进行接收、解析和处理。因而当前系统属于“数据面光转发、控制面电子处理”的部分全光方案，尚未满足严格意义上的全光交换。

### 已确认的技术判断

1. FDL 是全光交换的重要光域缓冲基础设施。没有光域暂存能力，offset 缩小后核心节点将缺少读取控制信息和配置 OXC 的时间窗口。
2. FDL 本身不能直接证明完整的全光交换已经实现。还需要验证控制信息的承载方式、burst 到达时序以及 OXC 配置过程是否能够在无独立 BCP 的条件下闭合。
3. 在 W=1 单波长星间链路约束下，FDL 的物理延迟、burst 时长、链路负载和边缘调度之间存在耦合关系，研究重点应放在可实现工作区间，而非单个参数的最优值。
4. 边缘调度和核心 FDL 需要通过联合实验判断相互作用。边缘调度在本研究中的价值，主要体现在与 FDL 的对照实验，而非单独重复第一阶段成果。

### 2026-08-14 新增判断：回环 FDL 不能证明 offset→0

现有 FDL 是**回环 FDL**：burst 进入 FDL 需要控制逻辑提前预约 OXC（`OBS_CoreControlLogic.cc:292-305`），这本身要求 offset > 0。offset = 0 时 OXC 未配置，burst 会被 OXC 直接丢弃，回环 FDL 无法介入。

支撑 offset→0 的是**输入 FDL**：位于每个输入端口、OXC 之前、无条件生效的固定延迟线，用于吸收标签读取与 OXC 配置时间。两者位置、数量、触发条件都不同。

因此：第二层的回环 FDL 有效回答研究问题 ① ②（τ 怎么选、FDL 够不够），但不能回答问题 ③。实验 C 必须先建立输入 FDL 时序模型。

### 2026-08-14 修正：目标口径

原挂起的"全光是否含控制面""`offset=0` 是否硬目标"两个问题，按以下口径关闭：

> **边缘节点发出时 offset = 0**（不再提前发送独立 BCP），核心节点内部所需处理时间由输入 FDL 吸收。严格 `offset = 0`（含节点内部）物理上不可达，不作为目标。

论文可证明的上限是"数据面严格全光 + 控制面定量时序预算"；控制面全光作为 future work。不把论文标题绑定在严格全光控制面上。

### 研究边界

本研究聚焦于已经分配的单波长通道内的 OBS burst 调度与 FDL 机制。波长分配由上层算法决定，本方案不重新设计波长分配算法；每条已分配的波长通道独立执行核心调度。

---

## 三、总体技术路线

```mermaid
flowchart LR
    A[现有 OBS 基线\nBCP + 正 offset] --> B[核心 FDL 机制\n光域延迟与竞争缓解]
    B --> C[FDL 工作区间\nτ、负载、burst 时长]
    C --> D[边缘-核心联合调度\n2×2 对照实验]
    D --> E[offset 缩减\n验证控制窗口边界]
    E --> F[光标签原型\n控制信息随 burst 传输]
    F --> G[严格全光交换\n数据面与控制面均不 OEO]
```

### 三层架构

| 层级 | 研究内容 | 当前状态 |
|---|---|---|
| 第一层：边缘节点 | P1-P4 优先级、LRU 抢占、Dynamic / NoPreemption / RoundRobin / Static | 代码已完成；作为联合实验变量 |
| 第二层：核心节点 FDL | No-FDL / Always-FDL、单次回环、FDL horizon 管理 | 模块和调度逻辑已实现；release 已通过；三分支功能测试已通过；当前树 ICMP 回归已通过；pre-FDL 提交对照有条件通过（仅 `my ID` 不同） |
| 第三层：光标签控制 | burst 携带控制信息，减少或取消独立 BCP，目标探索 `offset≈0` | 远期；依赖第二层实验结论 |

---

## 四、当前工作状态

| 任务 | 目标 | 状态 | 当前证据或缺口 |
|---|---|---|---|
| 第一阶段边缘调度 | 完成四种调度模式和 Burstifier 接口 | 已完成 | 代码已存在；需要使用干净仿真数据重新确认结果 |
| Task 1.1 Git 与仓库整理 | 建立可追踪的代码和结果基线 | 基本完成 | 已有基础提交；工作区仍有待整理的本地文件 |
| Task 1.2 CoreNode 调研 | 标注 FDL 集成点和调度数据流 | 已完成 | 研究指南和代码分析材料已形成 |
| Task 1.3 第一阶段实验重跑 | 从 `.sca` 读取真实指标并生成图表 | **已完成（2026-09-18），但交付口径需修正** | 68 run 全部 exit 0，图见 `results/task1.3-2026-09-18/phase1_four_modes.png`，脚本 `tools/plot_phase1_modes.py`。**发现：第一阶段负载仅 0.24%，44 个 run 丢包恒为 0**，交付不了"带丢包的 Static/Dynamic 基线"；已另补 `Task13-LoadedModes`（标定负载、FDL 关）顶替。详见 `research_reports/2026-09-week7/分析-task1.3与排队论.md` |
| Task 2.1 FDL 模块 | 新建 FDL simple module，接入 CoreNode/OXC | 已提交 | 最近提交：`1d45cf4 feat: add FDL module and integrate into CoreNode` |
| Task 2.2 FDL 调度逻辑 | 支持 `useFDL`、直通/回环/丢弃三种场景 | 已实现；静态复核无阻断；用户确认 release 已通过 | `-linet` 已由用户解决；三分支与当前树 ICMP 回归已用仿真验证 |
| Task 2.3 FDL 功能验证 | 验证回环、关闭兼容性和统计指标 | 三分支已通过；当前树 ICMP 回归已通过；pre-FDL 提交对照有条件通过（仅 `my ID` 不同） | Basic/TooShort/Off 见 `results/Test-FDL-*-0.sca`；Compatibility 见 `results/Test-FDL-Compatibility-0.sca`；pre-FDL 见 `results/pre-fdl-baseline/ICMPTest-0.sca` |
| 实验平台重建 | W=1、可标定负载、随机化、多源流量 | 已完成（2026-09-09；**09-18 重标定**） | 第 5 周 14 run；ρ=0.4 → 35.9 µs。**09-18 因 guardTime 修复重标定：ρ=0.4 → 35.6 µs**（`results/FDL-Calibrate-recal-2026-09-18/`，平台检查全绿）。仅证明 `useFDL=false` 负载，不证明回环 |
| 第 6 周文献定位 | go/no-go：本文新在哪 | 已完成（2026-09-11；09-14 补盲区；**09-18 补排队论**） | **有条件 Go、主叙事收缩**。Zhao 取证=SPIE HTML 摘录（非正式出版 PDF）。材料：`research_reports/2026-09-week6-novelty/`（正式三份 + `检索记录.md` + `D1-询问稿.md`）。书目 canonical：`research_reports/2026-09-week6-novelty/refs/refs.bib`（每读一篇立即登记）。**09-18 补排队论 10 条（题录级）**，并证伪"排队论都假定多波长"：`laevens2003single`/`vanhoudt2004channel` 即单波长，后者口径与 5.1 直接重叠 |
| 阶段三实验 A/B + ExpR | 第一篇：回环工作区间、重传对照、载荷可行性、边缘是否移动交叉点 | **ExpA（第 11 周）、图 6 与 D2（第 12 周）、ExpB 与交叉点（第 13 周）均已完成** | 第 11 周 225 run（release，repeat 5）：同步 τ/T=1 最优（6.78% / 13.51%）、异步 τ/T=2 最优（15.29% / 23.22%）、热点上 FDL 失效（丢包 34.8–35.4% 与 τ 无关）；端到端送达载荷回环 +21–23%、异步 VF +32–40%；重传同实测负载下只有回环一半且可用区间更窄。图 3–5 初版已出。**第 13 周**：`ExpB-Joint` 60 run + `ExpR-EdgePair` 10 run，图 7 三面板；**交叉点不移动**（2.33× / 2.24×），并更正第 7 周 Task 1.3（核心 burst loss 读得 Static 好 1.1 pp，端到端反而 Dynamic 好 5.0–12.6 pp）。**尚缺**：ExpR 若要高负载须先做重传窗口（第 14 周备选）；边缘队列阈值敏感性（第 14 周） |
| 实验 C / 光标签原型 | 输入 FDL 时序与控制信息随 burst 传输 | 第二篇；第 17 周起条件阶段 | 不进第一篇主线。立项页放在第 16 周 |

### 实验平台的三个既有缺陷（2026-08-14 核对）

这三条如不修正，实验 A/B 会跑出空结果。修正已写入 `fdl_params.ini` 的 `FDL-Scenario`，并已用 `FDL-Calibrate` 的 14 个 `.sca` 核对。

| 缺陷 | 实测 | 影响 |
|---|---|---|
| W=1 被破坏 | `params.ini:34` 设 `lambdasCore1to2 = 3` | 全网最拥塞的链路恰是唯一违反 W=1 的链路，所有结论都不是 W=1 结论 |
| 负载约 0.24% | FlowScaling 为 12 流 × 500B/2ms = 24 Mbps 对 10 Gbps 信道；LoadIntensity 最快档约 0.4% | 核心节点几乎无竞争，`fdlUsageCount` 会是 0，τ 扫描得到平直线。第 1 周 smoke 中"核心丢弃为 0"已是此现象 |
| 仿真确定性 | `UDPBasicApp` 用固定 `sendInterval` 与固定 `startTime` | 换随机种子结果完全相同，重复次数与置信区间无意义 |

另有两项待正式实验前确认：`BCPProcessingDelay`／`OEConversionDelay`／`EOConversionDelay` 当前均为 `0s`（offset 沿路径永不消耗，会使实验 C 得到虚假结论）；第一阶段流量全部源自 sat1，拓扑退化为单源星形，中间节点不存在多源汇聚竞争。

### FDL 当前实现口径

| 场景 | 判断条件 | 预期行为 |
|---|---|---|
| A：直通 | 目标通道在 burst 到达时可用 | 一次 OXC 预约，直接转发 |
| B：FDL 回环 | 目标通道冲突、等待时间不超过 τ、FDL 空闲 | `输入 → FDL → 目标输出`，进行两次 OXC 预约 |
| C：丢弃 | 直通和回环条件均不满足 | 丢弃 burst，并记录竞争丢包 |

当前只保留两种 FDL 开关模式：`useFDL=false` 作为原有行为基线，`useFDL=true` 作为 Always-FDL 验证模式。暂不预设 Load-Aware-FDL，是否需要引入由实验数据决定。

对照基线（不是新的 FDL 策略，默认关闭）：Horizon+void filling 调度器选项；host 侧无缓存重传。二者用于第一篇论文的对照组，不改变 `useFDL=false` 路径。

---

## 五、必须区分的“已知”和“待证明”

| 内容 | 当前结论 |
|---|---|
| FDL 模块是否已写入仿真模型 | 是 |
| FDL 是否已经通过编译 | debug 与 release 均已通过（release 为 2026-08-19 用户确认）；`-linet` 已解决 |
| FDL 回环是否确实被 burst 使用 | 是。确定性 Basic 中 sat2 的 `fdlUsageCount=1`，器件 `FDL usage count=2`（同一突发头尾），sat3 收齐 2 包 |
| `useFDL=false` 是否完全向后兼容 | 相对干净树 `3530c2f`：排除 `my ID` 后 1252 条旧标量全同，FDL 计数为 0；72 条 `my ID` 差是 CoreNode 多了 `fdl` 后的模块编号后移。相对 2026-08-13 的 `ICMPTest`：1422 条共有标量全同。计划原文对照为有条件通过 |
| FDL 是否能降低丢包率 | 需要实验 A/B 数据，不能凭代码推断 |
| FDL 是否支持任意小 offset | 未知，受 τ、处理时间、burst 长度和负载共同限制 |
| FDL 是否等于完整全光交换 | 否。它解决光域暂存问题，但尚未解决独立 BCP 的 OEO 问题 |
| `offset=0` 是否为最终必须目标 | 否。已改口径为"边缘 offset=0"，严格 0 物理不可达 |
| 现有回环 FDL 能否支撑 offset→0 | 否。回环需要提前预约 OXC，本身要求 offset > 0；需另建输入 FDL |
| FDL horizon 未加 τ 是否为缺陷 | 否。FDL 是延迟线不是缓存，horizon 建模的是入口占用，定长延迟保证出口不碰撞 |
| 当前实验配置能否支撑实验 A/B | 可以。M1 已锁 Scenario B +τ；负载已按修复后模型重标定（ρ=0.4 → 35.6 µs）；`seed-set` 缺口已补（6 个 config）。实验 B 基线改由 `Task13-LoadedModes` 提供（第一阶段负载仅 0.24%、零丢包，不足以当基线） |
| 第 6 周 novelty | 有条件 Go。不能声称首次 OBS+FDL 或首次卫星/W=1 OBS；必须对标 Zhao 与 L-OBS。书目见 `research_reports/2026-09-week6-novelty/refs/` |
| Void filling 是否改变回环结论 | **已实测（2026-09-20）**：同步偏移（各源同 `maxOffset`）下**不改变任何判定**——`voidFilledBursts = 0`，与 Horizon 逐标量相同；把各源偏移改成 700–970 µs 后 VF 生效（全网已调度 burst +23.0%）。实现是 Horizon 的严格推广，默认关闭 |
| 无缓存重传基线是否存在 | **已存在且已批量（2026-09-22）**：`src/Retransmit/`（源 + 确认 sink），`ExpR-Retransmit` 已接线。**同实测负载下送达载荷仅约回环臂一半**（1145 / 1126 对 2046–2520 Mbit/s），`retx/fresh ≈ 1.0`、线上有用率 ~61%；**可用区间更窄**（开环只能到新发 ρ≈0.2，35.6 µs 无界增长），且需一对超时值（60 / 120 ms）并列报告 |
| 全互联下 ρ=0.8 是否可达 | 否。第 5 周最重点约 0.59。出路是热点流量矩阵（`FDL-Hotspot`），不是继续缩间隔 |

---

## 六、逐周研究执行看板

### 计划口径

- 每个研究周投入 2 个完整工作日，不按自然周强行赶进度。
- 研究日 A 负责实现、配置、仿真或推导；研究日 B 负责验证、分析和周会材料。
- 每周必须形成图、表、日志或真实仿真数据之一，不能只汇报“做了代码”。
- 高复杂度任务被拆成多个周目标；某周验收未通过时，不跳过问题进入下一周。

### 16 个研究周主线

计划于 2026-08-14 修订，**2026-09-14 再修订**：第 7–16 周按第一篇论文骨架重排；实验 C 移出 16 周主线，进第 17 周起条件阶段。完整表格与验收标准见 `codex_phase2_tasks.md`。叙事唯一源见第十节。

| 周次 | 研究日 A | 研究日 B | 复杂度 | 状态 |
|---|---|---|---|---|
| 第 1 周 | 明确严格全光交换定义、当前差距和候选路线 | 核对 Task 2.2 状态，形成阶段基线 | 中 | 已完成（2026-08-13） |
| 第 2 周 | 修复 release 链接（`-linet`） | 复核两段 OXC 预约、horizon 和 BCP 时间更新 | 高 | 已完成（2026-08-19）：用户确认 release 已生成；静态复核无阻断 |
| 第 3 周 | 运行 `fdl_tests.ini` 三分支确定性场景 | 核对 EV、`.sca` 和 burst 到达时刻 | 高 | 已完成（2026-08-19）：三分支标量通过；EV 全文未归档 |
| 第 4 周 | 运行 `Test-FDL-Compatibility` 与 pre-FDL 对照 | 修复回归差异，检查新增统计量 | 高 | 已完成（2026-08-20 当前树 ICMP 回归；2026-09-04 pre-FDL 对照有条件通过，仅 `my ID` 不同） |
| 第 5 周 | 运行 `FDL-Calibrate`，回写负载常数 k | 核对 W=1 生效、多源流量成立、随机化有效 | 中 | 已完成（2026-09-09）：14 run；ρ=0.4 对应 35.9 µs；ρ=0.8 本扫描达不到 |
| 第 6 周 | 文献定位（go/no-go 决策门）：FDL/OBS 竞争缓解已有工作梳理 | 确定新颖性缺口；no-go 时须给出替代方案 | 中 | 已完成（2026-09-11 Go；2026-09-14 修取证/检索/盲区并入库） |
| 第 7 周 | Task 1.3：第一阶段实验干净重跑 | 用户跑 `Test-FDL-Basic/TooShort/Off` 过 M1；补 FDL 排队论检索 | 中 | **已完成（2026-09-18）**：M1 过；Task 1.3 交付口径修正（第一阶段零丢包，另补有负载基线）；排队论 10 条入库。另计划外修复 guardTime 缺陷并重标定 |
| 第 8 周 | 热点流量矩阵，确认瓶颈 ρ≥0.8 | 复标定；确认 D1 已发出（冻结日 2026-09-28） | 中 | **已完成（2026-09-18）**：瓶颈 sat3→sat2，**19.5 µs → ρ=0.8074** 且种子有差异；另提前跑通 ExpA 批量并修复 `maxFdlLoopsPerBurst` 红线配置。**D1 改为公开文献建表，器件参数已闭合**（MEMS 被定量排除） |
| 第 9 周 | 实现 VF 调度器选项（默认关） | 实现 host 侧重传基线（默认关）；两者确定性用例 | 高 | **已完成（2026-09-20）**：`OBS_ChannelCalendar` + `enableVoidFilling`、`src/Retransmit/` 两模块；VF 4 例 + 重传 2 例 + 载入诊断 4 例；`useFDL=false` 回归**逐标量全同**（1701/1701、1697/1697）；两篇单波长排队论**全文已读**。另得负面结论：同步偏移下 VF 补 0 个 burst（附证明） |
| 第 10 周 | ExpA pilot：负载 × 调度器 × τ | 冻结网格；载荷延迟器不等式写成文档 | 中 | **已完成（2026-09-22）**：网格冻结见 `research_reports/2026-09-week10/网格冻结.md`；异步臂 54 run 全部 exit 0 + pilot 曲线；**τ 最优点随偏移模型移动**（同步 τ/T=1、异步 Horizon τ/T=2）；异步下 VF 收益 ≥ FDL 收益；热点 ρ≈0.81 上 FDL 只买到 0.1–0.6 pp |
| 第 11 周 | ExpA 批量（按冻结表，repeat 5） | ExpR 批量（2 s） | 中 | **已完成（2026-09-22）**：225 run 全部 exit 0（异步 100 + 参考线 20 + 热点 30 + 同步 release 重跑 60 + 轻负载参考 5 + ExpR 5 + ExpR-120ms 5）；图 3–5 初版与 `batch_summary.csv`；**第 8 周判定为 debug 构建**，release 重跑逐位复现 12.88 / 18.71 / 6.78 / 13.51；实验 R 修掉确认包填充假象并查清开环重传的可用边界（新发 ρ≈0.2） |
| 第 12 周 | 可行性图（`fdl_design.py` 接器件表，图 6 三区） | RateCheck（关闭 D2） | 中 | **已完成（2026-09-22）**：图 6 双面板（有用区 τ/T∈[0.5,2]；器件代价电光 10.64 dB vs SOA 0.14 dB；MEMS/光束转向被不等式一定量排除）；**D2 关闭 = GO**（丢包 0.10%、利用率 0.21%、事件率 ≤0.06% 按 T_burst 归一；23% 原始计数差已由窗口比 1.3000 解释）|
| 第 13 周 | ExpB 2×2 批量 | 分析：联合是否移动交叉点 | 中 | **已完成（2026-09-22）**：`ExpB-Joint-release` 60 run + `ExpR-EdgePair-release` 10 run，审计 0 MISSING/DUPLICATE/STALE；图 7 三面板。**答案：交叉点不移动**（回环/重传 = 2.33× Static、2.24× Dynamic）。**边缘 Dispatcher 恒丢 14.21–14.27%（Static）/ 0.00%（Dynamic），核心计数器看不见 ⇒ 更正第 7 周 Task 1.3**：核心读得 Static 好 1.1 pp，端到端反而 Dynamic 好 5.0–12.6 pp。FDL 端到端收益 11.2–15.1 pp = 核心读数的 1.9–2.9 倍（六组核验 ≤0.05 pp）|
| 第 14 周 | 敏感性（突发长度、热点）+ 边缘队列阈值敏感性 | 论文第 3–5 章初稿；骨架 §5.4 定稿 | 中 | 待开始 |
| 第 15 周 | 全文初稿 | 内部评审 | 中 | 待开始 |
| 第 16 周 | 修改并投稿（目标 ≤ 2027-03） | 结题报告框架；实验 C 立项页 | 中 | 待开始 |

论文并行线：从第 11 周起每完成一组实验立即写入论文对应小节，第 15–16 周只做整合。实验 C 不在本表主线。

### 条件阶段：第 17～20 周

第一篇投稿后启动。先做实验 C（输入 FDL 时序），再决定是否进入原光标签最小原型；否则转向保留极小正 offset 的过渡架构。

| 周次 | 任务 | 复杂度 | 状态 |
|---|---|---|---|
| 第 17 周 | 实验 C 立项；输入 FDL 时序模型 | 高 | 条件任务 |
| 第 18 周 | 迟到判据修复；offset 缩减 pilot | 高 | 条件任务 |
| 第 19 周 | 实验 C 批量（有/无输入 FDL） | 高 | 条件任务 |
| 第 20 周 | 是否启动光标签最小原型的决策 | 高 | 条件任务 |

### 当前第 1 周的具体交付

| 交付物 | 内容 | 完成判据 |
|---|---|---|
| 研究方向页 | 严格全光目标、现有 OBS 差距、FDL 的真实定位 | 不把 FDL 等同于完整全光交换 |
| 技术路线页 | 基线 OBS → FDL 验证 → offset 缩减 → 光标签候选架构 | 明确每一步需要什么证据 |
| 待确认问题 | 全光交换是否包含控制面；`offset=0` 是否为硬目标 | 获得领导意见或记录待决策状态 |
| 代码基线表 | Task 2.1、2.2、2.3 的实现和验证状态 | 区分“已实现”和“已验证” |

### 第 1 周完成记录（2026-08-13）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| Git 与代码核对 | 分支 `task/2.1-fdl-module`，HEAD `1d45cf4`；Task 2.2 的 3 个文件仍为未提交修改 | Task 2.1 已提交，Task 2.2 已实现 | Task 2.2 尚未提交、尚未验证 |
| EdgeNode 保护检查 | `git diff -- src/EdgeNode` 为空，四种 `dispatchMode` 仍为 0～3 | 本周没有改动受保护基线 | 不能替代后续端到端基线回归 |
| release 编译 | 全部 C++/消息对象编译完成，最终链接报 `cannot find -linet` | 当前源码没有暴露编译错误 | release 可执行文件尚未生成 |
| debug 强制全量编译 | `make -B MODE=debug` 退出码 0，生成 `out/gcc-debug/obsmodules.exe` | 当前工作树可完整编译并链接 | 不等于 release 验收通过 |
| ICMP smoke 仿真 | 当前 debug 模型运行至事件 4569；20 个 ping 全部接收；FDL/核心丢弃合计均为 0 | 模型可启动、`useFDL=false` 场景可结束、标量可写出 | 未触发 FDL；未与旧提交逐标量对照；不是 W=1 实验结论 |
| 配置核对 | 实际配置在 `Examples/RingFdlOBS/`；sat1↔sat2 配置 3 个数据波长 | 识别了文档路径和 W=1 基线偏差 | 现有结果不能直接用于正式 FDL 实验 |

周会材料：`research_reports/2026-08-week1-baseline/周会材料.md`。真实 smoke 数据位于 `Examples/RingFdlOBS/results/week1-smoke/`，属于本次运行证据，不作为阶段三性能数据。

### 当前第 2 周的具体交付

| 交付物 | 内容 | 完成判据 |
|---|---|---|
| release 可执行文件 | 用户完成 `make MODE=release` | 链接不再报 `cannot find -linet` |
| FDL 调度时序图 | 直通 / 回环两段预约 / 丢弃 | 静态时序无已知阻断问题 |
| 问题清单 | 通配分支边界、horizon 不加 τ、实验 C 前置崩溃 | 不把可接受差异改成行为变更 |

### 第 2 周完成记录（2026-08-19）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| release 链接 | 用户确认 `-linet` 已解决且 release 可执行文件已生成 | 第 2 周研究日 A 通过；可进入 Task 2.3 | Agent 本会话未重跑 `make`；未用该 exe 跑 FDL 测试 |
| 静态时序复核 | 两段 OXC 预约、FDL 入口 horizon、BCP `+τ` 与源码一致 | 核心时序无第 2 周阻断问题 | 不是 EV 日志或 `.sca` 证明 |
| `git diff --check` | 修正 `OBS_CoreControlLogic.cc` 四处空白 | 空白不改变调度语义 | 不能替代功能测试 |
| EdgeNode 保护检查 | 本周未改 `src/EdgeNode/` | 未触碰受保护基线 | 不能替代第 4 周兼容性对照 |

周会材料：`research_reports/2026-08-week2-release/周会材料.md`。平台重建 walkthrough 归档于 `research_reports/2026-08-plan-review/walkthrough.md`。

### 第 3 周完成记录（2026-08-19）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| Test-FDL-Basic `.sca` | sat2 `fdlUsageCount=1`，丢包 0，sat3 收 2 包 | 回环分支可达 | 未归档 EV，不能文件证明预约间隔恰为 20 µs |
| Test-FDL-TooShort `.sca` | sat2 `fdlUsageCount=0`，竞争丢包 1，sat3 收 1 包 | τ 不够时拒绝回环，不是无条件进 FDL | 不是负载下的丢包率 |
| Test-FDL-Off `.sca` | 与 TooShort 1577 条标量全同 | 开关关闭有效 | 不能代替 ICMP 场景的 pre-FDL 基线对照（该对照已于 2026-09-04 补完） |

分析全文：`research_reports/2026-08-week3-fdl-tests/分析.md`。周会材料：`research_reports/2026-08-week3-fdl-tests/周会材料.md`。

### 第 4 周完成记录（2026-08-20；2026-09-04 补 pre-FDL 对照）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| `Test-FDL-Compatibility-0.sca` | ping 两路各 10/10、loss 0；10 个节点 `fdlUsageCount`、`FDL usage count`、`burstLossContention` 全为 0 | 关 FDL 时旧 ICMP 场景可结束，回环未被触发 | 不能写成 FDL 性能结论 |
| 对 `results/pre-fdl-baseline/ICMPTest-0.sca`（干净树 `3530c2f`，2026-09-04 17:03） | 共有 1332；相同 1260；不同 72 全是 `my ID`；多 235 条后加统计；只在 pre-FDL 0 条。排除 `my ID` 后 1252 条全同 | 计划原文的 pre-FDL 对照有条件通过；旧行为标量未被改掉 | `my ID` 与 pre-FDL 逐值相同（结构编号后移） |
| 对 `results/ICMPTest-0.sca`（2026-08-13 18:50） | 共有 1422 条标量全同；多 145 条，全是 8-14 新增统计 | 当前树相对 8-13 FDL 代码树无旧标量回归 | 对照文件本身已含 FDL 代码 |
| 对 `results/week1-smoke/ICMPTest-0.sca` | 同上：1422 同、0 差、145 新 | 与第 1 周 smoke 的旧标量一致 | 第 1 周 smoke 也不是 pre-FDL 提交 |

分析全文：`research_reports/2026-08-week4-compatibility/分析.md`。周会材料：`research_reports/2026-08-week4-compatibility/周会材料.md`。

### 第 5 周完成记录（2026-09-09）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| `FDL-Scenario` 波长冻结 | 13 条 ISL 与 10 个节点的端口串全部写为 1，不再只改 sat1/sat2 | FDL 实验不再吃 `params.ini` 的 `lambdasCore1to2=3` | 静态 ini 不能代替本次运行的 `portLambdas[]` |
| `portLambdas[p]` | `finish()` 新增，不改变调度 | W=1 可从 `.sca` 直接核对 | 未重新编译则 `.sca` 里没有该标量 |
| `FDL-Calibrate` | 7 档间隔 × `repeat=2`，`seed-set=${runnumber}`；14 个 `.sca` 已齐 | 一次扫描同时拟合 k 并做种子差异检查 | 不能写成回环 FDL 性能 |
| `calibrate_load.py` | 实测 ρ=0.4 → **35.9 µs**；W=1 / 多源 / 双种子 / FDL 关闭均通过 | 已回写默认间隔 | ρ=0.8 本扫描达不到（最重点 ≈0.59） |

分析全文：`research_reports/2026-09-week5-calibrate/分析.md`。周会材料：`research_reports/2026-09-week5-calibrate/周会材料.md`。第 5 周 walkthrough：`research_reports/2026-09-week5-calibrate/walkthrough.md`。

### 第 6 周完成记录（2026-09-11；2026-09-14 修订）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| 近邻核对 | Chen 2004、ChinaCom 2010、Zhao 2022 HTML 摘录、ADI 2025 HTML、L-OBS 2008/2010 PDF、Roethig/Mouammar 2026 PDF、Teng 2005 PDF | 对照表可定稿；Zhao 已占住卫星 OBS+W=1 且不用 FDL；L-OBS 输入 FDL ≈ 1.7 µs | **不是**通读 Zhao 出版 PDF；Lee 2006 / Qiao 2000 仍无 PDF |
| Zhao 取证统一 | 09-11 与 09-14 HTML 同文；官方 SPIE 被 Incapsula 拦截；数字逐条标摘录位置 | 利用率 0.563、丢包 63%、Table 7 估时可引用，须标 HTML 摘录 | 不可写「已读出版 PDF」或「只看过摘要」 |
| 检索留痕 | `检索记录.md`；无机构 Scopus 导出 | Go 决策可辩护到「网页+作者稿+专利+中文题录」这一层 | 不能声称已做完整 IEEE/CNKI 集合检索 |
| 盲区补扫 | 2023–2026 地面 FDL 刊；CN108809719A；中文星载 OBS 题录 | 组合缺口仍在，Go 维持；相关工作须写专利与中文 | 中文期刊原文、多数 2023–2026 刊未下 PDF |
| 用户裁剪 | 同意有条件 Go、主叙事收缩 | 可进入回环确定性用例，再谈实验 A | 不等于回环性能已证明 |
| 第一篇论文骨架 | 2026-09-14 晚落地 `论文骨架.md`；第 7–16 周按此重排 | 后续周次有唯一叙事源 | 不是投稿正文；VF/重传/ρ=0.8 尚未实现 |

周会材料：`research_reports/2026-09-week6-novelty/周会材料.md`。精读：`近邻论文精读分析.md`。D1：`D1-询问稿.md`。书目与原文：`research_reports/2026-09-week6-novelty/refs/`（canonical `refs.bib`；入口指针 `research_reports/refs/README.md`）。

### 第 7 周完成记录（2026-09-18）

计划内三项 + 一项计划外但必须做的修复。

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| M1 全量复跑 | Basic 两次 OXC 预约间隔**恰 20 µs = τ**；sat3 收到的 BCP offset 差**恰为 τ**；TooShort≡Off 1701 条全同；Compatibility 对 pre-FDL 基线非 `my ID` 差异 **0**；3 个负向用例 60/60 干净 `exit=1` | **Scenario B +τ 时序已锁**；`useFDL=false` 向后兼容成立；负向用例真正可达 | 不是回环性能；不是负载下的结论 |
| 审核遗留缺陷修复 | `Test-FDL-TauMismatch` 因 NED 绑定永不可能失败（假阳性，已换成 2 个可达用例）；负向用例 `0xC0000005` 崩溃（gdb 追出 3 个独立根因：`lambdasPerPort` 堆越界写、9 个模块析构释放未初始化指针、`OBS_CoreOutput` 析构以计数为界） | 负向用例从此可信；错误路径不再崩溃 | 不改变任何调度语义（正常配置零差异） |
| **计划外：ExpA 全线崩溃** | `ExpA-Pilot`/`ExpA-NoFDL`/`FDL-Calibrate` 均在 **t=1.94 ms** 崩溃。根因：09-15 H1 的 `guardTime=2us` 只覆盖 `**.sat*.coreSwitch.**`，边缘 `**.sender.guardTime` 仍为 1 ns → 预约窗口 `D+1.5µs` 对 pacing `D+1ns` → 实测 **12 次重叠，每次恰 1.50 µs，全在 `inGate0`（本地边缘口）** | 修复后可跑；A/B 对照（两端都 2 µs → exit 0）；M1 仍零差异 | 不是 FDL 逻辑本身的问题；但暴露出调度器**无输入口 horizon** |
| 重标定 | `results/FDL-Calibrate-recal-2026-09-18/` 14 run；**ρ=0.4 → 35.6 µs**（原 35.9）；W=1/多源/随机化/FDL off 全 PASS | 负载工作点已对齐新模型 | 仍不证明回环性能 |
| ExpA-Pilot 方差分析（H5） | 种子间 CV 0.2–0.7%；FDL 开/关效应量 **6.1–6.8 pp**；n=5 可分辨下限 0.05–0.10 pp → 信噪比 **71–123:1** | **`repeat = 5` 足够**；H5 的担心不成立 | 不覆盖 τ 扫描两端的零效应情形 |
| Task 1.3 四模式重跑 | 68 run 全部 exit 0；图 `results/task1.3-2026-09-18/phase1_four_modes.png`；脚本 `tools/plot_phase1_modes.py` | 模式在聚合行为与 P1–P4 命中上差异可量化 | **第一阶段负载仅 0.24%，44 run 丢包恒为 0** → 交付不了带丢包的基线 |
| 补有负载四模式基线 | Static 17.18%/11.77%、NoPreemption 18.49%/12.79%、Dynamic 18.71%/12.87%、RoundRobin 18.97%/13.28%（ρ≈0.59/0.40） | 实验 B 有了带丢包参考面；Static 优于 Dynamic ≈1.1 pp | RoundRobin 突发长差 2.7 倍，公平性未解决前不进 2×2 |
| 排队论检索 | 10 条入 `refs.bib`（**题录级，未取得全文**）；更正 3 处归属/口径 | 排队论上界一线已可引用；发现 `laevens2003single`/`vanhoudt2004channel` 即单波长、后者口径与 5.1 重叠 | **不能写"已读"**；2.1 正文前须取得这两篇全文 |
| 决策落定 | 回写 35.6/79.8 µs；H6.2 第一篇不做（sink 级 `endToEndDelay:histogram` 已在记录，种子 CV 0.1%，FDL 开/关差 0.6–0.9 ms）；临时 ini 清理 | 无需为时延改 EdgeNode，ExpA 可直接跑 | H6.2 若将来需要留第二篇 |

周会材料：`research_reports/2026-09-week7/周会材料.md`。分析：`分析.md`（M1/guardTime/重标定/pilot）、`分析-task1.3与排队论.md`（Task 1.3 与排队论）。

**数据完整性**：2026-09-18 Agent 用 `-r 0` 做对照时覆盖了第 5 周的 `results/FDL-Calibrate/FDL-Calibrate-0.sca`（400 µs / rep 0），**无法恢复**。影响已定量核查为可忽略（ρ=0.4 工作点由 42.6/30 µs 插值，不经该点；13 点与 14 点给出同为 35.58 µs），其派生数值留存在 `research_reports/2026-09-week5-calibrate/calibrate_report.txt`。其余 13 个 run 完好，全部历史结果已备份到 `results/_historical-2026-09/`。

### 第 8 周完成记录（2026-09-18）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| ExpA 批量（提前执行第 11 周） | `ExpA-TauSweep` 5 τ × 2 负载 × repeat 5 = 50 run + `ExpA-NoFDL` 10 run，全部 exit 0 | τ 工作区间已可画（图 3 / 图 4 雏形） | 不含第三档负载、不含 VF 维度；不能当"最终网格" |
| τ 扫描结果 | τ/T_burst ≈ **1 为最优**：ρ=0.40 时 12.88% → **6.78%**；ρ=0.59 时 18.71% → **13.51%**。τ/T=4 时 ρ=0.59 下 **19.61% 反高于 No-FDL 的 18.71%** | **"收益边界"有直接数据支撑**；延迟线过长不如不加 | 单次 τ 扫描，未做突发长度敏感性 |
| **红线缺陷（配置层）** | `maxFdlLoopsPerBurst` 原为 `-1`（无限制），而 BCP 跨节点携带 `fdlLoopCount` → 突发可沿途每节点各回环一次。实测多次回环 147,935 次；配对探针：τ=34.4 µs 时 12.64%→13.48%，τ=137.4 µs 时 23.30%→**19.61%**（差 3.69 pp）。已改为 `1` 并重跑 | 红线得以强制；定性结论存活 | 说明红线此前**靠人记、无自动巡检**（已补两个常驻诊断配置） |
| 热点流量标定 | 粗扫 40 run + 细扫 16 run。瓶颈**恒为 sat3→sat2**（该链路承载右侧 5 个源）。**19.5 µs → ρ=0.8074**，4 种子互异；12 µs 时 ρ=0.9455 但种子完全相同（完全饱和） | **第 8 周验收达成**：ρ≥0.8 且种子有差异 | 12 µs 不可作工作点；第三档负载尚未并入 ExpA |
| 新增工具 | `tools/plot_expA_tausweep.py`、`tools/analyze_hotspot.py`；`fdl_tests.ini` 增 `Test-FDL-Diag-LoopStrict/Loose` | 图 3/图 4 与热点表可复跑；红线可巡检 | — |
| D1 | **未发出**。Agent 判断：建议发（图 6 立论所系，且 MEMS 兜底会使主结果消失），但不阻塞第 8–11 周，只影响第 12 周 | — | 不能把"已拟"写成"已发出" |

周会材料：`research_reports/2026-09-week8/周会材料.md`。分析：`research_reports/2026-09-week8/分析.md`。

### 第 9 周完成记录（2026-09-20）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| VF 调度器选项 | `OBS_ChannelCalendar` + `enableVoidFilling`（默认关）；四条决策路径统一走 `channelAccepts/selectLambda/reserveChannel`；无空隙时与 Horizon **判定完全相同**（源码含推导）。debug/release 均 exit 0、无 warning | VF 是 Horizon 的严格推广；对照臂可跑 | 不是 FDL 的第三种策略；不改变 FDL 语义 |
| `useFDL=false` 回归 | 改动前备份 vs 改动后（同 debug）：Basic/TooShort/Off/SO 各 **1701/1701 相同**、Compatibility **1697/1697 相同**，差异 0、新增标量 0 | 计划验收项"回归仍全同"达成（逐标量，非容差） | 未重跑 2 s 载入批量（第 11 周） |
| VF 确定性用例 | `Test-VF-Off` 丢 1 收 2；`Test-VF-On` 丢 0 收 3 且 `voidFilledBursts=1`；`Test-VF-FDLOn` 同 On 且 `fdlUsageCount=0`；`Test-VF-FDLOnNoVF` 丢 1 收 2 | VF 分支可达；**结果改变来自调度器而非延迟线** | 不覆盖多波长（W=1 红线） |
| **VF 载入实测（负面结论）** | ρ≈0.59 双种子：`voidFilledBursts = 0`，与 Horizon 逐标量相同。原因：`OBS_BurstSender` 使所有 burst 偏移恒等于 `maxOffset` → 到达顺序 ≡ BCP 顺序 → 被拒 burst 必与已有窗口重叠（附证明） | 同步偏移的 W=1 链路**不存在可补空隙**；VF 主网格恒等于零 | 不等于"VF 无用"——异质偏移下全网已调度 +23.0%、`voidFilledBursts` 156–1294 |
| 重传基线 | `src/Retransmit/`（源 + 确认 sink + 消息），`like IUDPApp`，不改 `src/EdgeNode/`；`ExpR-Retransmit` 已接线 | 占位配置变为真实实验臂 | 未跑批量（第 11 周） |
| 重传确定性用例 | On：sat5 重传 1 / 放弃 0 / 完成时延 **52.02 ms**；Off（上限 0）：重传 0 / 放弃 1 / 未完成；两者差 **30.00 ms = 超时值** | 5.2 的时延对比成立：重传按 RTT 付、FDL 按 τ 付 | 不是负载下的 goodput 交叉点 |
| 重传载入冒烟 | 0.2 s：exit 0、无放弃；但 `requestsOutstanding` 3000–7500、`deliveryRatio` 0.14–0.39；实测 `maxChannelUtilization` 0.63/0.84 高于 FDL 臂 0.52/0.59 | ExpR 必须 2 s 跑且须单列在飞请求；5.2 交叉点须用热点矩阵取高负载点 | 该诊断数字**不得**当实验 R 结果 |
| 实现缺陷（已修） | 首版按 `gatesHorizon->getPortLambdas()` 定日历尺寸，而 `GatesHorizon` 的 `initialize()` 尚未执行 → 0 信道日历 → 越界读 → **每个 burst 被拒**（`calendarScanCount=0`）。改为解析 `lambdasPerOutPort`，并让日历对越界/0 信道 `opp_error` | 同类静默改结果的缺陷不再可能无声通过 | 不改变正常配置行为 |
| 文献闭合 | `laevens2003single`（6 页作者稿）、`vanhoudt2004channel`（7 页作者稿）**全文已读**，抽取文本入 `refs/extracts/`，`refs.bib` 备注更新 | 第 7 周遗留项闭合；**τ/T_burst≈1 属已知粒度结论**（最优依赖 burst 长度分布与负载；粒度偏离最优 10% 最坏可差数个数量级） | 不得声称首次给出 τ 选择规律 |
| 平台事实 | `.sca` 跨 debug/release 有 79–84 条末位差异 + 3 ps 结束时刻差；同一份新代码两种模式互比得到同样差异，debug 对旧基线 0 差异 | 差异属构建模式，与本周改动无关 | 第 8 周 ExpA 的模式未记录；第 10 周起须写进结果目录名 |

周会材料与分析：`research_reports/2026-09-week9/{周会材料.md,分析.md,文献精读-排队论单波长两篇.md}`。命令与结果：`walkthrough.md` 第十五节。

### 第 10 周完成记录（2026-09-22）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| 网格冻结（验收项） | `research_reports/2026-09-week10/网格冻结.md`：网格 A（同步主模型 + 异步并列，τ/T = 0.25/0.5/1/2/4，2 负载，repeat 5）与场景 B（热点 ρ≈0.81，FDL on/off × 3 τ，repeat 5）参数表定稿；结果目录名带构建模式 | 网格与重复次数已冻结；口径可复现 | 冻结的是**参数表**，不是最终数据 |
| 异步臂 τ 网格扩展 | 3 → 5 档（补 τ/T = 0.25 与 4）；pilot 40 + 参考线 8 run，release，exit 0 | 转折点落在网格内：异步 Horizon 在 τ/T=2 最优（15.28 → 16.46 回升） | 不是批量数据（repeat 2） |
| **τ 最优点随偏移模型移动** | 同步 τ/T=1（6.78% / 13.51%）；异步 Horizon τ/T=2（15.28% / 23.22%）；异步 +VF 在 ρ≈0.40 到 τ/T=4 仍在降 | "τ/T≈1 最优"**必须绑定偏移模型**；两条曲线分开报告 | 不覆盖 `offset/T_burst` 的其他取值 |
| **VF 与 FDL 的相对收益** | 异步偏移下 VF 收益 7.4–8.9 pp，FDL 收益 4.9–6.4 pp | 一个纯核心侧调度选择买到的不比那级延迟线少 | 两者不可相加成"联合收益" |
| **偏移模型本身是主变量** | 同一 `sendInterval`：同步 12.88% → 异步 20.76%（ρ≈0.40），实测负载同时下降 | 平台默认的偏移假设掩盖了比主角更大的变量（约 8 pp） | 不能用实测 ρ 轴直接比较两条曲线形状 |
| **热点 ρ≈0.81 上 FDL 失效** | 无 FDL 35.42%；FDL 34.80/35.03/35.36（τ/T=0.5/1/2）；`fdlUsageCount` 11–16 万、`fdlInFlightOccupancy` 最高 2.13 | 饱和瓶颈上"多等一个 τ"解决不了问题；与低负载 6.1 pp 构成 5.1→5.5 论证链 | 热点是**另一套流量矩阵**，不是网格 A 的高负载档 |
| pilot 曲线与工具 | `research_reports/figures/pilot_grid_freeze.png`；`tools/audit_runs.py`、`tools/plot_grid_freeze.py`（本地，不入库） | 曲线可复跑；结果目录可先审计再出图 | 工具不入库，复现需本地 |
| 数据卫生（两个静默错图问题，已修） | ① 作图脚本按迭代变量分组，把 `ExpA-NoFDL` 参考 run 混进 FDL 曲线（参考线算成 9.94%，真值 12.88%）；② `audit_runs.py` 漏读 `attr iterationvars2` 里的 `$repetition`，把每个 cell 报成缺全部重复 | 参考线口径恢复与第 8 周一致；重复次数可核对 | 说明"能画出图"不等于"口径正确"，批量前必须审计 |

周会材料与分析：`research_reports/2026-09-week10/{周会材料.md,网格冻结.md,分析-pilot与网格冻结建议.md}`。命令与结果：`walkthrough.md` 第十六节。

### 第 11 周完成记录（2026-09-22）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| 批量执行 | 225 run（异步 100 / 参考线 20 / 热点 30 / 同步 release 重跑 60 / 轻负载参考 5 / ExpR 5 / ExpR-120ms 5），release，repeat 5，全部 exit 0；8 个目录经 `audit_runs.py` 审计格齐、无重复、无跨批 | 网格按冻结表跑满，口径可复现 | 不含 ExpR 的高负载点（见下），不含 10 Gbps 归一化（第 12 周） |
| **第 8 周模式判定** | 按迭代格配对：`ExpA-TauSweep` 50/50、`ExpA-NoFDL` 10/10 全部有差异（平均 181 / 135 条每文件），全为末位浮点或皮秒级 ⇒ **debug 构建**；release 重跑复现 12.88 / 18.71 / 6.78 / 13.51 **逐位一致** | 第 8 周结论无需修正；图必须单模式；跨模式逐标量比较无意义 | 不能把第 8 周 `.sca` 与 release 结果逐值混用 |
| 图 3（τ 曲线） | 同步 τ/T=1 最优（6.78% / 13.51%）；异步 Horizon τ/T=2 最优（15.29% / 23.22%）；异步 +VF 最低最平（14.33–15.47%） | 两种偏移模型是两条曲线 | 不覆盖多波长（W=1 红线） |
| 图 4（为何失效） | FDL 使用率随 τ 升到 20–24%；在飞占用 τ/T=4 达 1.9–3.1（延迟线自己被塞满）；热点使用率 15–22%、占用 0.7–2.1 而剩余竞争丢包**恒在 35%** | "多等一个 τ"在饱和瓶颈无效；`fdlInFlightOccupancy > 1` 是延迟线变成排队的判据 | 未细分拒绝原因（wait>τ 与 FDL busy 未分别埋点，第 12 周可补） |
| 图 5（回环 vs 重传） | 同实测负载：无 FDL 2046 / FDL τ/T=1 2520 Mbit/s（ρ≈0.40）；重传仅 **1145 / 1126**（`retx/fresh ≈ 1.0`，线上有用率 ~61%）；异步 VF 相对 Horizon +32% / +40% | 回环 FDL 送达载荷 +21–23%；VF 的贡献大于延迟线本身；重传花一倍带宽只送到一半 | 两臂新发负载不同，只有实测负载轴可比；不得把"请求送达比例"与"包送达比例"相减 |
| **实验 R 建模假象（已修）** | `minSizeWithPadding = 500B` 把 64 B 确认填充成 4 µs 交换机占用（应 0.8 µs，×2.4）；未修时**不收敛**（活动消息 47k→729k，32 位进程 T=1.49 s 静默 0xC0000005） | 控制流量必须按真实尺寸计费；填充策略对控制面是假象 | 该假象不影响数据面（4.3 kB burst 从不触及 500 B） |
| **实验 R 真实边界** | 修后 79.8 µs 稳定（活动消息 31k 平坦），35.6 µs 仍无界增长（188k–980k，60/120 ms 皆然）⇒ 开环重传可用区间为新发 ρ≈0.2；60 ms 超时偏紧（266,536 次重传对 244,659 请求，平均完成时延 69 ms > 超时） | 无缓存重传可用区间比单级回环窄（0.2 对 0.8）；超时是这条臂定义的一部分 | 不能声称重传"高负载更差"——它在高负载下无法稳定运行，没有可比数据点 |
| 新工具 | `tools/{audit_runs,compare_dirs,check_sim_stability,plot_paper_figs}.py`（本地，不入库） | 批次可审计、模式可判定、不收敛可提前发现、图文同源 | 工具不入库，复现需本地 |

周会材料与分析：`research_reports/2026-09-week11/{周会材料.md,分析.md}`。图与逐格表：`research_reports/figures/`。命令与结果：`walkthrough.md` 第十七节。

### 第 12 周完成记录（2026-09-22）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| 图 6（验收项） | `research_reports/figures/fig6_feasibility.png`：面板 A = 第 11 周实测 τ 曲线 + 有用区（τ/T∈[0.5,2]，峰值 1：+6.1 / +5.2 pp）与无增益区（ρ≈0.59 下 τ/T=4 为 −0.9 pp）；面板 B = 器件代价与三条不等式边界 | **三区可画**（验收达成）；性能轴与器件轴同图 | 有用区 ≠ 可行区（成熟电光下二者不重叠）；器件的"有用区"仍受文献与假设约束 |
| 器件轴（来自第 8 周表，`fdl_design.py` 直接 import） | τ/T=1：电光 3 级 = **10.64 dB**（光纤仅 0.14 dB）、InP SOA **0.14 dB**；级数 2→5 随 τ/T 0.25→4 递增；MEMS/光束转向需 τ/T ≥ **2328 / 7276**（高 3 个数量级） | "成熟度 vs 链路预算"的取舍可量化；MEMS/光束转向被不等式一定量排除 | 质量体积预算仍是假设（图上 4/6/8 级敏感性）；步进 0.1 µs 档会把电光插损推到 21.14 dB（比质量体积更硬） |
| **D2 判定（验收项：关闭 D2）** | 新增 1 Gbps 孪生点 `FDL-RateCheckRef`；四项判据（2026-09-14 预登记）全部 PASS：丢包 0.10%（绝对差 0.0053 pp）、最忙信道利用率 0.21%、`fdlUsageCount` 0.06%、`carriedBursts` 0.00%（后两项按 T_burst 归一）。表：`research_reports/2026-09-week12/ratecheck-output.txt` | **扫描可按 τ/T_burst 归一化报告并外推到 10 Gbps 设计点** | 只覆盖 τ/T=1、sendInterval/T=1.24 一个点；ρ 不可跨 0.2 s 与 2 s 配置比较 |
| D2 的一个定量更正 | 两个计数量原始偏差 23.03% / 23.07%，原因不是归一化失败而是**测量窗口**：两运行时长差 10 倍（时长按速率缩放）而 warmup 是真实秒（排除每跳 5 ms 填路瞬态），窗口为 43,655 vs 56,752 T_burst，**比值 1.3000** 与 23% 吻合；按 T_burst 归一后差 0.06% / 0.00% | 判据在计数类量上只有按 T_burst 归一才有内容；工具同时打印原始与归一化两列，不隐藏 | 不得把 GO 外推到未测工作点 |
| 口径更正 | `FDL-RateCheckRef` 初始注释写成"两者都是 0.2 s"（实际参考点 2 s），已按真实窗口关系改写 | 配置注释与事实一致 | — |

周会材料与分析：`research_reports/2026-09-week12/{周会材料.md,分析.md,ratecheck-output.txt}`。图：`research_reports/figures/fig6_feasibility.png`。命令与结果：`walkthrough.md` 第十八节。

### 第 13 周完成记录（2026-09-22）

| 证据 | 实际结果 | 能支持什么 | 不能支持什么 |
|---|---|---|---|
| 2×2 联合（`ExpB-Joint-release`，60 run，release，repeat 5） | 四臂端到端丢包：Static off/on = 29.43/18.19%（79.8 µs）、42.10/28.79%（35.6 µs）、52.20/41.93%（21.3 µs）；Dynamic off/on = 19.56/5.60%、35.05/19.91%、47.19/36.31% | 边缘 × 核心的联合读数已齐（图 7 面板 A） | 只用 3 档负载；ρ 随 burst 长度与丢失浮动（0.18–0.66），不是同一个 ρ 上的一对一对照 |
| **交叉点判定（验收项：只回答是否移动）** | 同负载（ρ≈0.40–0.42）回环 2241.2 Mbit/s（Static）/ 2519.7（Dynamic）vs 重传 961.9 / 1125.7 ⇒ **2.33× / 2.24×**，交叉点**不移动** | 边缘调度模式只平移绝对损失水平，不改变「回环优于重传」的相对排序；第 12 周 5.4 节可定稿 | 重传臂无窗口控制，ρ 只能到 ≈0.42 ⇒ 只在该负载区间验证，不是全负载区间 |
| **边缘恒定丢弃（本周新发现 + 口径更正）** | Static 在 3 档负载、FDL 开/关下恒丢 **14.21–14.27%**（波动 ≤0.06 pp），Dynamic **0.00%**；发生在成束之前的 Dispatcher 队列阈值上 | **更正第 7 周 Task 1.3**：核心 burst loss 读得「Static 好 0.66–1.52 pp」，端到端反而 Dynamic 好 **5.01–12.59 pp** ⇒ 核心 burst loss 只能当核心内部诊断量 | 未做队列阈值扫描 ⇒ 14.2% 是本周配置下的常数，不能当与阈值无关的普适值 |
| 端到端 vs 核心的换算 | 六组「核心 Δ 4.54–6.16 pp vs 端到端 Δ 10.26–15.13 pp」（比值 1.88–2.86）；分解 `端到端 = 边缘丢弃 + (1−边缘丢弃) × 网络丢弃` 六组全部吻合 ≤0.05 pp | 端到端收益 = 核心读数 ×（每 burst 包数 ≈3）×（边缘存活比例：Static 0.858 / Dynamic 1） | 「Δ网络 ≈ Δ核心 × 每 burst 包数」在 ρ≈0.59 高估 3.6–9.0 pp ⇒ 该换算不是与负载无关的常数 |
| 四臂 burst 长度公平性（先报） | Static 恒 4295.0 B（`minSizeWithPadding` 下限）、Dynamic 4206.5–4210.3 B，最大差 2.1% | 四臂 burst 长度不同但量级不足以解释 5.0–12.6 pp 的端到端差距 | 不能把四臂的端到端差异归因于 burst 长度 |
| 工具口径修复 | `parse_run` 原按空白切分 `.sca`；真实布局是 `scalar <module> \t<name> \t<value>` 且含空格的名字被引号包裹（`"Dropped Packets"`）⇒ 图 7 的 `edgeDrop%` 整列曾为 `nan`。改为三字段正则后 14.2% 才显形 | 第 11 周全部图形逐值复核不变（12.88/6.78、23.22/15.29、35.4/34.8）⇒ 此前结论未受影响 | — |

周会材料与分析：`research_reports/2026-09-week13/{周会材料.md,分析.md}`。图：`research_reports/figures/fig7_edge_core_2x2.png`。命令与结果：`walkthrough.md` 第十九节。

---

## 七、当前需要确认的研究决策

原挂起的两个问题已于 2026-08-14 按第二节的口径关闭：全光交换按“数据面严格全光 + 控制面定量时序预算”表述，`offset=0` 改为“边缘 offset=0”。以下为新的待确认项：

以下三项已转化为正式决策请求表（见 `codex_phase2_tasks.md` 第十节），此处仅保留摘要与最新状态。

1. **仿真线速率归一化（D2）。** **已于 2026-09-22 关闭 = GO。** 做法：新增 1 Gbps 孪生点 `FDL-RateCheckRef`（τ/T、T_proc/T、guard/T、offset/T、sendInterval/T 五项比值与 10 Gbps 的 `FDL-RateCheck` 一一对应），按 **2026-09-14 预登记**的判据比对：主判据 `maxChannelUtilization` < 10%、`fdlUsageCount` < 15%、`carriedBursts` < 10%；辅助 `burstLossRate` < 10% 或绝对差 < 0.1 pp。结果四项 PASS（0.21% / 0.06% / 0.00% / 0.10% 与 0.0053 pp）。**一个定量更正**：两个计数量原始偏差 23.03% / 23.07%，原因不是归一化失败而是测量窗口——两运行时长差 10 倍（时长按速率缩放）而 warmup 必须留在真实秒（排除每跳 5 ms 的填路瞬态），窗口为 43,655 vs 56,752 T_burst，比值 **1.3000** 与 23% 完全吻合；按 T_burst 归一后差 0.06% / 0.00%。判据在计数类量上只有按 T_burst 归一才有内容，工具 `tools/compare_ratecheck.py` 同时打印原始与归一化两列。**范围限制**：仅覆盖 τ/T=1、sendInterval/T=1.24 一个点；ρ 不可跨 0.2 s 与 2 s 配置比较。表：`research_reports/2026-09-week12/ratecheck-output.txt`。
2. **FDL 延迟器预算（D1）。** **已于 2026-09-20 闭合，改为公开文献建表。** 用户决定不向他人询问（"问的人不一定准确"）；盘查后发现原稿 7 个参数里只有"质量/体积/功耗预算"真正在别人手上。器件参数取自 Mouammar 2026 TABLE II 及**原始厂商出处**（Polatis / GLsun / Agiltron 厂商页 + Feyisa 2022 *JLT*），表 1 可标「器件手册」。**MEMS 被定量排除**（τ ≥ 8 ms → τ/T_burst ≈ 2.3×10³）；电光开关 τ/T=1 时总插损 **10.64 dB**，InP SOA 仅 **0.14 dB**（但成熟度为实验阶段）。**更正旧默认**："每级插损 0.5 dB"过于乐观（Agiltron 实际 **3.5 dB/级**）。文档：`research_reports/2026-09-week8/器件参数与三条不等式.md`；原询问稿已作废。
3. **BCP 电子处理时延（D3）。** `fdl_params.ini` 中 1 µs 为假设值（assumed, to be confirmed against device datasheet），对实验 A/B 几乎无影响，对实验 C 敏感。实验 C（第 17 周起）前须做 0.5/1/5 µs 敏感性扫描。**状态：已关闭——暂用 1 µs。**
4. **VF 口径（D4，2026-09-20 裁决：A+B 并行）。** 第 9 周实测：在现有"各源同 `maxOffset`"的同步偏移下，VF **恒等于** Horizon（`voidFilledBursts = 0`，逐标量相同），因为到达顺序 ≡ BCP 处理顺序 ⇒ 被 horizon 拒绝的 burst 必与已有预约窗口重叠（证明见 `walkthrough.md` 第十五节与 `OBS_ChannelCalendar.cc` 头部）。
   - **(A) 加"偏移异质"维度**（纯 ini：每个源一个 `maxOffset`）。
   - **(B) 把 VF 降级为结构结论**：主网格只留 Horizon。
   - **用户裁决（2026-09-20）：A+B 并行。** 落地方式：主网格 `ExpA-TauSweep` / `ExpA-NoFDL` **保持第 8 周口径不变**（同步偏移 + Horizon），同步情形下"VF≡Horizon"写成结构性结论（附证明）；另建 `FDL-Scenario-Async`（每源一个偏移，700–970 µs）+ `ExpA-AsyncTauSweep` / `ExpA-AsyncNoFDL` 两组配置回答"何时 VF 才值"。**状态：已裁决，异步臂 pilot 进行中（0.2 s 冒烟已显示 VF 把丢包降 7.9–9.4 个百分点）。** 待第 10 周冻结：异步臂的最终 repeat、是否并入 ρ≈0.81 热点矩阵（需"流量形态"维度）。

---

## 八、每周研究汇报模板

每周只汇报四件事，保持与任务推进一一对应：

| 汇报项 | 内容形式 |
|---|---|
| 本周完成 | 代码变更、仿真配置或调研结论，最多三条 |
| 本周证据 | 一张图、一张表或一组真实仿真数据 |
| 本周判断 | 该证据支持了什么，尚不能支持什么 |
| 下周计划 | 一个可在两天研究时间内完成的明确任务 |

建议每周汇报最后固定保留一行：

> 当前结论：FDL 已解决/正在解决什么问题；距离严格全光交换还缺少哪一个闭环证据。

---

## 九、工作约束

- `src/EdgeNode/` 作为第一阶段受保护基线，默认不做无关修改；功能确有需要时允许受控扩展。
- EdgeNode 扩展必须保留原四种调度模式的语义和默认行为，保证旧配置可复现；新行为优先通过新参数、新模式、新接口或独立模块引入。
- 若需要改变已申请专利方案的核心调度逻辑，必须保留原模式并另建可独立对照的新实现，同时补充基线回归和端到端测试。
- `useFDL=false` 必须保持与 FDL 开发前的行为一致。
- 所有实验数据必须来自 OMNeT++ 实际仿真，禁止硬编码、伪造或人工修改结果。
- W=1 单波长约束保持不变。
- FDL 最多允许一次回环，不实现多次回环。
- τ 必须同时满足仿真有效性和卫星载荷可实现的延迟器参数约束（实际器件为光开关式可调延迟器，非固定长光纤）。
- 新增策略应由实验数据驱动，不预先堆叠复杂算法。
- 代码变更完成后应记录编译命令、测试配置、结果文件和遗留问题。
- 编译和仿真一律由用户执行；Agent 不运行 `make`、不启动 OMNeT++。
- release 链接（`-linet`）已由用户解决，不要再当作当前阻塞。

---

## 十、研究叙事与预期产出

### 研究叙事（唯一源）

`AGENTS.md`、Skill、`codex_phase2_tasks.md` 引用本节，不另写一套。章节设计见 `research_reports/2026-09-week6-novelty/论文骨架.md`。

> 面向卫星星间全光交换目标，现有 OBS 的独立 BCP 处理带来控制面的 OEO 瓶颈。公开文献里 OBS+FDL（Chen 2004、ChinaCom 2010 等）与卫星 W=1 OBS（Zhao 2022，且明确不用 FDL）均已存在；地面 L-OBS 用输入 FDL（试验台约 1.7 µs）吸收处理时延。第 6 周裁决为 **有条件 Go、主叙事收缩**：不能声称首次 OBS+FDL，也不能声称首次卫星 OBS 或首次 W=1 卫星 OBS。
>
> 第一篇论文不问「单级 FDL 有没有用」（排队论已给出性能上界），而问：**在 W=1 星间链路上，载荷可实现的单级回环 FDL 相对「无缓存 + 重传」是否值得加**；边缘调度是否移动这条边界。结论无论正负都成立。Zhao 2022 与 Roethig/Mouammar 2026 回避星上光缓存，是工程直觉而非量化对照——本文补的是这条对照，不是发明 OBS+FDL。
>
> 回环 FDL ≠ 输入 FDL ≠ 严格全光交换。第 5 周标定（ρ=0.4→35.9 µs，`useFDL=false`）不是回环性能。实验 C（输入 FDL / 边缘 offset→0）移出第一篇，进入第 17 周起的条件阶段或第二篇论文。
>
> Zhao 取证只认 `research_reports/2026-09-week6-novelty/近邻论文精读分析.md`：SPIE HTML 全文摘录，2026-09-11 抓取 / 2026-09-14 复核，非正式出版 PDF。

### 预期产出

| 产出 | 当前状态 |
|---|---|
| 边缘节点四模式调度实现 | 已完成 |
| FDL 模块与 CoreNode 集成 | 已完成并提交 |
| FDL 调度逻辑与功能测试 | 三分支已通过；当前树 ICMP 回归已通过；pre-FDL 提交对照有条件通过（仅 `my ID` 不同） |
| 第一篇论文骨架 | 已落地：`research_reports/2026-09-week6-novelty/论文骨架.md` |
| 载荷可行性图与回环 vs 重传交叉点 | **图 6 已形成（第 12 周）**：`research_reports/figures/fig6_feasibility.png`（有用区 τ/T∈[0.5,2]；电光 10.64 dB vs SOA 0.14 dB）；**图 7 已形成（第 13 周）**：`research_reports/figures/fig7_edge_core_2x2.png`（2×2 端到端、FDL 收益、两种边缘模式下的交叉点）；**图 5 已有初版**（第 11 周，回环 vs 重传按实测负载轴）；正负结论都可写。第 13 周答案：**交叉点不移动**（2.33× / 2.24×），边缘只平移绝对损失水平 |
| 边缘调度是否移动交叉点 | 实验 B；压缩为一节 |
| 光标签 / `offset=0` 可行性判断 | 实验 C，第二篇 |
| 论文与专利材料 | 第一篇目标投稿 ≤ 2027-03 |

---

## 附录：关键文件

| 文件 | 用途 |
|---|---|
| `src/CoreNode/OBS_CoreControlLogic.cc/h` | 核心调度与 FDL 分支 |
| `src/CoreNode/OBS_CoreControlLogic.ned` | FDL 统计信号与统计量声明 |
| `src/CoreNode/OBS_FiberDelayLine.cc/h/ned` | FDL 延迟模块 |
| `src/CoreNode/OBS_CoreOutputHorizon.cc/h` | 普通端口与 FDL 端口 horizon 管理 |
| `src/CoreNode/OBS_CoreNode.ned` | FDL 子模块、参数和 OXC 回环连接 |
| `Examples/RingFdlOBS/omnetpp.ini` | 当前 RingFdlOBS 场景总入口 |
| `Examples/RingFdlOBS/tests.ini` | 第一阶段功能测试（受保护，勿改） |
| `Examples/RingFdlOBS/experiments.ini` | 第一阶段性能实验配置（受保护，勿改） |
| `Examples/RingFdlOBS/fdl_params.ini` | FDL 场景基线：W=1、全互联随机流量、负载旋钮 |
| `Examples/RingFdlOBS/fdl_tests.ini` | Task 2.3 三分支确定性功能测试 |
| `Examples/RingFdlOBS/fdl_experiments.ini` | 负载标定、实验 A、实验 B、归一化校验 |
| `Examples/RingFdlOBS/tools/calibrate_load.py` | 由实测信道利用率反解发送间隔，并做第 5 周平台检查 |
| `Examples/RingFdlOBS/tools/fdl_design.py` | τ ↔ 光纤长度/延迟器参数 ↔ 损耗预算换算 |
| `research_reports/2026-08-week3-fdl-tests/` | 第 3 周三分支功能测试 |
| `research_reports/2026-08-week4-compatibility/` | 第 4 周 ICMP 兼容性对照 |
| `research_reports/2026-09-week5-calibrate/` | 第 5 周负载标定；`FDL机制与实验平台说明.md` 为对照源码的说明；`walkthrough.md` 为第 5 周命令与结果 |
| `research_reports/2026-09-week6-novelty/` | 第 6 周文献定位：正式三份（精读 / 对照 / 周会）+ 检索记录 + D1 询问稿 + 第一篇论文骨架；有条件 Go 已裁 |
| `research_reports/2026-09-week6-novelty/refs/` | 项目唯一文献库（`refs.bib` + `pdf/` + `extracts/`）；第 15–16 周相关工作仍用这里 |
| `research_reports/2026-08-week2-release/周会材料.md` | 第 2 周 release 链接与静态时序复核 |
| `research_reports/00-project-management/codex_phase2_tasks.md` | 原始交接文档和任务拆解 |
| `.agents/skills/obs-satellite-project/references/research_guide.md` | 详细研究指南与验收标准 |
