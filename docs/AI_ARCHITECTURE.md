# ZLMediaKit AI Vision 架构设计

## 概述

AI Vision模块为ZLMediaKit添加AI视觉分析能力，采用"CPU解码 + GPU推理"的架构策略。

## 核心架构

```
Media Stream → FFmpeg(CPU) → Format Convert → GPU Upload → AI Inference(GPU) → Results → Alert/Plugin
```

## 模块划分

### 1. 推理引擎层 (ai-vision/src/core/)
- InferenceEngine: 推理引擎抽象
- ModelRegistry: 模型注册管理
- DetectionResult: 结果数据结构

### 2. 任务管理层 (ai-vision/src/task/)
- AITaskManager: 任务管理（多对多关系）
- TaskScheduler: 任务调度

### 3. 告警系统 (ai-vision/src/alert/)
- AlertEngine: 告警引擎
- WebHookNotifier: 通知推送

### 4. 插件系统 (ai-vision/src/plugin/)
- PythonBridge: Python桥接
- PythonPluginManager: 插件管理

## 数据流

1. 视频流 → MediaSource
2. Frame → FFmpeg解码(CPU)
3. YUV → RGB格式转换
4. CPU内存 → GPU显存(异步上传)
5. GPU推理 → 检测结果
6. 结果 → 告警/插件处理

## 多对多关系

Stream-Task绑定采用双向索引：
- _stream_bindings: stream_id → set<task_id>
- _task_bindings: task_id → set<stream_id>

## GPU资源管理

- 100% GPU资源用于AI推理
- 显存不足时自动降级到CPU
- LRU策略淘汰长期未用的模型

详见 UpgradePlan.md
