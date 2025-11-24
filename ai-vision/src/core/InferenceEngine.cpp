/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "InferenceEngine.h"
#include "Util/logger.h"
#include "Util/util.h"

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// ==================== InferenceConfig ====================

bool InferenceConfig::fromJson(const string &json_str) {
    // TODO: 实现JSON解析
    InfoL << "InferenceConfig::fromJson - TODO";
    return false;
}

string InferenceConfig::toJson() const {
    // TODO: 实现JSON序列化
    return "{}";
}

// ==================== InferenceEngine ====================

InferenceEngine::Ptr InferenceEngine::create(const InferenceConfig &config) {
    // TODO: 根据config.provider创建对应的引擎实例
    // 目前返回nullptr，Phase 3实现
    WarnL << "InferenceEngine::create - Not implemented yet (Phase 3)";
    return nullptr;
}

} // namespace ai
} // namespace mediakit
