/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_GPU_UPLOADER_H
#define ZLMEDIAKIT_GPU_UPLOADER_H

#include "FrameConverter.h"
#include <memory>
#include <functional>

namespace mediakit {
namespace ai {

/**
 * @brief GPU帧数据
 * @note 性能优化：封装GPU显存，支持引用计数自动释放
 */
class GpuFrame {
public:
    using Ptr = std::shared_ptr<GpuFrame>;
    
    /**
     * @brief 创建GPU帧
     * @param width 宽度
     * @param height 高度
     * @param format 像素格式
     * @param device_id GPU设备ID
     * @return GPU帧智能指针
     */
    static Ptr create(int width, int height, PixelFormat format, int device_id = 0);
    
    virtual ~GpuFrame() = default;
    
    /**
     * @brief 获取GPU设备指针
     * @return GPU内存地址
     */
    virtual void *getDevicePtr() const = 0;
    
    /**
     * @brief 获取宽度
     */
    virtual int getWidth() const = 0;
    
    /**
     * @brief 获取高度
     */
    virtual int getHeight() const = 0;
    
    /**
     * @brief 获取格式
     */
    virtual PixelFormat getFormat() const = 0;
    
    /**
     * @brief 获取GPU设备ID
     */
    virtual int getDeviceId() const = 0;
    
    /**
     * @brief 获取时间戳
     */
    virtual uint64_t getPts() const = 0;
    
    /**
     * @brief 设置时间戳
     */
    virtual void setPts(uint64_t pts) = 0;
    
    /**
     * @brief 获取显存大小
     * @return 字节数
     */
    virtual size_t getSize() const = 0;
};

/**
 * @brief GPU上传器配置
 * @note 配置化：所有上传参数可配置
 */
struct UploaderConfig {
    int device_id = 0;              ///< GPU设备ID
    bool use_pinned_memory = true;  ///< 使用页锁定内存（性能优化）
    bool async_upload = true;       ///< 异步上传（性能优化）
    size_t pool_size = 5;           ///< 内存池大小
    
    /**
     * @brief 验证配置
     */
    bool isValid() const;
    
    /**
     * @brief 从JSON加载配置
     */
    bool fromJson(const std::string &json_str);
    
    /**
     * @brief 导出JSON配置
     */
    std::string toJson() const;
};

/**
 * @brief GPU数据上传器
 * @note 性能优化：
 *   1. 使用Pinned Memory加速CPU-GPU传输
 *   2. 异步上传，不阻塞主线程
 *   3. 内存池管理，减少分配开销
 * @note 最小化影响：仅在ENABLE_CUDA时编译
 */
class GpuUploader {
public:
    using Ptr = std::shared_ptr<GpuUploader>;
    using OnUploadCallback = std::function<void(const GpuFrame::Ptr &)>;
    
    /**
     * @brief 创建GPU上传器
     * @param config 配置
     * @return 上传器智能指针
     */
    static Ptr create(const UploaderConfig &config);
    
    virtual ~GpuUploader() = default;
    
    /**
     * @brief 初始化上传器
     * @return 初始化是否成功
     * @note 最小化影响：失败时降级到CPU模式
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 同步上传单帧
     * @param cpu_frame CPU帧数据
     * @return GPU帧智能指针，失败返回nullptr
     * @note 性能：约3-5ms@1080p
     */
    virtual GpuFrame::Ptr upload(const FrameData &cpu_frame) = 0;
    
    /**
     * @brief 异步上传单帧
     * @param cpu_frame CPU帧数据
     * @param callback 上传完成回调
     * @return 提交是否成功
     * @note 性能优化：不阻塞调用线程
     */
    virtual bool uploadAsync(const FrameData &cpu_frame, OnUploadCallback callback) = 0;
    
    /**
     * @brief 批量同步上传
     * @param cpu_frames CPU帧数组
     * @param gpu_frames GPU帧数组（输出）
     * @param count 帧数量
     * @return 成功上传的帧数
     * @note 性能优化：批量传输，减少开销
     */
    virtual int uploadBatch(const FrameData *cpu_frames,
                           GpuFrame::Ptr *gpu_frames,
                           int count) = 0;
    
    /**
     * @brief 检查GPU是否可用
     * @return true-可用 false-不可用
     */
    virtual bool isGpuAvailable() const = 0;
    
    /**
     * @brief 获取显存使用情况
     * @param free_memory 空闲显存（字节）
     * @param total_memory 总显存（字节）
     * @return 获取是否成功
     * @note 显存管理：用于降级策略
     */
    virtual bool getMemoryInfo(size_t &free_memory, size_t &total_memory) const = 0;
    
    /**
     * @brief 获取上传统计信息
     * @return JSON格式的统计信息
     * @note 可视化管理：上传耗时、带宽等
     */
    virtual std::string getStatistics() const;
    
    /**
     * @brief 重置统计信息
     */
    virtual void resetStatistics();
    
    /**
     * @brief 等待所有异步上传完成
     */
    virtual void waitAll() = 0;
    
protected:
    UploaderConfig _config;  ///< 上传配置
};

#ifdef ENABLE_CUDA
/**
 * @brief 基于CUDA的GPU上传器实现
 * @note 性能优化：充分利用CUDA异步传输和Stream
 */
class CudaUploader : public GpuUploader {
public:
    CudaUploader(const UploaderConfig &config);
    ~CudaUploader() override;
    
    bool initialize() override;
    GpuFrame::Ptr upload(const FrameData &cpu_frame) override;
    bool uploadAsync(const FrameData &cpu_frame, OnUploadCallback callback) override;
    int uploadBatch(const FrameData *cpu_frames,
                   GpuFrame::Ptr *gpu_frames,
                   int count) override;
    bool isGpuAvailable() const override;
    bool getMemoryInfo(size_t &free_memory, size_t &total_memory) const override;
    std::string getStatistics() const override;
    void resetStatistics() override;
    void waitAll() override;
    
private:
    struct Impl;  ///< PIMPL模式：隐藏CUDA实现细节
    std::unique_ptr<Impl> _impl;
    
    // 统计信息
    struct Statistics {
        uint64_t upload_count = 0;       ///< 上传次数
        uint64_t total_bytes = 0;        ///< 总字节数
        uint64_t total_time_us = 0;      ///< 总耗时(微秒)
        float avg_time_ms = 0.0f;        ///< 平均耗时(毫秒)
        float bandwidth_gbps = 0.0f;     ///< 带宽(GB/s)
        size_t peak_memory_mb = 0;       ///< 峰值显存占用(MB)
    } _stats;
};
#endif // ENABLE_CUDA

/**
 * @brief CPU回退实现（无GPU时使用）
 * @note 向后兼容：没有GPU时不报错，返回CPU内存
 */
class CpuFallbackUploader : public GpuUploader {
public:
    CpuFallbackUploader(const UploaderConfig &config);
    ~CpuFallbackUploader() override;
    
    bool initialize() override;
    GpuFrame::Ptr upload(const FrameData &cpu_frame) override;
    bool uploadAsync(const FrameData &cpu_frame, OnUploadCallback callback) override;
    int uploadBatch(const FrameData *cpu_frames,
                   GpuFrame::Ptr *gpu_frames,
                   int count) override;
    bool isGpuAvailable() const override { return false; }
    bool getMemoryInfo(size_t &free_memory, size_t &total_memory) const override;
    void waitAll() override {}
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_GPU_UPLOADER_H
