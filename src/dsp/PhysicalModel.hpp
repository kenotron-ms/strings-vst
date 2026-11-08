#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace strings {

struct BodyMode {
    void configure(double frequency, double quality, double mass, double angle, double amount, double rate);
    void clear();
    double history() const;
    void advance(double force);
    double energy() const;
    double position = 0, velocity = 0;
    double compliance = 0, denominator = 1, stiffness = 0, mass = 1, resistance = 0, step = 0;
    std::array<double, 2> projection {};
};

class PassiveBody {
public:
    void prepare(double rate, int instrument, int identity);
    void setCoupling(float amount);
    void reset();
    std::array<float, 2> scatter(const std::array<double, 2>& incidentForce, double load);
    double energy() const;
    float reflectionPhaseDelay(float frequency, float impedance, double otherLoad) const;
    double lastPowerLoss() const { return powerLoss; }
    std::array<float, 2> force() const { return bridgeForce; }
private:
    std::array<BodyMode, 8> modes;
    std::array<float, 2> bridgeForce {};
    double sampleRate = 48000, powerLoss = 0;
    int instrumentId = 0, playerId = 0;
    float coupling = -1;
};

struct FrictionResult {
    float force = 0, slip = 0;
    bool sticking = false;
};

class BowContact {
public:
    FrictionResult solve(float relativeFreeVelocity, float compliance, float normalForce, float rosin, double rate);
    void reset() { heat = 0; }
    float temperatureRise() const { return heat; }
private:
    float heat = 0;
};

class WaveDelay {
public:
    void prepare(size_t capacity);
    void reset();
    float read(float delay) const;
    void write(float value);
private:
    std::vector<float> samples;
    size_t cursor = 0;
};

class StringLane {
public:
    void prepare(double rate);
    void reset();
    void tune(float frequency, float position, float brightness, float stiffness, float decay, float bridgePhaseDelay);
    void read(float vibrato);
    void bow(float force);
    void bridge(float velocity);
    float freeVelocity() const { return fromNut + fromBridge; }
    float incidentVelocity() const { return toBridge; }
    float impedance = 0.2f;
private:
    WaveDelay nutLoop, bridgeOut, bridgeReturn;
    double sampleRate = 48000;
    float period = 100, targetPeriod = 100, bowPosition = 0.15f, retuneBlend = 0.025f;
    float lossPole = 0.2f, lossState = 0, reflection = 0.999f;
    float dispersion = -0.3f, dispersionState = 0;
    float fromNut = 0, fromBridge = 0, toBridge = 0;
    bool tuned = false;
};

struct PhysicalControls {
    float force = 0.5f, speed = 0.65f, direction = 1, engagement = 1;
    float position = 0.35f, brightness = 0.55f, coupling = 0.6f;
    float sympathetic = 0.45f, rosin = 0.5f;
};

struct PhysicalDiagnostics {
    double bodyEnergy = 0;
    float frictionPower = 0, bowForce = 0, bowSlip = 0, bridgeVelocity = 0;
    float sympatheticMotion = 0, temperatureRise = 0;
};

class PhysicalInstrument {
public:
    void prepare(double rate, int identity);
    void reset();
    void configure(int instrument);
    void note(float frequency, int midiNote, bool pluck, float velocity);
    void setFrequency(float frequency) { fundamental = frequency; }
    void controls(const PhysicalControls& values);
    float tick(float envelope, float vibrato, float bowGesture);
    PhysicalDiagnostics diagnostics() const { return observation; }
private:
    std::array<std::array<StringLane, 2>, 4> strings;
    PassiveBody body;
    BowContact contact;
    PhysicalControls settings;
    PhysicalDiagnostics observation;
    double sampleRate = 48000;
    int playerId = 0, instrumentId = 0, playedString = 0;
    float fundamental = 220, noteVelocity = 1, pluckTime = 0;
    std::array<float, 4> tunedFrequencies {};
    float dcInput = 0, dcOutput = 0;
    bool pizzicato = false;
};

}
