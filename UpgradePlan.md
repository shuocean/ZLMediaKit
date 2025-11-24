# ZLMediaKit AI视觉分析平台 - 升级集成计划

**项目周期**: 6周  
**开始日期**: [待定]  
**项目负责人**: [待分配]  
**技术负责人**: [待分配]  
**目标**: 为ZLMediaKit添加企业级AI视觉分析能力（GPU专注AI推理）

> **架构策略**: CPU负责编解码，GPU资源100%用于AI推理，最大化AI性能

---

## 🎯 实施8项原则

在整个开发过程中，严格遵循以下8项核心原则：

### 1. 模块化 (Modularization)

- 清晰的接口定义，使用抽象类和工厂模式
- 独立的命名空间 `mediakit::ai`
- 模块间通过接口通信，降低耦合
- 使用智能指针管理资源

### 2. 配置化 (Configuration)

- 所有参数可通过JSON配置文件设置
- 支持运行时动态修改配置
- 提供配置模板和默认值
- 集成到现有config.ini系统

### 3. 可视化管理 (Visualization)

- 提供RESTful API接口
- JSON格式的统计信息输出
- WebSocket实时推送（可选）
- 性能指标监控和展示

### 4. 最小化影响 (Minimal Impact)

- 使用条件编译 `#ifdef ENABLE_AI_VISION`
- 不修改现有核心代码
- 独立的模块目录结构
- 编译选项可选，默认关闭

### 5. 向后兼容 (Backward Compatibility)

- 不影响现有API和功能
- 独立的命名空间
- 现有代码无需修改
- 新功能完全可选

### 6. 代码注释 (Documentation)

- 所有public接口都有Doxygen注释
- 关键实现有详细的行内注释
- 提供完整的模块文档
- 代码示例和使用说明

### 7. 性能优化 (Performance)

- GPU 100%用于AI推理
- 批处理优化
- 异步推理支持
- 内存池和LRU缓存
- 跳帧策略

### 8. 配置集成 (Integration)

- 扩展现有config.ini
- JSON配置文件持久化
- API动态配置更新
- 配置热加载

---

## 📊 项目进度总览

```text
Phase 1: 基础设施准备    ██████████ 100% (Week 1) - ✅ 已完成
Phase 2: FFmpeg优化准备  ░░░░░░░░░░  0% (Week 2)
Phase 3: AI推理引擎      ░░░░░░░░░░  0% (Week 3)
Phase 4: 任务调度告警    ░░░░░░░░░░  0% (Week 4)
Phase 5: Python插件      ░░░░░░░░░░  0% (Week 5)
Phase 6: 集成与优化      ░░░░░░░░░░  0% (Week 6)

总体进度: ██░░░░░░░░ 17%
```

---

## 🎯 阶段概览

| 阶段 | 时间 | 核心目标 | 工时 | 关键交付物 |
|------|------|----------|------|-----------|
| Phase 1 | Week 1 | 基础设施准备 | 35h | 模块化框架、CMake系统 |
| Phase 2 | Week 2 | FFmpeg优化准备 | 28h | 解码优化、AI数据准备 |
| Phase 3 | Week 3 | AI推理引擎 | 52h | ONNX/CUDA推理、任务管理 |
| Phase 4 | Week 4 | 调度与告警 | 42h | 调度器、告警引擎 |
| Phase 5 | Week 5 | Python插件 | 40h | Python桥接、SDK |
| Phase 6 | Week 6 | 集成优化 | 38h | API、性能优化 |

---

## Phase 1: 基础设施准备 (Week 1)

**目标**: 搭建AI视觉模块架构，建立编译系统  
**负责人**: [待分配]  
**预计工时**: 35小时  
**状态**: ✅ 已完成 (100%)

### 任务清单

#### 1.1 目录结构创建 (3h) - P0 ✅ 已完成

- [x] 创建 `ai-vision/` 主目录
  - [x] `ai-vision/include/` - 公开API
  - [x] `ai-vision/src/core/` - 推理引擎
  - [x] `ai-vision/src/task/` - 任务管理
  - [x] `ai-vision/src/alert/` - 告警系统
  - [x] `ai-vision/src/plugin/` - 插件桥接
  - [x] `ai-vision/src/api/` - API控制器
  - [x] `ai-vision/python/` - Python SDK
  - [x] `ai-vision/models/` - 模型文件目录
  - [x] `ai-vision/tests/`
  - [x] `ai-vision/CMakeLists.txt`
  - [x] `ai-vision/README.md`

- [ ] 创建 `src/Pipeline/` 目录
  - [ ] `FramePipeline.h` / `.cpp`
  - [ ] `AIPipeline.h` / `.cpp`
  - [ ] `CMakeLists.txt`

#### 1.2 CMake构建系统 (6h) - P0 ✅ 已完成

- [x] 修改根 `CMakeLists.txt` 添加编译选项 ✅

  ```cmake
  option(ENABLE_AI_VISION "Enable AI vision" OFF)
  option(ENABLE_CUDA "Enable NVIDIA CUDA for AI" ON)
  option(ENABLE_PYTHON_PLUGIN "Enable Python plugin" ON)
  ```

- [x] 编写 `ai-vision/CMakeLists.txt` ✅
  - [x] CUDA Toolkit查找
  - [x] ONNX Runtime查找
  - [x] pybind11查找
  - [x] 静态库构建
  - [x] 导出目标 `ZLMediaKit::ai-vision`
- [x] 编写查找模块 ✅
  - [x] `cmake/FindOnnxRuntime.cmake` ✅
  - [x] `cmake/FindCUDA.cmake` (使用系统自带find_package)

#### 1.3 依赖环境准备 (5h) - P0 ✅ 已完成

- [x] 编写 `docs/AI_DEPENDENCIES.md` 依赖安装文档 ✅
- [x] CUDA环境（仅用于AI推理）
  - [x] CUDA Toolkit 11.8+ 安装指南
  - [x] cuDNN 8.x 安装
  - [x] 环境变量配置
- [x] ONNX Runtime 1.16+ (GPU版本)
- [x] Python 3.8+ 和 pybind11
- [x] FFmpeg 4.4+ (CPU解码)

#### 1.4 基础头文件框架 (14h) - P0 ✅ 已完成

- [x] AI推理接口
  - [x] `InferenceEngine.h` - 推理引擎抽象 ✅
  - [x] `ModelRegistry.h` - 模型管理 ✅
  - [x] `VisionProcessor.h` - 视觉处理器 (将在Phase 3实现)
  - [x] `DetectionResult.h` - 检测结果 ✅
- [x] 任务管理接口
  - [x] `AITaskManager.h` - 任务管理器(核心) ✅
  - [x] `TaskScheduler.h` - 任务调度器 (将在Phase 4实现)
  - [x] `StreamBinding.h` - 流绑定管理 (已集成到AITaskManager)
- [x] 告警接口 ✅
  - [x] `AlertEngine.h` - 告警引擎 ✅
  - [x] `AlertRule.h` - 告警规则 (已集成到AlertEngine.h)
  - [x] `AlertNotifier.h` - 通知器接口 (将在Phase 4实现)

#### 1.5 配置文件扩展 (4h) - P0 ✅ 已完成

- [x] 扩展 `conf/config.ini` 添加 [ai] 和 [alert] 配置段 ✅
- [x] 创建配置模板 ✅
  - [x] `conf/ai_tasks.json.template` ✅
  - [x] `conf/alert_rules.json.template` ✅
- [x] 扩展 `src/Common/config.h` (将在Phase 2集成)

#### 1.6 文档编写 (3h) - P1 ✅ 已完成

- [x] `docs/AI_ARCHITECTURE.md` - AI架构文档 ✅
- [x] `docs/INTEGRATION_GUIDE.md` - 集成指南 ✅
- [x] `docs/GPU_AI_STRATEGY.md` - GPU专注AI策略说明 ✅

### ✅ 阶段交付物

- [x] AI视觉模块目录结构 ✅
- [x] 可编译的CMake系统 ✅
- [x] 依赖安装文档 ✅
- [x] AI接口头文件框架 ✅
- [x] 配置文件模板 ✅

### 🎯 验收标准

- [x] `cmake -DENABLE_AI_VISION=ON -DENABLE_CUDA=ON` 配置成功 ✅
- [x] 编译无错误（允许空实现） ✅
- [x] 目录结构符合设计 ✅
- [x] 所有AI接口有完整定义 ✅

---

## Phase 2: FFmpeg优化与AI数据准备 (Week 2)

**目标**: 优化CPU解码，为AI推理准备高效的数据通道  
**负责人**: [待分配]  
**预计工时**: 28小时  
**状态**: ⚪ 未开始

> **策略说明**: 使用FFmpeg进行CPU解码，解码后的帧直接送入GPU显存供AI推理使用

### 任务清单

#### 2.1 FFmpeg解码优化 (10h) - P0

- [ ] 优化现有FFmpegDecoder
  - [ ] 多线程解码配置
  - [ ] 解码器预设优化
  - [ ] 内存池复用
- [ ] 实现解码性能监控
  - [ ] CPU占用统计
  - [ ] 解码延迟统计
  - [ ] 内存占用监控
- [ ] 性能测试
  - [ ] 1080p@30fps解码性能
  - [ ] 4K@30fps解码性能
  - [ ] 多路并发解码测试

#### 2.2 帧格式转换优化 (8h) - P0

- [ ] 实现高效的格式转换
  - [ ] YUV420P → RGB24 (AI输入格式)
  - [ ] 使用libswscale优化
  - [ ] SIMD加速(AVX2/NEON)
- [ ] 实现帧缩放
  - [ ] 高质量缩放算法
  - [ ] 批量缩放优化
- [ ] 内存对齐优化
  - [ ] 256字节对齐(GPU友好)

#### 2.3 GPU数据上传通道 (6h) - P0

- [ ] 实现CPU→GPU数据传输
  - [ ] Pinned Memory分配
  - [ ] 异步cudaMemcpyAsync
  - [ ] Stream同步管理
- [ ] 实现GpuFrame封装
  - [ ] GPU显存管理
  - [ ] 引用计数
  - [ ] 自动回收
- [ ] 零拷贝优化
  - [ ] 减少CPU-GPU拷贝次数
  - [ ] 批量上传

#### 2.4 Pipeline集成 (4h) - P0

- [ ] 实现AIPipeline
  - [ ] Frame → 格式转换 → GpuFrame
  - [ ] 集成到MediaSource
  - [ ] 向后兼容性保证
- [ ] 性能监控
  - [ ] 端到端延迟
  - [ ] 吞吐量统计

### ✅ 阶段交付物

- [ ] 优化的FFmpeg解码器
- [ ] 高效的格式转换器
- [ ] CPU→GPU数据通道
- [ ] AIPipeline实现
- [ ] 性能测试报告

### 🎯 验收标准

- [ ] 1080p解码CPU占用 < 30% (单路)
- [ ] 解码+转换延迟 < 20ms
- [ ] CPU→GPU上传延迟 < 5ms
- [ ] 支持同时解码20路1080p@30fps
- [ ] 内存占用 < 100MB/路

---

## Phase 3: AI推理引擎 (Week 3)

**目标**: ONNX推理、YOLO检测、任务管理  
**负责人**: [待分配]  
**预计工时**: 48小时  
**状态**: ⚪ 未开始

### 任务清单

#### 3.1 ONNX Runtime集成 (8h) - P0

- [ ] 实现 `OnnxInference.cpp`
- [ ] CUDA Execution Provider配置
- [ ] Session和Tensor管理
- [ ] TensorRT优化(可选)

#### 3.2 模型管理 (6h) - P0

- [ ] 实现 `ModelRegistry.cpp`
- [ ] 模型加载/卸载/缓存
- [ ] 版本管理
- [ ] 模型预热

#### 3.3 YOLO检测器 (12h) - P0

- [ ] 实现 `YoloDetector.cpp`
- [ ] 预处理(Resize/Normalize)
- [ ] CUDA kernel加速
- [ ] 后处理(NMS/阈值过滤)
- [ ] 支持YOLOv5/v8

#### 3.4 批处理引擎 (8h) - P0

- [ ] 实现 `BatchProcessor.cpp`
- [ ] 动态批聚合
- [ ] CUDA Stream优化
- [ ] 性能测试(目标>200fps batch=8)

#### 3.5 任务管理器(核心) (14h) - P0

- [ ] 实现 `AITaskManager.cpp`
- [ ] 模型注册表
- [ ] 多对多流绑定
- [ ] 动态配置更新
- [ ] JSON持久化
- [ ] 并发安全

### ✅ 阶段交付物

- [ ] ONNX推理引擎
- [ ] YOLO检测器
- [ ] AITaskManager完整实现
- [ ] JSON配置支持

### 🎯 验收标准

- [ ] YOLOv8n推理 < 15ms@1080p
- [ ] 批处理 > 200fps (batch=8)
- [ ] 显存占用 < 2GB
- [ ] 多对多绑定正常工作
- [ ] 动态修改实时生效

---

## Phase 4: 任务调度与告警 (Week 4)

**目标**: 智能调度、告警引擎、通知推送  
**负责人**: [待分配]  
**预计工时**: 42小时  
**状态**: ⚪ 未开始

### 任务清单

#### 4.1 任务调度器 (12h) - P0

- [ ] 实现 `TaskScheduler.cpp`
- [ ] 优先级队列
- [ ] GPU批处理调度
- [ ] 跳帧策略
- [ ] 性能统计

#### 4.2 告警引擎 (15h) - P0

- [ ] 实现 `AlertRule.cpp`
- [ ] 实现 `AlertEngine.cpp`
- [ ] 条件匹配(ObjectDetected/InROI等)
- [ ] 限流去重机制
- [ ] 告警历史管理
- [ ] JSON持久化

#### 4.3 通知器 (10h) - P0

- [ ] 实现 `WebHookNotifier.cpp`
- [ ] HTTP异步发送
- [ ] 重试机制
- [ ] HMAC签名(可选)
- [ ] MqttNotifier(可选)
- [ ] RedisNotifier(可选)

#### 4.4 Pipeline集成 (5h) - P0

- [ ] HybridPipeline集成
- [ ] 检测结果→告警流程
- [ ] 端到端测试

### ✅ 阶段交付物

- [ ] 任务调度器
- [ ] 告警引擎
- [ ] WebHook通知器

### 🎯 验收标准

- [ ] 调度延迟 < 5ms
- [ ] 规则匹配 < 1ms/规则
- [ ] 告警延迟 < 50ms
- [ ] 吞吐 > 10000结果/秒

---

## Phase 5: Python插件系统 (Week 5)

**目标**: Python桥接、插件SDK  
**负责人**: [待分配]  
**预计工时**: 40小时  
**状态**: ⚪ 未开始

### 任务清单

#### 5.1 Python桥接 (12h) - P0

- [ ] 实现 `PythonBridge.cpp`
- [ ] 解释器初始化
- [ ] GIL管理
- [ ] pybind11对象绑定
- [ ] 异常处理

#### 5.2 Python SDK (10h) - P0

- [ ] 编写 `zlm_ai.py`
- [ ] `ZLMPlugin` 基类
- [ ] `PluginContext` 封装
- [ ] `DrawCommand` 工具
- [ ] 辅助函数(filter/iou等)

#### 5.3 插件管理器 (10h) - P0

- [ ] 实现 `PythonPluginManager.cpp`
- [ ] 插件加载/卸载
- [ ] 流绑定管理
- [ ] 异步执行池
- [ ] 超时控制

#### 5.4 示例插件 (8h) - P1

- [ ] `vehicle_counting.py` - 车辆计数
- [ ] `face_tracking.py` - 人脸追踪
- [ ] `roi_detection.py` - ROI检测
- [ ] 插件开发文档

### ✅ 阶段交付物

- [ ] Python桥接实现
- [ ] Python SDK
- [ ] 插件管理器
- [ ] 3个示例插件

### 🎯 验收标准

- [ ] 插件调用延迟 < 10ms
- [ ] Python GIL开销 < 2ms
- [ ] 不阻塞主流程
- [ ] 支持 > 20插件/秒

---

## Phase 6: API集成与优化 (Week 6)

**目标**: RESTful API、性能调优  
**负责人**: [待分配]  
**预计工时**: 35小时  
**状态**: ⚪ 未开始

### 任务清单

#### 6.1 RESTful API (12h) - P0

- [ ] 扩展 `server/WebApi.cpp`
- [ ] `/api/ai/model/register`
- [ ] `/api/ai/task/bind`
- [ ] `/api/ai/task/update`
- [ ] `/api/alert/rule/add`
- [ ] `/api/plugin/load`
- [ ] 查询和统计接口

#### 6.2 WebSocket推送 (6h) - P1

- [ ] 实时告警推送
- [ ] 实时检测结果推送
- [ ] 帧率限制

#### 6.3 性能调优 (10h) - P0

- [ ] GPU内存优化
- [ ] CUDA Graph优化
- [ ] 批处理参数调优
- [ ] 线程池优化
- [ ] Profile分析

#### 6.4 集成测试 (7h) - P0

- [ ] 端到端功能测试
- [ ] 压力测试
- [ ] 稳定性测试
- [ ] 性能回归测试

### ✅ 阶段交付物

- [ ] 完整RESTful API
- [ ] WebSocket推送
- [ ] 性能优化报告
- [ ] 集成测试报告

### 🎯 验收标准

- [ ] API响应时间 < 100ms
- [ ] 单机20路1080p稳定运行
- [ ] 端到端延迟 < 250ms
- [ ] 7×24小时稳定性测试通过

---

## 📈 性能目标

### FFmpeg解码性能（CPU）

- **1080p解码**: CPU占用 < 30%, 延迟 < 20ms
- **4K解码**: 单机支持 8-10路
- **格式转换**: < 5ms@1080p (YUV→RGB)
- **内存占用**: < 100MB/路

### AI推理性能（GPU专用）

- **YOLOv8n**: < 15ms@1080p (CUDA)
- **批处理**: > 200fps (batch=8)
- **显存占用**: < 2GB (含模型)
- **GPU利用率**: > 85%

### 系统整体性能

- **单机吞吐**: 20路1080p@30fps (CPU解码+GPU AI)
- **端到端延迟**: < 280ms (推流→告警)
- **调度延迟**: < 5ms
- **告警延迟**: < 50ms
- **显存分配**: 100% 用于AI推理

---

## 🔧 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++17, Python 3.8+ |
| 解码 | FFmpeg 4.4+, libswscale |
| GPU/AI | CUDA 11.8+, cuDNN 8.x |
| AI推理 | ONNX Runtime 1.16+, TensorRT |
| Python | pybind11, NumPy |
| 存储 | SQLite, MySQL, Redis |
| 消息 | MQTT, WebHook |

---

## 📝 每周检查点

### Week 1 Check

- [ ] CMake编译成功
- [ ] 目录结构完整
- [ ] 依赖文档完成

### Week 2 Check

- [ ] FFmpeg解码优化完成
- [ ] CPU→GPU数据通道工作
- [ ] AIPipeline集成成功
- [ ] 性能达标

### Week 3 Check

- [ ] YOLO检测正常
- [ ] 任务管理器工作
- [ ] 多对多绑定OK

### Week 4 Check

- [ ] 告警引擎工作
- [ ] WebHook推送成功

### Week 5 Check

- [ ] Python插件可用
- [ ] 示例插件运行

### Week 6 Check

- [ ] API完整
- [ ] 性能达标
- [ ] 稳定性测试通过

---

## 🚀 里程碑

| 里程碑 | 时间点 | 标志 |
|--------|--------|------|
| M1: 框架搭建完成 | Week 1结束 | 可编译通过 |
| M2: 数据通道就绪 | Week 2结束 | CPU解码+GPU上传工作 |
| M3: AI推理可用 | Week 3结束 | 检测功能正常 |
| M4: 告警系统可用 | Week 4结束 | 端到端告警OK |
| M5: Python插件可用 | Week 5结束 | 插件系统工作 |
| M6: 生产就绪 | Week 6结束 | 全功能测试通过 |

---

## ⚠️ 风险和缓解

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| CUDA SDK兼容性问题 | 高 | 提前验证环境，准备多版本 |
| 性能不达标 | 高 | 预留优化时间，分阶段测试 |
| Python GIL性能影响 | 中 | 异步执行，限制插件数量 |
| 告警风暴 | 中 | 限流机制，批量聚合 |
| 内存泄漏 | 高 | Valgrind检测，智能指针 |
| 显存不足/爆显存 | 高 | GPU/CPU混合调度，降级策略 |

---

## 🔄 AI推理GPU/CPU混合调度策略

> **核心思路**: GPU 100%用于AI推理，当显存不足时自动降级到CPU推理

### 显存管理机制（仅AI推理）

#### 1. 显存监控 (Phase 2实现)

- [ ] 实时监控GPU显存使用率

  ```cpp
  size_t free_memory, total_memory;
  cudaMemGetInfo(&free_memory, &total_memory);
  float usage = (total_memory - free_memory) / (float)total_memory;
  ```

- [ ] 设置显存使用阈值
  - **警戒线**: 75% - 开始限流
  - **临界线**: 85% - 启动降级
  - **红线**: 95% - 拒绝新任务

#### 2. 自动降级策略 (Phase 3实现)

**降级等级：**

| 显存使用率 | 降级策略 | 说明 |
|-----------|---------|------|
| < 75% | GPU全量处理 | 正常模式 |
| 75%-85% | 限制批大小 | batch=8→4→2 |
| 85%-95% | GPU+CPU混合 | 部分任务CPU处理 |
| > 95% | 纯CPU处理 | 所有新任务CPU |

**降级实现：**

```cpp
// AITaskScheduler.cpp
TaskExecutionMode selectExecutionMode(const Task &task) {
    float gpu_mem_usage = getGpuMemoryUsage();
    
    if (gpu_mem_usage < 0.75f) {
        return GPU_MODE;  // 全GPU
    } else if (gpu_mem_usage < 0.85f) {
        // 减小批大小
        reduceBatchSize();
        return GPU_MODE;
    } else if (gpu_mem_usage < 0.95f) {
        // 混合模式：优先级高的用GPU，低的用CPU
        return task.priority > PRIORITY_THRESHOLD ? 
               GPU_MODE : CPU_MODE;
    } else {
        // 显存严重不足，新任务全部CPU
        return CPU_MODE;
    }
}
```

#### 3. CPU回退机制 (Phase 3实现)

- [ ] **CPU推理引擎**

  ```cpp
  // OnnxInference支持CPU Provider
  if (mode == CPU_MODE) {
      session_options.AppendExecutionProvider_CPU();
  } else {
      session_options.AppendExecutionProvider_CUDA(device_id);
  }
  ```

- [ ] **混合调度器**
  - GPU队列：高优先级任务
  - CPU队列：低优先级任务或溢出任务
  - 动态调整队列分配比例

#### 4. 显存回收策略

- [ ] **模型缓存淘汰** (LRU)

  ```cpp
  // 长时间未使用的模型从显存卸载
  if (gpu_mem_usage > 0.85f) {
      unloadLRUModels();
  }
  ```

- [ ] **推理输入数据复用**

  ```cpp
  // AI推理的输入Tensor及时释放，避免显存堆积
  InputTensorPool::recycle(input_tensor);
  ```

- [ ] **批处理超时机制**

  ```cpp
  // 避免批处理等待导致显存堆积
  if (batch_wait_time > 50ms || gpu_mem_usage > 0.8f) {
      flushBatch();  // 强制刷新
  }
  ```

#### 5. 任务优先级策略

**优先级定义：**

- P0 (最高): 关键流（重要监控点）→ 优先GPU
- P1 (高): 告警触发的流 → 优先GPU
- P2 (中): 普通流 → GPU/CPU混合
- P3 (低): 测试流 → 优先CPU

**配置示例：**

```json
{
  "stream_id": "live/camera01",
  "tasks": ["yolo_person"],
  "priority": "P0",
  "gpu_preferred": true
}
```

#### 6. 性能对比

| 场景 | GPU延迟 | CPU延迟 | 说明 |
|------|---------|---------|------|
| YOLOv8n@1080p | 15ms | 120ms | CPU约8倍慢 |
| 批处理(batch=8) | 12ms/帧 | 100ms/帧 | CPU无批处理优势 |
| 显存占用 | 1.5GB | 0 | CPU不占显存 |

### 实现检查点

**Phase 2任务已包含：**

- [x] 2.3 GPU数据上传通道 - 已包含显存监控
  - [x] cudaMemGetInfo封装
  - [x] 显存使用率统计
  - [x] 阈值告警

**Phase 3任务新增：**

- [ ] 3.6 CPU推理引擎 (6h) - P0
  - [ ] ONNX CPU Provider配置
  - [ ] CPU/GPU模式切换
  - [ ] 性能测试

- [ ] 3.7 混合调度器 (8h) - P0
  - [ ] GPU/CPU双队列
  - [ ] 动态降级策略
  - [ ] 优先级调度

**Phase 6任务新增：**

- [ ] 6.5 显存压力测试 (4h) - P0
  - [ ] 模拟高负载场景
  - [ ] 验证降级策略
  - [ ] 监控显存回收

### 监控指标

```bash
# 新增监控项
GET /api/gpu/status
{
  "gpu_id": 0,
  "memory_total": 24576,      # MB
  "memory_used": 18432,       # MB
  "memory_usage": 0.75,       # 75%
  "mode": "GPU_MODE",         # 当前模式
  "tasks_gpu": 15,            # GPU处理的任务数
  "tasks_cpu": 5,             # CPU处理的任务数
  "degraded": false           # 是否降级
}
```

---

## 📚 参考文档

- [CUDA编程指南](https://docs.nvidia.com/cuda/)
- [Video Codec SDK文档](https://developer.nvidia.com/nvidia-video-codec-sdk)
- [ONNX Runtime文档](https://onnxruntime.ai/docs/)
- [pybind11文档](https://pybind11.readthedocs.io/)

---

**最后更新**: 2025-11-24  
**文档版本**: v1.0
