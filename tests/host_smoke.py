import json
from pathlib import Path
import sys

import numpy as np
from pedalboard import load_plugin


def estimate_pitch(audio, sample_rate, expected):
    segment = audio[int(sample_rate * 0.25):int(sample_rate * 0.85)]
    spectrum = np.abs(np.fft.rfft(segment * np.hanning(len(segment)), n=262144))
    frequencies = np.fft.rfftfreq(262144, 1 / sample_rate)
    candidates = np.where((frequencies > expected * 0.92) & (frequencies < expected * 1.08))[0]
    peak = candidates[np.argmax(spectrum[candidates])]
    logs = np.log(np.maximum(spectrum[peak - 1:peak + 2], 1e-20))
    adjustment = 0.5 * (logs[0] - logs[2]) / (logs[0] - 2 * logs[1] + logs[2])
    return (peak + adjustment) * sample_rate / 262144


def main():
    plugin = load_plugin(str(Path(sys.argv[1]).resolve()))
    assert plugin.is_instrument
    plugin.vibrato_depth = 0.0
    plugin.room = 0.0
    names = ["Violin", "Viola", "Cello", "Double bass"]
    roots = [69, 62, 50, 38]
    results = []
    for sample_rate in (44100, 48000, 96000):
        for instrument, note in zip(names, roots):
            plugin.instrument = instrument
            plugin.articulation = "Arco"
            plugin.ensemble = 0
            events = [([0x90, note, 100], 0.125), ([0x80, note, 0], 1.0)]
            audio = plugin(events, duration=2, sample_rate=sample_rate, buffer_size=256)
            assert audio.shape == (2, sample_rate * 2)
            assert np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 1
            assert np.max(np.abs(audio[:, :int(sample_rate * 0.125)])) == 0, "MIDI played before its timestamp"
            rms = float(np.sqrt(np.mean(audio[:, int(sample_rate * 0.3):int(sample_rate * 0.9)] ** 2)))
            assert rms > 0.001, f"Silent {instrument}"
            expected = 440 * 2 ** ((note - 69) / 12)
            pitch = float(estimate_pitch(audio[0], sample_rate, expected))
            cents = float(1200 * np.log2(pitch / expected))
            assert abs(cents) < 12, f"{instrument} tuning error: {cents:.2f} cents"
            results.append(dict(instrument=instrument, sample_rate=sample_rate, pitch_hz=pitch, error_cents=cents, rms=rms))
    plugin.instrument = "Violin"
    for articulation in ("Pizzicato", "Tremolo"):
        plugin.articulation = articulation
        audio = plugin([([0x90, 69, 100], 0), ([0x80, 69, 0], 1)], duration=2, sample_rate=48000)
        assert np.max(np.abs(audio)) > 0.001 and np.all(np.isfinite(audio))
    plugin.articulation = "Arco"
    plugin.ensemble = 1
    plugin.section_players = 8
    audio = plugin([([0x90, note, 100], 0) for note in (60, 64, 67)], duration=1, sample_rate=48000)
    assert np.sqrt(np.mean((audio[0] - audio[1]) ** 2)) > 0.0001, "Ensemble stereo collapsed"
    plugin.instrument = "Cello"
    plugin.bow_pressure = 0.713
    plugin([], duration=0.02, sample_rate=48000)
    saved_state = plugin.raw_state
    plugin.instrument = "Violin"
    plugin.bow_pressure = 0.1
    plugin([], duration=0.02, sample_rate=48000)
    plugin.raw_state = saved_state
    assert str(plugin.instrument) == "Cello", "Instrument state did not restore"
    assert abs(float(plugin.bow_pressure) - 0.713) < 0.002, "Parameter state did not restore"
    report = Path("build/host-smoke.json")
    report.write_text(json.dumps(dict(status="passed", measurements=results,
                                     checks=["actual VST3 MIDI rendering", "sample offsets", "tuning", "articulations", "stereo ensemble", "state roundtrip"]), indent=2))
    print(report.read_text())


if __name__ == "__main__":
    main()
