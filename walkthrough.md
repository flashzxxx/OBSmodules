# 最近一次工作记录

> 日期：2026-09-18（第 7 周收口 + 第 8 周执行）
> 详细记录：
> - `research_reports/2026-09-week7/分析.md`（M1 复跑、guardTime 根因、重标定、pilot 方差分析、三项决策）
> - `research_reports/2026-09-week7/分析-task1.3与排队论.md`（Task 1.3 四模式重跑与排队论检索）
> - `research_reports/2026-09-week8/分析.md`（ExpA 批量、单次回环红线缺陷、热点标定）
> - `research_reports/2026-09-code-review/walkthrough.md` 第五节（两个缺陷的代码级修复）
> 分支：`task/code-review-2026-09`
> 编译：`make MODE=debug` 通过（用户授权代跑）；未提交

## 一、M1 复跑：通过

四个确定性配置的 τ 时序与开关语义全部对上；Basic 两次 OXC 预约间隔**恰为 τ=20 µs**，
BCP offset 传导差**恰为 τ**；`TooShort` 与 `Off` 1701 条标量全同；
`Compatibility` 对 pre-FDL 基线非 `my ID` 差异 **0**。

## 二、修复了两个缺陷（详见 code-review walkthrough 第五节）

1. **`Test-FDL-TauMismatch` 不可能失败** —— NED 把 `fdl.delayTime` 绑定为 `fdlDelayTime`，
   ini 覆盖不了，守卫不可达。已换成两个真正可达的负向用例。
2. **负向用例以 `0xC0000005` 崩溃** —— gdb 追出三个独立根因：`lambdasPerPort` tokenizer
   堆越界写；9 个模块析构释放未初始化指针；`OBS_CoreOutput` 析构以计数而非数组指针为界。
   全部修复；负向用例 60/60 干净退出 exit=1。

## 三、ExpA 全部配置崩溃的根因（新发现，已修复）

`ExpA-Pilot` / `ExpA-NoFDL` / `FDL-Calibrate` 全部在 **t=1.94 ms** 崩溃，标量全零。
根因是 **09-15 的 H1 只改了一半**：

```
fdl_params.ini   **.sat*.coreSwitch.**.guardTime = 2us     ← 只覆盖 coreSwitch
params.ini:118   **.sender.guardTime = 0.000000001s        ← 边缘发送端仍是 1 ns
```

核心 OXC 预约窗口宽 `D + 3g/4` = `D + 1.5 µs`，而边缘发送端 pacing 只有 `D + 1 ns`
→ 本地口上相邻预约**重叠 1.5 µs**（实测 12 次重叠，每次恰 1.50 µs，全在 `inGate0`=port0）。
H1 之前两边都是 1 ns 所以第 5 周能跑。修复：补 `**.sat*.**.sender.guardTime = 2us`
（注意模式必须带 `**`，`sender` 不是 `sat*` 的直接子模块）。修复后 M1 仍零差异。

另发现 `ExpA-*` / `ExpB-Joint` / `ExpR-Retransmit` / `FDL-RateCheck` **缺 `seed-set`**，
已给 6 个 config 补 `seed-set = ${runnumber}`。

## 四、重标定与 pilot（H5 结项）

重标定输出到 `results/FDL-Calibrate-recal-2026-09-18/`（不覆盖第 5 周原文件）：
平台检查全绿，**ρ=0.4 → 35.58 µs**。

`ExpA-Pilot`（20 run）+ `ExpA-NoFDL`（10 run）：FDL 开/关效应量 **6.1–6.8 个百分点**，
而 n=5 可分辨下限仅 0.05–0.10 个百分点，**信噪比 71–123 : 1**。
**H5 的担心被实测否证，`repeat = 5` 保留不变。**

## 五、三项决策（2026-09-18 落定）

1. **回写负载间隔** —— 已执行：ρ=0.40 → **35.6 µs**（原 35.9），ρ=0.20 → **79.8 µs**（原 80.5），
   ρ=0.59 → 21.3 µs 不变。Week-5 数值在注释中保留并标注被取代。
2. **H6.2（突发级端到端时延）第一篇不做** —— sink 级 `endToEndDelay:histogram` 本来就在记录
   （count/mean/stddev/min/max），种子间均值 CV 仅 0.1%，而 FDL 开/关时延差 0.6–0.9 ms
   （50–80 倍噪声），图 5 的时延轴够用；τ 自身贡献另有 `burstNodeDelay`（H6.1）与
   `fdlLoopCount` 覆盖。**因此不必在 ExpA 批量前改 EdgeNode，批量可直接跑。**
   附带发现：FDL 开比关高 0.6–0.9 ms，远超 τ 本身（0.0344 ms），主因是间接效应，
   论文里必须写清，否则与"FDL 只加 τ"的直觉冲突。
3. **临时配置已清理** —— `_diag.ini` 的 4 个诊断配置折进 `fdl_tests.ini` 的 `Test-FDL-Diag-*`
   （显式写明两端 guard，不复用默认值），`_diag.ini` 与 `_recal.ini` 均已删除；
   重标定做法写进 `FDL-Calibrate` 的注释。

## 六、Task 1.3：第一阶段四模式重跑 + 排队论检索

**四模式重跑**：`FlowScaling`(24) + `LoadIntensity`(20) 用临时 `_task13.ini` 转出到
`results/task1.3-2026-09-18/`（不覆盖 2026-07-31 旧结果），全部 exit 0。
新增分析脚本 `tools/plot_phase1_modes.py`，图 `phase1_four_modes.png`。

**关键发现：第一阶段负载只有 0.24%，44 个 run 的 `ΣburstLossTotal` 恒为 0**
→ 该平台交付不了"带丢包的 Static/Dynamic 基线"，而这是 Task 1.3 的验收标准。
已另补 `Task13-LoadedModes`（标定负载、`useFDL=false`、每档 3 种子）：

| 模式 | ρ≈0.59 | ρ≈0.40 | 平均突发长 |
|---|---|---|---|
| **Static** | **17.18%** | **11.77%** | 4295 B |
| NoPreemption | 18.49% | 12.79% | 4295 B |
| Dynamic | 18.71% | 12.87% | 4210 B |
| RoundRobin | 18.97% | 13.28% | **1613 B** |

Static 优于 Dynamic ≈1.1 pp（两档一致）——这就是实验 B 需要的干净基线。
`RoundRobin` 突发长与其余三种差 2.7 倍，**公平性未解决前不进 2×2**。

**排队论检索**：10 条入 `refs.bib`（**题录级，未取得全文**）。三处更正：
"退化缓存"作者是 Rogiest 等不是 Lambert；WASPNET 第一作者是 Nizam；
**第 6 周"排队论都假定多波长"被证伪** —— `laevens2003single`(INFOCOM 2003) 与
`vanhoudt2004channel`(GLOBECOM 2004) 就是单波长，且后者给出信道利用率与丢包率，
与 5.1 口径直接重叠。已同步更正 `新颖性对照与继续条件.md` 与 `论文骨架.md` 2.1 节。

## 七、数据完整性

**Agent 用 `-r 0` 做对照时覆盖了第 5 周的 `results/FDL-Calibrate/FDL-Calibrate-0.sca`，
无法恢复**（仓库内无副本）。**影响已定量核查为可忽略**：ρ=0.4 工作点由 42.6/30 µs 插值而来，
不经 400 µs 点；实测 13 点与 14 点给出**同为 35.58 µs**；该 run 的全部派生数值留存在
`research_reports/2026-09-week5-calibrate/calibrate_report.txt`。

其余 13 个标定 run 完好，全部历史结果已备份到 `results/_historical-2026-09/`（111 个文件）。
之后所有重跑改用独立输出目录。

## 八、第 8 周（2026-09-18）

### ExpA 批量（提前执行原第 11 周）

`ExpA-TauSweep`（5 τ × 2 负载 × repeat 5 = 50 run）+ `ExpA-NoFDL`（10 run），全部 exit 0。
新增 `tools/plot_expA_tausweep.py`，图 `results/ExpA-TauSweep/expA_tau_sweep.png`。

| τ/T_burst | ρ≈0.40 丢包 | ρ≈0.59 丢包 | FDL 使用率（ρ≈0.59） |
|---|---|---|---|
| 0.25 | 10.79% | 16.32% | 4.8% |
| 0.50 | 9.09% | 14.76% | 9.0% |
| **1.00** | **6.78%** | **13.51%** | 15.3% |
| 2.00 | 8.35% | 16.38% | 19.4% |
| 3.99 | 11.74% | **19.61%** | 24.2% |
| **No-FDL** | 12.88% | **18.71%** | — |

**τ/T_burst ≈ 1 为最优**；ρ≈0.59 下 τ/T=4 的丢包（19.61%）**反高于不加 FDL（18.71%）**。
FDL 使用率随 τ 单调升而丢包先降后升 —— "用得更多 ≠ 效果更好"。

### 途中查出的红线缺陷（配置层）

`maxFdlLoopsPerBurst` 原为 `-1`（无限制），而 BCP 跨节点携带 `fdlLoopCount`，
所以突发**可以沿途每个节点各回环一次**，违反"单次回环"红线。第一遍实测多次回环 **147,935 次**。

配对探针（同种子，ρ≈0.59，只改该参数）：

| τ | `-1` | `1` | 多次回环 |
|---|---|---|---|
| 34.4 µs | 12.64% | 13.48% | 21,818 → **0** |
| 137.4 µs | 23.30% | **19.61%** | 41,997 → **0** |

**最大影响 3.69 pp**，所以整批重跑（1366 s）；定性结论存活。
第一遍数据留在 `results/ExpA-TauSweep-multiloop-2026-09-18/` 作对照。
已把红线做成常驻诊断：`Test-FDL-Diag-LoopStrict`（必须 0，实测 0）/ `LoopLoose`（预期 >0，实测 41,997）。

### 热点流量标定：ρ≥0.8 达成

瓶颈**恒为 sat3→sat2**（该链路承载右侧 5 个源：sat3/sat6/sat7/sat9/sat10）。
粗扫 40 run + 细扫 16 run：

| 间隔 | 25 µs | 21.3 µs | 20 µs | **19.5 µs** | 19 µs | 18 µs | 15 µs | 12 µs |
|---|---|---|---|---|---|---|---|---|
| ρ | 0.725 | 0.776 | 0.797 | **0.807** | 0.815 | 0.833 | 0.894 | 0.946 |
| 种子有差异 | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✗ |

**第三档负载点定为 19.5 µs → ρ=0.807**，已回写 `FDL-Hotspot` 的 `meanInterval`。
12 µs 时 4 个种子的 ρ 完全相同（完全饱和），不可作工作点。
新增 `tools/analyze_hotspot.py`。

> 教训：中途只扫到 21.3 µs 时误判"ρ 封顶在 0.78"（丢包已 34.65%，看着像饱和），
> 继续缩短才发现 18 µs 就有 0.833。**"看起来饱和"要扫穿才敢下结论。**

### D1 判断

Agent 建议**发出**：图 6 载荷可行性图的立论全系于此；且兜底默认值若落到 MEMS（切换 ≥8 ms，
而突发仅 34.4 µs），回环窗口不成立、第一篇主结果消失 —— 这个二元结果值得先问。
成本一页纸；**不阻塞第 8–11 周**，只影响第 12 周的图 6。用户选择暂缓。

## 九、下一步

- review 本次代码与配置改动；确认后可提交
- **第 9 周**：实现 VF 调度器选项 + host 侧重传基线（均默认关）+ 两者的确定性用例；
  取得 `laevens2003single` / `vanhoudt2004channel` 全文或摘要（否则论文 2.1 只能写题录级）
- **第 10 周前置**：设计"流量形态"维度，才能把热点 ρ≈0.81 并入 ExpA
- 仍待办：release 构建复跑、`colours` 分支同类越界（休眠）、L1（已评估为不必做）、D1 发不发
- 详细记录：`research_reports/2026-09-week8/分析.md` 与 `周会材料.md`

## 十、release 链接失败的根因（2026-09-20，待用户复跑验证）

**现象**：`make MODE=release` 全部 39 个对象编译完成后，链接报
`cannot find -linet`（`out/gcc-release/obsmodules.exe` 未生成）。

**根因**：`Makefile:283` 的 `LIBS = -L$(INET_PROJ)/out/$(CONFIGNAME)/src -linet`，release 需要
`D:/inet/out/gcc-release/src/libinet.dll`。INET 以 `--make-so` 构建为共享库
（`D:/inet/.oppbuildspec`），但当时只有 debug 输出（`D:/inet/out/gcc-debug/src`），
release 侧为空 → 链接期找不到库。**是环境缺件，不是源码错误。**

判据（静态核对，未跑 make）：

- PE ld（binutils 2.24）的 `-l` 搜索列表含 `lib%s.dll`（由 ld.exe 字符串确认），
  故 `libinet.dll` 本身即可作导入目标，不需要 `.dll.a`；`opp_shlib_postprocess`
  在 Windows 上是 no-op，本来就不生成导入库。
- 现有 `D:/inet/out/gcc-release/src/libinet.dll`（2026-09-20 11:23:48，15,305,579 B）
  带导出表、**24,608 个导出符号**，含 `UDPBasicApp` 等 INET 类；构建用 `-DINET_EXPORT`，
  与 obsmodules 编译时的 `-DINET_IMPORT` 对应。
- `out/gcc-release/src/` 下 39 个 `.o` 齐备 → 复跑只重链，不重编。

**用户动作**：仓库根目录重跑 `make MODE=release`。

**同时发现的连带问题**：`D:/inet/out/gcc-debug/src` 已不存在（INET debug 输出被清），
11:13 生成的 `out/gcc-debug/obsmodules.exe` 现在缺 `libinet.dll`，启动即
`0xC0000135`。需 `cd /d D:/inet/src && make MODE=debug` 重建。
**禁止用 release 的 `libinet.dll` 跑 debug 的 exe**：debug 内核是 `liboppsimd.dll`、
release 内核是 `liboppsim.dll`，而 `libinet.dll` 导入的是它自己那一版，
混用会把两个内核同时载入同一进程。

**配套改动**：`Examples/RingFdlOBS/tools/run-sim.ps1` 与 `run-sim.cmd` 原先不论
`-Mode`/第三参数都写死 `D:\inet\out\gcc-debug\src`，已改为按模式取 `gcc-$Mode\src`
（`OBS_INET_DLL` 环境变量覆盖仍有效），否则 release 模式脚本会直接用错 DLL 或报目录不存在。

**状态**：Agent 未复跑编译（`make`/仿真按约定由用户执行；本机沙箱内 gcc 子进程无法启动，
无法做最小链接探针）。等用户贴出 `make MODE=release` 结果再定性。

## 十一、仓库分类与入库范围收缩（2026-09-20）

release 通过后对工作区做了一次严格分类，只把必要内容纳入版本库：

- **`.gitignore`**：仿真结果改为整目录规则 `**/results/*`（仅保留 `.gitkeep`）——原有按扩展名的
  规则只覆盖 `results/` 第一层，`results/ExpA-*/` 之类嵌套输出其实一直没被忽略；另新增
  `/Examples/RingFdlOBS/tools/`、direction-review 的 deck 沙箱与 `*.pptx`/渲染图、
  `.doc/*.visual-check.*`、`/.dsh-*/`、`*.bak-*`。
- **`Makefile` 移出改动集**：它的 +369 行 diff 全是 makemake 自动生成噪音（184 行 `-I` +
  184 行 `rm -f` + 1 行 makedepend），根因是 `.oppbuildspec` 的 `--deep --meta:auto-include-path`
  把全树目录都当成 include 路径。已把 Makefile 还原为 HEAD 版本（`git diff HEAD -- Makefile` 为空），
  并在本机 `.oppbuildspec` 增加 `-X research_reports`、`-X Examples/RingFdlOBS/{tools,results}`
  等排除项，使下次 IDE 构建重新生成的 Makefile 同样干净。**注意**：`.oppbuildspec` 目前被
  `.gitignore` 忽略，属本机配置，未纳入版本库；换机器重新 clone 需要重新加这几项。
- **结果图入库**：新增 `research_reports/figures/`（4 个成图/表 + 来源 README），均为
  `results/` 下对应文件的 SHA256 一致副本；原始 `.sca`/`.vec`（约 1.1 GB）继续留在本地不入库。
- **恢复两个已跟踪的名录附件**：`conference_list_full.txt`、`journal_list_full.txt`（此前被删出
  工作区，按「名录以项目附件为准」保留）；只提交删除 `.download-ripgrep/tmp-file`。
- **暂存集**：117 个文件（`src/**` 22 + 实验/场景配置 4 + `.gitignore` 1 + 研究报告 67 +
  `.doc` 架构图 12 + Skill 2 + 根文档 3 + 结果图 5 + 删除 1）。**未提交**，由用户决定。
- 分类后 `git status` 的未跟踪条目为 0，`git add -A --dry-run` 输出为 0：工作区每一项要么已暂存、
  要么被显式忽略。`tools/`、`results/`、deck 沙箱等文件仍留在盘上，只是不再入库。
