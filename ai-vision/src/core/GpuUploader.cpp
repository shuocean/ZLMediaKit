/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "GpuUploader.h"
#include "Util/logger.h"

#ifdef ENABLE_CUDA
#include <cuda_runtime.h>
#endif

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

// ==================== GpuFrame ====================

class GpuFrameImpl : public GpuFrame {
public:
    GpuFrameImpl(int width, int height, PixelFormat format, int device_id)
        : _width(width), _height(height), _format(format), _device_id(device_id) {
        #ifdef ENABLE_CUDA
        size_t size = width * height * 3; // Simplified
        cudaMalloc(&_device_ptr, size);
        _size = size;
        #endif
    }
    
    ~GpuFrameImpl() override {
        #ifdef ENABLE_CUDA
        if (_device_ptr) {
            cudaFree(_device_ptr);
        }
        #endif
    }
    
    void *getDevicePtr() const override { return _device_ptr; }
    int getWidth() const override { return _width; }
    int getHeight() const override { return _height; }
    PixelFormat getFormat() const override { return _format; }
    int getDeviceId() const override { return _device_id; }
    uint64_t getPts() const override { return _pts; }
    void setPts(uint64_t pts) override { _pts = pts; }
    size_t getSize() const override { return _size; }
    
private:
    void *_device_ptr = nullptr;
    int _width, _height, _device_id;
    PixelFormat _format;
    uint64_t _pts = 0;
    size_t _size = 0;
};

GpuFrame::Ptr GpuFrame::create(int width, int height, PixelFormat format, int device_id) {
    return make_shared<GpuFrameImpl>(width, height, format, device_id);
}

// ==================== UploaderConfig ====================

bool UploaderConfig::isValid() const {
    return device_id >= 0 && pool_size > 0;
}

// ==================== GpuUploader ====================

GpuUploader::Ptr GpuUploader::create(const UploaderConfig &config) {
    #ifdef ENABLE_CUDA
    auto uploader = make_shared<CudaUploader>(config);
    if (uploader->initialize()) {
        return uploader;
    }
    WarnL << "CUDA uploader initialization failed, falling back to CPU";
    #endif
    
    return make_shared<CpuFallbackUploader>(config);
}

string GpuUploader::getStatistics() const {
    return "{}";
}

void GpuUploader::resetStatistics() {
}

// ==================== CudaUploader ====================

#ifdef ENABLE_CUDA

struct CudaUploader::Impl {
    bool initialized = false;
};

CudaUploader::CudaUploader(const UploaderConfig &config)
    : _impl(new Impl()) {
    _config = config;
}

CudaUploader::~CudaUploader() {
}

bool CudaUploader::initialize() {
    cudaError_t err = cudaSetDevice(_config.device_id);
    if (err != cudaSuccess) {
        ErrorL << "Failed to set CUDA device: " << cudaGetErrorString(err);
        return false;
    }
    
    _impl->initialized = true;
    InfoL << "CudaUploader initialized on device " << _config.device_id;
    return true;
}

GpuFrame::Ptr CudaUploader::upload(const FrameData &cpu_frame) {
    if (!_impl->initialized) return nullptr;
    
    auto gpu_frame = GpuFrame::create(cpu_frame.width, cpu_frame.height, 
                                      cpu_frame.format, _config.device_id);
    if (!gpu_frame) return nullptr;
    
    // TODO: Phase 3 - 实际上传逻辑
    // cudaMemcpy(gpu_frame->getDevicePtr(), cpu_frame.data, ...);
    
    _stats.upload_count++;
    return gpu_frame;
}

bool CudaUploader::uploadAsync(const FrameData &cpu_frame, OnUploadCallback callback) {
    // TODO: Phase 3 - 异步上传
    auto gpu_frame = upload(cpu_frame);
    if (gpu_frame && callback) {
        callback(gpu_frame);
    }
    return gpu_frame != nullptr;
}

int CudaUploader::uploadBatch(const FrameData *cpu_frames,
                              GpuFrame::Ptr *gpu_frames,
                              int count) {
    int success = 0;
    for (int i = 0; i < count; ++i) {
        gpu_frames[i] = upload(cpu_frames[i]);
        if (gpu_frames[i]) success++;
    }
    return success;
}

bool CudaUploader::isGpuAvailable() const {
    return _impl->initialized;
}

bool CudaUploader::getMemoryInfo(size_t &free_memory, size_t &total_memory) const {
    if (!_impl->initialized) return false;
    return cudaMemGetInfo(&free_memory, &total_memory) == cudaSuccess;
}

string CudaUploader::getStatistics() const {
    // TODO: Phase 3 - JSON序列化
    return "{}";
}

void CudaUploader::resetStatistics() {
    _stats = Statistics();
}

void CudaUploader::waitAll() {
    if (_impl->initialized) {
        cudaDeviceSynchronize();
    }
}

#endif // ENABLE_CUDA

// ==================== CpuFallbackUploader ====================

CpuFallbackUploader::CpuFallbackUploader(const UploaderConfig &config) {
    _config = config;
}

CpuFallbackUploader::~CpuFallbackUploader() {
}

bool CpuFallbackUploader::initialize() {
    InfoL << "Using CPU fallback uploader (no GPU)";
    return true;
}

GpuFrame::Ptr CpuFallbackUploader::upload(const FrameData &cpu_frame) {
    // CPU模式：直接返回CPU内存包装
    return nullptr; // TODO: Phase 3 - CPU frame wrapper
}

bool CpuFallbackUploader::uploadAsync(const FrameData &cpu_frame, OnUploadCallback callback) {
    auto frame = upload(cpu_frame);
    if (frame && callback) {
        callback(frame);
    }
    return frame != nullptr;
}

int CpuFallbackUploader::uploadBatch(const FrameData *cpu_frames,
                                     GpuFrame::Ptr *gpu_frames,
                                     int count) {
    return 0; // TODO: Phase 3
}

bool CpuFallbackUploader::getMemoryInfo(size_t &free_memory, size_t &total_memory) const {
    free_memory = total_memory = 0;
    return false;
}

} // namespace ai
} // namespace mediakit
