/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_AI_PIPELINE_H
#define ZLMEDIAKIT_AI_PIPELINE_H

#include "FrameConverter.h"
#include "GpuUploader.h"
#include "DetectionResult.h"
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <mutex>

namespace mediakit {
namespace ai {

/**
 * @brief AI处理Pipeline配置
 * @note 配置化：所有Pipeline参数可配置
 */
struct PipelineConfig {
    std::string stream_id;          ///< 流ID
    
    // 转换器配置
    bool enable_converter = true;   ///< 是否启用格式转换
    ConverterConfig converter;      ///< 转换器配置
    
    // 上传器配置
    bool enable_gpu_upload = true;  ///< 是否启用GPU上传
    UploaderConfig uploader;        ///< 上传器配置
    
    // AI任务配置
    std::vector<std::string> task_ids;  ///< 绑定的AI任务ID列表
    
    // 性能配置
    int skip_frames = 0;            ///< 跳帧间隔（0=不跳帧）
    int queue_size = 5;             ///< 处理队列大小
    bool async_processing = true;   ///< 异步处理
    
    /**
     * @brief 验证配置
     */
    bool isValid() const;
    
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
 * @brief AI处理Pipeline
 * @note 核心组件：连接媒体流和AI处理的桥梁
 * @note 数据流：Frame → Convert → Upload → AI Inference → Results
 * @note 性能优化：
 *   1. 异步处理，不阻塞媒体流
 *   2. 跳帧策略，降低CPU/GPU负载
 *   3. 队列缓冲，平滑负载波动
 * @note 最小化影响：与现有MediaSource解耦，通过Hook接入
 */
class AIPipeline {
public:
    using Ptr = std::shared_ptr<AIPipeline>;
    using OnResultCallback = std::function<void(const DetectionResult::Ptr &)>;
    
    /**
     * @brief 创建Pipeline实例
     * @param config Pipeline配置
     * @return Pipeline智能指针
     */
    static Ptr create(const PipelineConfig &config);
    
    virtual ~AIPipeline() = default;
    
    /**
     * @brief 初始化Pipeline
     * @return 初始化是否成功
     * @note 最小化影响：失败时禁用AI功能，不影响媒体流
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 处理视频帧
     * @param frame 视频帧数据（YUV420P格式）
     * @return 处理是否成功
     * @note 向后兼容：接受ZLMediaKit的Frame数据
     * @note 性能：<5ms处理开销（转换+上传）
     */
    virtual bool processFrame(const FrameData &frame) = 0;
    
    /**
     * @brief 设置结果回调
     * @param callback 回调函数
     * @note 模块化：通过回调与告警/插件模块解耦
     */
    virtual void setResultCallback(OnResultCallback callback) = 0;
    
    /**
     * @brief 启动Pipeline
     * @return 启动是否成功
     */
    virtual bool start() = 0;
    
    /**
     * @brief 停止Pipeline
     */
    virtual void stop() = 0;
    
    /**
     * @brief 检查Pipeline是否运行中
     */
    virtual bool isRunning() const = 0;
    
    /**
     * @brief 获取Pipeline配置
     */
    virtual const PipelineConfig &getConfig() const = 0;
    
    /**
     * @brief 更新Pipeline配置（热更新）
     * @param config 新配置
     * @return 更新是否成功
     * @note 配置化：支持运行时修改配置
     */
    virtual bool updateConfig(const PipelineConfig &config) = 0;
    
    /**
     * @brief 获取统计信息
     * @return JSON格式的统计信息
     * @note 可视化管理：吞吐量、延迟、跳帧率等
     */
    virtual std::string getStatistics() const = 0;
    
    /**
     * @brief 重置统计信息
     */
    virtual void resetStatistics() = 0;
};

/**
 * @brief 标准AI Pipeline实现
 * @note 实现完整的 Frame → Convert → Upload → AI → Result 流程
 */
class StandardAIPipeline : public AIPipeline {
public:
    StandardAIPipeline(const PipelineConfig &config);
    ~StandardAIPipeline() override;
    
    bool initialize() override;
    bool processFrame(const FrameData &frame) override;
    void setResultCallback(OnResultCallback callback) override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    const PipelineConfig &getConfig() const override;
    bool updateConfig(const PipelineConfig &config) override;
    std::string getStatistics() const override;
    void resetStatistics() override;
    
private:
    struct Impl;  ///< PIMPL模式：隐藏实现细节
    std::unique_ptr<Impl> _impl;
    
    PipelineConfig _config;  ///< Pipeline配置
    
    // 统计信息
    struct Statistics {
        uint64_t total_frames = 0;       ///< 总帧数
        uint64_t processed_frames = 0;   ///< 处理的帧数
        uint64_t skipped_frames = 0;     ///< 跳过的帧数
        uint64_t failed_frames = 0;      ///< 失败的帧数
        uint64_t total_time_us = 0;      ///< 总耗时(微秒)
        float avg_fps = 0.0f;            ///< 平均帧率
        float avg_latency_ms = 0.0f;     ///< 平均延迟(毫秒)
        float skip_ratio = 0.0f;         ///< 跳帧率
    } _stats;
};

/**
 * @brief Pipeline管理器（单例模式）
 * @note 模块化设计：集中管理所有流的Pipeline
 * @note 性能优化：Pipeline复用，减少初始化开销
 */
class PipelineManager {
public:
    using Ptr = std::shared_ptr<PipelineManager>;
    
    /**
     * @brief 获取单例实例
     */
    static PipelineManager &Instance();
    
    /**
     * @brief 为流创建Pipeline
     * @param stream_id 流ID
     * @param config Pipeline配置
     * @return Pipeline智能指针
     */
    AIPipeline::Ptr createPipeline(const std::string &stream_id, 
                                    const PipelineConfig &config);
    
    /**
     * @brief 获取流的Pipeline
     * @param stream_id 流ID
     * @return Pipeline智能指针，不存在返回nullptr
     */
    AIPipeline::Ptr getPipeline(const std::string &stream_id) const;
    
    /**
     * @brief 删除流的Pipeline
     * @param stream_id 流ID
     * @return 删除是否成功
     * @note 最小化影响：流关闭时自动清理
     */
    bool removePipeline(const std::string &stream_id);
    
    /**
     * @brief 获取所有Pipeline的流ID列表
     */
    std::vector<std::string> getAllStreamIds() const;
    
    /**
     * @brief 获取Pipeline数量
     */
    size_t getPipelineCount() const;
    
    /**
     * @brief 获取全局统计信息
     * @return JSON格式的全局统计
     */
    std::string getGlobalStatistics() const;
    
    /**
     * @brief 清空所有Pipeline
     */
    void clear();
    
private:
    PipelineManager() = default;
    ~PipelineManager() = default;
    PipelineManager(const PipelineManager &) = delete;
    PipelineManager &operator=(const PipelineManager &) = delete;
    
    mutable std::recursive_mutex _mutex;  ///< 线程安全锁
    std::map<std::string, AIPipeline::Ptr> _pipelines;  ///< Pipeline表
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_AI_PIPELINE_H
