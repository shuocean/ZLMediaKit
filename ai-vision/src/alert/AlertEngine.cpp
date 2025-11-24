/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "AlertEngine.h"
#include "Util/logger.h"

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

bool AlertRule::fromJson(const string &json_str) {
    // TODO: Phase 4 - JSON parsing
    return false;
}

string AlertRule::toJson() const {
    // TODO: Phase 4 - JSON serialization
    return "{}";
}

string AlertEvent::toJson() const {
    // TODO: Phase 4 - JSON serialization
    return "{}";
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
    // TODO: Phase 4 - Detailed rule matching logic
    return false;
}

bool AlertEngine::isInCooldown(const string &rule_id) const {
    // TODO: Phase 4 - Cooldown check
    return false;
}

void AlertEngine::triggerAlert(const AlertRule &rule, const DetectionResult::Ptr &detection) {
    // TODO: Phase 4 - Alert triggering and callback
    InfoL << "Alert triggered: " << rule.rule_id;
}

} // namespace ai
} // namespace mediakit
