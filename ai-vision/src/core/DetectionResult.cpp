/*
 * Copyright (c) 2016-2025 The ZLMediaKit project authors. All Rights Reserved.
 */

#include "DetectionResult.h"
#include <cmath>

using namespace std;

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
    // TODO: Implement proper JSON serialization
    return "{}";
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
    // TODO: Implement proper JSON serialization
    return "{}";
}

bool DetectionResult::fromJson(const string &json_str) {
    // TODO: Implement JSON deserialization
    return false;
}

} // namespace ai
} // namespace mediakit
