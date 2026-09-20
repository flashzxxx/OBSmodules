# 研究资料归档索引

本目录集中保存研究计划、架构图、周会材料和汇报制作过程文件，避免在项目根目录散放阶段性产物。

| 目录 | 内容 |
|---|---|
| `00-project-management/` | 长期执行计划与任务拆解 |
| `architecture/` | 可交互架构 HTML 及其源 JSON |
| `2026-08-plan-review/` | 2026-08-14 计划修订、问题清单与平台重建 walkthrough |
| `2026-08-direction-review/` | 研究方向确认汇报、渲染图和制作过程文件 |
| `2026-08-week1-baseline/` | 第 1 周仓库基线与周会证据 |
| `2026-08-week2-release/` | 第 2 周 release 链接与静态时序复核 |
| `2026-08-week3-fdl-tests/` | 第 3 周 FDL 三分支功能测试分析 |
| `2026-08-week4-compatibility/` | 第 4 周 `Test-FDL-Compatibility` 与 8-13 `ICMPTest`、pre-FDL `3530c2f` 对照 |
| `2026-09-week5-calibrate/` | 第 5 周负载标定 |
| `2026-09-week6-novelty/` | 第 6 周文献定位（go/no-go）；正式三份 + 检索记录 + D1；文献库在其下 `refs/` |
| `refs/` | 指向 canonical 书目的短指针，不复制 PDF；真正的 `refs.bib` 在 `2026-09-week6-novelty/refs/` |

根目录只保留需要固定入口的 `AGENTS.md`、`research_status.md` 和 `walkthrough.md`。`_working/` 下的 manifest、QA JSON 和 inspect 文件属于历史构建记录，其中可能保留归档前的绝对路径；可重新执行的 `.mjs` 脚本已更新为归档后路径。
