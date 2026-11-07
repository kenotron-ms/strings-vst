#pragma once
#include <array>
#include <cmath>
#include <cstdint>

namespace strings {

enum ParameterId : uint32_t {
    Instrument, Ensemble, Players, BowPressure, BowSpeed, BowPosition,
    Vibrato, VibratoRate, Brightness, Attack, Release, Humanize, Room, Gain,
    Articulation, Meter, ActivePlayers, ParameterCount
};

struct ParameterSpec {
    const char* name;
    const char* symbol;
    const char* unit;
    float minimum;
    float maximum;
    float initial;
    bool integer;
};

inline constexpr std::array<ParameterSpec, ParameterCount> parameterSpecs {{
    {"Instrument", "instrument", "", 0, 3, 0, true},
    {"Ensemble", "ensemble", "", 0, 1, 0, true},
    {"Section players", "players", "", 2, 16, 8, true},
    {"Bow pressure", "bow_pressure", "%", 0, 1, 0.5f, false},
    {"Bow speed", "bow_speed", "%", 0, 1, 0.65f, false},
    {"Bow position", "bow_position", "%", 0, 1, 0.35f, false},
    {"Vibrato depth", "vibrato", "%", 0, 1, 0.22f, false},
    {"Vibrato rate", "vibrato_rate", "Hz", 3, 8, 5.5f, false},
    {"Brightness", "brightness", "%", 0, 1, 0.55f, false},
    {"Attack", "attack", "ms", 5, 500, 70, false},
    {"Release", "release", "ms", 20, 1500, 280, false},
    {"Player variation", "humanize", "%", 0, 1, 0.35f, false},
    {"Room", "room", "%", 0, 1, 0.18f, false},
    {"Output", "gain", "dB", -48, 6, -6, false},
    {"Articulation", "articulation", "", 0, 2, 0, true},
    {"Output level", "meter", "", 0, 1, 0, false},
    {"Active players", "active_players", "", 0, 16, 0, true}
}};

inline constexpr const char* instrumentNames[] = {"Violin", "Viola", "Cello", "Double bass"};
inline constexpr const char* articulationNames[] = {"Arco", "Pizzicato", "Tremolo"};

inline float sanitizeParameter(uint32_t index, float value) {
    if (index >= ParameterCount) return 0.0f;
    const auto& spec = parameterSpecs[index];
    if (!std::isfinite(value)) return spec.initial;
    value = value < spec.minimum ? spec.minimum : (value > spec.maximum ? spec.maximum : value);
    return spec.integer ? std::round(value) : value;
}

}
