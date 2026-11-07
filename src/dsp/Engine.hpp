#pragma once
#include "Parameters.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace strings {

constexpr double pi = 3.14159265358979323846;

class Delay {
public:
    void prepare(size_t capacity);
    void clear();
    float read(float samples) const;
    void write(float value);
private:
    std::vector<float> data;
    size_t position = 0;
};

struct Resonator {
    void configure(float frequency, float quality, double sampleRate);
    float tick(float input);
    void clear();
    float numerator = 0, feedback1 = 0, feedback2 = 0;
    float input2 = 0, input1 = 0, output1 = 0, output2 = 0;
};

class Voice {
public:
    void prepare(double sampleRate, int identity);
    void reset();
    void start(int midiNote, int midiChannel, float velocity, int instrument, bool pluck);
    void stop();
    void controls(const std::array<float, ParameterCount>& values, float bend, float expression);
    float tick();
    bool active() const { return running; }
    bool held() const { return gate; }
    int note = -1;
    int channel = 0;
    float pan = 0;
private:
    void configureBody(int instrument);
    float random();
    double sampleRate = 48000;
    Delay neck, bridge;
    std::array<Resonator, 8> body;
    uint32_t randomState = 1;
    int playerId = 0, instrumentId = -1;
    bool running = false, gate = false, pizzicato = false;
    float envelope = 0, velocity = 0, frequency = 220, targetFrequency = 220;
    float bowEnvelope = 0, stringLowpass = 0, dcInput = 0, dcOutput = 0;
    float neckLength = 100, bridgeLength = 20, basePeriod = 120;
    float pole = 0.55f, loopGain = 0.995f, bowSlope = 3, bowVelocity = 0.15f;
    float attackCoefficient = 0.001f, releaseCoefficient = 0.001f;
    float vibratoDepth = 0, vibratoIncrement = 0, vibratoPhase = 0;
    float drift = 0, variation = 0, tremoloPhase = 0, tremoloGain = 1;
    float pluckEnvelope = 0, pluckDecay = 0.99f, outputEnvelope = 0;
    uint32_t age = 0;
};

class RoomModel {
public:
    void prepare(double sampleRate);
    void reset();
    std::array<float, 2> tick(float left, float right);
private:
    std::array<Delay, 8> combs;
    std::array<float, 8> lengths {}, damped {};
};

class Engine {
public:
    Engine();
    void prepare(double sampleRate);
    void reset();
    void setParameter(uint32_t index, float value);
    float getParameter(uint32_t index) const;
    void midi(const uint8_t* data, uint32_t size);
    void render(float* left, float* right, uint32_t frames);
    int activeVoiceCount() const;
private:
    struct HeldNote { int note = 0, channel = 0; float velocity = 0; bool keyDown = false; uint64_t order = 0; };
    void redistribute();
    void updateControls();
    void noteOn(int note, int channel, float velocity);
    void noteOff(int note, int channel);
    std::array<float, ParameterCount> parameters {}, smoothed {};
    std::array<Voice, 16> voices;
    std::array<HeldNote, 128> notes {};
    std::array<float, 16> bends {}, expressions {}, modulations {}, pressures {};
    std::array<bool, 16> sustain {};
    RoomModel room;
    double sampleRate = 48000;
    uint64_t orderCounter = 0;
    uint32_t controlCounter = 0;
    float meter = 0, outputGain = 0.5f, playerNormalization = 1;
};

}
