#pragma once
#include "Parameters.hpp"

namespace strings {
inline constexpr const char* presetNames[] = {
    "Violin / intimate", "Viola / warm", "Cello / cantabile", "Double bass / arco",
    "Violins / chamber", "Violas / chamber", "Cellos / chamber", "Basses / chamber",
    "Violin / pizzicato", "Double bass / pizzicato", "Violins / tremolo", "Cello / sul tasto"
};
inline constexpr uint32_t presetCount = sizeof(presetNames) / sizeof(presetNames[0]);

template<class Setter> void applyPreset(uint32_t index, Setter set) {
    if (index >= presetCount) return;
    for (uint32_t parameter = 0; parameter < ParameterCount; ++parameter)
        if (isInputParameter(parameter)) set(parameter, parameterSpecs[parameter].initial);
    set(Instrument, static_cast<float>(index % 4));
    if (index >= 4 && index < 8) {
        set(Ensemble, 1); set(Players, index == 7 ? 4 : 8);
        set(Humanize, 0.5f); set(Room, 0.3f); set(Attack, 110);
    }
    if (index == 1 || index == 2 || index == 3) set(Brightness, 0.45f);
    if (index == 8 || index == 9) {
        set(Instrument, index == 8 ? 0 : 3); set(Articulation, 1);
        set(Attack, 5); set(Release, 600); set(Vibrato, 0); set(Room, 0.12f);
    }
    if (index == 10) {
        set(Instrument, 0); set(Ensemble, 1); set(Players, 12); set(Articulation, 2);
        set(Humanize, 0.6f); set(Room, 0.32f);
    }
    if (index == 11) { set(Instrument, 2); set(BowPosition, 0.85f); set(Brightness, 0.25f); set(BowPressure, 0.35f); }
}
}
