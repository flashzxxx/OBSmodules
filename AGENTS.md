# OBSmodules 项目指令

## 项目概述

- 项目名称：OBSmodules — 卫星星间全光交换仿真。
- 技术栈：OMNeT++ 4.x、C++、NED。
- 目标：实现基于 FDL（光纤延迟线）的卫星 OBS 全光交换，使数据在星间传输时保持光信号，消除核心节点 OEO。
- 项目结题时间：2027 年 9 月。
- 当前阶段：实验与结论更正已收尾（最近一次更正 2026-09-24，边缘队列口径）；论文写作按用户 2026-09-22 指示暂缓。
- **本仓库只放代码。** 研究报告、方案、架构图、文献与原始数据都不在本仓库。

## 仓库划分（2026-10-14 起）

研究工作内容从本仓库拆到同名同级的文档库，两边各自独立提交：

| 内容 | 位置 |
|---|---|
| 代码：`src/`、`Examples/`、`test/`、`utils/`、`Makefile` | **本仓库** `OBSmodules`（public） |
| 周报、分析、论文骨架、`figures/` | `../OBSmodules-docs/research_reports/` |
| 架构图/时序图（html+json）与 drawio | `../OBSmodules-docs/.doc/`、`../OBSmodules-docs/.docs/` |
| 原始仿真数据（S5–S8 的 `.sca`/`.csv`） | `../OBSmodules-docs/.仿真原始数据/` |
| **研究叙事与结论唯一源** | `../OBSmodules-docs/research_status.md`（第十节） |
| 文献台账：`refs.bib`、PDF、正文摘录 | `../OBSmodules-docs/research_reports/2026-09-week6-novelty/refs/` |
| 实验参数与阶段计划（原 skill 的 references） | `../OBSmodules-docs/research_guide.md` |
| 分析与绘图脚本（读本仓库 `results/`） | `../OBSmodules-docs/tools/` |
| 仿真运行脚本 `run-sim.ps1`/`.cmd`（本地，不入库） | 本仓库 `Examples/RingFdlOBS/tools/` |

- **研究结论、实验数字与各条「不得」口径不在本文件复述**，一律以 `../OBSmodules-docs/research_status.md` 第十节为准；需要研究上下文时先读文档库。
- 文档库是 **private** 仓库；跨库引用一律写成 `../OBSmodules-docs/...`。
- 写报告、分析、图、修订文献台账时，在文档库内提交，不要在代码仓库留下这些产物。

### 内容边界（2026-10-14 用户确认）

**本仓库只提交与代码相关的内容。**

| 允许 | 例子 |
|---|---|
| 源码、构建与 NED 定义 | `src/**`、`Examples/**`、`test/**`、`utils/**`、`Makefile`、`doxy.cfg`、`package.ned`、`.cproject`/`.project` |
| 能复现仿真的输入 | 实验配置 `*.ini`、路由 `*.irt`/`*.dat`、示例测试数据（如 `Examples/EdgeNodeTest/bursts.txt`） |
| 结果目录占位符 | `**/results/.gitkeep` |
| Agent 工作协议与代码任务的变更记录 | `AGENTS.md`、`.agents/**`、`walkthrough.md`、根 `README` |

**禁止**：研究报告与周报、方案、架构图/时序图、论文骨架与图表、文献（PDF / `refs.bib` / 正文摘录）、
原始仿真数据（`.sca`/`.csv`/`.vec`）、选刊名录、分析与绘图脚本，以及**任何一次运行的输出或报错捕获**。
这些一律进 `../OBSmodules-docs`（边界规则见其 `AGENTS.md`）。`.gitignore` 已有保险规则，
但提交前仍要用 `git status` 自查一遍。

## 项目结构

```text
OBSmodules/                    # 本仓库：只有代码
├── src/
│   ├── CoreNode/          # 核心节点模块（当前开发重点）
│   │   ├── OBS_CoreControlLogic.cc/h  # 核心调度逻辑（含 FDL）
│   │   ├── OBS_FiberDelayLine.cc/h    # FDL 模块
│   │   ├── OBS_CoreOutputHorizon.cc/h # 信道 horizon 管理
│   │   ├── OBS_CoreNode.ned           # 核心节点定义
│   │   └── OBS_OpticalCrossConnect.cc/h # OXC 光交叉连接
│   ├── EdgeNode/          # 边缘节点模块：第一阶段受保护基线，必要时可受控扩展
│   │   ├── OBS_PacketDispatcher.cc/h  # 四模式调度器
│   │   └── OBS_PacketBurstifier.cc/h  # 组帧器
│   ├── SatelliteNode/     # 卫星复合节点（Host+Edge+Core）
│   ├── Retransmit/        # host 侧重传对照基线（默认关闭）
│   ├── messages/          # OMNeT++ 消息定义（.msg 与生成的 _m.cc/h）
│   ├── misc/              # 辅助模块
│   └── tests/             # 单元测试
├── Examples/              # 示例场景
│   └── RingFdlOBS/        # 当前卫星网络、测试与实验配置（omnetpp/params/tests/experiments.ini）
├── .agents/               # Agent Skill 配置（留在本仓库，指向 ../OBSmodules-docs）
├── walkthrough.md         # 变更/验证记录（中文）
└── out/                   # 编译输出（gitignore）

../OBSmodules-docs/            # 同级文档库（private）：报告、图、文献、原始数据、分析脚本
```

## 关键命令

```bash
# 编译（OMNeT++ IDE 或命令行）
make MODE=release

# 运行仿真
cd Examples/RingFdlOBS
../../out/gcc-debug/obsmodules.exe -u Cmdenv -f omnetpp.ini -c <ConfigName> -n "../..;.;D:/inet/src"

# 批量运行：使用 Examples/RingFdlOBS/ 下的批量脚本

# 清理编译
make clean
```

编译和仿真验证由用户执行；Agent 不主动运行 `make` 或仿真。release 链接（`-linet`）问题已解决，不要再当作当前阻塞。

## 核心技术红线

- **边缘节点基线保护**：`src/EdgeNode/` 的第一阶段实现已申请专利，默认不做无关修改；功能确有需要时允许受控扩展，但必须保留原四种模式的语义和默认行为、保证旧实验可复现，并优先采用新参数、新模式、新接口或独立模块。若改变核心调度逻辑，保留原模式并新增可独立对照的实现。
- **向后兼容**：`useFDL=false` 时行为必须与 FDL 开发前完全一致。
- **数据真实**：所有实验数据必须来自 OMNeT++ 仿真，不得硬编码、伪造或人为修改。
- **单次回环**：FDL 最大回环次数为 1，不实现多次回环。
- **W=1 单波长**：每条星间链路在任一时刻仅使用一个波长通道。
- **FDL 物理受限**：卫星载荷空间有限，FDL 延迟和长度必须具有物理可行性。
- **算力受限**：不得依赖模糊逻辑、在线 ML 推理等复杂星载在线算法。
- **文献登记**：文献每读一篇立即登记到 `../OBSmodules-docs/research_reports/2026-09-week6-novelty/refs/refs.bib`（DOI + 访问日期 + 实际读到的版本）。

## FDL 回环架构

OXC 增加一对回环端口 `in[N]`/`out[N]`，通过延迟为 τ 的 FDL 连接：

- 场景 A（直通）：目标通道空闲，burst 直接转发。
- 场景 B（FDL 回环）：发生冲突、`waitTime <= τ` 且 FDL 空闲，burst 进入 FDL，延迟后重试。
- 场景 C（丢弃）：以上条件均不满足。

策略仅包含：

- No-FDL（`useFDL=false`）：冲突时直接丢弃，作为基线。
- Always-FDL（`useFDL=true`）：冲突且满足条件时执行一次回环。

对照基线（**不是**新的 FDL 策略，默认关闭，不改变 `useFDL=false` 路径）：

- Horizon + void filling 调度器选项（另建实现，参数选择，默认仍 Horizon）。
- host 侧无缓存重传（独立模块或应用层，不改 `src/EdgeNode/`）。

二者服务于第一篇论文的对照组，不算「预先堆叠算法」。

## 代码风格

- 遵循 OMNeT++ 4.x 及仓库现有编码风格，保持兼容旧版 API。
- 类名使用 `OBS_` 前缀。
- 使用 OMNeT++ 标准 signal/statistic 机制记录数据。
- 代码注释和 EV 日志使用英文。
- 新文件沿用现有文件的 license header。
- 修改前先阅读关联 `.cc`、`.h`、`.ned` 和消息定义，避免只改声明或单侧连接。

## Git 规范

- 代码任务使用 feature branch：`git checkout -b task/<编号>`。
- 不覆盖或清理用户已有的未提交改动。
- commit message 使用 `feat:`、`fix:`、`test:` 或 `chore:` 前缀。
- 用户完成编译验证后再提交或合并；未经要求不提交、不合并、不打标签。
- **不在本仓库提交研究报告/图/PDF/原始数据**：提交前用 `git status` 自查，一旦出现 `research_reports/`、`.doc/`、`.仿真原始数据/`、`*.sca` 之类，先移到 `../OBSmodules-docs/` 再提交。`.gitignore` 已加保险规则。
- 文档库的提交在文档库内进行，规则见 `../OBSmodules-docs/README.md`。

## 协作模式

- Agent：调研代码、提出方案、实现获批范围内的改动、进行静态检查并产出 `walkthrough.md`。
- 用户：审批方案、执行编译和仿真验证、作最终确认。
- 每个任务完成后必须更新或产出 `walkthrough.md`（在本仓库），说明变更、设计理由、验证方法及未执行事项；研究结论与周报写到文档库。
- 涉及实验设计或后续阶段时，按需读取 `../OBSmodules-docs/research_guide.md`。
- **设计前提不得由 Agent 单方面定性为"配置问题"**（2026-09-24 立，缘于第 14 周边缘队列数事件）：
  反事实实验（改一个参数看结果怎么变）能查清**机制**，但**"哪个取值属于设计前提、哪个属于可调配置"
  只能由设计者裁决**。Agent 可以把机制查清并如实报告数字，**不得**据此把某个设计取值判为「配置伪影」、
  也不得擅自"清除"它带来的代价；需要判断时先问用户。**更正本身也会出错**：某次更正若被后续证明过强，
  按同样规矩追加带日期的更正段，并在总结里记一条教训。
