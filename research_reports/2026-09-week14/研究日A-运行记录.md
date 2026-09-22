# 第 14 周研究日 A：敏感性实验的实施与运行（2026-09-22）

> 范围：本文件只记录**研究日 A**（配置、冒烟、批量、审计、初步读数）。正式分析、图 8 与周会材料
> 属研究日 B，不在本文件内。论文写作按用户 2026-09-22 指示暂缓。

## 一、三个臂与各自的判决标准

第 13 周收口时把两件事留给了第 14 周：一是「Static 恒丢 14.2%」这个骨架数字到底从哪来，二是
5.3 的「有用区 τ/T_burst ∈ [0.5, 2]」是不是只在一种粒度下成立。研究日 A 把两者连同热点的 τ 网
格补齐一起做成三个臂。

| 臂 | 配置 | 迭代 | run 数 | 判决标准 |
| --- | --- | --- | --- | --- |
| S1 边缘队列混叠 | `ExpS-EdgeQueues` | `numPacketBurstifiers` ∈ {8, 10} × `dispatchMode` ∈ {Static=3, Dynamic=0} × 负载 {79.8, 35.6, 21.3 µs} × repeat 5 | 60 | 若 10 队列下 Static 丢弃归零 ⇒ 第 13 周的 14.2% 是**配置伪影**（10 目的地映射到 8 队列），必须限定表述；若仍为 ~14% ⇒ 它是 Static 的性质，5.4 保留原话 |
| S2 突发长度 | `ExpS-BurstLen` | `numPackets` ∈ {1, 3, 6} × τ/T 名义比 ∈ {0.5, 0.85, 1, 2} × 负载 {35.6, 21.3 µs} × repeat 5 | 120 | 逐臂用**实测** T_burst 归一后，最优点仍应落在 τ/T ≈ 1；若随粒度漂移，5.3 的有用区必须按粒度重新表述 |
| S2 参考线 | `ExpS-BurstLen-NoFDL` | `numPackets` ∈ {1, 3, 6} × 负载 {35.6, 21.3 µs} × repeat 5 | 30 | 每个粒度各自的参考线（粒度改变了竞争形态，不能共用一条 No-FDL 线） |
| S3 热点 τ 网格两端 | `ExpA-HotspotGrid` | τ ∈ {8.6 (τ/T=0.25), 137.4 µs (τ/T=4)} × `useFDL` ∈ {false, true} × repeat 5 | 20 | 第 11 周热点只有 τ/T = 0.5/1/2 三个点，平台可能是局部平段；补两端后若仍平 ⇒「饱和瓶颈上 FDL 与 τ 无关」成立 |

合计 **230 run**，全部 release 构建、`repeat = 5`、`seed-set = ${runnumber}`。

### 关键设计选择

1. **队列数是 ini 参数，不动 `src/EdgeNode/`**。`OBS_PacketDispatcher` 的丢弃判据是
   `(targetLabel - 1) % numQueues`（mode 3 分支），而 `numQueues` 在 `OBS_BurstAssembler.ned` 里被
   绑成 `numQueues = numPacketBurstifiers`，dispatcher 的输出门数、burstifier 数组与 sender 输入
   数组都由同一个参数定尺寸。因此改 ini 即可，NED 自动伸缩。**`params.ini` 的 8 队列默认值保持不
   动**（那是基线），只有本实验要求 10。
2. **τ 是推导量而不是枚举量**。S2 用
   `fdlDelayTime = ${ratio} * (1429 * ${numPackets} + 8) * 0.008us`，
   这样归一化比值才是迭代变量，绝对延迟不会在各臂之间漂移。系数 1429 是**实测**倒推的：
   3 × 1428 B（1400 B 载荷 + 28 B UDP/IP 头）+ 8 B burst 头 + 3 × 1 B 每包头 = 4295 B，与第 13 周
   从 `carriedBytes/carriedBursts` 读回的 4295.0 B 完全一致。
3. **S2 保持包速率不变**（沿用主网格的 `sendInterval`），只改变打包方式；若改成保持 burst 速率，
   报出字节数会随 n 一起变，ρ 就不对齐了。每臂实测 ρ 与实测 T_burst 一并报告。
4. **S3 独立配置而不继承 `ExpA-HotspotPoint`**：OMNeT++ 拒绝在派生配置里重定义继承来的迭代变量，
   而第 11 周的配置必须继续产出它当时产出的目录内容，所以 10 行 `destAddresses` 逐字重写——那 10
   行**就是**热点，从别的配置派生反而会让本臂随 Scenario B 的后续改动静默改变。

## 二、冒烟与机制探针（已完成）

按约定先跑 0.2 s 冒烟（`fdl_tests.ini` 的 `Test-Smoke-*`，全部 exit 0），并在冒烟里发现两件需要
先弄清的事，各追加了一个定向探针。

### 2.1 队列数确实生效，且 Static 的丢弃在 10 队列下归零

模块清单就是证据：8 队列的 cell 里 `packetBurstifier` 最大下标为 7，10 队列的 cell 里出现 `[9]`。
若 `numPacketBurstifiers` 被忽略，NED 仍会建 8 个，本臂就会静默地重复基线。

0.2 s 冒烟、三档负载合计：

| `numPacketBurstifiers` | `dispatchMode` | 丢弃率 |
| --- | --- | --- |
| 8 | Static (3) | **14.128%**（175,266 包） |
| 8 | Dynamic (0) | 0.000% |
| 10 | Static (3) | **0.000%** |
| 10 | Dynamic (0) | 0.000% |

这正是混叠读数预测的结果：10 个目的地映射到 8 个队列时，label 1 与 9 共用队列 0、label 2 与 10
共用队列 1，某包的 label 与队列中正在组帧的 burst 不一致时该包被丢弃；队列数 ≥ 目的地数后映射是
一一的，丢弃归零。**第 13 周把它写成「接入侧队列阈值」，这个机制描述是错的**（见第六节）。

### 2.2 长 burst 被 Dynamic 的抢占截断（并已排除「偏置预算」这一竞争解释）

冒烟发现 `numPackets = 6` 的实测 burst 只有 **7321–7361 B（≈5.1 包）**，而不是名义的 8582 B
（6 包）；`numPackets = 1` 与 3 则分别精确为 1437.0 B 与 4210.5–4215.9 B。因为
`OBS_PacketBurstifier` 的 flush 条件里明确包含 `numBurstPackets == numPackets`，这个缺口必须另有
原因。两个候选各自做了一个探针（均 0.2 s、单种子、`results/_diag`，只作机制判断，不作结果）：

| 探针 | 改了什么 | `numPackets = 6` 实测 burst | 结论 |
| --- | --- | --- | --- |
| `Test-Smoke-BurstLenOffset` | `minOffset/maxOffset` 由 0.5 ms/1 ms 放宽到 2 ms/4 ms | 7321.4–7361.1 B（与基准每个 cell 相差 ≤2 B） | **偏置预算不是原因** |
| `Test-Smoke-BurstLenNoPreempt` | `dispatchMode = 1`（NoPreemption） | **8582.0 B（恰好 6.00 包）**，且 n=1/3 也精确为 1437.0/4295.0 B | **抢占截断就是原因** |

即：Dynamic（mode 0）在目的地数超过空闲队列数时会抢占最久未用的忙队列，`forceFlush()` 把正在组帧
的 burst 提前发走，于是长 burst 永远装不满；mode 1 改为丢包而不抢占，mode 3 把冲突包丢掉而让当前
burst 继续组帧（第 13 周实测 Static 的 burst 恰好恒为 4295.0 B，正是同一机制的另一面），两者都能
达到配置的粒度。

**对主网格的影响**：主网格是 Dynamic + `numPackets = 3`，实测 4211 B 对名义 4295 B，只差 2.0%，
不足以移动 τ/T ≈ 1 的最优点，第 11 周的结论不受影响。

**因此 S2 的 τ 轴必须按实测 T_burst 归一**：n=6 的实测 T_burst 是 58.6–58.9 µs，而名义值是
68.66 µs，名义比 1.0 对应的实测比是 1.17。S2 的比值列表因此加了 **0.85**，让 n=6 也拿到一个落在
实测比 ≈1.0 的点（0.85 × 68.656 = 58.4 µs ÷ 58.7 µs = 0.994）。

### 2.3 τ 的可追溯性

`fdlDelayTime` 没有被记成任何标量，所以 τ 由 `.sca` 的 `attr iterationvars` 反推：
`τ = ratio × (1429 × numPackets + 8) × 0.008 µs`。两条独立核对：

1. 实测 burst 长度逐臂印证了 `${numPackets}` 确实被代入（1437 / 4211 / 7324 三档与各自的名义式
   一致）；
2. S2 的 (n=3, ratio=1) 就是 τ = 34.4 µs、与第 11 周 `ExpA-TauSweep-release` 的最优 cell 同参
   数，批量完成后用 `fdlUsageCount` 与该 cell 对表（见第五节）。

## 三、批量清单与运行状态

四个臂并行运行（release，`repeat = 5`，`seed-set = ${runnumber}`），全部 exit 0。因为
`params.ini` 把 Cmdenv 文本日志固定在 `results/simulation_log.txt` 一个路径上，四臂并行时必须逐
作业覆盖 `--cmdenv-output-file`，否则会互相抢写同一份日志。

| 目录 | cell × repeat | run 数 | 窗口 | 耗时 | 作业 |
| --- | --- | --- | --- | --- | --- |
| `results/ExpS-EdgeQueues-release` | 12 × 5 | 60 | 18:41 → 18:52 | 10.6 min | pwsh-28 |
| `results/ExpS-BurstLen-release` | 24 × 5 | 120 | 18:42 → 19:10 | 28.8 min | pwsh-29 |
| `results/ExpS-BurstLen-NoFDL-release` | 6 × 5 | 30 | 18:42 → 18:50 | 8.2 min | pwsh-30 |
| `results/ExpA-HotspotGrid-release` | 4 × 5 | 20 | 18:42 → 18:47 | 5.5 min | pwsh-31 |
| 合计 | 46 × 5 | **230** | — | 28.8 min（并行） | — |

单 run 实测约 8 s（由第 13 周 `ExpB-Joint-release` 的 60 个文件时间戳标定：477.2 s / 59），因此
本批 230 run 若串行约 31 min，四臂并行后由最长的 S2 决定，约 29 min。

## 四、审计

`python tools/audit_runs.py results/<目录> --repeat 5`，四个目录全部格齐、无问题：

| 目录 | cell 数 | 满 5 重复的 cell | MISSING / DUPLICATE / STALE |
| --- | --- | --- | --- |
| `ExpS-EdgeQueues-release` | 12 | 12 | 0 / 0 / 0 |
| `ExpS-BurstLen-release` | 24 | 24 | 0 / 0 / 0 |
| `ExpS-BurstLen-NoFDL-release` | 6 | 6 | 0 / 0 / 0 |
| `ExpA-HotspotGrid-release` | 4 | 4 | 0 / 0 / 0 |

每个 cell 的 `span = 0.0 d`，说明没有跨批混入的文件。

## 五、初步读数

聚合脚本：`python tools/analyze_w14_sensitivity.py [--only S1|S2|S3]`（只读 `.sca`）。

### 5.1 S1：8 队列上的 14.2% 在 10 队列上归零

| `numPacketBurstifiers` | `dispatchMode` | 79.8 µs | 35.6 µs | 21.3 µs | 三档合计丢弃率 | 端到端丢包（三档） |
| --- | --- | --- | --- | --- | --- | --- |
| 8 | Static | 14.218% | 14.208% | 14.213% | **14.212%**（1,751,293 包） | 29.43% / 42.10% / 52.20% |
| 10 | Static | 0.000% | 0.000% | 0.000% | **0.000%**（1,750,553 包） | 19.62% / 34.97% / 47.17% |
| 8 | Dynamic | 0.000% | 0.000% | 0.000% | 0.000% | 19.55% / 35.07% / 47.22% |
| 10 | Dynamic | 0.000% | 0.000% | 0.000% | 0.000% | 19.44% / 35.00% / 47.17% |

三条读数：

1. **8 队列的 Static 臂逐值复现第 13 周**（14.212% 对 14.21–14.27%；端到端 29.43/42.10/52.20
   对第 13 周的 29.43/42.10/52.20），说明本臂与第 13 周同配置、可直接比较。
2. **队列数升到 10 后 Static 的丢弃精确归零**，端到端丢包同时落到 Dynamic 的水平（19.62/34.97/
   47.17 对 19.44/35.00/47.17）。核心 burst loss 也从 17.17/11.81/6.00 回到 18.66/12.89/6.68，
   与 Dynamic 的 18.66/12.90/6.62 一致。
3. **因此「Static 比 Dynamic 端到端差 5.0–12.6 pp」由 10 个目的地挤 8 个队列造成**：队列够用时
   Static 与 Dynamic 在核心与端到端两侧都无法区分（±0.2 pp 以内）。

### 5.2 S2：τ/T_burst ≈ 1 的最优点在三种粒度下都成立

τ 用实测 T_burst 归一（`T_real = 实测 burst 字节 × 0.008 µs`）；`FDL pp` 是该臂相对**同粒度**
无 FDL 参考线的端到端丢包改善。

| n | 实测 T_burst | 负载 | ratio 0.5 | 0.85 | 1.0 | 2.0 | 最优点 | 无 FDL 参考 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 11.50 µs | 35.6 µs | 10.08% | 8.60% | **8.16%** | 9.28% | τ/T = 1.00 | 13.37% |
| 1 | 11.50 µs | 21.3 µs | 15.95% | 15.05% | **14.93%** | 17.05% | τ/T = 1.00 | 19.13% |
| 3 | 33.66 µs | 35.6 µs | 9.09% | 7.31% | **6.80%** | 8.35% | τ/T = 1.02 | 12.93% |
| 3 | 33.66 µs | 21.3 µs | 14.79% | 13.66% | **13.50%** | 16.41% | τ/T = 1.02 | 18.71% |
| 6 | 58.8 µs | 35.6 µs | 8.64% | 7.04% | 6.68% | 8.69% | τ/T = 0.99–1.17（平） | 12.84% |
| 6 | 58.8 µs | 21.3 µs | 14.32% | 13.58% | 13.53% | 16.87% | τ/T = 0.99–1.17（平） | 18.57% |

（表中为全网核心 burst 丢失率；实测归一比：n=1 → 0.50/0.85/1.00/2.00；n=3 → 0.51/0.87/1.02/2.04；
n=6 → 0.58/0.99/1.17/2.34。）

- **最优点都在 τ/T_burst ≈ 1**：n=1 精确落在 1.00，n=3 落在 1.02，n=6 在 0.99 与 1.17 之间持平
  （两点差 0.05 pp @21.3 µs）。**5.3 的「有用区 τ/T ∈ [0.5, 2]，峰值在 1」不被粒度推翻**——这是本
  周该臂的验收标准。
- **粒度本身的影响很小**：同负载下三条无 FDL 参考线为 12.84–13.37%（ρ≈0.40）与 18.57–19.13%
  （ρ≈0.59），即 n 从 1 到 6 只动 0.5 pp，方向是越粗越好（与已知粒度结论一致，且远小于 τ 的效
  应）。
- **FDL 的端到端收益随粒度增大而增大**：ρ≈0.40 时 n=1/3/6 的最优收益为 12.6 / 15.1 / 15.1 pp，
  ρ≈0.59 时为 8.7 / 10.9 / 10.5 pp。粗 burst 一次丢失带走更多包，正是第 13 周
  `端到端 = 边缘丢弃 + (1−边缘丢弃) × 网络丢弃` 里「一个 burst ≈ n 个包」那一项的体现。
- **τ 表达式的独立核对通过**：n=3 且 ratio=1 就是 τ = 34.36 µs、35.6 µs 负载、Dynamic——与第 11
  周 `ExpA-TauSweep-release` 的 τ/T=1 最优 cell 同参数，实测核心丢包 **6.80% / 13.50%** 对第 11 周
  的 **6.78% / 13.51%**，差 0.02 pp。τ 若被表达式算错，该点不可能落回第 11 周的最优点；
  见 2.3 节。

### 5.3 S3：热点上补两端后，「与 τ 无关」成立且大 τ 侧归零

| τ | τ/T_burst | 无 FDL | FDL 开 | 收益 | `fdlUsageCount` |
| --- | --- | --- | --- | --- | --- |
| 8.6 µs | 0.25 | 35.42% | 34.87% | **+0.55 pp** | 63,802 |
| 137.4 µs | 4 | 35.41% | 35.41% | **0.00 pp** | 160,891 |

与第 11 周已有的三点（τ/T = 0.5 → +0.61 pp、1 → +0.37 pp、2 → +0.04 pp，由
`results/ExpA-HotspotPoint-release` 逐 cell 重算：35.420−34.811 / 35.410−35.038 / 35.423−35.380）
连起来，热点上的收益在 τ/T=0.5 达到峰值后单调衰减（0.25 → +0.55 pp 已在峰值之下），到 τ/T=4 完全
归零，而 `fdlUsageCount` 翻到 16 万次——**延迟线被用得更多、买到的东西更少**。第 11 周「饱和瓶颈
上多等一个 τ 解决不了问题」的读数在网格两端都成立，且不再是局部平段的可能。

## 六、对既有结论的影响

### 6.1 必须更正：第 13 周对 14.2% 的机制描述是错的

第 13 周（`2026-09-week13/分析.md` 第三节、`walkthrough.md` 第十九节、`research_status.md` 与
`AGENTS.md`）把 Static 的恒定丢弃写成「接入侧 Dispatcher 队列阈值」。读
`src/EdgeNode/OBS_PacketDispatcher.cc` 的 mode 3 分支可以看到判据是
`fixedQueue = (targetLabel - 1) % numQueues`：队列数少于目的地数时多个 label 共用一个队列，某包的
label 与队列中正在组帧的 burst 不一致时该包被丢弃。

S1 的 10 队列臂把丢弃精确打到 0，机制由此从**代码判据 + 对照实验**两侧确认。**这个量是配置伪影，
不是 Static 调度的性质**：它是「10 目的地只用 8 个物理队列」的直接结果，`params.ini` 里那行注释
（`# Strictly 8 physical queues for all 10 destinations`）当时就是把它当设计选择写下的。

### 6.2 连带更正：「端到端与核心读数方向相反」也是伪影造成的

第 13 周的核心新发现是一句推断：「核心 burst loss 读得 Static 好 0.66–1.52 pp，端到端反而 Dynamic
好 5.01–12.59 pp，所以核心计数器不可作评价依据」（引述第 13 周的表述，非本文件结论）。S1 表明这
5–12.6 pp **全部来自混叠丢弃**：队列够用时两个模式在两侧都一致（端到端 47.17/34.97/19.62 对
47.17/35.00/19.44，核心 18.66/12.89/6.68 对 18.66/12.90/6.62）。

因此第 13 周那句「更正第 7 周 Task 1.3」的**结论过强**，正确表述是：

- **仍然成立且值得写进论文**：核心 burst loss 与端到端口径不可互换（一个 burst 约 3 个包，核心
  1 pp 在端到端约值 3 pp，再乘边缘存活比例）；评价「谁更好」必须用端到端送达。
- **必须收回**：把「Dynamic 端到端优于 Static 5.0–12.6 pp」当作 Static/Dynamic 的模式差异。它
  是 8 队列对 10 目的地的配置差异，在 10 队列下消失。
- 第 7 周 Task 1.3 的原读数（核心上 Static 好 0.66–1.52 pp）本身没有被推翻——它同样是混叠把
  Static 的 burst 变少变大造成的，两个口径都在描述同一个伪影，只是符号相反。

### 6.3 不受影响的部分

- **交叉点不移动（第 13 周主结论）**：回环对重传在两种边缘模式下都是 2.2–2.3 倍。Static 两臂都
  带着同一个混叠（回环臂曾丢 14.2%、重传臂丢 15.75%），量级差 2 倍以上，不因边缘修正而翻转；但
  Static 臂的**绝对**数字（2241.2 / 961.9 Mbit/s）带伪影，正文若引用需注明或改用 10 队列重跑。
- **FDL 端到端收益 11.2–15.1 pp = 核心读数的 1.9–2.9 倍**：分解式本身把边缘丢弃显式剥离，逻辑不
  依赖丢弃的成因；但该组数字同样是在 8 队列下测的。
- **主网格 τ/T≈1 最优点**：主网格是 Dynamic + n=3，实测 burst 4211 B 对名义 4295 B 只差 2.0%，且
  无混叠（Dynamic 丢弃为 0），不受影响。

### 6.4 研究日 B 待办

1. 若正文要引用 Static 臂的绝对数字，用 10 队列重跑 ExpB / ExpR 的 Static 臂（本臂已证明 10 队列
   可建、可跑）。
2. 图 8 的三个面板：S1 丢弃率 vs 队列数、S2 三条归一曲线叠图、S3 热点 τ 网格。
3. 把 2.2 节的抢占截断写成一条可交付结论（NoPreemption/Static 能达到配置粒度，Dynamic 不能），
   目前证据是 0.2 s 单种子探针；若 5.5 要用，需补一个 2 s × repeat 5 的小臂。

## 附：复现

```powershell
cd Examples\RingFdlOBS
# 冒烟（全部 0.2s，进 results/_diag）
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config Test-Smoke-EdgeQueues  -Mode release
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config Test-Smoke-BurstLen    -Mode release
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config Test-Smoke-HotspotGrid -Mode release
# 机制探针
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config Test-Smoke-BurstLenOffset   -Mode release
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config Test-Smoke-BurstLenNoPreempt -Mode release
# 批量（四臂并行；--cmdenv-output-file 必须逐作业指定，否则四个作业抢写同一份日志）
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config ExpS-EdgeQueues      -Mode release -ExtraArgs "--cmdenv-output-file=results/_logs/ExpS-EdgeQueues.log"
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config ExpS-BurstLen        -Mode release -ExtraArgs "--cmdenv-output-file=results/_logs/ExpS-BurstLen.log"
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config ExpS-BurstLen-NoFDL  -Mode release -ExtraArgs "--cmdenv-output-file=results/_logs/ExpS-BurstLen-NoFDL.log"
powershell -ExecutionPolicy Bypass -File .\tools\run-sim.ps1 -Config ExpA-HotspotGrid      -Mode release -ExtraArgs "--cmdenv-output-file=results/_logs/ExpA-HotspotGrid.log"
```

`run-sim.ps1` 在本机默认执行策略下直接用 `&` 调用会被拒（`UnauthorizedAccess`），必须经
`powershell -ExecutionPolicy Bypass` 或以进程级 Bypass 运行。
