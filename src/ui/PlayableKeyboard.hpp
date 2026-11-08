#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace strings::ui {

inline constexpr bool keyboardBlackNote(int note) {
    const int chroma = note % 12;
    return chroma == 1 || chroma == 3 || chroma == 6 || chroma == 8 || chroma == 10;
}

struct PlayableKeyboard {
    static constexpr int firstNote = 21, lastNote = 108;
    static constexpr float left = 28, top = 560, width = 984, height = 108;
    static constexpr float whiteWidth = width / 52, blackWidth = whiteWidth * 0.62f, blackHeight = 70;
    inline static constexpr auto whiteNotes = [] {
        std::array<int, 52> result {};
        size_t index = 0;
        for (int note = firstNote; note <= lastNote; ++note) if (!keyboardBlackNote(note)) result[index++] = note;
        return result;
    }();
    inline static constexpr auto blackNotes = [] {
        std::array<int, 36> result {};
        size_t index = 0;
        for (int note = firstNote; note <= lastNote; ++note) if (keyboardBlackNote(note)) result[index++] = note;
        return result;
    }();
    static constexpr std::array<char, 13> shortcuts {{'a', 'w', 's', 'e', 'd', 'f', 't', 'g', 'y', 'h', 'u', 'j', 'k'}};
    static constexpr bool typingRangeVisible(int base) { return base >= firstNote && base + 12 <= lastNote; }
    static int keyIndex(uint32_t key) {
        if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
        for (size_t index = 0; index < shortcuts.size(); ++index)
            if (key == static_cast<uint32_t>(shortcuts[index])) return static_cast<int>(index);
        return -1;
    }
    static float noteLeft(int note) {
        size_t count = 0;
        for (const int white : whiteNotes) if (white < note) ++count;
        return left + count * whiteWidth - (keyboardBlackNote(note) ? blackWidth * 0.5f : 0);
    }
    static float blackLeft(size_t index) { return noteLeft(blackNotes[index]); }
    static int noteAt(float horizontal, float vertical) {
        if (!(horizontal >= left && horizontal < left + width && vertical >= top && vertical < top + height)) return -1;
        if (vertical < top + blackHeight)
            for (size_t index = 0; index < blackNotes.size(); ++index)
                if (horizontal >= blackLeft(index) && horizontal < blackLeft(index) + blackWidth) return blackNotes[index];
        return whiteNotes[static_cast<size_t>((horizontal - left) / whiteWidth)];
    }
    int press(uint32_t key, int base) {
        const int index = keyIndex(key);
        if (index < 0 || notes[static_cast<size_t>(index)] >= 0) return -1;
        return notes[static_cast<size_t>(index)] = std::clamp(base + index, 0, 127);
    }
    int release(uint32_t key) {
        const int index = keyIndex(key);
        if (index < 0) return -1;
        auto& held = notes[static_cast<size_t>(index)];
        const int result = held;
        held = -1;
        return result;
    }
    bool active(int note) const { return std::find(notes.begin(), notes.end(), note) != notes.end(); }
    std::array<int, 13> notes {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
};

}
