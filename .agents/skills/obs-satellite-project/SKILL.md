---
name: obs-satellite-project
description: OBSmodules 卫星星间全光交换仿真项目的专用上下文、技术约束与工作流程。在 OBSmodules 工作区内进行任何开发、代码调研、故障诊断、功能验证、实验设计、数据分析、论文或文档工作时使用；当用户提到 OBS、光突发交换、FDL、光纤延迟线、OXC、BCP、offset、光标签、全光交换、OEO、卫星、星间链路、W=1、burst 调度、回环、horizon、CoreNode、EdgeNode、SatelliteNode、OMNeT++、C++、NED、仿真或实验 A/B/C 时必须触发。研究类材料在同级 private 仓库 `../OBSmodules-docs`。
---

# OBS 卫星全光交换项目

## 仓库划分（2026-10-14 起）

本仓库 `OBSmodules` **只放代码**；研究侧材料全部在同级 private 文档库 `../OBSmodules-docs/`：

| 内容 | 位置 |
|---|---|
| 周报、逐周分析、论文骨架、`figures/` | `../OBSmodules-docs/research_reports/` |
| 架构图/时序图（html+json）、drawio | `../OBSmodules-docs/.doc/`、`../OBSmodules-docs/.docs/` |
| 原始仿真数据（S5–S8 `.sca`/`.csv`） | `../OBSmodules-docs/.仿真原始数据/` |
| **研究叙事与结论唯一源** | `../OBSmodules-docs/research_status.md`（第十节） |
| 文献台账 `refs.bib`、PDF、正文摘录 | `../OBSmodules-docs/research_reports/2026-09-week6-novelty/refs/` |
| 实验参数与阶段计划 | `../OBSmodules-docs/research_guide.md` |
| 分析与绘图脚本（读本仓库 `results/`） | `../OBSmodules-docs/tools/` |

- 研究结论、周次进度与各条「不得」口径**以 `research_status.md` 第十节为准**，本技能不复述。
- 需要研究上下文时先读文档库；写报告/图/文献台账在文档库内提交。
- 涉及实验设计或后续阶段时读 `../OBSmodules-docs/research_guide.md`。
- 文档库另有一份面向文档工作区的配套 skill：
  `../OBSmodules-docs/.agents/skills/obs-satellite-docs/SKILL.md`（把文档库作为工作目录打开时生效；
  在代码工作区干活时不需要它，两者不重名、可同时存在）。

## 项目目标与阶段

实现卫星星间全光交换，使数据在星间传输时始终保持光信号，不进行 OEO 转换。现有 OBS 的 BCP 在核心节点仍需 OEO；远期以光标签支持 `offset=0`，FDL 则承担无提前通知场景下的光域暂存。

采用三层架构：

| 层级 | 内容 | 当前状态 |
|---|---|---|
| 第一层：边缘节点 | P1-P4 优先级、LRU 抢占；Dynamic/NoPreemption/RoundRobin/Static 四种模式 | 已完成并申请专利；作为受保护基线，必要时可受控扩展 |
| 第二层：核心节点 FDL | No-FDL / Always-FDL、单次回环 | 代码已合入并验证；标定、实验 A/B/S/R 与结论更正均已完成（细节见 `research_status.md`） |
| 第三层：光标签 | 光域控制信息识别，目标 `offset=0` | 远期，不进第一篇 |

## FDL 回环模型

OXC 增加一对回环端口 `in[N]`/`out[N]`，通过延迟为 τ 的 FDL 连接：

- 直通：目标通道空闲，直接预约并转发 burst。
- FDL 回环：目标通道冲突、等待时间不超过 τ 且 FDL 空闲，执行一次回环并在延迟后转发。
- 丢弃：直通和回环条件均不满足。

使用两种策略：`useFDL=false` 为冲突即丢弃的基线；`useFDL=true` 为条件满足时执行一次回环的 Always-FDL。

对照基线（不是新的 FDL 策略，默认关闭）：Horizon+void filling 调度器选项；host 侧无缓存重传。二者用于第一篇论文对照，不改 `src/EdgeNode/`，且 `useFDL=false` 行为不变。

## 已完成接口

### OBS_FiberDelayLine

- NED 参数 `delayTime` 表示 FDL 延迟，门为 `in` 和 `out`。
- `initialize()` 读取 `delayTime` 并初始化使用计数。
- `handleMessage()` 对每个光 burst 调用 `sendDelayed(msg, delayTime, "out")`。
- `finish()` 记录标量 `FDL usage count`。

### OBS_CoreControlLogic 的 FDL 分支

- 从父级 CoreNode 读取 `useFDL` 和 `fdlDelayTime`（τ）。
- 将 horizon 的 `port == numPorts`、`lambda == 0` 作为 FDL 独立预约状态。
- 直通失败时检查目标通道在延迟后是否可用以及 FDL 是否空闲。
- 回环时安排两段 OXC 预约：`原输入门 -> FDL 输出门`，再由 `FDL 输入门 -> 目标输出门`。
- 更新 FDL 与目标通道 horizon，并将 BCP 的到达偏移增加 τ。
- 记录 `fdlUsageCount`、`fdlUtilization`、`burstLossContention`、`burstLossTotal` signal/statistic。
- 保持 `useFDL=false` 的原始调度路径不变。

### 对照基线（默认关闭）

- Horizon + void filling 调度器选项：`src/CoreNode/` 内的独立实现，由参数选择，默认仍为 Horizon。
- host 侧无缓存重传：`src/Retransmit/` 独立模块，不改 `src/EdgeNode/`。

## 核心研究问题

1. 在 W=1 星间链路上，载荷可实现的单级回环 FDL 相对「无缓存 + 重传」是否值得加？对应实验 A、ExpR 与载荷可行性图。
2. 边缘调度是否移动这条边界？对应实验 B：2×2，压缩为论文一节。
3. FDL 能否支持 offset 趋近 0？对应实验 C：输入 FDL 时序。 **第一篇不做**；第 17 周起条件阶段或第二篇。

文献每读一篇立即登记到 `../OBSmodules-docs/research_reports/2026-09-week6-novelty/refs/refs.bib`。

## 关键文件

| 文件 | 用途 |
|---|---|
| `src/CoreNode/OBS_CoreControlLogic.cc/h` | 核心调度与 FDL 分支、参数及统计状态 |
| `src/CoreNode/OBS_CoreControlLogic.ned` | FDL 和丢包 signal/statistic 声明 |
| `src/CoreNode/OBS_FiberDelayLine.cc/h/ned` | FDL 延迟模块、接口和 NED 定义 |
| `src/CoreNode/OBS_CoreOutputHorizon.cc/h` | 输出通道及 FDL 独立 horizon 管理 |
| `src/CoreNode/OBS_CoreNode.ned` | `useFDL`、`fdlDelayTime`、FDL 子模块和 OXC 回环连接 |
| `src/CoreNode/OBS_OpticalCrossConnect.cc/h` | OXC 门连接与切换 |
| `src/CoreNode/OBS_ChannelCalendar.cc/h` | 信道预约日历（horizon 的无序容器实现） |
| `src/EdgeNode/OBS_PacketDispatcher.cc/h` | 四模式边缘调度；保护原模式语义，必要时可新增兼容扩展 |
| `src/EdgeNode/OBS_PacketBurstifier.cc/h` | 组帧器；光标签等功能可能需要受控扩展 |
| `src/Retransmit/OBS_Retransmit{Source,Sink}.cc/h/ned` | host 侧无缓存重传对照基线（默认关闭） |
| `src/SatelliteNode/OBS_SatelliteNode.ned` | Host、Edge、Core 复合卫星节点 |
| `src/messages/` | OMNeT++ 消息定义（`.msg` 与生成的 `_m.cc/h`） |
| `Examples/RingFdlOBS/omnetpp.ini` | 当前场景总入口 |
| `Examples/RingFdlOBS/params.ini` `fdl_params.ini` | 参数基线（FDL 场景覆盖其中三项） |
| `Examples/RingFdlOBS/tests.ini` `fdl_tests.ini` | 功能测试配置 |
| `Examples/RingFdlOBS/experiments.ini` `fdl_experiments.ini` | 性能实验配置 |

## 工作流程

1. 先读取任务涉及的实现文件、头文件、NED 定义、消息定义和实验配置。
2. 明确当前行为、预期行为及 `useFDL=false` 基线，再进行修改。
3. 只修改任务需要的文件；涉及 EdgeNode 时先完成必要性与影响分析，并遵守边缘节点基线保护规则。
4. 编译验证：代码完成后由用户执行 `make MODE=release`；Agent 不运行 `make`。
5. 功能验证：由用户运行相关测试配置；Agent 根据用户给出的结果更新文档。
6. 完成后产出 `walkthrough.md`（代码仓库内），记录变更、理由、验证结果和遗留事项；研究结论与周报写到 `../OBSmodules-docs/`。

## 协作模式

Agent 负责技术方案设计、代码实现、静态核对和文档。用户负责编译、仿真验证和最终审查。

- Agent 自主决定实现方案，但必须遵守红线约束。
- Agent 不运行 `make`，不启动 OMNeT++。release 链接（`-linet`）已由用户解决，不要再当作阻塞。
- 代码注释和 EV 日志使用英文。
- walkthrough.md 使用中文，留在代码仓库。

## Git 规范

1. 代码任务使用 `git checkout -b task/<编号>` 创建 feature branch。
2. commit message 使用 `feat:`、`fix:`、`test:` 或 `chore:` 前缀。
3. 用户编译和测试通过后再提交。
4. 不主动合并到 main，由用户决定合并时机。
5. **研究报告/图/PDF/原始数据不进代码仓库**；这类内容在 `../OBSmodules-docs/` 提交。

## 红线约束

- **边缘节点基线保护**：`src/EdgeNode/` 的第一阶段实现已申请专利，默认不做与任务无关的修改；若新功能确实需要，可进行受控扩展，但必须保留原四种模式的语义和默认行为、维持旧实验可复现，并优先通过新参数、新模式、新接口或独立模块引入新行为。若需要改变专利方案的核心逻辑，必须保留原模式，另建可独立对照的新实现。
- **向后兼容**：`useFDL=false` 时转发/丢弃判定须与 FDL 开发前一致（统计 signal/statistic 允许有增量）。
- **数据真实**：所有实验数据必须来自 OMNeT++ 仿真，不得硬编码、伪造或人为修改。
- **单次回环**：FDL 最大回环次数为 1，不实现多次回环。
- **W=1 单波长**：每条星间链路任一时刻仅使用一个波长通道。
- **FDL 物理受限**：τ 必须考虑卫星载荷空间及可实现的光纤长度。
- **算力受限**：不得依赖模糊逻辑、在线 ML 推理等复杂星载在线算法。
- **发表限制**：论文仅投实验室认可的期刊或会议，名录以项目附件为准（选刊名录见 `../OBSmodules-docs/journal_list_full.txt`、`conference_list_full.txt`）。
- **文献登记**：文献每读一篇立即登记到 `../OBSmodules-docs/research_reports/2026-09-week6-novelty/refs/refs.bib`（DOI + 访问日期 + 实际读到的版本）。
- **设计前提不得由 Agent 单方面定性为"配置问题"**：反事实实验能查清机制，但「哪个取值属于设计前提」由设计者裁决；需要判断时先问用户（缘于 2026-09-24 边缘队列数事件，详情见 `research_status.md`）。

## 详细参考

- 研究叙事与当前状态：`../OBSmodules-docs/research_status.md`（第十节）。
- 实验参数与阶段计划：`../OBSmodules-docs/research_guide.md`（含 Phase 1-4 的历史 Gemini 提示词，**提示词区块保持原样**）。
- 周次材料与论文骨架：`../OBSmodules-docs/research_reports/`。
