/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "ModelRegistry.h"
#include "Util/logger.h"

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

bool ModelInfo::fromJson(const string &json_str) {
    // TODO: Phase 3 implementation
    return false;
}

string ModelInfo::toJson() const {
    return "{}";
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
    return "{}";
}

int ModelRegistry::loadFromFile(const string &json_file) {
    // TODO: Phase 1.5 implementation
    return 0;
}

bool ModelRegistry::saveToFile(const string &json_file) const {
    // TODO: Phase 1.5 implementation
    return false;
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
