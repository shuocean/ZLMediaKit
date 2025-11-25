/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "ModelRegistry.h"
#include "JsonHelper.h"
#include "Util/logger.h"
#include <sstream>
#include <fstream>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

bool ModelInfo::fromJson(const string &json_str) {
    JsonHelper::parseString(json_str, "model_id", model_id);
    JsonHelper::parseString(json_str, "model_path", model_path);
    JsonHelper::parseString(json_str, "model_type", model_type);
    JsonHelper::parseString(json_str, "version", version);
    JsonHelper::parseBool(json_str, "enabled", enabled);
    
    // 解析InferenceConfig对象
    string config_str = JsonHelper::extractObject(json_str, "config");
    if (!config_str.empty() && config_str != "{}") {
        config.fromJson(config_str);
    }
    
    InfoL << "ModelInfo loaded from JSON: " << model_id 
          << ", config provider: " << (int)config.provider;
    return true;
}

string ModelInfo::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("model_id", model_id);
    ss << JsonHelper::field("model_path", model_path);
    ss << JsonHelper::field("model_type", model_type);
    ss << JsonHelper::field("version", version);
    ss << JsonHelper::field("enabled", enabled);
    ss << JsonHelper::field("use_count", (int)use_count);
    ss << JsonHelper::fieldObject("config", config.toJson(), true);
    ss << JsonHelper::objectEnd();
    return ss.str();
}

ModelRegistry &ModelRegistry::Instance() {
    static ModelRegistry instance;
    return instance;
}

bool ModelRegistry::registerModel(const string &model_id, const ModelInfo &info) {
    lock_guard<recursive_mutex> lock(_mutex);
    _models[model_id] = info;
    InfoL << "Model registered: " << model_id;
    return true;
}

bool ModelRegistry::unregisterModel(const string &model_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    unloadModel(model_id);
    _models.erase(model_id);
    InfoL << "Model unregistered: " << model_id;
    return true;
}

ModelInfo ModelRegistry::getModelInfo(const string &model_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _models.find(model_id);
    return it != _models.end() ? it->second : ModelInfo();
}

InferenceEngine::Ptr ModelRegistry::getEngine(const string &model_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    // TODO: Phase 3 implementation - load and cache engine
    return nullptr;
}

bool ModelRegistry::warmupModel(const string &model_id) {
    // TODO: Phase 3 implementation
    return false;
}

bool ModelRegistry::unloadModel(const string &model_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    _engines.erase(model_id);
    return true;
}

vector<string> ModelRegistry::getAllModelIds() const {
    lock_guard<recursive_mutex> lock(_mutex);
    vector<string> ids;
    for (const auto &pair : _models) {
        ids.push_back(pair.first);
    }
    return ids;
}

size_t ModelRegistry::getLoadedModelCount() const {
    lock_guard<recursive_mutex> lock(_mutex);
    return _engines.size();
}

string ModelRegistry::getStatistics() const {
    lock_guard<recursive_mutex> lock(_mutex);
    
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("registered_models", (int)_models.size());
    ss << JsonHelper::field("loaded_engines", (int)_engines.size());
    
    // 统计启用/禁用的模型数
    int enabled_count = 0;
    for (const auto &pair : _models) {
        if (pair.second.enabled) enabled_count++;
    }
    ss << JsonHelper::field("enabled_models", enabled_count);
    
    // 模型列表
    ss << "\"models\":[";
    size_t i = 0;
    for (const auto &pair : _models) {
        ss << pair.second.toJson();
        if (++i < _models.size()) ss << ",";
    }
    ss << "]";
    
    ss << JsonHelper::objectEnd();
    return ss.str();
}

int ModelRegistry::loadFromFile(const string &json_file) {
    ifstream file(json_file);
    if (!file.is_open()) {
        ErrorL << "Failed to open model registry file: " << json_file;
        return 0;
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    string json_str = buffer.str();
    file.close();
    
    // 解析JSON文件中的models数组
    string models_array = JsonHelper::extractArray(json_str, "models");
    vector<string> model_objects = JsonHelper::splitObjectArray(models_array);
    
    int loaded_count = 0;
    lock_guard<recursive_mutex> lock(_mutex);
    
    for (const auto &model_str : model_objects) {
        ModelInfo info;
        if (info.fromJson(model_str)) {
            _models[info.model_id] = info;
            loaded_count++;
            InfoL << "Loaded model: " << info.model_id;
        }
    }
    
    InfoL << "Model registry loaded from: " << json_file 
          << ", models: " << loaded_count;
    return loaded_count;
}

bool ModelRegistry::saveToFile(const string &json_file) const {
    ofstream file(json_file);
    if (!file.is_open()) {
        ErrorL << "Failed to open file for writing: " << json_file;
        return false;
    }
    
    file << getStatistics();
    
    InfoL << "Model registry saved to: " << json_file;
    return true;
}

size_t ModelRegistry::evictLRU(size_t target_free_memory) {
    // TODO: Phase 3 implementation
    return 0;
}

void ModelRegistry::clear() {
    lock_guard<recursive_mutex> lock(_mutex);
    _engines.clear();
    _models.clear();
}

} // namespace ai
} // namespace mediakit
