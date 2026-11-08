#pragma once
#include "PerformanceSession.hpp"
#include "PerformanceKnob.hpp"
#include "NanoVG.hpp"
#include <cstdio>

namespace strings::ui {

class PerformancePainter {
public:
    using Color = DGL_NAMESPACE::Color;
    explicit PerformancePainter(DGL_NAMESPACE::NanoVG& renderer) : canvas(renderer) {}
    const Color paper {246, 245, 240}, ink {32, 59, 53}, pine {32, 76, 66};
    const Color sage {220, 226, 213}, brass {186, 145, 79}, muted {102, 118, 109}, line {204, 213, 199};
    void box(float left, float top, float width, float height, Color color, float radius = 0) {
        canvas.beginPath(); canvas.roundedRect(left, top, width, height, radius); canvas.fillColor(color); canvas.fill();
    }
    void label(float left, float top, const char* value, float size = 12, bool subdued = false, bool centered = false) {
        canvas.fontFace(NANOVG_DEJAVU_SANS_TTF); canvas.fontSize(size); canvas.fillColor(subdued ? muted : ink);
        canvas.textAlign((centered ? DGL_NAMESPACE::NanoVG::ALIGN_CENTER : DGL_NAMESPACE::NanoVG::ALIGN_LEFT)
            | DGL_NAMESPACE::NanoVG::ALIGN_BASELINE); canvas.text(left, top, value, nullptr);
    }
    void button(float left, float top, float width, const char* text, bool selected = false, bool enabled = true) {
        canvas.beginPath(); canvas.roundedRect(left, top, width, 34, 5);
        canvas.fillColor(selected ? pine : paper); canvas.fill();
        canvas.strokeColor(enabled ? muted : line); canvas.strokeWidth(1); canvas.stroke();
        canvas.fontFace(NANOVG_DEJAVU_SANS_TTF); canvas.fontSize(12);
        canvas.fillColor(!enabled ? muted.withAlpha(0.5f) : selected ? paper : ink);
        canvas.textAlign(DGL_NAMESPACE::NanoVG::ALIGN_CENTER | DGL_NAMESPACE::NanoVG::ALIGN_BASELINE);
        canvas.text(left + width * 0.5f, top + 22, text, nullptr);
    }
    void rule(float top) { box(28, top, 984, 1, line); }
private:
    DGL_NAMESPACE::NanoVG& canvas;
};

class KeyboardPerformanceView {
public:
    void draw(DGL_NAMESPACE::NanoVG& canvas, float weight, float contact, float vibrato) const {
        PerformancePainter paint(canvas);
        constexpr const char* titles[] = {"Energy", "Tone", "Vibrato"};
        const float amounts[] = {weight, 1 - contact, vibrato};
        for (size_t index = 0; index < performanceKnobs.size(); ++index) {
            const auto& knob = performanceKnobs[index];
            const float value = std::clamp(amounts[index], 0.0f, 1.0f);
            constexpr float start = 2.3561945f, sweep = 4.712389f;
            paint.label(knob.horizontal, 203, titles[index], 19, false, true);
            canvas.beginPath(); canvas.circle(knob.horizontal, knob.vertical, knob.radius - 8);
            canvas.fillColor(paint.sage.withAlpha(0.35f)); canvas.fill();
            canvas.beginPath(); canvas.arc(knob.horizontal, knob.vertical, knob.radius, start, start + sweep, DGL_NAMESPACE::NanoVG::CW);
            canvas.strokeColor(paint.sage); canvas.strokeWidth(4); canvas.stroke();
            if (value > 0) {
                canvas.beginPath(); canvas.arc(knob.horizontal, knob.vertical, knob.radius, start, start + value * sweep, DGL_NAMESPACE::NanoVG::CW);
                canvas.strokeColor(paint.pine); canvas.strokeWidth(4); canvas.stroke();
            }
            for (int tick = 0; tick <= 8; ++tick) {
                const float angle = start + sweep * tick / 8;
                canvas.beginPath(); canvas.moveTo(knob.horizontal + std::cos(angle) * (knob.radius + 7), knob.vertical + std::sin(angle) * (knob.radius + 7));
                canvas.lineTo(knob.horizontal + std::cos(angle) * (knob.radius + 10), knob.vertical + std::sin(angle) * (knob.radius + 10));
                canvas.strokeColor(paint.muted.withAlpha(0.5f)); canvas.strokeWidth(1); canvas.stroke();
            }
            const float angle = start + sweep * value;
            canvas.beginPath(); canvas.circle(knob.horizontal + std::cos(angle) * knob.radius, knob.vertical + std::sin(angle) * knob.radius, 4);
            canvas.fillColor(paint.brass); canvas.fill();
            char amount[16]; std::snprintf(amount, sizeof(amount), "%.0f%%", value * 100);
            paint.label(knob.horizontal, knob.vertical + 9, amount, 25, false, true);
        }
    }
};

}
