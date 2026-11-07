#include "dsp/Engine.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void write16(std::ofstream& stream, uint16_t value) {
    stream.put(static_cast<char>(value & 255)); stream.put(static_cast<char>((value >> 8) & 255));
}
void write32(std::ofstream& stream, uint32_t value) { write16(stream, value & 65535); write16(stream, value >> 16); }

int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "strings-demo.wav";
    const int instrument = argc > 2 ? std::clamp(std::stoi(argv[2]), 0, 3) : 0;
    const int players = argc > 3 ? std::clamp(std::stoi(argv[3]), 1, 16) : 1;
    const int articulation = argc > 4 ? std::clamp(std::stoi(argv[4]), 0, 2) : 0;
    const bool sustained = argc > 5 && std::string(argv[5]) == "sustain";
    constexpr uint32_t sampleRate = 48000, duration = 8, frames = sampleRate * duration;
    strings::Engine engine;
    engine.prepare(sampleRate);
    engine.setParameter(strings::Instrument, static_cast<float>(instrument));
    engine.setParameter(strings::Ensemble, players > 1 ? 1.0f : 0.0f);
    engine.setParameter(strings::Players, static_cast<float>(players));
    engine.setParameter(strings::Articulation, static_cast<float>(articulation));
    engine.setParameter(strings::Vibrato, sustained ? 0.0f : 0.25f);
    std::vector<float> left(frames), right(frames);
    constexpr int roots[] = {69, 62, 50, 38};
    constexpr int melody[] = {0, 2, 3, 7, 5, 3, 2, 0};
    const auto start = std::chrono::steady_clock::now();
    int previousNote = -1;
    for (uint32_t cursor = 0; cursor < frames; cursor += 240) {
        if (cursor % sampleRate == 0 && cursor < sampleRate * 6 && (!sustained || cursor == 0)) {
            if (previousNote >= 0) {
                const uint8_t off[] = {0x80, static_cast<uint8_t>(previousNote), 0}; engine.midi(off, 3);
            }
            previousNote = roots[instrument] + (sustained ? 0 : melody[cursor / sampleRate]);
            const uint8_t on[] = {0x90, static_cast<uint8_t>(previousNote), 96}; engine.midi(on, 3);
        }
        if (cursor == sampleRate * 6) {
            const uint8_t off[] = {0x80, static_cast<uint8_t>(previousNote), 0}; engine.midi(off, 3);
        }
        engine.render(left.data() + cursor, right.data() + cursor, 240);
    }
    std::ofstream file(path, std::ios::binary);
    if (!file) { std::cerr << "Cannot write " << path << '\n'; return 1; }
    file.write("RIFF", 4); write32(file, 36 + frames * 4); file.write("WAVEfmt ", 8);
    write32(file, 16); write16(file, 1); write16(file, 2); write32(file, sampleRate);
    write32(file, sampleRate * 4); write16(file, 4); write16(file, 16); file.write("data", 4); write32(file, frames * 4);
    float peak = 0; double energy = 0;
    for (uint32_t index = 0; index < frames; ++index) {
        for (float sample : {left[index], right[index]}) {
            if (!std::isfinite(sample)) { std::cerr << "Non-finite audio\n"; return 2; }
            peak = std::max(peak, std::abs(sample)); energy += sample * sample;
            write16(file, static_cast<uint16_t>(static_cast<int16_t>(std::clamp(sample, -1.0f, 1.0f) * 32767)));
        }
    }
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << path << " peak=" << peak << " rms=" << std::sqrt(energy / (frames * 2))
              << " render_seconds=" << elapsed << " realtime_factor=" << duration / elapsed << '\n';
    return peak > 0.001f ? 0 : 3;
}
