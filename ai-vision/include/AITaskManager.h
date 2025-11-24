/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_AI_TASK_MANAGER_H
#define ZLMEDIAKIT_AI_TASK_MANAGER_H

#include "InferenceEngine.h"
#include "DetectionResult.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <functional>

namespace mediakit {
namespace ai {

/**
 * @brief AI任务配置
 * @note 配置化核心：所有任务参数均可通过配置文件或API动态设置
 */
struct AITaskConfig {
    std::string task_id;              ///< 任务唯一标识符
    std::string model_id;             ///< 使用的模型ID
    std::string model_type;           ///< 模型类型（yolo/face_det/pose等）

    // 推理参数（可动态修改）
    float conf_threshold = 0.5f;      ///< 置信度阈值
    float nms_threshold = 0.4f;       ///< NMS阈值
    int skip_frames = 0;              ///< 跳帧间隔（0=不跳帧，性能优化）

    // ROI配置（归一化坐标）
    struct ROI {
        float x = 0.0f, y = 0.0f;    ///< 左上角坐标
        float w = 1.0f, h = 1.0f;    ///< 宽度和高度
        bool enabled = false;         ///< 是否启用ROI
    } roi;

    // 运行状态
    bool enabled = true;              ///< 是否启用该任务
    int priority = 0;                 ///< 优先级（0-9，越大越高）

    // 统计信息
    uint64_t process_count = 0;       ///< 处理帧数
    float avg_inference_time_ms = 0;  ///< 平均推理时间

    /**
     * @brief 从JSON加载配置
     */
    bool fromJson(const std::string &json_str);

    /**
     * @brief 转换为JSON
     */
    std::string toJson() const;
};

/**
 * @brief 流-任务绑定关系
 * @note 多对多关系：一个流可绑定多个任务，一个任务可应用于多个流
 */
struct StreamTaskBinding {
    std::string stream_id;            ///< 流ID（格式：vhost/app/stream）
    std::vector<std::string> task_ids;///< 绑定的任务ID列表

    /**
     * @brief 序列化
     */
    std::string toJson() const;

    /**
     * @brief 反序列化
     */
    bool fromJson(const std::string &json_str);
};

/**
 * @brief AI任务管理器（单例模式）
 * @note 核心组件：管理流与AI任务的多对多关系
 * @note 配置化：所有操作支持动态配置，无需重启服务
 * @note 线程安全：所有public方法都是线程安全的
 * @note 性能优化：任务调度、批处理、跳帧等优化策略
 */
class AITaskManager {
public:
    using Ptr = std::shared_ptr<AITaskManager>;
    using OnDetectionCallback = std::function<void(const DetectionResult::Ptr &)>;

    /**
     * @brief 获取单例实例
     */
    static AITaskManager &Instance();

    // ==================== 任务管理 ====================

    /**
     * @brief 注册AI任务
     * @param task_id 任务ID
     * @param config 任务配置
     * @return 注册是否成功
     * @note 配置化：支持运行时动态注册
     */
    bool registerTask(const std::string &task_id, const AITaskConfig &config);

    /**
     * @brief 注销AI任务
     * @param task_id 任务ID
     * @return 注销是否成功
     * @note 最小化影响：注销时检查绑定关系，防止影响运行中的流
     */
    bool unregisterTask(const std::string &task_id);

    /**
     * @brief 获取任务配置
     * @param task_id 任务ID
     * @return 任务配置
     */
    AITaskConfig getTaskConfig(const std::string &task_id) const;

    /**
     * @brief 更新任务配置（动态修改）
     * @param task_id 任务ID
     * @param config 新配置
     * @return 更新是否成功
     * @note 配置化：支持热更新，立即生效
     */
    bool updateTaskConfig(const std::string &task_id, const AITaskConfig &config);

    /**
     * @brief 动态修改任务阈值（最常用的配置）
     * @param task_id 任务ID
     * @param conf_threshold 置信度阈值
     * @param nms_threshold NMS阈值
     * @return 更新是否成功
     * @note API接口：供RESTful API调用
     */
    bool updateTaskThreshold(const std::string &task_id,
                            float conf_threshold,
                            float nms_threshold);

    /**
     * @brief 启用/禁用任务
     * @param task_id 任务ID
     * @param enable true-启用 false-禁用
     * @return 操作是否成功
     * @note 可视化管理：支持临时禁用任务而不删除配置
     */
    bool enableTask(const std::string &task_id, bool enable = true);

    /**
     * @brief 检查任务是否启用
     */
    bool isTaskEnabled(const std::string &task_id) const;

    /**
     * @brief 获取所有任务ID列表
     * @return 任务ID数组
     */
    std::vector<std::string> getAllTaskIds() const;

    // ==================== 流-任务绑定管理 ====================

    /**
     * @brief 绑定任务到流
     * @param stream_id 流ID
     * @param task_id 任务ID
     * @return 绑定是否成功
     * @note 多对多关系：一个流可绑定多个任务
     */
    bool bindTaskToStream(const std::string &stream_id, const std::string &task_id);

    /**
     * @brief 从流解绑任务
     * @param stream_id 流ID
     * @param task_id 任务ID
     * @return 解绑是否成功
     */
    bool unbindTaskFromStream(const std::string &stream_id, const std::string &task_id);

    /**
     * @brief 获取流绑定的所有任务
     * @param stream_id 流ID
     * @return 任务ID列表
     */
    std::vector<std::string> getStreamTasks(const std::string &stream_id) const;

    /**
     * @brief 获取任务应用的所有流
     * @param task_id 任务ID
     * @return 流ID列表
     */
    std::vector<std::string> getTaskStreams(const std::string &task_id) const;

    /**
     * @brief 清空流的所有绑定
     * @param stream_id 流ID
     * @note 清理：流关闭时调用
     */
    void clearStreamBindings(const std::string &stream_id);

    // ==================== 配置持久化 ====================

    /**
     * @brief 从配置文件加载任务和绑定关系
     * @param filepath 配置文件路径（JSON格式）
     * @return 加载成功的任务数
     * @note 配置化：启动时自动加载
     */
    int loadFromFile(const std::string &filepath);

    /**
     * @brief 保存当前配置到文件
     * @param filepath 配置文件路径
     * @return 保存是否成功
     * @note 配置持久化：运行时修改可保存
     */
    bool saveToFile(const std::string &filepath) const;

    // ==================== 检测结果回调 ====================

    /**
     * @brief 设置检测结果回调
     * @param callback 回调函数
     * @note 模块化：通过回调与告警、插件等模块解耦
     */
    void setDetectionCallback(OnDetectionCallback callback);

    // ==================== 统计与监控 ====================

    /**
     * @brief 获取任务统计信息
     * @param task_id 任务ID
     * @return JSON格式的统计信息
     * @note 可视化管理：供Web界面展示
     */
    std::string getTaskStatistics(const std::string &task_id) const;

    /**
     * @brief 获取全局统计信息
     * @return JSON格式的全局统计
     */
    std::string getGlobalStatistics() const;

    /**
     * @brief 重置统计信息
     * @param task_id 任务ID（空则重置全部）
     */
    void resetStatistics(const std::string &task_id = "");

    /**
     * @brief 清空所有任务和绑定
     * @note 用于关闭或重启
     */
    void clear();

private:
    AITaskManager() = default;
    ~AITaskManager() = default;
    AITaskManager(const AITaskManager &) = delete;
    AITaskManager &operator=(const AITaskManager &) = delete;

    mutable std::recursive_mutex _mutex;                         ///< 线程安全锁
    std::map<std::string, AITaskConfig> _tasks;                  ///< 任务配置表
    std::map<std::string, std::set<std::string>> _stream_bindings; ///< 流→任务绑定
    std::map<std::string, std::set<std::string>> _task_bindings;   ///< 任务→流反向索引
    OnDetectionCallback _detection_callback;                     ///< 检测结果回调
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_AI_TASK_MANAGER_H
