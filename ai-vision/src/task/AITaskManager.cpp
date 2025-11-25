/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "AITaskManager.h"
#include "JsonHelper.h"
#include "Util/logger.h"
#include <sstream>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

bool AITaskConfig::fromJson(const string &json_str) {
    JsonHelper::parseString(json_str, "task_id", task_id);
    JsonHelper::parseString(json_str, "model_id", model_id);
    JsonHelper::parseString(json_str, "model_type", model_type);
    JsonHelper::parseFloat(json_str, "conf_threshold", conf_threshold);
    JsonHelper::parseFloat(json_str, "nms_threshold", nms_threshold);
    JsonHelper::parseInt(json_str, "skip_frames", skip_frames);
    JsonHelper::parseBool(json_str, "enabled", enabled);
    JsonHelper::parseInt(json_str, "priority", priority);
    
    // 解析ROI对象
    string roi_str = JsonHelper::extractObject(json_str, "roi");
    if (!roi_str.empty() && roi_str != "{}") {
        JsonHelper::parseFloat(roi_str, "x", roi.x);
        JsonHelper::parseFloat(roi_str, "y", roi.y);
        JsonHelper::parseFloat(roi_str, "w", roi.w);
        JsonHelper::parseFloat(roi_str, "h", roi.h);
        JsonHelper::parseBool(roi_str, "enabled", roi.enabled);
    }
    
    InfoL << "AITaskConfig loaded from JSON: " << task_id 
          << ", ROI enabled: " << roi.enabled;
    return true;
}

string AITaskConfig::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("task_id", task_id);
    ss << JsonHelper::field("model_id", model_id);
    ss << JsonHelper::field("model_type", model_type);
    ss << JsonHelper::field("conf_threshold", conf_threshold);
    ss << JsonHelper::field("nms_threshold", nms_threshold);
    ss << JsonHelper::field("skip_frames", skip_frames);
    ss << JsonHelper::field("enabled", enabled);
    ss << JsonHelper::field("priority", priority);
    ss << JsonHelper::field("process_count", (int)process_count);
    ss << JsonHelper::field("avg_inference_time_ms", avg_inference_time_ms);
    
    // ROI
    ss << "\"roi\":";
    ss << "{";
    ss << "\"x\":" << roi.x << ",";
    ss << "\"y\":" << roi.y << ",";
    ss << "\"w\":" << roi.w << ",";
    ss << "\"h\":" << roi.h << ",";
    ss << "\"enabled\":" << (roi.enabled ? "true" : "false");
    ss << "}";
    
    ss << JsonHelper::objectEnd();
    return ss.str();
}

string StreamTaskBinding::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("stream_id", stream_id);
    
    ss << "\"task_ids\":[";
    for (size_t i = 0; i < task_ids.size(); ++i) {
        ss << "\"" << task_ids[i] << "\"";
        if (i < task_ids.size() - 1) ss << ",";
    }
    ss << "]";
    
    ss << JsonHelper::objectEnd();
    return ss.str();
}

bool StreamTaskBinding::fromJson(const string &json_str) {
    JsonHelper::parseString(json_str, "stream_id", stream_id);
    
    // 解析task_ids数组
    string array_str = JsonHelper::extractArray(json_str, "task_ids");
    task_ids = JsonHelper::parseStringArray(array_str);
    
    InfoL << "StreamTaskBinding loaded from JSON: " << stream_id 
          << ", tasks: " << task_ids.size();
    return true;
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
