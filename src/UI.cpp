#include "DistrhoUI.hpp"
#include "Parameters.hpp"
#include "Presets.hpp"
#include "Font.hpp"
#include <algorithm>
#include <array>
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
    ~StringsUI() override { releaseKeys(); }
protected:
    void parameterChanged(uint32_t index, float value) override {
        if (index < values.size()) values[index] = strings::sanitizeParameter(index, value);
        repaint();
    }
    void programLoaded(uint32_t index) override { presetIndex = index; repaint(); }
    void uiIdle() override {
        if (values[strings::Meter] > 0.0001f) { animation += 0.18f; repaint(); }
    }
    void onNanoDisplay() override {
        scale(getWidth() / 1040.0f, getHeight() / 720.0f);
        box(0, 0, 1040, 720, background);
        box(0, 0, 1040, 80, deep);
        label(28, 40, "Strings", 31, ivory, false, true);
        label(132, 43, "Kenotron", 12, muted);
        button(264, 23, 32, 34, "<", false);
        label(312, 43, strings::presetNames[presetIndex % strings::presetCount], 13, ivory);
        button(598, 23, 32, 34, ">", false);
        button(660, 23, 78, 34, "Solo", values[strings::Ensemble] < 0.5f);
        button(742, 23, 106, 34, "Ensemble", values[strings::Ensemble] > 0.5f);
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%+.1f dB", values[strings::Gain]);
        label(877, 30, "Output", 11, muted);
        label(1008, 30, buffer, 11, ivory, true);
        box(878, 48, 130, 4, lineColor, 2);
        box(878, 48, 130 * (values[strings::Gain] + 48) / 54, 4, brass, 2);
        box(878, 62, 130, 2, lineColor);
        box(878, 62, 130 * std::min(1.0f, values[strings::Meter] * 2), 2, silver);
        for (int instrument = 0; instrument < 4; ++instrument)
            button(28 + instrument * 249.0f, 98, 237, 36, strings::instrumentNames[instrument], static_cast<int>(values[strings::Instrument]) == instrument);

        box(28, 153, 650, 204, deep, 10);
        box(697, 153, 315, 204, deep, 10);
        const int instrument = static_cast<int>(values[strings::Instrument]);
        label(58, 202, strings::instrumentNames[instrument], 47, ivory, false, true);
        label(648, 191, values[strings::Articulation] == 1 ? "Plucked string" : "Bow / string interaction", 11, muted, true);
        const float contact = 525 - values[strings::BowPosition] * 280;
        const float activity = std::min(1.0f, values[strings::Meter] * 5);
        for (int stringIndex = 0; stringIndex < 4; ++stringIndex) {
            const float baseline = 233 + stringIndex * 23.0f;
            beginPath();
            for (int point = 0; point <= 110; ++point) {
                const float fraction = point / 110.0f;
                const float displacement = std::sin(fraction * 3.14159f) * std::sin(fraction * (18 + stringIndex * 3) + animation) * activity * 4;
                if (point == 0) moveTo(60, baseline);
                else lineTo(60 + fraction * 585, baseline + displacement);
            }
            strokeColor(stringIndex == 1 ? brass : silver.withAlpha(0.5f)); strokeWidth(1.0f + stringIndex * 0.3f); stroke();
        }
        beginPath(); moveTo(contact - 19, 220); lineTo(contact + 26, 322);
        strokeColor(ivory.withAlpha(0.65f)); strokeWidth(5); stroke();
        beginPath(); moveTo(contact - 12, 220); lineTo(contact + 33, 322);
        strokeColor(brass); strokeWidth(1.2f); stroke();
        label(58, 339, "Bridge", 10, muted);
        label(645, 339, "Fingerboard", 10, muted, true);
        label(721, 181, "The players", 13, ivory);
        std::snprintf(buffer, sizeof(buffer), "%d %s", values[strings::Ensemble] > 0.5f ? static_cast<int>(values[strings::Players]) : 1,
                      values[strings::Ensemble] > 0.5f ? "players / divisi" : "player / legato");
        label(988, 181, buffer, 10, muted, true);
        const int players = values[strings::Ensemble] > 0.5f ? static_cast<int>(values[strings::Players]) : 1;
        for (int player = 0; player < players; ++player) {
            const float angle = players == 1 ? 0 : (player / static_cast<float>(players - 1) - 0.5f) * 2.2f;
            const float radius = player % 2 ? 75.0f : 55.0f;
            const float playerX = 854 + std::sin(angle) * radius;
            const float playerY = 273 - std::cos(angle) * radius;
            beginPath(); circle(playerX, playerY, 5); fillColor(activity > 0.02f ? brass : silver); fill();
            beginPath(); moveTo(playerX - 6, playerY + 13); bezierTo(playerX - 4, playerY + 5, playerX + 4, playerY + 5, playerX + 6, playerY + 13);
            strokeColor(silver.withAlpha(0.5f)); strokeWidth(1.2f); stroke();
        }
        label(854, 292, "Section size", 10, muted, false, false, true);
        button(788, 306, 32, 27, "-", false);
        std::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(values[strings::Players]));
        label(854, 324, buffer, 15, values[strings::Ensemble] > 0.5f ? ivory : muted, false, false, true);
        button(888, 306, 32, 27, "+", false);
        for (int articulation = 0; articulation < 3; ++articulation)
            button(28 + articulation * 112.0f, 371, 102, 27, strings::articulationNames[articulation], static_cast<int>(values[strings::Articulation]) == articulation);
        label(1010, 389, "Drag to shape the instrument. Double-click to reset.", 11, muted, true);
        for (const auto& control : knobs) drawKnob(control);
        drawKeyboard();
        label(28, 703, "Waveguide strings / no samples", 10, muted);
        label(520, 703, "Play: A S D F G H J K     Sustain: MIDI CC64", 10, muted, false, false, true);
        label(1012, 703, "0.1.0 / experimental", 10, muted, true);
    }
    bool onMouse(const MouseEvent& event) override {
        const float mouseX = event.pos.getX() * 1040.0f / getWidth();
        const float mouseY = event.pos.getY() * 720.0f / getHeight();
        if (!event.press) {
            if (dragParameter >= 0) { editParameter(static_cast<uint32_t>(dragParameter), false); dragParameter = -1; }
            if (mouseNote >= 0) { sendNote(0, static_cast<uint8_t>(mouseNote), 0); mouseNote = -1; repaint(); }
            return true;
        }
        if (event.button != 1) return false;
        for (int instrument = 0; instrument < 4; ++instrument)
            if (inside(mouseX, mouseY, 28 + instrument * 249.0f, 98, 237, 36)) { releaseKeys(); change(strings::Instrument, static_cast<float>(instrument)); return true; }
        if (inside(mouseX, mouseY, 660, 23, 78, 34)) { change(strings::Ensemble, 0); return true; }
        if (inside(mouseX, mouseY, 742, 23, 106, 34)) { change(strings::Ensemble, 1); return true; }
        if (inside(mouseX, mouseY, 264, 23, 32, 34)) { choosePreset(-1); return true; }
        if (inside(mouseX, mouseY, 598, 23, 32, 34)) { choosePreset(1); return true; }
        if (inside(mouseX, mouseY, 788, 306, 32, 27)) { change(strings::Players, values[strings::Players] - 1); return true; }
        if (inside(mouseX, mouseY, 888, 306, 32, 27)) { change(strings::Players, values[strings::Players] + 1); return true; }
        for (int articulation = 0; articulation < 3; ++articulation)
            if (inside(mouseX, mouseY, 28 + articulation * 112.0f, 371, 102, 27)) { change(strings::Articulation, static_cast<float>(articulation)); return true; }
        if (inside(mouseX, mouseY, 875, 35, 138, 28)) {
            change(strings::Gain, (mouseX - 878) / 130 * 54 - 48); return true;
        }
        for (const auto& control : knobs) {
            if (inside(mouseX, mouseY, control.centerX - 75, control.centerY - 40, 150, 85)) {
                if (event.time - lastClick < 350 && lastClicked == static_cast<int>(control.parameter)) {
                    change(control.parameter, strings::parameterSpecs[control.parameter].initial);
                } else {
                    dragParameter = static_cast<int>(control.parameter); dragStart = values[control.parameter]; dragY = mouseY;
                    editParameter(control.parameter, true);
                }
                lastClick = event.time; lastClicked = static_cast<int>(control.parameter);
                return true;
            }
        }
        if (mouseY >= 612 && mouseY <= 676) {
            mouseNote = keyAt(mouseX, mouseY);
            if (mouseNote >= 0) sendNote(0, static_cast<uint8_t>(mouseNote), 100);
            repaint(); return true;
        }
        return false;
    }
    bool onMotion(const MotionEvent& event) override {
        if (dragParameter < 0) return false;
        const auto& spec = strings::parameterSpecs[static_cast<size_t>(dragParameter)];
        const float mouseY = event.pos.getY() * 720.0f / getHeight();
        const float sensitivity = (event.mod & kModifierShift) ? 0.1f : 1.0f;
        const float value = dragStart + (dragY - mouseY) / 160 * (spec.maximum - spec.minimum) * sensitivity;
        values[static_cast<size_t>(dragParameter)] = strings::sanitizeParameter(static_cast<uint32_t>(dragParameter), value);
        setParameterValue(static_cast<uint32_t>(dragParameter), values[static_cast<size_t>(dragParameter)]);
        repaint(); return true;
    }
    bool onScroll(const ScrollEvent& event) override {
        const float mouseX = event.pos.getX() * 1040.0f / getWidth(), mouseY = event.pos.getY() * 720.0f / getHeight();
        for (const auto& control : knobs) if (inside(mouseX, mouseY, control.centerX - 75, control.centerY - 40, 150, 85)) {
            const auto& spec = strings::parameterSpecs[control.parameter];
            change(control.parameter, values[control.parameter] + event.delta.getY() * (spec.maximum - spec.minimum) * 0.02f); return true;
        }
        return false;
    }
    bool onKeyboard(const KeyboardEvent& event) override {
        const char* whiteKeys = "asdfghjk";
        constexpr int whiteNotes[] = {0, 2, 4, 5, 7, 9, 11, 12};
        const char* found = event.key < 128 ? std::strchr(whiteKeys, static_cast<int>(event.key)) : nullptr;
        if (!found || event.key == 0) return false;
        const auto key = static_cast<size_t>(found - whiteKeys);
        if (event.press && keyboardNotes[key] < 0) {
            keyboardNotes[key] = keyboardBase() + whiteNotes[key];
            sendNote(0, static_cast<uint8_t>(keyboardNotes[key]), 100);
        } else if (!event.press && keyboardNotes[key] >= 0) {
            sendNote(0, static_cast<uint8_t>(keyboardNotes[key]), 0); keyboardNotes[key] = -1;
        }
        repaint(); return true;
    }
private:
    struct Knob { uint32_t parameter; float centerX, centerY; const char* caption; };
    const std::array<Knob, 10> knobs {{{strings::BowPressure, 110, 444, "Bow pressure"}, {strings::BowSpeed, 315, 444, "Bow speed"},
        {strings::BowPosition, 520, 444, "Contact point"}, {strings::Brightness, 725, 444, "Brightness"}, {strings::Vibrato, 930, 444, "Vibrato"},
        {strings::Attack, 110, 548, "Attack"}, {strings::Release, 315, 548, "Release"}, {strings::Humanize, 520, 548, "Player variation"},
        {strings::Room, 725, 548, "Room"}, {strings::VibratoRate, 930, 548, "Vibrato rate"}}};
    const Color background {32, 44, 57}, deep {23, 33, 44}, ivory {238, 232, 219}, brass {203, 178, 124};
    const Color silver {155, 184, 198}, muted {144, 160, 173}, lineColor {57, 73, 88};
    std::array<float, strings::ParameterCount> values {};
    std::array<int, 8> keyboardNotes {{-1, -1, -1, -1, -1, -1, -1, -1}};
    uint32_t presetIndex = 0;
    int dragParameter = -1, mouseNote = -1, lastClicked = -1;
    float dragStart = 0, dragY = 0, animation = 0;
    uint32_t lastClick = 0;
    static bool inside(float pointX, float pointY, float left, float top, float width, float height) {
        return pointX >= left && pointX <= left + width && pointY >= top && pointY <= top + height;
    }
    void box(float left, float top, float width, float height, Color color, float radius = 0) {
        beginPath(); roundedRect(left, top, width, height, radius); fillColor(color); fill();
    }
    void label(float left, float top, const char* value, float size, Color color, bool right = false, bool title = false, bool center = false) {
        fontFace(title ? "instrument" : NANOVG_DEJAVU_SANS_TTF); fontSize(size); fillColor(color);
        textAlign((center ? ALIGN_CENTER : right ? ALIGN_RIGHT : ALIGN_LEFT) | ALIGN_BASELINE); text(left, top, value, nullptr);
    }
    void button(float left, float top, float width, float height, const char* caption, bool selected) {
        box(left, top, width, height, selected ? brass : lineColor.withAlpha(0.35f), 5);
        label(left + width / 2, top + height / 2 + 4, caption, 12, selected ? deep : muted, false, false, true);
    }
    void drawKnob(const Knob& control) {
        const auto& spec = strings::parameterSpecs[control.parameter];
        const float normalized = (values[control.parameter] - spec.minimum) / (spec.maximum - spec.minimum);
        const float start = 0.75f * 3.14159265f, end = 2.25f * 3.14159265f;
        beginPath(); arc(control.centerX, control.centerY, 27, start, end, CW); strokeColor(lineColor); strokeWidth(3); stroke();
        beginPath(); arc(control.centerX, control.centerY, 27, start, start + normalized * (end - start), CW); strokeColor(brass); strokeWidth(3); stroke();
        const float angle = start + normalized * (end - start);
        beginPath(); moveTo(control.centerX + std::cos(angle) * 13, control.centerY + std::sin(angle) * 13);
        lineTo(control.centerX + std::cos(angle) * 20, control.centerY + std::sin(angle) * 20); strokeColor(ivory); strokeWidth(2); stroke();
        char buffer[32];
        if (control.parameter == strings::VibratoRate) std::snprintf(buffer, sizeof(buffer), "%.1f Hz", values[control.parameter]);
        else if (control.parameter == strings::Attack || control.parameter == strings::Release) std::snprintf(buffer, sizeof(buffer), "%.0f ms", values[control.parameter]);
        else std::snprintf(buffer, sizeof(buffer), "%.0f%%", values[control.parameter] * 100);
        label(control.centerX, control.centerY - 36, control.caption, 12, ivory, false, false, true);
        label(control.centerX, control.centerY + 46, buffer, 11, muted, false, false, true);
    }
    void change(uint32_t parameter, float value) {
        values[parameter] = strings::sanitizeParameter(parameter, value);
        editParameter(parameter, true); setParameterValue(parameter, values[parameter]); editParameter(parameter, false); repaint();
    }
    void choosePreset(int direction) {
        releaseKeys();
        presetIndex = static_cast<uint32_t>((static_cast<int>(presetIndex) + direction + strings::presetCount) % strings::presetCount);
        strings::applyPreset(presetIndex, [this](uint32_t parameter, float value) { change(parameter, value); });
    }
    int keyboardBase() const { constexpr int bases[] = {60, 48, 36, 24}; return bases[static_cast<int>(values[strings::Instrument])]; }
    static bool black(int note) { const int pitch = note % 12; return pitch == 1 || pitch == 3 || pitch == 6 || pitch == 8 || pitch == 10; }
    bool pressed(int note) const { return mouseNote == note || std::find(keyboardNotes.begin(), keyboardNotes.end(), note) != keyboardNotes.end(); }
    void drawKeyboard() {
        constexpr float width = 984.0f / 21;
        int whiteIndex = 0;
        for (int note = 0; note < 36; ++note) if (!black(note)) {
            const float left = 28 + whiteIndex++ * width;
            box(left + 1, 612, width - 2, 64, pressed(keyboardBase() + note) ? brass : ivory, 2);
            if (note % 12 == 0) {
                char caption[8]; std::snprintf(caption, sizeof(caption), "C%d", (keyboardBase() + note) / 12 - 1);
                label(left + 8, 667, caption, 10, deep);
            }
        }
        whiteIndex = 0;
        for (int note = 0; note < 36; ++note) {
            if (!black(note)) ++whiteIndex;
            else box(28 + whiteIndex * width - width * 0.3f, 612, width * 0.6f, 39, pressed(keyboardBase() + note) ? brass : deep, 2);
        }
    }
    int keyAt(float mouseX, float mouseY) const {
        constexpr float width = 984.0f / 21;
        int whiteIndex = 0;
        if (mouseY < 651) for (int note = 0; note < 36; ++note) {
            if (!black(note)) ++whiteIndex;
            else if (inside(mouseX, mouseY, 28 + whiteIndex * width - width * 0.3f, 612, width * 0.6f, 39)) return keyboardBase() + note;
        }
        whiteIndex = 0;
        for (int note = 0; note < 36; ++note) if (!black(note)) {
            if (inside(mouseX, mouseY, 28 + whiteIndex++ * width, 612, width, 64)) return keyboardBase() + note;
        }
        return -1;
    }
    void releaseKeys() {
        for (auto& note : keyboardNotes) if (note >= 0) { sendNote(0, static_cast<uint8_t>(note), 0); note = -1; }
        if (mouseNote >= 0) { sendNote(0, static_cast<uint8_t>(mouseNote), 0); mouseNote = -1; }
    }
};

UI* createUI() { return new StringsUI; }

END_NAMESPACE_DISTRHO
