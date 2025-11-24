/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-style license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_DETECTION_RESULT_H
#define ZLMEDIAKIT_DETECTION_RESULT_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace mediakit {
namespace ai {

/**
 * @brief 检测框（目标检测结果）
 * @note 兼容性：使用归一化坐标[0,1]，适配不同分辨率
 */
struct DetectionBox {
    int class_id;           ///< 类别ID
    std::string label;      ///< 类别标签（如"person", "car"）
    float confidence;       ///< 置信度 [0, 1]
    float x, y, w, h;      ///< 归一化坐标：x,y为左上角，w,h为宽高

    /**
     * @brief 获取绝对坐标
     * @param width 图像宽度
     * @param height 图像高度
     */
    void getAbsoluteCoords(int width, int height,
                          int &abs_x, int &abs_y, int &abs_w, int &abs_h) const {
        abs_x = static_cast<int>(x * width);
        abs_y = static_cast<int>(y * height);
        abs_w = static_cast<int>(w * width);
        abs_h = static_cast<int>(h * height);
    }

    /**
     * @brief 获取中心点坐标（归一化）
     */
    void getCenter(float &cx, float &cy) const {
        cx = x + w / 2.0f;
        cy = y + h / 2.0f;
    }

    /**
     * @brief 计算与另一个框的IoU
     * @param other 另一个检测框
     * @return IoU值 [0, 1]
     */
    float iou(const DetectionBox &other) const;

    /**
     * @brief 序列化为JSON
     */
    std::string toJson() const;
};

/**
 * @brief AI检测结果
 * @note 模块化设计：统一的检测结果格式，适用于所有模型
 */
class DetectionResult {
public:
    using Ptr = std::shared_ptr<DetectionResult>;

    std::vector<DetectionBox> boxes;    ///< 检测框列表
    uint64_t timestamp;                 ///< 时间戳（毫秒）
    int width;                          ///< 原始图像宽度
    int height;                         ///< 原始图像高度
    std::string model_id;               ///< 使用的模型ID
    std::string stream_id;              ///< 来源流ID
    float inference_time_ms;            ///< 推理耗时（毫秒）

    /**
     * @brief 按类别过滤
     * @param class_ids 目标类别ID列表
     * @return 过滤后的检测框
     */
    std::vector<DetectionBox> filterByClass(const std::vector<int> &class_ids) const;

    /**
     * @brief 按置信度过滤
     * @param threshold 置信度阈值
     * @return 过滤后的检测框
     */
    std::vector<DetectionBox> filterByConfidence(float threshold) const;

    /**
     * @brief 按ROI区域过滤
     * @param roi_x ROI左上角x（归一化）
     * @param roi_y ROI左上角y（归一化）
     * @param roi_w ROI宽度（归一化）
     * @param roi_h ROI高度（归一化）
     * @return ROI内的检测框
     * @note 可配置：ROI可通过配置文件或API设置
     */
    std::vector<DetectionBox> filterByROI(float roi_x, float roi_y,
                                          float roi_w, float roi_h) const;

    /**
     * @brief 获取指定类别的数量
     * @param class_id 类别ID
     * @return 数量
     */
    int getClassCount(int class_id) const;

    /**
     * @brief 获取所有检测到的类别
     * @return 类别ID列表
     */
    std::vector<int> getAllClasses() const;

    /**
     * @brief 序列化为JSON
     * @return JSON字符串
     * @note 可视化管理：供Web界面展示
     */
    std::string toJson() const;

    /**
     * @brief 从JSON反序列化
     * @param json_str JSON字符串
     * @return 解析是否成功
     */
    bool fromJson(const std::string &json_str);
};

} // namespace ai
} // namespace mediakit

#endif // ZLMEDIAKIT_DETECTION_RESULT_H
