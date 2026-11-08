#pragma once
#include "ExpressionPad.hpp"
#include "NanoVG.hpp"

namespace strings::ui {

class ExpressionPadView {
public:
    void draw(DGL_NAMESPACE::NanoVG& canvas, const ExpressionPad& model,
              const ExpressionPadLayout& layout, double now, bool direct, const char* font) const {
        using Color = DGL_NAMESPACE::Color;
        const Color ink {32, 59, 53}, muted {102, 118, 109};
        const Color guide {204, 213, 199}, accent {186, 145, 79}, trailColor {32, 76, 66};
        const float left = layout.plotLeft(), top = layout.plotTop();
        const float width = layout.plotWidth(), height = layout.plotHeight();
        const auto horizontal = [left, width](float motion) { return left + (motion + 1) * width * 0.5f; };
        const auto vertical = [top, height](float weight) { return top + (1 - weight) * height; };
        const auto label = [&](float horizontalPosition, float verticalPosition, const char* text, float size, Color color, int align) {
            canvas.fontFace(font); canvas.fontSize(size); canvas.fillColor(color);
            canvas.textAlign(align | DGL_NAMESPACE::NanoVG::ALIGN_BASELINE);
            canvas.text(horizontalPosition, verticalPosition, text, nullptr);
        };
        label(left, layout.top + 29, "Shape the phrase", 22, ink, DGL_NAMESPACE::NanoVG::ALIGN_LEFT);
        const char* mode = direct ? (model.dragging() ? "Direct / held" : "Direct / ready") : "Follow controls";
        label(left + width, layout.top + 28, mode, 11, direct ? accent : muted, DGL_NAMESPACE::NanoVG::ALIGN_RIGHT);
        label(left, top - 9, "Bow weight / firm at top", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_LEFT);
        label(left + width, top - 9, "Smoothed engine commands", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_RIGHT);
        label(left + 8, top + height - 8, "Light", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_LEFT);
        canvas.beginPath(); canvas.moveTo(horizontal(0), top); canvas.lineTo(horizontal(0), top + height);
        canvas.strokeColor(guide); canvas.strokeWidth(1); canvas.stroke();
        canvas.save(); canvas.scissor(left, top, width, height);
        for (size_t index = 1; index < model.size(); ++index) {
            const auto& previous = model.sample(index - 1);
            const auto& current = model.sample(index);
            if (current.time - previous.time > 0.15) continue;
            const float alpha = static_cast<float>(std::clamp(1 - (now - current.time) / ExpressionPad::trailSeconds, 0.0, 1.0));
            canvas.beginPath(); canvas.moveTo(horizontal(previous.point.motion), vertical(previous.point.weight));
            canvas.lineTo(horizontal(current.point.motion), vertical(current.point.weight));
            canvas.strokeColor(trailColor.withAlpha(alpha * 0.4f)); canvas.strokeWidth(1.5f); canvas.stroke();
        }
        canvas.restore();
        if (model.ready()) {
            const auto point = model.observed();
            canvas.beginPath(); canvas.circle(horizontal(point.motion), vertical(point.weight), 5);
            canvas.fillColor(model.active() ? accent : muted); canvas.fill();
        }
        if (model.dragging()) {
            const auto target = model.requested();
            canvas.beginPath(); canvas.circle(horizontal(target.motion), vertical(target.weight), 9);
            canvas.strokeColor(ink); canvas.strokeWidth(1.5f); canvas.stroke();
        }
        label(left, top + height + 14, "Reverse", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_LEFT);
        label(horizontal(0), top + height + 14, "Rest / signed motion", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_CENTER);
        label(left + width, top + height + 14, "Forward", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_RIGHT);
        label(left, layout.top + layout.height - 6, model.ready()
            ? "Dot: engine commands   Ring: pointer target   Y: weight (not sensed pressure)"
            : "Waiting for engine readback", 10, muted, DGL_NAMESPACE::NanoVG::ALIGN_LEFT);
    }
};

}
