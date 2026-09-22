# OBSmodules 项目指令

## 项目概述

- 项目名称：OBSmodules — 卫星星间全光交换仿真。
- 技术栈：OMNeT++ 4.x、C++、NED。
- 目标：实现基于 FDL（光纤延迟线）的卫星 OBS 全光交换，使数据在星间传输时保持光信号，消除核心节点 OEO。
- 项目结题时间：2027 年 9 月。
- 当前重点：**第 12 周已完成（2026-09-22）：图 6 载荷可行性三区图 + D2 关闭（GO）**（`research_reports/2026-09-week12/分析.md`；图 `research_reports/figures/fig6_feasibility.png`；归一化表 `ratecheck-output.txt`）。结论：① **三区已量化**——有用区 τ/T ∈ [0.5, 2]（峰值在 1：+6.1 / +5.2 pp），< 0.5 无增益，> 2 收益衰减且 ρ≈0.59 下 τ/T=4 **为 −0.9 pp（比不加更差）**；**MEMS/光束转向被不等式一定量排除**（需 τ/T ≥ 2328 / 7276）。② **代价决定结论**——性能最优档在成熟商用**电光**开关下要付 **10.64 dB**（3 级 × 3.5 dB/级），实验阶段 **InP SOA 只要 0.14 dB**，光纤仅 0.14 dB ⇒ **"有用"与"付得起"在成熟器件上不重叠**，问题是"成熟度 vs 链路预算"；质量体积预算仍为假设（4/6/8 级敏感性）。**D2 GO**：1 Gbps 孪生点 vs 10 Gbps，丢包 0.10%、利用率 0.21%、按 T_burst 归一的事件率 ≤0.06%；两个计数量原始偏差 23% 已由窗口比 1.3000（warmup 是真实秒、时长按速率缩放）解释。研究叙事以 `research_status.md` 第十节为唯一源，不在本文件复述。**下一周为第 13 周**：`ExpB-Joint` 批量（2×2：边缘 Static/Dynamic × 核心 FDL 关/开，3 档负载）+ 分析联合是否移动交叉点（图 7 + burst 长度公平性表；四臂 burst 长度分布不同，须先报 `carriedBytes/carriedBursts` 均值）。**不得**把图 6 的"有用区"当"可行区"（二者在成熟电光下不重叠，这正是结论）；**不得**把假设的 10 dB 链路余量/6 级上限当实测值；D2 的 GO 只覆盖 τ/T=1、sendInterval/T=1.24 一个点，不能外推到未测工作点。第 11 周：225 run 批量 + 图 3–5（同步 τ 最优 τ/T=1、异步 τ/T=2；回环送达载荷 +21–23%，异步下 VF 贡献大于延迟线本身；热点 ρ≈0.81 上 FDL 失效；第 8 周判定为 debug 构建）。第 10 周：ExpA 网格冻结。第 9 周：VF 调度器选项与 host 侧重传基线（均默认关，不改 `src/EdgeNode/`），`useFDL=false` 回归**逐标量全同**。第一篇论文骨架：`research_reports/2026-09-week6-novelty/论文骨架.md`。**D1 已于 2026-09-20 闭合**（器件参数取自公开文献与厂商资料：`research_reports/2026-09-week8/器件参数与三条不等式.md`）。第 7 周遗留的两篇单波长排队论文已于第 9 周取得**全文**——**τ/T_burst≈1 属已知粒度结论，不得当新发现**。不得把第 5 周标定（ρ=0.4→35.9 µs，`useFDL=false`）写成回环性能；**第 7 周重标定为 35.6 µs，35.9 µs 仅属第 5 周记录**。Task 2.1–2.3 三分支与 ICMP 回归已通过；pre-FDL 对照有条件通过（仅 `my ID` 不同）；release 链接（`-linet`）已由用户解决。

## 项目结构

```text
OBSmodules/
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
│   ├── messages/          # OMNeT++ 消息定义（.msg）
│   ├── misc/              # 辅助模块
│   └── tests/             # 单元测试
├── Examples/              # 示例场景
│   └── RingFdlOBS/        # 当前卫星网络、测试与实验配置（omnetpp/params/tests/experiments.ini）
├── .agents/               # Agent Skill 配置
└── out/                   # 编译输出（gitignore）
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
- **文献登记**：文献每读一篇立即登记到 `research_reports/2026-09-week6-novelty/refs/refs.bib`（DOI + 访问日期 + 实际读到的版本）。

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

## 协作模式

- Agent：调研代码、提出方案、实现获批范围内的改动、进行静态检查并产出 `walkthrough.md`。
- 用户：审批方案、执行编译和仿真验证、作最终确认。
- 每个任务完成后必须更新或产出 `walkthrough.md`，说明变更、设计理由、验证方法及未执行事项。
- 涉及实验设计或后续阶段时，按需读取 `.agents/skills/obs-satellite-project/references/research_guide.md`。
