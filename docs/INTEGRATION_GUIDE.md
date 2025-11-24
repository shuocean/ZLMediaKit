# ZLMediaKit AI Vision 集成指南

## 快速开始

### 1. 编译

```bash
cmake .. -DENABLE_AI_VISION=ON -DENABLE_CUDA=ON
make -j$(nproc)
```

### 2. 配置

编辑 `conf/config.ini`:
```ini
[ai]
enable=1
config_file=./conf/ai_tasks.json

[alert]
enable=1
config_file=./conf/alert_rules.json
```

### 3. 配置AI任务

复制并编辑 `conf/ai_tasks.json.template` → `conf/ai_tasks.json`

### 4. 启动服务

```bash
./MediaServer -c conf/config.ini
```

## RESTful API

### 注册模型
```bash
POST /index/api/ai/model/register
{
  "model_id": "yolo_person",
  "model_path": "./models/yolov8n.onnx",
  "provider": "cuda"
}
```

### 绑定任务
```bash
POST /index/api/ai/task/bind
{
  "stream_id": "__default__/live/camera01",
  "task_ids": ["task_person_det"]
}
```

### 更新阈值
```bash
POST /index/api/ai/task/update
{
  "task_id": "task_person_det",
  "conf_threshold": 0.7
}
```

### 查询统计
```bash
GET /index/api/ai/statistics
```

## WebHook告警格式

```json
{
  "alert_id": "uuid",
  "rule_id": "rule_person_detected",
  "stream_id": "__default__/live/camera01",
  "timestamp": 1700000000000,
  "condition_type": "ObjectDetected",
  "detection": {
    "boxes": [...],
    "model_id": "yolo_person"
  }
}
```

## Python插件开发

```python
from zlm_ai import ZLMPlugin, PluginContext

class MyPlugin(ZLMPlugin):
    def process(self, ctx: PluginContext):
        # 处理检测结果
        for box in ctx.detection.boxes:
            if box.class_id == 0:  # person
                print(f"Person detected: {box.confidence}")
        return ctx
```

详见 ai-vision/README.md
