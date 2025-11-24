/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_MODEL_REGISTRY_H
#define ZLMEDIAKIT_MODEL_REGISTRY_H

#include "InferenceEngine.h"
#include <string>
#include <map>
#include <mutex>

namespace mediakit {
namespace ai {

/**
 * @brief 模型信息
 * @note 配置化：模型的所有元数据可通过配置文件管理
 */
struct ModelInfo {
    std::string model_id;           ///< 模型唯一标识符
    std::string model_path;         ///< 模型文件路径
    std::string model_type;         ///< 模型类型（yolo/face/pose等）
    std::string version;            ///< 模型版本号
    InferenceConfig config;         ///< 推理配置
    bool enabled = true;            ///< 是否启用
    int64_t load_time = 0;          ///< 加载时间戳（用于LRU淘汰）
    int64_t last_use_time = 0;      ///< 最后使用时间（用于LRU淘汰）
    size_t use_count = 0;           ///< 使用次数统计

    /**
     * @brief 从JSON加载模型信息
     */
    bool fromJson(const std::string &json_str);

    /**
     * @brief 转换为JSON
     */
    std::string toJson() const;
};

/**
 * @brief 模型注册表（单例模式）
 * @note 模块化设计：集中管理所有AI模型，支持动态加载/卸载
 * @note 性能优化：LRU缓存策略，自动淘汰长期未用的模型
 * @note 线程安全：所有操作都是线程安全的
 */
class ModelRegistry {
public:
    using Ptr = std::shared_ptr<ModelRegistry>;

    /**
     * @brief 获取单例实例
     * @return 模型注册表实例
     */
    static ModelRegistry &Instance();

    /**
     * @brief 注册模型
     * @param model_id 模型ID
     * @param info 模型信息
     * @return 注册是否成功
     * @note 配置化：支持动态注册，无需重启服务
     */
    bool registerModel(const std::string &model_id, const ModelInfo &info);

    /**
     * @brief 注销模型
     * @param model_id 模型ID
     * @return 注销是否成功
     * @note 最小化影响：注销时检查是否有使用中的引用
     */
    bool unregisterModel(const std::string &model_id);

    /**
     * @brief 获取模型信息
     * @param model_id 模型ID
     * @return 模型信息，不存在则返回空结构
     */
    ModelInfo getModelInfo(const std::string &model_id) const;

    /**
     * @brief 获取推理引擎实例
     * @param model_id 模型ID
     * @return 推理引擎实例，失败返回nullptr
     * @note 性能优化：使用缓存，避免重复加载
     */
    InferenceEngine::Ptr getEngine(const std::string &model_id);

    /**
     * @brief 预热模型（提前加载到显存）
     * @param model_id 模型ID
     * @return 预热是否成功
     * @note 性能优化：避免首次推理时的加载延迟
     */
    bool warmupModel(const std::string &model_id);

    /**
     * @brief 卸载模型（释放显存）
     * @param model_id 模型ID
     * @return 卸载是否成功
     * @note 显存管理：显存不足时可主动卸载
     */
    bool unloadModel(const std::string &model_id);

    /**
     * @brief 获取所有已注册的模型ID列表
     * @return 模型ID数组
     * @note 可视化管理：供管理界面使用
     */
    std::vector<std::string> getAllModelIds() const;

    /**
     * @brief 获取已加载的模型数量
     * @return 已加载模型数
     */
    size_t getLoadedModelCount() const;

    /**
     * @brief 获取注册表统计信息
     * @return JSON格式的统计信息
     * @note 可视化管理：显存占用、加载耗时等
     */
    std::string getStatistics() const;

    /**
     * @brief 从JSON配置文件加载所有模型
     * @param json_file 配置文件路径
     * @return 加载成功的模型数量
     * @note 配置化：启动时自动加载配置
     */
    int loadFromFile(const std::string &json_file);

    /**
     * @brief 保存当前配置到文件
     * @param json_file 配置文件路径
     * @return 保存是否成功
     * @note 配置持久化：运行时修改可保存
     */
    bool saveToFile(const std::string &json_file) const;

    /**
     * @brief LRU淘汰策略：卸载最久未使用的模型
     * @param target_free_memory 目标释放的显存大小（字节）
     * @return 实际释放的显存大小
     * @note 显存管理：自动淘汰机制
     */
    size_t evictLRU(size_t target_free_memory);

    /**
     * @brief 清空所有模型
     * @note 用于关闭或重启
     */
    void clear();

private:
    ModelRegistry() = default;
    ~ModelRegistry() = default;
    ModelRegistry(const ModelRegistry &) = delete;
    ModelRegistry &operator=(const ModelRegistry &) = delete;

    mutable std::recursive_mutex _mutex;                          ///< 线程安全锁
    std::map<std::string, ModelInfo> _models;                     ///< 模型信息表
    std::map<std::string, InferenceEngine::Ptr> _engines;         ///< 引擎缓存
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_MODEL_REGISTRY_H
