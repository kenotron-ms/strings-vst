#include "ui/PerformanceSession.hpp"
#include "ui/PerformanceKnob.hpp"
#include "dsp/Engine.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>

void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

int main() {
    try {
        using strings::ui::ExpressionTake;
        ExpressionTake take;
        require(take.size() == 0 && take.at(0).gate == 0, "Empty take produces excitation");
        require(take.begin({0, 0.5f, 0}), "Recording did not begin");
        require(!take.begin({1, 1, 1}), "Nested recording destroyed take");
        require(take.capture(1, {1, 0.8f, 1}), "Capture failed");
        require(take.capture(2, {-1, 0.2f, 1}), "Reverse capture failed");
        require(take.capture(2.001, {0, 0.2f, 0}), "Fast gate edge was lost");
        require(!take.capture(1.9, {1, 1, 1}), "Backwards time accepted");
        require(!take.capture(9, {1, 1, 1}), "Capture outside phrase accepted");
        require(!take.capture(3, {std::numeric_limits<float>::quiet_NaN(), 0, 0}), "Nonfinite capture accepted");
        take.finish();
        require(std::abs(take.at(1.5).motion) < 1e-5 && std::abs(take.at(1.5).weight - 0.5f) < 1e-5, "Interpolation incorrect");
        require(take.at(8).gate == 0 && take.at(0.5).gate == 0, "Gate was interpolated or stuck on");
        require(take.begin({1, 1, 1}), "Second take failed");
        for (int index = 1; index <= 8000; ++index) take.capture(index * 0.001, {0.5f, 0.7f, 1});
        require(take.size() > 300 && take.size() <= ExpressionTake::capacity, "Storage/rate limit incorrect");
        take.finish(); take.undo(); require(take.size() == 4, "Undo lost first take");
        take.undo(); require(take.size() > 300, "Redo swap failed");
        strings::ui::PerformanceLayout layout;
        require(layout.pad.contains(550, 260) && !layout.pad.contains(550, 500), "Performance hit region wrong");
        strings::ui::PhraseEditor phrase;
        const auto first = phrase.notes()[0];
        require(phrase.activeNote(0) == 0 && phrase.activeNote(0.9) == 1, "Overlapping phrase does not prefer latest note");
        require(phrase.activeNote(8) == -1 && phrase.activeNote(std::numeric_limits<double>::quiet_NaN()) == -1, "Invalid phrase time sounds");
        require(!phrase.begin(8) && !phrase.set(8, first), "Invalid phrase index accepted");
        require(phrase.begin(0) && phrase.set(0, {7, 3.0, 0.75}), "Note edit failed");
        require(phrase.activeNote(3.2) == 0, "Edited phrase follows array order instead of time");
        require(!phrase.set(0, {7, std::numeric_limits<double>::quiet_NaN(), 1}), "Nonfinite note time accepted");
        require(phrase.undo() && phrase.notes()[0].start == first.start && phrase.notes()[0].semitone == first.semitone, "Note undo failed");
        require(phrase.set(0, {-20, 9, -3}) && phrase.notes()[0].start == 7.75
            && phrase.notes()[0].length == 0.25 && phrase.notes()[0].semitone == 0, "Phrase bounds failed");
        for (bool expanded : {false, true}) {
            const strings::ui::PhraseRollLayout roll(expanded);
            require(roll.noteAt(phrase, roll.horizontal(7.8), roll.noteTop(0) + 1) == 0, "Piano-roll hit test failed");
            require(roll.noteAt(phrase, 0, 0) == -1 && roll.onCurve(500, roll.curveTop + 10), "Phrase editing regions overlap");
            require(roll.timeAt(-200) == 0 && roll.timeAt(2000) == 8, "Curve time is unbounded");
        }
        ExpressionTake edited;
        require(edited.beginEdit({-0.6f, 0.5f, 1}) && edited.size() == 33, "Empty curve did not initialize");
        require(edited.editWeight(4, 0.9f) && std::abs(edited.at(4).weight - 0.9f) < 1e-6f
            && edited.at(4).motion == -0.6f && edited.at(4).gate == 1, "Weight edit changes motion or lift");
        require(!edited.editWeight(-1, 0) && !edited.editWeight(2, std::numeric_limits<float>::infinity()), "Invalid curve edit accepted");
        require(edited.undo() && edited.size() == 0, "Curve undo did not restore empty original");
        require(edited.begin({-1, 0.4f, 0}), "Lifted take capture failed"); edited.capture(1, {1, 0.7f, 1}); edited.finish();
        require(edited.beginEdit({1, 1, 1}) && edited.editWeight(0, 1) && edited.at(0).gate == 0, "Editing weight lowered a lifted bow");
        for (float value : {-2.0f, 0.0f, 0.3f, 1.0f, 5.0f, std::numeric_limits<float>::quiet_NaN()}) {
            const auto point = strings::ui::KeyboardExpression::energy(value);
            require(std::isfinite(point.weight) && point.weight >= 0 && point.weight <= 1
                && point.motion >= 0.15f && point.motion <= 0.85f, "Keyboard recipe is unbounded");
            require(strings::ui::KeyboardExpression::contact(value) >= 0
                && strings::ui::KeyboardExpression::contact(value) <= 1, "Tone contact is unbounded");
        }
        double previous = 0;
        for (const auto& knob : strings::ui::performanceKnobs) {
            require(knob.contains(knob.horizontal, knob.vertical) && !knob.contains(knob.horizontal + 70, knob.vertical), "Knob hit region incorrect");
            require(!knob.contains(std::numeric_limits<float>::quiet_NaN(), knob.vertical), "Nonfinite knob position accepted");
        }
        using strings::ui::PerformanceKnob;
        require(PerformanceKnob::drag(0.5f, 80, false) == 1 && PerformanceKnob::drag(0.5f, -1000, false) == 0, "Knob drag is unbounded");
        require(std::abs(PerformanceKnob::drag(0.5f, 80, true) - 0.55f) < 1e-6f, "Fine knob drag incorrect");
        require(PerformanceKnob::drag(0.5f, std::numeric_limits<float>::infinity(), false) == 0, "Nonfinite knob motion accepted");
        for (const double rate : {44100.0, 48000.0, 96000.0}) {
            strings::Engine engine; engine.prepare(rate); engine.setParameter(strings::BowPressure, 1);
            std::array<float, 64> left {}, right {};
            int remaining = static_cast<int>(rate * 0.013);
            while (remaining) { const int count = std::min(remaining, 64); engine.render(left.data(), right.data(), count); remaining -= count; }
            const double weight = engine.getParameter(strings::BowWeightMonitor);
            require(weight > 0.80 && weight < 0.83, "13ms physical-control response incorrect");
            if (previous) require(std::abs(previous - weight) < 0.009, "Sample rate changes gesture response time");
            previous = weight;
            for (const float target : {0.15f, 0.5f, 0.713f}) {
                engine.setParameter(strings::BowPressure, target);
                int settling = static_cast<int>(rate * 0.5);
                while (settling) { const int count = std::min(settling, 64); engine.render(left.data(), right.data(), count); settling -= count; }
                require(engine.getParameter(strings::BowWeightMonitor) == target, "Smoothing stalls short of its exact target");
            }
        }
        std::cout << "PASS: bounded takes, curve/note editing, undo, pitch/time hit testing, keyboard recipes and rate-independent smoothing\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
