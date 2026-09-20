# useFDL=true 回环确定性用例验证 — 跑法说明

> 日期：2026-09-10  
> 目的：正式开实验 A 前的功能通路验证（**不是**性能结论；**不使用** FDL-Calibrate 的标定数字）  
> 过关标准（阿sir）：冲突 → 进 FDL → `burstArrival+τ` 再出；BCP 到达偏移含 τ；Horizon 访问 FDL 口不崩

## 用哪条配置

已有配置，不必新建：

| Config | 作用 |
|---|---|
| `Test-FDL-Basic` | `useFDL=true`，`τ=20us` > waitTime ≈6.5us → **应走回环** |
| `Test-FDL-TooShort` | `τ=2us` → 应拒绝回环并丢弃（防「无条件收一切」假通过） |
| `Test-FDL-Off` | `useFDL=false` 对照 |

主验证跑 **`Test-FDL-Basic`**；建议同批再跑 TooShort / Off 作分支对照。

## 用户执行（Agent 不跑 make / 仿真）

仓库根目录若近期改过 CoreNode，先：

```text
make MODE=release
```

在 `Examples/RingFdlOBS/`：

```text
..\..\out\gcc-release\obsmodules.exe -u Cmdenv -f omnetpp.ini -c Test-FDL-Basic -n "../..;.;D:/inet/src"
```

可选对照：

```text
..\..\out\gcc-release\obsmodules.exe -u Cmdenv -f omnetpp.ini -c Test-FDL-TooShort -n "../..;.;D:/inet/src"
..\..\out\gcc-release\obsmodules.exe -u Cmdenv -f omnetpp.ini -c Test-FDL-Off -n "../..;.;D:/inet/src"
```

## 跑完交给阿sir的材料

1. `results/` 下对应 `.sca`（至少 `Test-FDL-Basic`）
2. sat2 相关 EV 片段（冲突、两次 OXC 预约、BCP offset +τ）
3. 自检表（填是/否）：

| 检查项 | Basic 预期 |
|---|---|
| sat2 `fdlUsageCount` > 0 | 是 |
| sat2 `burstLossContention` == 0 | 是 |
| sat3 收齐两侧包 | 是 |
| EV：第二次 OXC connect = 第一次 + τ | 是 |
| EV：BCP offset 增加恰好 τ | 是 |
| Horizon 访问 FDL 口无崩溃/断言 | 是 |

未过：只修回环路径，**不得**用标定 ρ/间隔数字充作回环证据。
