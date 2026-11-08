#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace strings::ui {

struct PerformanceKnob {
    float horizontal, vertical, radius;
    bool contains(float positionX, float positionY) const {
        const float distanceX = positionX - horizontal, distanceY = positionY - vertical;
        return distanceX * distanceX + distanceY * distanceY <= (radius + 12) * (radius + 12);
    }
    static float drag(float initial, float deltaY, bool fine) {
        if (!std::isfinite(initial) || !std::isfinite(deltaY)) return 0;
        return std::clamp(initial + deltaY / 160 * (fine ? 0.1f : 1.0f), 0.0f, 1.0f);
    }
};

inline constexpr std::array<PerformanceKnob, 3> performanceKnobs {{{145, 263, 42}, {350, 263, 42}, {555, 263, 42}}};

}
