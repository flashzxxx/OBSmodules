# FDL 加入后：核心节点代码改动说明（给壮哥）

> 用途：从「加 FDL 之前你熟悉的 OBS 核心」讲到「现在仓库里实际长什么样」  
> 范围：主要是 `src/CoreNode/`，以及 `Examples/RingFdlOBS/` 里为验证 FDL 加的配置  
> 不覆盖：边缘四模式调度细节（第一阶段基线，默认不动）  
> 更新日期：2026-09-10  
> 证据边界：文中「已实现 / 已用确定性用例验过」≠「实验 A/B 性能结论」；第 5 周标定是 `useFDL=false`，只证明负载，不证明回环调度

---

## 0. 一分钟版

加 FDL 之前：核心收到 BCP → 查路由 → 看输出波长 horizon 空不空 → **空就直通预约 OXC，不空就丢**。

加 FDL 之后：多了一个开关 `useFDL` 和一条 **OXC 回环延迟线**。冲突时若等待时间不超过 `τ`（`fdlDelayTime`），可以把突发先拐进 FDL，延迟 `τ` 后再送到原目标输出。

| 场景 | 条件（简化） | 行为 |
|---|---|---|
| A 直通 | 目标通道在突发到达时可用 | 一次 OXC 预约，直接转发 |
| B 回环 | 冲突，且 `waitTime ≤ τ`，且 FDL 口空闲 | 两次 OXC 预约：入口→FDL，再 FDL→目标 |
| C 丢弃 | 直通、回环都不满足 | 丢突发，记竞争丢包 |

这是 **回环 FDL**（解决输出撞车），不是 **输入 FDL**（解决边缘 offset→0 的时序窗口）。后者还没做。

---

## 1. 加 FDL 前，核心数据面长什么样

数据突发走光交叉，控制走 BCP（电处理）：

```text
光纤入口 → CoreInput ──控制信道──► ControlUnit（解析 BCP、查表、写 horizon、预约 OXC）
                └─数据信道──► OXC ──► CoreOutput → 光纤出口
```

关键对象：

- **BCP**：带着目的标签、突发颜色、突发长度、以及 **offset**（`burstArrivalDelta`：BCP 与数据突发的时间差）
- **Horizon 表**：每个「输出端口 × 波长」记录「该通道最早空闲时刻」
- **OXC**：按控制逻辑预约的 inGate→outGate，在突发到达前后把光路接好/拆掉

冲突时旧行为只有：**丢**。没有光域暂存。

---

## 2. 加了哪些文件 / 改了哪些文件

Git 上明确合入的起点：`1d45cf4 feat: add FDL module and integrate into CoreNode`。  
之后在控制逻辑、horizon、统计量、实验配置上还有演进（标定用的 `portLambdas[]` 等）。

| 文件 | 角色 |
|---|---|
| `src/CoreNode/OBS_FiberDelayLine.{ned,cc,h}` | 新模块：固定延迟 `delayTime`，统计 `FDL usage count` |
| `src/CoreNode/OBS_CoreNode.ned` | 挂上 `fdl` 子模块；增加参数 `useFDL`、`fdlDelayTime`；OXC 多一对回环口接到 FDL |
| `src/CoreNode/OBS_CoreControlLogic.cc/h` | 调度三分支（直通 / 回环 / 丢弃）；FDL 相关统计与 signal |
| `src/CoreNode/OBS_CoreOutputHorizon.cc/h` | horizon 多出一个「FDL 口」：`port == numPorts`，1 个波长 |
| `Examples/RingFdlOBS/fdl_tests.ini` | 确定性三分支 + 兼容性配置 |
| `Examples/RingFdlOBS/fdl_params.ini` / `fdl_experiments.ini` | W=1 场景冻结、标定与后续实验入口 |
| `Examples/RingFdlOBS/tools/calibrate_load.py` 等 | 由实测利用率反解发送间隔（第 5 周） |

边缘 `src/EdgeNode/` 作为第一阶段受保护基线，FDL 工作默认不改它。

---

## 3. 拓扑怎么接：回环接到哪

`OBS_CoreNode.ned` 里（逻辑示意）：

```text
                    ┌──────── ControlUnit ────────┐
光纤 in[] → Input ─┤                             ├→ Output → 光纤 out[]
                    │                             │
                    └─数据口──► OXC ◄──数据口─────┘
                                 │
                          多出来的一对口
                                 │
                            ┌────▼────┐
                            │   fdl   │  delayTime = fdlDelayTime (τ)
                            └────┬────┘
                                 │
                            再回到 OXC 输入
```

要点：

1. **FDL 接在 OXC 上**，不是接在入口解复用之前。  
2. 突发要进 FDL，控制逻辑必须先把 OXC 配成「入口 → FDL 出口口」；出来后再配「FDL 入口 → 目标输出」。  
3. 因此：**回环 FDL 仍然依赖 BCP 先到、offset 足够**，不能单独支撑「边缘 offset=0」。

参数（CoreNode）：

- `useFDL`（默认 `false`）：关则行为应对齐「加 FDL 前」的转发/丢弃判定  
- `fdlDelayTime`（默认 `10us`）：即文中的 **τ**

---

## 4. 调度逻辑：代码在干什么（带数字例子）

入口：`OBS_CoreControlLogic::handleMessage`，收到电域 BCP 后：

1. 读 offset → `burstArrival = now + arrivalDelta`  
2. 查路由得到 `outPort / outColour`  
3. 看目标波长在 `burstArrival` 时是否空闲  
4. 不空闲且 `useFDL==true` → 尝试场景 B  
5. 预约 OXC（一次或两次），改 BCP 的颜色/端口/剩余 offset，延迟 `processingTime` 后转发出去

### 例子 1：场景 A（直通）

假设：

- 突发将在 `t=100µs` 到达核心数据口  
- 目标波长 horizon = `90µs`（已空闲）  
- `useFDL` 无所谓

则：

- 只做 **一次** OXC：`inGate → 目标 outGate`  
- 连接约在 `100µs − guard/2`，断开约在突发结束 + guard  
- 更新目标波长 horizon 到「突发结束 + guard」  
- BCP 的 `burstArrivalDelta` 减去处理时延后继续往下游传  

EV 里大致会看到：`OXC Reserved (Direct): ...`

### 例子 2：场景 B（回环）——对应 `Test-FDL-Basic`

确定性冲突设计（`fdl_tests.ini`）：

- sat1、sat5 都发往 sat3，在 **sat2** 汇聚到同一输出  
- 传播时延相同；sat5 故意晚 **5µs** 发，让第二突发砸进第一突发已占用的窗口  
- `useFDL=true`，`fdlDelayTime=20us`（τ 大于 5µs 等待）

预期：

| 节点/量 | 期望 |
|---|---|
| sat2 `fdlUsageCount` | ≥1（控制逻辑计数） |
| FDL 器件 `FDL usage count` | 头尾相关，常为 2 |
| sat3 收包 | 两路都能收到 |
| EV | 出现 `OXC Reserved (FDL): 1st ...; 2nd ...` |

代码路径（固定颜色通道时，条件更直白）：

```text
若 目标 horizon > burstArrival:          # 冲突
  waitTime = horizon - burstArrival
  若 useFDL 且 waitTime ≤ τ 且 FDL 口 horizon ≤ burstArrival:
      走回环
  否则丢弃
```

回环时做两件事：

1. **第一次 OXC**：入口 → FDL；占用 FDL 口 horizon（索引 `port = numPorts`）  
2. **第二次 OXC**：FDL 回来的口 → 目标波长；占用目标 horizon，但时间轴整体平移到 `burstArrival + τ`  
3. **BCP offset 更新**：`arrivalDelta + τ - processingTime`  
   ——下游节点会认为数据突发晚了 τ 才到，和光路上真延迟一致

### 例子 3：场景 C（丢弃）——`Test-FDL-TooShort` / `Test-FDL-Off`

同一冲突图案，但：

- `Test-FDL-TooShort`：`useFDL=true`，`τ=2us` ＜ 5µs 等待 → **拒绝回环，丢**  
- `Test-FDL-Off`：`useFDL=false` → **直接丢**

两者旧行为标量应对齐；`fdlUsageCount` 均为 0。  
用来证明：FDL 不是「无条件全收」，关开关真的回到旧逻辑。

---

## 5. Horizon 表怎么为 FDL 扩容

`OBS_CoreOutputHorizon`：

- 以前：`horizon[0 .. numPorts-1][lambda]`，只覆盖真实输出光纤  
- 现在：多一行 `horizon[numPorts][0]`，表示 **FDL 回环口**（固定 1 个波长通道）

回环预约时：

- `getHorizon(numPorts, 0)` / `updateHorizon(numPorts, 0, ...)` 管 FDL 口忙闲  
- 普通口仍用原来的 `outPort, lambda`

注意：FDL 是 **定长延迟线**，不是随机读写缓存。horizon 建模的是「入口占用」；定长 τ 保证同一时刻不会在出口自己撞自己。这不等于已经用 `useFDL=true` 把所有边界情况都验穿——正式实验前仍要用回环确定性用例把 Scenario B 的二次预约和 `+τ` 再锁一遍。

---

## 6. FDL 模块本身极简

`OBS_FiberDelayLine`：

```text
收到消息 → fdlUsageCount++ → sendDelayed(msg, delayTime, "out")
```

它不做调度决策。  
**谁进 FDL、何时进**，全由 `OBS_CoreControlLogic` + OXC 预约决定。

---

## 7. 统计量：看什么文件、别误读

控制逻辑 `finish()` 会写出（节选）：

| 标量 | 含义 |
|---|---|
| `fdlUsageCount` | 调度上走了回环的次数 |
| `fdlUtilization` | FDL 忙时占比（粗） |
| `burstLossContention` | 因竞争丢掉的突发 |
| `burstLossRate` | 总丢 / 总收 |
| `maxChannelUtilization` | 最忙输出纤利用率（标定 ρ 的对照） |
| `portLambdas[p]` | 第 5 周加：运行时核对是否真是 W=1 |

器件侧另有 `FDL usage count`（进出各计一次时会大于调度计数）。

**易混点（重要）：**

- `FDL-Calibrate` 显式 `useFDL=false` → 得到 ρ=0.4 ↔ 35.9µs，只说明 **负载平台**  
- **不能**据此说「回环 FDL 调度已经用实验证明有效」

---

## 8. 和「输入 FDL / offset」的关系（防概念串台）

| | 回环 FDL（已实现） | 输入 FDL（未实现，实验 C） |
|---|---|---|
| 位置 | OXC 旁路回环 | 每输入口、OXC 之前 |
| 目的 | 输出竞争时暂存/错峰 | 吸收读标签 + 配 OXC 的时间 |
| 与 offset | 仍要 BCP 先到才能预约进 FDL | 用来支撑「边缘 offset=0」口径 |
| 当前证据 | 模块在；确定性三分支测过；正式实验前仍要再做 `useFDL=true` 回环确定性用例验证 | 仅有研究口径，无代码 |

一句话：**回环还的是「输出撞车」这笔时间债；输入 FDL 还的是「控制处理」这笔时间债。**

---

## 9. 你本地怎么亲手看一遍（建议顺序）

1. 打开 `OBS_CoreNode.ned`：找到 `fdl` 子模块和 OXC 回环两根线。  
2. 打开 `OBS_CoreControlLogic.cc`：搜 `usedFDLForThisBurst` / `Scenario B`。  
3. 打开 `fdl_tests.ini`：读 `Test-FDL-Basic / TooShort / Off` 的注释过关标准。  
4. 编译后跑（需你本机执行）：

```text
cd Examples\RingFdlOBS
..\..\out\gcc-release\obsmodules.exe -u Cmdenv -f omnetpp.ini -c Test-FDL-Basic -n "../..;.;D:/inet/src"
```

看 sat2 的 `.sca` 与 EV 是否符合第四节例子 2。  
更完整的跑法说明见：`research_reports/2026-09-week6-novelty/useFDL-true-回环确定性用例验证-跑法说明.md`。

---

## 10. 当前进度对照（读完代码后该有的地图）

| 阶段 | 状态（截至 research_status 2026-09-09） |
|---|---|
| FDL 模块接入 CoreNode | 已合入 |
| 调度三分支代码 | 已实现；确定性用例测过 |
| `useFDL=false` 兼容 | 转发/丢弃判定对齐（统计允许增量）；pre-FDL 对照有条件通过 |
| 实验平台标定 W=1 / 多源 / 随机化 | 第 5 周完成；ρ=0.4 → 35.9µs |
| 正式实验 A（τ 扫描） | **未开**；门槛是回环确定性用例再验一遍 |
| 输入 FDL / 边缘 offset=0 | **未做** |

---

## 附录：关键路径速查

| 路径 | 看什么 |
|---|---|
| `src/CoreNode/OBS_FiberDelayLine.*` | 延迟器件 |
| `src/CoreNode/OBS_CoreNode.ned` | 参数与回环接线 |
| `src/CoreNode/OBS_CoreControlLogic.cc` | A/B/C 决策与两次 OXC |
| `src/CoreNode/OBS_CoreOutputHorizon.*` | FDL 口 horizon |
| `Examples/RingFdlOBS/fdl_tests.ini` | 确定性验证场景 |
| `research_status.md` | 研究判断与周次看板 |
| `walkthrough.md` | 最近一次变更与命令 |

如需配图（数据流 / 时序），可再让小牛按本文第四节例子补框架图与时序图。