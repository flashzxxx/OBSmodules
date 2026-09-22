# OBSmodules 项目指令

## 项目概述

- 项目名称：OBSmodules — 卫星星间全光交换仿真。
- 技术栈：OMNeT++ 4.x、C++、NED。
- 目标：实现基于 FDL（光纤延迟线）的卫星 OBS 全光交换，使数据在星间传输时保持光信号，消除核心节点 OEO。
- 项目结题时间：2027 年 9 月。
- 当前重点：**第 10 周已完成（2026-09-22）：ExpA 网格已冻结**（`research_reports/2026-09-week10/网格冻结.md`），异步臂 pilot 54 run 全部 exit 0。四条结论：① **τ 最优点随偏移模型移动**——同步 τ/T=1（6.78% / 13.51%），异步 Horizon τ/T=2（15.28% / 23.22%），两条曲线必须分开报告；② 异步偏移下 **VF 收益 7.4–8.9 pp ≥ FDL 收益 4.9–6.4 pp**；③ **偏移模型自身效应约 8 pp**（同步 12.88% → 异步 20.76%），大于主角的边际效应，故"τ/T≈1 最优"**不是无条件结论**；④ **热点 ρ≈0.81 上 FDL 基本失效**（只买到 0.1–0.6 pp，却用了 11–16 万次）。**下一周为第 11 周**：按冻结表跑批量（210 run，repeat 5），同步臂 60 run 须与异步臂**同一构建模式**重跑后再出图（第 8 周目录未记录模式，`.sca` 不能跨 debug/release 比较）；每批前后跑 `tools/audit_runs.py <目录> --repeat 5`。**第 9 周（2026-09-20）**：两条对照臂实现并验证——VF 调度器选项（`enableVoidFilling`，默认关，`src/CoreNode/OBS_ChannelCalendar.*`，Horizon 的严格推广）与 host 侧无缓存重传基线（`src/Retransmit/`，不改 `src/EdgeNode/`）；`useFDL=false` 回归**逐标量全同**（1701/1701、1697/1697）；同步偏移下 VF 补 **0 个** burst（附证明，D4 据此裁决 A+B）；重传确定性用例完成时延差 30.00 ms 恰等于超时值。第 8 周：τ 最优区间 τ/T_burst ≈ 1（仅同步偏移模型），热点 19.5 µs → ρ=0.8074。研究叙事以 `research_status.md` 第十节为唯一源，不在本文件复述。第一篇论文骨架：`research_reports/2026-09-week6-novelty/论文骨架.md`。**D1 已于 2026-09-20 闭合**：器件参数取自公开文献与厂商资料（`research_reports/2026-09-week8/器件参数与三条不等式.md`），仅质量/体积/功耗预算留作假设 + 敏感性。第 7 周遗留的两篇单波长排队论文已于第 9 周取得**全文**（`laevens2003single`、`vanhoudt2004channel`）——**τ/T_burst≈1 属已知粒度结论，不得当新发现**。不得把第 5 周标定（ρ=0.4→35.9 µs，`useFDL=false`）写成回环性能；**第 7 周重标定为 35.6 µs，35.9 µs 仅属第 5 周记录**。Task 2.1–2.3 三分支与 ICMP 回归已通过；pre-FDL 对照有条件通过（仅 `my ID` 不同）；release 链接（`-linet`）已由用户解决。

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
