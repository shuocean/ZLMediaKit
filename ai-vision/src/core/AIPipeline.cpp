/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "AIPipeline.h"
#include "AITaskManager.h"
#include "ModelRegistry.h"
#include "Util/logger.h"
#include "Util/util.h"
#include <chrono>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// ==================== PipelineConfig ====================

bool PipelineConfig::isValid() const {
    return !stream_id.empty() && queue_size > 0;
}

bool PipelineConfig::fromJson(const string &json_str) {
    // TODO: Phase 3 - JSON parsing
    return false;
}

string PipelineConfig::toJson() const {
    // TODO: Phase 3 - JSON serialization
    return "{}";
}

// ==================== AIPipeline ====================

AIPipeline::Ptr AIPipeline::create(const PipelineConfig &config) {
    if (!config.isValid()) {
        ErrorL << "Invalid pipeline config";
        return nullptr;
    }
    
    auto pipeline = make_shared<StandardAIPipeline>(config);
    if (!pipeline->initialize()) {
        ErrorL << "Failed to initialize pipeline";
        return nullptr;
    }
    
    return pipeline;
}

// ==================== StandardAIPipeline ====================

struct StandardAIPipeline::Impl {
    FrameConverter::Ptr converter;
    GpuUploader::Ptr uploader;
    OnResultCallback result_callback;
    
    atomic<bool> running{false};
    atomic<uint64_t> frame_counter{0};
    
    mutex callback_mutex;
    
    bool initialized = false;
};

StandardAIPipeline::StandardAIPipeline(const PipelineConfig &config)
    : _impl(new Impl()), _config(config) {
}

StandardAIPipeline::~StandardAIPipeline() {
    stop();
}

bool StandardAIPipeline::initialize() {
    // 创建格式转换器
    if (_config.enable_converter) {
        _impl->converter = FrameConverter::create(_config.converter);
        if (!_impl->converter) {
            WarnL << "Converter creation failed, will skip conversion";
        }
    }
    
    // 创建GPU上传器
    if (_config.enable_gpu_upload) {
        _impl->uploader = GpuUploader::create(_config.uploader);
        if (!_impl->uploader || !_impl->uploader->isGpuAvailable()) {
            WarnL << "GPU uploader not available, using CPU mode";
        }
    }
    
    _impl->initialized = true;
    InfoL << "AIPipeline initialized for stream: " << _config.stream_id;
    
    return true;
}

bool StandardAIPipeline::processFrame(const FrameData &frame) {
    if (!_impl->initialized || !_impl->running) {
        return false;
    }
    
    auto start = chrono::high_resolution_clock::now();
    
    _stats.total_frames++;
    uint64_t current_frame = _impl->frame_counter++;
    
    // 跳帧策略（性能优化）
    if (_config.skip_frames > 0 && current_frame % (_config.skip_frames + 1) != 0) {
        _stats.skipped_frames++;
        return true;
    }
    
    // TODO: Phase 3 - 实际处理流程
    // 1. 格式转换 (FrameConverter)
    // 2. GPU上传 (GpuUploader)
    // 3. AI推理 (调用AITaskManager)
    // 4. 回调结果
    
    _stats.processed_frames++;
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    _stats.total_time_us += duration.count();
    
    // 计算统计信息
    if (_stats.processed_frames > 0) {
        _stats.avg_latency_ms = _stats.total_time_us / (float)_stats.processed_frames / 1000.0f;
        _stats.skip_ratio = (float)_stats.skipped_frames / _stats.total_frames;
    }
    
    return true;
}

void StandardAIPipeline::setResultCallback(OnResultCallback callback) {
    lock_guard<mutex> lock(_impl->callback_mutex);
    _impl->result_callback = callback;
}

bool StandardAIPipeline::start() {
    if (_impl->running) {
        WarnL << "Pipeline already running";
        return true;
    }
    
    _impl->running = true;
    InfoL << "Pipeline started: " << _config.stream_id;
    return true;
}

void StandardAIPipeline::stop() {
    if (!_impl->running) return;
    
    _impl->running = false;
    
    // 等待GPU上传完成
    if (_impl->uploader) {
        _impl->uploader->waitAll();
    }
    
    InfoL << "Pipeline stopped: " << _config.stream_id;
}

bool StandardAIPipeline::isRunning() const {
    return _impl->running;
}

const PipelineConfig &StandardAIPipeline::getConfig() const {
    return _config;
}

bool StandardAIPipeline::updateConfig(const PipelineConfig &config) {
    if (!config.isValid()) {
        ErrorL << "Invalid pipeline config";
        return false;
    }
    
    // TODO: Phase 3 - 热更新支持
    _config = config;
    InfoL << "Pipeline config updated: " << _config.stream_id;
    return true;
}

string StandardAIPipeline::getStatistics() const {
    // TODO: Phase 3 - JSON序列化
    return "{}";
}

void StandardAIPipeline::resetStatistics() {
    _stats = Statistics();
    _impl->frame_counter = 0;
}

// ==================== PipelineManager ====================

PipelineManager &PipelineManager::Instance() {
    static PipelineManager instance;
    return instance;
}

AIPipeline::Ptr PipelineManager::createPipeline(const string &stream_id, 
                                                 const PipelineConfig &config) {
    lock_guard<recursive_mutex> lock(_mutex);
    
    // 检查是否已存在
    auto it = _pipelines.find(stream_id);
    if (it != _pipelines.end()) {
        WarnL << "Pipeline already exists for stream: " << stream_id;
        return it->second;
    }
    
    // 创建新Pipeline
    auto pipeline = AIPipeline::create(config);
    if (!pipeline) {
        ErrorL << "Failed to create pipeline for stream: " << stream_id;
        return nullptr;
    }
    
    _pipelines[stream_id] = pipeline;
    InfoL << "Pipeline created for stream: " << stream_id;
    
    return pipeline;
}

AIPipeline::Ptr PipelineManager::getPipeline(const string &stream_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _pipelines.find(stream_id);
    return it != _pipelines.end() ? it->second : nullptr;
}

bool PipelineManager::removePipeline(const string &stream_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    
    auto it = _pipelines.find(stream_id);
    if (it == _pipelines.end()) {
        return false;
    }
    
    // 停止Pipeline
    if (it->second) {
        it->second->stop();
    }
    
    _pipelines.erase(it);
    InfoL << "Pipeline removed for stream: " << stream_id;
    
    return true;
}

vector<string> PipelineManager::getAllStreamIds() const {
    lock_guard<recursive_mutex> lock(_mutex);
    vector<string> ids;
    for (const auto &pair : _pipelines) {
        ids.push_back(pair.first);
    }
    return ids;
}

size_t PipelineManager::getPipelineCount() const {
    lock_guard<recursive_mutex> lock(_mutex);
    return _pipelines.size();
}

string PipelineManager::getGlobalStatistics() const {
    // TODO: Phase 3 - 全局统计
    return "{}";
}

void PipelineManager::clear() {
    lock_guard<recursive_mutex> lock(_mutex);
    
    // 停止所有Pipeline
    for (auto &pair : _pipelines) {
        if (pair.second) {
            pair.second->stop();
        }
    }
    
    _pipelines.clear();
    InfoL << "All pipelines cleared";
}

} // namespace ai
} // namespace mediakit
