import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import time
import wave

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
TEMPO = 126
EIGHTH = 30 / TEMPO
OFFSETS = dict(zip("asdfghjk", (0, 2, 4, 5, 7, 9, 11, 12)))
MELODY = (
    (("d", 1), ("g", 1), ("h", 1), ("j", 2), ("g", 1)),
    (("h", 1.5), ("g", 0.5), ("d", 1), ("s", 1), ("d", 2)),
    (("g", 1), ("j", 1), ("k", 1), ("j", 1), ("h", 2)),
    (("g", 1.5), ("h", 0.5), ("d", 2), ("s", 1), ("d", 1)),
    (("h", 2), ("k", 1), ("j", 1), ("h", 1), ("g", 1)),
    (("k", 1), ("h", 1), ("g", 2), ("d", 1), ("s", 1)),
    (("j", 1), ("g", 0.5), ("h", 0.5), ("j", 1), ("s", 1), ("d", 2)),
    (("s", 1), ("g", 1), ("d", 3), (None, 1)),
    (("d", 0.5), ("g", 0.5), ("j", 1), ("h", 1), ("g", 1), ("d", 2)),
    (("g", 1), ("h", 1), ("j", 1.5), ("h", 0.5), ("s", 2)),
    (("h", 1), ("k", 2), ("j", 1), ("g", 1), ("h", 1)),
    (("g", 1), ("d", 1), ("s", 1), ("d", 2), (None, 1)),
    (("k", 1.5), ("j", 0.5), ("h", 1), ("g", 1), ("h", 2)),
    (("j", 1), ("h", 1), ("g", 1), ("s", 1), ("d", 2)),
    (("s", 0.5), ("d", 0.5), ("g", 1), ("j", 1), ("h", 1), ("g", 2)),
    (("s", 1), ("g", 1), ("d", 3), (None, 1)),
)
HARMONY = ("E", "E", "A", "A", "E", "E", "B", "B", "A", "A", "E", "E", "B", "B", "E", "E")
DURATION = len(MELODY) * 6 * EIGHTH


def score():
    melody = []
    for bar, notes in enumerate(MELODY):
        assert sum(length for _, length in notes) == 6
        position = bar * 6
        for key, length in notes:
            if key is not None:
                melody.append(dict(key=key, start=position * EIGHTH, duration=length * EIGHTH + 0.018))
            position += length
    for index, note in enumerate(melody):
        if index + 1 < len(melody) and melody[index + 1]["key"] == note["key"]:
            note["duration"] = min(note["duration"], melody[index + 1]["start"] - note["start"] - 0.03)
        note["duration"] = min(note["duration"], DURATION - note["start"])
    parts = [dict(name="violin", base=60, notes=melody)]
    for name, base in (("viola", 48), ("cello", 36), ("double-bass", 28)):
        notes = []
        for bar, harmony in enumerate(HARMONY):
            if name == "viola":
                keys = dict(E="dgj", A="hk", B="sj")[harmony]
                notes.extend(dict(key=key, start=bar * 6 * EIGHTH, duration=5.6 * EIGHTH) for key in keys)
            else:
                key = (dict(E="d", A="h", B="j") if name == "cello" else dict(E="a", A="f", B="g"))[harmony]
                for offset in (0, 3):
                    notes.append(dict(key=key, start=(bar * 6 + offset) * EIGHTH, duration=2.6 * EIGHTH))
        parts.append(dict(name=name, base=base, notes=notes))
    for part in parts:
        for note in part["notes"]:
            note["midi"] = part["base"] + OFFSETS[note["key"]]
            assert 0 <= note["start"] < DURATION and 0 < note["duration"] <= DURATION - note["start"] + 1e-6
    return parts


def variable_length(value):
    result = bytearray([value & 127])
    while value >> 7:
        value >>= 7
        result.insert(0, 128 | (value & 127))
    return bytes(result)


def write_midi(path, parts):
    tracks = []
    for channel, part in enumerate(parts):
        events = []
        for note in part["notes"]:
            start = round(note["start"] * TEMPO / 60 * 480)
            end = round((note["start"] + note["duration"]) * TEMPO / 60 * 480)
            events.extend(((start, bytes([0x90 | channel, note["midi"], 100])), (end, bytes([0x80 | channel, note["midi"], 0]))))
        title = part["name"].encode()
        data = bytearray(b"\x00\xff\x03" + variable_length(len(title)) + title)
        if not channel:
            data.extend(b"\x00\xff\x51\x03" + round(60000000 / TEMPO).to_bytes(3, "big"))
            data.extend(b"\x00\xff\x58\x04\x06\x03\x18\x08")
        previous = 0
        for tick, message in sorted(events, key=lambda event: (event[0], event[1][0])):
            data.extend(variable_length(tick - previous) + message)
            previous = tick
        data.extend(b"\x00\xff\x2f\x00")
        tracks.append(b"MTrk" + struct.pack(">I", len(data)) + data)
    path.write_bytes(b"MThd" + struct.pack(">IHHH", 6, 1, len(tracks), 480) + b"".join(tracks))


def read_audio(path):
    with wave.open(str(path), "rb") as stream:
        assert stream.getsampwidth() == 2 and stream.getnchannels() == 2
        rate = stream.getframerate()
        audio = np.frombuffer(stream.readframes(stream.getnframes()), dtype="<i2").reshape(-1, 2).astype(np.float64) / 32768
    return rate, audio


def write_audio(path, audio, rate):
    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(2)
        stream.setsampwidth(2)
        stream.setframerate(rate)
        stream.writeframes((np.clip(audio, -1, 1) * 32767).astype("<i2").tobytes())


def capture_errors(text):
    return [line for line in text.splitlines() if "JackEngine::XRun:" in line or "Process error" in line]


def main():
    parser = argparse.ArgumentParser(description="Record an original cue through the actual standalone keyboard/knob UI")
    parser.add_argument("--output", type=Path, default=ROOT / "build/standalone-workflow")
    parser.add_argument("--plan-only", action="store_true")
    parser.add_argument("--period", type=int, choices=(1024, 2048, 4096), default=2048)
    arguments = parser.parse_args()
    output = arguments.output.resolve()
    if (output / "session.json").exists():
        raise FileExistsError("Choose a new output folder to preserve the previous session")
    output.mkdir(parents=True, exist_ok=True)
    screenshots = output / "screenshots"
    screenshots.mkdir(exist_ok=True)
    parts = score()
    (output / "score.json").write_text(json.dumps(dict(title="Harbour Run", original=True, tempo=TEMPO,
        meter="6/8", seconds=DURATION, parts=parts), indent=2) + "\n")
    write_midi(output / "harbour-run.mid", parts)
    if arguments.plan_only:
        print(output / "score.json")
        return
    processes, logs, captures, measurements = [], [], [], []
    binary = ROOT / "build/bin/Strings"
    fingerprint = hashlib.sha256(binary.read_bytes()).hexdigest()
    try:
        display = subprocess.Popen(["Xvfb", "-displayfd", "1", "-screen", "0", "1280x900x24"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        processes.append(display)
        number = display.stdout.readline().strip()
        assert number
        env = dict(os.environ, DISPLAY=f":{number}", LIBGL_ALWAYS_SOFTWARE="1", JACK_DEFAULT_SERVER=f"strings-cue-{os.getpid()}")
        for name, command in (("jack", ["jackd", "--no-realtime", "--name", env["JACK_DEFAULT_SERVER"], "-d", "dummy", "-r", "48000", "-p", str(arguments.period)]),
                              ("standalone", [str(binary)])):
            log = (output / f"{name}.log").open("w")
            logs.append(log)
            processes.append(subprocess.Popen(command, env=env, stdout=log, stderr=log))
            time.sleep(2)

        def command(*items):
            return subprocess.check_output(items, env=env, text=True).strip()

        def pointer(*items):
            command("xdotool", *map(str, items))

        window = command("xdotool", "search", "--pid", str(processes[-1].pid)).splitlines()[-1]
        pointer("windowmove", window, 0, 0, "windowfocus", "--sync", window)

        def capture(name, asynchronous=False):
            path = screenshots / f"{name}.png"
            if asynchronous:
                processes.append(subprocess.Popen(["import", "-window", window, str(path)], env=env,
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
            else:
                time.sleep(0.08)
                command("import", "-window", window, str(path))
            captures.append(str(path.relative_to(output)))

        capture("01-components-ready")
        pointer("mousemove", 370, 491, "mousedown", 1, "mousemove", 420, 471, "mouseup", 1)
        capture("02-local-note-revision")
        pointer("mousemove", 978, 39, "click", 1, "mousemove", 500, 590, "mousedown", 1, "mousemove", 650, 575, "mouseup", 1)
        capture("03-weight-lane-revision")
        pointer("mousemove", 978, 39, "click", 1, "mousemove", 817, 39, "click", 1)
        pointer("mousemove", 372, 39, "click", 1)
        pointer("mousemove", 944, 679, "click", 1, "mousemove", 665, 555, "mousedown", 1,
            "mousemove", 665, 601, "mouseup", 1)
        capture("04a-four-player-sections")
        pointer("mousemove", 944, 679, "click", 1)
        capture("04-ensemble-ready")
        outputs = [port for port in command("jack_lsp").splitlines() if port.startswith("Strings:") and "out" in port.lower()]
        assert len(outputs) == 2
        takes = []
        current = [0.5, 0.65, 0.22]

        def knob(index, value):
            horizontal = (145, 350, 555)[index]
            ending = round(263 - (value - current[index]) * 160)
            pointer("mousemove", horizontal, 263, "mousedown", 1, "mousemove", horizontal, ending, "mouseup", 1)
            current[index] = min(1, max(0, current[index] + (263 - ending) / 160))

        for pass_index, part in enumerate([parts[0], *parts]):
            name = "violin-baseline" if pass_index == 0 else part["name"]
            if pass_index > 1:
                pointer("mousemove", 240, 39, "click", 1)
            for index, value in enumerate((0.5, 0.65, 0.22)):
                knob(index, value)
            path = output / f"{name}-raw.wav"
            log = (output / f"{name}-recorder.log").open("w")
            logs.append(log)
            jack_offset = (output / "jack.log").stat().st_size
            recorder = subprocess.Popen(["jack_rec", "-f", str(path), "-d", str(math.ceil(DURATION + 3)),
                "-b", "16", "-B", "4194304", *outputs], env=env, stdout=log, stderr=log)
            processes.append(recorder)
            recording_started = time.monotonic()
            origin = recording_started + 1.0
            events = []
            for note in part["notes"]:
                events.extend(((note["start"], "keydown", note["key"]), (note["start"] + note["duration"], "keyup", note["key"])))
            if pass_index:
                for bar in range(len(MELODY)):
                    energy = (0.44 if part["name"] in ("viola", "double-bass") else 0.5) + 0.17 * math.sin(math.pi * (bar % 8) / 7)
                    events.append((bar * 6 * EIGHTH + 0.11, "energy", energy))
                events.extend(((6 * EIGHTH, "tone", 0.42), (48 * EIGHTH, "tone", 0.7),
                               (72 * EIGHTH, "vibrato", 0.38)))
            events.append((12 * EIGHTH + 0.45, "screenshot", f"{pass_index + 5:02d}-{name}-performance"))
            lateness = []
            for when, action, value in sorted(events, key=lambda item: item[0]):
                time.sleep(max(0, origin + when - time.monotonic()))
                lateness.append(max(0, time.monotonic() - origin - when))
                if action in ("keydown", "keyup"):
                    pointer(action, value)
                elif action == "screenshot":
                    capture(value, asynchronous=True)
                else:
                    knob(dict(energy=0, tone=1, vibrato=2)[action], value)
            pointer("key", "Escape")
            recorder.wait(timeout=8)
            assert recorder.returncode == 0
            log.flush()
            status = (output / f"{name}-recorder.log").read_text()
            assert "overruns" not in status.lower() and "failed" not in status.lower(), status
            jack_errors = capture_errors((output / "jack.log").read_text()[jack_offset:])
            if jack_errors:
                (output / "session.json").write_text(json.dumps(dict(status="failed", failed_take=name,
                    reason="JACK missed processing deadlines during capture", error_count=len(jack_errors),
                    error_examples=jack_errors[:5], screenshots=captures), indent=2) + "\n")
                raise RuntimeError(f"JACK capture failed for {name}: {len(jack_errors)} processing errors")
            rate, audio = read_audio(path)
            assert rate == 48000 and np.all(np.isfinite(audio))
            peak = float(np.max(np.abs(audio)))
            rms = float(np.sqrt(np.mean(audio ** 2)))
            tail = float(np.max(np.abs(audio[-rate // 4:])))
            assert 0.001 < peak < 0.99 and rms > 0.0001 and tail < 0.0001, (name, peak, rms, tail)
            takes.append((name, audio))
            measurements.append(dict(name=name, peak=peak, rms=rms, final_quarter_second_peak=tail,
                scheduled_note_count=len(part["notes"]), max_event_lateness_seconds=max(lateness),
                jack_processing_errors=len(jack_errors),
                source="actual standalone keyboard/knob UI -> JACK -> jack_rec", file=path.name))
            print(f"Captured {name}: peak={peak:.4f}, rms={rms:.4f}, late={max(lateness):.3f}s", flush=True)
        length = min(len(audio) for _, audio in takes[1:])
        mix = sum(audio[:length] * gain for (_, audio), gain in zip(takes[1:], (0.9, 0.5, 0.65, 0.55)))
        peak = float(np.max(np.abs(mix)))
        gain = 0.8 / peak
        mix *= gain
        master = output / "harbour-run-strings.wav"
        write_audio(master, mix, 48000)
        capture("10-final-standalone-state")
        for filename in captures:
            with (output / filename).open("rb") as image:
                assert image.read(8) == b"\x89PNG\r\n\x1a\n", filename
        report = dict(status="passed", title="Harbour Run", original=True, not_the_film_theme=True,
            tempo=TEMPO, meter="6/8", musical_seconds=DURATION, standalone_sha256=fingerprint,
            section_players=4, jack_period=arguments.period,
            binary=str(binary.relative_to(ROOT)), measurements=measurements, screenshots=captures,
            master=master.name, midi="harbour-run.mid", constant_master_gain_db=20 * math.log10(gain),
            limitations=["Synthetic X11 keys and knob drags; no physical keyboard/controller or human listening study",
                "Standalone sibling shares DSP/UI with VST3; this recording does not itself load the VST3 bundle",
                "External recorder and fixed MIDI script, not native WAV export or MIDI import",
                "Wall-clock performance scheduling and recorder lead-in, not sample-accurate DAW track alignment",
                "Original cue, not a reproduction or arrangement of the requested film theme"])
        (output / "session.json").write_text(json.dumps(report, indent=2) + "\n")
        print(master, flush=True)
    finally:
        for process in reversed(processes):
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
        for log in logs:
            log.close()


if __name__ == "__main__":
    main()
