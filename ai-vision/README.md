# ZLMediaKit AI Vision Module

## 概述

AI视觉分析模块为ZLMediaKit添加企业级AI视觉能力，支持目标检测、人脸识别、行为分析等功能。

**核心设计原则**：

1. **模块化**：清晰的接口定义，易于扩展
2. **配置化**：所有参数可通过配置文件或API动态设置
3. **可视化管理**：提供RESTful API和WebSocket接口
4. **最小化影响**：使用条件编译，不影响现有功能
5. **向后兼容**：完全兼容原有代码
6. **高性能**：GPU专注AI推理，CPU负责编解码
7. **易集成**：集成到现有config.ini配置系统

## 架构设计

### GPU资源分配策略

```
CPU: FFmpeg解码 → 格式转换 → 数据上传
  ↓
GPU: 100%资源用于AI推理（ONNX Runtime / TensorRT）
  ↓
输出: 检测结果 → 告警 → Python插件后处理
```

### 多对多关系模型

```
Stream 1 ──┬──→ Task A (YOLOv8 Person)
           ├──→ Task B (Face Detection)
           └──→ Task C (Vehicle Counting)

Stream 2 ──┬──→ Task A
           └──→ Task D (Pose Estimation)

Task A ──→ Stream 1, Stream 2, Stream 3...
```

## 快速开始

### 1. 编译选项

```bash
mkdir build && cd build
cmake .. \
  -DENABLE_AI_VISION=ON \
  -DENABLE_CUDA=ON \
  -DENABLE_PYTHON_PLUGIN=ON
make -j8
```

### 2. 配置文件 (conf/config.ini)

```ini
[ai]
enable=1
config_file=./conf/ai_tasks.json
default_provider=cuda
default_device_id=0

[alert]
enable=1
config_file=./conf/alert_rules.json
webhook_url=http://localhost:8080/webhook
```

### 3. AI任务配置 (conf/ai_tasks.json)

```json
{
  "models": {
    "yolo_person": {
      "model_path": "./models/yolov8n.onnx",
      "provider": "cuda",
      "conf_threshold": 0.6
    }
  },
  "bindings": {
    "live/camera01": ["yolo_person"],
    "live/camera02": ["yolo_person"]
  }
}
```

### 4. RESTful API使用

```bash
# 注册模型
curl -X POST http://localhost/index/api/ai/model/register \
  -d "model_id=yolo_person" \
  -d "model_path=./models/yolov8n.onnx"

# 绑定任务到流
curl -X POST http://localhost/index/api/ai/task/bind \
  -d "stream_id=live/camera01" \
  -d 'task_ids=["yolo_person"]'

# 动态修改阈值
curl -X POST http://localhost/index/api/ai/task/update \
  -d "task_id=yolo_person" \
  -d "conf_threshold=0.7"
```

## 目录结构

```
ai-vision/
├── include/           # 公开API头文件
│   ├── InferenceEngine.h     # 推理引擎接口
│   ├── ModelRegistry.h       # 模型注册表
│   ├── DetectionResult.h     # 检测结果
│   ├── AITaskManager.h       # 任务管理器（核心）
│   ├── AlertEngine.h         # 告警引擎
│   └── PythonPlugin.h        # Python插件接口
├── src/
│   ├── core/          # 核心实现
│   ├── task/          # 任务管理
│   ├── alert/         # 告警系统
│   ├── plugin/        # Python插件
│   └── api/           # API控制器
├── python/            # Python SDK
├── models/            # 模型文件目录
└── tests/             # 单元测试
```

## 性能指标

- **FFmpeg解码**: <30% CPU占用, <20ms延迟@1080p
- **AI推理**: <15ms@1080p (YOLOv8n + CUDA)
- **批处理**: >200fps (batch=8)
- **显存占用**: <2GB (单模型)
- **端到端延迟**: <280ms (推流→告警)

## 依赖项

- CUDA 11.8+
- cuDNN 8.x
- ONNX Runtime 1.16+ (GPU版本)
- FFmpeg 4.4+
- Python 3.8+ (可选，插件功能)
- pybind11 (可选，插件功能)

## 文档

- [架构设计](../docs/AI_ARCHITECTURE.md)
- [集成指南](../docs/INTEGRATION_GUIDE.md)
- [API参考](../docs/API_REFERENCE.md)
- [升级计划](../UpgradePlan.md)

## 许可证

MIT License - 详见根目录LICENSE文件
