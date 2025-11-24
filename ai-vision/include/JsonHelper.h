/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_JSON_HELPER_H
#define ZLMEDIAKIT_JSON_HELPER_H

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace mediakit {
namespace ai {

/**
 * @brief 简化的JSON辅助类
 * @note 性能优化：避免引入大型JSON库，使用轻量级实现
 * @note 向后兼容：可替换为nlohmann/json等库
 */
class JsonHelper {
public:
    /**
     * @brief 构建JSON对象起始
     */
    static std::string objectStart() { return "{"; }
    
    /**
     * @brief 构建JSON对象结束
     */
    static std::string objectEnd() { return "}"; }
    
    /**
     * @brief 构建JSON数组起始
     */
    static std::string arrayStart() { return "["; }
    
    /**
     * @brief 构建JSON数组结束
     */
    static std::string arrayEnd() { return "]"; }
    
    /**
     * @brief 添加字符串字段
     */
    static std::string field(const std::string &key, const std::string &value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":\"" << escape(value) << "\"";
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 添加整数字段
     */
    static std::string field(const std::string &key, int value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":" << value;
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 添加长整数字段
     */
    static std::string field(const std::string &key, uint64_t value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":" << value;
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 添加浮点数字段
     */
    static std::string field(const std::string &key, float value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":" << value;
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 添加布尔字段
     */
    static std::string field(const std::string &key, bool value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":" << (value ? "true" : "false");
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 添加对象字段
     */
    static std::string fieldObject(const std::string &key, const std::string &value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":" << value;
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 添加数组字段
     */
    static std::string fieldArray(const std::string &key, const std::string &value, bool last = false) {
        std::stringstream ss;
        ss << "\"" << key << "\":" << value;
        if (!last) ss << ",";
        return ss.str();
    }
    
    /**
     * @brief 整数数组转JSON
     */
    static std::string arrayInt(const std::vector<int> &values) {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < values.size(); ++i) {
            ss << values[i];
            if (i < values.size() - 1) ss << ",";
        }
        ss << "]";
        return ss.str();
    }
    
    /**
     * @brief 字符串数组转JSON
     */
    static std::string arrayString(const std::vector<std::string> &values) {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < values.size(); ++i) {
            ss << "\"" << escape(values[i]) << "\"";
            if (i < values.size() - 1) ss << ",";
        }
        ss << "]";
        return ss.str();
    }
    
    /**
     * @brief 转义特殊字符
     */
    static std::string escape(const std::string &str) {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '"':  ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b"; break;
                case '\f': ss << "\\f"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default:
                    if (c < 0x20) {
                        ss << "\\u" << std::hex << (int)c;
                    } else {
                        ss << c;
                    }
            }
        }
        return ss.str();
    }
    
    /**
     * @brief 简单的JSON解析（仅支持基本类型）
     * @note 完整解析建议使用nlohmann/json
     */
    static bool parseString(const std::string &json, const std::string &key, std::string &value);
    static bool parseInt(const std::string &json, const std::string &key, int &value);
    static bool parseFloat(const std::string &json, const std::string &key, float &value);
    static bool parseBool(const std::string &json, const std::string &key, bool &value);
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_JSON_HELPER_H
