import hashlib
import json
import math
import os
from pathlib import Path
import subprocess
import time
import wave

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/live-expression-validation"


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / "native.json").unlink(missing_ok=True)
    processes, logs, phases, snapshots = [], [], [], []
    try:
        display = subprocess.Popen(["Xvfb", "-displayfd", "1", "-screen", "0", "1280x900x24"],
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        processes.append(display)
        number = display.stdout.readline().strip()
        assert number
        env = {**os.environ, "DISPLAY": f":{number}", "LIBGL_ALWAYS_SOFTWARE": "1",
               "JACK_DEFAULT_SERVER": f"strings-pad-{os.getpid()}"}
        for name, arguments in (("jack", ["jackd", "--no-realtime", "--name", env["JACK_DEFAULT_SERVER"],
                                          "-d", "dummy", "-r", "48000", "-p", "1024"]),
                                ("standalone", [str(ROOT / "build/bin/Strings")])):
            stream = (OUTPUT / f"{name}.log").open("w")
            logs.append(stream)
            processes.append(subprocess.Popen(arguments, env=env, stdout=stream, stderr=stream))
            time.sleep(2)

        def command(*arguments):
            return subprocess.check_output(arguments, env=env, text=True).strip()

        window = command("xdotool", "search", "--pid", str(processes[-1].pid)).splitlines()[-1]
        command("xdotool", "windowmove", window, "0", "0", "windowfocus", "--sync", window)
        outputs = [port for port in command("jack_lsp").splitlines() if port.startswith("Strings:") and "out" in port.lower()]
        assert len(outputs) == 2
        recorder_log = (OUTPUT / "recorder.log").open("w")
        logs.append(recorder_log)
        recorder = subprocess.Popen(["jack_rec", "-f", str(OUTPUT / "pad-performance.wav"), "-d", "30",
                                     "-b", "16", "-B", "1048576", *outputs],
                                    env=env, stdout=recorder_log, stderr=recorder_log)
        processes.append(recorder)
        origin = time.monotonic()
        time.sleep(0.7)

        def phase(name, audible, duration=0.8):
            start = time.monotonic() - origin
            time.sleep(duration)
            phases.append(dict(name=name, audible=audible, start=start + 0.35, end=time.monotonic() - origin - 0.08))

        def snapshot(name, horizontal, vertical, scale=1):
            path = OUTPUT / f"{name}.png"
            command("import", "-window", window, str(path))
            position_x, position_y = round(horizontal * scale), round(vertical * scale)
            raw = subprocess.check_output(["convert", str(path), "-crop", f"7x7+{position_x - 3}+{position_y - 3}",
                                           "-depth", "8", "rgb:-"], env=env)
            pixels = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3).astype(int)
            assert np.any(np.max(np.abs(pixels - [186, 145, 79]), axis=1) < 12), (name, horizontal, vertical)
            snapshots.append(str(path))

        def knob(name, center, amount):
            angle = 2.3561945 + amount * 4.712389
            snapshot(name, center + math.cos(angle) * 58, 282 + math.sin(angle) * 58)

        phase("idle", False)
        command("xdotool", "keydown", "g")
        phase("keyboard-note", True)
        command("xdotool", "mousemove", "200", "282", "mousedown", "1", "mousemove", "200", "234", "mouseup", "1")
        phase("energy", True)
        knob("energy", 200, 0.8)
        command("xdotool", "mousemove", "520", "282", "mousedown", "1", "mousemove", "520", "330", "mouseup", "1")
        phase("tone", True)
        knob("tone", 520, 0.35)
        command("xdotool", "mousemove", "840", "282", "mousedown", "1", "mousemove", "840", "234", "mouseup", "1")
        phase("vibrato", True)
        knob("vibrato", 840, 0.52)
        command("xdotool", "mousemove", "840", "282", "click", "--repeat", "2", "--delay", "110", "1")
        knob("vibrato-reset", 840, 0.22)
        command("xdotool", "keyup", "g")
        phase("keyboard-release", False, 1.2)
        command("xdotool", "mousemove", "860", "84", "click", "1", "keydown", "g", "mousemove", "800", "260", "mousedown", "1")
        phase("bow-forward", True, 1.1)
        snapshot("bow-forward", 800, 260)
        command("xdotool", "mousemove", "240", "360")
        phase("bow-reverse", True, 1.1)
        snapshot("bow-reverse", 240, 360)
        command("xdotool", "click", "3")
        phase("secondary-button-safety", True)
        command("xdotool", "key", "Escape", "mouseup", "1", "keyup", "g")
        phase("escape-release", False, 1.2)
        command("xdotool", "mousemove", "965", "84", "click", "1", "keydown", "g")
        phase("details-restores-bow-baseline", True, 1.1)
        command("xdotool", "keyup", "g")
        phase("details-release", False, 1.2)
        command("xdotool", "mousemove", "760", "84", "click", "1", "keydown", "g")
        phase("knobs-restore-bow-baseline", True, 1.1)
        command("xdotool", "keyup", "g")
        phase("released", False, 1.2)
        command("xdotool", "mousemove", "860", "84", "click", "1", "windowsize", window, "832", "544")
        time.sleep(0.4)
        command("xdotool", "keydown", "g", "mousemove", "640", "208", "mousedown", "1")
        phase("scaled-bow", True, 1.1)
        snapshot("scaled-bow", 800, 260, 0.8)
        command("xdotool", "mouseup", "1", "keyup", "g")
        phase("final-release", False, 1.2)
        recorder.wait(timeout=40)
        assert recorder.returncode == 0
        recorder_log.flush()
        assert "overruns" not in (OUTPUT / "recorder.log").read_text()
        with wave.open(str(OUTPUT / "pad-performance.wav"), "rb") as recording:
            assert recording.getnchannels() == 2 and recording.getsampwidth() == 2
            rate = recording.getframerate()
            audio = np.frombuffer(recording.readframes(recording.getnframes()), dtype="<i2").astype(float).reshape(-1, 2) / 32768
        assert np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 0.99
        for item in phases:
            segment = audio[int(item["start"] * rate):int(item["end"] * rate)]
            assert len(segment)
            item["rms"] = float(np.sqrt(np.mean(segment ** 2)))
            assert item["rms"] > 0.0005 if item["audible"] else item["rms"] < 0.0001, item
        report = dict(status="passed", standalone_sha256=hashlib.sha256((ROOT / "build/bin/Strings").read_bytes()).hexdigest(),
                      phases=phases, screenshots=snapshots,
                      scope="Native Linux knobs and optional bow view; synthetic X11 input/JACK audio, no hardware controller or DAW")
        (OUTPUT / "native.json").write_text(json.dumps(report, indent=2) + "\n")
        print(json.dumps(report, indent=2))
    finally:
        for process in reversed(processes):
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
        for stream in logs:
            stream.close()


if __name__ == "__main__":
    main()
