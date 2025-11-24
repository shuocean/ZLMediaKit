/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_INFERENCE_ENGINE_H
#define ZLMEDIAKIT_INFERENCE_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace mediakit {
namespace ai {

/**
 * @brief AI推理执行后端
 * @note 配置化设计：支持多种后端，可通过配置文件切换
 */
enum class ExecutionProvider {
    CPU,        ///< CPU推理（显存不足时的降级选项）
    CUDA,       ///< NVIDIA CUDA（GPU推理，推荐）
    TensorRT,   ///< TensorRT加速推理（最快，需转换模型）
    Auto        ///< 自动选择（优先GPU）
};

/**
 * @brief AI推理引擎配置
 * @note 配置化原则：所有参数可通过JSON配置文件或API动态设置
 */
struct InferenceConfig {
    std::string model_path;                  ///< 模型文件路径（支持相对/绝对路径）
    ExecutionProvider provider = ExecutionProvider::Auto;  ///< 执行后端
    int device_id = 0;                       ///< GPU设备ID（多卡环境）
    int batch_size = 1;                      ///< 批处理大小（性能优化）
    int num_threads = 4;                     ///< CPU线程数（CPU模式）
    bool enable_cuda_graph = true;           ///< 启用CUDA Graph优化
    bool enable_fp16 = false;                ///< 启用FP16推理（降低显存，略降精度）
    int intra_op_num_threads = 0;            ///< ONNX Runtime内部并行度
    int inter_op_num_threads = 0;            ///< ONNX Runtime操作间并行度

    /**
     * @brief 从JSON字符串加载配置
     * @param json_str JSON格式的配置字符串
     * @return 解析是否成功
     */
    bool fromJson(const std::string &json_str);

    /**
     * @brief 转换为JSON字符串
     * @return JSON格式的配置
     */
    std::string toJson() const;
};

/**
 * @brief AI推理引擎抽象接口
 * @note 模块化设计：通过抽象接口支持多种推理框架
 * @note 性能优化：支持批处理、异步推理、内存复用
 */
class InferenceEngine {
public:
    using Ptr = std::shared_ptr<InferenceEngine>;

    /**
     * @brief 创建推理引擎实例
     * @param config 推理配置
     * @return 推理引擎智能指针
     * @note 工厂模式：根据配置自动选择合适的后端
     */
    static Ptr create(const InferenceConfig &config);

    virtual ~InferenceEngine() = default;

    /**
     * @brief 初始化推理引擎
     * @return 初始化是否成功
     * @note 最小化影响：失败时不影响主流程，仅记录日志
     */
    virtual bool initialize() = 0;

    /**
     * @brief 检查引擎是否就绪
     * @return true-就绪 false-未就绪
     */
    virtual bool isReady() const = 0;

    /**
     * @brief 获取输入层名称列表
     * @return 输入层名称数组
     */
    virtual std::vector<std::string> getInputNames() const = 0;

    /**
     * @brief 获取输出层名称列表
     * @return 输出层名称数组
     */
    virtual std::vector<std::string> getOutputNames() const = 0;

    /**
     * @brief 获取输入层形状
     * @param name 输入层名称
     * @return 形状数组，如 [1, 3, 640, 640]
     */
    virtual std::vector<int64_t> getInputShape(const std::string &name) const = 0;

    /**
     * @brief 执行推理
     * @param inputs 输入数据映射：名称 -> 数据指针
     * @param outputs 输出数据映射：名称 -> 数据指针（需预分配）
     * @return 推理是否成功
     * @note 性能优化：支持批处理，单次调用可处理多帧
     * @note 兼容性：异常情况下返回false，不抛出异常
     */
    virtual bool infer(const std::map<std::string, void*> &inputs,
                      std::map<std::string, void*> &outputs) = 0;

    /**
     * @brief 异步推理（可选实现）
     * @param inputs 输入数据
     * @param callback 推理完成回调
     * @return 提交是否成功
     * @note 性能优化：支持异步推理，不阻塞主线程
     */
    virtual bool inferAsync(const std::map<std::string, void*> &inputs,
                           std::function<void(std::map<std::string, void*>)> callback) {
        return false; // 默认不支持，由子类实现
    }

    /**
     * @brief 获取推理统计信息
     * @return JSON格式的统计信息
     * @note 可视化管理：提供性能统计供监控使用
     */
    virtual std::string getStatistics() const {
        return "{}";
    }

    /**
     * @brief 重置推理引擎
     * @note 错误恢复：清理资源，重新初始化
     */
    virtual void reset() {}

protected:
    InferenceConfig _config;  ///< 推理配置（受保护，子类可访问）
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_INFERENCE_ENGINE_H
