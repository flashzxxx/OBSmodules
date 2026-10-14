# ============================================================
# 科研PPT性能对比：2×2 四宫格
# 行：FlowScaling / LoadIntensity 两个实验
# 列：丢包率 / 信令开销 两个指标
# ============================================================
library(ggplot2)
library(grid)

strats <- c("Dynamic", "No-Preemption", "Round-Robin", "Static")
colors <- c(
    "Dynamic" = "#000000", "No-Preemption" = "#E67E22",
    "Round-Robin" = "#8E44AD", "Static" = "#2980B9"
)
shapes <- c(
    "Dynamic" = 16, "No-Preemption" = 17,
    "Round-Robin" = 2, "Static" = 18
)
ltypes <- c(
    "Dynamic" = "solid", "No-Preemption" = "dashed",
    "Round-Robin" = "dotted", "Static" = "longdash"
)

# ============ Experiment 1: FlowScaling ============
flows <- c(2, 4, 6, 8, 10, 12)

fs <- data.frame(
    X = rep(flows, 4),
    Strategy = factor(rep(strats, each = 6), levels = strats),
    Pb = c(
        0, 0, 0, 0, 0, 0, # Dynamic: 零丢包
        0, 0, 0, 0, 0.050, 0.083, # NP: 超载后丢包
        0, 0, 0, 0, 0, 0, # RR: 不丢包
        0, 0, 0, 0, 0.125, 0.166 # Static: 丢包最严重
    ),
    So = c(
        0.20, 0.20, 0.20, 0.20, 0.32, 0.40, # Dynamic: 超载后适度上升
        0.20, 0.20, 0.20, 0.20, 0.15, 0.14, # NP: 丢包导致So下降
        0.50, 0.33, 1.00, 0.20, 1.00, 1.00, # RR: 极不稳定
        0.20, 0.20, 0.20, 0.20, 0.18, 0.16 # Static: 类似NP
    )
)

# ============ Experiment 2: LoadIntensity ============
# 10 flows fixed, variable send rate
arr_rate <- c(10, 50, 200, 500, 1000)
arr_labels <- c("10", "50", "200", "500", "1000")

li <- data.frame(
    X = rep(arr_rate, 4),
    Strategy = factor(rep(strats, each = 5), levels = strats),
    Pb = c(
        0, 0, 0, 0, 0, # Dynamic: 零丢包
        0, 0, 0, 0.04, 0.07, # NP: 高速时丢包
        0, 0, 0, 0, 0, # RR: 不丢包
        0, 0, 0.02, 0.08, 0.12 # Static: 高速时丢包更多
    ),
    So = c(
        0.25, 0.22, 0.20, 0.28, 0.35, # Dynamic: 适度上升
        0.25, 0.22, 0.20, 0.18, 0.15, # NP: 下降(丢包)
        1.00, 0.80, 0.60, 0.80, 1.00, # RR: 始终高
        0.25, 0.22, 0.20, 0.19, 0.16 # Static: 类似NP
    )
)

# ============ Theme (compact for 2x2 grid) ============
ieee <- function(leg_x = 0.03, leg_y = 0.97, leg_just = c(0, 1)) {
    theme(
        plot.background = element_rect(fill = "#E8E8E0", color = NA),
        panel.background = element_rect(fill = "white", color = "black", linewidth = 0.8),
        panel.grid.major = element_line(color = "gray80", linewidth = 0.25),
        panel.grid.minor = element_line(color = "gray92", linewidth = 0.12),
        plot.title = element_text(size = 10, face = "bold", hjust = 0.5),
        axis.title = element_text(size = 9),
        axis.text = element_text(size = 7.5, color = "black"),
        axis.ticks = element_line(color = "black", linewidth = 0.3),
        plot.margin = margin(4, 8, 4, 4),
        legend.background = element_rect(fill = "white", color = "black", linewidth = 0.3),
        legend.key = element_rect(fill = "white"),
        legend.key.size = unit(0.3, "cm"),
        legend.key.width = unit(0.8, "cm"),
        legend.title = element_blank(),
        legend.text = element_text(size = 6.5),
        legend.margin = margin(1, 3, 1, 3),
        legend.position = c(leg_x, leg_y),
        legend.justification = leg_just
    )
}

common_aes <- function(data, x, y) {
    list(
        geom_line(linewidth = 0.8),
        geom_point(size = 2.2),
        scale_color_manual(values = colors),
        scale_shape_manual(values = shapes),
        scale_linetype_manual(values = ltypes)
    )
}

# ======== (a) FlowScaling — 丢包率 ========
p_a <- ggplot(fs, aes(x = X, y = Pb, color = Strategy, shape = Strategy, linetype = Strategy)) +
    geom_line(linewidth = 0.8) +
    geom_point(size = 2.2) +
    geom_vline(xintercept = 8, linetype = "dashed", color = "gray50", linewidth = 0.3) +
    annotate("rect",
        xmin = 8, xmax = 12.5, ymin = -0.003, ymax = 0.19,
        fill = "#FFCCCC", alpha = 0.15
    ) +
    annotate("text", x = 10, y = 0.185, label = "超载区", size = 2.8, color = "#C0392B") +
    annotate("text", x = 7.7, y = 0.185, label = "Q=8", size = 2.5, color = "gray40", hjust = 1) +
    scale_color_manual(values = colors) +
    scale_shape_manual(values = shapes) +
    scale_linetype_manual(values = ltypes) +
    scale_x_continuous(breaks = flows) +
    scale_y_continuous(
        limits = c(-0.003, 0.19), breaks = seq(0, 0.18, 0.03),
        labels = function(x) sprintf("%.0f%%", x * 100)
    ) +
    labs(title = "(a) 丢包率 vs 并发流数", x = "并发数据流数量 F", y = "丢包率") +
    ieee(0.03, 0.55)

# ======== (b) FlowScaling — 信令开销 ========
p_b <- ggplot(fs, aes(x = X, y = So, color = Strategy, shape = Strategy, linetype = Strategy)) +
    geom_line(linewidth = 0.8) +
    geom_point(size = 2.2) +
    geom_vline(xintercept = 8, linetype = "dashed", color = "gray50", linewidth = 0.3) +
    annotate("rect",
        xmin = 8, xmax = 12.5, ymin = -0.01, ymax = 1.08,
        fill = "#FFCCCC", alpha = 0.15
    ) +
    scale_color_manual(values = colors) +
    scale_shape_manual(values = shapes) +
    scale_linetype_manual(values = ltypes) +
    scale_x_continuous(breaks = flows) +
    scale_y_continuous(limits = c(0, 1.08), breaks = seq(0, 1.0, 0.2)) +
    labs(title = "(b) 信令开销 vs 并发流数", x = "并发数据流数量 F", y = "信令开销比") +
    ieee(0.03, 0.97)

# ======== (c) LoadIntensity — 丢包率 ========
p_c <- ggplot(li, aes(x = X, y = Pb, color = Strategy, shape = Strategy, linetype = Strategy)) +
    geom_line(linewidth = 0.8) +
    geom_point(size = 2.2) +
    annotate("rect",
        xmin = 150, xmax = 1100, ymin = -0.002, ymax = 0.135,
        fill = "#FFCCCC", alpha = 0.15
    ) +
    annotate("text", x = 600, y = 0.13, label = "高速区", size = 2.8, color = "#C0392B") +
    scale_color_manual(values = colors) +
    scale_shape_manual(values = shapes) +
    scale_linetype_manual(values = ltypes) +
    scale_x_log10(breaks = arr_rate, labels = arr_labels) +
    scale_y_continuous(
        limits = c(-0.002, 0.14), breaks = seq(0, 0.12, 0.02),
        labels = function(x) sprintf("%.0f%%", x * 100)
    ) +
    labs(title = "(c) 丢包率 vs 发包速率", x = "发包速率 (packets/s)", y = "丢包率") +
    ieee(0.03, 0.55)

# ======== (d) LoadIntensity — 信令开销 ========
p_d <- ggplot(li, aes(x = X, y = So, color = Strategy, shape = Strategy, linetype = Strategy)) +
    geom_line(linewidth = 0.8) +
    geom_point(size = 2.2) +
    annotate("rect",
        xmin = 150, xmax = 1100, ymin = -0.01, ymax = 1.08,
        fill = "#FFCCCC", alpha = 0.15
    ) +
    scale_color_manual(values = colors) +
    scale_shape_manual(values = shapes) +
    scale_linetype_manual(values = ltypes) +
    scale_x_log10(breaks = arr_rate, labels = arr_labels) +
    scale_y_continuous(limits = c(0, 1.08), breaks = seq(0, 1.0, 0.2)) +
    labs(title = "(d) 信令开销 vs 发包速率", x = "发包速率 (packets/s)", y = "信令开销比") +
    ieee(0.03, 0.40)

# ======== Save 4 separate figures ========
ggsave("results/fig_ppt_a_flowscaling_pb.png", p_a, width = 6, height = 4.5, dpi = 200)
cat("Saved: fig_ppt_a_flowscaling_pb.png\n")

ggsave("results/fig_ppt_b_flowscaling_so.png", p_b, width = 6, height = 4.5, dpi = 200)
cat("Saved: fig_ppt_b_flowscaling_so.png\n")

ggsave("results/fig_ppt_c_loadintensity_pb.png", p_c, width = 6, height = 4.5, dpi = 200)
cat("Saved: fig_ppt_c_loadintensity_pb.png\n")

ggsave("results/fig_ppt_d_loadintensity_so.png", p_d, width = 6, height = 4.5, dpi = 200)
cat("Saved: fig_ppt_d_loadintensity_so.png\n")
