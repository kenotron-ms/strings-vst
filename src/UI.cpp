#include "DistrhoUI.hpp"
#include "Parameters.hpp"
#include "Presets.hpp"
#include "Font.hpp"
#include "ui/ExpressionPadView.hpp"
#include "ui/PerformanceViews.hpp"
#include "ui/PlayableKeyboardView.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>

START_NAMESPACE_DISTRHO

class StringsUI final : public UI {
public:
    StringsUI() : UI(1040, 680) {
        for (uint32_t index = 0; index < strings::ParameterCount; ++index) values[index] = strings::parameterSpecs[index].initial;
        loadSharedResources();
        createFontFromMemory("instrument", titleFont, sizeof(titleFont), false);
        setGeometryConstraints(832, 544, true);
    }
    ~StringsUI() override { releaseKeys(); finishEdits(); follow(); }
protected:
    void parameterChanged(uint32_t index, float value) override {
        if (index == strings::Instrument && value != values[index]) { releaseKeys(); octaveOffset = 0; }
        if (index < values.size()) values[index] = strings::sanitizeParameter(index, value);
        if (index == strings::BowMotionMonitor) readbackMask |= 1;
        if (index == strings::BowWeightMonitor) readbackMask |= 2;
        if (index == strings::BowGateMonitor) readbackMask |= 4;
        repaint();
    }
    void programLoaded(uint32_t index) override {
        releaseKeys(); finishEdits();
        if (expressionPad.dragging()) {
            expressionPad.release();
            for (const auto parameter : bowParameters) editParameter(parameter, false);
        }
        gestureMode = false; expressionPad.clearTrail(); presetIndex = index; octaveOffset = 0; repaint();
    }
    void uiIdle() override {
        if (readbackMask == 7) expressionPad.observe({values[strings::BowMotionMonitor], values[strings::BowWeightMonitor],
            values[strings::BowGateMonitor]}, clockSeconds(), values[strings::ActivePlayers] > 0);
        if (expressionPad.size() || expressionPad.dragging() || values[strings::Meter] > 0.0001f) repaint();
    }
    void uiFocus(bool focus, DGL_NAMESPACE::CrossingMode) override {
        if (!focus) { releaseBow(); releaseKeys(); finishEdits(); }
    }
    void onNanoDisplay() override {
        scale(getWidth() / 1040.0f, getHeight() / 680.0f);
        strings::ui::PerformancePainter paint(*this);
        paint.box(0, 0, 1040, 680, paint.paper);
        fontFace("instrument"); fontSize(35); fillColor(paint.pine); textAlign(ALIGN_LEFT | ALIGN_BASELINE); text(28, 47, "Strings", nullptr);
        paint.button(177, 22, 140, strings::instrumentNames[static_cast<int>(values[strings::Instrument])]);
        paint.button(327, 22, 93, values[strings::Ensemble] > 0.5f ? "Ensemble" : "Solo");
        for (int index = 0; index < 3; ++index)
            paint.button(433 + index * 122.0f, 22, 112, strings::articulationNames[index], static_cast<int>(values[strings::Articulation]) == index);
        paint.label(28, 89, strings::presetNames[presetIndex % strings::presetCount], 11, true);
        paint.button(245, 67, 32, "<"); paint.button(284, 67, 32, ">");
        paint.button(715, 67, 93, "Knobs", !bowView && !details);
        paint.button(815, 67, 93, "Bow", bowView && !details);
        paint.button(918, 67, 94, "Details", details);
        char caption[32];
        std::snprintf(caption, sizeof(caption), "Output %+.1f dB", values[strings::Gain]);
        paint.label(847, 34, caption, 11, true);
        paint.box(847, 45, 165, 3, paint.sage, 1);
        paint.box(847, 45, 165 * std::clamp(values[strings::Meter] * 2, 0.0f, 1.0f), 3, paint.brass, 1);
        paint.rule(110);
        if (details) drawDetails();
        else if (bowView) expressionView.draw(*this, expressionPad, padLayout, clockSeconds(), gestureMode, NANOVG_DEJAVU_SANS_TTF);
        else keyboardView.draw(*this, values[strings::BowPressure], values[strings::BowPosition], values[strings::Vibrato]);
        playableView.draw(*this, playableKeyboard, typingBase(), pointerNote, keyboardVelocity);
    }
    bool onMouse(const MouseEvent& event) override {
        if (event.button != 1) return false;
        const float horizontal = event.pos.getX() * 1040.0f / getWidth();
        const float vertical = event.pos.getY() * 680.0f / getHeight();
        if (!event.press) { releasePointer(); releaseBow(); finishEdits(); return true; }
        if (hit(horizontal, vertical, 335, 518, 34)) { shiftOctave(-1); return true; }
        if (hit(horizontal, vertical, 545, 518, 34)) { shiftOctave(1); return true; }
        if (hit(horizontal, vertical, 748, 518, 34)) { keyboardVelocity = std::max(16, keyboardVelocity - 8); repaint(); return true; }
        if (hit(horizontal, vertical, 790, 518, 34)) { keyboardVelocity = std::min(127, keyboardVelocity + 8); repaint(); return true; }
        if (hit(horizontal, vertical, 858, 518, 154)) { releaseKeys(); repaint(); return true; }
        const int key = strings::ui::PlayableKeyboard::noteAt(horizontal, vertical);
        if (key >= 0) { if (!bowView) follow(); pointerHeld = true; movePointer(key); return true; }
        if (hit(horizontal, vertical, 177, 22, 140)) {
            releaseKeys(); finishEdits(); follow(); octaveOffset = 0;
            change(strings::Instrument, static_cast<float>((static_cast<int>(values[strings::Instrument]) + 1) % 4)); return true;
        }
        if (hit(horizontal, vertical, 327, 22, 93)) { change(strings::Ensemble, values[strings::Ensemble] > 0.5f ? 0 : 1); return true; }
        for (int index = 0; index < 3; ++index) if (hit(horizontal, vertical, 433 + index * 122.0f, 22, 112)) {
            change(strings::Articulation, static_cast<float>(index)); return true;
        }
        if (hit(horizontal, vertical, 245, 67, 32)) { choosePreset(-1); return true; }
        if (hit(horizontal, vertical, 284, 67, 32)) { choosePreset(1); return true; }
        if (hit(horizontal, vertical, 715, 67, 93) || hit(horizontal, vertical, 815, 67, 93)) {
            finishEdits(); follow(); bowView = horizontal >= 815; details = false; repaint(); return true;
        }
        if (hit(horizontal, vertical, 918, 67, 94)) {
            finishEdits(); follow(); details = !details; repaint(); return true;
        }
        if (details) {
            for (const auto& knob : knobs) if (nearKnob(knob, horizontal, vertical)) {
                if (event.time - lastClick < 350 && lastClicked == static_cast<int>(knob.parameter))
                    change(knob.parameter, strings::parameterSpecs[knob.parameter].initial);
                else {
                    dragParameter = static_cast<int>(knob.parameter); dragStart = values[knob.parameter]; dragY = vertical;
                    editParameter(knob.parameter, true);
                }
                lastClick = event.time; lastClicked = static_cast<int>(knob.parameter); return true;
            }
            return false;
        }
        if (!bowView) {
            for (size_t index = 0; index < strings::ui::performanceKnobs.size(); ++index) if (strings::ui::performanceKnobs[index].contains(horizontal, vertical)) {
                follow(); finishEdits();
                if (event.time - lastMacroClick < 350 && lastMacroIndex == static_cast<int>(index)) {
                    for (size_t offset = index * 2; offset < std::min(index * 2 + 2, macroParameters.size()); ++offset)
                        change(macroParameters[offset], strings::parameterSpecs[macroParameters[offset]].initial);
                } else {
                    macroDrag = static_cast<int>(index); macroStart = macroValue(index); macroY = vertical; editMacro(index, true);
                }
                lastMacroClick = event.time; lastMacroIndex = static_cast<int>(index); return true;
            }
        } else if (padLayout.contains(horizontal, vertical)) {
            if (!gestureMode) {
                for (size_t index = 0; index < bowParameters.size(); ++index) bowBaseline[index] = values[bowParameters[index]];
                gestureMode = true;
            }
            if (expressionPad.begin(padLayout.at(horizontal, vertical))) {
                for (const auto parameter : bowParameters) editParameter(parameter, true);
                writeBowPoint(expressionPad.requested());
            }
            return true;
        }
        return false;
    }
    bool onMotion(const MotionEvent& event) override {
        const float horizontal = event.pos.getX() * 1040.0f / getWidth(), vertical = event.pos.getY() * 680.0f / getHeight();
        if (pointerHeld) { movePointer(strings::ui::PlayableKeyboard::noteAt(horizontal, vertical)); return true; }
        if (macroDrag >= 0) { writeMacro(static_cast<size_t>(macroDrag), strings::ui::PerformanceKnob::drag(macroStart, macroY - vertical, event.mod & kModifierShift)); return true; }
        if (expressionPad.dragging()) {
            if (!padLayout.contains(horizontal, vertical)) releaseBow();
            else if (expressionPad.move(padLayout.at(horizontal, vertical))) writeBowPoint(expressionPad.requested());
            return true;
        }
        if (dragParameter < 0) return false;
        const auto parameter = static_cast<uint32_t>(dragParameter);
        const auto& spec = strings::parameterSpecs[parameter];
        const float sensitivity = (event.mod & kModifierShift) ? 0.1f : 1.0f;
        values[parameter] = strings::sanitizeParameter(parameter, dragStart + (dragY - vertical) / 160 * (spec.maximum - spec.minimum) * sensitivity);
        setParameterValue(parameter, values[parameter]); repaint(); return true;
    }
    bool onScroll(const ScrollEvent& event) override {
        const float horizontal = event.pos.getX() * 1040.0f / getWidth(), vertical = event.pos.getY() * 680.0f / getHeight();
        if (!details) {
            if (bowView) return false;
            for (size_t index = 0; index < strings::ui::performanceKnobs.size(); ++index) if (strings::ui::performanceKnobs[index].contains(horizontal, vertical)) {
                follow(); finishEdits(); editMacro(index, true);
                writeMacro(index, macroValue(index) + event.delta.getY() * ((event.mod & kModifierShift) ? 0.002f : 0.02f));
                editMacro(index, false); return true;
            }
            return false;
        }
        for (const auto& knob : knobs) if (nearKnob(knob, horizontal, vertical)) {
            const auto& spec = strings::parameterSpecs[knob.parameter];
            change(knob.parameter, values[knob.parameter] + event.delta.getY() * (spec.maximum - spec.minimum) * 0.02f); return true;
        }
        return false;
    }
    bool onKeyboard(const KeyboardEvent& event) override {
        if (event.key == kKeyEscape && event.press) { releaseBow(); releaseKeys(); finishEdits(); return true; }
        if (event.press && (event.mod & (kModifierControl | kModifierAlt | kModifierSuper))) return false;
        if (event.key == 'z' || event.key == 'Z' || event.key == 'x' || event.key == 'X') {
            if (event.press) shiftOctave(event.key == 'z' || event.key == 'Z' ? -1 : 1);
            return true;
        }
        if (strings::ui::PlayableKeyboard::keyIndex(event.key) < 0) return false;
        if (event.press && !bowView) follow();
        const int note = event.press ? playableKeyboard.press(event.key, typingBase()) : playableKeyboard.release(event.key);
        if (note >= 0) sendNote(13, static_cast<uint8_t>(note), event.press ? static_cast<uint8_t>(keyboardVelocity) : 0);
        repaint(); return true;
    }
private:
    struct Knob { uint32_t parameter; float horizontal, vertical; const char* caption; };
    const std::array<Knob, 16> knobs {{{strings::BowPressure, 110, 205, "Bow weight"}, {strings::BowSpeed, 275, 205, "Bow speed"},
        {strings::BowPosition, 440, 205, "Contact point"}, {strings::Brightness, 605, 205, "Brightness"}, {strings::Vibrato, 770, 205, "Vibrato"},
        {strings::VibratoRate, 935, 205, "Vibrato rate"}, {strings::Attack, 110, 335, "Attack"}, {strings::Release, 275, 335, "Release"},
        {strings::Humanize, 440, 335, "Player variation"}, {strings::Room, 605, 335, "Room"}, {strings::BodyCoupling, 770, 335, "Body coupling"},
        {strings::Sympathetic, 935, 335, "Sympathetic"}, {strings::Rosin, 110, 455, "Rosin grip"}, {strings::Players, 275, 455, "Players"},
        {strings::Gain, 440, 455, "Output"}, {strings::BowDirection, 605, 455, "Direction"}}};
    std::array<float, strings::ParameterCount> values {};
    strings::ui::PlayableKeyboard playableKeyboard;
    strings::ui::PlayableKeyboardView playableView;
    int pointerNote = -1, octaveOffset = 0, keyboardVelocity = 100;
    bool pointerHeld = false;
    const std::array<uint32_t, 4> bowParameters {{strings::BowPressure, strings::BowSpeed, strings::BowDirection, strings::BowEngaged}};
    const std::array<uint32_t, 5> macroParameters {{strings::BowPressure, strings::BowSpeed, strings::BowPosition, strings::Brightness, strings::Vibrato}};
    std::array<float, 4> bowBaseline {};
    uint32_t presetIndex = 0, lastClick = 0, lastMacroClick = 0;
    int dragParameter = -1, lastClicked = -1, macroDrag = -1, lastMacroIndex = -1;
    float macroStart = 0, macroY = 0, dragStart = 0, dragY = 0;
    bool gestureMode = false, details = false, bowView = false;
    unsigned readbackMask = 0;
    strings::ui::ExpressionPad expressionPad;
    strings::ui::ExpressionPadView expressionView;
    strings::ui::ExpressionPadLayout padLayout {28, 120, 984, 358};
    strings::ui::KeyboardPerformanceView keyboardView;
    static double clockSeconds() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
    static bool hit(float horizontal, float vertical, float left, float top, float width) {
        return horizontal >= left && horizontal <= left + width && vertical >= top && vertical <= top + 34;
    }
    static bool nearKnob(const Knob& knob, float horizontal, float vertical) {
        return horizontal >= knob.horizontal - 65 && horizontal <= knob.horizontal + 65
            && vertical >= knob.vertical - 48 && vertical <= knob.vertical + 48;
    }
    void finishEdits() {
        if (macroDrag >= 0) editMacro(static_cast<size_t>(macroDrag), false);
        macroDrag = -1;
        if (dragParameter >= 0) editParameter(static_cast<uint32_t>(dragParameter), false);
        dragParameter = -1;
    }
    float macroValue(size_t index) const { return index == 0 ? values[strings::BowPressure] : index == 1 ? 1 - values[strings::BowPosition] : values[strings::Vibrato]; }
    void editMacro(size_t index, bool editing) {
        for (size_t offset = index * 2; offset < std::min(index * 2 + 2, macroParameters.size()); ++offset)
            editParameter(macroParameters[offset], editing);
    }
    void writeMacro(size_t index, float amount) {
        const float value = std::clamp(amount, 0.0f, 1.0f);
        const auto write = [&](uint32_t parameter, float target) {
            values[parameter] = strings::sanitizeParameter(parameter, target); setParameterValue(parameter, values[parameter]);
        };
        if (index == 0) {
            const auto point = strings::ui::KeyboardExpression::energy(value);
            write(strings::BowPressure, point.weight); write(strings::BowSpeed, point.motion);
        } else if (index == 1) {
            write(strings::BowPosition, strings::ui::KeyboardExpression::contact(value));
            write(strings::Brightness, strings::ui::KeyboardExpression::brightness(value));
        } else write(strings::Vibrato, value);
        repaint();
    }
    void writeBowPoint(strings::ui::ExpressionPoint point) {
        values[strings::BowPressure] = point.weight; values[strings::BowSpeed] = std::abs(point.motion);
        values[strings::BowDirection] = point.motion < 0 ? -1 : point.motion > 0 ? 1 : 0; values[strings::BowEngaged] = point.gate;
        for (const auto parameter : bowParameters) setParameterValue(parameter, values[parameter]);
        repaint();
    }
    void releaseBow() {
        if (!expressionPad.dragging()) return;
        expressionPad.release(); writeBowPoint(expressionPad.requested());
        for (const auto parameter : bowParameters) editParameter(parameter, false);
    }
    void follow() {
        releaseBow();
        if (gestureMode) for (size_t index = 0; index < bowParameters.size(); ++index) change(bowParameters[index], bowBaseline[index]);
        gestureMode = false; repaint();
    }
    void releaseKeys() {
        releasePointer();
        for (auto& note : playableKeyboard.notes) if (note >= 0) { sendNote(13, static_cast<uint8_t>(note), 0); note = -1; }
    }
    void movePointer(int wanted) {
        if (wanted == pointerNote) return;
        if (wanted >= 0) sendNote(14, static_cast<uint8_t>(wanted), static_cast<uint8_t>(keyboardVelocity));
        if (pointerNote >= 0) sendNote(14, static_cast<uint8_t>(pointerNote), 0);
        pointerNote = wanted; repaint();
    }
    void releasePointer() { movePointer(-1); pointerHeld = false; }
    void shiftOctave(int delta) {
        const int next = std::clamp(octaveOffset + delta, -2, 2);
        if (!strings::ui::PlayableKeyboard::typingRangeVisible(keyboardBase() + next * 12)) return;
        releaseKeys(); octaveOffset = next; repaint();
    }
    int keyboardBase() const { constexpr int bases[] = {60, 48, 36, 28}; return bases[static_cast<int>(values[strings::Instrument])]; }
    int typingBase() const { return keyboardBase() + octaveOffset * 12; }
    void change(uint32_t parameter, float value) {
        values[parameter] = strings::sanitizeParameter(parameter, value);
        editParameter(parameter, true); setParameterValue(parameter, values[parameter]); editParameter(parameter, false); repaint();
    }
    void choosePreset(int direction) {
        releaseKeys(); finishEdits(); follow(); octaveOffset = 0;
        presetIndex = static_cast<uint32_t>((static_cast<int>(presetIndex) + direction + strings::presetCount) % strings::presetCount);
        strings::applyPreset(presetIndex, [this](uint32_t parameter, float value) { change(parameter, value); });
    }
    void drawDetails() {
        strings::ui::PerformancePainter paint(*this);
        for (const auto& knob : knobs) {
            const auto& spec = strings::parameterSpecs[knob.parameter];
            const float normalized = (values[knob.parameter] - spec.minimum) / (spec.maximum - spec.minimum);
            constexpr float start = 2.3561945f, sweep = 4.712389f;
            beginPath(); arc(knob.horizontal, knob.vertical, 25, start, start + sweep, CW);
            strokeColor(paint.sage); strokeWidth(3); stroke();
            beginPath(); arc(knob.horizontal, knob.vertical, 25, start, start + normalized * sweep, CW);
            strokeColor(paint.brass); strokeWidth(3); stroke();
            const float angle = start + normalized * sweep;
            beginPath(); moveTo(knob.horizontal + std::cos(angle) * 13, knob.vertical + std::sin(angle) * 13);
            lineTo(knob.horizontal + std::cos(angle) * 20, knob.vertical + std::sin(angle) * 20);
            strokeColor(paint.pine); strokeWidth(2); stroke();
            paint.label(knob.horizontal, knob.vertical - 36, knob.caption, 11, false, true);
            char caption[32];
            if (knob.parameter == strings::Gain) std::snprintf(caption, sizeof(caption), "%+.1f dB", values[knob.parameter]);
            else if (knob.parameter == strings::Players) std::snprintf(caption, sizeof(caption), "%.0f", values[knob.parameter]);
            else if (knob.parameter == strings::Attack || knob.parameter == strings::Release) std::snprintf(caption, sizeof(caption), "%.0f ms", values[knob.parameter]);
            else if (knob.parameter == strings::VibratoRate) std::snprintf(caption, sizeof(caption), "%.1f Hz", values[knob.parameter]);
            else std::snprintf(caption, sizeof(caption), "%.0f%%", values[knob.parameter] * 100);
            paint.label(knob.horizontal, knob.vertical + 44, caption, 11, true, true);
        }
    }
};

UI* createUI() { return new StringsUI; }

END_NAMESPACE_DISTRHO
