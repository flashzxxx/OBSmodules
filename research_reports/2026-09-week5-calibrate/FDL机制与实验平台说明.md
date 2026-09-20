# FDL 机制与实验平台说明

> 日期：2026-09-09  
> 读者：需要对照源码把事情搞懂的人（也可直接抽段落做汇报）  
> 范围：核心节点回环 FDL 怎么工作；第 5 周实验平台标定改了什么、跑出了什么  
> 配套：`walkthrough.md`（当周变更清单）、同目录 `分析.md`（标定数字明细）

本文按「先建立图景 → 再对代码」写。遇到专有名词，第一次出现会用白话解释。代码都是仓库里当前实现，不是示意伪代码。

---

## 1. 先建立图景

### 1.1 这个项目在仿真什么

卫星之间用光链路传数据。我们用的是 OBS（光突发交换）：边缘节点把若干 IP 包聚成一段较长的光信号，叫做**突发（burst）**。突发前面先走一个很小的控制包，叫做 **BCP**。核心节点读 BCP，提前把光开关（OXC）拨好，突发到达时尽量在光域直接穿过去，不做光-电-光（OEO）。

现有 OBS 并不「全程全光」：突发可以光域转发，但 BCP 在每个核心节点仍要光电转换、电子解析。真正的全光控制（光标签、发出时 offset=0）是更后面的事。当前阶段的 FDL，解决的是更窄的问题：

> 单波长星间链路上，两个突发抢同一根出纤时，能不能在光域把后到的那个推迟一小段时间再送，而不是立刻丢掉。

### 1.2 当前 FDL 是什么、不是什么

当前实现的是**回环 FDL**：核心节点的光交叉连接多一对门，中间接一根延迟为 τ 的光纤。冲突且条件满足时，突发被接到这对门上，在光纤里走 τ，再从另一头出来，第二次经过光开关，送到真正的出口。

它**不是**：

- 电缓存或 RAM（那会做 OEO，违背数据面全光）
- 多次回环（最多一次）
- 「offset=0 时把突发暂存起来等标签读完」的那种器件

后一种叫做**输入 FDL**，应放在每个入口、OXC 之前、无条件延迟。现有回环要提前预约光开关，本身要求 BCP 比突发先到，也就是 offset > 0。所以回环 FDL 能回答「单波长下 τ 怎么选、FDL 够不够」，不能单独证明 offset→0。

### 1.3 最近工作卡在哪一步

| 阶段 | 做了什么 | 状态 |
|---|---|---|
| 边缘四种调度 | Dynamic / NoPreemption / RoundRobin / Static | 已完成，申请专利；默认不改 |
| FDL 模块 + 调度 | 直通 / 回环一次 / 丢掉 | 代码已有；第 3 周三分支测过；第 4 周关 FDL 的 ICMP 回归有条件通过 |
| **实验平台标定（第 5 周，最近）** | 单波长、十星互发、负载用仿真测出来 | **已跑完 14 次**；默认间隔写成 35.9 µs |
| 实验 A：扫 τ | 开 FDL，看丢包随 τ 怎么变 | 未开始 |
| 实验 B：边缘 × FDL | 四种组合对照 | 未开始 |

最近改的**不是**「FDL 什么时候进回环」这段判断，而是：实验场景、结束时多记的波长数、标定扫描、用结果反推发送间隔。

---

## 2. 名词

| 说法 | 白话 |
|---|---|
| 星间链路 / ISL | 卫星和卫星之间的光纤 |
| W=1 / 单波长 | 每条星间链路同一时刻只有 1 个**数据**波长。NED 里还会多建 1 根光纤传 BCP，那是控制信道，不算第二个数据波长 |
| 突发 / burst | 边缘聚好的一大段光数据 |
| BCP | 突发控制包，比突发先走，告诉核心节点路由和到达时刻 |
| offset | BCP 比突发提前的时间。核心节点靠这段时间读 BCP、拨开关 |
| τ（tau） | FDL 光纤的延迟。代码里是 `fdlDelayTime` |
| horizon | 「这根波长要忙到什么时刻」。新突发到达时刻如果还早于 horizon，就冲突 |
| ρ（rho） | 最忙那条星间**数据**波长被突发占用的时间比例。0.4 ≈ 占用 40% |
| 发送间隔 | 每颗星 UDP 应用平均隔多久发一个包。间隔越小，网上越忙 |
| run | OMNeT++ 用一组参数完整跑完一次。窗口标题 `#0` 就是第 0 次 |
| `.sca` | 仿真结束写出的标量文件（利用率、丢包数等） |
| seed / 种子 | 随机数发生器的初值。换种子，指数发包过程会不同 |
| `useFDL` | 核心节点开关。`false`：冲突就丢；`true`：条件满足时回环一次 |

---

## 3. 一个突发在核心节点怎么走

```text
BCP 先到 ControlLogic
        │
        ├─ 查路由：应该从几号出口出去
        ├─ 看该出口的 horizon：到达时刻空不空
        │
        ├─ 空 ──────────────────────────────► 直通
        │                                      预约一次 OXC：入口 → 目标出口
        │
        ├─ 忙，且 useFDL，且 waitTime≤τ，
        │     且 FDL 入口空 ────────────────► 回环一次
        │                                      预约两段 OXC：入口→FDL，再 FDL→目标出口
        │                                      突发在光纤里被推迟 τ
        │
        └─ 否则 ────────────────────────────► 丢掉，记竞争丢包
```

光本身不进控制模块。控制模块只预约 OXC；FDL 模块对进来的消息做 `sendDelayed(..., τ)`。

---

## 4. 核心节点里有哪些模块

文件：`src/CoreNode/OBS_CoreNode.ned`。

一颗卫星的核心节点大致是：

| 子模块 | 作用 |
|---|---|
| Input / Output | 把「光纤 + 波长」映射成内部门号 |
| ControlUnit / ControlLogic | 读 BCP，做调度，预约 OXC |
| GatesHorizon | 记下每根出纤、每个波长忙到何时；FDL 入口单独占一格 |
| OXC | 光交叉连接，按预约把入口接到某个出口 |
| fdl | 延迟线，只有 `in` / `out` 两个门 |

节点参数里和 FDL 直接相关的是：

```34:35:src/CoreNode/OBS_CoreNode.ned
        bool useFDL = default(false);
        double fdlDelayTime @unit(s) = default(10us);
```

延迟线子模块把 `fdlDelayTime` 传给器件：

```77:80:src/CoreNode/OBS_CoreNode.ned
        fdl: OBS_FiberDelayLine {
            parameters:
                delayTime = fdlDelayTime;
```

OXC 的门比普通端口多 1，多出来的就是回环口。连接是：

```109:111:src/CoreNode/OBS_CoreNode.ned
        //Connect the fiber delay line loopback
        OXC.out[sizeof(out)-numPorts] --> fdl.in;
        fdl.out --> OXC.in[sizeof(in)-numPorts];
```

读法：OXC 最后一个出口接到 FDL 入口；FDL 出口接到 OXC 最后一个入口。调度里用 `oxc->gateSize("out") - 1` 找到这对门。

---

## 5. 机制核心代码

### 5.1 延迟线：到了就推迟 τ

文件：`src/CoreNode/OBS_FiberDelayLine.cc`。

```25:38:src/CoreNode/OBS_FiberDelayLine.cc
void OBS_FiberDelayLine::initialize() {
    delayTime = par("delayTime");
    fdlUsageCount = 0;
    WATCH(fdlUsageCount);
}

void OBS_FiberDelayLine::handleMessage(cMessage *msg) {
    fdlUsageCount++;
    sendDelayed(msg, delayTime, "out");
}

void OBS_FiberDelayLine::finish() {
    recordScalar("FDL usage count", fdlUsageCount);
}
```

要点：

- 没有队列，没有「满了怎么办」。进来一个，就延迟 `delayTime` 再从 `out` 送走。
- 多个突发只要**进入 FDL 的时刻不重叠**，可以同时在光纤里传播。所以 FDL 的 horizon 记的是**入口占用**，更新时不加 τ。定长延迟会保持间隔，出口不会因为「里面已经有人」而挤在一起。
- `FDL usage count` 是器件计数：一个突发的头、尾各过一次，常常是调度模块 `fdlUsageCount` 的 2 倍。第 3 周 Basic 测试里 sat2 调度记 1、器件记 2，就是这个原因。

### 5.2 horizon：每根波长忙到何时

文件：`src/CoreNode/OBS_CoreOutputHorizon.cc`。

初始化时按 ini 里的 `"1 1 1"` 这种串，给每个普通出口记下有几根数据波长，并额外给 FDL 口留一格（下标 `numPorts`，波长数固定为 1）：

```35:45:src/CoreNode/OBS_CoreOutputHorizon.cc
   portLambdas= (int*)calloc(numPorts + 1,sizeof(int));
   
   cStringTokenizer tokenizer(par("lambdasPerPort").stringValue());
   while(tokenizer.hasMoreTokens()){
      portLambdas[i] = atoi(tokenizer.nextToken());
      i++;
   }
   portLambdas[numPorts] = 1; // FDL loopback port has 1 wavelength channel
```

找「到达时刻已经空闲」的波长。判断用的是严格大于：到达时刻刚好等于 horizon 也算忙。

```61:82:src/CoreNode/OBS_CoreOutputHorizon.cc
int OBS_CoreOutputHorizon::findNearestLambda(int port,simtime_t arrivalTime){
   ...
   for(i=0;i<portLambdas[port];i++){
      if(arrivalTime > horizon[port][i]){ 
         ...
      }
   }
   if(minDiff == -1) return -1;
```

返回 `-1` 就是：这个出口上，到达时刻没有任何数据波长空闲。单波长时，每个出口只有 `portLambdas[port] == 1`，相当于「这一根忙了就没有备选」。

读某个出口实际有几根波长（标定用来核对 W=1）：

```89:93:src/CoreNode/OBS_CoreOutputHorizon.cc
int OBS_CoreOutputHorizon::getPortLambdas(int port){
   Enter_Method_Silent();
   int numPorts = par("numPorts");
   if(portLambdas == NULL || port < 0 || port > numPorts) return 0;
   return portLambdas[port];
```

### 5.3 调度入口：读开关和 τ

文件：`src/CoreNode/OBS_CoreControlLogic.cc` 的 `initialize()`。

```60:63:src/CoreNode/OBS_CoreControlLogic.cc
   cModule *coreNode = getParentModule()->getParentModule();
   useFDL = coreNode->par("useFDL").boolValue();
   tau = coreNode->par("fdlDelayTime");
```

`useFDL=false` 时，下面 5.4 里所有 `else if (useFDL)` 都不会进去，行为应与加 FDL 之前一致。第 4 周 ICMP 回归测的就是这件事。

### 5.4 三种结果：直通、回环、丢掉

BCP 查完路由，得到 `outPort`。然后看目标出口在 `burstArrival` 是否空闲。

指定波长（不是通配 `*`）时更直观：

```224:249:src/CoreNode/OBS_CoreControlLogic.cc
	if(gatesHorizon->getHorizon(outPort,lambda) <= burstArrival ){
            scheduled = true;
	} else if (useFDL) {
            // Scenario B: FDL Loopback
            simtime_t fdlHorizon = gatesHorizon->getHorizon(numPorts, 0);
            simtime_t waitTime = gatesHorizon->getHorizon(outPort, lambda) - burstArrival;
            if (waitTime <= tau && fdlHorizon <= burstArrival) {
                scheduled = true;
                usedFDLForThisBurst = true;
            }
        }

        if (!scheduled) {
            delete msg;
            dropCounter++;
            burstLossContentionCounter++;
            ...
            return;
        }
```

翻译成三句话：

1. **直通**：目标波长的 horizon 已经不晚于突发到达时刻 → `scheduled = true`，不进 FDL。  
2. **回环**：FDL 开着，且 `waitTime <= τ`（推迟 τ 之后目标已经空），且 FDL 入口现在空（`fdlHorizon <= burstArrival`）→ 打上 `usedFDLForThisBurst`。  
3. **丢掉**：上面都不成立 → 删 BCP，竞争丢包加一，函数返回。对应的光突发以后到达时，OXC 没有预约，会被丢掉。

通配颜色 `outColour == -9` 时逻辑类似，只是用 `findNearestLambda` 先找一根空的；找不到再试「到达时刻 + τ」那一根是否空。见同文件约 187–204 行。

第 3 周三个测试就是把这一段的三条路都走到：

| 测试 | 设定 | 预期 |
|---|---|---|
| Test-FDL-Basic | τ=20 µs > 等待约 6.5 µs | sat2 `fdlUsageCount=1`，两个突发都到 sat3 |
| Test-FDL-TooShort | τ=2 µs < 等待 | FDL 开着也不进，丢 1 个 |
| Test-FDL-Off | `useFDL=false` | 与 TooShort 标量相同 |

### 5.5 回环时预约两段 OXC，并把 BCP 的提前量加上 τ

```294:358:src/CoreNode/OBS_CoreControlLogic.cc
      // Scenario B: FDL Loopback
      int fdlOutGate = oxc->gateSize("out") - 1;
      int fdlInGate = oxc->gateSize("in") - 1;

      // 1st OXC reservation: inGate -> fdlOutGate
      simtime_t fdlConnectTime = burstArrival - guardTime/2;
      ...
      gatesHorizon->updateHorizon(numPorts, 0, newFDLHorizon);
      ...
      scheduleAt(fdlConnectTime, fdlControlInfo);
      ...
      // 2nd OXC reservation: fdlInGate -> dest
      simtime_t delayedArrival = burstArrival + tau;
      ...
      gatesHorizon->updateHorizon(outPort, lambda, newDestHorizon);
      ...
      bcp->setBurstArrivalDelta(arrivalDelta + tau - processingTime);
```

要点：

- 第一段：突发到达前后，把**原来的入口**接到 **FDL 出口门**。FDL horizon 更新为 `burstArrival + burstDuration + 3g/4`，**不加 τ**（见 5.1）。  
- 第二段：在 `burstArrival + τ` 前后，把 **FDL 入口门**接到**真正的目标出口**。目标波长的 horizon 按推迟后的到达来更新。  
- `setBurstArrivalDelta(... + tau ...)`：下游节点必须知道光会晚 τ 到，否则会按旧时刻去等。

直通只预约一段：入口 → 目标出口。代码在同文件 253–292 行，此处不重复。

### 5.6 占用时间：直通和回环都算到同一根出纤上

```361:364:src/CoreNode/OBS_CoreControlLogic.cc
   portBusyTime[outPort] += burstDuration;
   portCarriedBytes[outPort] += (double)burstLength;
```

无论走哪条路，只要最终占用了目标出纤一个突发时长，就累加一次。后面「负载 ρ」用的就是这个累加值。回环不会把占用算两遍。

---

## 6. 为什么第 5 周先标定、不直接扫 τ

旧的第一阶段配置（`params.ini`）直接拿来做 FDL 实验，会得到空结果：

1. **单波长被破坏**：`lambdasCore1to2 = 3`，全网最容易堵的 sat1—sat2 反而是唯一违反 W=1 的链路。  
2. **负载极低**：大约只有 0.24% 量级，核心几乎不冲突，`fdlUsageCount` 会一直是 0，扫 τ 得到平直线。  
3. **流量是确定的**：固定间隔、固定启动时刻，换种子结果完全一样，重复实验没有意义。  
4. **流量几乎只从 sat1 出**：中间星看不到多路上游汇聚，而 FDL 要处理的正是这种汇聚。

所以单独做了 `fdl_params.ini` 里的 `[Config FDL-Scenario]`：**不改** `params.ini`，旧 ICMP 仍可复现。

另外：模型里没有「请把负载设成 0.4」这种旋钮，只能调 UDP 平均发送间隔。间隔和最忙星间口占用的关系取决于路由、谁最忙，必须仿真来测。这就是 `FDL-Calibrate`。

---

## 7. 标定相关代码（最近改的）

### 7.1 把所有星间链路冻成 1 个数据波长

文件：`Examples/RingFdlOBS/fdl_params.ini`。原先只改了 sat1/sat2，其余节点仍可能吃到 `params.ini` 的 General。现在 13 条 ISL、10 个节点的端口串全部写死。

```71:89:Examples/RingFdlOBS/fdl_params.ini
**.lambdasCore1to2 = 1
**.lambdasCore2to3 = 1
...（其余 lambdasCore* 均为 1）
**.sat1.coreSwitch.lambdasPerInPort  = "1 1 1"
**.sat1.coreSwitch.lambdasPerOutPort = "1 1 1"
**.sat2.coreSwitch.lambdasPerInPort  = "1 1 1 1"
**.sat2.coreSwitch.lambdasPerOutPort = "1 1 1 1"
```

端口串怎么读：第一个数是**本地边缘口**（Port0），后面每个邻星一口。sat2 有三个邻居（sat1、sat3、sat5），所以是四个 1。旧 `params.ini` 里 sat1 是 `"1 3 1"`，中间那个 3 就是 sat1—sat2 的三个数据波长。

只数「有几个端口」发现不了这个 3。所以结束时把每口波长数写进 `.sca`（见 7.3）。

### 7.2 十星互发，间隔可调；`stopTime` 必须是 -1s

```168:185:Examples/RingFdlOBS/fdl_params.ini
**.sat*.host.udpApp[0].typename = "UDPBasicApp"
...
**.sat*.host.udpApp[0].startTime = uniform(0s, 1ms)
**.sat*.host.udpApp[0].stopTime = -1s
**.sat*.host.udpApp[0].sendInterval = exponential(35.9us)
...
**.sat1.host.udpApp[0].destAddresses  = "sat2.host sat3.host ... sat10.host"
```

- `udpApp[0]` 发送，`udpApp[1]` 是 `UDPSink` 接收。  
- `destAddresses` 列出其余 9 颗星，INET 每包均匀随机选一个目的地。  
- `exponential(35.9us)`：平均 35.9 微秒发一包，间隔随机。35.9 是 9 月 9 日按实测占用约 40% 写回去的。  
- `startTime = uniform(0s, 1ms)`：各星不是同一瞬间开始。  
- `stopTime = -1s`：INET 的 `UDPBasicApp` 规定负数表示发到仿真结束。写成 `0s` 会被当成「在 0 秒停止」，再叠加随机 `startTime`，初始化就会报 `Invalid startTime/stopTime parameters`。这是 9 月 9 日图形界面启动失败的原因。

### 7.3 仿真结束：利用率 + 波长数

文件：`src/CoreNode/OBS_CoreControlLogic.cc` 的 `finish()`。调度判断未改，只多记了 `portLambdas[p]`。

```401:430:src/CoreNode/OBS_CoreControlLogic.cc
   recordScalar("burstsReceived", recvTotal);
   recordScalar("burstsScheduled", schedTotal);
   if(recvTotal > 0) recordScalar("burstLossRate", (double)dropCounter / (double)recvTotal);

   for(i=0;i<numOutPorts;i++){
      int lambdas = gatesHorizon->getPortLambdas(i);
      ...
      portUtilization = SIMTIME_DBL(portBusyTime[i]) / (SIMTIME_DBL(simTime()) * (double)lambdas);
      ...
      recordScalar("channelUtilization[p]", portUtilization);
      recordScalar("portLambdas[p]", lambdas);
      recordScalar("carriedBursts[p]", ...);
      recordScalar("carriedBytes[p]", ...);
   }
   recordScalar("maxChannelUtilization", maxUtilization);
```

（源码里名字是用 `sprintf` 拼的 `channelUtilization[%d]`，上面写成 `p` 只为阅读。）

- 利用率 = 该口累计突发时长 /（仿真时长 × 该口波长数）。波长若仍是 3，同一个占用会被除以 3，看起来「没那么忙」，标定会偏。  
- `portLambdas[p]` 必须为 1，否则单波长没生效。14 次结果里 10 个节点共 36 个出口，全部是 1。  
- 脚本默认**跳过 Port0**（本地边缘落地口），只在星间口里找最忙的。

### 7.4 标定扫描：关 FDL，7 档间隔 × 2 个种子

文件：`Examples/RingFdlOBS/fdl_experiments.ini`。

```38:46:Examples/RingFdlOBS/fdl_experiments.ini
[Config FDL-Calibrate]
extends = FDL-Scenario
**.sat*.coreSwitch.useFDL = false
**.sat*.host.udpApp[0].sendInterval = exponential(${meanInterval = 400us, 200us, 100us, 60us, 42.6us, 30us, 21.3us})
sim-time-limit = 0.5s
repeat = 2
seed-set = ${runnumber}
result-dir = results/FDL-Calibrate
```

`${meanInterval=...}` 让 OMNeT++ 自动扫 7 档；`repeat=2` 每档两个种子，共 14 次。FDL 关掉，测的是关 FDL 时「发多快 → 最忙星间口实际多忙」。

图形界面一次只跑 `#0`。要 14 次一起跑，用命令行 Cmdenv（需带上 OMNeT++ 的 `bin` 和 INET 的 dll 路径）：

```text
cd Examples/RingFdlOBS
..\..\out\gcc-debug\obsmodules.exe -u Cmdenv -f omnetpp.ini -c FDL-Calibrate -n "../..;.;D:/inet/src"
```

9 月 9 日已跑完，结果在 `Examples/RingFdlOBS/results/FDL-Calibrate/FDL-Calibrate-0.sca` … `13.sca`。

### 7.5 分析脚本：用实测曲线反推间隔

文件：`Examples/RingFdlOBS/tools/calibrate_load.py`。它**不启动**仿真，只读 `.sca`。

原先设想：丢包很低时，占用 ρ 和 `1/发送间隔` 成正比，拟合一个常数 k，再算间隔 = k / 目标ρ。单波长 OBS 上，枢纽节点在占用只有 4% 时就会有百分之几的突发撞车，不能用「丢包 < 1% 才算有效点」那种口径。脚本改为：把 7 档测到的 ρ 连成曲线，在相邻两档之间按 `1/间隔` 插值。

```215:235:Examples/RingFdlOBS/tools/calibrate_load.py
def interpolate_interval(curve, target):
    """Invert rho(interval) by interpolating in 1/interval."""
    ...
    if target < rhos[0] or target > rhos[-1]:
        return None   # 扫描范围内达不到这个 ρ
```

跑法（14 个文件齐了之后，在 `Examples/RingFdlOBS/` 下）：

```text
python tools/calibrate_load.py results/FDL-Calibrate
```

它同时检查：波长是否全 1、10 星是否都发出了突发、两个种子是否不同、FDL 计数是否为 0。

---

## 8. 14 次结果怎么读

最忙出口始终是 **sat7 的 Port1**（星间口，不是本地边缘）。两种子平均：

| 平均发送间隔 | 最忙链路占用 ρ | 枢纽节点丢包 | 全网丢包 | 含义 |
|---|---:|---:|---:|---|
| 400 µs | 0.044 | 2.5% | 1.4% | 很空，但单波长上突发仍会偶发对撞 |
| 200 µs | 0.085 | 4.5% | 2.7% | 占用大约翻倍，还在较线性的区间 |
| 100 µs | 0.166 | 8.3% | 5.3% | |
| 60 µs | 0.260 | 12.6% | 8.2% | |
| **42.6 µs** | **0.348** | 16.6% | 10.9% | 原来写在 ini 里的估算值，**不是 40%** |
| 30 µs | 0.465 | 21.2% | 14.1% | |
| 21.3 µs | 0.594 | 26.8% | 18.0% | 本扫描最快一档；再快也填不满到 80% |

插值：

| 想要的占用 | 该设的平均间隔 | 去向 |
|---|---|---|
| ρ ≈ 0.20 | 80.5 µs | 写入实验 B 的负载列表 |
| **ρ ≈ 0.40** | **35.9 µs** | 已写入 `fdl_params.ini` 默认值 |
| ρ ≈ 0.80 | 达不到 | 最重点只有 ≈ 0.59 |

平台检查（都可以写进「已验证」）：

- W=1：36 条 `portLambdas[]` 全为 1  
- 多源：10 星都有 `burst sent > 0`，每个核心 `burstsReceived > 0`  
- 随机化：同一档间隔、两个种子，ρ 和发包数都不同  
- FDL 关着：全网 `fdlUsageCount = 0`

已写回配置：

- `fdl_params.ini`：`sendInterval = exponential(35.9us)`  
- `fdl_experiments.ini` 的 `ExpB-Joint`：三档 `80.5us, 35.9us, 21.3us`（大约 ρ=0.20 / 0.40 / 0.59）

占用 40% 时枢纽仍有约百分之十几的突发丢包，这是**关 FDL 时的基线**，后面开 FDL 扫 τ，比的就是能不能把这条基线打下来。不要把它理解成「标定失败」。

---

## 9. 文件地图

| 文件 | 现在扮演的角色 |
|---|---|
| `src/CoreNode/OBS_FiberDelayLine.cc` | 延迟线器件 |
| `src/CoreNode/OBS_CoreNode.ned` | 回环口接线、`useFDL` / `fdlDelayTime` |
| `src/CoreNode/OBS_CoreControlLogic.cc` | 直通 / 回环 / 丢掉；结束时记利用率与 `portLambdas` |
| `src/CoreNode/OBS_CoreOutputHorizon.cc` | horizon 与每口波长数 |
| `src/EdgeNode/` | 第一阶段专利基线，本次未改 |
| `Examples/RingFdlOBS/params.ini` | 第一阶段总配置，**保持含 3 波长的旧样子** |
| `Examples/RingFdlOBS/fdl_params.ini` | FDL 实验场景：单波长、十星互发、35.9 µs |
| `Examples/RingFdlOBS/fdl_tests.ini` | 第 3–4 周功能/兼容测试 |
| `Examples/RingFdlOBS/fdl_experiments.ini` | 标定、实验 A/B、速率校验 |
| `Examples/RingFdlOBS/tools/calibrate_load.py` | 读 `.sca`，反推间隔 |
| `Examples/RingFdlOBS/results/FDL-Calibrate/` | 14 次原始结果 |

---

## 10. 不要理解成什么；下一步是什么

不要说成：

- FDL 已经验证能降低丢包（标定关着 FDL）  
- 已经在 40% 负载下扫过 τ（实验 A 没跑）  
- 改了边缘调度（没改 `src/EdgeNode/`）  
- 42.6 µs 就是 40% 负载（实测约 35%）  
- 负载可以扫到 80%（这套流量下最忙链路大约到 59%）  
- 回环 FDL 已经证明 offset=0 可行（回环需要提前预约 OXC）

下一步按 16 周计划是第 6 周文献定位（这篇工作新在哪），不是立刻开实验 A。实验 A 要开时，用现在的 35.9 µs 当默认负载，打开 `useFDL`，扫 `fdlDelayTime`。
