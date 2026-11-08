#pragma once
#include "ExpressionPad.hpp"
#include <array>
#include <cmath>

namespace strings::ui {

struct PhraseNote { int semitone; double start, length; };
inline constexpr std::array<PhraseNote, 8> auditionPhrase {{{0, 0, 0.95}, {2, 0.85, 0.95},
    {4, 1.7, 1.05}, {7, 2.65, 1.0}, {5, 3.55, 1.05}, {4, 4.5, 1.05},
    {2, 5.45, 1.05}, {0, 6.4, 1.4}}};
inline constexpr double phraseDuration = 8;

struct KeyboardExpression {
    static ExpressionPoint energy(float value) {
        const float bounded = std::isfinite(value) ? std::clamp(value, 0.0f, 1.0f) : 0;
        return {0.15f + 0.7f * bounded, bounded, 1};
    }
    static float contact(float tone) { return 1 - std::clamp(std::isfinite(tone) ? tone : 0.5f, 0.0f, 1.0f); }
    static float brightness(float tone) { return 0.25f + 0.65f * (1 - contact(tone)); }
};

class PhraseEditor {
public:
    const auto& notes() const { return current; }
    bool begin(size_t index) {
        if (index >= current.size()) return false;
        previous = current; undoAvailable = true; return true;
    }
    bool set(size_t index, PhraseNote note) {
        if (index >= current.size() || !std::isfinite(note.start) || !std::isfinite(note.length)) return false;
        note.start = std::clamp(note.start, 0.0, phraseDuration - 0.25);
        note.length = std::clamp(note.length, 0.25, phraseDuration - note.start);
        note.semitone = std::clamp(note.semitone, 0, 12);
        current[index] = note; return true;
    }
    int activeNote(double time) const {
        int selected = -1;
        if (!std::isfinite(time)) return selected;
        for (size_t index = 0; index < current.size(); ++index) {
            const auto& note = current[index];
            if (time >= note.start && time < note.start + note.length
                && (selected < 0 || note.start >= current[static_cast<size_t>(selected)].start)) selected = static_cast<int>(index);
        }
        return selected;
    }
    bool canUndo() const { return undoAvailable; }
    bool undo() { if (!undoAvailable) return false; current.swap(previous); return true; }
private:
    std::array<PhraseNote, 8> current = auditionPhrase, previous = auditionPhrase;
    bool undoAvailable = false;
};

struct PhraseRollLayout {
    explicit PhraseRollLayout(bool expanded) : top(expanded ? 370.0f : 518.0f),
        notesTop(expanded ? 407.0f : 543.0f), rowHeight(expanded ? 10.0f : 3.8f),
        curveTop(expanded ? 568.0f : 603.0f), curveHeight(expanded ? 53.0f : 30.0f) {}
    float top, notesTop, rowHeight, curveTop, curveHeight;
    static constexpr float left = 182, width = 824;
    float horizontal(double time) const { return left + static_cast<float>(time / phraseDuration) * width; }
    float noteTop(int semitone) const { return notesTop + (12 - semitone) * rowHeight; }
    bool onCurve(float horizontalPosition, float verticalPosition) const {
        return horizontalPosition >= left && horizontalPosition <= left + width
            && verticalPosition >= curveTop && verticalPosition <= curveTop + curveHeight;
    }
    double timeAt(float position) const { return std::clamp(static_cast<double>((position - left) / width) * phraseDuration, 0.0, phraseDuration); }
    int noteAt(const PhraseEditor& phrase, float horizontalPosition, float verticalPosition) const {
        for (size_t index = 0; index < phrase.notes().size(); ++index) {
            const auto& note = phrase.notes()[index];
            if (horizontalPosition >= horizontal(note.start) && horizontalPosition <= horizontal(note.start + note.length)
                && verticalPosition >= noteTop(note.semitone) && verticalPosition <= noteTop(note.semitone) + rowHeight)
                return static_cast<int>(index);
        }
        return -1;
    }
};

class ExpressionTake {
public:
    struct Frame { double time; ExpressionPoint point; };
    static constexpr size_t capacity = 512;
    bool begin(ExpressionPoint initial) {
        if (recording) return false;
        previous = frames; previousCount = count;
        count = 0; recording = true;
        capture(0, initial);
        return true;
    }
    bool capture(double time, ExpressionPoint point) {
        if (!recording || !std::isfinite(time) || time < 0 || time > phraseDuration
            || !std::isfinite(point.motion) || !std::isfinite(point.weight) || !std::isfinite(point.gate)) return false;
        if (count && time < frames[count - 1].time) return false;
        const Frame frame {time, {std::clamp(point.motion, -1.0f, 1.0f), std::clamp(point.weight, 0.0f, 1.0f),
            std::clamp(point.gate, 0.0f, 1.0f)}};
        if (count && time == frames[count - 1].time) { frames[count - 1] = frame; return true; }
        if (count && time - frames[count - 1].time < 1.0 / 60.0
            && frame.point.gate == frames[count - 1].point.gate) return false;
        if (count == capacity) return false;
        frames[count++] = frame;
        return true;
    }
    void finish() { recording = false; }
    bool undo() {
        if (recording) return false;
        frames.swap(previous); std::swap(count, previousCount);
        return true;
    }
    bool isRecording() const { return recording; }
    bool beginEdit(ExpressionPoint baseline) {
        if (recording || !std::isfinite(baseline.motion) || !std::isfinite(baseline.weight) || !std::isfinite(baseline.gate)) return false;
        previous = frames; previousCount = count;
        if (count < 2) {
            count = 33;
            for (size_t index = 0; index < count; ++index)
                frames[index] = {phraseDuration * index / (count - 1), {std::clamp(baseline.motion, -1.0f, 1.0f),
                    std::clamp(baseline.weight, 0.0f, 1.0f), std::clamp(baseline.gate, 0.0f, 1.0f)}};
        }
        return true;
    }
    bool editWeight(double time, float weight) {
        if (recording || !count || !std::isfinite(time) || !std::isfinite(weight) || time < 0 || time > phraseDuration) return false;
        size_t closest = 0;
        for (size_t index = 1; index < count; ++index)
            if (std::abs(frames[index].time - time) < std::abs(frames[closest].time - time)) closest = index;
        frames[closest].point.weight = std::clamp(weight, 0.0f, 1.0f);
        return true;
    }
    size_t size() const { return count; }
    const Frame& frame(size_t index) const { return frames[index]; }
    ExpressionPoint at(double time) const {
        if (!count || !std::isfinite(time)) return {};
        if (time <= frames[0].time) return frames[0].point;
        for (size_t index = 1; index < count; ++index) if (time <= frames[index].time) {
            const auto& before = frames[index - 1];
            const auto& after = frames[index];
            const float blend = static_cast<float>((time - before.time) / (after.time - before.time));
            return {before.point.motion + blend * (after.point.motion - before.point.motion),
                before.point.weight + blend * (after.point.weight - before.point.weight), before.point.gate};
        }
        return frames[count - 1].point;
    }
private:
    std::array<Frame, capacity> frames {}, previous {};
    size_t count = 0, previousCount = 0;
    bool recording = false;
};

struct PerformanceLayout {
    ExpressionPadLayout pad {28, 120, 650, 358};
    static constexpr float ribbonTop = 518, takeTop = 585, footerTop = 650;
};

}
