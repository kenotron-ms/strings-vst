import hashlib
import json
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from standalone_workflow import capture_errors, read_audio


def main():
    folder = Path(sys.argv[1]).resolve()
    session = json.loads((folder / "session.json").read_text())
    assert session["status"] == "passed", "Capture session did not pass"
    assert hashlib.sha256((ROOT / session["binary"]).read_bytes()).hexdigest() == session["standalone_sha256"]
    errors = capture_errors((folder / "jack.log").read_text())
    assert not errors, f"JACK logged {len(errors)} processing errors"
    audio_checks = []
    for measurement in session["measurements"]:
        rate, audio = read_audio(folder / measurement["file"])
        assert rate == 48000 and np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 0.99
        assert np.max(np.abs(audio[:rate // 2])) < 0.0001, "Unexpected audio before scheduled notes"
        assert np.max(np.abs(audio[-rate // 4:])) < 0.0001, "Recorded notes did not release"
        assert measurement["jack_processing_errors"] == 0
        audio_checks.append(dict(name=measurement["name"], seconds=len(audio) / rate,
            peak=float(np.max(np.abs(audio))), rms=float(np.sqrt(np.mean(audio ** 2)))))
    rate, master = read_audio(folder / session["master"])
    assert rate == 48000 and np.max(np.abs(master)) < 0.801
    assert np.max(np.abs(master)) > 0.7 and np.max(np.abs(master[-rate // 4:])) < 0.0001
    reconstructed = np.zeros_like(master)
    for name, gain in (("violin", 0.9), ("viola", 0.5), ("cello", 0.65), ("double-bass", 0.55)):
        _, stem = read_audio(folder / f"{name}-raw.wav")
        reconstructed += stem[:len(master)] * gain
    reconstructed *= 10 ** (session["constant_master_gain_db"] / 20)
    mix_error = float(np.max(np.abs(reconstructed - master)))
    assert mix_error < 2 / 32768, "Master does not match its recorded source stems"
    score = json.loads((folder / "score.json").read_text())
    rate, baseline = read_audio(folder / "violin-baseline-raw.wav")
    _, shaped = read_audio(folder / "violin-raw.wav")
    assert baseline.shape == shaped.shape
    difference = float(np.sqrt(np.mean((baseline - shaped) ** 2)))
    assert difference > 0.001
    mono = baseline.mean(axis=1)
    pitch_checks = []
    for note in score["parts"][0]["notes"]:
        if note["duration"] < 0.45:
            continue
        expected = 440 * 2 ** ((note["midi"] - 69) / 12)
        start = int((1 + note["start"] + 0.14) * rate)
        end = int((1 + note["start"] + note["duration"] - 0.07) * rate)
        segment = mono[start:end:4]
        segment = (segment - segment.mean()) * np.hanning(len(segment))
        size = 1 << (2 * len(segment) - 1).bit_length()
        spectrum = np.fft.rfft(segment, n=size)
        correlation = np.fft.irfft(np.abs(spectrum) ** 2, n=size)[:len(segment)]
        low = int(rate / 4 / expected * 0.95)
        high = int(rate / 4 / expected * 1.05) + 1
        peak = low + int(np.argmax(correlation[low:high]))
        before, center, after = correlation[peak - 1:peak + 2]
        adjustment = 0.5 * (before - after) / (before - 2 * center + after)
        actual = rate / 4 / (peak + adjustment)
        confidence = float(center / correlation[0])
        cents = float(1200 * np.log2(actual / expected))
        if confidence >= 0.7:
            assert abs(cents) < 35, (note["start"], note["midi"], cents)
        pitch_checks.append(dict(start=note["start"], midi=note["midi"], cents=cents,
            normalized_autocorrelation=confidence, accepted_for_tuning=confidence >= 0.7))
    assert sum(check["accepted_for_tuning"] for check in pitch_checks) >= 8
    report = dict(status="passed", audio=audio_checks, master_peak=float(np.max(np.abs(master))),
        jack_processing_errors=len(errors), baseline_shaped_difference_rms=difference,
        reconstructed_master_maximum_error=mix_error,
        audio_sha256={path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in folder.glob("*.wav")},
        melody_pitch=pitch_checks, standalone_sha256=session["standalone_sha256"],
        limitations=["Pitch analysis covers long baseline melody notes near their expected frequency, not full score transcription",
            "RMS difference is not an isolated perceptual/timbre comparison; wall-clock takes have timing variation",
            "Numerical checks cannot establish acoustic realism or replace human listening"])
    (folder / "audio-audit.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"PASS: {len(audio_checks)} raw captures, clean JACK log, nonclipped master and baseline melody checks")


if __name__ == "__main__":
    main()
