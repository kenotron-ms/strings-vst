#include "dsp/PhysicalModel.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

int main() {
    try {
        double greatestBalanceError = 0, leastDissipation = 0;
        for (double rate : {44100.0, 48000.0, 96000.0}) {
            strings::BodyMode mode;
            mode.configure(550, 20, 0.035, 0, 1, rate);
            for (int sample = 0; sample < 30000; ++sample) {
                const double force = sample < 300 ? std::sin(sample * 0.13) : 0;
                const double before = mode.energy();
                const double midpoint = mode.history() + mode.compliance * force;
                mode.advance(force);
                const double work = (force * midpoint - mode.resistance * midpoint * midpoint) / rate;
                greatestBalanceError = std::max(greatestBalanceError, std::abs(mode.energy() - before - work));
            }
            check(greatestBalanceError < 1e-12, "Modal oscillator violates discrete energy balance");
            strings::PassiveBody body;
            body.prepare(rate, 0, 0);
            for (int sample = 0; sample < 50000; ++sample) {
                const std::array<double, 2> incident {sample < 100 ? 0.2 : 0, sample < 100 ? -0.07 : 0};
                const double before = body.energy();
                const auto velocity = body.scatter(incident, 1.0);
                const auto force = body.force();
                const double work = (force[0] * velocity[0] + force[1] * velocity[1] - body.lastPowerLoss()) / rate;
                check(std::abs(body.energy() - before - work) < 2e-10, "Coupled bridge violates power balance");
                if (sample >= 100) check(body.energy() <= before + 1e-14, "Unforced loaded body gained energy");
            }
            check(body.energy() < 1e-12, "Body failed to decay");
            for (float normal : {0.0f, 0.01f, 0.2f, 2.0f}) {
                strings::BowContact contact;
                for (int sample = 0; sample < 10000; ++sample) {
                    const float relative = std::sin(sample * 0.017f) * 1.5f;
                    const auto result = contact.solve(relative, 2.5f, normal, 0.6f, rate);
                    leastDissipation = std::min(leastDissipation, static_cast<double>(result.force * result.slip));
                    check(std::abs(result.force) <= normal * 1.248f + 1e-6f, "Friction exceeds static bound");
                    check(result.force * result.slip > -1e-6f, "Bow contact creates frictional energy");
                    if (normal == 0) check(result.force == 0, "Lifted bow still applies force");
                }
            }
        }
        for (const double rate : {44100.0, 48000.0, 96000.0}) {
            for (const float compliance : {0.1f, 1.0f, 5.0f}) {
                for (const float normal : {0.01f, 0.1f, 1.0f}) {
                    strings::BowContact contact;
                    for (int index = -100; index <= 100; ++index) {
                        const float relative = index * 0.04f;
                        const float factor = 1.04f / (1 + contact.temperatureRise() * 0.12f);
                        const auto result = contact.solve(relative, compliance, normal, 0.6f, rate);
                        check(std::abs(result.slip - (relative - compliance * result.force)) < 1e-6f, "Junction slip is inconsistent");
                        if (result.sticking) check(std::abs(result.slip) < 1e-6f, "Static contact slides");
                        else {
                            const float speed = std::abs(result.slip);
                            const float expected = normal * factor * (0.4f * std::exp(-speed / 0.01f)
                                + 0.45f * std::exp(-speed / 0.1f) + 0.35f);
                            check(std::abs(std::abs(result.force) - expected) < 0.001f * normal + 1e-6f, "Nonlinear contact does not satisfy friction equation");
                        }
                    }
                }
            }
        }
        for (int instrument = 0; instrument < 4; ++instrument) {
            constexpr int notes[] = {69, 62, 50, 38};
            const float frequency = 440 * std::exp2((notes[instrument] - 69.0f) / 12);
            for (float sympathetic : {0.0f, 1.0f}) {
                strings::PhysicalInstrument model;
                model.prepare(48000, 0); model.configure(instrument);
                model.note(frequency, notes[instrument], false, 0.8f);
                strings::PhysicalControls controls;
                controls.sympathetic = sympathetic;
                model.controls(controls);
                double energy = 0, transferred = 0, sustainedEnergy = 0;
                for (int sample = 0; sample < 96000; ++sample) {
                    if (sample == 48000) { controls.direction = -1; model.controls(controls); }
                    const float output = model.tick(1, 1, 1);
                    check(std::isfinite(output) && std::abs(output) < 10, "Unbounded internal physical output");
                    energy += output * output;
                    if (sample >= 72000) sustainedEnergy += output * output;
                    transferred += model.diagnostics().sympatheticMotion;
                }
                check(energy > 0.01, "Physical instrument is silent");
                check(std::sqrt(sustainedEnergy / 24000) > 0.002, "Bow falls into inaudible steady sliding");
                if (sympathetic == 0) check(transferred == 0, "Disconnected strings receive energy");
                else check(transferred > 0.001, "Bridge does not excite open strings");
                controls.engagement = 0; model.reset(); model.controls(controls);
                for (int sample = 0; sample < 10000; ++sample)
                    check(model.tick(1, 1, 1) == 0, "Lifted bow excites an initially silent string");
                controls.engagement = 1; controls.force = 0; model.controls(controls);
                for (int sample = 0; sample < 10000; ++sample)
                    check(model.tick(1, 1, 1) == 0, "Zero force excites an initially silent string");
                controls.force = 0.5f; controls.speed = 0; model.controls(controls);
                for (int sample = 0; sample < 10000; ++sample)
                    check(model.tick(1, 1, 1) == 0, "Stationary bow excites an initially silent string");
            }
        }
        std::cout << "PASS: oscillator/bridge energy balance at 3 rates; max modal error=" << greatestBalanceError
                  << "; min friction dissipation=" << leastDissipation
                  << "; four-instrument sympathetic transfer, reversals, lifted/zero-force/stationary bow\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
