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
- 仍待办：`colours` 分支同类越界（休眠）、L1（已评估为不必做）、**质量体积预算（D1 唯一未闭合项，用假设 + 敏感性）**
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
- **提交**：按实现时间点整理，见第十二节。

分类后 `git status` 的未跟踪条目为 0，`git add -A --dry-run` 输出为 0：工作区每一项要么已提交、
要么被显式忽略。`tools/`、`results/`、deck 沙箱等文件仍留在盘上，只是不再入库。

## 十二、按实现时间点提交（2026-09-20）

工作区收尾后按改动所属实现阶段整理提交，发布到 `master` 的历史是：

| 提交 | 作者日期 | 内容 |
|---|---|---|
| `c95fb3b` | 2026-09-18 | `test:` FDL 场景/测试/实验配置（`omnetpp.ini` + 3 个 `fdl_*.ini`，4 文件） |
| `c393010` | 2026-09-17 | `feat:` FDL 审核项落地 + 由其引发的崩溃修复（`src/**` 22 文件，+626/−79） |
| `df3a272` | 2026-09-20 | `docs:` 第 1–8 周报告、文献台账、架构图、结果图 |
| `54c870b` | 2026-09-20 | `chore:` 结果不入库、忽略本地工具、移除 `.download-ripgrep/tmp-file` |

**拆分过程**：`src/**` 的改动横跨两个阶段——09-15 的审核项落地与 09-17 由 M1 复跑暴露的崩溃修复——
其中 6 个文件（`OBS_CoreControlLogic`、`OBS_CoreOutputHorizon`、`OBS_OpticalCrossConnect` 的
`.cc/.h`）同时含两个阶段的改动，因此做了**行级**拆分：先按 09-17 的改动清单移除对应代码块、提交
09-15 状态，再恢复并提交 09-17 状态。拆分前对全部 27 个待提交文件记录 SHA256，恢复后逐字节一致
（0 处不匹配）；09-17 那一段的 diff 恰为 **17 文件 +133/−10**，与事后统计完全吻合。

**两段最终合并为一个提交**：09-15 那段（原 `f2c523e`）未经编译验证（Agent 不跑 make），而 09-17
状态正是本次 `make MODE=release` 通过的状态。为让已发布历史里每个提交都是编译验证过的状态，
两段已合并为 `c393010`（**同一棵树，只有提交结构改变**：合并前 tip `698ff52` 与合并后 tip
`5222565` 的 `git diff` 为空）。合并前的两提交历史完整保留在本地分支
`archive/fdl-epoch-split-2026-09-20`：

```bash
git log --oneline 1d45cf4..archive/fdl-epoch-split-2026-09-20   # 两段时间点都在
git show archive/fdl-epoch-split-2026-09-20~4                    # 09-15 的中间状态（即 f2c523e）
git diff archive/fdl-epoch-split-2026-09-20~4 archive/fdl-epoch-split-2026-09-20~3 -- src   # 09-17 那批修复：17 文件 +133/−10
```

提交身份沿用仓库既有作者 `zhengzhx <759274219@qq.com>`，通过环境变量传入，**未写入任何 git config**；
作者日期按阶段设定，提交日期为实际提交时间。

## 十三、GitHub 连通性与 git 代理配置（2026-09-20）

推送完成后排查了这台机器的外网链路，记录供后续会话复用：

- **症状**：`git push` 报 `Failed to connect to github.com port 443 ... Couldn't connect to server`。
- **根因**：本机对 `github.com` 的解析（路由器 `192.168.8.17`、`223.5.5.5`、`114.114.114.114`、
  `1.1.1.1`、`8.8.8.8` 全都试过）一律返回 `20.205.243.166`，而该 IP 直连不可达（5 s 超时）；
  同一时刻 GitHub 其他 IP（`140.82.121.4` / `140.82.112.3` / `140.82.114.3`）0.2 s 返回 200。
  **换 DNS 服务器解决不了**——各解析器给的是同一个不可达 IP。
- **可用通道**：Clash Verge（`verge-mihomo`）监听 `127.0.0.1:7897`，系统 WinINET 代理也指向它
  （所以浏览器正常），但 git/curl 走直连，因此 git 被墙。
- **已做的配置**：`git config --global http.proxy http://127.0.0.1:7897`。验证：
  `git ls-remote --heads origin` 与 `git push --dry-run` 均通过（配置前 5/5 次直连推送全部失败）。
  撤销：`git config --global --unset http.proxy`（Clash 关闭时 git 远端操作会报连接 7897 失败）。
- **临时替代法（已还原）**：往 Git 自带的 `/etc/hosts`（`D:\Git\etc\hosts`）临时加
  `140.82.121.4 github.com`，推送完成后逐字节还原；系统 hosts 需要管理员权限，本会话不可用。
- 若要让 GitHub 对**所有**程序可用（IDE、pip 等），需在 Clash Verge 启用 TUN/虚拟网卡模式，
  或由管理员在系统 hosts 固定可用 IP。

## 十四、D1 闭合：改为公开文献建器件参数表（2026-09-20）

**取代第八节末尾的「D1 判断」**（当时结论是"建议发、用户暂缓"）。用户决定**不向他人询问** ——
理由是"问的人不一定准确"。改走公开文献与厂商资料，交付
`research_reports/2026-09-week8/器件参数与三条不等式.md`。

### 为什么可行

盘查后发现原 D1 询问稿问的 7 个参数里，**只有"质量/体积/功耗预算"真正在别人手上**：
- τ_max = 级数 × 步进 → 设计选择
- 光纤损耗 0.2 dB/km → 标准值
- **每级插损、切换时间 → 厂商资料**
- 工作波长 1550 nm → 常规

### 器件参数（含原始出处）

取自 Mouammar 等 arXiv:2605.04829 的 **TABLE II**，其脚注即原始出处，已逐条登记 `refs.bib`：

| 参数 | 光束转向 | MEMS | 电光 | InP SOA |
|---|---|---|---|---|
| 出处 | HUBER+SUHNER Polatis 6000s | 桂林 GLsun 4×4 级联 | Agiltron 4×4 高速 | Feyisa 等 *JLT* 40(19) 2022 |
| 切换时间 | 25 ms | 8 ms | **0.1 µs** | **5.2 ns** |
| 功耗 | 5 W | 1.25 W | 10 W | **0.58 W** |
| 每级插损 | 1 dB | 2.6 dB | **3.5 dB** | **0 dB** |
| 成熟度 | 商用 | 商用 | 商用 | 实验阶段 |

用 PyMuPDF 提取了该文全文（6 页）入 `refs/extracts/mouammar-2026-arxiv-2605.04829-text-2026-09-18.txt`。

### 三条不等式（`fdl_design.py` 器件模式）

1. **τ ≥ 切换时间**：扫描最小点 0.859 µs（10 Gbps 设计点）是电光切换时间的 8.6 倍、SOA 的 165 倍。
   **MEMS 被定量排除** —— τ ≥ 8 ms 等价于 τ/T_burst ≈ 2.3×10³，而实测最优在 1。
   → 不必再依赖"结构专业会不会说只能用 MEMS"这个外部答复。
2. **累计插损 ≤ 链路预算余量**：τ/T_burst=1 需 3 级（步进 0.5 µs）。
   电光 → **10.64 dB**；InP SOA → **0.14 dB**。即"成熟度 vs 链路预算"的取舍。
3. **级数 ≤ 质量/体积上限**：**唯一仍开放**，厂商表无此数据 → 显式假设 + 敏感性，不阻塞图 6。

### 两处更正

- **每级插损旧默认 0.5 dB 过于乐观**：Agiltron 实际 **3.5 dB/级**（差 7 倍）。
  `fdl_design.py` 的 `--switch-loss` 默认值同样过乐观，正式出图必须显式传参。
- 开关候选由"默认电光"扩为"电光 / InP SOA 二者可选"，MEMS 与光束转向被排除。

### 附带收获（新颖性）

Mouammar 原文（p.3）明确写：OPS/OBS "**not technologically mature due to the need for optical buffering**"，
故其采用 "OBS with path reservation (no contention resolution at nodes)"；
全文检索确认**未出现 FDL / 光纤延迟线**。这为论文 2.2 节"卫星 OBS 文献回避星上缓存、
依据是工程直觉而不是量化对照"提供了**可直接引用的原文**。

### 改动文件

改：`refs.bib`（+4 条器件出处，并更新 Mouammar 条目）、`D1-询问稿.md`（标作废）、
`论文骨架.md`（表 1/图 6 数据源、week-8 行）、`codex_phase2_tasks.md`（D1 行）、
`research_status.md`（头部 + 第 8 周行 + 第七节 D1 项）、`AGENTS.md`、week8 两份材料。
新增：`research_reports/2026-09-week8/器件参数与三条不等式.md`（**未提交**）、
`refs/extracts/mouammar-...-text-2026-09-18.txt`（**未提交**）。

**入库提示**：`Examples/RingFdlOBS/tools/` 已被 09-20 的决定设为不入库，
因此本轮没有新增工具脚本需要提交；D1 交付物是 Markdown 文档，正常纳入。

---

## 十五、第 9 周：VF 调度器与 host 侧重传基线（2026-09-20）

### 做了什么

1. **VF（Horizon + void filling）**：新类 `src/CoreNode/OBS_ChannelCalendar.{h,cc}`（每个 (port,lambda) 一条预约区间表），
   `OBS_CoreNode.ned` 新增 `enableVoidFilling`（默认 false），`OBS_CoreControlLogic` 新增
   `channelAccepts()` / `selectLambda()` / `reserveChannel()` 三个谓词，四条决策路径统一走它们。
2. **重传基线（实验 R）**：新目录 `src/Retransmit/`（`OBS_RetransmitSource` / `OBS_RetransmitSink` / `OBS_RetransmitPacket.msg`），
   两个模块 `like IUDPApp`，直接替换 `udpApp[i].typename`；`ExpR-Retransmit` 从占位改为真正跑重传。**未改 `src/EdgeNode/`**。
3. **确定性用例**：VF 4 例 + 重传 2 例 + 载入诊断 4 例，写入 `fdl_tests.ini`；新工具 `tools/compare_sca.py`（逐标量对照，可当 pass/fail）。
4. **文献**：`laevens2003single`、`vanhoudt2004channel` 两篇全文取得（作者稿 PDF），抽取文本入 `refs/extracts/`，`refs.bib` 由"题录级"升为"全文已读"。

### 设计要点（为什么默认路径一定没变）

Horizon 存标量 `horizon = A0 + D0 + 3g/4`，最后一条预约窗口右端 `= horizon − g/2`，
故 `horizon ≤ A'` ⟺ `A' − g/2 ≥ 窗口右端`，正是区间不重叠判据。
**无空隙时 VF 与 Horizon 判定完全相同**，VF 只能多接纳。实现上 `enableVoidFilling=false` 时
日历不分配、不访问，VF 标量不记录，因此默认运行的 `.sca` 与改动前**逐标量全同**。

### 验证方法与结果

编译：`mingw32-make MODE=release`、`MODE=debug` 均 exit 0、无 error/warning。
运行环境需把 OMNeT++ 的 `tools/win32/usr/bin`（MSYS `mkdir -p`/`sh`）也放进 PATH，否则 Makefile 的 `MKPATH` 会失败。

| 检查 | 结果 |
|---|---|
| `useFDL=false` 回归（debug，改动前备份 vs 改动后） | Basic/TooShort/Off/SO 各 **1701/1701 相同**；Compatibility **1697/1697 相同**；差异 0、新增标量 0 |
| VF 确定性对照 | Off：丢 1、收 2；On：丢 0、收 3、`voidFilledBursts=1`；FDLOn：同上且 `fdlUsageCount=0`；FDLOnNoVF：丢 1、收 2 |
| 重传确定性对照 | On：sat5 重传 1 / 放弃 0 / 完成时延 **52.02 ms**；Off：重传 0 / 放弃 1 / 未完成；差值 30.00 ms = 超时 |
| FDL 载入冒烟（`Test-FDL-Diag-LoopStrict`） | exit 0；`burstsLoopedMultiple` 全 0（红线仍成立）；`fdlUsageCount` 最大 35,443 |

**两个必须记住的坑**：

1. **`.sca` 不能跨构建模式比较**。release 跑回归时出现 79–84 条末位差异 + 3 ps 结束时刻差；
   把同一份新代码分别编 debug/release 对比得到同样量级的差异，而 debug 版对旧基线 0 差异 —— 差异来自构建模式，不是代码。
   跨周比较必须固定模式（第 8 周 ExpA 的模式未记录，第 10 周起写进结果目录名）。
2. **日历初始化顺序缺陷（已修）**：首版按 `gatesHorizon->getPortLambdas()` 定尺寸，而 `GatesHorizon` 在 NED 里声明在
   `ControlLogic` 之后、initialize 尚未执行 → 日历建成 0 信道 → 越界读导致**每个 burst 都被拒绝**。
   改为解析 `lambdasPerOutPort` 参数，并让 `OBS_ChannelCalendar` 对越界/0 信道**响亮 opp_error**，不再静默改结果。

### 新发现（负面但可发表）

载入实测（`Test-VF-Diag-LoadedOff/On`，ρ≈0.59，双种子）显示 **VF 补了 0 个 burst**，与 Horizon 逐标量相同。
原因是 `OBS_BurstSender` 使所有 burst 的偏移恒等于 `maxOffset`（本场景 1 ms）⇒ 到达顺序 ≡ BCP 顺序 ⇒
被 horizon 拒绝的 burst 必与已存在窗口重叠，**没有空隙可补**（分析见 week9/分析.md 第三节，附证明）。
把 10 个源的 `maxOffset` 改成 700–970 µs 后 VF 立即生效：全网已调度 burst 38,843 → **47,748（+23.0%）**。
**第 10 周需要用户裁决**：主网格是否引入"偏移异质"维度，或把 VF 降级为结构结论。

### 改动文件

新增：`src/CoreNode/OBS_ChannelCalendar.{h,cc}`；`src/Retransmit/OBS_RetransmitPacket.msg`、
`OBS_RetransmitSource.{h,cc,ned}`、`OBS_RetransmitSink.{h,cc,ned}`；
`research_reports/2026-09-week9/{分析.md,周会材料.md,文献精读-排队论单波长两篇.md}`；
`refs/extracts/laevens2003-infocom-text-2026-09-20.txt`、`refs/extracts/vanhoudt2004-globecom-text-2026-09-20.txt`。
改：`src/CoreNode/OBS_CoreNode.ned`、`src/CoreNode/OBS_CoreControlLogic.{h,cc}`、`Makefile`（OBJS/MSGFILES/依赖/clean/makedepend）、
`fdl_params.ini`（`enableVoidFilling` 默认值）、`fdl_tests.ini`（+10 个配置）、`fdl_experiments.ini`（ExpR 接线 + 说明）、`refs.bib`。

未提交（本地工具，按 09-20 决定不入库）：`tools/compare_sca.py`、`tools/run-week9-regression.ps1`、
`results/_regression-baseline-week9/`、`results/_week9-logs/`、`results/_week9-release-new/`。
