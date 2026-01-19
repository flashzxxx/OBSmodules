# 面向资源受限环境的 OBS 边缘节点动态队列调度方案

## 1. 背景与问题

### 1.1 OBS 边缘节点的职责
在光突发交换（Optical Burst Switching, OBS）网络中，边缘节点（Edge Node）承担着将传统 IP 数据包"汇聚"并"封装"成光突发（Burst）的核心任务。这个过程由 **组帧器（Burstifier）** 完成。

### 1.2 资源受限问题
在实际硬件或仿真环境中，物理组帧队列的数量是有限的。例如，本项目中配置了 **8 个物理队列**。然而，网络中的目的节点（由 Label 标识）数量往往超过这个限制，例如本项目中有 **10 个目的标签（Label 1-10）**。

**核心矛盾**：如何用 8 个物理队列服务 10 个业务流标签？

### 1.3 传统方案的缺陷
传统的静态映射方案（Label X 固定使用 Queue X）在目的地数量超过队列数时会直接导致丢包。这在动态流量场景下是不可接受的。

---

## 2. 方案设计原理

本方案的核心思想是：**解耦物理队列索引与业务标签（Label）**，引入基于优先级的动态分配机制和 LRU（Least Recently Used）抢占算法。

### 2.1 整体数据流

```
IP 数据包 -> [Dispatcher] -> 匹配 Label -> 寻找物理队列 -> [Burstifier] -> 光突发
                                              |
                                   四级优先级调度决策
```

### 2.2 四级优先级调度策略

当一个数据包到达 Dispatcher 后，系统按以下顺序寻找合适的物理队列：

| 优先级 | 名称 | 描述 | 触发条件 |
|--------|------|------|----------|
| **P1** | 活跃匹配 | 找到一个**正在工作**且 Label 匹配的队列 | `!isIdle && currentLabel == targetLabel` |
| **P2** | 缓存复用 | 找到一个**空闲**但历史 Label 匹配的队列 | `isIdle && currentLabel == targetLabel` |
| **P3** | 新配空闲 | 找到第一个**空闲**队列并分配新 Label | `isIdle` |
| **P4** | LRU 抢占 | 所有队列都忙时，强制驱逐最久未活跃的队列 | `selectedQueue == -1` (所有队列都被占用) |

### 2.3 LRU 抢占机制详解

当 8 个队列全部被占用且无任何匹配时，系统启动 LRU 算法：

1. **选择被置换队列**：遍历所有队列的 `lastAccessTime`，选择时间戳最小（最久未被使用）的队列作为置换目标。
2. **强制冲刷（Force Flush）**：立即将该队列中的现有数据包打包成 Burst 发送出去，**不等待超时**。
3. **身份切换**：清空后将队列的 Label 切换为新数据包的目标 Label。
4. **新包入队**：新数据包进入该队列。

这确保了：
- **零丢包**：新流量总能获得可用的队列资源。
- **数据隔离**：不同 Label 的数据永不在同一个 Burst 中混合。

---

## 3. 关键代码实现

### 3.1 Dispatcher 核心调度逻辑（单次扫描优化）

**文件**：`src/EdgeNode/OBS_PacketDispatcher.cc`

为了提高效率，我们将 P1、P2、P3 的查找合并为**一次循环（Single-Pass）**，避免对队列数组进行多次遍历：

```cpp
// 初始化候选变量
int selectedQueue = -1;    // 最终选中的队列
int idleMatchQueue = -1;   // P2 候选：空闲但历史匹配
int firstEmptyQueue = -1;  // P3 候选：第一个空闲位

// 单次扫描：同时收集 P1、P2、P3 的候选结果
for(i=0; i < numQueues; i++){
   bool isIdle = burstifiers[i]->isIdle();
   int currentLabel = burstifiers[i]->getDestLabel();

   // P1：活跃匹配 - 最高优先，一旦命中立即跳出
   if(!isIdle && currentLabel == targetLabel){
      selectedQueue = i;
      break;
   }

   // P2：空闲但历史匹配（仅记录第一个）
   if(isIdle && idleMatchQueue == -1 && currentLabel == targetLabel){
      idleMatchQueue = i;
   }

   // P3：第一个纯空闲队列（仅记录第一个）
   if(isIdle && firstEmptyQueue == -1){
      firstEmptyQueue = i;
   }
}

// 决策阶段：P1 未命中时，依次尝试 P2 和 P3
if(selectedQueue == -1){
   if(idleMatchQueue != -1){
      selectedQueue = idleMatchQueue;  // 复用旧缓存
   }
   else if(firstEmptyQueue != -1){
      selectedQueue = firstEmptyQueue;
      burstifiers[selectedQueue]->setDestLabel(targetLabel);  // 分配新标签
   }
}
```

### 3.2 LRU 抢占逻辑

当上述三级优先级都无法找到队列时，触发 P4：

```cpp
if(selectedQueue == -1){
    // 寻找最久未活跃的队列作为置换目标
    int replacedQueue = 0;
    simtime_t minTime = lastAccessTimes[0];
    for(i=1; i < numQueues; i++){
        if(lastAccessTimes[i] < minTime){
            minTime = lastAccessTimes[i];
            replacedQueue = i;
        }
    }
    
    // 强制冲刷并切换标签
    burstifiers[replacedQueue]->forceFlush();
    burstifiers[replacedQueue]->setDestLabel(targetLabel);
    selectedQueue = replacedQueue;
}
```

### 3.3 Burstifier 的强制冲刷接口

**文件**：`src/EdgeNode/OBS_PacketBurstifier.cc`

```cpp
void OBS_PacketBurstifier::forceFlush(){
    Enter_Method("forceFlush()");
    if (!burstContent.isEmpty()) {
        EV << "[Burstifier] FORCED FLUSH (Preemption triggered)" << endl;
        if(timeout_msg->isScheduled()) cancelEvent(timeout_msg);  // 取消既有定时器
        assembleBurst();  // 立即组帧并发送
    }
}
```

### 3.4 最终发送与 LRU 时间戳更新

无论通过哪种优先级找到队列，最终都在统一位置执行发送：

```cpp
if(selectedQueue != -1){
    lastAccessTimes[selectedQueue] = simTime();  // 更新活跃时间戳
    send(msg, "out", selectedQueue);
}
```

---

## 4. 方案优势总结

| 特性 | 传统静态方案 | 本动态调度方案 |
|------|--------------|----------------|
| 队列利用率 | 低（1:1 绑定） | 高（N:M 动态映射） |
| 抗突发能力 | 差 | 强（LRU 自动平衡） |
| 首包延迟 | 高（多次遍历） | 低（单次扫描） |
| 数据隔离 | 天然隔离 | 通过 `forceFlush` 保证 |
| 丢包率 | 高（队列满时直接丢） | 极低（抢占保底） |

---

## 5. 涉及的源文件清单

| 文件路径 | 修改内容 |
|----------|----------|
| `src/EdgeNode/OBS_PacketDispatcher.h` | 新增 `lastAccessTimes` 数组、`burstifiers` 指针数组 |
| `src/EdgeNode/OBS_PacketDispatcher.cc` | 实现四级优先级调度、LRU 算法、单次扫描优化 |
| `src/EdgeNode/OBS_PacketBurstifier.h` | 新增 `isIdle()`、`getDestLabel()`、`setDestLabel()`、`forceFlush()` 接口 |
| `src/EdgeNode/OBS_PacketBurstifier.cc` | 实现上述接口 |

---

## 6. 配置说明

在 `omnetpp.ini` 中，可通过以下参数控制队列行为：

```ini
**.edgeRouter.obs.assembler.numQueues = 8          # 物理队列数量
**.edgeRouter.obs.assembler.packetBurstifier[*].timeout = 0.001s  # 组帧超时
**.edgeRouter.obs.assembler.packetBurstifier[*].maxSize = 500     # 最大 Burst 大小
```

