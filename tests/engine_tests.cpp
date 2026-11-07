#include "dsp/Engine.hpp"
#include "Presets.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>

static bool countAllocations = false;
static size_t allocationCount = 0;

void* operator new(std::size_t size) {
    if (countAllocations) ++allocationCount;
    if (void* memory = std::malloc(size == 0 ? 1 : size)) return memory;
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void midi(strings::Engine& engine, uint8_t status, uint8_t number, uint8_t value) {
    const uint8_t bytes[] = {status, number, value}; engine.midi(bytes, 3);
}
double render(strings::Engine& engine, int frames) {
    std::array<float, 256> left {}, right {};
    double energy = 0;
    while (frames > 0) {
        const int count = std::min(frames, 256);
        engine.render(left.data(), right.data(), count);
        for (int index = 0; index < count; ++index) {
            require(std::isfinite(left[index]) && std::isfinite(right[index]), "Non-finite audio");
            require(std::abs(left[index]) <= 1 && std::abs(right[index]) <= 1, "Unbounded audio");
            energy += left[index] * left[index] + right[index] * right[index];
        }
        frames -= count;
    }
    return energy;
}

int main() {
    try {
        strings::Engine engine;
        constexpr int notes[] = {69, 62, 50, 38};
        int scenarios = 0;
        for (double sampleRate : {44100.0, 48000.0, 96000.0}) {
            for (int instrument = 0; instrument < 4; ++instrument) {
                for (int articulation = 0; articulation < 3; ++articulation) {
                    engine.prepare(sampleRate);
                    engine.setParameter(strings::Instrument, static_cast<float>(instrument));
                    engine.setParameter(strings::Articulation, static_cast<float>(articulation));
                    engine.setParameter(strings::Room, 0);
                    require(render(engine, 256) == 0, "Idle instrument is not silent");
                    midi(engine, 0x90, static_cast<uint8_t>(notes[instrument]), 100);
                    require(render(engine, static_cast<int>(sampleRate)) > 0.001, "Instrument failed to sound");
                    midi(engine, 0x80, static_cast<uint8_t>(notes[instrument]), 0);
                    render(engine, static_cast<int>(sampleRate * 3));
                    require(engine.activeVoiceCount() == 0, "Stuck voice after note-off");
                    require(render(engine, 2048) < 0.000001, "Non-silent release tail");
                    ++scenarios;
                }
            }
        }
        engine.prepare(48000);
        engine.setParameter(strings::Ensemble, 1); engine.setParameter(strings::Players, 8);
        midi(engine, 0x90, 60, 100); midi(engine, 0x90, 64, 90); midi(engine, 0x90, 67, 80);
        require(engine.activeVoiceCount() == 8, "Chord multiplied section size");
        require(render(engine, 48000) > 0.001, "Ensemble is silent");
        midi(engine, 0xb0, 64, 127);
        midi(engine, 0x80, 60, 0); midi(engine, 0x80, 64, 0); midi(engine, 0x80, 67, 0);
        render(engine, 48000);
        require(engine.activeVoiceCount() == 8, "Sustain did not retain notes");
        midi(engine, 0xb0, 64, 0); render(engine, 144000);
        require(engine.activeVoiceCount() == 0, "Sustain release stuck");
        engine.setParameter(strings::Ensemble, 0);
        midi(engine, 0x90, 60, 100); midi(engine, 0x90, 64, 100); midi(engine, 0x80, 64, 0);
        require(engine.activeVoiceCount() == 1, "Solo legato allocation failed");
        midi(engine, 0xb0, 120, 0);
        require(engine.activeVoiceCount() == 0, "All sound off failed");
        for (uint32_t index = 0; index < strings::Meter; ++index) {
            engine.setParameter(index, std::numeric_limits<float>::quiet_NaN());
            require(std::isfinite(engine.getParameter(index)), "NaN parameter accepted");
            engine.setParameter(index, std::numeric_limits<float>::infinity());
            require(std::isfinite(engine.getParameter(index)), "Infinite parameter accepted");
        }
        for (uint32_t preset = 0; preset < strings::presetCount; ++preset) {
            engine.reset(); strings::applyPreset(preset, [&engine](uint32_t index, float value) { engine.setParameter(index, value); });
            midi(engine, 0x90, static_cast<uint8_t>(notes[static_cast<int>(engine.getParameter(strings::Instrument))]), 100);
            require(render(engine, 24000) > 0.001, "Silent factory preset");
        }
        engine.midi(nullptr, 0);
        const uint8_t shortEvent[] = {0x90}; engine.midi(shortEvent, 1);
        engine.render(nullptr, nullptr, 0);
        engine.prepare(48000);
        engine.setParameter(strings::Ensemble, 1); engine.setParameter(strings::Players, 16);
        allocationCount = 0; countAllocations = true;
        for (int note = 48; note < 72; ++note) midi(engine, 0x90, static_cast<uint8_t>(note), 100);
        render(engine, 2048);
        for (int note = 48; note < 72; ++note) midi(engine, 0x80, static_cast<uint8_t>(note), 0);
        countAllocations = false;
        require(allocationCount == 0, "Heap allocation during audio/MIDI processing");
        for (int instrument = 0; instrument < 4; ++instrument) {
            for (int extreme = 0; extreme < 2; ++extreme) {
                engine.reset();
                for (uint32_t parameter = strings::BowPressure; parameter < strings::Meter; ++parameter) {
                    const auto& spec = strings::parameterSpecs[parameter];
                    engine.setParameter(parameter, extreme == 0 ? spec.minimum : spec.maximum);
                }
                engine.setParameter(strings::Instrument, static_cast<float>(instrument));
                midi(engine, 0x90, static_cast<uint8_t>(notes[instrument]), 127);
                render(engine, 48000);
            }
        }
        std::cout << "PASS: " << scenarios << " instrument/rate/articulation cases; ensemble divisi, sustain, solo legato, panic, parameter sanitization, " << strings::presetCount << " presets\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
