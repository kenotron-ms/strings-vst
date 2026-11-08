import json
from pathlib import Path
import time
import wave

import numpy as np
from pedalboard import load_plugin

from host_smoke import estimate_pitch


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/research-validation"


def write_audio(name, audio, rate):
    path = OUTPUT / f"{name}.wav"
    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(2)
        stream.setsampwidth(2)
        stream.setframerate(rate)
        stream.writeframes((np.clip(audio.T, -1, 1) * 32767).astype("<i2").tobytes())
    return str(path.relative_to(ROOT))


def render(plugin, events, duration=3, rate=48000, transient=False):
    started = time.perf_counter()
    audio = plugin(events, duration=duration, sample_rate=rate, buffer_size=256)
    elapsed = time.perf_counter() - started
    assert np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 1
    audible_window = audio[:, :int(rate * 0.25)] if transient else audio
    assert np.sqrt(np.mean(audible_window ** 2)) > 0.0005, "Inaudible audition"
    assert np.max(np.abs(audio)) > 0.001, "Audition peak too quiet"
    return audio, elapsed


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    plugin = load_plugin(str(ROOT / "build/bin/Strings.vst3"))
    plugin.room = 0
    plugin.vibrato_depth = 0
    plugin.ensemble = 0
    tuning = []
    auditions = []
    registers = [("Violin", (55, 69, 88)), ("Viola", (48, 62, 81)),
                 ("Cello", (36, 50, 69)), ("Double bass", (28, 38, 55))]
    for instrument, notes in registers:
        plugin.instrument = instrument
        for note in notes:
            events = [([0x90, note, 100], 0.125), ([0x80, note, 0], 2)]
            audio, elapsed = render(plugin, events)
            expected = 440 * 2 ** ((note - 69) / 12)
            pitch = float(estimate_pitch(audio[0], 48000, expected))
            cents = float(1200 * np.log2(pitch / expected))
            rms = float(np.sqrt(np.mean(audio[:, 14400:43200] ** 2)))
            tuning.append(dict(instrument=instrument, midi=note, cents=cents, rms=rms))
            assert rms > 0.001, f"{instrument}/{note}: inaudible sustained tone"
            assert abs(cents) < 25, f"{instrument}/{note}: exceeds exploratory register bound: {cents:.2f} cents"
            if note == notes[1]:
                path = write_audio(instrument.lower().replace(" ", "-") + "-solo", audio, 48000)
                auditions.append(dict(path=path, seconds=3, render_seconds=elapsed,
                                      realtime_factor=3 / elapsed))
    plugin.instrument = "Violin"
    plugin.bow_engaged = 1
    plugin.bow_direction = 1
    expression = [([0x90, 69, 100], 0), ([0xB0, 19, 0], 0.65),
                  ([0xB0, 18, 100], 1.1), ([0xB0, 21, 100], 1.3),
                  ([0xB0, 22, 127], 1.45), ([0xB0, 23, 100], 1.6),
                  ([0xB0, 20, 0], 2), ([0xB0, 120, 0], 3)]
    audio, elapsed = render(plugin, expression, duration=4)
    assert np.max(np.abs(audio[:, -24000:])) == 0, "Panic not silent"
    auditions.append(dict(path=write_audio("bow-reversal-contact-lift", audio, 48000),
                          seconds=4, render_seconds=elapsed, realtime_factor=4 / elapsed))
    for articulation in ("Pizzicato", "Tremolo"):
        plugin = load_plugin(str(ROOT / "build/bin/Strings.vst3"))
        plugin.room = 0
        plugin.vibrato_depth = 0
        plugin.articulation = articulation
        plugin.bow_engaged = 1
        plugin.bow_direction = 1
        audio, elapsed = render(plugin, [([0x90, 69, 100], 0), ([0x80, 69, 0], 2)],
                                transient=articulation == "Pizzicato")
        auditions.append(dict(path=write_audio(articulation.lower(), audio, 48000),
                              seconds=3, render_seconds=elapsed, realtime_factor=3 / elapsed))
    plugin = load_plugin(str(ROOT / "build/bin/Strings.vst3"))
    plugin.room = 0
    plugin.articulation = "Arco"
    plugin.vibrato_depth = 0.22
    for count in (1, 8, 16):
        plugin.ensemble = count > 1
        plugin.section_players = max(2, count)
        audio, elapsed = render(plugin, [([0x90, 69, 100], 0), ([0x80, 69, 0], 3)], duration=4)
        auditions.append(dict(path=write_audio(f"section-{count}", audio, 48000),
                              seconds=4, render_seconds=elapsed, realtime_factor=4 / elapsed))
    plugin.body_coupling = 0.713
    plugin.sympathetic_strings = 0.321
    plugin.rosin_grip = 0.654
    plugin.bow_direction = -0.73
    plugin.bow_engaged = 0
    plugin([], duration=0.02, sample_rate=48000)
    saved = plugin.raw_state
    plugin.body_coupling = 0
    plugin.bow_direction = 1
    plugin.bow_engaged = 1
    plugin([], duration=0.02, sample_rate=48000)
    plugin.raw_state = saved
    for name, expected in (("body_coupling", 0.713), ("sympathetic_strings", 0.321),
                           ("rosin_grip", 0.654), ("bow_direction", -0.73), ("bow_engaged", 0)):
        assert abs(float(getattr(plugin, name)) - expected) < 0.002, f"State lost {name}"
    report = dict(status="passed", sample_rate=48000, register_acceptance_cents=25,
                  limitations="Exploratory register check, not precision intonation. Strong bowing can sharpen high notes.",
                  tuning=tuning, auditions=auditions,
                  state="new physical controls restored", hardware="offline host; not an audio device deadline test")
    (OUTPUT / "audio.json").write_text(json.dumps(report, indent=2))
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
