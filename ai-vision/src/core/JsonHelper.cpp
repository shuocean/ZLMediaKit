/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "JsonHelper.h"
#include <algorithm>
#include <cctype>

namespace mediakit {
namespace ai {

// 简化的JSON解析实现
// 注意：这是轻量级实现，仅支持简单的key-value解析
// 生产环境建议使用nlohmann/json等成熟库

static std::string trim(const std::string &str) {
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(str[start])) start++;
    while (end > start && std::isspace(str[end - 1])) end--;
    
    return str.substr(start, end - start);
}

static std::string extractValue(const std::string &json, const std::string &key) {
    // 查找 "key": value 或 "key":value
    std::string search_pattern = "\"" + key + "\"";
    size_t pos = json.find(search_pattern);
    
    if (pos == std::string::npos) {
        return "";
    }
    
    // 找到冒号
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return "";
    }
    
    pos++; // 跳过冒号
    
    // 跳过空格
    while (pos < json.length() && std::isspace(json[pos])) {
        pos++;
    }
    
    if (pos >= json.length()) {
        return "";
    }
    
    // 判断值的类型
    if (json[pos] == '"') {
        // 字符串值
        pos++; // 跳过开始引号
        size_t end_pos = json.find('"', pos);
        if (end_pos == std::string::npos) {
            return "";
        }
        return json.substr(pos, end_pos - pos);
    } else {
        // 数字或布尔值
        size_t end_pos = pos;
        while (end_pos < json.length() && 
               json[end_pos] != ',' && 
               json[end_pos] != '}' && 
               json[end_pos] != ']') {
            end_pos++;
        }
        return trim(json.substr(pos, end_pos - pos));
    }
}

bool JsonHelper::parseString(const std::string &json, const std::string &key, std::string &value) {
    std::string extracted = extractValue(json, key);
    if (extracted.empty()) {
        return false;
    }
    value = extracted;
    return true;
}

bool JsonHelper::parseInt(const std::string &json, const std::string &key, int &value) {
    std::string extracted = extractValue(json, key);
    if (extracted.empty()) {
        return false;
    }
    try {
        value = std::stoi(extracted);
        return true;
    } catch (...) {
        return false;
    }
}

bool JsonHelper::parseFloat(const std::string &json, const std::string &key, float &value) {
    std::string extracted = extractValue(json, key);
    if (extracted.empty()) {
        return false;
    }
    try {
        value = std::stof(extracted);
        return true;
    } catch (...) {
        return false;
    }
}

bool JsonHelper::parseBool(const std::string &json, const std::string &key, bool &value) {
    std::string extracted = extractValue(json, key);
    if (extracted.empty()) {
        return false;
    }
    
    std::string lower = extracted;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "true" || lower == "1") {
        value = true;
        return true;
    } else if (lower == "false" || lower == "0") {
        value = false;
        return true;
    }
    
    return false;
}

} // namespace ai
} // namespace mediakit
