/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "AlertEngine.h"
#include "Util/logger.h"
#include "Util/util.h"
#include <chrono>
#include <sstream>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// 获取当前时间戳（毫秒）
static uint64_t getCurrentTimeMs() {
    return chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
}

// 生成UUID（简化版）
static string generateUUID() {
    stringstream ss;
    ss << getCurrentTimeMs() << "-" << rand();
    return ss.str();
}

bool AlertRule::fromJson(const string &json_str) {
    // TODO: 完整JSON解析（Phase 4后期）
    return false;
}

string AlertRule::toJson() const {
    // TODO: 完整JSON序列化（Phase 4后期）
    return "{}";
}

string AlertEvent::toJson() const {
    // TODO: 完整JSON序列化（Phase 4后期）
    stringstream ss;
    ss << "{"
       << "\"alert_id\":\"" << alert_id << "\","
       << "\"rule_id\":\"" << rule_id << "\","
       << "\"stream_id\":\"" << stream_id << "\","
       << "\"timestamp\":" << timestamp
       << "}";
    return ss.str();
}

AlertEngine &AlertEngine::Instance() {
    static AlertEngine instance;
    return instance;
}

bool AlertEngine::registerRule(const AlertRule &rule) {
    lock_guard<recursive_mutex> lock(_mutex);
    _rules[rule.rule_id] = rule;
    InfoL << "Alert rule registered: " << rule.rule_id;
    return true;
}

bool AlertEngine::unregisterRule(const string &rule_id) {
    lock_guard<recursive_mutex> lock(_mutex);
    _rules.erase(rule_id);
    _last_alert_time.erase(rule_id);
    _alert_count_per_minute.erase(rule_id);
    InfoL << "Alert rule unregistered: " << rule_id;
    return true;
}

AlertRule AlertEngine::getRule(const string &rule_id) const {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _rules.find(rule_id);
    return it != _rules.end() ? it->second : AlertRule();
}

bool AlertEngine::updateRule(const string &rule_id, const AlertRule &rule) {
    lock_guard<recursive_mutex> lock(_mutex);
    if (_rules.find(rule_id) == _rules.end()) {
        return false;
    }
    _rules[rule_id] = rule;
    InfoL << "Alert rule updated: " << rule_id;
    return true;
}

bool AlertEngine::enableRule(const string &rule_id, bool enable) {
    lock_guard<recursive_mutex> lock(_mutex);
    auto it = _rules.find(rule_id);
    if (it == _rules.end()) {
        return false;
    }
    it->second.enabled = enable;
    InfoL << "Alert rule " << (enable ? "enabled" : "disabled") << ": " << rule_id;
    return true;
}

vector<string> AlertEngine::getAllRuleIds() const {
    lock_guard<recursive_mutex> lock(_mutex);
    vector<string> ids;
    for (const auto &pair : _rules) {
        ids.push_back(pair.first);
    }
    return ids;
}

void AlertEngine::processDetection(const DetectionResult::Ptr &detection) {
    lock_guard<recursive_mutex> lock(_mutex);
    // TODO: Phase 4 - Rule matching and alert triggering
    for (const auto &pair : _rules) {
        const auto &rule = pair.second;
        if (!rule.enabled) continue;
        
        if (matchRule(rule, detection)) {
            if (!isInCooldown(rule.rule_id)) {
                triggerAlert(rule, detection);
            }
        }
    }
}

void AlertEngine::setAlertCallback(OnAlertCallback callback) {
    lock_guard<recursive_mutex> lock(_mutex);
    _alert_callback = callback;
}

int AlertEngine::loadFromFile(const string &filepath) {
    // TODO: Phase 4 - Load rules from JSON file
    return 0;
}

bool AlertEngine::saveToFile(const string &filepath) const {
    // TODO: Phase 4 - Save rules to JSON file
    return false;
}

string AlertEngine::getRuleStatistics(const string &rule_id) const {
    // TODO: Phase 4 - Rule statistics
    return "{}";
}

string AlertEngine::getGlobalStatistics() const {
    // TODO: Phase 4 - Global statistics
    return "{}";
}

void AlertEngine::resetStatistics(const string &rule_id) {
    // TODO: Phase 4 - Reset statistics
}

void AlertEngine::clear() {
    lock_guard<recursive_mutex> lock(_mutex);
    _rules.clear();
    _last_alert_time.clear();
    _alert_count_per_minute.clear();
    _alert_callback = nullptr;
    InfoL << "AlertEngine cleared";
}

bool AlertEngine::matchRule(const AlertRule &rule, const DetectionResult::Ptr &detection) {
    if (!detection || detection->boxes.empty()) {
        return false;
    }
    
    // 过滤目标类别
    vector<DetectionBox> filtered_boxes;
    if (rule.target_classes.empty()) {
        // 空表示所有类别
        filtered_boxes = detection->boxes;
    } else {
        // 过滤指定类别
        for (const auto &box : detection->boxes) {
            for (int class_id : rule.target_classes) {
                if (box.class_id == class_id && box.confidence >= rule.min_confidence) {
                    filtered_boxes.push_back(box);
                    break;
                }
            }
        }
    }
    
    if (filtered_boxes.empty()) {
        return false;
    }
    
    // 根据条件类型匹配
    switch (rule.condition_type) {
        case AlertConditionType::ObjectDetected:
            // 检测到目标即触发
            return !filtered_boxes.empty();
            
        case AlertConditionType::ObjectInROI:
            // ROI区域检测
            if (!rule.roi.enabled) return false;
            for (const auto &box : filtered_boxes) {
                float cx, cy;
                box.getCenter(cx, cy);
                if (cx >= rule.roi.x && cx <= rule.roi.x + rule.roi.w &&
                    cy >= rule.roi.y && cy <= rule.roi.y + rule.roi.h) {
                    return true;
                }
            }
            return false;
            
        case AlertConditionType::ObjectCount:
            // 数量检测
            {
                int count = (int)filtered_boxes.size();
                bool min_ok = (rule.min_count <= 0) || (count >= rule.min_count);
                bool max_ok = (rule.max_count <= 0) || (count <= rule.max_count);
                return min_ok && max_ok;
            }
            
        case AlertConditionType::ObjectStay:
            // 停留检测（需要跟踪，暂不实现）
            // TODO: 需要目标跟踪支持
            return false;
            
        case AlertConditionType::ObjectCrossLine:
            // 越线检测（需要跟踪，暂不实现）
            // TODO: 需要目标跟踪支持
            return false;
            
        default:
            return false;
    }
}

bool AlertEngine::isInCooldown(const string &rule_id) const {
    auto it = _last_alert_time.find(rule_id);
    if (it == _last_alert_time.end()) {
        return false; // 从未触发过
    }
    
    auto rule_it = _rules.find(rule_id);
    if (rule_it == _rules.end()) {
        return false;
    }
    
    uint64_t now = getCurrentTimeMs();
    uint64_t elapsed_ms = now - it->second;
    uint64_t cooldown_ms = rule_it->second.cooldown_seconds * 1000;
    
    return elapsed_ms < cooldown_ms;
}

void AlertEngine::triggerAlert(const AlertRule &rule, const DetectionResult::Ptr &detection) {
    // 检查限流（每分钟最大告警数）
    uint64_t now = getCurrentTimeMs();
    uint64_t minute_ago = now - 60000;
    
    // 清理过期的计数（简化实现）
    auto &count = _alert_count_per_minute[rule.rule_id];
    if (count >= rule.max_alerts_per_minute) {
        WarnL << "Alert rate limit reached for rule: " << rule.rule_id;
        return;
    }
    
    // 创建告警事件
    AlertEvent event;
    event.alert_id = generateUUID();
    event.rule_id = rule.rule_id;
    event.stream_id = detection->stream_id;
    event.timestamp = now;
    event.condition_type = rule.condition_type;
    event.detection = detection;
    event.metadata = rule.metadata;
    
    // 过滤触发告警的检测框
    for (const auto &box : detection->boxes) {
        if (rule.target_classes.empty() || 
            find(rule.target_classes.begin(), rule.target_classes.end(), box.class_id) != rule.target_classes.end()) {
            if (box.confidence >= rule.min_confidence) {
                event.trigger_boxes.push_back(box);
            }
        }
    }
    
    // 更新最后告警时间和计数
    _last_alert_time[rule.rule_id] = now;
    count++;
    
    // 触发回调
    if (_alert_callback) {
        try {
            _alert_callback(event);
        } catch (const exception &e) {
            ErrorL << "Alert callback error: " << e.what();
        }
    }
    
    InfoL << "Alert triggered: " << rule.rule_id 
          << ", stream: " << detection->stream_id
          << ", objects: " << event.trigger_boxes.size();
}

} // namespace ai
} // namespace mediakit
