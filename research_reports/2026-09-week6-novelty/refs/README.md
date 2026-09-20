# 参考文献存档（canonical）

本目录是项目**唯一**文献库，从第 6 周建立，一直用到第 15–16 周写相关工作。不要另建第二份 `refs.bib`，也不要复制 PDF。

- 书目：`refs.bib`
- 能下到的 PDF：`pdf/`
- HTML 摘录：`extracts/`

访问日期未另行注明者均为 **2026-09-14**。

## 登记规则

**文献每读一篇立即登记到 `research_reports/2026-09-week6-novelty/refs/refs.bib`（DOI + 访问日期 + 实际读到的版本）。**

具体做法：

1. 立刻在 `refs.bib` 追加一条：作者、题名、年、DOI（若有）、url、`urldate`（访问日期）、`note`（实际读到：PDF / HTML摘录 / 仅摘要）。
2. 能下的 PDF 放入 `pdf/`，文件名与 bib 的 `note` 一致。
3. 期刊 HTML 全文摘录放入 `extracts/`，并在 `note` 写清路径；这不是出版 PDF。
4. 磁盘上没有的文件，不得在 `note` 写成「已存 PDF」。
5. 未读原文（无 PDF、无全文级 HTML 摘录）不得进入相关工作「必写名单」。

Zhao 2022 的固定口径：`SPIE HTML 摘录，非正式出版 PDF`。不要写成「通读出版 PDF」，也不要写成「被墙未读」。

## 已取得 PDF（`pdf/`）

| 文件 | 文献 |
|---|---|
| `chen-qiao-yu-2004-ieee-network.pdf` | Chen/Qiao/Yu, IEEE Network 2004（Buffalo 教程公开稿） |
| `li-sun-geng-2010-chinacom.pdf` | Li/Sun/Geng, ChinaCom 2010 |
| `legrand-cousin-brochier-2010-wocn-lobs.pdf` | Legrand 等, WOCN 2010（IRISA 作者稿） |
| `legrand-2008-broadnets-lobs-testbed.pdf` | Legrand 等, WOBS/Broadnets 2008（IRISA 作者稿） |
| `teng-rouskas-2005-pnet.pdf` | Teng & Rouskas, Photon. Netw. Commun. 2005（作者站点） |
| `roethig-2026-arxiv-2606.27050.pdf` | Roethig 等, arXiv:2606.27050 |
| `mouammar-2026-arxiv-2605.04829.pdf` | Mouammar 等, arXiv:2605.04829 |

磁盘上目前 **7** 个 PDF。没有 Zhao 2022、ADI 2025、CN108809719A 的出版/全文 PDF。

## HTML 摘录（`extracts/`，非正式出版 PDF）

| 文件 | 说明 |
|---|---|
| `zhao-2022-oe-061106-spie-html-2026-09-11.txt` | 2026-09-11 首次抓到的 SPIE HTML 正文 |
| `zhao-2022-oe-061106-spie-html-2026-09-14.txt` | 2026-09-14 复核；与 09-11 同文。**SPIE HTML 摘录，非正式出版 PDF** |
| `adi-2025-adi.0097-spj-html-2026-09-14.txt` | ADI 2025 期刊 HTML；卷 6、论文号 0097、无起止页；PDF 接口 403 |
| `legrand-2010-wocn-hal-html-2026-09-14.txt` | HAL 页文本（另已存 IRISA 作者稿 PDF） |
| `legrand-2008-wobs-irisa-text-2026-09-14.txt` | 2008 试验台 IRISA 页文本 |

## 未取得 PDF（bib 里已标 note）

- Zhao 2022 出版 PDF（已读 SPIE HTML 摘录，不是被墙未读）
- ADI 2025 出版 PDF（已读期刊 HTML）
- Qiao 2000 LOBS（IEEE 付费；作者旧链 404）
- Lee 2006 LNCS（Springer 付费）
- CN108809719A 专利全文（公开说明级）
- 2023–2026 地面 OBS/FDL 期刊、中文期刊：本轮只核摘要 / 公开说明

写相关工作前：必写名单只收「已读 PDF 或已存全文级 HTML 摘录」的条目。Lee 2006、Qiao 2000 不进必写。Teng 2005 已有 PDF，仅作 JIT/JET/Horizon 经典对照，可选。
