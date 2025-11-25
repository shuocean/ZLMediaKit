/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "FrameConverter.h"
#include "Util/logger.h"
#include "Util/util.h"
#include <cstring>
#include <algorithm>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// ==================== FrameData ====================

size_t FrameData::getSize() const {
    if (!data) return 0;
    
    switch (format) {
        case PixelFormat::YUV420P:
            return width * height * 3 / 2;
        case PixelFormat::NV12:
            return width * height * 3 / 2;
        case PixelFormat::RGB24:
        case PixelFormat::BGR24:
            return width * height * 3;
        case PixelFormat::RGBA:
        case PixelFormat::BGRA:
            return width * height * 4;
        case PixelFormat::GRAY:
            return width * height;
        default:
            return 0;
    }
}

bool FrameData::allocate(int w, int h, PixelFormat fmt) {
    release();
    
    width = w;
    height = h;
    format = fmt;
    
    // 256字节对齐，GPU友好
    size_t size = getSize();
    if (size == 0) return false;
    
    // 使用对齐分配
    #ifdef _WIN32
    data = (uint8_t*)_aligned_malloc(size, 256);
    #else
    if (posix_memalign((void**)&data, 256, size) != 0) {
        data = nullptr;
    }
    #endif
    
    if (!data) {
        ErrorL << "Failed to allocate frame memory: " << size << " bytes";
        return false;
    }
    
    owns_data = true;
    linesize = width * (format == PixelFormat::RGB24 || format == PixelFormat::BGR24 ? 3 : 4);
    
    return true;
}

void FrameData::release() {
    if (data && owns_data) {
        #ifdef _WIN32
        _aligned_free(data);
        #else
        free(data);
        #endif
        data = nullptr;
    }
    owns_data = false;
}

FrameData::FrameData(const FrameData &other) {
    if (other.data && other.owns_data) {
        allocate(other.width, other.height, other.format);
        if (data) {
            memcpy(data, other.data, getSize());
        }
    } else {
        data = other.data;
        owns_data = false;
    }
    width = other.width;
    height = other.height;
    linesize = other.linesize;
    format = other.format;
    pts = other.pts;
}

FrameData &FrameData::operator=(const FrameData &other) {
    if (this != &other) {
        release();
        if (other.data && other.owns_data) {
            allocate(other.width, other.height, other.format);
            if (data) {
                memcpy(data, other.data, getSize());
            }
        } else {
            data = other.data;
            owns_data = false;
        }
        width = other.width;
        height = other.height;
        linesize = other.linesize;
        format = other.format;
        pts = other.pts;
    }
    return *this;
}

FrameData::FrameData(FrameData &&other) noexcept {
    data = other.data;
    width = other.width;
    height = other.height;
    linesize = other.linesize;
    format = other.format;
    pts = other.pts;
    owns_data = other.owns_data;
    
    other.data = nullptr;
    other.owns_data = false;
}

FrameData &FrameData::operator=(FrameData &&other) noexcept {
    if (this != &other) {
        release();
        data = other.data;
        width = other.width;
        height = other.height;
        linesize = other.linesize;
        format = other.format;
        pts = other.pts;
        owns_data = other.owns_data;
        
        other.data = nullptr;
        other.owns_data = false;
    }
    return *this;
}

FrameData::~FrameData() {
    release();
}

// ==================== ConverterConfig ====================

bool ConverterConfig::isValid() const {
    return src_width > 0 && src_height > 0 &&
           dst_width > 0 && dst_height > 0;
}

// ==================== FrameConverter ====================

FrameConverter::Ptr FrameConverter::create(const ConverterConfig &config) {
    if (!config.isValid()) {
        ErrorL << "Invalid converter config";
        return nullptr;
    }
    
    auto converter = make_shared<SwscaleConverter>(config);
    if (!converter->initialize()) {
        ErrorL << "Failed to initialize converter";
        return nullptr;
    }
    
    return converter;
}

int FrameConverter::convertBatch(const FrameData *src_frames, 
                                 FrameData *dst_frames, 
                                 int count) {
    int success = 0;
    for (int i = 0; i < count; ++i) {
        if (convert(src_frames[i], dst_frames[i])) {
            success++;
        }
    }
    return success;
}

string FrameConverter::getStatistics() const {
    return "{}";
}

void FrameConverter::resetStatistics() {
}

// ==================== SwscaleConverter ====================

struct SwscaleConverter::Impl {
    SwsContext *sws_ctx = nullptr;
    AVPixelFormat src_fmt = AV_PIX_FMT_YUV420P;
    AVPixelFormat dst_fmt = AV_PIX_FMT_RGB24;
    bool initialized = false;
};

SwscaleConverter::SwscaleConverter(const ConverterConfig &config)
    : _impl(new Impl()) {
    _config = config;
}

SwscaleConverter::~SwscaleConverter() {
    if (_impl && _impl->sws_ctx) {
        sws_freeContext(_impl->sws_ctx);
    }
}

bool SwscaleConverter::initialize() {
    // 像素格式映射
    map<PixelFormat, AVPixelFormat> fmt_map = {
        {PixelFormat::YUV420P, AV_PIX_FMT_YUV420P},
        {PixelFormat::NV12, AV_PIX_FMT_NV12},
        {PixelFormat::RGB24, AV_PIX_FMT_RGB24},
        {PixelFormat::BGR24, AV_PIX_FMT_BGR24},
        {PixelFormat::RGBA, AV_PIX_FMT_RGBA},
        {PixelFormat::BGRA, AV_PIX_FMT_BGRA},
        {PixelFormat::GRAY, AV_PIX_FMT_GRAY8}
    };
    
    _impl->src_fmt = fmt_map[_config.src_format];
    _impl->dst_fmt = fmt_map[_config.dst_format];
    
    // 缩放算法
    int flags = SWS_BILINEAR;
    if (_config.quality == 0) flags = SWS_FAST_BILINEAR;
    else if (_config.quality == 2) flags = SWS_BICUBIC;
    
    _impl->sws_ctx = sws_getContext(
        _config.src_width, _config.src_height, _impl->src_fmt,
        _config.dst_width, _config.dst_height, _impl->dst_fmt,
        flags, nullptr, nullptr, nullptr
    );
    
    if (!_impl->sws_ctx) {
        ErrorL << "Failed to create swscale context";
        return false;
    }
    
    _impl->initialized = true;
    InfoL << "SwscaleConverter initialized: "
          << _config.src_width << "x" << _config.src_height << " -> "
          << _config.dst_width << "x" << _config.dst_height;
    
    return true;
}

bool SwscaleConverter::convert(const FrameData &src, FrameData &dst) {
    if (!_impl->initialized) return false;
    
    auto start = chrono::high_resolution_clock::now();
    
    // TODO: Phase 3 - 实际转换逻辑
    // 这里提供框架，Phase 3完善
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    
    _stats.convert_count++;
    _stats.total_time_us += duration.count();
    _stats.avg_time_ms = _stats.total_time_us / (float)_stats.convert_count / 1000.0f;
    
    return true;
}

int SwscaleConverter::convertBatch(const FrameData *src_frames, 
                                   FrameData *dst_frames, 
                                   int count) {
    return FrameConverter::convertBatch(src_frames, dst_frames, count);
}

string SwscaleConverter::getStatistics() const {
    // TODO: Phase 3 - JSON序列化
    return "{}";
}

void SwscaleConverter::resetStatistics() {
    _stats = Statistics();
}

} // namespace ai
} // namespace mediakit
