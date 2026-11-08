#include "ui/ExpressionPad.hpp"
#include "dsp/Engine.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void render(strings::Engine& engine, int frames) {
    std::array<float, 256> left {}, right {};
    while (frames > 0) {
        const int count = std::min(frames, 256);
        engine.render(left.data(), right.data(), count);
        frames -= count;
    }
}

int main() {
    try {
        using strings::ui::ExpressionPad;
        strings::ui::ExpressionPadLayout layout;
        const auto reverse = layout.at(58, 216);
        const auto forward = layout.at(648, 326);
        const auto center = layout.at(353, 271);
        require(reverse.motion == -1 && reverse.weight == 1, "Upper-left mapping is wrong");
        require(forward.motion == 1 && forward.weight == 0, "Lower-right mapping is wrong");
        require(center.motion == 0 && center.weight == 0.5f, "Center mapping is wrong");
        require(layout.at(354, 271).motion == 0, "Rest deadzone missing");
        require(layout.contains(58, 216) && !layout.contains(57, 216), "Hit region is inconsistent");
        ExpressionPad pad;
        require(!pad.ready() && !pad.dragging() && pad.size() == 0, "Pad invents initial engine state");
        require(!pad.move(forward), "Unarmed motion changed the pad");
        const float invalid = std::numeric_limits<float>::quiet_NaN();
        require(!pad.begin({invalid, 1, 1}), "Nonfinite pointer accepted");
        require(pad.begin({2, -1, 1}), "Pointer begin failed");
        require(pad.requested().motion == 1 && pad.requested().weight == 0, "Pointer bounds failed");
        require(pad.move(reverse), "Pointer motion failed");
        require(!pad.ready(), "Pointer target masquerades as engine readback");
        pad.observe({0.2f, 0.4f, 1}, 1, true);
        require(pad.observed().motion == 0.2f && pad.requested().motion == -1, "Target and engine were conflated");
        require(pad.size() == 1 && pad.active(), "Real readback was not sampled");
        pad.observe({0.3f, 0.5f, 1}, 1.001, true);
        require(pad.size() == 1, "Trail sample rate is not bounded");
        for (int index = 1; index <= 1000; ++index) {
            pad.observe({0.5f, 0.5f, 1}, 1 + index * 0.04, true);
            require(pad.size() <= ExpressionPad::capacity, "Trail exceeds fixed storage");
        }
        require(pad.size() > 10 && pad.sample(pad.size() - 1).time - pad.sample(0).time <= 2.5, "Trail window is incorrect");
        pad.observe({0, 0.5f, 0}, 44, false);
        require(pad.size() == 0 && !pad.active(), "Idle trail did not expire");
        pad.observe({0.5f, 0.5f, 1}, 45, true);
        pad.observe({-0.5f, 0.5f, 1}, 2, true);
        require(pad.size() == 1, "Clock reset retained stale trail");
        require(!pad.move({invalid, 1, 1}) && !pad.dragging(), "Invalid input did not cancel gesture");
        require(pad.requested().motion == 0 && pad.requested().gate == 0, "Release left excitation engaged");
        for (const double rate : {44100.0, 48000.0, 96000.0}) {
            strings::Engine engine;
            engine.prepare(rate);
            engine.setParameter(strings::BowPressure, 0.8f);
            engine.setParameter(strings::BowSpeed, 0.7f);
            engine.setParameter(strings::BowDirection, -1);
            engine.setParameter(strings::BowEngaged, 1);
            const auto before = engine.getParameter(strings::BowWeightMonitor);
            require(before != 0.8f, "Readback bypasses engine smoothing");
            render(engine, static_cast<int>(rate * 0.25));
            require(std::abs(engine.getParameter(strings::BowWeightMonitor) - 0.8f) < 0.001f, "Weight monitor is incorrect");
            require(std::abs(engine.getParameter(strings::BowMotionMonitor) + 0.7f) < 0.001f, "Signed motion monitor is incorrect");
            for (const auto parameter : {strings::BowMotionMonitor, strings::BowWeightMonitor, strings::BowGateMonitor}) {
                const auto value = engine.getParameter(parameter);
                require(!strings::isInputParameter(parameter), "Monitor exposed as automatable input");
                engine.setParameter(parameter, value - 0.2f);
                require(engine.getParameter(parameter) == value, "Monitor accepted a write");
            }
            const uint8_t centerDirection[] = {0xb0, 19, 64};
            engine.midi(centerDirection, 3);
            render(engine, static_cast<int>(rate * 0.25));
            require(std::abs(engine.getParameter(strings::BowMotionMonitor)) < 0.001f, "MIDI does not reach engine monitor");
            engine.setParameter(strings::BowEngaged, 0);
            render(engine, static_cast<int>(rate * 0.25));
            require(engine.getParameter(strings::BowGateMonitor) < 0.001f, "Lift monitor did not decay");
        }
        std::cout << "PASS: mapping, bounds, target/readback separation, bounded trails, cancel/lift and three-rate engine/MIDI telemetry\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
