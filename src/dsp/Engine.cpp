#include "Engine.hpp"
#include <algorithm>
#include <cmath>

namespace strings {

void Delay::prepare(size_t capacity) { data.assign(capacity + 4, 0); position = 0; }
void Delay::clear() { std::fill(data.begin(), data.end(), 0); position = 0; }
float Delay::read(float samples) const {
    samples = std::clamp(samples, 1.0f, static_cast<float>(data.size() - 2));
    float readPosition = static_cast<float>(position) - samples;
    if (readPosition < 0) readPosition += static_cast<float>(data.size());
    const auto first = static_cast<size_t>(readPosition);
    const auto second = (first + 1) % data.size();
    const float fraction = readPosition - static_cast<float>(first);
    return data[first] + fraction * (data[second] - data[first]);
}
void Delay::write(float value) { data[position] = value; position = (position + 1) % data.size(); }

void Resonator::configure(float frequency, float quality, double sampleRate) {
    const float omega = static_cast<float>(2 * pi * std::min(frequency, static_cast<float>(sampleRate * 0.43)) / sampleRate);
    const float alpha = std::sin(omega) / (2 * quality);
    numerator = alpha / (1 + alpha);
    feedback1 = -2 * std::cos(omega) / (1 + alpha);
    feedback2 = (1 - alpha) / (1 + alpha);
}
float Resonator::tick(float input) {
    const float output = numerator * (input - input2) - feedback1 * output1 - feedback2 * output2;
    input2 = input1; input1 = input;
    output2 = output1; output1 = output;
    return output;
}
void Resonator::clear() { input1 = input2 = output1 = output2 = 0; }

void Voice::prepare(double newSampleRate, int identity) {
    sampleRate = newSampleRate; playerId = identity;
    pluckDecay = static_cast<float>(std::exp(-1.0 / (sampleRate * 0.002)));
    neck.prepare(static_cast<size_t>(sampleRate / 15));
    bridge.prepare(static_cast<size_t>(sampleRate / 15));
    randomState = 0x1234567u + static_cast<uint32_t>(identity) * 1973u;
    instrumentId = -1;
    reset();
}
void Voice::reset() {
    neck.clear(); bridge.clear();
    for (auto& mode : body) mode.clear();
    note = -1; running = gate = false;
    envelope = bowEnvelope = stringLowpass = dcInput = dcOutput = outputEnvelope = 0;
    pluckEnvelope = 0; age = 0;
}
float Voice::random() {
    randomState ^= randomState << 13; randomState ^= randomState >> 17; randomState ^= randomState << 5;
    return static_cast<float>(randomState >> 8) / 8388608.0f - 1;
}
void Voice::configureBody(int instrument) {
    constexpr float scales[] = {1.0f, 0.78f, 0.43f, 0.25f};
    constexpr float modes[] = {275, 460, 550, 720, 1100, 1600, 2250, 3100};
    for (size_t index = 0; index < body.size(); ++index)
        body[index].configure(modes[index] * scales[instrument] * (1 + 0.003f * playerId), 3.5f + index * 0.9f, sampleRate);
    instrumentId = instrument;
}
void Voice::start(int midiNote, int midiChannel, float newVelocity, int instrument, bool pluck) {
    const bool legato = running && gate && !pluck && !pizzicato && instrument == instrumentId;
    if (!legato) {
        reset();
        frequency = 440 * std::pow(2.0f, (midiNote - 69) / 12.0f);
        vibratoPhase = static_cast<float>(playerId * 1.713);
        tremoloPhase = playerId * 0.37f;
    }
    if (instrumentId != instrument) configureBody(instrument);
    note = midiNote; channel = midiChannel; velocity = newVelocity;
    targetFrequency = 440 * std::pow(2.0f, (midiNote - 69) / 12.0f);
    gate = running = true; pizzicato = pluck;
    if (pluck) { pluckEnvelope = velocity; envelope = 1; }
    age = 0;
}
void Voice::stop() { gate = false; }
void Voice::controls(const std::array<float, ParameterCount>& values, float bend, float expression) {
    if (!running) return;
    variation = values[Ensemble] > 0.5f ? values[Humanize] : 0;
    const float identity = std::sin(playerId * 2.39996f + 0.7f);
    drift = variation * identity * 7.0f;
    frequency += 0.18f * (targetFrequency - frequency);
    const float tuned = frequency * std::pow(2.0f, (bend * 200 + drift) / 1200.0f);
    pole = std::clamp(0.72f - values[Brightness] * 0.46f, 0.1f, 0.8f);
    pole = std::pow(pole, static_cast<float>(48000 / sampleRate));
    basePeriod = std::max(4.0f, static_cast<float>(sampleRate / tuned) - pole / (1 - pole));
    const float position = std::clamp(0.055f + values[BowPosition] * 0.22f + variation * identity * 0.008f, 0.04f, 0.3f);
    bridgeLength = basePeriod * position;
    neckLength = basePeriod * (1 - position);
    bowSlope = 5 - 4 * values[BowPressure];
    bowVelocity = (0.03f + values[BowSpeed] * 0.22f) * (0.4f + velocity * 0.6f) * expression;
    attackCoefficient = 1 - std::exp(-1.0f / static_cast<float>(sampleRate * values[Attack] * 0.001));
    releaseCoefficient = 1 - std::exp(-1.0f / static_cast<float>(sampleRate * values[Release] * 0.001 / 5));
    vibratoDepth = values[Vibrato] * (0.022f + variation * (identity + 1) * 0.004f);
    vibratoIncrement = static_cast<float>(2 * pi / sampleRate) * values[VibratoRate] * (1 + variation * identity * 0.12f);
    tremoloGain = static_cast<int>(values[Articulation]) == 2 ? 0.5f : 1.0f;
    const float decay = pizzicato ? (instrumentId + 1) * 1.3f : 7.0f;
    loopGain = std::pow(0.001f, 1.0f / (tuned * decay));
}
float Voice::tick() {
    if (!running) return 0;
    ++age;
    envelope += ((gate ? 1.0f : 0.0f) - envelope) * (gate ? attackCoefficient : releaseCoefficient);
    vibratoPhase += vibratoIncrement;
    if (vibratoPhase > 2 * pi) vibratoPhase -= static_cast<float>(2 * pi);
    tremoloPhase += static_cast<float>(2 * pi * (8.5 + variation * std::sin(playerId)) / sampleRate);
    if (tremoloPhase > 2 * pi) tremoloPhase -= static_cast<float>(2 * pi);
    const float vibratoRamp = std::min(1.0f, static_cast<float>(age / (sampleRate * 0.3)));
    const float modulation = 1 + vibratoDepth * vibratoRamp * std::sin(vibratoPhase);
    const float bridgeWave = bridge.read(bridgeLength * modulation);
    stringLowpass = (1 - pole) * bridgeWave + pole * stringLowpass;
    const float fromBridge = -loopGain * stringLowpass;
    const float fromNut = -neck.read(neckLength * modulation);
    float excitation = 0;
    if (pizzicato) {
        excitation = random() * pluckEnvelope * 1.2f;
        pluckEnvelope *= pluckDecay;
    } else {
        const float tremolo = tremoloGain == 1 ? 1 : 0.35f + 0.65f * (0.5f + 0.5f * std::sin(tremoloPhase));
        const float relativeVelocity = bowVelocity * envelope * tremolo - fromBridge - fromNut;
        const float frictionBase = std::abs((relativeVelocity + 0.001f) * bowSlope) + 0.75f;
        const float squared = frictionBase * frictionBase;
        const float friction = std::clamp(1.0f / (squared * squared), 0.01f, 0.98f);
        excitation = relativeVelocity * friction * envelope;
    }
    bridge.write(std::clamp(fromNut + excitation, -4.0f, 4.0f));
    neck.write(std::clamp(fromBridge + excitation, -4.0f, 4.0f));
    const float raw = bridgeWave;
    float resonance = 0;
    constexpr float weights[] = {0.8f, 1, 0.85f, 0.7f, 0.65f, 0.6f, 0.5f, 0.35f};
    for (size_t index = 0; index < body.size(); ++index) resonance += weights[index] * body[index].tick(raw);
    const float input = raw * 0.28f + resonance * 1.8f;
    const float output = input - dcInput + 0.995f * dcOutput;
    dcInput = input; dcOutput = output;
    outputEnvelope += 0.002f * (std::abs(output) - outputEnvelope);
    if (!gate && envelope < 0.0001f) {
        running = false; note = -1;
        return 0;
    }
    return output * envelope * 2;
}

void RoomModel::prepare(double sampleRate) {
    constexpr float times[] = {0.0297f, 0.0371f, 0.0411f, 0.0437f, 0.0309f, 0.0383f, 0.0427f, 0.0451f};
    for (size_t index = 0; index < combs.size(); ++index) {
        lengths[index] = std::round(times[index] * static_cast<float>(sampleRate));
        combs[index].prepare(static_cast<size_t>(lengths[index]) + 2);
    }
    reset();
}
void RoomModel::reset() { for (auto& comb : combs) comb.clear(); damped.fill(0); }
std::array<float, 2> RoomModel::tick(float left, float right) {
    std::array<float, 2> output {};
    for (size_t index = 0; index < combs.size(); ++index) {
        const float reflected = combs[index].read(lengths[index]);
        damped[index] = reflected * 0.25f + damped[index] * 0.75f;
        combs[index].write((left + right) * 0.18f + damped[index] * 0.73f);
        output[index / 4] += reflected * 0.25f;
    }
    return output;
}

Engine::Engine() {
    for (size_t index = 0; index < parameters.size(); ++index) parameters[index] = parameterSpecs[index].initial;
    smoothed = parameters;
    expressions.fill(1); pressures.fill(1);
}
void Engine::prepare(double newSampleRate) {
    sampleRate = std::clamp(std::isfinite(newSampleRate) ? newSampleRate : 48000.0, 8000.0, 384000.0);
    for (size_t index = 0; index < voices.size(); ++index) voices[index].prepare(sampleRate, static_cast<int>(index));
    room.prepare(sampleRate); reset();
}
void Engine::reset() {
    for (auto& voice : voices) voice.reset();
    notes.fill({}); sustain.fill(false); bends.fill(0); expressions.fill(1); modulations.fill(0); pressures.fill(1);
    room.reset(); smoothed = parameters; meter = 0; controlCounter = 0; orderCounter = 0;
}
void Engine::setParameter(uint32_t index, float value) {
    if (index >= Meter) return;
    const float sanitized = sanitizeParameter(index, value);
    if (parameters[index] == sanitized) return;
    parameters[index] = sanitized;
    if (index == Instrument || index == Articulation) {
        for (auto& voice : voices) voice.stop();
        redistribute();
    } else if (index == Ensemble || index == Players) redistribute();
}
float Engine::getParameter(uint32_t index) const {
    if (index == Meter) return meter;
    if (index == ActivePlayers) return static_cast<float>(activeVoiceCount());
    return index < ParameterCount ? parameters[index] : 0;
}
int Engine::activeVoiceCount() const {
    int count = 0;
    for (const auto& voice : voices) if (voice.active()) ++count;
    return count;
}
void Engine::noteOn(int note, int channel, float velocity) {
    HeldNote* freeSlot = nullptr;
    for (auto& held : notes) {
        if (held.velocity > 0 && held.note == note && held.channel == channel) { freeSlot = &held; break; }
        if (held.velocity == 0 && !freeSlot) freeSlot = &held;
    }
    if (!freeSlot) freeSlot = &*std::min_element(notes.begin(), notes.end(), [](const HeldNote& first, const HeldNote& second) { return first.order < second.order; });
    *freeSlot = {note, channel, velocity, true, ++orderCounter};
    for (auto& voice : voices)
        if (voice.note == note && voice.channel == channel && parameters[Articulation] == 1) voice.stop();
    redistribute();
}
void Engine::noteOff(int note, int channel) {
    for (auto& held : notes) if (held.note == note && held.channel == channel && held.velocity > 0) {
        held.keyDown = false;
        if (!sustain[channel]) held.velocity = 0;
    }
    redistribute();
}
void Engine::redistribute() {
    std::array<const HeldNote*, 128> held {};
    size_t count = 0;
    for (const auto& entry : notes) if (entry.velocity > 0) held[count++] = &entry;
    std::sort(held.begin(), held.begin() + count, [](const HeldNote* first, const HeldNote* second) { return first->order > second->order; });
    const int playerCount = parameters[Ensemble] > 0.5f ? static_cast<int>(parameters[Players]) : 1;
    const int instrument = static_cast<int>(parameters[Instrument]);
    for (size_t index = 0; index < voices.size(); ++index) {
        auto& voice = voices[index];
        if (static_cast<int>(index) >= playerCount || count == 0) { voice.stop(); continue; }
        const auto& assigned = *held[index % std::min(count, static_cast<size_t>(playerCount))];
        voice.pan = playerCount > 1 ? (static_cast<float>(index) / (playerCount - 1) - 0.5f) * 1.5f : 0;
        if (voice.note != assigned.note || voice.channel != assigned.channel || !voice.held())
            voice.start(assigned.note, assigned.channel, assigned.velocity, instrument, parameters[Articulation] == 1);
    }
    playerNormalization = 1 / std::sqrt(static_cast<float>(playerCount));
    updateControls();
}
void Engine::midi(const uint8_t* data, uint32_t size) {
    if (!data || size < 1) return;
    const int status = data[0] & 0xf0, channel = data[0] & 0x0f;
    if (status == 0xd0 && size >= 2) { pressures[channel] = (data[1] & 127) / 127.0f; return; }
    if (size < 3) return;
    const int number = data[1] & 127, value = data[2] & 127;
    if (status == 0x90 && value > 0) noteOn(number, channel, value / 127.0f);
    else if (status == 0x80 || status == 0x90) noteOff(number, channel);
    else if (status == 0xe0) bends[channel] = ((number + (value << 7)) - 8192) / 8192.0f;
    else if (status == 0xb0) {
        if (number == 1) modulations[channel] = value / 127.0f;
        else if (number == 11) expressions[channel] = value / 127.0f;
        else if (number == 64) {
            sustain[channel] = value >= 64;
            if (!sustain[channel]) {
                for (auto& entry : notes) if (entry.channel == channel && !entry.keyDown) entry.velocity = 0;
                redistribute();
            }
        } else if (number == 120 || number == 123) {
            for (auto& entry : notes) if (entry.channel == channel) entry.velocity = 0;
            sustain[channel] = false;
            if (number == 120) for (auto& voice : voices) if (voice.channel == channel) voice.reset();
            redistribute();
        } else if (number == 121) {
            sustain[channel] = false; bends[channel] = modulations[channel] = 0; expressions[channel] = pressures[channel] = 1;
            for (auto& entry : notes) if (entry.channel == channel && !entry.keyDown) entry.velocity = 0;
            redistribute();
        }
    }
}
void Engine::updateControls() {
    for (size_t index = 0; index < Meter; ++index) {
        if (parameterSpecs[index].integer) smoothed[index] = parameters[index];
        else smoothed[index] += 0.05f * (parameters[index] - smoothed[index]);
    }
    outputGain = std::pow(10.0f, smoothed[Gain] / 20.0f) * 0.7f;
    for (auto& voice : voices) if (voice.active()) {
        auto controls = smoothed;
        controls[Vibrato] = std::max(smoothed[Vibrato], modulations[voice.channel]);
        voice.controls(controls, bends[voice.channel], expressions[voice.channel] * (0.5f + 0.5f * pressures[voice.channel]));
    }
}
void Engine::render(float* left, float* right, uint32_t frames) {
    for (uint32_t frame = 0; frame < frames; ++frame) {
        if ((controlCounter++ & 31u) == 0) updateControls();
        float dryLeft = 0, dryRight = 0;
        for (auto& voice : voices) if (voice.active()) {
            const float sample = voice.tick() * playerNormalization;
            dryLeft += sample * std::sqrt((1 - voice.pan) * 0.5f);
            dryRight += sample * std::sqrt((1 + voice.pan) * 0.5f);
        }
        const auto reflections = room.tick(dryLeft, dryRight);
        const float mixedLeft = (dryLeft * (1 - smoothed[Room] * 0.3f) + reflections[0] * smoothed[Room]) * outputGain;
        const float mixedRight = (dryRight * (1 - smoothed[Room] * 0.3f) + reflections[1] * smoothed[Room]) * outputGain;
        left[frame] = mixedLeft / (1 + std::abs(mixedLeft));
        right[frame] = mixedRight / (1 + std::abs(mixedRight));
        meter = std::max(std::max(std::abs(left[frame]), std::abs(right[frame])), meter * 0.9998f);
    }
}

}
