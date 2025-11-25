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

string JsonHelper::extractObject(const string &json_str, const string &key) {
    string search_key = "\"" + key + "\"";
    size_t key_pos = json_str.find(search_key);
    if (key_pos == string::npos) {
        return "{}";
    }
    
    size_t start = json_str.find('{', key_pos);
    if (start == string::npos) {
        return "{}";
    }
    
    // 找到匹配的右花括号
    int depth = 1;
    size_t end = start + 1;
    while (end < json_str.size() && depth > 0) {
        if (json_str[end] == '{') depth++;
        else if (json_str[end] == '}') depth--;
        end++;
    }
    
    if (depth == 0) {
        return json_str.substr(start, end - start);
    }
    
    return "{}";
}

string JsonHelper::extractArray(const string &json_str, const string &key) {
    string search_key = "\"" + key + "\"";
    size_t key_pos = json_str.find(search_key);
    if (key_pos == string::npos) {
        return "[]";
    }
    
    size_t start = json_str.find('[', key_pos);
    if (start == string::npos) {
        return "[]";
    }
    
    // 找到匹配的右方括号
    int depth = 1;
    size_t end = start + 1;
    while (end < json_str.size() && depth > 0) {
        if (json_str[end] == '[') depth++;
        else if (json_str[end] == ']') depth--;
        end++;
    }
    
    if (depth == 0) {
        return json_str.substr(start, end - start);
    }
    
    return "[]";
}

vector<string> JsonHelper::parseStringArray(const string &array_str) {
    vector<string> result;
    
    if (array_str.empty() || array_str == "[]") {
        return result;
    }
    
    // 简化版：假设数组格式为 ["str1","str2","str3"]
    size_t start = array_str.find('[');
    size_t end = array_str.rfind(']');
    
    if (start == string::npos || end == string::npos || start >= end) {
        return result;
    }
    
    string content = array_str.substr(start + 1, end - start - 1);
    
    // 分割字符串
    size_t pos = 0;
    while (pos < content.size()) {
        // 跳过空白和逗号
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == ',')) {
            pos++;
        }
        
        if (pos >= content.size()) break;
        
        // 找到引号
        if (content[pos] == '"') {
            pos++;
            size_t start_quote = pos;
            while (pos < content.size() && content[pos] != '"') {
                if (content[pos] == '\\') pos++; // 跳过转义字符
                pos++;
            }
            
            if (pos < content.size()) {
                result.push_back(content.substr(start_quote, pos - start_quote));
                pos++;
            }
        }
    }
    
    return result;
}

vector<int> JsonHelper::parseIntArray(const string &array_str) {
    vector<int> result;
    
    if (array_str.empty() || array_str == "[]") {
        return result;
    }
    
    // 简化版：假设数组格式为 [1,2,3]
    size_t start = array_str.find('[');
    size_t end = array_str.rfind(']');
    
    if (start == string::npos || end == string::npos || start >= end) {
        return result;
    }
    
    string content = array_str.substr(start + 1, end - start - 1);
    
    // 分割并解析整数
    stringstream ss(content);
    string token;
    while (getline(ss, token, ',')) {
        token = trim(token);
        if (!token.empty()) {
            try {
                result.push_back(stoi(token));
            } catch (...) {
                // 忽略无效整数
            }
        }
    }
    
    return result;
}

vector<string> JsonHelper::splitObjectArray(const string &array_str) {
    vector<string> result;
    
    if (array_str.empty() || array_str == "[]") {
        return result;
    }
    
    // 找到数组边界
    size_t start = array_str.find('[');
    size_t end = array_str.rfind(']');
    
    if (start == string::npos || end == string::npos || start >= end) {
        return result;
    }
    
    // 遍历数组内容，按花括号配对分割对象
    size_t pos = start + 1;
    while (pos < end) {
        // 跳过空白和逗号
        while (pos < end && (array_str[pos] == ' ' || array_str[pos] == ',' || 
                              array_str[pos] == '\n' || array_str[pos] == '\t')) {
            pos++;
        }
        
        if (pos >= end) break;
        
        // 找到对象开始
        if (array_str[pos] == '{') {
            size_t obj_start = pos;
            int depth = 1;
            pos++;
            
            // 找到匹配的右花括号
            while (pos < end && depth > 0) {
                if (array_str[pos] == '{') depth++;
                else if (array_str[pos] == '}') depth--;
                pos++;
            }
            
            if (depth == 0) {
                // 提取完整对象
                result.push_back(array_str.substr(obj_start, pos - obj_start));
            }
        } else {
            // 不是对象，跳过
            pos++;
        }
    }
    
    return result;
}

} // namespace ai
} // namespace mediakit
