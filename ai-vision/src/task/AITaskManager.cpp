/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "AITaskManager.h"
#include "Util/logger.h"

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

bool AITaskConfig::fromJson(const string &json_str) {
    // TODO: Phase 1.5 - JSON parsing
    return false;
}

string AITaskConfig::toJson() const {
    // TODO: Phase 1.5 - JSON serialization
    return "{}";
}

string StreamTaskBinding::toJson() const {
    return "{}";
}

bool StreamTaskBinding::fromJson(const string &json_str) {
    return false;
}

AITaskManager &AITaskManager::Instance() {
    static AITaskManager instance;
    return instance;
}

bool AITaskManager::registerTask(const string &task_id, const AITaskConfig &config) {
    lock_guard<recursive_mutex> lock(_mutex);
    _tasks[task_id] = config;
    InfoL << "AI Task registered: " << task_id;
    return true;
}

bool AITaskManager::unregisterTask(const string &task_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    
    // Remove all bindings
    for (auto &pair : _stream_bindings) {
        pair.second.erase(task_id);
    }
    _task_bindings.erase(task_id);
    _tasks.erase(task_id);
    
    InfoL << "AI Task unregistered: " << task_id;
    return true;
}

AITaskConfig AITaskManager::getTaskConfig(const string &task_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _tasks.find(task_id);
    return it != _tasks.end() ? it->second : AITaskConfig();
}

bool AITaskManager::updateTaskConfig(const string &task_id, const AITaskConfig &config) {
    lock_guard<recursive_mutex> lock(_mutex);
    if (_tasks.find(task_id) == _tasks.end()) {
        return false;
    }
    _tasks[task_id] = config;
    InfoL << "AI Task config updated: " << task_id;
    return true;
}

bool AITaskManager::updateTaskThreshold(const string &task_id,
                                        float conf_threshold,
                                        float nms_threshold) {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _tasks.find(task_id);
    if (it == _tasks.end()) {
        return false;
    }
    it->second.conf_threshold = conf_threshold;
    it->second.nms_threshold = nms_threshold;
    InfoL << "Task threshold updated: " << task_id 
          << ", conf=" << conf_threshold
          << ", nms=" << nms_threshold;
    return true;
}

bool AITaskManager::enableTask(const string &task_id, bool enable) {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _tasks.find(task_id);
    if (it == _tasks.end()) {
        return false;
    }
    it->second.enabled = enable;
    InfoL << "Task " << (enable ? "enabled" : "disabled") << ": " << task_id;
    return true;
}

bool AITaskManager::isTaskEnabled(const string &task_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _tasks.find(task_id);
    return it != _tasks.end() && it->second.enabled;
}

vector<string> AITaskManager::getAllTaskIds() const {
    lock_guard<recursive_mutex> lock(_mutex);
    vector<string> ids;
    for (const auto &pair : _tasks) {
        ids.push_back(pair.first);
    }
    return ids;
}

bool AITaskManager::bindTaskToStream(const string &stream_id, const string &task_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    if (_tasks.find(task_id) == _tasks.end()) {
        WarnL << "Task not found: " << task_id;
        return false;
    }
    
    _stream_bindings[stream_id].insert(task_id);
    _task_bindings[task_id].insert(stream_id);
    InfoL << "Task bound: " << stream_id << " -> " << task_id;
    return true;
}

bool AITaskManager::unbindTaskFromStream(const string &stream_id, const string &task_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    _stream_bindings[stream_id].erase(task_id);
    _task_bindings[task_id].erase(stream_id);
    InfoL << "Task unbound: " << stream_id << " -> " << task_id;
    return true;
}

vector<string> AITaskManager::getStreamTasks(const string &stream_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    vector<string> tasks;
    auto it = _stream_bindings.find(stream_id);
    if (it != _stream_bindings.end()) {
        tasks.assign(it->second.begin(), it->second.end());
    }
    return tasks;
}

vector<string> AITaskManager::getTaskStreams(const string &task_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    vector<string> streams;
    auto it = _task_bindings.find(task_id);
    if (it != _task_bindings.end()) {
        streams.assign(it->second.begin(), it->second.end());
    }
    return streams;
}

void AITaskManager::clearStreamBindings(const string &stream_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _stream_bindings.find(stream_id);
    if (it != _stream_bindings.end()) {
        for (const auto &task_id : it->second) {
            _task_bindings[task_id].erase(stream_id);
        }
        _stream_bindings.erase(it);
    }
    InfoL << "Stream bindings cleared: " << stream_id;
}

int AITaskManager::loadFromFile(const string &filepath) {
    // TODO: Phase 1.5 - Load from JSON file
    return 0;
}

bool AITaskManager::saveToFile(const string &filepath) const {
    // TODO: Phase 1.5 - Save to JSON file
    return false;
}

void AITaskManager::setDetectionCallback(OnDetectionCallback callback) {
    lock_guard<recursive_mutex> lock(_mutex);
    _detection_callback = callback;
}

string AITaskManager::getTaskStatistics(const string &task_id) const {
    // TODO: Phase 3 - Statistics
    return "{}";
}

string AITaskManager::getGlobalStatistics() const {
    // TODO: Phase 3 - Global statistics
    return "{}";
}

void AITaskManager::resetStatistics(const string &task_id) {
    // TODO: Phase 3 - Reset statistics
}

void AITaskManager::clear() {
    lock_guard<recursive_mutex> lock(_mutex);
    _tasks.clear();
    _stream_bindings.clear();
    _task_bindings.clear();
    _detection_callback = nullptr;
    InfoL << "AITaskManager cleared";
}

} // namespace ai
} // namespace mediakit
