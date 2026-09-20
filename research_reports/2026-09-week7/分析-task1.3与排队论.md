# 第 7 周 Task 1.3：第一阶段四模式干净重跑 + FDL 排队论检索

> 日期：2026-09-18
> 分支：`task/code-review-2026-09`
> 对应计划：`codex_phase2_tasks.md` 第 7 周（研究日 A：Task 1.3；研究日 B：排队论检索）
> 同周另一份记录：`分析.md`（M1 复跑、guardTime 根因、重标定、ExpA-Pilot）
> 输出：`Examples/RingFdlOBS/results/task1.3-2026-09-18/`（68 个 `.sca` + 1 张图）

---

## 一、Task 1.3 要交付什么

计划原文（`codex_phase2_tasks.md:135`）：

| 研究日 A | 研究日 B | 周会证据 | 验收门槛 |
|---|---|---|---|
| Task 1.3：第一阶段实验干净重跑 | 用户跑 `Test-FDL-Basic/TooShort/Off` 过 M1；补 FDL 排队论检索 | 四模式对照图；M1 标量；排队论缺口表 | **实验 B 基线可信**；M1 过；排队论读后登记 `refs.bib` |

`research_status.md` 补一句："实验 B 另依赖 Task 1.3"，以及"实验 B 前置依赖：Task 1.3 完成，Static/Dynamic 基线数据干净"。

---

## 二、重跑方法与数据保护

第一阶段配置（`experiments.ini` 的 `FlowScaling` / `LoadIntensity`）属**受保护基线**，未做任何修改；
用临时 `_task13.ini` 派生两个配置转出输出目录（跑完即删），避免覆盖 `results/` 下 2026-07-31 的旧 `.sca`。

| 配置 | 迭代 | run 数 | 结果 |
|---|---|---|---|
| `Task13-FlowScaling` | `${mode=0,1,2,3}` × `${flows=2,4,6,8,10,12}` | 24 | exit 0 |
| `Task13-LoadIntensity` | `${mode=0,1,2,3}` × `${interval=0.1s…0.001s}` | 20 | exit 0 |
| `Task13-LoadedModes` | `${mode=0,1,2,3}` × 两档标定负载 × repeat 3 | 24 | exit 0 |

分析/绘图脚本：`Examples/RingFdlOBS/tools/plot_phase1_modes.py`（新增，可复跑）。
指标在**全网**聚合：丢包率 = `ΣburstLossTotal / ΣburstsReceived`；
平均突发长 = `ΣcarriedBytes / ΣcarriedBursts`；另取 dispatcher 的 `P1–P4 Hits` 专利优先级计数。

图：`results/task1.3-2026-09-18/phase1_four_modes.png`

---

## 三、发现 1：第一阶段配置根本不在竞争区（loss ≡ 0）

**44 个第一阶段 run，`ΣburstLossTotal` 全部为 0。** 不是"接近 0"，是恒等于 0。

原因与第 5 周记录的缺陷同源：`FlowScaling` 是 10–12 条 500 B 流、间隔 2 ms ≈ **24 Mbps**，
对 1 Gbps 信道；`LoadIntensity` 最快档也不过 ~0.4%。链路负载约 0.24%，
核心节点几乎无竞争。

**这直接改变了 Task 1.3 的结论**：第一阶段配置的能量化的是**组帧/调度行为差异**，
**不能**提供带丢包的 Static/Dynamic 基线。而"实验 B 基线可信"恰恰需要后者。

### 该配置仍能量化的东西：模式在聚合行为上差异巨大

同一 44 个 run 里，四种模式的平均突发长与突发数量差异显著（图 B）：

| 现象 | 数据 |
|---|---|
| `RoundRobin` 突发明显更小 | 537–1590 B，且随流数剧烈波动；其余三种 2653 B 起 |
| `Dynamic` 在高流数下塌缩 | flows≥10 时平均突发降到 568–646 B，而 `NoPreemption`/`Static` 升到 2836–3291 B |
| `RoundRobin` 突发数最多 | 最重档送出 15000 个突发，`NoPreemption`/`Static` 只有 2000 |

这本身就是对**实验 B 公平性警告**（`fdl_experiments.ini` 里那条）的定量证据：
`dispatchMode` 会通过突发长度直接进入 FDL 准入判据 `waitTime`。见第六节。

---

## 四、发现 2：补一个有负载的四模式基线

因为第一阶段负载不足以支撑验收，另跑 `Task13-LoadedModes`：同样四种模式、`useFDL=false`，
但用**标定过的 FDL-Scenario 负载**（全互联、warmup 50 ms、`seed-set=${runnumber}`），
两档 ρ、每档 3 个种子。

| 模式 | ρ≈0.59 丢包率 | ρ≈0.40 丢包率 | 平均突发长 |
|---|---|---|---|
| **Static (3)** | **17.18%** | **11.77%** | 4295 B |
| NoPreemption (1) | 18.49% | 12.79% | 4295 B |
| Dynamic (0) | 18.71% | 12.87% | 4210 B |
| RoundRobin (2) | 18.97% | 13.28% | **1613 B** |

三点值得记：

1. **丢包首次出现模式间可分辨差异**：Static 比 Dynamic 低约 1.1 个百分点（两档一致）。
   这就是 Task 1.3 要的那条干净基线。
2. **Static 优于 Dynamic**，与"动态调度更自适应"的直觉相反。可能的机理是
   Dynamic 把包分散到多个队列，突发更小更多 → 竞争更频繁。留待实验 B 解释，先如实记录。
3. `NoPreemption` 与 `Static` 平均突发长**完全相同（4295 B）**但丢包不同（12.79 vs 11.77），
   说明这两者之间的差异不是突发长度假象；而 `RoundRobin` 的 1613 B 与其余三种
   相差 2.7 倍，**不可直接比较**。

图 C/D：`phase1_four_modes.png`。D 面板给出重负载下 P1–P4 命中构成：
Dynamic/NoPreemption/Static 都是约 65% P1 + 25% P2 + 10% P3；
RoundRobin 约 90% P3（fresh idle），与该模式的语义一致。

---

## 五、对实验 B 的意义

- 实验 B（`ExpB-Joint`，2×2 边缘×FDL）现在有了**带丢包的四模式参考面**，
  而不再只有第一阶段那组零丢包数据。
- 公平性警告被量化：`RoundRobin` 的突发长与其余三种差 2.7 倍，
  若把 `RoundRobin` 放进 2×2 而不报告 `carriedBytes/carriedBursts`，结论不可辩护。
  当前 `ExpB-Joint` 用的是 Static(3) 与 Dynamic(0)，两者突发长 4295 vs 4210 B（差 2%），
  **公平性可控**。
- Static 在无 FDL 时优于 Dynamic 1.1 pp —— 这给实验 B 的"边缘调度是否移动交叉点"
  提供了一个明确的先验方向。

---

## 六、FDL 排队论检索（研究日 B）

### 6.1 已登记 `refs.bib` 共 10 条

统一读取级别：**题录级** —— 通过 Crossref API 与出版方书目逐条核对 DOI、卷期页、作者全名与年份；
其中 2008/2009 两篇还读了其参考文献表，并据此发现下面第 6.2 条的线索。
**未取得任何一篇全文 PDF，也未读摘要正文**（无机构订阅；Springer/Elsevier 直链跳转登录页）。

| 条目 | 出处 | 为什么相关 |
|---|---|---|
| `callegati2000optical` | IEEE Comm. Lett. 4(9):292-294, 2000 | 变长分组下 FDL 缓存的"粒度"问题 |
| `chlamtac1996cord` | IEEE JSAC 14(5):1014-1029, 1996 | CORD：延迟线竞争缓解奠基 |
| `hunter1998slob` | IEEE/OSA JLT 16(10):1725-1736, 1998 | 大容量光缓存交换结构 |
| `nizam1998waspnet` | IEE Colloquium, London, 1998 | 第 6 周点名的那篇 |
| **`laevens2003single`** | IEEE INFOCOM 2003 | **单波长光缓存解析，不假设波长变换** |
| **`vanhoudt2004channel`** | IEEE GLOBECOM 2004 | **单波长 FDL 缓存的信道利用率与丢包率解析** |
| `rogiest2007degenerate` | Queueing Systems 56(3-4):203-212, 2007 | 退化缓存解析 |
| `rogiest2009unified` | Performance Evaluation 66(7):343-355, 2009 | 同步/异步 FDL 闭式统一模型 |
| `lambert2008hessenberg` | LNCS 5055:101-113, 2008 | **FDL 长度（即 τ）优化** |
| `lambert2006correlated` | Stochastic Models 22(2):233-251, 2006 | 相关到达/服务队列用于光缓存 |

### 6.2 三处必须更正的归属 / 口径

1. **"退化缓存"的作者不是 Lambert 等**，是 Rogiest / Laevens / Walraevens / Bruneel。
2. **WASPNET 第一作者是 Nizam**，Hunter 排第 4；第 6 周口径写作"Hunter：WASPNET"。
3. **最重要：第 6 周"排队论都假定多波长或全波长变换"被证伪。**
   `laevens2003single`（INFOCOM 2003）与 `vanhoudt2004channel`（GLOBECOM 2004）
   就是**单波长**设定；后者还直接给出**信道利用率与丢包率**，
   与本文 5.1 的指标口径直接重叠。

### 6.3 对新颖性的影响（需要正视）

第 6 周的 Go 结论建立在"组合缺口仍在"之上。排队论这一线补齐后，缺口**进一步收窄**：

- 剩下的差别只剩三条：**（a）载荷器件代价**（插损、切换时间、档位、质量体积）
  从未进入解析模型；**（b）星间场景**；**（c）与「无缓存 + 重传」的量化对照**。
- 单波长、单级 FDL 的**性能上界已被解析给出**，因此 5.1 的 τ 扫描**只能作为证据，
  不能作为贡献**——这一点第 6 周已经写了，但现在依据更强。
- `lambert2008hessenberg` 已做 **FDL 长度优化**，所以"我们把 τ 窗口接到载荷约束上"
  这条也必须在相关工作里逐字区分。

已同步更新 `新颖性对照与继续条件.md` 的对应对照行与 `论文骨架.md` 的 2.1 节。

### 6.4 未完成、必须补的

**写 2.1 正文之前必须取得 `laevens2003single` 与 `vanhoudt2004channel` 的全文或至少摘要。**
当前是题录级，不足以支撑"A 与 B 的差别只剩载荷代价"这一论断的逐条论证。
`refs.bib` 的 `note` 字段已如实标注为"题录级"，没有写成已读。

---

## 七、未执行

- 未修改 `experiments.ini` / `tests.ini` / `params.ini`（受保护基线）
- 四模式**未**跑 `repeat` 多倍（单种子或 3 种子），置信区间未做；这是**诊断性基线**，
  正式实验 B 仍按 `ExpB-Joint` 跑
- 排队论全文未取得（见 6.4）
- `RoundRobin` 突发长异常（537 B vs 2653 B）的机理未查证
- release 构建未复跑；未提交 git
