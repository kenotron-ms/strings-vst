#include "DistrhoUI.hpp"
#include "Parameters.hpp"
#include "Presets.hpp"
#include "Font.hpp"
#include "ui/ExpressionPadView.hpp"
#include "ui/PerformanceViews.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>

START_NAMESPACE_DISTRHO

class StringsUI final : public UI {
public:
    StringsUI() : UI(1040, 720) {
        for (uint32_t index = 0; index < strings::ParameterCount; ++index) values[index] = strings::parameterSpecs[index].initial;
        loadSharedResources();
        createFontFromMemory("instrument", titleFont, sizeof(titleFont), false);
        setGeometryConstraints(832, 576, true);
    }
    ~StringsUI() override { stopSession(); releaseKeys(); releaseBow(); finishKnob(); finishLocalEdit(); if (gestureMode) restoreBowBaseline(); }
protected:
    void parameterChanged(uint32_t index, float value) override {
        if (index < values.size()) values[index] = strings::sanitizeParameter(index, value);
        if (index == strings::BowMotionMonitor) readbackMask |= 1;
        if (index == strings::BowWeightMonitor) readbackMask |= 2;
        if (index == strings::BowGateMonitor) readbackMask |= 4;
        repaint();
    }
    void programLoaded(uint32_t index) override {
        stopSession(false); releaseKeys(); finishKnob();
        if (expressionPad.dragging()) {
            expressionPad.release();
            for (const auto parameter : bowParameters) editParameter(parameter, false);
        }
        finishLocalEdit(); gestureMode = false; expressionPad.clearTrail(); presetIndex = index; repaint();
    }
    void uiIdle() override {
        const double now = clockSeconds();
        if (playing) {
            elapsed = std::clamp(now - started, 0.0, strings::ui::phraseDuration);
            if (now - lastIdle > 0.3 || elapsed >= strings::ui::phraseDuration) stopSession();
            else {
                int wanted = -1;
                const int active = phrase.activeNote(elapsed);
                if (active >= 0) wanted = keyboardBase() + phrase.notes()[static_cast<size_t>(active)].semitone;
                if (wanted != auditionNote) {
                    if (wanted >= 0) sendNote(15, static_cast<uint8_t>(wanted), 96);
                    if (auditionNote >= 0) sendNote(15, static_cast<uint8_t>(auditionNote), 0);
                    auditionNote = wanted;
                }
                if (replaying) writeBowPoint(take.at(elapsed));
                if (take.isRecording()) take.capture(elapsed, commandedPoint());
            }
        }
        lastIdle = now;
        if (readbackMask == 7) expressionPad.observe({values[strings::BowMotionMonitor], values[strings::BowWeightMonitor],
            values[strings::BowGateMonitor]}, now, values[strings::ActivePlayers] > 0);
        if (playing || expressionPad.size() || expressionPad.dragging() || values[strings::Meter] > 0.0001f) repaint();
    }
    void uiFocus(bool focus, DGL_NAMESPACE::CrossingMode) override {
        if (!focus) { stopSession(); releaseBow(); releaseKeys(); finishKnob(); finishLocalEdit(); }
    }
    void onNanoDisplay() override {
        scale(getWidth() / 1040.0f, getHeight() / 720.0f);
        strings::ui::PerformancePainter paint(*this);
        paint.box(0, 0, 1040, 720, paint.paper);
        fontFace("instrument"); fontSize(35); fillColor(paint.pine); textAlign(ALIGN_LEFT | ALIGN_BASELINE); text(28, 47, "Strings", nullptr);
        paint.button(177, 22, 140, strings::instrumentNames[static_cast<int>(values[strings::Instrument])]);
        paint.button(327, 22, 93, values[strings::Ensemble] > 0.5f ? "Ensemble" : "Solo");
        paint.button(433, 22, 114, playing ? "Playing phrase" : "Audition", playing);
        paint.button(556, 22, 59, "Stop");
        paint.button(625, 22, 141, inputRoute == 2 ? "Record MIDI in DAW" : take.isRecording() ? "Finish recording" : "Record expression", take.isRecording(), inputRoute != 2);
        paint.button(778, 22, 80, "Original", !useTake);
        paint.button(866, 22, 67, "Take", useTake, take.size() > 1);
        paint.button(944, 22, 68, "Undo", false, (noteUndo ? phrase.canUndo() : take.size() > 0) && !playing);
        paint.label(28, 89, strings::presetNames[presetIndex % strings::presetCount], 11, true);
        paint.button(245, 67, 32, "<"); paint.button(284, 67, 32, ">");
        paint.label(338, 88, "Pitch: DAW MIDI or A S D F G H J K", 11, true);
        char caption[96];
        std::snprintf(caption, sizeof(caption), "Output %+.1f dB", values[strings::Gain]);
        paint.label(847, 83, caption, 11, true);
        paint.box(847, 94, 165, 3, paint.sage, 1);
        paint.box(847, 94, 165 * std::clamp(values[strings::Meter] * 2, 0.0f, 1.0f), 3, paint.brass, 1);
        paint.rule(110);
        if (inputRoute == 3) keyboardView.draw(*this, values[strings::BowPressure], values[strings::BowSpeed],
            values[strings::BowPosition], values[strings::Vibrato], values[strings::ActivePlayers] > 0);
        else {
            expressionView.draw(*this, expressionPad, layout.pad, clockSeconds(), gestureMode || replaying, NANOVG_DEJAVU_SANS_TTF);
            paint.label(58, 497, replaying ? "Replaying take / grab the surface to override" : "Drag to shape. Center rests; lift to release.", 11, true);
        }
        inputView.draw(*this, inputRoute, values[strings::BowPosition]);
        paint.button(715, 124, 69, "Keys", inputRoute == 3);
        paint.button(791, 124, 69, "Mouse", inputRoute == 0);
        paint.button(867, 124, 69, "Touch", inputRoute == 1);
        paint.button(943, 124, 69, "Gamepad", inputRoute == 2);
        phraseView.draw(*this, elapsed, playing, take, phrase, selectedNote, keyboardBase(), inputRoute == 3, values[strings::BowPressure]);
        paint.label(28, 680, replaying ? "Expression: local take" : inputRoute == 3 ? "Expression: keys + physical controls" : inputRoute == 2 ? "Expression: MIDI CC16-20 adapter" : "Expression: mouse / touchpad", 11, true);
        paint.button(383, 662, 122, "Release gesture");
        paint.button(518, 662, 135, "Follow controls", !gestureMode && !replaying);
        paint.button(863, 662, 149, details ? "Close details" : "Instrument details", details);
        paint.label(28, 708, "Research physical model / synthetic body profiles", 10, true);
        paint.label(690, 708, "Local audition/take, not host transport", 10, true);
        if (details) drawDetails();
    }
    bool onMouse(const MouseEvent& event) override {
        if (event.button != 1) return false;
        const float horizontal = event.pos.getX() * 1040.0f / getWidth();
        const float vertical = event.pos.getY() * 720.0f / getHeight();
        if (!event.press) {
            releaseBow();
            finishKnob();
            finishLocalEdit();
            return true;
        }
        if (hit(horizontal, vertical, 863, 662, 149)) { releaseBow(); details = !details; repaint(); return true; }
        if (details && vertical >= 119 && vertical <= 645) {
            for (const auto& knob : knobs) if (nearKnob(knob, horizontal, vertical)) {
                if (event.time - lastClick < 350 && lastClicked == static_cast<int>(knob.parameter))
                    change(knob.parameter, strings::parameterSpecs[knob.parameter].initial);
                else {
                    dragParameter = static_cast<int>(knob.parameter); dragStart = values[knob.parameter]; dragY = vertical;
                    editParameter(knob.parameter, true);
                }
                lastClick = event.time; lastClicked = static_cast<int>(knob.parameter); return true;
            }
            for (int index = 0; index < 3; ++index) if (hit(horizontal, vertical, 58 + index * 117.0f, 560, 107)) {
                change(strings::Articulation, static_cast<float>(index)); return true;
            }
            return true;
        }
        if (hit(horizontal, vertical, 177, 22, 140)) { stopSession(); releaseKeys(); follow(); change(strings::Instrument, static_cast<float>((static_cast<int>(values[strings::Instrument]) + 1) % 4)); return true; }
        if (hit(horizontal, vertical, 327, 22, 93)) { change(strings::Ensemble, values[strings::Ensemble] > 0.5f ? 0 : 1); return true; }
        if (hit(horizontal, vertical, 433, 22, 114)) { startSession(false); return true; }
        if (hit(horizontal, vertical, 556, 22, 59) || hit(horizontal, vertical, 383, 662, 122)) { stopSession(); releaseBow(); releaseKeys(); return true; }
        if (hit(horizontal, vertical, 625, 22, 141)) { if (inputRoute != 2) { if (take.isRecording()) stopSession(); else startSession(true); } return true; }
        if (hit(horizontal, vertical, 778, 22, 80)) { stopSession(); useTake = false; repaint(); return true; }
        if (hit(horizontal, vertical, 866, 22, 67) && take.size() > 1) { stopSession(); useTake = true; repaint(); return true; }
        if (hit(horizontal, vertical, 944, 22, 68) && !playing) {
            if (noteUndo) phrase.undo(); else take.undo();
            if (take.size() < 2) useTake = false; repaint(); return true;
        }
        if (hit(horizontal, vertical, 245, 67, 32)) { choosePreset(-1); return true; }
        if (hit(horizontal, vertical, 284, 67, 32)) { choosePreset(1); return true; }
        constexpr int routes[] = {3, 0, 1, 2};
        for (int index = 0; index < 4; ++index) if (hit(horizontal, vertical, 715 + index * 76.0f, 124, 69)) {
            stopSession(); follow(); finishLocalEdit(); inputRoute = routes[index]; repaint(); return true;
        }
        if (hit(horizontal, vertical, 518, 662, 135)) { stopSession(); follow(); return true; }
        if (inputRoute == 3) {
            for (size_t index = 0; index < strings::ui::performanceKnobs.size(); ++index) if (strings::ui::performanceKnobs[index].contains(horizontal, vertical)) {
                if (replaying) { endReplay(); useTake = false; }
                follow(); finishLocalEdit();
                if (event.time - lastMacroClick < 350 && lastMacroIndex == static_cast<int>(index)) {
                    for (size_t offset = index * 2; offset < std::min(index * 2 + 2, macroParameters.size()); ++offset)
                        change(macroParameters[offset], strings::parameterSpecs[macroParameters[offset]].initial);
                } else {
                    macroDrag = static_cast<int>(index); macroStart = macroValue(index); macroY = vertical; editMacro(index, true);
                }
                lastMacroClick = event.time; lastMacroIndex = static_cast<int>(index); return true;
            }
        }
        const strings::ui::PhraseRollLayout area(inputRoute == 3);
        const int selected = area.noteAt(phrase, horizontal, vertical);
        if (selected >= 0) {
            stopSession(); phrase.begin(static_cast<size_t>(selected)); selectedNote = noteDrag = selected;
            originalNote = phrase.notes()[static_cast<size_t>(selected)]; noteResize = horizontal >= area.horizontal(originalNote.start + originalNote.length) - 8;
            dragNoteX = horizontal; dragNoteY = vertical; noteUndo = true; repaint(); return true;
        }
        if (area.onCurve(horizontal, vertical)) {
            stopSession(); follow();
            if (take.beginEdit(commandedPoint())) { curveDrag = true; useTake = true; noteUndo = false; writeCurve(horizontal, vertical); }
            return true;
        }
        if (layout.pad.contains(horizontal, vertical) && (inputRoute == 0 || inputRoute == 1)) {
            if (replaying) { endReplay(); useTake = false; }
            if (!gestureMode) {
                for (size_t index = 0; index < bowParameters.size(); ++index) bowBaseline[index] = values[bowParameters[index]];
                gestureMode = true;
            }
            if (expressionPad.begin(layout.pad.at(horizontal, vertical))) {
                for (const auto parameter : bowParameters) editParameter(parameter, true);
                writeBowPoint(expressionPad.requested());
            }
            return true;
        }
        return false;
    }
    bool onMotion(const MotionEvent& event) override {
        const float horizontal = event.pos.getX() * 1040.0f / getWidth();
        const float vertical = event.pos.getY() * 720.0f / getHeight();
        if (macroDrag >= 0) { writeMacro(static_cast<size_t>(macroDrag), strings::ui::PerformanceKnob::drag(macroStart, macroY - vertical, event.mod & kModifierShift)); return true; }
        if (curveDrag) { writeCurve(horizontal, vertical); return true; }
        if (noteDrag >= 0) {
            const strings::ui::PhraseRollLayout area(inputRoute == 3);
            auto note = originalNote;
            if (std::abs(horizontal - dragNoteX) > 4) {
                const double delta = (horizontal - dragNoteX) / strings::ui::PhraseRollLayout::width * strings::ui::phraseDuration;
                if (noteResize) note.length = std::round((originalNote.length + delta) * 4) / 4;
                else note.start = std::round((originalNote.start + delta) * 4) / 4;
            }
            if (!noteResize) note.semitone += static_cast<int>(std::round((dragNoteY - vertical) / area.rowHeight));
            phrase.set(static_cast<size_t>(noteDrag), note); repaint(); return true;
        }
        if (expressionPad.dragging()) {
            if (!layout.pad.contains(horizontal, vertical)) releaseBow();
            else if (expressionPad.move(layout.pad.at(horizontal, vertical))) writeBowPoint(expressionPad.requested());
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
        const float horizontal = event.pos.getX() * 1040.0f / getWidth(), vertical = event.pos.getY() * 720.0f / getHeight();
        if (!details) {
            if (inputRoute != 3) return false;
            for (size_t index = 0; index < strings::ui::performanceKnobs.size(); ++index) if (strings::ui::performanceKnobs[index].contains(horizontal, vertical)) {
                if (replaying) { endReplay(); useTake = false; }
                follow(); finishLocalEdit(); editMacro(index, true);
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
        if (event.key == kKeyEscape && event.press) { stopSession(); releaseBow(); releaseKeys(); finishKnob(); finishLocalEdit(); return true; }
        const char* keys = "asdfghjk";
        constexpr int offsets[] = {0, 2, 4, 5, 7, 9, 11, 12};
        const char* found = event.key < 128 ? std::strchr(keys, static_cast<int>(event.key)) : nullptr;
        if (!found || event.key == 0) return false;
        const auto index = static_cast<size_t>(found - keys);
        if (event.press && keyboardNotes[index] < 0) {
            keyboardNotes[index] = keyboardBase() + offsets[index]; sendNote(0, static_cast<uint8_t>(keyboardNotes[index]), 100);
        } else if (!event.press && keyboardNotes[index] >= 0) {
            sendNote(0, static_cast<uint8_t>(keyboardNotes[index]), 0); keyboardNotes[index] = -1;
        }
        repaint(); return true;
    }
private:
    struct Knob { uint32_t parameter; float horizontal, vertical; const char* caption; };
    const std::array<Knob, 16> knobs {{{strings::BowPressure, 110, 260, "Bow weight"}, {strings::BowSpeed, 275, 260, "Bow speed"},
        {strings::BowPosition, 440, 260, "Contact point"}, {strings::Brightness, 605, 260, "Brightness"}, {strings::Vibrato, 770, 260, "Vibrato"},
        {strings::VibratoRate, 935, 260, "Vibrato rate"}, {strings::Attack, 110, 394, "Attack"}, {strings::Release, 275, 394, "Release"},
        {strings::Humanize, 440, 394, "Player variation"}, {strings::Room, 605, 394, "Room"}, {strings::BodyCoupling, 770, 394, "Body coupling"},
        {strings::Sympathetic, 935, 394, "Sympathetic"}, {strings::Rosin, 500, 555, "Rosin grip"}, {strings::Players, 665, 555, "Players"},
        {strings::Gain, 810, 555, "Output"}, {strings::BowDirection, 935, 555, "Direction"}}};
    std::array<float, strings::ParameterCount> values {};
    std::array<int, 8> keyboardNotes {{-1, -1, -1, -1, -1, -1, -1, -1}};
    const std::array<uint32_t, 4> bowParameters {{strings::BowPressure, strings::BowSpeed, strings::BowDirection, strings::BowEngaged}};
    const std::array<uint32_t, 5> macroParameters {{strings::BowPressure, strings::BowSpeed, strings::BowPosition, strings::Brightness, strings::Vibrato}};
    std::array<float, 4> bowBaseline {}, replayBaseline {};
    uint32_t presetIndex = 0, lastClick = 0;
    int inputRoute = 3, dragParameter = -1, auditionNote = -1, lastClicked = -1;
    int selectedNote = -1, noteDrag = -1, macroDrag = -1;
    int lastMacroIndex = -1;
    uint32_t lastMacroClick = 0;
    bool curveDrag = false, noteResize = false, noteUndo = false;
    float dragNoteX = 0, dragNoteY = 0;
    float macroStart = 0, macroY = 0;
    strings::ui::PhraseNote originalNote {};
    float dragStart = 0, dragY = 0;
    bool gestureMode = false, details = false, playing = false, replaying = false, useTake = false;
    unsigned readbackMask = 0;
    double started = 0, elapsed = 0, lastIdle = 0;
    strings::ui::ExpressionPad expressionPad;
    strings::ui::ExpressionPadView expressionView;
    strings::ui::PerformanceLayout layout;
    strings::ui::InputMapView inputView;
    strings::ui::PhraseRibbonView phraseView;
    strings::ui::KeyboardPerformanceView keyboardView;
    strings::ui::PhraseEditor phrase;
    strings::ui::ExpressionTake take;
    static double clockSeconds() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
    static bool hit(float horizontal, float vertical, float left, float top, float width) {
        return horizontal >= left && horizontal <= left + width && vertical >= top && vertical <= top + 34;
    }
    static bool nearKnob(const Knob& knob, float horizontal, float vertical) {
        return horizontal >= knob.horizontal - 65 && horizontal <= knob.horizontal + 65
            && vertical >= knob.vertical - 48 && vertical <= knob.vertical + 48;
    }
    void finishLocalEdit() {
        if (macroDrag >= 0) editMacro(static_cast<size_t>(macroDrag), false);
        noteDrag = macroDrag = -1; curveDrag = false;
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
    void writeCurve(float horizontal, float vertical) {
        const strings::ui::PhraseRollLayout area(inputRoute == 3);
        take.editWeight(area.timeAt(horizontal), 1 - (vertical - area.curveTop) / area.curveHeight); repaint();
    }
    strings::ui::ExpressionPoint commandedPoint() const {
        return {values[strings::BowSpeed] * values[strings::BowDirection], values[strings::BowPressure], values[strings::BowEngaged]};
    }
    void writeBowPoint(strings::ui::ExpressionPoint point) {
        values[strings::BowPressure] = point.weight; values[strings::BowSpeed] = std::abs(point.motion);
        values[strings::BowDirection] = point.motion < 0 ? -1 : point.motion > 0 ? 1 : 0; values[strings::BowEngaged] = point.gate;
        for (const auto parameter : bowParameters) setParameterValue(parameter, values[parameter]);
        if (take.isRecording()) take.capture(std::clamp(clockSeconds() - started, 0.0, strings::ui::phraseDuration), point);
        repaint();
    }
    void releaseBow() {
        if (!expressionPad.dragging()) return;
        expressionPad.release(); writeBowPoint(expressionPad.requested());
        for (const auto parameter : bowParameters) editParameter(parameter, false);
    }
    void restoreBowBaseline() {
        for (size_t index = 0; index < bowParameters.size(); ++index) change(bowParameters[index], bowBaseline[index]);
    }
    void follow() { releaseBow(); if (gestureMode) restoreBowBaseline(); gestureMode = false; repaint(); }
    void startSession(bool record) {
        stopSession(); follow(); elapsed = 0; started = lastIdle = clockSeconds(); playing = true;
        if (record) { take.begin(commandedPoint()); useTake = false; noteUndo = false; }
        else if (useTake && take.size() > 1) {
            for (size_t index = 0; index < bowParameters.size(); ++index) {
                replayBaseline[index] = values[bowParameters[index]]; editParameter(bowParameters[index], true);
            }
            replaying = true;
        }
        repaint();
    }
    void endReplay(bool restore = true) {
        if (!replaying) return;
        replaying = false;
        for (size_t index = 0; index < bowParameters.size(); ++index) {
            const auto parameter = bowParameters[index];
            if (restore) { values[parameter] = replayBaseline[index]; setParameterValue(parameter, values[parameter]); }
            editParameter(parameter, false);
        }
    }
    void stopSession(bool restore = true) {
        if (auditionNote >= 0) { sendNote(15, static_cast<uint8_t>(auditionNote), 0); auditionNote = -1; }
        if (restore) releaseBow();
        playing = false; take.finish(); endReplay(restore);
        repaint();
    }
    void finishKnob() {
        if (dragParameter < 0) return;
        editParameter(static_cast<uint32_t>(dragParameter), false); dragParameter = -1;
    }
    void releaseKeys() {
        for (auto& note : keyboardNotes) if (note >= 0) { sendNote(0, static_cast<uint8_t>(note), 0); note = -1; }
    }
    int keyboardBase() const { constexpr int bases[] = {60, 48, 36, 28}; return bases[static_cast<int>(values[strings::Instrument])]; }
    void change(uint32_t parameter, float value) {
        values[parameter] = strings::sanitizeParameter(parameter, value);
        editParameter(parameter, true); setParameterValue(parameter, values[parameter]); editParameter(parameter, false); repaint();
    }
    void choosePreset(int direction) {
        stopSession(); releaseKeys(); follow();
        presetIndex = static_cast<uint32_t>((static_cast<int>(presetIndex) + direction + strings::presetCount) % strings::presetCount);
        strings::applyPreset(presetIndex, [this](uint32_t parameter, float value) { change(parameter, value); });
    }
    void drawDetails() {
        strings::ui::PerformancePainter paint(*this);
        paint.box(28, 119, 984, 526, paint.paper, 8);
        paint.label(58, 157, "Inside the instrument", 24);
        paint.label(58, 183, "Physical excitation, coupled body and string response. Drag dials; Shift for fine control.", 12, true);
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
        paint.label(58, 536, "Articulation", 12);
        for (int index = 0; index < 3; ++index)
            paint.button(58 + index * 117.0f, 560, 107, strings::articulationNames[index], static_cast<int>(values[strings::Articulation]) == index);
        paint.label(58, 628, "Body profiles are synthetic, not measured instrument calibrations. No trained performance model is loaded.", 11, true);
    }
};

UI* createUI() { return new StringsUI; }

END_NAMESPACE_DISTRHO
