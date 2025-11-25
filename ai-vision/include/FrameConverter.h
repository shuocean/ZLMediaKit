/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_FRAME_CONVERTER_H
#define ZLMEDIAKIT_FRAME_CONVERTER_H

#include <memory>
#include <cstdint>
#include <string>

namespace mediakit {
namespace ai {

/**
 * @brief 像素格式枚举
 * @note 配置化：支持常见的AI输入格式
 */
enum class PixelFormat {
    YUV420P,    ///< YUV 4:2:0 Planar (FFmpeg默认解码格式)
    NV12,       ///< YUV 4:2:0 Semi-Planar (GPU友好)
    RGB24,      ///< RGB 24-bit (AI常用格式)
    BGR24,      ///< BGR 24-bit (OpenCV格式)
    RGBA,       ///< RGBA 32-bit
    BGRA,       ///< BGRA 32-bit
    GRAY        ///< 灰度图
};

/**
 * @brief 帧数据结构
 * @note 模块化设计：统一的帧数据抽象
 */
struct FrameData {
    uint8_t *data = nullptr;        ///< 数据指针
    int width = 0;                  ///< 宽度
    int height = 0;                 ///< 高度
    int linesize = 0;               ///< 行字节数（stride）
    PixelFormat format = PixelFormat::YUV420P;  ///< 像素格式
    uint64_t pts = 0;               ///< 时间戳（毫秒）
    bool owns_data = false;         ///< 是否拥有数据所有权
    
    /**
     * @brief 获取数据大小
     * @return 数据字节数
     */
    size_t getSize() const;
    
    /**
     * @brief 分配内存
     * @param w 宽度
     * @param h 高度  
     * @param fmt 格式
     * @return 分配是否成功
     * @note 性能优化：256字节对齐，GPU友好
     */
    bool allocate(int w, int h, PixelFormat fmt);
    
    /**
     * @brief 释放内存
     */
    void release();
    
    /**
     * @brief 拷贝构造（深拷贝）
     */
    FrameData(const FrameData &other);
    FrameData &operator=(const FrameData &other);
    
    /**
     * @brief 移动构造（零拷贝）
     * @note 性能优化：避免不必要的内存拷贝
     */
    FrameData(FrameData &&other) noexcept;
    FrameData &operator=(FrameData &&other) noexcept;
    
    FrameData() = default;
    ~FrameData();
};

/**
 * @brief 帧格式转换器配置
 * @note 配置化：所有转换参数可配置
 */
struct ConverterConfig {
    PixelFormat src_format = PixelFormat::YUV420P;   ///< 源格式
    PixelFormat dst_format = PixelFormat::RGB24;     ///< 目标格式
    int src_width = 0;                               ///< 源宽度
    int src_height = 0;                              ///< 源高度
    int dst_width = 0;                               ///< 目标宽度（缩放）
    int dst_height = 0;                              ///< 目标高度（缩放）
    bool use_simd = true;                            ///< 是否使用SIMD优化
    int quality = 1;                                 ///< 缩放质量 (0-快速 1-双线性 2-双三次)
    
    /**
     * @brief 验证配置
     * @return 配置是否有效
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
 * @brief 帧格式转换器
 * @note 性能优化：使用libswscale硬件加速，支持SIMD
 * @note 向后兼容：不依赖ZLMediaKit现有代码，独立工作
 */
class FrameConverter {
public:
    using Ptr = std::shared_ptr<FrameConverter>;
    
    /**
     * @brief 创建转换器实例
     * @param config 转换配置
     * @return 转换器智能指针
     * @note 工厂模式：根据配置创建优化的转换器
     */
    static Ptr create(const ConverterConfig &config);
    
    virtual ~FrameConverter() = default;
    
    /**
     * @brief 初始化转换器
     * @return 初始化是否成功
     * @note 最小化影响：失败时不影响主流程
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 转换单帧
     * @param src 源帧数据
     * @param dst 目标帧数据（需预分配）
     * @return 转换是否成功
     * @note 性能优化：支持原地转换和批量处理
     */
    virtual bool convert(const FrameData &src, FrameData &dst) = 0;
    
    /**
     * @brief 批量转换（性能优化）
     * @param src_frames 源帧数组
     * @param dst_frames 目标帧数组
     * @param count 帧数量
     * @return 成功转换的帧数
     * @note 性能优化：减少上下文切换，提高缓存命中率
     */
    virtual int convertBatch(const FrameData *src_frames, 
                            FrameData *dst_frames, 
                            int count);
    
    /**
     * @brief 获取转换统计信息
     * @return JSON格式的统计信息
     * @note 可视化管理：转换耗时、吞吐量等
     */
    virtual std::string getStatistics() const;
    
    /**
     * @brief 重置统计信息
     */
    virtual void resetStatistics();
    
protected:
    ConverterConfig _config;  ///< 转换配置
};

/**
 * @brief 基于libswscale的转换器实现
 * @note 性能优化：利用FFmpeg的SIMD优化和硬件加速
 */
class SwscaleConverter : public FrameConverter {
public:
    SwscaleConverter(const ConverterConfig &config);
    ~SwscaleConverter() override;
    
    bool initialize() override;
    bool convert(const FrameData &src, FrameData &dst) override;
    int convertBatch(const FrameData *src_frames, 
                    FrameData *dst_frames, 
                    int count) override;
    std::string getStatistics() const override;
    void resetStatistics() override;
    
private:
    struct Impl;  ///< PIMPL模式：隐藏FFmpeg实现细节
    std::unique_ptr<Impl> _impl;
    
    // 统计信息（性能监控）
    struct Statistics {
        uint64_t convert_count = 0;      ///< 转换次数
        uint64_t total_time_us = 0;      ///< 总耗时(微秒)
        uint64_t total_pixels = 0;       ///< 总像素数
        float avg_time_ms = 0.0f;        ///< 平均耗时(毫秒)
        float throughput_mpps = 0.0f;    ///< 吞吐量(百万像素/秒)
    } _stats;
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_FRAME_CONVERTER_H
