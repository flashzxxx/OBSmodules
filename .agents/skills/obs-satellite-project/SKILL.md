---
name: obs-satellite-project
description: OBSmodules 卫星星间全光交换仿真项目的专用上下文、技术约束与工作流程。在 OBSmodules 工作区内进行任何开发、代码调研、故障诊断、功能验证、实验设计、数据分析、论文或文档工作时使用；当用户提到 OBS、光突发交换、FDL、光纤延迟线、OXC、BCP、offset、光标签、全光交换、OEO、卫星、星间链路、W=1、burst 调度、回环、horizon、CoreNode、EdgeNode、SatelliteNode、OMNeT++、C++、NED、仿真或实验 A/B/C 时必须触发。
---

# OBS 卫星全光交换项目

## 项目目标与阶段

实现卫星星间全光交换，使数据在星间传输时始终保持光信号，不进行 OEO 转换。现有 OBS 的 BCP 在核心节点仍需 OEO；远期以光标签支持 `offset=0`，FDL 则承担无提前通知场景下的光域暂存。

采用三层架构：

| 层级 | 内容 | 当前状态 |
|---|---|---|
| 第一层：边缘节点 | P1-P4 优先级、LRU 抢占；Dynamic/NoPreemption/RoundRobin/Static 四种模式 | 已完成并申请专利；作为受保护基线，必要时可受控扩展 |
| 第二层：核心节点 FDL | No-FDL / Always-FDL、单次回环 | 代码已合入 + 静态复核 / 三分支确定性测过；第 5 周负载标定已完成（ρ=0.4→35.9 µs，useFDL=false）。第 6 周有条件 Go（须对标 Zhao 2022 与 L-OBS）。Horizon 扩容与 Scenario B +τ 时序尚未 useFDL=true 回环确定性用例验证锁死 |
| 第三层：光标签 | 光域控制信息识别，目标 `offset=0` | 远期 |

任务进度：

- Task 1.1 Git 清理：已完成。
- Task 1.2 CoreNode 代码调研：已完成。
- Task 1.3 实验重跑准备：待完成。
- Task 2.1 FDL 模块创建：已完成。
- Task 2.2 FDL 调度逻辑：代码已合入 + 静态复核通过（用户确认 release 已链接）；三分支确定性测过。Horizon 对 FDL 口扩容与 Scenario B 二次 OXC / burstArrivalDelta+τ 时序尚未被 useFDL=true 回环确定性用例验证锁死。
- Task 2.3 FDL 功能验证：三分支确定性测试已通过；当前树 ICMP 回归已通过；pre-FDL 提交对照有条件通过（仅 `my ID` 不同）。
- 第 5 周实验平台标定：已完成。`FDL-Calibrate`（useFDL=false）14 run 通过平台核对；ρ=0.4 对应发送间隔已回写为 35.9 µs。该结果只证明负载标定，不证明回环 FDL 调度正确。
- 第 6 周文献定位：已完成（2026-09-11）。**有条件 Go、主叙事收缩**。必须对标 Zhao 2022（卫星 OBS+W=1、不用 FDL）与 L-OBS 输入 FDL（试验台 ~1.7 µs）；不得声称首次 OBS+FDL。2026-09-14：Zhao 取证统一为 SPIE HTML 全文摘录（非正式出版 PDF）；检索留痕见 `检索记录.md`；D1 询问稿默认冻结 2026-09-28。材料：`research_reports/2026-09-week6-novelty/`（正式三份：精读 / 对照 / 周会）。第一篇论文骨架：同目录 `论文骨架.md`。研究叙事以 `research_status.md` 第十节为唯一源。
- 阶段三 FDL 实验验证：待开始。正式实验 A 前仍需 useFDL=true 回环确定性用例验证锁死 Scenario B 时序；实验 B 另依赖 Task 1.3。第 7 周并行：Task 1.3 + M1 + FDL 排队论补检索。实验 C 不进第一篇，第 17 周起。

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

## 核心研究问题

1. 在 W=1 星间链路上，载荷可实现的单级回环 FDL 相对「无缓存 + 重传」是否值得加？对应实验 A、ExpR 与载荷可行性图。
2. 边缘调度是否移动这条边界？对应实验 B：2×2，压缩为论文一节。
3. FDL 能否支持 offset 趋近 0？对应实验 C：输入 FDL 时序。 **第一篇不做**；第 17 周起条件阶段或第二篇。

第 7 周文献侧须补 FDL 排队论一线（Callegati、Rogiest/Laevens/Bruneel、Hunter WASPNET、Lambert 退化缓存）。读后立即登记 `research_reports/2026-09-week6-novelty/refs/refs.bib`。Naji 2023 按第 6 周结论降级，不进必写，且触算力红线。

## 关键文件

| 文件 | 用途 |
|---|---|
| `src/CoreNode/OBS_CoreControlLogic.cc/h` | 核心调度与 FDL 分支、参数及统计状态 |
| `src/CoreNode/OBS_CoreControlLogic.ned` | FDL 和丢包 signal/statistic 声明 |
| `src/CoreNode/OBS_FiberDelayLine.cc/h/ned` | FDL 延迟模块、接口和 NED 定义 |
| `src/CoreNode/OBS_CoreOutputHorizon.cc/h` | 输出通道及 FDL 独立 horizon 管理 |
| `src/CoreNode/OBS_CoreNode.ned` | `useFDL`、`fdlDelayTime`、FDL 子模块和 OXC 回环连接 |
| `src/CoreNode/OBS_OpticalCrossConnect.cc/h` | OXC 门连接与切换 |
| `src/EdgeNode/OBS_PacketDispatcher.cc/h` | 四模式边缘调度；保护原模式语义，必要时可新增兼容扩展 |
| `src/EdgeNode/OBS_PacketBurstifier.cc/h` | 组帧器；光标签等功能可能需要受控扩展 |
| `src/SatelliteNode/OBS_SatelliteNode.ned` | Host、Edge、Core 复合卫星节点 |
| `src/messages/` | OMNeT++ 消息定义 |
| `Examples/RingFdlOBS/omnetpp.ini` | 当前场景总入口 |
| `Examples/RingFdlOBS/tests.ini` | 现有功能测试配置 |
| `Examples/RingFdlOBS/experiments.ini` | 现有性能实验配置 |

## 工作流程

1. 先读取任务涉及的实现文件、头文件、NED 定义、消息定义和实验配置。
2. 明确当前行为、预期行为及 `useFDL=false` 基线，再进行修改。
3. 只修改任务需要的文件；涉及 EdgeNode 时先完成必要性与影响分析，并遵守边缘节点基线保护规则。
4. 编译验证：代码完成后由用户执行 `make MODE=release`；Agent 不运行 `make`。
5. 功能验证：由用户运行相关测试配置；Agent 根据用户给出的结果更新文档。
6. 完成后产出 `walkthrough.md`，记录变更、理由、验证结果和遗留事项。

按需读取 `references/research_guide.md` 获取详细实验参数和阶段计划。

## 协作模式

Agent 负责技术方案设计、代码实现、静态核对和文档。用户负责编译、仿真验证和最终审查。

- Agent 自主决定实现方案，但必须遵守红线约束。
- Agent 不运行 `make`，不启动 OMNeT++。release 链接（`-linet`）已由用户解决，不要再当作阻塞。
- 代码注释和 EV 日志使用英文。
- walkthrough.md 使用中文。

## Git 规范

1. 代码任务使用 `git checkout -b task/<编号>` 创建 feature branch。
2. commit message 使用 `feat:`、`fix:`、`test:` 或 `chore:` 前缀。
3. 用户编译和测试通过后再提交。
4. 不主动合并到 main，由用户决定合并时机。

## 红线约束

- **边缘节点基线保护**：`src/EdgeNode/` 的第一阶段实现已申请专利，默认不做与任务无关的修改；若新功能确实需要，可进行受控扩展，但必须保留原四种模式的语义和默认行为、维持旧实验可复现，并优先通过新参数、新模式、新接口或独立模块引入新行为。若需要改变专利方案的核心逻辑，必须保留原模式，另建可独立对照的新实现。
- **向后兼容**：`useFDL=false` 时转发/丢弃判定须与 FDL 开发前一致（统计 signal/statistic 允许有增量）。
- **数据真实**：所有实验数据必须来自 OMNeT++ 仿真，不得硬编码、伪造或人为修改。
- **单次回环**：FDL 最大回环次数为 1，不实现多次回环。
- **W=1 单波长**：每条星间链路任一时刻仅使用一个波长通道。
- **FDL 物理受限**：τ 必须考虑卫星载荷空间及可实现的光纤长度。
- **算力受限**：不得依赖模糊逻辑、在线 ML 推理等复杂星载在线算法。
- **发表限制**：论文仅投实验室认可的期刊或会议，名录以项目附件为准。
- **文献登记**：文献每读一篇立即登记到 `research_reports/2026-09-week6-novelty/refs/refs.bib`（DOI + 访问日期 + 实际读到的版本）。

## 详细参考

按需读取 `references/research_guide.md`。该文件含 Phase 1-4 的历史 Gemini 提示词，**提示词区块保持原样**；2026-09-14 起的周次与叙事以文首重定向说明、`research_status.md` 第十节和 `codex_phase2_tasks.md` 为准。
