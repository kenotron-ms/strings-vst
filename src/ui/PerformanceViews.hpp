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

class InputMapView {
public:
    void draw(DGL_NAMESPACE::NanoVG& canvas, int route, float contact) const {
        PerformancePainter paint(canvas);
        paint.label(715, 189, "Your hands, the instrument", 16);
        paint.label(715, 215, route == 3 ? "Notes first, expression second" : route == 2 ? "Controller mapping via MIDI adapter" : "Position becomes bow expression", 11, true);
        const auto segment = [&](float left, float top, float right, float bottom) {
            canvas.beginPath(); canvas.moveTo(left, top); canvas.lineTo(right, bottom);
            canvas.strokeColor(paint.pine); canvas.strokeWidth(1.5f); canvas.stroke();
        };
        if (route == 3) {
            constexpr const char* keys[] = {"A", "S", "D", "F", "G", "H", "J", "K"};
            for (int index = 0; index < 8; ++index) {
                paint.box(719 + index * 36.0f, 261, 33, 64, paint.sage, 3);
                paint.label(735 + index * 36.0f, 310, keys[index], 12, false, true);
            }
            paint.label(715, 348, "DAW MIDI / hold keys / overlap for legato", 10, true);
        } else if (route == 2) {
            canvas.beginPath(); canvas.moveTo(734, 301);
            canvas.bezierTo(740, 255, 769, 245, 802, 263);
            canvas.bezierTo(828, 256, 865, 256, 891, 263);
            canvas.bezierTo(925, 245, 956, 269, 962, 319);
            canvas.bezierTo(978, 373, 954, 384, 930, 353);
            canvas.lineTo(904, 323); canvas.lineTo(791, 323); canvas.lineTo(766, 353);
            canvas.bezierTo(736, 390, 715, 368, 734, 301); canvas.closePath();
            canvas.strokeColor(paint.pine); canvas.strokeWidth(1.5f); canvas.stroke();
            for (float center : {802.0f, 891.0f}) {
                canvas.beginPath(); canvas.circle(center, 307, 17); canvas.stroke();
                canvas.beginPath(); canvas.circle(center, 307, 10); canvas.stroke();
            }
            segment(925, 266, 957, 241); paint.label(873, 235, "R2 / weight", 11);
            segment(802, 307, 738, 401); paint.label(715, 419, "Left stick / motion", 11);
            paint.label(715, 448, "Device detection lives in the adapter", 10, true);
            paint.label(715, 464, "Touchpad / haptics not native here", 10, true);
        } else {
            canvas.beginPath(); canvas.roundedRect(786, 254, 94, 122, 39);
            canvas.strokeColor(paint.pine); canvas.strokeWidth(1.5f); canvas.stroke();
            segment(833, 255, 833, 306); segment(788, 306, 878, 306);
            canvas.beginPath(); canvas.circle(833, 280, 5); canvas.fillColor(paint.brass); canvas.fill();
            segment(746, 328, 770, 328); segment(746, 328, 752, 322); segment(746, 328, 752, 334);
            segment(895, 328, 919, 328); segment(919, 328, 913, 322); segment(919, 328, 913, 334);
            paint.label(715, 419, "Left / right: signed bow motion", 11);
            paint.label(715, 440, "Up / down: light to firm weight", 11);
            paint.label(715, 464, route == 1 ? "Touchpad pointer; no sensed pressure" : "Hold to sustain. Release to lift.", 10, true);
        }
        char caption[40]; std::snprintf(caption, sizeof(caption), "Contact point  %.0f%%", contact * 100);
        if (route != 3) paint.label(715, 493, caption, 11, true);
    }
};

class KeyboardPerformanceView {
public:
    void draw(DGL_NAMESPACE::NanoVG& canvas, float weight, float speed, float contact, float vibrato, bool active) const {
        PerformancePainter paint(canvas);
        paint.label(58, 151, "Play notes. Shape the feeling.", 22);
        paint.label(58, 179, "Drag up to shape. Shift for detail. Double-click to reset.", 12, true);
        constexpr const char* titles[] = {"Energy", "Tone", "Vibrato"};
        constexpr const char* descriptions[] = {"Bow force + speed", "Contact + brightness", "Pitch movement"};
        constexpr const char* ranges[] = {"Light to strong", "Round to edgy", "Still to expressive"};
        const float amounts[] = {weight, 1 - contact, vibrato};
        for (size_t index = 0; index < performanceKnobs.size(); ++index) {
            const auto& knob = performanceKnobs[index];
            const float value = std::clamp(amounts[index], 0.0f, 1.0f);
            constexpr float start = 2.3561945f, sweep = 4.712389f;
            paint.label(knob.horizontal, 207, titles[index], 15, false, true);
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
            paint.label(knob.horizontal, knob.vertical + 7, amount, 19, false, true);
            paint.label(knob.horizontal, 331, ranges[index], 11, false, true);
            paint.label(knob.horizontal, 347, descriptions[index], 10, true, true);
        }
        char caption[96];
        std::snprintf(caption, sizeof(caption), "%s   Bow force %.0f%% / speed %.0f%%", active ? "Sounding" : "Ready for notes", weight * 100, speed * 100);
        paint.label(58, 365, caption, 9, true);
    }
};

class PhraseRibbonView {
public:
    void draw(DGL_NAMESPACE::NanoVG& canvas, double elapsed, bool playing, const ExpressionTake& take,
              const PhraseEditor& phrase, int selected, int root, bool expanded, float baseline) const {
        PerformancePainter paint(canvas);
        const PhraseRollLayout area(expanded);
        paint.rule(area.top);
        paint.label(28, area.top + 24, "Local phrase", 13);
        paint.label(28, area.top + 43, "8 s / 120 BPM", 10, true);
        if (expanded) {
            paint.label(28, area.top + 81, "Drag notes to move", 10, true);
            paint.label(28, area.top + 99, "Right edge: duration", 10, true);
            paint.label(28, area.top + 117, "Undo above", 10, true);
        }
        constexpr const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        for (int semitone = 0; semitone <= 12; ++semitone) {
            const int pitch = root + semitone, chroma = pitch % 12;
            const bool black = chroma == 1 || chroma == 3 || chroma == 6 || chroma == 8 || chroma == 10;
            paint.box(PhraseRollLayout::left, area.noteTop(semitone), PhraseRollLayout::width, area.rowHeight - 0.5f,
                black ? paint.sage.withAlpha(0.45f) : paint.sage.withAlpha(0.12f));
            if (expanded && (chroma == 0 || semitone == 0 || semitone == 12)) {
                char name[16]; std::snprintf(name, sizeof(name), "%s%d", names[chroma], pitch / 12 - 1);
                paint.label(146, area.noteTop(semitone) + area.rowHeight - 1, name, 9, true);
            }
        }
        for (int beat = 0; beat <= 16; ++beat) {
            const float horizontal = area.horizontal(beat * 0.5);
            paint.box(horizontal, area.notesTop, 0.7f, area.rowHeight * 13, paint.line.withAlpha(beat % 4 ? 0.4f : 0.9f));
            if (beat % 4 == 0) {
                char bar[8]; std::snprintf(bar, sizeof(bar), "%d", beat / 4 + 1);
                paint.label(horizontal, area.notesTop - 7, bar, 10, true);
            }
        }
        for (size_t index = 0; index < phrase.notes().size(); ++index) {
            const auto& note = phrase.notes()[index];
            paint.box(area.horizontal(note.start), area.noteTop(note.semitone),
                std::max(2.0f, area.horizontal(note.start + note.length) - area.horizontal(note.start) - 1),
                std::max(3.0f, area.rowHeight - 1), static_cast<int>(index) == selected ? paint.brass : paint.pine, 1);
        }
        paint.label(28, area.curveTop + 15, take.isRecording() ? "Recording weight" : "Bow weight", 12);
        if (expanded) paint.label(28, area.curveTop + 34, "Drag to revise", 10, true);
        paint.box(PhraseRollLayout::left, area.curveTop, PhraseRollLayout::width, area.curveHeight, paint.sage.withAlpha(0.25f));
        canvas.beginPath(); canvas.moveTo(PhraseRollLayout::left, area.curveTop + area.curveHeight);
        if (take.size() < 2) {
            const float height = area.curveTop + (1 - baseline) * area.curveHeight;
            canvas.lineTo(PhraseRollLayout::left, height);
            canvas.lineTo(PhraseRollLayout::left + PhraseRollLayout::width, height);
        } else for (size_t index = 0; index < take.size(); ++index) {
            const auto& frame = take.frame(index);
            canvas.lineTo(area.horizontal(frame.time), area.curveTop + (1 - frame.point.weight) * area.curveHeight);
        }
        canvas.lineTo(PhraseRollLayout::left + PhraseRollLayout::width, area.curveTop + area.curveHeight);
        canvas.closePath(); canvas.fillColor(paint.brass.withAlpha(0.12f)); canvas.fill();
        if (take.size() < 2) paint.box(PhraseRollLayout::left, area.curveTop + (1 - baseline) * area.curveHeight,
            PhraseRollLayout::width, 1, paint.muted.withAlpha(0.5f));
        else {
            canvas.beginPath();
            for (size_t index = 0; index < take.size(); ++index) {
                const auto& frame = take.frame(index);
                const float horizontal = area.horizontal(frame.time);
                const float vertical = area.curveTop + (1 - frame.point.weight) * area.curveHeight;
                if (!index) canvas.moveTo(horizontal, vertical); else canvas.lineTo(horizontal, vertical);
            }
            canvas.strokeColor(paint.pine); canvas.strokeWidth(1.5f); canvas.stroke();
        }
        if (playing) paint.box(area.horizontal(elapsed), area.notesTop, 1.5f, area.curveTop + area.curveHeight - area.notesTop, paint.brass);
        if (expanded) paint.label(182, 643, "Plugin-owned phrase, not your DAW clip. Weight edits preserve motion and lift.", 10, true);
        paint.rule(PerformanceLayout::footerTop);
    }
};

}
