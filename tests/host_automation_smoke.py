import hashlib
from importlib.metadata import version
import json
from pathlib import Path
import platform
import sys
import wave

import numpy as np
from pedalboard import load_plugin


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/performance-validation"
BASELINE = dict(ensemble=0, articulation="Arco", player_variation=0.0,
                vibrato_depth=0.0, room=0.0, attack=20.0, release=60.0,
                bow_pressure=0.5, bow_speed=0.65, bow_position=0.35,
                bow_direction=1.0, bow_engaged=1.0, output=-6.0)
CHANGES = dict(bow_pressure=0.15, bow_position=0.85, bow_direction=-1.0,
               bow_engaged=0.0, vibrato_depth=0.7)


def rms(audio):
    return float(np.sqrt(np.mean(np.square(audio, dtype=np.float64))))


def render(plugin, instrument, note, sample_rate, change=None):
    initialization = plugin([], duration=0.01, sample_rate=sample_rate, buffer_size=256, reset=True)
    assert np.max(np.abs(initialization)) == 0, "Reset retained audio"
    plugin.instrument = instrument
    for name, value in BASELINE.items():
        setattr(plugin, name, value)
    idle = plugin([], duration=0.5, sample_rate=sample_rate, buffer_size=256, reset=False)
    assert np.max(np.abs(idle)) == 0, "Reset or idle preroll retained audio"
    prefix = plugin([([0x90, note, 100], 0)], duration=0.75,
                    sample_rate=sample_rate, buffer_size=256, reset=False)
    if change is not None:
        setattr(plugin, *change)
    suffix = plugin([], duration=0.75, sample_rate=sample_rate,
                    buffer_size=256, reset=False)
    tail = plugin([([0x80, note, 0], 0)], duration=0.7,
                  sample_rate=sample_rate, buffer_size=256, reset=False)
    audio = np.concatenate((prefix, suffix, tail), axis=1)
    assert np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 1
    assert rms(prefix[:, -sample_rate // 4:]) > 0.0001, "Initial note was silent"
    assert np.max(np.abs(tail[:, -sample_rate // 4:])) < 0.0001, "Note release failed"
    return prefix, suffix, audio


def write_audio(path, audio, sample_rate):
    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(2)
        stream.setsampwidth(2)
        stream.setframerate(sample_rate)
        stream.writeframes((audio.T * 32767).astype("<i2").tobytes())


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    report = OUTPUT / "host-automation.json"
    report.unlink(missing_ok=True)
    plugin_path = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / "build/bin/Strings.vst3"
    if platform.system() == "Windows":
        plugin_path = plugin_path / "Contents/x86_64-win/Strings.vst3"
    plugin = load_plugin(str(plugin_path))
    assert plugin.is_instrument
    measurements = []
    examples = []
    for sample_rate in (44100, 48000, 96000):
        for instrument, note in zip(("Violin", "Viola", "Cello", "Double bass"), (69, 62, 50, 38)):
            original_prefix, original_suffix, original_audio = render(plugin, instrument, note, sample_rate)
            assert rms(original_suffix) > 0.0001, "Host failed to retain a held note between calls"
            for parameter, value in CHANGES.items():
                prefix, suffix, audio = render(plugin, instrument, note, sample_rate, (parameter, value))
                prefix_error = float(np.max(np.abs(prefix - original_prefix)))
                assert prefix_error < 0.000001, f"Renders differ before {parameter}: {instrument}/{sample_rate}: {prefix_error}"
                relative_difference = rms(suffix - original_suffix) / rms(original_suffix)
                assert relative_difference > 0.001, f"No measurable host change: {parameter}"
                assert abs(float(getattr(plugin, parameter)) - value) < 0.002, "Host parameter readback failed"
                measurements.append(dict(instrument=instrument, sample_rate=sample_rate,
                                         parameter=parameter, value=value, prefix_error=prefix_error,
                                         relative_audio_difference=relative_difference,
                                         tail_peak=float(np.max(np.abs(audio[:, -sample_rate // 4:])))))
                if instrument == "Violin" and sample_rate == 48000 and parameter == "bow_engaged":
                    for name, example in (("baseline", original_audio), ("lift", audio)):
                        path = OUTPUT / f"automation-{name}.wav"
                        write_audio(path, example, sample_rate)
                        examples.append(str(path.relative_to(ROOT)))
            for parameter, value in CHANGES.items():
                setattr(plugin, parameter, value)
            plugin([], duration=0.02, sample_rate=sample_rate)
            saved_state = plugin.raw_state
            for parameter in CHANGES:
                setattr(plugin, parameter, BASELINE[parameter])
            plugin([], duration=0.02, sample_rate=sample_rate)
            plugin.raw_state = saved_state
            assert str(plugin.instrument) == instrument
            for parameter, value in CHANGES.items():
                assert abs(float(getattr(plugin, parameter)) - value) < 0.002, f"State failed to recall {parameter}"
    modules = [path for path in plugin_path.rglob("*") if path.suffix in (".so", ".vst3") and path.is_file()] if plugin_path.is_dir() else [plugin_path]
    report.write_text(json.dumps(dict(
        status="passed", host=f"Pedalboard {version('pedalboard')}",
        route="actual VST3 sustained note -> host parameter change -> reset=False continuation -> note off",
        scope="Host parameter updates at render-call boundaries, not sample-accurate DAW automation curves",
        setup="Reset and activate first, then set baseline parameters and settle without another reset; host reinstantiation can otherwise restore stale processor state over queued changes",
        plugin_sha256={str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in modules},
        checks=["five expressive parameters measurably affect held-note audio without retrigger",
                "identical audio before automation", "four instruments at three rates",
                "finite bounded output", "release after automation", "parameter state recall"],
        measurements=measurements, audio_examples=examples), indent=2) + "\n")
    print(f"Passed {len(measurements)} sustained-note automation comparisons: {report}")


if __name__ == "__main__":
    main()
