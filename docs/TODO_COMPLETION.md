# AI Vision 模块 TODO 完善计划

## 📊 TODO统计

### 总体统计

- **总TODO数**: 约35个
- **已完成**: 0个
- **进行中**: 1个 (JsonHelper)
- **待完成**: 34个

### 按优先级分类

#### P0 - 核心功能（必须完成）

1. **JSON序列化/反序列化** (20个TODO)
   - InferenceConfig::fromJson/toJson
   - YoloConfig::fromJson/toJson
   - PipelineConfig::fromJson/toJson
   - AlertRule::fromJson/toJson
   - AlertEvent::toJson
   - DetectionResult::fromJson/toJson
   - 等等...

2. **统计信息JSON化** (8个TODO)
   - FrameConverter::getStatistics()
   - GpuUploader::getStatistics()
   - YoloDetector::getStatistics()
   - AIPipeline::getStatistics()
   - AlertEngine::getStatistics()

3. **文件加载/保存** (4个TODO)
   - AlertEngine::loadFromFile()
   - AlertEngine::saveToFile()
   - ModelRegistry文件操作

#### P1 - 性能优化（重要）

4. **SIMD优化** (2个TODO)
   - FrameConverter前处理SIMD
   - YoloDetector前处理SIMD

5. **GPU批处理** (2个TODO)
   - YoloDetector::detectBatchGpu()
   - GpuUploader::uploadBatch()优化

6. **CUDA Kernel** (1个TODO)
   - YoloDetector CUDA加速

#### P2 - 高级功能（可选）

7. **目标跟踪** (2个TODO)
   - AlertConditionType::ObjectStay
   - AlertConditionType::ObjectCrossLine

## 🔧 详细完善方案

### 方案1：使用nlohmann/json库（推荐）

**优点**：

- 成熟稳定
- 性能优秀
- API友好

**实施步骤**：

```cpp
// 1. 添加依赖
find_package(nlohmann_json REQUIRED)

// 2. 实现fromJson
bool InferenceConfig::fromJson(const string &json_str) {
    try {
        auto j = nlohmann::json::parse(json_str);
        model_path = j.value("model_path", "");
        provider = j.value("provider", ExecutionProvider::Auto);
        device_id = j.value("device_id", 0);
        // ...
        return true;
    } catch(...) {
        return false;
    }
}

// 3. 实现toJson
string InferenceConfig::toJson() const {
    nlohmann::json j;
    j["model_path"] = model_path;
    j["provider"] = (int)provider;
    j["device_id"] = device_id;
    // ...
    return j.dump();
}
```

### 方案2：使用JsonHelper轻量级实现（已提供）

**优点**：

- 无额外依赖
- 代码轻量
- 够用

**实施步骤**：

```cpp
string InferenceConfig::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("model_path", model_path);
    ss << JsonHelper::field("device_id", device_id);
    ss << JsonHelper::field("batch_size", batch_size, true);
    ss << JsonHelper::objectEnd();
    return ss.str();
}
```

### 方案3：分阶段实现

**第一阶段** (1-2天):

- ✅ 完成核心配置类JSON (InferenceConfig, YoloConfig, AlertRule)
- ✅ 完成统计信息JSON (所有getStatistics())
- ✅ 文件加载/保存基础实现

**第二阶段** (2-3天):

- ✅ SIMD优化
- ✅ GPU批处理优化
- ✅ 性能测试

**第三阶段** (可选):

- ✅ 目标跟踪
- ✅ 高级告警条件

## 📝 逐个TODO实现示例

### 1. InferenceConfig JSON实现

```cpp
// InferenceEngine.cpp

#include "JsonHelper.h"

bool InferenceConfig::fromJson(const string &json_str) {
    // 简单实现
    JsonHelper::parseString(json_str, "model_path", model_path);
    int provider_int = 0;
    JsonHelper::parseInt(json_str, "provider", provider_int);
    provider = (ExecutionProvider)provider_int;
    JsonHelper::parseInt(json_str, "device_id", device_id);
    JsonHelper::parseInt(json_str, "batch_size", batch_size);
    JsonHelper::parseBool(json_str, "enable_profiling", enable_profiling);
    return true;
}

string InferenceConfig::toJson() const {
    stringstream ss;
    ss << "{";
    ss << "\"model_path\":\"" << model_path << "\",";
    ss << "\"provider\":" << (int)provider << ",";
    ss << "\"device_id\":" << device_id << ",";
    ss << "\"batch_size\":" << batch_size << ",";
    ss << "\"enable_profiling\":" << (enable_profiling ? "true" : "false");
    ss << "}";
    return ss.str();
}
```

### 2. YoloDetector统计信息实现

```cpp
string OnnxYoloDetector::getStatistics() const {
    stringstream ss;
    ss << "{";
    ss << "\"detect_count\":" << _stats.detect_count << ",";
    ss << "\"avg_fps\":" << _stats.avg_fps << ",";
    ss << "\"avg_latency_ms\":" << _stats.avg_latency_ms << ",";
    ss << "\"preprocess_time_us\":" << _stats.preprocess_time_us << ",";
    ss << "\"inference_time_us\":" << _stats.inference_time_us << ",";
    ss << "\"postprocess_time_us\":" << _stats.postprocess_time_us << ",";
    ss << "\"avg_detections\":" << _stats.avg_detections;
    ss << "}";
    return ss.str();
}
```

### 3. AlertEngine文件加载实现

```cpp
int AlertEngine::loadFromFile(const string &filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        ErrorL << "Failed to open alert rules file: " << filepath;
        return 0;
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    string json_str = buffer.str();
    
    // 解析JSON并注册规则
    // TODO: 完整JSON解析
    
    return 0;
}

bool AlertEngine::saveToFile(const string &filepath) const {
    ofstream file(filepath);
    if (!file.is_open()) {
        ErrorL << "Failed to open file for writing: " << filepath;
        return false;
    }
    
    stringstream ss;
    ss << "{\"rules\":[";
    
    lock_guard<recursive_mutex> lock(_mutex);
    size_t i = 0;
    for (const auto &pair : _rules) {
        ss << pair.second.toJson();
        if (++i < _rules.size()) ss << ",";
    }
    
    ss << "]}";
    file << ss.str();
    return true;
}
```

### 4. SIMD优化示例

```cpp
// YoloDetector.cpp - 前处理SIMD优化

bool OnnxYoloDetector::preprocess(const FrameData &input, float *output) {
    #ifdef __AVX2__
    // AVX2优化版本
    const uint8_t *src = input.data;
    __m256 scale = _mm256_set1_ps(1.0f / 255.0f);
    
    for (int i = 0; i < total_pixels; i += 8) {
        __m256i pixels = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i*)(src + i)));
        __m256 pixels_f = _mm256_cvtepi32_ps(pixels);
        __m256 normalized = _mm256_mul_ps(pixels_f, scale);
        _mm256_store_ps(output + i, normalized);
    }
    #else
    // 普通版本
    for (int i = 0; i < total_pixels; ++i) {
        output[i] = src[i] / 255.0f;
    }
    #endif
    
    return true;
}
```

## 🎯 快速完善脚本

创建辅助脚本自动生成JSON代码：

```python
# generate_json.py
def generate_to_json(class_name, fields):
    code = f'''
string {class_name}::toJson() const {{
    stringstream ss;
    ss << "{{";
'''
    for i, (name, type_) in enumerate(fields):
        last = i == len(fields) - 1
        if type_ == 'string':
            code += f'    ss << "\\\"{name}\\\":\\\\"" << {name} << "\\\\"'
        else:
            code += f'    ss << "\\\"{name}\\":" << {name}'
        if not last:
            code += ' << ","'
        code += ';\n'
    code += '''    ss << "}";
    return ss.str();
}
'''
    return code

# 使用示例
fields = [
    ('model_path', 'string'),
    ('device_id', 'int'),
    ('batch_size', 'int')
]
print(generate_to_json('InferenceConfig', fields))
```

## 📈 预计工作量

| 任务 | 工时 | 优先级 |
|------|------|--------|
| JSON序列化（所有类） | 8h | P0 |
| 统计信息JSON化 | 4h | P0 |
| 文件加载/保存 | 3h | P0 |
| SIMD优化 | 6h | P1 |
| GPU批处理优化 | 8h | P1 |
| 目标跟踪 | 16h | P2 |
| **总计** | **45h** | |

## ✅ 建议的完善顺序

1. **JsonHelper实现** (已完成) ✅
2. **核心配置JSON** (InferenceConfig, YoloConfig, AlertRule)
3. **统计信息JSON** (所有getStatistics())
4. **文件操作** (load/save)
5. **性能优化** (SIMD, GPU批处理)
6. **高级功能** (目标跟踪)
7. **测试验证** (单元测试、集成测试)

## 🔗 相关文档

- `JsonHelper.h` - 已创建
- `UpgradePlan.md` - 总体计划
- `AI_DEPENDENCIES.md` - 依赖说明
- `INTEGRATION_GUIDE.md` - 集成指南
