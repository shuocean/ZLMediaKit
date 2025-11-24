/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_ALERT_ENGINE_H
#define ZLMEDIAKIT_ALERT_ENGINE_H

#include "DetectionResult.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

namespace mediakit {
namespace ai {

/**
 * @brief 告警条件类型
 * @note 配置化：支持多种告警条件
 */
enum class AlertConditionType {
    ObjectDetected,     ///< 检测到目标
    ObjectInROI,        ///< 目标进入ROI
    ObjectCount,        ///< 目标数量达到阈值
    ObjectStay,         ///< 目标停留时间达到阈值
    ObjectCrossLine,    ///< 目标越线
    Custom              ///< 自定义条件(Python插件)
};

/**
 * @brief 告警规则配置
 * @note 配置化核心：所有告警规则均可通过配置文件或API设置
 */
struct AlertRule {
    std::string rule_id;               ///< 规则唯一标识
    std::string rule_name;             ///< 规则名称（可读性）
    bool enabled = true;               ///< 是否启用
    
    // 触发条件
    AlertConditionType condition_type; ///< 条件类型
    std::vector<int> target_classes;   ///< 目标类别ID列表（空=所有类别）
    float min_confidence = 0.5f;       ///< 最小置信度
    
    // 数量条件
    int min_count = 1;                 ///< 最小数量
    int max_count = -1;                ///< 最大数量（-1=不限）
    
    // ROI条件
    struct {
        float x, y, w, h;              ///< ROI区域（归一化坐标）
        bool enabled = false;
    } roi;
    
    // 时间条件
    int cooldown_seconds = 60;         ///< 冷却时间（防止重复告警）
    int stay_duration_seconds = 0;     ///< 停留时长（0=立即）
    
    // 限流配置
    int max_alerts_per_minute = 10;    ///< 每分钟最大告警数（防止告警风暴）
    bool aggregate_alerts = true;      ///< 是否聚合相似告警
    
    // 通知配置
    std::vector<std::string> notifier_ids; ///< 通知器ID列表
    std::map<std::string, std::string> metadata; ///< 自定义元数据
    
    /**
     * @brief 从JSON加载规则
     */
    bool fromJson(const std::string &json_str);
    
    /**
     * @brief 转换为JSON
     */
    std::string toJson() const;
};

/**
 * @brief 告警事件
 */
struct AlertEvent {
    std::string alert_id;              ///< 告警ID（UUID）
    std::string rule_id;               ///< 触发的规则ID
    std::string stream_id;             ///< 来源流ID
    uint64_t timestamp;                ///< 告警时间戳（毫秒）
    
    AlertConditionType condition_type; ///< 条件类型
    DetectionResult::Ptr detection;    ///< 检测结果
    std::vector<DetectionBox> trigger_boxes; ///< 触发告警的检测框
    
    std::map<std::string, std::string> metadata; ///< 元数据
    
    /**
     * @brief 序列化为JSON
     */
    std::string toJson() const;
};

/**
 * @brief 告警引擎（单例模式）
 * @note 模块化设计：集中管理所有告警规则和事件
 * @note 性能优化：限流、去重、聚合机制
 * @note 线程安全：所有方法都是线程安全的
 */
class AlertEngine {
public:
    using Ptr = std::shared_ptr<AlertEngine>;
    using OnAlertCallback = std::function<void(const AlertEvent &)>;
    
    /**
     * @brief 获取单例实例
     */
    static AlertEngine &Instance();
    
    // ==================== 规则管理 ====================
    
    /**
     * @brief 注册告警规则
     * @param rule 告警规则
     * @return 注册是否成功
     * @note 配置化：支持运行时动态注册
     */
    bool registerRule(const AlertRule &rule);
    
    /**
     * @brief 注销告警规则
     * @param rule_id 规则ID
     * @return 注销是否成功
     */
    bool unregisterRule(const std::string &rule_id);
    
    /**
     * @brief 获取规则
     * @param rule_id 规则ID
     * @return 规则配置
     */
    AlertRule getRule(const std::string &rule_id) const;
    
    /**
     * @brief 更新规则
     * @param rule_id 规则ID
     * @param rule 新规则配置
     * @return 更新是否成功
     * @note 配置化：支持热更新
     */
    bool updateRule(const std::string &rule_id, const AlertRule &rule);
    
    /**
     * @brief 启用/禁用规则
     * @param rule_id 规则ID
     * @param enable true-启用 false-禁用
     * @return 操作是否成功
     * @note 可视化管理：临时禁用规则
     */
    bool enableRule(const std::string &rule_id, bool enable = true);
    
    /**
     * @brief 获取所有规则ID
     */
    std::vector<std::string> getAllRuleIds() const;
    
    // ==================== 告警处理 ====================
    
    /**
     * @brief 处理检测结果，判断是否触发告警
     * @param detection 检测结果
     * @note 核心功能：根据规则自动匹配并触发告警
     */
    void processDetection(const DetectionResult::Ptr &detection);
    
    /**
     * @brief 设置告警回调
     * @param callback 回调函数
     * @note 模块化：通过回调通知外部系统
     */
    void setAlertCallback(OnAlertCallback callback);
    
    // ==================== 配置持久化 ====================
    
    /**
     * @brief 从配置文件加载规则
     * @param filepath JSON配置文件路径
     * @return 加载成功的规则数
     */
    int loadFromFile(const std::string &filepath);
    
    /**
     * @brief 保存规则到文件
     * @param filepath JSON配置文件路径
     * @return 保存是否成功
     */
    bool saveToFile(const std::string &filepath) const;
    
    // ==================== 统计与监控 ====================
    
    /**
     * @brief 获取规则统计信息
     * @param rule_id 规则ID
     * @return JSON格式的统计信息
     * @note 可视化管理：触发次数、冷却状态等
     */
    std::string getRuleStatistics(const std::string &rule_id) const;
    
    /**
     * @brief 获取全局统计信息
     * @return JSON格式的全局统计
     */
    std::string getGlobalStatistics() const;
    
    /**
     * @brief 重置统计信息
     * @param rule_id 规则ID（空=重置全部）
     */
    void resetStatistics(const std::string &rule_id = "");
    
    /**
     * @brief 清空所有规则
     */
    void clear();
    
private:
    AlertEngine() = default;
    ~AlertEngine() = default;
    AlertEngine(const AlertEngine &) = delete;
    AlertEngine &operator=(const AlertEngine &) = delete;
    
    /**
     * @brief 检查规则是否匹配
     * @param rule 告警规则
     * @param detection 检测结果
     * @return 是否匹配
     */
    bool matchRule(const AlertRule &rule, const DetectionResult::Ptr &detection);
    
    /**
     * @brief 检查冷却时间
     * @param rule_id 规则ID
     * @return 是否在冷却中
     */
    bool isInCooldown(const std::string &rule_id) const;
    
    /**
     * @brief 触发告警
     * @param rule 告警规则
     * @param detection 检测结果
     */
    void triggerAlert(const AlertRule &rule, const DetectionResult::Ptr &detection);
    
    mutable std::recursive_mutex _mutex;              ///< 线程安全锁
    std::map<std::string, AlertRule> _rules;          ///< 规则表
    std::map<std::string, uint64_t> _last_alert_time; ///< 最后告警时间（冷却）
    std::map<std::string, int> _alert_count_per_minute; ///< 每分钟告警计数（限流）
    OnAlertCallback _alert_callback;                  ///< 告警回调
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_ALERT_ENGINE_H
