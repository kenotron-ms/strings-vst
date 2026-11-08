#pragma once
#include "PlayableKeyboard.hpp"
#include "PerformanceViews.hpp"
#include <cstdio>

namespace strings::ui {

class PlayableKeyboardView {
public:
    void draw(DGL_NAMESPACE::NanoVG& canvas, const PlayableKeyboard& keyboard, int base, int pointerNote, int velocity) const {
        PerformancePainter paint(canvas);
        constexpr const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        paint.rule(510);
        paint.label(28, 540, "Keyboard", 14);
        paint.button(335, 518, 34, "-");
        paint.button(545, 518, 34, "+");
        char caption[32];
        std::snprintf(caption, sizeof(caption), "Octave %s%d", noteNames[base % 12], base / 12 - 1);
        paint.label(458, 540, caption, 11, true, true);
        for (size_t index = 0; index < PlayableKeyboard::whiteNotes.size(); ++index) {
            const int midiNote = PlayableKeyboard::whiteNotes[index];
            const int semitone = midiNote - base;
            const float horizontal = PlayableKeyboard::left + index * PlayableKeyboard::whiteWidth;
            const bool active = keyboard.active(midiNote) || pointerNote == midiNote;
            const bool typing = semitone >= 0 && semitone < 13;
            paint.box(horizontal, PlayableKeyboard::top, PlayableKeyboard::whiteWidth - 1, PlayableKeyboard::height,
                active ? paint.brass : typing ? paint.sage : paint.sage.withAlpha(0.3f), 1);
            if (midiNote % 12 == 0 || midiNote == PlayableKeyboard::firstNote) {
                std::snprintf(caption, sizeof(caption), "%s%d", noteNames[midiNote % 12], midiNote / 12 - 1);
                paint.label(horizontal + PlayableKeyboard::whiteWidth * 0.5f, 661, caption, 8, true, true);
            }
            if (typing) {
                char label[] = {static_cast<char>(PlayableKeyboard::shortcuts[static_cast<size_t>(semitone)] - 'a' + 'A'), 0};
                paint.label(horizontal + PlayableKeyboard::whiteWidth * 0.5f, 647, label, 10, false, true);
            }
        }
        for (size_t index = 0; index < PlayableKeyboard::blackNotes.size(); ++index) {
            const int midiNote = PlayableKeyboard::blackNotes[index];
            const int semitone = midiNote - base;
            const float horizontal = PlayableKeyboard::blackLeft(index);
            const bool active = keyboard.active(midiNote) || pointerNote == midiNote;
            paint.box(horizontal, PlayableKeyboard::top, PlayableKeyboard::blackWidth, PlayableKeyboard::blackHeight, active ? paint.brass : paint.pine, 1);
            if (semitone >= 0 && semitone < 13) {
                char label[] = {static_cast<char>(PlayableKeyboard::shortcuts[static_cast<size_t>(semitone)] - 'a' + 'A'), 0};
                canvas.fontFace(NANOVG_DEJAVU_SANS_TTF); canvas.fontSize(9); canvas.fillColor(paint.paper);
                canvas.textAlign(DGL_NAMESPACE::NanoVG::ALIGN_CENTER | DGL_NAMESPACE::NanoVG::ALIGN_BASELINE);
                canvas.text(horizontal + PlayableKeyboard::blackWidth * 0.5f, 620, label, nullptr);
            }
        }
        std::snprintf(caption, sizeof(caption), "Velocity %d", velocity);
        paint.label(660, 540, caption, 11);
        paint.button(748, 518, 34, "-"); paint.button(790, 518, 34, "+");
        paint.button(858, 518, 154, "Release");
    }
};

}
