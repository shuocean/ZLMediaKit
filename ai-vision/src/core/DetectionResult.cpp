/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "DetectionResult.h"
#include "JsonHelper.h"
#include "Util/logger.h"
#include <sstream>
#include <algorithm>

using namespace std;
using namespace toolkit;

namespace mediakit {
namespace ai {

float DetectionBox::iou(const DetectionBox &other) const {
    float x1 = max(x, other.x);
    float y1 = max(y, other.y);
    float x2 = min(x + w, other.x + other.w);
    float y2 = min(y + h, other.y + other.h);
    
    if (x2 < x1 || y2 < y1) return 0.0f;
    
    float inter = (x2 - x1) * (y2 - y1);
    float area1 = w * h;
    float area2 = other.w * other.h;
    float union_area = area1 + area2 - inter;
    
    return union_area > 0 ? inter / union_area : 0.0f;
}

string DetectionBox::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("x", x);
    ss << JsonHelper::field("y", y);
    ss << JsonHelper::field("w", w);
    ss << JsonHelper::field("h", h);
    ss << JsonHelper::field("confidence", confidence);
    ss << JsonHelper::field("class_id", class_id);
    ss << JsonHelper::field("label", label, true);
    ss << JsonHelper::objectEnd();
    return ss.str();
}

bool DetectionBox::fromJson(const string &json_str) {
    JsonHelper::parseFloat(json_str, "x", x);
    JsonHelper::parseFloat(json_str, "y", y);
    JsonHelper::parseFloat(json_str, "w", w);
    JsonHelper::parseFloat(json_str, "h", h);
    JsonHelper::parseFloat(json_str, "confidence", confidence);
    JsonHelper::parseInt(json_str, "class_id", class_id);
    JsonHelper::parseString(json_str, "label", label);
    return true;
}

vector<DetectionBox> DetectionResult::filterByClass(const vector<int> &class_ids) const {
    vector<DetectionBox> result;
    for (const auto &box : boxes) {
        for (int id : class_ids) {
            if (box.class_id == id) {
                result.push_back(box);
                break;
            }
        }
    }
    return result;
}

vector<DetectionBox> DetectionResult::filterByConfidence(float threshold) const {
    vector<DetectionBox> result;
    for (const auto &box : boxes) {
        if (box.confidence >= threshold) {
            result.push_back(box);
        }
    }
    return result;
}

vector<DetectionBox> DetectionResult::filterByROI(float roi_x, float roi_y,
                                                   float roi_w, float roi_h) const {
    vector<DetectionBox> result;
    for (const auto &box : boxes) {
        float cx, cy;
        box.getCenter(cx, cy);
        if (cx >= roi_x && cx <= roi_x + roi_w &&
            cy >= roi_y && cy <= roi_y + roi_h) {
            result.push_back(box);
        }
    }
    return result;
}

int DetectionResult::getClassCount(int class_id) const {
    int count = 0;
    for (const auto &box : boxes) {
        if (box.class_id == class_id) count++;
    }
    return count;
}

vector<int> DetectionResult::getAllClasses() const {
    vector<int> classes;
    for (const auto &box : boxes) {
        if (find(classes.begin(), classes.end(), box.class_id) == classes.end()) {
            classes.push_back(box.class_id);
        }
    }
    return classes;
}

string DetectionResult::toJson() const {
    stringstream ss;
    ss << JsonHelper::objectStart();
    ss << JsonHelper::field("stream_id", stream_id);
    ss << JsonHelper::field("model_id", model_id);
    ss << JsonHelper::field("timestamp", (int)timestamp);
    ss << JsonHelper::field("width", width);
    ss << JsonHelper::field("height", height);
    ss << JsonHelper::field("inference_time_ms", inference_time_ms);
    
    // boxes数组
    ss << "\"boxes\":[";
    for (size_t i = 0; i < boxes.size(); ++i) {
        ss << boxes[i].toJson();
        if (i < boxes.size() - 1) ss << ",";
    }
    ss << "]";
    
    ss << JsonHelper::objectEnd();
    return ss.str();
}

bool DetectionResult::fromJson(const string &json_str) {
    JsonHelper::parseString(json_str, "stream_id", stream_id);
    JsonHelper::parseString(json_str, "model_id", model_id);
    
    int ts = 0;
    JsonHelper::parseInt(json_str, "timestamp", ts);
    timestamp = ts;
    
    JsonHelper::parseInt(json_str, "width", width);
    JsonHelper::parseInt(json_str, "height", height);
    JsonHelper::parseFloat(json_str, "inference_time_ms", inference_time_ms);
    
    // 解析boxes数组
    boxes.clear();
    string array_str = JsonHelper::extractArray(json_str, "boxes");
    vector<string> box_objects = JsonHelper::splitObjectArray(array_str);
    
    for (const auto &box_str : box_objects) {
        DetectionBox box;
        if (box.fromJson(box_str)) {
            boxes.push_back(box);
        }
    }
    
    InfoL << "DetectionResult loaded from JSON: " << boxes.size() << " boxes";
    return true;
}

} // namespace ai
} // namespace mediakit
