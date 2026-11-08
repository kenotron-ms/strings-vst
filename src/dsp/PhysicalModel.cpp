#include "PhysicalModel.hpp"
#include <algorithm>
#include <cmath>
#include <complex>

namespace strings {
namespace {
constexpr double tau = 6.28318530717958647692;
constexpr int openNotes[4][4] = {{55, 62, 69, 76}, {48, 55, 62, 69}, {36, 43, 50, 57}, {28, 33, 38, 43}};
constexpr float impedances[4][4] = {{0.23f, 0.20f, 0.17f, 0.13f}, {0.36f, 0.29f, 0.24f, 0.20f},
    {0.95f, 0.73f, 0.55f, 0.42f}, {1.8f, 1.5f, 1.2f, 0.95f}};
constexpr double bodyFrequencies[4][8] = {{275, 460, 550, 720, 1100, 1600, 2250, 3100},
    {220, 350, 430, 620, 950, 1350, 1950, 2600}, {95, 170, 240, 350, 520, 850, 1250, 1800},
    {55, 100, 160, 240, 380, 620, 900, 1350}};
float pitch(float note) { return 440.0f * std::exp2((note - 69.0f) / 12.0f); }
}

void BodyMode::configure(double frequency, double quality, double newMass, double angle, double amount, double rate) {
    mass = newMass;
    const double angular = tau * std::min(frequency, rate * 0.35);
    stiffness = mass * angular * angular;
    resistance = mass * angular / quality;
    step = 1.0 / rate;
    denominator = 1 + resistance * step / (2 * mass) + stiffness * step * step / (4 * mass);
    compliance = step / (2 * mass * denominator);
    projection = {amount * std::cos(angle), amount * std::sin(angle)};
}
void BodyMode::clear() { position = velocity = 0; }
double BodyMode::history() const { return (velocity - stiffness * step * position / (2 * mass)) / denominator; }
void BodyMode::advance(double force) {
    const double midpoint = history() + compliance * force;
    position += step * midpoint;
    velocity = 2 * midpoint - velocity;
}
double BodyMode::energy() const { return 0.5 * (mass * velocity * velocity + stiffness * position * position); }

void PassiveBody::prepare(double rate, int instrument, int identity) {
    sampleRate = rate; instrumentId = instrument; playerId = identity;
    coupling = -1; setCoupling(0.6f); reset();
}
void PassiveBody::setCoupling(float amount) {
    amount = std::clamp(amount, 0.0f, 1.0f);
    if (amount == coupling) return;
    coupling = amount;
    constexpr double masses[] = {0.035, 0.055, 0.16, 0.32};
    for (size_t index = 0; index < modes.size(); ++index) {
        const double spread = 1 + 0.0015 * playerId;
        const double weight = std::sqrt(amount) * (index < 3 ? 1.0 : 0.7);
        modes[index].configure(bodyFrequencies[instrumentId][index] * spread, 9 + index * 2,
            masses[instrumentId] * (1 + index * 0.25), 0.27 + index * 1.17, weight, sampleRate);
    }
}
void PassiveBody::reset() {
    for (auto& mode : modes) mode.clear();
    bridgeForce = {}; powerLoss = 0;
}
std::array<float, 2> PassiveBody::scatter(const std::array<double, 2>& incidentForce, double load) {
    double historyX = 0, historyY = 0, admittanceXX = 0, admittanceXY = 0, admittanceYY = 0;
    for (const auto& mode : modes) {
        const double history = mode.history(), axisX = mode.projection[0], axisY = mode.projection[1];
        historyX += axisX * history; historyY += axisY * history;
        admittanceXX += axisX * axisX * mode.compliance;
        admittanceXY += axisX * axisY * mode.compliance;
        admittanceYY += axisY * axisY * mode.compliance;
    }
    const double matrixXX = 1 + load * admittanceXX, matrixYY = 1 + load * admittanceYY;
    const double matrixXY = load * admittanceXY, determinant = matrixXX * matrixYY - matrixXY * matrixXY;
    const double rhsX = historyX + admittanceXX * incidentForce[0] + admittanceXY * incidentForce[1];
    const double rhsY = historyY + admittanceXY * incidentForce[0] + admittanceYY * incidentForce[1];
    const double velocityX = (matrixYY * rhsX - matrixXY * rhsY) / determinant;
    const double velocityY = (matrixXX * rhsY - matrixXY * rhsX) / determinant;
    const double forceX = incidentForce[0] - load * velocityX, forceY = incidentForce[1] - load * velocityY;
    bridgeForce = {static_cast<float>(forceX), static_cast<float>(forceY)};
    powerLoss = 0;
    for (auto& mode : modes) {
        const double force = mode.projection[0] * forceX + mode.projection[1] * forceY;
        const double midpoint = mode.history() + mode.compliance * force;
        powerLoss += mode.resistance * midpoint * midpoint;
        mode.advance(force);
    }
    return {static_cast<float>(velocityX), static_cast<float>(velocityY)};
}
double PassiveBody::energy() const {
    double result = 0;
    for (const auto& mode : modes) result += mode.energy();
    return result;
}
float PassiveBody::reflectionPhaseDelay(float frequency, float impedance, double otherLoad) const {
    using Complex = std::complex<double>;
    const double angular = tau * frequency / sampleRate;
    const double warped = 2 * sampleRate * std::tan(angular * 0.5);
    Complex admittanceXX {}, admittanceXY {}, admittanceYY {};
    for (const auto& mode : modes) {
        const Complex response = Complex(0, warped) / Complex(mode.stiffness - mode.mass * warped * warped, mode.resistance * warped);
        admittanceXX += mode.projection[0] * mode.projection[0] * response;
        admittanceXY += mode.projection[0] * mode.projection[1] * response;
        admittanceYY += mode.projection[1] * mode.projection[1] * response;
    }
    const Complex determinant = (1.0 + otherLoad * admittanceXX) * (1.0 + otherLoad * admittanceYY)
        - otherLoad * otherLoad * admittanceXY * admittanceXY;
    const Complex loadedXX = (admittanceXX + otherLoad * (admittanceXX * admittanceYY - admittanceXY * admittanceXY)) / determinant;
    const Complex loadedYY = (admittanceYY + otherLoad * (admittanceXX * admittanceYY - admittanceXY * admittanceXY)) / determinant;
    const Complex loadedXY = admittanceXY / determinant;
    const Complex projected = 0.9775 * loadedXX + 0.2966058 * loadedXY + 0.0225 * loadedYY;
    const Complex reflection = (1.0 - static_cast<double>(impedance) * projected) / (1.0 + static_cast<double>(impedance) * projected);
    return static_cast<float>(std::arg(reflection) / angular);
}

FrictionResult BowContact::solve(float relativeFreeVelocity, float compliance, float normalForce, float rosin, double rate) {
    normalForce = std::max(0.0f, normalForce);
    const float materialFactor = (0.8f + 0.4f * rosin) / (1 + heat * 0.12f);
    const float staticCoefficient = 1.2f * materialFactor;
    const float dynamicCoefficient = 0.35f * materialFactor;
    const float staticLimit = normalForce * staticCoefficient;
    FrictionResult result;
    if (normalForce > 0 && compliance > 0) {
        const float stickingForce = relativeFreeVelocity / compliance;
        if (std::abs(stickingForce) <= staticLimit) {
            result.force = stickingForce; result.sticking = true;
        } else {
            const float direction = std::copysign(1.0f, relativeFreeVelocity);
            const float relativeSpeed = std::abs(relativeFreeVelocity);
            float lower = normalForce * dynamicCoefficient, upper = staticLimit;
            for (int iteration = 0; iteration < 14; ++iteration) {
                const float candidate = (lower + upper) * 0.5f;
                const float slip = std::max(0.0f, relativeSpeed - compliance * candidate);
                const float friction = normalForce * materialFactor * (0.4f * std::exp(-slip / 0.01f) + 0.45f * std::exp(-slip / 0.1f) + 0.35f);
                if (candidate > friction) upper = candidate; else lower = candidate;
            }
            result.force = direction * (lower + upper) * 0.5f;
        }
    }
    result.slip = relativeFreeVelocity - compliance * result.force;
    const float dissipatedPower = std::max(0.0f, result.force * result.slip);
    heat += static_cast<float>((dissipatedPower * 24 - heat * 3) / rate);
    heat = std::clamp(heat, 0.0f, 10.0f);
    return result;
}

void WaveDelay::prepare(size_t capacity) { samples.assign(capacity + 4, 0); cursor = 0; }
void WaveDelay::reset() { std::fill(samples.begin(), samples.end(), 0); cursor = 0; }
float WaveDelay::read(float delay) const {
    delay = std::clamp(delay, 1.0f, static_cast<float>(samples.size() - 2));
    float position = static_cast<float>(cursor) - delay;
    if (position < 0) position += static_cast<float>(samples.size());
    const auto first = static_cast<size_t>(position), second = (first + 1) % samples.size();
    return samples[first] + (position - static_cast<float>(first)) * (samples[second] - samples[first]);
}
void WaveDelay::write(float value) { samples[cursor] = value; cursor = (cursor + 1) % samples.size(); }

void StringLane::prepare(double rate) {
    sampleRate = rate;
    retuneBlend = static_cast<float>(1 - std::exp(-1 / (rate * 0.000823)));
    const auto capacity = static_cast<size_t>(rate / 15);
    nutLoop.prepare(capacity); bridgeOut.prepare(capacity); bridgeReturn.prepare(capacity);
    reset();
}
void StringLane::reset() {
    nutLoop.reset(); bridgeOut.reset(); bridgeReturn.reset();
    lossState = dispersionState = fromNut = fromBridge = toBridge = 0; tuned = false;
}
void StringLane::tune(float frequency, float position, float brightness, float stiffness, float decay, float bridgePhaseDelay) {
    frequency = std::clamp(frequency, 20.0f, static_cast<float>(sampleRate * 0.12));
    bowPosition = std::clamp(position, 0.04f, 0.32f);
    lossPole = std::pow(0.65f - 0.5f * brightness, static_cast<float>(48000 / sampleRate));
    dispersion = -0.15f - 0.45f * stiffness;
    const double angular = tau * frequency / sampleRate;
    const double lowpassDelay = std::atan2(lossPole * std::sin(angular), 1 - lossPole * std::cos(angular)) / angular;
    const double allpassDelay = -std::atan2(-(1 - dispersion * dispersion) * std::sin(angular),
        2 * dispersion + (1 + dispersion * dispersion) * std::cos(angular)) / angular;
    targetPeriod = std::max(7.0f, static_cast<float>(sampleRate / frequency - lowpassDelay - allpassDelay) + bridgePhaseDelay);
    if (!tuned) { period = targetPeriod; tuned = true; }
    reflection = std::pow(0.001f, 1.0f / (frequency * decay));
}
void StringLane::read(float vibrato) {
    period += retuneBlend * (targetPeriod - period);
    const float actualPeriod = period * vibrato;
    fromNut = nutLoop.read(actualPeriod * (1 - bowPosition));
    fromBridge = bridgeReturn.read(actualPeriod * bowPosition * 0.5f);
    toBridge = bridgeOut.read(actualPeriod * bowPosition * 0.5f);
}
void StringLane::bow(float force) {
    const float injectedVelocity = force / (2 * impedance);
    const float towardsNut = fromBridge + injectedVelocity;
    lossState = (1 - lossPole) * towardsNut + lossPole * lossState;
    const float dispersed = dispersion * lossState + dispersionState;
    dispersionState = lossState - dispersion * dispersed;
    nutLoop.write(-reflection * dispersed);
    bridgeOut.write(fromNut + injectedVelocity);
}
void StringLane::bridge(float velocity) { bridgeReturn.write(velocity - toBridge); }

void PhysicalInstrument::prepare(double rate, int identity) {
    sampleRate = rate; playerId = identity;
    for (auto& string : strings) for (auto& lane : string) lane.prepare(rate);
    configure(0); reset();
}
void PhysicalInstrument::reset() {
    for (auto& string : strings) for (auto& lane : string) lane.reset();
    body.reset(); contact.reset(); observation = {}; pluckTime = dcInput = dcOutput = 0;
    tunedFrequencies.fill(0);
}
void PhysicalInstrument::configure(int instrument) {
    instrumentId = std::clamp(instrument, 0, 3);
    body.prepare(sampleRate, instrumentId, playerId);
    tunedFrequencies.fill(0);
    for (size_t string = 0; string < strings.size(); ++string)
        for (auto& lane : strings[string]) lane.impedance = impedances[instrumentId][string];
}
void PhysicalInstrument::note(float frequency, int midiNote, bool pluck, float velocity) {
    fundamental = frequency; noteVelocity = velocity; pizzicato = pluck;
    playedString = 0;
    for (int string = 0; string < 4; ++string) if (midiNote >= openNotes[instrumentId][string]) playedString = string;
    if (pluck) pluckTime = 0.004f;
}
void PhysicalInstrument::controls(const PhysicalControls& values) {
    const bool retune = values.position != settings.position || values.brightness != settings.brightness
        || values.coupling != settings.coupling || values.sympathetic != settings.sympathetic;
    settings = values; body.setCoupling(settings.coupling);
    for (size_t string = 0; string < strings.size(); ++string) {
        const float frequency = static_cast<int>(string) == playedString ? fundamental : pitch(static_cast<float>(openNotes[instrumentId][string]));
        if (!retune && tunedFrequencies[string] == frequency) continue;
        tunedFrequencies[string] = frequency;
        float otherLoad = 0;
        for (size_t other = 0; other < strings.size(); ++other)
            if (other != string) otherLoad += strings[other][0].impedance * settings.sympathetic * settings.sympathetic;
        const float phaseDelay = body.reflectionPhaseDelay(frequency, strings[string][0].impedance, otherLoad);
        for (size_t polarisation = 0; polarisation < 2; ++polarisation) {
            const float split = polarisation == 0 ? 1 : 1.0008f;
            strings[string][polarisation].tune(frequency * split, 0.055f + settings.position * 0.22f,
                settings.brightness, 0.25f + instrumentId * 0.15f, pizzicato ? 2.5f : 5.0f, phaseDelay);
        }
    }
}
float PhysicalInstrument::tick(float envelope, float vibrato, float bowGesture) {
    constexpr std::array<float, 2> bowAxis {0.988686f, 0.149999f};
    double load = 0;
    std::array<double, 2> incident {};
    float sympatheticMotion = 0;
    for (size_t string = 0; string < strings.size(); ++string) {
        const bool played = static_cast<int>(string) == playedString;
        const float transformer = played ? 1.0f : settings.sympathetic;
        const float impedance = strings[string][0].impedance;
        load += transformer * transformer * impedance;
        for (size_t polarisation = 0; polarisation < 2; ++polarisation) {
            auto& lane = strings[string][polarisation];
            lane.read(played ? vibrato : 1.0f);
            incident[polarisation] += 2 * transformer * impedance * lane.incidentVelocity();
            if (!played) sympatheticMotion += std::abs(lane.incidentVelocity());
        }
    }
    const auto bridgeVelocity = body.scatter(incident, load);
    auto& played = strings[playedString];
    const float impedance = played[0].impedance;
    float freeVelocity = 0, compliance = 0;
    for (size_t polarisation = 0; polarisation < 2; ++polarisation) {
        freeVelocity += bowAxis[polarisation] * played[polarisation].freeVelocity();
        compliance += bowAxis[polarisation] * bowAxis[polarisation] / (2 * impedance);
    }
    const float bowVelocity = settings.direction * settings.speed * 0.45f * bowGesture * envelope;
    const float normalForce = impedance * settings.force * 3.5f * settings.engagement * envelope * (0.4f + 0.6f * noteVelocity);
    const auto friction = contact.solve(bowVelocity - freeVelocity, compliance, pizzicato ? 0 : normalForce, settings.rosin, sampleRate);
    float force = friction.force;
    if (pizzicato && pluckTime > 0) {
        force = impedance * noteVelocity * 0.7f * std::sin(static_cast<float>(tau * 0.5) * pluckTime / 0.004f);
        pluckTime = std::max(0.0f, pluckTime - static_cast<float>(1 / sampleRate));
    }
    for (size_t string = 0; string < strings.size(); ++string) {
        const bool active = static_cast<int>(string) == playedString;
        const float transformer = active ? 1.0f : settings.sympathetic;
        for (size_t polarisation = 0; polarisation < 2; ++polarisation) {
            strings[string][polarisation].bow(active ? force * bowAxis[polarisation] : 0);
            strings[string][polarisation].bridge(transformer * bridgeVelocity[polarisation]);
        }
    }
    const auto bridgeForce = body.force();
    const float scale = impedances[instrumentId][0];
    const float signal = (bridgeForce[0] * 0.16f + bridgeForce[1] * 0.08f + bridgeVelocity[0] * 11 + bridgeVelocity[1] * 7) / std::sqrt(scale);
    const float output = signal - dcInput + static_cast<float>(std::exp(-tau * 8 / sampleRate)) * dcOutput;
    dcInput = signal; dcOutput = output;
    observation = {body.energy(), friction.force * friction.slip, friction.force, friction.slip,
        std::hypot(bridgeVelocity[0], bridgeVelocity[1]), sympatheticMotion, contact.temperatureRise()};
    return output;
}

}
