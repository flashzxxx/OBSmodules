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

### D4 裁决与异步偏移臂（2026-09-20，用户裁决 A+B）

用户裁决 **A+B 并行**：主网格保持第 8 周口径（同步偏移 + Horizon），把"同步情形 VF≡Horizon"写成结构性结论；
另建异步偏移臂回答"何时 VF 才值"。落地：

- `fdl_params.ini` 新增 `[Config FDL-Scenario-Async]`：10 个源各一个偏移，700–970 µs、步长 30 µs。
  选择约束写在配置注释里：每个值都远高于最坏每跳 BCP 处理累计（5 跳 × 1 µs），且不超过 FDL-Scenario 的 1 ms 上界，
  因此**报流量与负载旋钮不变**（变的只是 BCP 与其 burst 的相对位置）。
- `fdl_experiments.ini` 新增 `ExpA-AsyncTauSweep`（FDL 开：τ ∈ {17.2, 34.4, 68.8} µs × 2 负载 × 2 调度器，repeat 2）
  与 `ExpA-AsyncNoFDL`（同负载 × 2 调度器的参考线；异质偏移改变了竞争形态，故必须自建参考线，不能拿同步的 `ExpA-NoFDL` 比）。
- `fdl_tests.ini` 新增 `Test-VF-Diag-AsyncPilotSmoke`（0.2 s × 12 run，进 `results/_diag`），保证该臂**不在未冒烟的情况下**首次以 2 s 规模运行。
- 新工具 `tools/analyze_async_vf.py`（本地，不入库）：按 (τ, 负载, vf) 分组输出全网 received/scheduled/loss/voidFill/fdlUse/maxUtil。

0.2 s 冒烟结果（`results/_diag/Test-VF-Diag-AsyncPilotSmoke-*`，单种子，仅作方向性判断）：

| 负载 | τ | 丢包（Horizon） | 丢包（VF） | 差 | voidFill | 全网已调度 |
|---|---|---|---|---|---|---|
| 35.6 µs | 17.2 µs | 18.66% | 10.85% | −7.8 pp | 3,862 | 29,717 → 35,571 |
| 35.6 µs | 34.4 µs | 17.10% | 9.19% | −7.9 pp | 3,876 | 30,976 → 37,077 |
| 35.6 µs | 68.8 µs | 15.11% | 7.20% | −7.9 pp | 3,966 | 32,774 → 39,195 |
| 21.3 µs | 17.2 µs | 26.16% | 16.74% | −9.4 pp | 6,679 | 40,876 → 50,994 |
| 21.3 µs | 34.4 µs | 24.63% | 15.37% | −9.3 pp | 6,414 | 42,602 → 52,752 |
| 21.3 µs | 68.8 µs | 23.32% | 14.35% | −9.0 pp | 6,315 | 44,073 → 54,247 |

**这条臂的量级值得注意**：VF 在异步偏移下把丢包降 7.9–9.4 个百分点，而第 8 周测到的 FDL 自身效应是 5–7 个百分点。
也就是说**调度器的选择可能比延迟线本身影响更大**——这必须在 5.1 里如实报告，并且意味着"FDL 值不值"的判断须在固定调度器下陈述。
（0.2 s、单种子，仅作方向判断；2 s × repeat 2 的 pilot 见 `results/ExpA-AsyncTauSweep/`、`results/ExpA-AsyncNoFDL/`。）

### 改动文件

新增：`src/CoreNode/OBS_ChannelCalendar.{h,cc}`；`src/Retransmit/OBS_RetransmitPacket.msg`、
`OBS_RetransmitSource.{h,cc,ned}`、`OBS_RetransmitSink.{h,cc,ned}`；
`research_reports/2026-09-week9/{分析.md,周会材料.md,文献精读-排队论单波长两篇.md}`；
`refs/extracts/laevens2003-infocom-text-2026-09-20.txt`、`refs/extracts/vanhoudt2004-globecom-text-2026-09-20.txt`。
改：`src/CoreNode/OBS_CoreNode.ned`、`src/CoreNode/OBS_CoreControlLogic.{h,cc}`、`Makefile`（OBJS/MSGFILES/依赖/clean/makedepend）、
`fdl_params.ini`（`enableVoidFilling` 默认值）、`fdl_tests.ini`（+10 个配置）、`fdl_experiments.ini`（ExpR 接线 + 说明）、`refs.bib`。

未提交（本地工具，按 09-20 决定不入库）：`tools/compare_sca.py`、`tools/run-week9-regression.ps1`、
`results/_regression-baseline-week9/`、`results/_week9-logs/`、`results/_week9-release-new/`。

---

## 十六、第 10 周：ExpA 网格冻结（2026-09-22）

### 做了什么

1. **配置冻结**：异步臂 τ 由 3 档扩到 5 档（τ/T = 0.25 / 0.5 / 1 / 2 / 4），结果目录改为带构建模式
   （`results/ExpA-AsyncTauSweep-release`、`ExpA-AsyncNoFDL-release`）；新增场景 B `ExpA-HotspotPoint`
   （热点 ρ≈0.81、FDL on/off × 3 τ、目录同样带模式）。主网格 `ExpA-TauSweep` / `ExpA-NoFDL` **未改**，
   只把"VF 维为何不开"的注释换成 D4 裁决的说明。
2. **pilot**：`ExpA-AsyncTauSweep` 40 + `ExpA-AsyncNoFDL` 8 + `ExpA-HotspotPoint` 6 = 54 run，release，全部 exit 0。
3. **交付**：`research_reports/2026-09-week10/网格冻结.md`（参数表 + pilot 曲线 + 第 11 周批量清单 + 不得声称清单）、
   `周会材料.md`、`research_reports/figures/pilot_grid_freeze.png`。
4. **新增本地工具**：`tools/audit_runs.py`（结果目录清单 + MISSING/DUPLICATE/STALE 检查）、`tools/plot_grid_freeze.py`。

### 结果（丢包率 %，全网汇总）

| 负载 | 偏移 | 调度器 | 无 FDL | 0.25 | 0.5 | 1 | 2 | 4 |
|---|---|---|---|---|---|---|---|---|
| ρ≈0.40 | 同步 | Horizon | 12.88 | 10.79 | 9.09 | **6.78** | 8.35 | 11.74 |
| ρ≈0.59 | 同步 | Horizon | 18.71 | 16.32 | 14.76 | **13.51** | 16.38 | 19.61 |
| ρ≈0.40 | 异步 | Horizon | 20.76 | 19.75 | 18.95 | 17.42 | **15.28** | 16.46 |
| ρ≈0.59 | 异步 | Horizon | 28.07 | 26.99 | 26.11 | 24.58 | **23.22** | 25.86 |
| ρ≈0.40 | 异步 | +VF | 13.40 | 12.06 | 11.04 | 9.44 | **7.26** | 7.04 |
| ρ≈0.59 | 异步 | +VF | 19.48 | 17.90 | 16.87 | 15.47 | **14.32** | 14.56 |
| ρ≈0.81 热点 | 同步 | Horizon | 35.42 | — | 34.80 | 35.03 | 35.36 | — |

**四条结论**：① τ 最优点随偏移模型移动（同步 1、异步 Horizon 2，转折点都在网格内）；
② 异步下 VF 收益 7.4–8.9 pp ≥ FDL 收益 4.9–6.4 pp；③ 偏移模型自身效应约 8 pp（同步 12.88% → 异步 20.76%），
大于主角的边际效应，故"τ/T≈1 最优"不是无条件结论；④ 热点上 FDL 被使用 11–16 万次却只买到 0.1–0.6 pp
（`fdlInFlightOccupancy` 最高 2.13，已打满）——饱和瓶颈上多等一个 τ 解决不了问题。

### 本周修掉的两个"会静默出错图"的问题

1. **参考线被并进曲线**：`plot_grid_freeze.py` 第一版按迭代变量分组，而 `useFDL` 是配置里写死的、不在迭代变量中，
   于是 `ExpA-NoFDL` 的参考 run 与 FDL 开的曲线被合并平均——ρ≈0.40 的参考线算成 9.94%，真值 12.88%
   （第 8 周记录一致）。改为**按目录划分 family**，教训写进脚本头注释。
2. **`$repetition` 读不到**：OMNeT++ 4.6 把它放在 `attr iterationvars2`，`audit_runs.py` 第一版因此把每个 cell
   都报成"缺全部重复"。已同时修正两处解析。

**教训**：能画出图 ≠ 口径正确。第 11 周批量前后都要 `python tools/audit_runs.py <目录> --repeat 5`。

### 第 11 周批量清单（冻结表）

异步臂 100 + 异步参考线 20 + 热点 30 + 同步臂重跑 60（须与本批**同一构建模式**，第 8 周目录未记录模式）= 210 run。

### 改动文件

新增：`research_reports/2026-09-week10/{网格冻结.md,周会材料.md}`（`分析-pilot与网格冻结建议.md` 已于上一提交入库）、
`research_reports/figures/pilot_grid_freeze.png`。
改：`fdl_experiments.ini`（异步臂 5 档 τ + 模式化目录 + 场景 B）、`research_status.md`、`AGENTS.md`、
`codex_phase2_tasks.md`、`walkthrough.md`。
本地未入库：`tools/audit_runs.py`、`tools/plot_grid_freeze.py`、`tools/analyze_async_vf.py`、`results/ExpA-*-release/`。

---

## 十七、第 11 周：批量与图 3–5 初版（2026-09-22）

### 做了什么

1. **按冻结表跑批量**：异步臂 repeat 2→5、热点 1→5；新增同步臂的 release 重跑配置（`ExpA-TauSweep-release`、`ExpA-NoFDL-release`，
   写进新目录，第 8 周目录保持不动）与轻负载无恢复参考 `ExpA-NoFDL-Light`；`ExpR-Retransmit` 目录加模式后缀。
   四个后台作业并行（32 核），合计 **225 run 全部 exit 0**。
2. **每批审计**：`tools/audit_runs.py <目录> --repeat 5`，8 个目录全部格齐、无重复、无跨批文件。
3. **判定第 8 周模式**：新增 `tools/compare_dirs.py`（按 `iterationvars2` 配对，因为文件名带配置名），
   50/50 与 10/10 格全部有末位差异（平均 181 / 135 条/文件）⇒ **第 8 周是 debug 构建**；release 重跑复现
   12.88 / 18.71 / 6.78 / 13.51 **逐位一致** ⇒ 模式差异对结论无影响。
4. **图 3–5 与逐格表**：新脚本 `tools/plot_paper_figs.py`（一个脚本出三张图 + `--csv` 汇总，避免图文两条代码路径分叉）。
5. **实验 R 修复与边界查清**：见下。

### 实验 R 的两个发现（一个建模假象、一条真实边界）

**假象**：`minSizeWithPadding = 500B` 把 64 B 的确认包填充成 4 µs 的交换机占用（本应 0.8 µs，放大约 2.4 倍）。
未修时该臂**不收敛**：活动消息 1.5 s 内 47k → 729k（延迟线各臂稳定 16–19k），32 位进程 T=1.49 s 以 0xC0000005 静默退出。
已在 `ExpR-Retransmit` 内覆盖为 64 B。

**边界**：修掉后 79.8 µs 稳定（活动消息 31k 平坦），35.6 µs 仍无界增长（188k–980k，60/120 ms 两个超时值皆然）。
⇒ 开环重传的可用区间比单级回环窄（新发 ρ≈0.2 对 0.8）；进高负载区必须加重传窗口，属独立设计变更。
另：60 ms 超时对最坏路径（5 跳 × 5 ms 每程）偏紧，实测平均完成时延 69 ms > 超时 ⇒ 大量误触发；并列跑了 120 ms 版本，
两者送达比例相近（80.2% 对 81.6%），说明 79.8 µs 下重传主要是真丢包。

**新工具**：`tools/check_sim_stability.py`（从 Cmdenv 的 `present:` 计数判断批次是否会因积压耗尽内存，可作闸门）。

### 结果

| 指标 | 值 |
|---|---|
| 同步 τ 最优点 | τ/T=1：6.78%（ρ≈0.40）、13.51%（ρ≈0.59） |
| 异步 τ 最优点 | τ/T=2：15.29% / 23.22%（Horizon）；VF 曲线更平更低（14.33–15.47%） |
| 端到端送达载荷（同步，τ/T=1） | 2520 / 3349 Mbit/s，相对无 FDL **+23% / +21%** |
| 异步 VF 的贡献 | 相对 Horizon **+32% / +40%**（大于延迟线本身） |
| 热点 ρ≈0.81 | 丢包 34.8–35.4%（τ 无关），送达载荷 ~1396 Mbit/s；FDL 使用率 15–22%、在飞占用 0.7–2.1 |
| 无缓存重传（同实测负载） | 送达载荷 **1145 / 1126 Mbit/s**，约为回环臂一半；`retx/fresh ≈ 1.0`，线上有用率 ~61% |

### 改动文件

新增：`research_reports/2026-09-week11/{分析.md,周会材料.md}`、`research_reports/figures/{fig3_loss_vs_tau.png,fig4_fdl_usage.png,fig5_loopback_vs_retransmit.png,batch_summary.csv}`。
改：`fdl_experiments.ini`（异步臂 repeat 5、模式化目录、同步 release 重跑、轻负载参考、ExpR 填充覆盖与稳定负载、120 ms 变体、场景 B repeat 5）、
`fdl_tests.ini`（`Test-ExpR-Diag-Smoke`）、`research_status.md`、`AGENTS.md`、`codex_phase2_tasks.md`、`walkthrough.md`。
本地未入库：`tools/{audit_runs,compare_dirs,check_sim_stability,plot_paper_figs,plot_grid_freeze,analyze_async_vf}.py`、`results/ExpA-*-release/`、`results/ExpR-*-release/`、`results/_pilot-week10/`、`results/_crash-2026-09-22/`。

---

## 十八、第 12 周：图 6 可行性图与 D2 关闭（2026-09-22）

### 做了什么

1. **图 6 载荷可行性三区图**（脚本 `tools/plot_fig6_feasibility.py`）：面板 A 用第 11 周冻结网格的实测 τ 曲线 + 有用区/无增益区；面板 B 用第 8 周器件表的级联档位模型（**直接 import `fdl_design.py`**，不重算）画电光与 SOA 的累计插损、级数与假设边界。性能轴与器件轴来自同一脚本，避免图文两条路径分叉。
2. **D2 判定**：新增 `FDL-RateCheckRef`（1 Gbps 孪生点，五项无量纲比值与 10 Gbps 的 `FDL-RateCheck` 一一对应），跑 3+3 run，按 **2026-09-14 预登记**的四项判据比对（脚本 `tools/compare_ratecheck.py`）。
3. **一处口径更正**：`FDL-RateCheckRef` 的注释最初写成"两者都是 0.2 s"，实际参考点跑 2 s；已按真实窗口关系改写。

### 图 6 结果

| 区域 | τ/T_burst | 依据 |
|---|---|---|
| 有用 | 0.5 – 2（峰值 1） | 收益 +3.8～+6.1 pp（ρ≈0.40）、+2.3～+5.2 pp（ρ≈0.59） |
| 无增益 | < 0.5 或 > 2 | 收益衰减；ρ≈0.59 下 τ/T=4 为 **−0.9 pp**（比不加更差） |
| 物理不可行 | MEMS 需 τ/T ≥ 2328、光束转向 ≥ 7276 | 不等式一；比实测最优高 3 个数量级 |

器件代价（τ/T=1）：电光 **3 级 = 10.64 dB**（光纤仅 0.14 dB）对 InP SOA **0.14 dB**；假设链路余量 10 dB（敏感性 6/10/14）、级数上限 6（敏感性 4/6/8）、步进 0.5 µs。**结论：对成熟电光器件，"有用"与"付得起"不重叠**——问题不是能不能装，而是成熟度 vs 链路预算。步进取 0.1 µs 时电光插损升到 21.14 dB（级数 6），比质量体积更硬。

### D2 结果（GO）

| 量 | 10 Gbps | 1 Gbps | 原始偏差 | 按 T_burst 归一 | 判定 |
|---|---|---|---|---|---|
| maxChannelUtilization | 0.392468 | 0.391643 | 0.21% | 0.21% | PASS |
| fdlUsageCount | 38,512.3 | 50,037.3 | 23.03% | **0.06%** | PASS |
| carriedBursts | 340,764 | 442,972 | 23.07% | **0.00%** | PASS |
| burstLossRate | 0.0514225 | 0.0513696 | 0.10% | 0.10% | PASS（绝对差 0.0053 pp） |

**两个计数量为什么会先"失败"**：两运行时长差 10 倍（无量纲点相同 ⇒ 时长按速率缩放），但 **warmup 不能缩放**——它排除的是每跳 5 ms 的填路瞬态，是真实秒。于是测量窗口为 43,655 vs 56,752 T_burst，**比值 1.3000**，与 23% 的原始偏差完全吻合；按 T_burst 归一后差 0.06% / 0.00%。判据在计数类量上只有归一化后才有内容，工具同时打印原始与归一化两列，不隐藏原始结果。

**范围限制**：只覆盖 τ/T=1、sendInterval/T=1.24 一个点；ρ 不可跨 0.2 s 与 2 s 的配置比较。

### 改动文件

新增：`research_reports/2026-09-week12/{分析.md,周会材料.md,ratecheck-output.txt}`、`research_reports/figures/fig6_feasibility.png`。
改：`fdl_experiments.ini`（`FDL-RateCheckRef` + 窗口说明）、`research_status.md`、`AGENTS.md`、`codex_phase2_tasks.md`、`论文骨架.md`（5.3 与图表映射）、`walkthrough.md`。
本地未入库：`tools/{plot_fig6_feasibility,compare_ratecheck}.py`、`results/FDL-RateCheck*/`。

---

## 十九、第 13 周：实验 B 联合 2×2 与「边缘是否移动交叉点」（2026-09-22）

### 做了什么

1. **实验 B 联合 2×2**：边缘 `dispatchMode` ∈ {Static=3, Dynamic=0} × 核心 FDL ∈ {关, 开}，
   79.8 / 35.6 / 21.3 µs 三档负载，repeat 5，共 **60 run**（`results/ExpB-Joint-release`，release）。
2. **重传 × 边缘配对臂**（新配置 `ExpR-EdgePair`）：host 侧无缓存重传 × 两种边缘模式，79.8 µs
   请求间隔、重传超时 120 ms、重传上限 3 次；对照建立在**同一实测信道负载**（ρ≈0.40–0.42）而非同一等待时长上，共 **10 run**（`results/ExpR-EdgePair-release`）。
   两批审计 0 MISSING / 0 DUPLICATE / 0 STALE，退出码均 0。
3. **图 7**（`tools/plot_fig7_expB.py` → `figures/fig7_edge_core_2x2.png`）：三面板——2×2 端到端
   丢包、FDL 端到端收益、两种边缘模式下的交叉点；同时打印三张表（2×2 全量、逐臂 FDL 收益、
   重传对照）。
4. **回答第 12 周留下的 5.4 节问题**：交叉点是否移动。

### 结果：交叉点不移动

| 边缘    | 回环（35.6 µs 负载点；τ = 34.4 µs，τ/T_burst ≈ 1.00） | 重传（79.8 µs 请求间隔，超时 120 ms，上限 3 次） | 比值 |
|---|---|---|---|
| Static  | 2241.2 Mbit/s @ ρ=0.4114 | 961.9 Mbit/s @ ρ=0.4019 | 2.33 |
| Dynamic | 2519.7 Mbit/s @ ρ=0.4571 | 1125.7 Mbit/s @ ρ=0.4150 | 2.24 |

重传把 ρ 堆到同一水平（0.40–0.42）靠的是 24–33 万次重传副本（请求 24.4 万；Static 另有 15.75%
边缘丢弃、端到端丢 31.47%，Dynamic 边缘 0%、端到端丢 19.81%），有效载荷只有回环的 43% / 45%。
**两种边缘模式下回环都远高于重传 ⇒ 交叉点不移动**；边缘模式只平移绝对损失水平（最高 12.6 pp）。

### 主要发现：一个核心计数器看不见的恒定丢弃，以及据此对第 7 周的更正

Static 在全部 3 档负载、FDL 开与关下恒定丢弃 **14.21–14.27%**（波动 ≤0.06 pp），Dynamic 恒为
**0.00%**。这部分发生在接入侧 Dispatcher 的队列阈值上、在成束之前，**核心任何计数器都看不到**。**0.00%**。这部分发生在成束之前，**核心任何计数器都看不到**。**第 14 周研究日 A 已定量更正**：该机制不是「队列阈值」，而是 `(targetLabel - 1) % numQueues` 在「10 目的地、8 队列」下的混叠；队列数提到 10 后丢弃精确归零，端到端落到 Dynamic 水平。**14.2% 是配置伪影，不是 Static 的性质。** 证据：`research_reports/2026-09-week14/研究日A-运行记录.md` 第五节 5.1。

因此第 7 周 Task 1.3 的结论必须更正：当时按核心 burst loss 读得「Static 比 Dynamic 好约
1.1 pp」（21.3 µs：17.18% vs 18.71%），端到端口径反过来，Dynamic 好 5.0–12.6 pp。

| 负载 | 核心 burst loss（Static / Dynamic） | 端到端丢包（Static / Dynamic） |
|---|---|---|
| 79.8 µs | 6.00% / 6.66%（Static 好 0.66 pp） | 29.43% / 19.56%（Dynamic 好 9.87 pp） |
| 35.6 µs | 11.81% / 12.93%（Static 好 1.12 pp） | 42.10% / 35.05%（Dynamic 好 7.05 pp） |
| 21.3 µs | 17.17% / 18.69%（Static 好 1.52 pp） | 52.20% / 47.19%（Dynamic 好 5.01 pp） |

机理可从计数器直接读出：Static 的 burst 更少更大（79.8 µs 下 206,793 束 vs Dynamic 244,255
束），故核心看到的失败束数更少；Dynamic 的抢占会冲掉部分正在成束的包，反而抬高核心 burst
loss。**教训：核心 burst loss 只能当核心内部诊断量，评价边缘或联合效果必须看端到端送达。****教训：核心 burst loss 只能当核心内部诊断量，评价边缘或联合效果必须看端到端送达。****第 14 周研究日 A 更正**：上述「端到端与核心方向相反」的两个方向出自同一个混叠伪影，队列数升到 10 后两模式在核心与端到端两侧都一致——「Dynamic 端到端更优」不成立。仍然成立的是：核心 burst loss 与端到端不可互换，评价「谁更好」必须看端到端送达，但引用这些数字须带 8 队列对 10 目的地的配置前提。

### 副产物：端到端 pp 收益为什么是核心读数的 1.9–2.9 倍

两层换算，六组全部核验：

- 一个 burst 约 3 个包（4295/1400=3.07、4207/1400=3.00）⇒ 核心 burst 丢失率每 1 pp 在端到端
  上约值 3 pp；
- 边缘恒定丢弃给收益打折：`端到端 = 边缘丢弃 + (1−边缘丢弃) × 网络丢弃` ⇒ FDL 的端到端收益 =
  `(1−边缘丢弃) × Δ(网络丢弃)`，Static 乘 0.858、Dynamic 乘 1。

| 边缘 | 负载 | 核心 Δ | 端到端 Δ | 比值 | `(1−边缘丢弃)×Δ网络` 复核 |
|---|---|---|---|---|---|
| Static | 79.8 µs | 4.54 pp | 11.24 pp | 2.48 | 11.29 pp（实测 11.24） |
| Static | 35.6 µs | 6.11 pp | 13.31 pp | 2.18 | 13.34 pp（实测 13.31） |
| Static | 21.3 µs | 5.46 pp | 10.26 pp | 1.88 | 10.27 pp（实测 10.26） |
| Dynamic | 79.8 µs | 4.87 pp | 13.95 pp | 2.86 | 13.96 pp（实测 13.95） |
| Dynamic | 35.6 µs | 6.16 pp | 15.13 pp | 2.46 | 15.14 pp（实测 15.13） |
| Dynamic | 21.3 µs | 5.21 pp | 10.88 pp | 2.09 | 10.88 pp（实测 10.88） |

「Δ网络 ≈ Δ核心 burst 丢失 × 每 burst 包数」这一近似在 ρ≈0.20 吻合到 ≤0.7 pp，但在 ρ≈0.59
系统性高估 3.6–9.0 pp ⇒ 高负载下被丢的 burst 平均短于总体均值，该换算不是与负载无关的常数。

**burst 长度公平性**（四臂先报）：Static 恒 4295.0 B（`minSizeWithPadding` 下限），Dynamic
4206.5–4210.3 B，最大差 2.1%，不足以解释 5.0–12.6 pp 的端到端差距。

### 本周修掉的一个会静默出错图的问题

`plot_paper_figs.parse_run` 按空白切分 `.sca`。`.sca` 的真实布局是 `scalar <module> \t<name>
\t<value>`，**名字含空格时会被引号包起来**（如 Dispatcher 的 `"Dropped Packets"`），空白切分
把模块名和名字都切错，导致图 7 新增的 `edgeDrop%` 整列算不出来（`nan`）。改为按三个字段做正则
匹配后，14.2% 的常数才显形。已用第 11 周全部图形复核，同步 12.88/6.78、异步 23.22/15.29、热点
35.4/34.8 **逐值不变**，确认此前结论未受影响。

### 改动文件

新增：`research_reports/2026-09-week13/{分析.md,周会材料.md}`、`research_reports/figures/fig7_edge_core_2x2.png`。
改：`fdl_experiments.ini`（`ExpB-Joint`、`ExpR-EdgePair`）、`research_status.md`、`AGENTS.md`、
`codex_phase2_tasks.md`、`论文骨架.md`（5.4 与图表映射）、`walkthrough.md`。
本地未入库：`tools/{plot_fig7_expB,plot_paper_figs}.py`、`results/ExpB-Joint-release/`、`results/ExpR-EdgePair-release/`。

---

## 二十、第 14 周研究日 A：三个敏感性臂（2026-09-22）

### 做了什么

1. **S1 边缘队列数敏感性**：`ExpS-EdgeQueues`，`numPacketBurstifiers` ∈ {8, 10} ×
   `dispatchMode` ∈ {Static, Dynamic} × 3 档负载 × repeat 5 = **60 run**。
2. **S2 突发长度敏感性**：`ExpS-BurstLen`（`numPackets` ∈ {1, 3, 6} × τ/T 比 ∈ {0.5, 0.85, 1, 2} ×
   2 档负载 × repeat 5 = **120 run**）与 `ExpS-BurstLen-NoFDL`（同三档粒度各自的无 FDL 参考线，
   **30 run**）。
3. **S3 热点 τ 网格两端**：`ExpA-HotspotGrid`，补 τ/T = 0.25（8.6 µs）与 4（137.4 µs），
   **20 run**。
4. 合计 **230 run**，release、repeat 5，四臂并行约 29 min，全部 exit 0；`audit_runs.py` 四个目录
   46 个 cell 全部满 5 重复、0 MISSING / 0 DUPLICATE / 0 STALE。
5. **先冒烟再批量**（`Test-Smoke-EdgeQueues/BurstLen/HotspotGrid`，均 0.2 s），并在冒烟发现的异常上
   追加两个机制探针（偏置预算、NoPreemption）。
6. 新工具（本地不入库）：`tools/analyze_w14_sensitivity.py`（三臂聚合）、`tools/check_w14_smoke.py`
   （冒烟裁决：队列数是否生效、实测 burst 长度、τ 是否可追溯）。

### 机制更正：第 13 周的 14.2% 与「方向相反」都是配置伪影

第 13 周把 Static 的恒定丢弃写成「接入侧 Dispatcher 队列阈值」，并据此得出「核心 burst loss 读得
Static 好 0.66–1.52 pp，端到端反而 Dynamic 好 5.01–12.59 pp，所以核心计数器不可作评价依据」。读
`src/EdgeNode/OBS_PacketDispatcher.cc` 的 mode 3 分支可以看到真实判据：

```cpp
int fixedQueue = (targetLabel - 1) % numQueues;   // label 1-10 -> queue 0-7
// 该队列忙且 label 不一致时：丢弃（selectedQueue 保持 -1）
```

10 个目的地只用 8 个队列时 label 1/9 共用队列 0、2/10 共用队列 1，于是恒定丢 14.2%。
`numQueues` 在 `OBS_BurstAssembler.ned` 里被绑成 `numQueues = numPacketBurstifiers`，dispatcher 输
出门数、burstifier 数组与 sender 输入数组都由它定尺寸，所以**把队列数提到 10 只需 ini 参数，没改
`src/EdgeNode/` 任何源码**（`params.ini` 的 8 仍是基线，只有本实验要求 10）。

| `numPacketBurstifiers` | `dispatchMode` | 丢弃率（3 档负载合计） | 端到端丢包 79.8/35.6/21.3 µs |
|---|---|---|---|
| 8 | Static | **14.212%**（逐值复现第 13 周） | 29.43 / 42.10 / 52.20% |
| 10 | Static | **0.000%** | 19.62 / 34.97 / 47.17% |
| 8 | Dynamic | 0.000% | 19.55 / 35.07 / 47.22% |
| 10 | Dynamic | 0.000% | 19.44 / 35.00 / 47.17% |

核心 burst loss 同步回到 Dynamic 水平（10 队列 Static 18.66/12.89/6.68 对 Dynamic
18.66/12.90/6.62）。**因此 14.2% 与「端到端反向」都是「10 目的地挤 8 队列」的伪影**，第 7 周 Task
1.3 的原读数也没有被推翻——两者是同一伪影的两个符号。仍然成立的是「核心 burst loss 与端到端不可
互换」（一 burst ≈ n 包）。第 13 周的分析、周会材料、本节 §十九、`research_status.md`、`AGENTS.md`
与论文骨架 §5.4 已按此收窄，第 13 周的原始数字保留为实测记录。

### 结果

| 臂 | 关键读数 |
|---|---|
| S1 | 见上表；队列数 ≥ 目的地数后 Static ≡ Dynamic（两侧都在 ±0.2 pp 内） |
| S2 | 三种粒度最优都在 τ/T_burst ≈ 1：实测归一 n=1 为 1.00（8.16% / 14.93%）、n=3 为 1.02（6.80% / 13.50%）、n=6 在 0.99–1.17 持平（6.68–7.04% / 13.53–13.58%）；无 FDL 参考线只随粒度动 0.5 pp（12.84–13.37% / 18.57–19.13%）；FDL 端到端收益随粒度增大（ρ≈0.40 时 12.6 / 15.1 / 15.1 pp）⇒ **5.3 的有用区不被粒度推翻** |
| S2 交叉核对 | n=3、ratio=1 即 τ=34.36 µs：实测 6.80% / 13.50% 对第 11 周最优 cell 的 6.78% / 13.51%（差 0.02 pp）⇒ τ 推导式求值正确（τ 本身不是 `.sca` 标量，只能这样核对） |
| S3 | τ/T=0.25 收益 +0.55 pp、τ/T=4 为 0.00 pp；与第 11 周 0.5/1/2 的 +0.61/+0.37/+0.04 pp 连成单调衰减，`fdlUsageCount` 在 τ/T=4 反而升到 160,891 ⇒「饱和瓶颈上多等一个 τ 没用」在网格两端都成立 |

### 附带查明：Dynamic 的抢占会截断长 burst

冒烟发现 `numPackets = 6` 的实测 burst 只有 **7322–7361 B（5.12–5.15 包）**，而名义值是 8582 B。
两个探针（0.2 s、单种子、`results/_diag`，只作机制判断）：

| 探针 | 改动 | n=6 实测 | 结论 |
|---|---|---|---|
| `Test-Smoke-BurstLenOffset` | `minOffset/maxOffset` 放宽到 2 ms / 4 ms | 7321.4–7361.1 B（与基准差 ≤2 B） | 偏置预算**不是**原因 |
| `Test-Smoke-BurstLenNoPreempt` | `dispatchMode = 1`（NoPreemption） | **8582.0 B（恰好 6.00 包）**，n=1/3 也精确为 1437.0 / 4295.0 B | 截断来自抢占：mode 0 为腾出队列执行 `forceFlush()`，正在组帧的 burst 被提前发走 |

对主网格无影响（Dynamic + n=3，实测 4211 B 对名义 4295 B，差 2.0%），但 S2 的 τ 轴必须按**实测**
T_burst 归一——n=6 的实测 T_burst 是 58.7 µs 而非 68.66 µs，所以比值列表里加了 0.85，让 n=6 也拿到
一个落在实测比 ≈1.0 的点。若 5.5 要把它写成可交付结论，需把它从 0.2 s 探针升格为 2 s × repeat 5
的小臂（列入研究日 B）。

### 改动文件

新增：`research_reports/2026-09-week14/研究日A-运行记录.md`。
改：`fdl_experiments.ini`（`ExpS-EdgeQueues`、`ExpS-BurstLen`、`ExpS-BurstLen-NoFDL`、
`ExpA-HotspotGrid`）、`fdl_tests.ini`（3 个冒烟 + 2 个机制探针）、`research_reports/2026-09-week13/{分析.md,周会材料.md}`
（机制与结论收窄）、`research_status.md`、`AGENTS.md`、`codex_phase2_tasks.md`、论文骨架 §5.4、
`walkthrough.md`。
本地未入库：`tools/{analyze_w14_sensitivity,check_w14_smoke}.py`、`results/ExpS-*-release/`、
`results/ExpA-HotspotGrid-release/`、`results/_logs/`、`results/_diag/`。

**两个环境坑**：① 本机默认执行策略下 `& .\tools\run-sim.ps1` 会被拒（`UnauthorizedAccess`），须
经 `powershell -ExecutionPolicy Bypass -File` 或以进程级 Bypass 调用；② `params.ini` 把 Cmdenv 文本
日志固定在 `results/simulation_log.txt`，四臂并行必须逐作业覆盖 `--cmdenv-output-file`，否则抢写同
一份日志。
