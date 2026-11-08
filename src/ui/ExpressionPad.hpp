#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace strings::ui {

struct ExpressionPoint {
    float motion = 0;
    float weight = 0;
    float gate = 0;
};

struct ExpressionPadLayout {
    float left = 28, top = 153, width = 650, height = 204;
    float plotLeft() const { return left + 30; }
    float plotTop() const { return top + 63; }
    float plotWidth() const { return width - 60; }
    float plotHeight() const { return height - 94; }
    bool contains(float horizontal, float vertical) const {
        return horizontal >= plotLeft() && horizontal <= plotLeft() + plotWidth()
            && vertical >= plotTop() && vertical <= plotTop() + plotHeight();
    }
    ExpressionPoint at(float horizontal, float vertical) const {
        float motion = std::clamp((horizontal - plotLeft()) / plotWidth() * 2 - 1, -1.0f, 1.0f);
        if (std::abs(motion) < 0.025f) motion = 0;
        return {motion, std::clamp(1 - (vertical - plotTop()) / plotHeight(), 0.0f, 1.0f), 1};
    }
};

class ExpressionPad {
public:
    struct Sample { ExpressionPoint point; double time = 0; };
    static constexpr size_t capacity = 96;
    static constexpr double trailSeconds = 2.5;

    bool begin(ExpressionPoint point) {
        if (!valid(point)) return false;
        held = true;
        target = bounded(point);
        target.gate = 1;
        return true;
    }
    bool move(ExpressionPoint point) {
        if (!held) return false;
        if (!valid(point)) { release(); return false; }
        target = bounded(point);
        target.gate = 1;
        return true;
    }
    void release() {
        held = false;
        target.motion = target.gate = 0;
    }
    bool dragging() const { return held; }
    const ExpressionPoint& requested() const { return target; }
    const ExpressionPoint& observed() const { return readback; }
    bool ready() const { return haveReadback; }
    bool active() const { return sounding && readback.gate > 0.01f; }
    size_t size() const { return count; }
    const Sample& sample(size_t index) const { return trail[(first + index) % capacity]; }
    void clearTrail() { first = count = 0; }
    void observe(ExpressionPoint point, double now, bool voicesActive) {
        if (!valid(point) || !std::isfinite(now)) return;
        if (haveReadback && now < lastObservation) clearTrail();
        readback = bounded(point);
        sounding = voicesActive;
        haveReadback = true;
        lastObservation = now;
        while (count && now - sample(0).time > trailSeconds) {
            first = (first + 1) % capacity;
            --count;
        }
        if (!active() || (count && now - sample(count - 1).time < 1.0 / 30.0)) return;
        if (count == capacity) { first = (first + 1) % capacity; --count; }
        trail[(first + count) % capacity] = {readback, now};
        ++count;
    }
private:
    static bool valid(ExpressionPoint point) {
        return std::isfinite(point.motion) && std::isfinite(point.weight) && std::isfinite(point.gate);
    }
    static ExpressionPoint bounded(ExpressionPoint point) {
        return {std::clamp(point.motion, -1.0f, 1.0f), std::clamp(point.weight, 0.0f, 1.0f),
            std::clamp(point.gate, 0.0f, 1.0f)};
    }
    ExpressionPoint target {}, readback {};
    std::array<Sample, capacity> trail {};
    size_t first = 0, count = 0;
    bool held = false, haveReadback = false, sounding = false;
    double lastObservation = 0;
};

}
