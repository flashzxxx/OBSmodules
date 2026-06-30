# OBS 仿真项目新手入门导读

> **作者视角**：本文档从"数据流"的角度出发，用通俗易懂的语言帮助你理解 OBS（光突发交换）仿真项目的工作原理，而非深入代码细节。

---

## 📁 第一步：项目地图（Structure Overview）

### 核心目录结构

```
OBSmodules/
├── src/                          # 源代码目录（核心逻辑实现）
│   ├── EdgeNode/                 # 边缘节点模块（数据进出口）
│   │   ├── *.ned                 # 网络拓扑定义文件
│   │   └── *.cc/*.h              # C++ 实现文件
│   ├── CoreNode/                 # 核心节点模块（光交换核心）
│   │   ├── *.ned                 # 网络拓扑定义文件
│   │   └── *.cc/*.h              # C++ 实现文件
│   └── messages/                 # 消息定义（数据包结构）
│       └── *.msg                 # OMNeT++ 消息定义
│
├── Examples/                     # 示例仿真场景
│   ├── TreeTopologyOBS/          # 🎯 推荐入门示例（树形拓扑）
│   │   ├── TreeTopologyOBS.ned   # 网络拓扑定义
│   │   ├── omnetpp.ini           # 仿真配置文件（主配置）
│   │   ├── timeout.ini           # Burst 超时参数
│   │   ├── params.ini            # 其他参数
│   │   └── run.ini               # 运行配置
│   └── [其他示例...]
│
├── test/                         # 单元测试目录
└── INSTALL                       # 安装说明
```

### 文件类型说明

| 文件类型 | 作用 | 示例 |
|---------|------|------|
| **`.ned`** | 定义网络拓扑结构（节点、连接、参数） | `TreeTopologyOBS.ned` |
| **`.cc/.h`** | 实现具体的业务逻辑（C++ 代码） | `OBS_PacketBurstifier.cc` |
| **`.ini`** | 仿真配置文件（参数设置、运行时长等） | `omnetpp.ini` |
| **`.msg`** | 定义消息结构（类似数据包格式） | `OBS_Burst.msg` |

### 🚀 快速开始：运行你的第一个仿真

**推荐路径**：`Examples/TreeTopologyOBS/`

1. **打开配置文件**：`omnetpp.ini`
2. **关键参数**：
   - `sim-time-limit = 0.5s`：仿真运行 0.5 秒
   - `network = TreeTopologyOBS`：使用树形拓扑网络
3. **运行方式**：
   - 在 OMNeT++ IDE 中右键 `omnetpp.ini` → 选择 "Run As → OMNeT++ Simulation"
   - 或使用命令行：`opp_run -u Cmdenv -f omnetpp.ini`

---

## 🌊 第二步：数据的生命周期（The Flow Logic）

> **核心思想**：在 OBS 网络中，数据包（IP Packet）会被组装成"突发包"（Burst），然后通过光网络传输。控制信号（BHP）会提前发送，为数据预留资源。

### 完整流程图（文字版）

```
[主机 Host] 
    ↓ (发送 IP 数据包)
[边缘节点 Edge Node]
    ├─→ [PacketBurstifier] ──────┐ (组装 Burst)
    │                             ↓
    │                      [BurstSender] 
    │                             ├─→ BCP (控制包，提前发送)
    │                             └─→ Data Burst (数据包，延迟发送)
    ↓
[核心节点 Core Node 1]
    ├─→ [CoreInput] (分离控制/数据)
    │       ├─→ BCP → [OE Converter] → [CoreControlLogic]
    │       │                               ↓ (查路由表 + 调度)
    │       │                          [OXC 光交叉连接]
    │       └─→ Data Burst ──────────→ [OXC] (光交换)
    ↓
[核心节点 Core Node 2, 3...] (重复上述流程)
    ↓
[边缘节点 Edge Node (目标)]
    └─→ [BurstDisassembler] (拆包)
            ↓ (还原 IP 数据包)
        [主机 Host]
```

---

### 详细流程讲解

#### 1️⃣ **边缘节点（Edge Node）：数据的"打包工厂"**

**场景**：想象你在寄快递，需要把零散的物品打包成一个大箱子。

- **数据进入点**：主机（Host）通过以太网发送 IP 数据包到边缘节点
- **核心模块**：`OBS_PacketBurstifier`（包突发器）
  
**工作流程**：
1. **收集数据包**：像一个"缓冲区"，不断接收来自主机的 IP 数据包
2. **触发组装条件**（满足任一即触发）：
   - ⏱️ **超时**：等待时间达到 `timeout` 参数（例如 0.001 秒）
   - 📦 **大小满**：累积数据达到 `maxSize`（例如 125MB）
   - 🔢 **数量满**：数据包数量达到 `numPackets`
3. **生成 Burst**：将所有缓存的数据包封装成一个 `OBS_Burst` 对象
4. **发送到 Sender**：传递给 `OBS_BurstSender` 模块

**关键 C++ 类**：
- `OBS_PacketBurstifier.cc`：实现组装逻辑
- 核心方法：`handleMessage()`（接收数据包）、`assembleBurst()`（组装突发包）

---

#### 2️⃣ **信令分离：控制包（BCP）与数据包（Data Burst）**

**场景**：就像高速公路上，警车会提前开道，为后面的车队预留车道。

- **BurstSender 的任务**：
  1. **生成控制包（BCP）**：
     - 包含：目标地址、Burst 大小、到达时间差（Offset）
     - **提前发送**：比数据包早 `minOffset` 到 `maxOffset` 时间（例如 0.01-0.1 秒）
  2. **发送数据包（Data Burst）**：
     - 延迟发送，确保 BCP 先到达核心节点

**关键参数**（在 `omnetpp.ini` 中配置）：
```ini
**.packetBurstifier[*].minOffset = 0.00001024s  # 最小偏置时间
**.packetBurstifier[*].maxOffset = 0.0001024s   # 最大偏置时间
**.sender.BCPSize = 256B                        # BCP 大小
```

**关键 C++ 类**：
- `OBS_BurstSender.cc`：负责发送 BCP 和 Data Burst
- 核心方法：`handleMessage()`（处理 Burst 并分离发送）

---

#### 3️⃣ **核心节点（Core Node）：光交换的"调度中心"**

**场景**：机场的空中交通管制，需要为每架飞机分配跑道和起降时间。

##### 当 BCP 到达核心节点时：

1. **接收与分离**（`OBS_CoreInput`）：
   - 识别这是控制包（通过波长/通道编号）
   - 转发到 `OE Converter`（光电转换器）

2. **电域处理**（`OBS_CoreControlLogic`）：
   - **解析 BCP**：提取目标地址、Burst 大小、到达时间
   - **查询路由表**（`OBS_CoreRoutingTable`）：
     - 输入：来源端口、波长、目标标签
     - 输出：出端口、出波长、新标签
   - **资源调度**（核心算法！）：
     - 检查 `GatesHorizon`（波长占用时间表）
     - 如果目标波长空闲 → 预留资源
     - 如果占用 → **丢弃 Burst**（竞争失败）
   - **配置光交叉连接（OXC）**：
     - 在 Burst 到达前设置光开关
     - 在 Burst 离开后释放光开关

3. **数据转发**（`OBS_OpticalCrossConnect`）：
   - Data Burst 到达时，直接通过预先配置好的光路转发
   - **全光域**：无需光电转换，速度极快

**关键 C++ 类**：
- `OBS_CoreInput.cc`：分离控制/数据通道
- `OBS_CoreControlLogic.cc`：**核心调度逻辑**（JET 协议实现）
- `OBS_OpticalCrossConnect.cc`：光交叉连接矩阵

**关键时间计算**（在 `OBS_CoreControlLogic.cc` 中）：
```cpp
// 第 127-131 行
simtime_t OXCConnectTime = burstArrival - guardTime/2;      // 光开关连接时间
simtime_t OXCChannelIsFree = burstArrival + burstLength/dataRate;  // 通道释放时间
simtime_t OXCDisconnectTime = OXCChannelIsFree + guardTime/4;      // 光开关断开时间
simtime_t newHorizon = OXCChannelIsFree + 3*guardTime/4;            // 新的占用时间
```

---

#### 4️⃣ **接收端：Burst 的"拆包还原"**

**场景**：快递到达后，拆开箱子取出里面的物品。

- **核心模块**：`OBS_BurstDisassembler`（突发包拆解器）

**工作流程**：
1. **接收 Burst 开始标记**（`kind = 1`）：
   - 将 Burst 放入接收队列 `receivedBursts`
2. **接收 Burst 结束标记**（`kind = 2`）：
   - 从队列中找到对应的 Burst（通过 `burstifierId` 和 `numSeq` 匹配）
   - 提取所有 IP 数据包
   - 逐个发送到网络层
3. **清理**：删除 Burst 对象，释放内存

**关键 C++ 类**：
- `OBS_BurstDisassembler.cc`：实现拆包逻辑
- 核心方法：`handleMessage()`（处理 Burst 开始/结束）

---

## 🔬 第三步：核心模块深挖（Key Modules for Research）

### A. 路由与调度（JET/JIT 协议）

**如果你想修改偏置时间（Offset Time）或调度协议**：

#### 📍 修改位置：

1. **偏置时间配置**：
   - **文件**：`Examples/TreeTopologyOBS/omnetpp.ini`
   - **参数**：
     ```ini
     **.packetBurstifier[*].minOffset = 0.00001024s
     **.packetBurstifier[*].maxOffset = 0.0001024s
     ```

2. **调度算法实现**：
   - **文件**：`src/CoreNode/OBS_CoreControlLogic.cc`
   - **关键函数**：`handleMessage()`（第 76-217 行）
   - **核心逻辑**：
     - 第 144-158 行：**波长选择算法**（当前使用"最近可用波长"）
     - 第 163-174 行：**竞争检测**（检查波长是否被占用）

#### 🛠️ 修改示例：实现 JIT（Just-In-Time）协议

**JET vs JIT 区别**：
- **JET**（当前实现）：BCP 提前发送，预留资源
- **JIT**：BCP 和 Burst 同时到达，实时调度

**修改步骤**：
1. 在 `OBS_BurstSender.cc` 中修改 BCP 发送时间：
   ```cpp
   // 原代码：BCP 提前发送
   sendDelayed(bcp, 0, "obsOut", lambda);
   sendDelayed(burst, offset, "obsOut", lambda);
   
   // JIT 修改：同时发送
   sendDelayed(bcp, offset, "obsOut", lambda);
   sendDelayed(burst, offset, "obsOut", lambda);
   ```

2. 在 `OBS_CoreControlLogic.cc` 中调整调度逻辑：
   - 修改第 127 行的 `OXCConnectTime` 计算
   - 减少 `guardTime` 的使用

---

### B. 竞争解决（Contention Resolution）

**如果你想加入 FDL（光纤延迟线）或波长转换逻辑**：

#### 📍 修改位置：

1. **波长转换配置**：
   - **文件**：`Examples/TreeTopologyOBS/omnetpp.ini`
   - **当前设置**：
     ```ini
     **.core*.inputColours = ""   # 空字符串 = 全波长转换
     **.core*.outputColours = ""
     ```
   - **限制波长转换**：指定颜色映射（例如 `"0 1 2 | 3 4 5"`）

2. **FDL 实现位置**：
   - **文件**：`src/CoreNode/OBS_CoreControlLogic.cc`
   - **插入点**：第 148-158 行（竞争检测失败后）
   - **逻辑**：
     ```
     if (lambda == -1) {  // 当前：直接丢弃
         // 新增：尝试 FDL 延迟
         lambda = tryFDL(outPort, burstArrival, burstLength);
         if (lambda == -1) {
             dropCounter++;  // FDL 也失败，才丢弃
             return;
         }
     }
     ```

3. **需要新增的模块**：
   - 创建 `OBS_FDL.cc/.h`：实现延迟队列
   - 在 `OBS_CoreNode.ned` 中添加 FDL 子模块

#### 🛠️ 修改示例：添加简单的 FDL 逻辑

**步骤**：
1. 在 `OBS_CoreControlLogic.h` 中添加 FDL 队列：
   ```cpp
   std::queue<OBS_BurstControlPacket*> fdlQueue[MAX_PORTS][MAX_LAMBDAS];
   ```

2. 在 `OBS_CoreControlLogic.cc` 的 `handleMessage()` 中：
   ```cpp
   // 第 148 行后插入
   if (lambda == -1) {
       // 尝试延迟 Burst
       simtime_t delayTime = guardTime * 2;  // 延迟 2 个保护时间
       lambda = gatesHorizon->findNearestLambda(outPort, burstArrival + delayTime);
       
       if (lambda != -1) {
           // 延迟成功，重新调度
           bcp->setBurstArrivalDelta(arrivalDelta + delayTime);
           scheduleAt(simTime() + delayTime, bcp);
           return;
       }
       // 延迟失败，丢弃
       dropCounter++;
       delete msg;
       return;
   }
   ```

---

### C. 关键数据结构

| 数据结构 | 文件 | 作用 |
|---------|------|------|
| `OBS_Burst` | `src/messages/OBS_Burst.msg` | 存储突发包（包含多个 IP 数据包） |
| `OBS_BurstControlPacket` | `src/messages/OBS_BurstControlPacket.msg` | 控制包（BCP），包含路由信息 |
| `OBS_CoreRoutingTable` | `src/CoreNode/OBS_CoreRoutingTable.cc` | 路由表（查询出端口/波长） |
| `OBS_CoreOutputHorizon` | `src/CoreNode/OBS_CoreOutputHorizon.cc` | 波长占用时间表（调度核心） |

---

## ⚠️ 第四步：新手避坑指南

### 坑 1：OMNeT++ 版本兼容性

**问题**：项目基于 **OMNeT++ 4.2** 和 **INET 2.0.0**（2012 年版本），与最新版本不兼容。

**症状**：
- 编译错误：`undefined reference to 'IPv4Datagram'`
- 运行错误：`Cannot find module type 'StandardHost'`

**解决方案**：
1. **推荐**：安装 OMNeT++ 4.2 和 INET 2.0.0（可从官网下载历史版本）
2. **升级代码**（高级）：
   - 将 `IPv4Datagram` 替换为 `Ipv4Header`
   - 更新 `.ned` 文件中的模块路径（例如 `inet.nodes.inet.StandardHost` → `inet.node.inet.StandardHost`）

**检查方法**：
```bash
# 查看 OMNeT++ 版本
opp_run --version

# 查看 INET 版本
cat <INET_DIR>/Version
```

---

### 坑 2：路由表文件缺失

**问题**：运行仿真时报错 `Cannot open routing file: core1Route.dat`

**原因**：`.ned` 文件中引用了路由表文件，但文件不存在。

**解决方案**：
1. **检查路径**：
   - 路由表文件应放在仿真目录（例如 `Examples/TreeTopologyOBS/`）
   - 文件名在 `.ned` 中定义（例如 `core1Route.dat`）

2. **路由表格式**（示例）：
   ```
   # 格式：入端口 入波长 目标标签 出端口 出波长 出标签
   0 0 2 1 0 2
   0 0 3 2 0 3
   1 0 1 0 0 1
   ```

3. **快速生成**：
   - 使用项目自带的示例文件（`Examples/TreeTopologyOBS/core*.dat`）
   - 或运行 `utils/` 目录下的脚本生成

---

### 坑 3：仿真时间过短，看不到结果

**问题**：仿真运行完成，但没有输出统计数据。

**原因**：`sim-time-limit` 设置过短，数据包还未到达目的地。

**解决方案**：
1. 在 `omnetpp.ini` 中增加仿真时间：
   ```ini
   sim-time-limit = 5s  # 从 0.5s 改为 5s
   ```

2. 检查日志：
   - 查看 `results/` 目录下的 `.sca` 和 `.vec` 文件
   - 使用 OMNeT++ IDE 的 "Analysis Tool" 查看统计图表

---

### 坑 4：编译时找不到 INET 库

**问题**：编译报错 `fatal error: IPv4Datagram.h: No such file or directory`

**解决方案**：
1. **检查项目引用**：
   - 右键项目 → Properties → Project References
   - 勾选 INET 项目

2. **检查 Makefile**：
   - 确保 `-I` 参数包含 INET 的 `src` 目录
   - 例如：`-I../../inet/src`

3. **重新生成 Makefile**：
   ```bash
   opp_makemake -f --deep -I../../inet/src -L../../inet/out/gcc-debug/src -linet
   ```

---

## 📚 总结：数据流视角的核心要点

1. **数据进入**：主机 → 边缘节点 → `PacketBurstifier`（组装）
2. **信令分离**：`BurstSender` 分别发送 BCP（提前）和 Data Burst（延迟）
3. **核心调度**：`CoreControlLogic` 根据 BCP 预留资源，配置 OXC
4. **光交换**：Data Burst 通过预配置的光路转发（全光域）
5. **数据还原**：边缘节点 → `BurstDisassembler`（拆包）→ 主机

---

## 🎯 下一步建议

1. **运行示例**：先跑通 `TreeTopologyOBS`，观察仿真过程
2. **修改参数**：调整 `timeout`、`maxSize`、`minOffset` 等参数，观察影响
3. **阅读代码**：重点关注 `OBS_CoreControlLogic.cc` 的调度逻辑
4. **实现算法**：尝试添加 FDL 或修改波长选择策略

---

**祝你仿真顺利！如有疑问，欢迎随时提问。** 🚀
