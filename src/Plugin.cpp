#include "DistrhoPlugin.hpp"
#include "Presets.hpp"
#include "dsp/Engine.hpp"
#include <algorithm>

START_NAMESPACE_DISTRHO

class StringsPlugin final : public Plugin {
public:
    StringsPlugin() : Plugin(strings::ParameterCount, strings::presetCount, 0) { engine.prepare(getSampleRate()); }
protected:
    const char* getLabel() const override { return "Strings"; }
    const char* getDescription() const override { return "Physically modeled bowed and plucked solo and ensemble strings."; }
    const char* getMaker() const override { return "Kenotron"; }
    const char* getHomePage() const override { return "https://github.com/kenotron-ms/strings-vst"; }
    const char* getLicense() const override { return "MIT"; }
    uint32_t getVersion() const override { return d_version(0, 1, 0); }
    int64_t getUniqueId() const override { return d_cconst('K', 'S', 't', 'r'); }
    void initAudioPort(bool input, uint32_t index, AudioPort& port) override {
        Plugin::initAudioPort(input, index, port);
        port.groupId = kPortGroupStereo;
    }
    void initParameter(uint32_t index, Parameter& parameter) override {
        const auto& spec = strings::parameterSpecs[index];
        parameter.name = spec.name; parameter.symbol = spec.symbol; parameter.unit = spec.unit;
        parameter.ranges.min = spec.minimum; parameter.ranges.max = spec.maximum; parameter.ranges.def = spec.initial;
        parameter.hints = index >= strings::Meter ? kParameterIsOutput : kParameterIsAutomatable;
        if (spec.integer) parameter.hints |= kParameterIsInteger;
        if (index == strings::Ensemble) parameter.hints |= kParameterIsBoolean;
        if (index == strings::Instrument || index == strings::Articulation) {
            const uint8_t count = index == strings::Instrument ? 4 : 3;
            parameter.enumValues.count = count;
            parameter.enumValues.restrictedMode = true;
            parameter.enumValues.values = new ParameterEnumerationValue[count];
            for (uint8_t entry = 0; entry < count; ++entry) {
                parameter.enumValues.values[entry].value = entry;
                parameter.enumValues.values[entry].label = index == strings::Instrument ? strings::instrumentNames[entry] : strings::articulationNames[entry];
            }
        }
    }
    float getParameterValue(uint32_t index) const override { return engine.getParameter(index); }
    void setParameterValue(uint32_t index, float value) override { engine.setParameter(index, value); }
    void initProgramName(uint32_t index, String& name) override { name = strings::presetNames[index]; }
    void loadProgram(uint32_t index) override {
        engine.reset();
        strings::applyPreset(index, [this](uint32_t parameter, float value) { engine.setParameter(parameter, value); });
    }
    void sampleRateChanged(double sampleRate) override { engine.prepare(sampleRate); }
    void activate() override { engine.reset(); }
    void run(const float**, float** outputs, uint32_t frames, const MidiEvent* events, uint32_t count) override {
        uint32_t cursor = 0;
        for (uint32_t index = 0; index < count; ++index) {
            const auto& event = events[index];
            const uint32_t offset = std::clamp(event.frame, cursor, frames);
            engine.render(outputs[0] + cursor, outputs[1] + cursor, offset - cursor);
            const uint8_t* bytes = event.size > MidiEvent::kDataSize ? event.dataExt : event.data;
            engine.midi(bytes, event.size);
            cursor = offset;
        }
        engine.render(outputs[0] + cursor, outputs[1] + cursor, frames - cursor);
    }
private:
    strings::Engine engine;
};

Plugin* createPlugin() { return new StringsPlugin; }

END_NAMESPACE_DISTRHO
