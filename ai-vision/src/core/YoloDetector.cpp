/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "YoloDetector.h"
#include "JsonHelper.h"
#include "Util/logger.h"
#include "Util/util.h"
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// COCO数据集80个类别
static const vector<string> COCO_CLASSES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
    "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
    "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack",
    "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball",
    "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket",
    "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair",
    "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
    "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator",
    "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
};

// ==================== YoloConfig ====================

void YoloConfig::loadCocoClasses() {
    class_names = COCO_CLASSES;
}

bool YoloConfig::isValid() const {
    return !model_path.empty() &&
           input_width > 0 && input_height > 0 &&
           conf_threshold > 0 && conf_threshold < 1 &&
           nms_threshold > 0 && nms_threshold < 1;
}

bool YoloConfig::fromJson(const string &json_str) {
    JsonHelper::parseString(json_str, "model_path", model_path);
    
    int provider_int = 0;
    JsonHelper::parseInt(json_str, "provider", provider_int);
    provider = static_cast<ExecutionProvider>(provider_int);
    
    JsonHelper::parseInt(json_str, "device_id", device_id);
    JsonHelper::parseInt(json_str, "input_width", input_width);
    JsonHelper::parseInt(json_str, "input_height", input_height);
    JsonHelper::parseBool(json_str, "normalize", normalize);
    JsonHelper::parseFloat(json_str, "conf_threshold", conf_threshold);
    JsonHelper::parseFloat(json_str, "nms_threshold", nms_threshold);
    JsonHelper::parseInt(json_str, "max_det", max_det);
    JsonHelper::parseInt(json_str, "batch_size", batch_size);
    JsonHelper::parseBool(json_str, "enable_fp16", enable_fp16);
    
    // 解析class_names数组
    string array_str = JsonHelper::extractArray(json_str, "class_names");
    class_names = JsonHelper::parseStringArray(array_str);
    
    InfoL << "YoloConfig loaded from JSON, classes: " << class_names.size();
    return true;
}

string YoloConfig::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("model_path", model_path);
    ss << JsonHelper::field("provider", static_cast<int>(provider));
    ss << JsonHelper::field("device_id", device_id);
    ss << JsonHelper::field("input_width", input_width);
    ss << JsonHelper::field("input_height", input_height);
    ss << JsonHelper::field("normalize", normalize);
    ss << JsonHelper::field("conf_threshold", conf_threshold);
    ss << JsonHelper::field("nms_threshold", nms_threshold);
    ss << JsonHelper::field("max_det", max_det);
    ss << JsonHelper::field("batch_size", batch_size);
    ss << JsonHelper::field("enable_fp16", enable_fp16);
    
    if (!class_names.empty()) {
        ss << JsonHelper::fieldArray("class_names", 
            JsonHelper::arrayString(class_names), true);
    } else {
        ss << JsonHelper::fieldArray("class_names", "[]", true);
    }
    
    ss << JsonHelper::objectEnd();
    return ss.str();
}

// ==================== YoloDetector ====================

YoloDetector::Ptr YoloDetector::create(const YoloConfig &config) {
    if (!config.isValid()) {
        ErrorL << "Invalid YOLO config";
        return nullptr;
    }
    
    auto detector = make_shared<OnnxYoloDetector>(config);
    if (!detector->initialize()) {
        ErrorL << "Failed to initialize YOLO detector";
        return nullptr;
    }
    
    return detector;
}

// ==================== OnnxYoloDetector ====================

struct OnnxYoloDetector::Impl {
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "ZLMediaKit-AI"};
    Ort::SessionOptions session_options;
    unique_ptr<Ort::Session> session;
    
    vector<const char*> input_names;
    vector<const char*> output_names;
    vector<int64_t> input_shape;
    vector<int64_t> output_shape;
    
    bool initialized = false;
    
    // 性能优化：预分配内存
    vector<float> input_tensor_data;
    vector<float> output_tensor_data;
};

OnnxYoloDetector::OnnxYoloDetector(const YoloConfig &config)
    : _impl(new Impl()) {
    _config = config;
    
    // 加载COCO类别（如果未设置）
    if (_config.class_names.empty()) {
        _config.loadCocoClasses();
    }
}

OnnxYoloDetector::~OnnxYoloDetector() {
}

bool OnnxYoloDetector::initialize() {
    try {
        // 配置Session选项
        _impl->session_options.SetIntraOpNumThreads(4);
        _impl->session_options.SetGraphOptimizationLevel(
            GraphOptimizationLevel::ORT_ENABLE_ALL);
        
        // 配置执行提供器
        if (_config.provider == ExecutionProvider::CUDA || 
            _config.provider == ExecutionProvider::Auto) {
#ifdef ENABLE_CUDA
            try {
                OrtCUDAProviderOptions cuda_options;
                cuda_options.device_id = _config.device_id;
                _impl->session_options.AppendExecutionProvider_CUDA(cuda_options);
                InfoL << "Using CUDA provider for YOLO, device: " << _config.device_id;
            } catch (...) {
                WarnL << "CUDA provider not available, falling back to CPU";
            }
#endif
        }
        
        // 加载模型
        #ifdef _WIN32
        wstring model_path_w(model_path.begin(), model_path.end());
        _impl->session = make_unique<Ort::Session>(_impl->env, model_path_w.c_str(),
                                                   _impl->session_options);
        #else
        _impl->session = make_unique<Ort::Session>(_impl->env, _config.model_path.c_str(),
                                                   _impl->session_options);
        #endif
        
        // 获取输入输出信息
        Ort::AllocatorWithDefaultOptions allocator;
        
        // 输入
        size_t num_input_nodes = _impl->session->GetInputCount();
        if (num_input_nodes > 0) {
            auto input_name = _impl->session->GetInputNameAllocated(0, allocator);
            _impl->input_names.push_back(input_name.get());
            
            auto input_type_info = _impl->session->GetInputTypeInfo(0);
            auto tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
            _impl->input_shape = tensor_info.GetShape();
            
            // 更新输入形状（支持动态batch）
            if (_impl->input_shape[0] == -1) {
                _impl->input_shape[0] = _config.batch_size;
            }
        }
        
        // 输出
        size_t num_output_nodes = _impl->session->GetOutputCount();
        if (num_output_nodes > 0) {
            auto output_name = _impl->session->GetOutputNameAllocated(0, allocator);
            _impl->output_names.push_back(output_name.get());
            
            auto output_type_info = _impl->session->GetOutputTypeInfo(0);
            auto tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
            _impl->output_shape = tensor_info.GetShape();
        }
        
        // 预分配内存（性能优化）
        size_t input_size = _config.batch_size * 3 * _config.input_height * _config.input_width;
        _impl->input_tensor_data.resize(input_size);
        
        _impl->initialized = true;
        InfoL << "YOLO detector initialized: " << _config.model_path;
        InfoL << "  Input shape: [" << _impl->input_shape[0] << ", " 
              << _impl->input_shape[1] << ", " << _impl->input_shape[2] << ", "
              << _impl->input_shape[3] << "]";
        
        return true;
        
    } catch (const Ort::Exception &e) {
        ErrorL << "ONNX Runtime error: " << e.what();
        return false;
    } catch (const std::exception &e) {
        ErrorL << "Error initializing YOLO: " << e.what();
        return false;
    }
}

bool OnnxYoloDetector::isReady() const {
    return _impl->initialized;
}

DetectionResult::Ptr OnnxYoloDetector::detect(const FrameData &frame_data) {
    if (!_impl->initialized) {
        return nullptr;
    }
    
    auto start = chrono::high_resolution_clock::now();
    auto result = make_shared<DetectionResult>();
    
    try {
        // 1. 前处理
        auto prep_start = chrono::high_resolution_clock::now();
        if (!preprocess(frame_data, _impl->input_tensor_data.data())) {
            ErrorL << "Preprocess failed";
            return nullptr;
        }
        auto prep_end = chrono::high_resolution_clock::now();
        _stats.preprocess_time_us += 
            chrono::duration_cast<chrono::microseconds>(prep_end - prep_start).count();
        
        // 2. 推理
        auto infer_start = chrono::high_resolution_clock::now();
        
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault);
        
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            _impl->input_tensor_data.data(),
            _impl->input_tensor_data.size(),
            _impl->input_shape.data(),
            _impl->input_shape.size()
        );
        
        auto output_tensors = _impl->session->Run(
            Ort::RunOptions{nullptr},
            _impl->input_names.data(),
            &input_tensor,
            1,
            _impl->output_names.data(),
            1
        );
        
        auto infer_end = chrono::high_resolution_clock::now();
        _stats.inference_time_us += 
            chrono::duration_cast<chrono::microseconds>(infer_end - infer_start).count();
        
        // 3. 后处理
        auto post_start = chrono::high_resolution_clock::now();
        
        float *output_data = output_tensors[0].GetTensorMutableData<float>();
        result = postprocess(output_data, frame_data.width, frame_data.height);
        
        auto post_end = chrono::high_resolution_clock::now();
        _stats.postprocess_time_us += 
            chrono::duration_cast<chrono::microseconds>(post_end - post_start).count();
        
        // 填充结果元数据
        result->width = frame_data.width;
        result->height = frame_data.height;
        result->timestamp = frame_data.pts;
        result->model_id = _config.model_path;
        
        auto end = chrono::high_resolution_clock::now();
        result->inference_time_ms = 
            chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0f;
        
        // 更新统计
        _stats.detect_count++;
        _stats.total_time_us += 
            chrono::duration_cast<chrono::microseconds>(end - start).count();
        _stats.avg_latency_ms = _stats.total_time_us / (float)_stats.detect_count / 1000.0f;
        _stats.avg_fps = 1000000.0f / (_stats.total_time_us / (float)_stats.detect_count);
        
        return result;
        
    } catch (const Ort::Exception &e) {
        ErrorL << "ONNX inference error: " << e.what();
        return nullptr;
    }
}

DetectionResult::Ptr OnnxYoloDetector::detectGpu(const GpuFrame::Ptr &gpu_frame) {
    // TODO: Phase 3后期 - GPU直接推理
    // 目前降级到CPU模式
    return nullptr;
}

int OnnxYoloDetector::detectBatch(const FrameData *frames,
                                  DetectionResult::Ptr *results,
                                  int count) {
    // TODO: Phase 3后期 - 批处理优化
    int success = 0;
    for (int i = 0; i < count; ++i) {
        results[i] = detect(frames[i]);
        if (results[i]) success++;
    }
    return success;
}

int OnnxYoloDetector::detectBatchGpu(const GpuFrame::Ptr *gpu_frames,
                                     DetectionResult::Ptr *results,
                                     int count) {
    // TODO: Phase 3后期 - GPU批处理
    return 0;
}

bool OnnxYoloDetector::updateThresholds(float conf_threshold, float nms_threshold) {
    if (conf_threshold <= 0 || conf_threshold >= 1 ||
        nms_threshold <= 0 || nms_threshold >= 1) {
        return false;
    }
    
    _config.conf_threshold = conf_threshold;
    _config.nms_threshold = nms_threshold;
    
    InfoL << "YOLO thresholds updated: conf=" << conf_threshold 
          << ", nms=" << nms_threshold;
    return true;
}

const YoloConfig &OnnxYoloDetector::getConfig() const {
    return _config;
}

string OnnxYoloDetector::getStatistics() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("detect_count", (int)_stats.detect_count);
    ss << JsonHelper::field("total_time_us", (int)_stats.total_time_us);
    ss << JsonHelper::field("preprocess_time_us", (int)_stats.preprocess_time_us);
    ss << JsonHelper::field("inference_time_us", (int)_stats.inference_time_us);
    ss << JsonHelper::field("postprocess_time_us", (int)_stats.postprocess_time_us);
    ss << JsonHelper::field("avg_fps", _stats.avg_fps);
    ss << JsonHelper::field("avg_latency_ms", _stats.avg_latency_ms);
    ss << JsonHelper::field("avg_detections", _stats.avg_detections, true);
    ss << JsonHelper::objectEnd();
    return ss.str();
}

void OnnxYoloDetector::resetStatistics() {
    _stats = Statistics();
}

// ==================== 前处理 ====================

bool OnnxYoloDetector::preprocess(const FrameData &input, float *output) {
    // 输入: RGB24, 输出: CHW格式, 归一化[0,1]
    int input_h = _config.input_height;
    int input_w = _config.input_width;
    
    // TODO: Phase 3完善 - SIMD优化
    // 简化版本：直接拷贝和归一化
    
    if (input.width == input_w && input.height == input_h) {
        // 无需缩放，直接归一化
        const uint8_t *src = input.data;
        for (int c = 0; c < 3; ++c) {
            for (int h = 0; h < input_h; ++h) {
                for (int w = 0; w < input_w; ++w) {
                    int dst_idx = c * input_h * input_w + h * input_w + w;
                    int src_idx = (h * input_w + w) * 3 + c;
                    output[dst_idx] = src[src_idx] / 255.0f;
                }
            }
        }
    } else {
        // TODO: 需要缩放（使用FrameConverter）
        WarnL << "Image resize not implemented yet";
        return false;
    }
    
    return true;
}

// ==================== 后处理 ====================

DetectionResult::Ptr OnnxYoloDetector::postprocess(float *output, int orig_w, int orig_h) {
    auto result = make_shared<DetectionResult>();
    vector<DetectionBox> boxes;
    
    // YOLOv8输出格式: [1, 84, 8400] 或 [1, num_classes+4, anchors]
    // 84 = 4(bbox) + 80(classes)
    
    int num_classes = _config.class_names.size();
    int num_anchors = 8400; // YOLOv8默认
    
    // 解析检测框
    for (int i = 0; i < num_anchors; ++i) {
        // 获取最大类别置信度
        float max_conf = 0;
        int max_class = 0;
        
        for (int c = 0; c < num_classes; ++c) {
            int idx = (4 + c) * num_anchors + i;
            float conf = output[idx];
            if (conf > max_conf) {
                max_conf = conf;
                max_class = c;
            }
        }
        
        // 过滤低置信度
        if (max_conf < _config.conf_threshold) {
            continue;
        }
        
        // 获取边界框坐标 (cx, cy, w, h)
        float cx = output[0 * num_anchors + i];
        float cy = output[1 * num_anchors + i];
        float w = output[2 * num_anchors + i];
        float h = output[3 * num_anchors + i];
        
        // 转换为 (x, y, w, h) 格式
        float x = cx - w / 2;
        float y = cy - h / 2;
        
        // 归一化坐标
        DetectionBox box;
        box.x = x / _config.input_width;
        box.y = y / _config.input_height;
        box.w = w / _config.input_width;
        box.h = h / _config.input_height;
        box.confidence = max_conf;
        box.class_id = max_class;
        box.label = max_class < (int)_config.class_names.size() ?
                    _config.class_names[max_class] : "unknown";
        
        boxes.push_back(box);
    }
    
    // NMS非极大值抑制
    result->boxes = nms(boxes, _config.nms_threshold);
    
    return result;
}

// ==================== NMS ====================

vector<DetectionBox> OnnxYoloDetector::nms(const vector<DetectionBox> &boxes,
                                          float nms_threshold) {
    if (boxes.empty()) {
        return {};
    }
    
    // 按置信度降序排序
    vector<DetectionBox> sorted_boxes = boxes;
    sort(sorted_boxes.begin(), sorted_boxes.end(),
         [](const DetectionBox &a, const DetectionBox &b) {
             return a.confidence > b.confidence;
         });
    
    vector<bool> suppressed(sorted_boxes.size(), false);
    vector<DetectionBox> result;
    
    for (size_t i = 0; i < sorted_boxes.size(); ++i) {
        if (suppressed[i]) continue;
        
        result.push_back(sorted_boxes[i]);
        
        // 抑制重叠框
        for (size_t j = i + 1; j < sorted_boxes.size(); ++j) {
            if (suppressed[j]) continue;
            
            // 计算IoU
            float iou = sorted_boxes[i].iou(sorted_boxes[j]);
            if (iou > nms_threshold) {
                suppressed[j] = true;
            }
        }
    }
    
    return result;
}

} // namespace ai
} // namespace mediakit
