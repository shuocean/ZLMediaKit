/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "InferenceEngine.h"
#include "JsonHelper.h"
#include "Util/logger.h"
#include <sstream>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// ==================== InferenceConfig ====================

bool InferenceConfig::fromJson(const string &json_str) {
    // 简化JSON解析实现
    JsonHelper::parseString(json_str, "model_path", model_path);
    
    int provider_int = 0;
    JsonHelper::parseInt(json_str, "provider", provider_int);
    provider = static_cast<ExecutionProvider>(provider_int);
    
    JsonHelper::parseInt(json_str, "device_id", device_id);
    JsonHelper::parseInt(json_str, "batch_size", batch_size);
    JsonHelper::parseBool(json_str, "enable_profiling", enable_profiling);
    
    InfoL << "InferenceConfig loaded from JSON";
    return true;
}

string InferenceConfig::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("model_path", model_path);
    ss << JsonHelper::field("provider", static_cast<int>(provider));
    ss << JsonHelper::field("device_id", device_id);
    ss << JsonHelper::field("batch_size", batch_size);
    ss << JsonHelper::field("enable_profiling", enable_profiling, true);
    ss << JsonHelper::objectEnd();
    return ss.str();
}

// ==================== InferenceEngine ====================

InferenceEngine::Ptr InferenceEngine::create(const InferenceConfig &config) {
    if (config.model_path.empty()) {
        ErrorL << "Model path is empty";
        return nullptr;
    }
    
    // 根据provider创建对应的引擎
    switch (config.provider) {
        case ExecutionProvider::CUDA:
        case ExecutionProvider::TensorRT:
        case ExecutionProvider::CPU:
        case ExecutionProvider::Auto:
            InfoL << "Creating inference engine with provider: " << (int)config.provider;
            // TODO: 返回实际的OnnxInferenceEngine实例
            // return make_shared<OnnxInferenceEngine>(config);
            return nullptr;
        default:
            ErrorL << "Unsupported provider: " << (int)config.provider;
            return nullptr;
    }
}

} // namespace ai
} // namespace mediakit
