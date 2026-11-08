from collections import Counter
import hashlib
import json
from pathlib import Path
import sys
import wave

import mido
import numpy as np


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_audio(path):
    with wave.open(str(path), "rb") as source:
        assert source.getnchannels() == 2 and source.getsampwidth() == 2
        assert source.getframerate() == 48000
        audio = np.frombuffer(source.readframes(source.getnframes()), dtype="<i2").reshape(-1, 2).T.astype(np.float64) / 32767
    assert np.all(np.isfinite(audio))
    assert np.max(np.abs(audio)) < 0.999
    assert np.max(np.abs(audio[:, :9600])) == 0
    assert np.max(np.abs(audio[:, -12000:])) < 0.0001
    assert float(np.sqrt(np.mean(audio ** 2))) > 0.0001
    return audio


def midi_events(path):
    midi = mido.MidiFile(path)
    events = Counter()
    tempos = []
    for track in midi.tracks:
        tick = 0
        for message in track:
            tick += message.time
            if message.type == "set_tempo":
                tempos.append((tick, message.tempo))
            elif message.type == "note_on" and message.velocity > 0:
                events["on", tick, message.note, message.velocity] += 1
            elif message.type == "note_off" or (message.type == "note_on" and message.velocity == 0):
                events["off", tick, message.note] += 1
    return midi, events, sorted(tempos)


def main():
    directory = Path(sys.argv[1]).resolve()
    report = json.loads((directory / "render.json").read_text())
    assert report["status"] == "passed", "Render did not finish"
    plan = report["plan"]
    source = Path(plan["source"])
    assert digest(source) == plan["source_sha256"]
    arranged = directory / "hes-a-pirate-strings.mid"
    assert digest(arranged) == report["arranged_midi_sha256"]
    original, source_events, source_tempos = midi_events(source)
    new, arranged_events, arranged_tempos = midi_events(arranged)
    assert source_events == arranged_events, "Arrangement changed MIDI note events"
    assert source_tempos == arranged_tempos, "Arrangement changed tempo events"
    assert original.ticks_per_beat == new.ticks_per_beat
    assert sum(count for event, count in source_events.items() if event[0] == "on") == plan["source_notes"]
    assert len(source_tempos) == plan["tempo_events"]
    for filename, expected in report["plugin_sha256"].items():
        assert digest(Path(filename)) == expected, "VST3 changed since rendering"
    master_path = directory / report["master"]
    assert digest(master_path) == report["master_sha256"]
    master = read_audio(master_path)
    assert master.shape[1] == round(report["duration_seconds"] * 48000)
    assert abs(float(np.max(np.abs(master))) - 0.82) < 2 / 32767
    mixed = np.zeros_like(master)
    measurements = []
    for stem, part in zip(report["stems"], plan["parts"]):
        path = directory / stem["file"]
        assert digest(path) == stem["sha256"]
        audio = read_audio(path)
        assert audio.shape == master.shape
        assert part["max_polyphony"] <= stem["players"] <= 16
        mixed += audio * stem["mix_level"]
        active_windows = 0
        for start in range(0, audio.shape[1] - 48000, 48000):
            if np.sqrt(np.mean(audio[:, start:start + 48000] ** 2)) > 0.0001:
                active_windows += 1
        assert active_windows > 10, "Stem contains only a short fragment"
        measurements.append(dict(profile=stem["profile"], notes=part["notes"],
                                 peak=float(np.max(np.abs(audio))), rms=float(np.sqrt(np.mean(audio ** 2))),
                                 audible_one_second_windows=active_windows))
    assert len(measurements) == 4
    gain = 10 ** (report["master_gain_db"] / 20)
    error = float(np.max(np.abs(master - mixed * gain)))
    quantization_bound = (sum(stem["mix_level"] for stem in report["stems"]) * gain + 1.1) / 32767
    assert error <= quantization_bound, "Master cannot be reconstructed from the declared VST3 stems"
    output = dict(status="passed", source_note_count=plan["source_notes"], tempo_events=len(source_tempos),
                  original_midi_seconds=original.length, master_seconds=master.shape[1] / 48000,
                  master_peak=float(np.max(np.abs(master))), mix_reconstruction_error=error,
                  quantization_error_bound=quantization_bound, stems=measurements,
                  plugin_sha256=report["plugin_sha256"], master_sha256=digest(master_path),
                  checks=["Original and orchestrated MIDI have identical note-on/off tick/pitch/velocity multisets",
                          "All original tempo events and PPQ retained",
                          "Four stereo nonclipped VST3 stems and master, silent pre-roll and final release",
                          "All stems cover more than ten audible one-second windows",
                          "Constant-gain mix reconstructs within PCM quantization bound"],
                  limits=["This does not transcribe the rendered audio or prove every note perceptually audible",
                          "No human listening, acoustic realism or live-device claim"])
    (directory / "audio-audit.json").write_text(json.dumps(output, indent=2) + "\n")
    print(f"PASS: {plan['source_notes']} note pairs, {len(source_tempos)} tempos, four real VST3 stems and reconstructed master")


if __name__ == "__main__":
    main()
