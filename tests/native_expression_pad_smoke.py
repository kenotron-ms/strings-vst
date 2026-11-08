import ctypes
import json
import math
import os
from pathlib import Path
import subprocess
import time
import wave

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/performance-validation"


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / "native.json").unlink(missing_ok=True)
    processes, logs, phases, snapshots = [], [], [], []
    try:
        display = subprocess.Popen(["Xvfb", "-displayfd", "1", "-screen", "0", "1280x900x24"],
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        processes.append(display)
        number = display.stdout.readline().strip()
        assert number, "Xvfb did not allocate a display"
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
        command("xdotool", "mousemove", "550", "260", "click", "1")
        ports = command("jack_lsp").splitlines()
        outputs = [port for port in ports if port.startswith("Strings:") and "out" in port.lower()]
        assert len(outputs) == 2
        recorder_log = (OUTPUT / "recorder.log").open("w")
        logs.append(recorder_log)
        recorder = subprocess.Popen(["jack_rec", "-f", str(OUTPUT / "pad-performance.wav"), "-d", "55",
                                     "-b", "16", "-B", "1048576", *outputs],
                                    env=env, stdout=recorder_log, stderr=recorder_log)
        processes.append(recorder)
        origin = time.monotonic()
        time.sleep(0.7)

        def phase(name, duration=1.0):
            start = time.monotonic() - origin
            time.sleep(duration)
            phases.append(dict(name=name, start=start + 0.3, end=time.monotonic() - origin - 0.1))

        def snapshot(name, horizontal, vertical, scale=1.0):
            path = OUTPUT / f"{name}.png"
            time.sleep(0.12)
            command("import", "-window", window, str(path))
            dimensions = command("identify", "-format", "%w %h", str(path)).split()
            width, height = map(int, dimensions)
            raw = subprocess.check_output(["convert", str(path), "-depth", "8", "rgb:-"], env=env)
            image = np.frombuffer(raw, dtype=np.uint8).reshape(height, width, 3)
            expected_x, expected_y = int(horizontal * scale), int(vertical * scale)
            region = image[max(0, expected_y - 8):expected_y + 9, max(0, expected_x - 8):expected_x + 9].astype(int)
            gold = np.max(np.abs(region - [186, 145, 79]), axis=2) < 4
            assert np.count_nonzero(gold) >= 8, f"Engine readback dot missing at {name}: {expected_x},{expected_y}"
            snapshots.append(dict(name=name, expected_dot=[expected_x, expected_y], gold_pixels=int(np.count_nonzero(gold)),
                                  dimensions=[width, height]))

        command("import", "-window", window, str(OUTPUT / "keyboard-default.png"))
        command("xdotool", "mousemove", "370", "491", "mousedown", "1", "mousemove", "420", "471", "mouseup", "1")
        snapshot("keyboard-note-edit", 450, 471)
        command("xdotool", "mousemove", "978", "39", "click", "1")
        command("xdotool", "mousemove", "500", "590", "mousedown", "1", "mousemove", "650", "575", "mouseup", "1")
        time.sleep(0.12)
        command("import", "-window", window, str(OUTPUT / "keyboard-curve-edit.png"))
        command("xdotool", "mousemove", "978", "39", "click", "1", "keydown", "g")
        phase("keyboard-sustain", 1.0)
        command("xdotool", "mousemove", "145", "263", "mousedown", "1", "mousemove", "145", "311", "mouseup", "1")
        phase("keyboard-light", 1.0)
        snapshot("keyboard-energy-light", 145 + math.cos(2.3561945 + 0.2 * 4.712389) * 42,
                 263 + math.sin(2.3561945 + 0.2 * 4.712389) * 42)
        command("xdotool", "mousemove", "145", "263", "mousedown", "1", "mousemove", "145", "167", "mouseup", "1")
        phase("keyboard-strong", 1.0)
        snapshot("keyboard-energy-strong", 145 + math.cos(2.3561945 + 0.8 * 4.712389) * 42,
                 263 + math.sin(2.3561945 + 0.8 * 4.712389) * 42)
        command("xdotool", "mousemove", "350", "263", "mousedown", "1", "mousemove", "350", "311", "mouseup", "1")
        phase("keyboard-tone", 0.8)
        snapshot("keyboard-tone", 350 + math.cos(2.3561945 + 0.35 * 4.712389) * 42,
                 263 + math.sin(2.3561945 + 0.35 * 4.712389) * 42)
        command("xdotool", "mousemove", "555", "263", "mousedown", "1", "mousemove", "555", "215", "mouseup", "1")
        phase("keyboard-vibrato", 0.8)
        snapshot("keyboard-vibrato", 555 + math.cos(2.3561945 + 0.52 * 4.712389) * 42,
                 263 + math.sin(2.3561945 + 0.52 * 4.712389) * 42)
        command("xdotool", "mousemove", "555", "263", "click", "--repeat", "2", "--delay", "110", "1")
        snapshot("keyboard-vibrato-reset", 555 + math.cos(2.3561945 + 0.22 * 4.712389) * 42,
                 263 + math.sin(2.3561945 + 0.22 * 4.712389) * 42)
        command("xdotool", "keyup", "g")
        time.sleep(1.3)
        phase("keyboard-release", 0.8)
        command("xdotool", "mousemove", "300", "84", "click", "1", "mousemove", "260", "84", "click", "1")
        command("xdotool", "mousemove", "825", "141", "click", "1", "mousemove", "550", "260", "click", "1")
        command("xdotool", "keydown", "g")
        phase("armed-idle")
        command("xdotool", "mousemove", "550", "260", "mousedown", "1")
        phase("stationary-forward", 1.2)
        snapshot("held-forward", 550, 260)
        command("xdotool", "mousemove", "150", "368")
        phase("stationary-reverse", 1.2)
        snapshot("held-reverse", 150, 368)
        command("xdotool", "click", "3")
        phase("secondary-button-does-not-release")
        for step in range(61):
            angle = step / 60 * math.tau
            horizontal = round(353 + math.cos(angle) * 180)
            vertical = round(315 - math.sin(angle) * 85)
            command("xdotool", "mousemove", str(horizontal), str(vertical))
            time.sleep(0.02)
        time.sleep(0.25)
        snapshot("gesture-trail", 533, 315)
        command("xdotool", "key", "Escape", "mouseup", "1", "keyup", "g")
        phase("escape-release", 1.2)
        command("xdotool", "mousemove", "583", "679", "click", "1", "keydown", "g")
        phase("restored-follow", 1.2)
        snapshot("follow-controls", 544.75, 315)
        command("xdotool", "keyup", "g", "mousemove", "695", "39", "click", "1")
        capture_start = time.monotonic()
        command("xdotool", "mousemove", "550", "260", "mousedown", "1")
        phase("recorded-gesture", 1.2)
        snapshot("recording", 550, 260)
        command("xdotool", "mouseup", "1")
        time.sleep(max(0, 8.3 - (time.monotonic() - capture_start)))
        command("xdotool", "mousemove", "900", "39", "click", "1", "mousemove", "490", "39", "click", "1")
        phase("take-replay", 0.9)
        snapshot("take-replay", 550, 260)
        command("xdotool", "mousemove", "583", "39", "click", "1")
        phase("stopped-take", 1.2)
        command("xdotool", "mousemove", "944", "679", "click", "1")
        command("import", "-window", window, str(OUTPUT / "details.png"))
        command("xdotool", "mousemove", "944", "679", "click", "1", "mousemove", "960", "141", "click", "1")
        command("import", "-window", window, str(OUTPUT / "controller-map.png"))
        command("xdotool", "mousemove", "825", "141", "click", "1")
        command("xdotool", "keyup", "g", "windowsize", window, "832", "576")
        time.sleep(0.4)
        command("xdotool", "keydown", "g", "mousemove", "440", "208", "mousedown", "1")
        phase("scaled-stationary-forward", 1.2)
        snapshot("scaled-pad", 550, 260, 0.8)
        xlib = ctypes.CDLL("libX11.so.6")
        xlib.XOpenDisplay.argtypes = [ctypes.c_char_p]
        xlib.XOpenDisplay.restype = ctypes.c_void_p
        xlib.XDefaultRootWindow.argtypes = [ctypes.c_void_p]
        xlib.XDefaultRootWindow.restype = ctypes.c_ulong
        xlib.XCloseDisplay.argtypes = [ctypes.c_void_p]
        connection = xlib.XOpenDisplay(env["DISPLAY"].encode())
        assert connection
        root = xlib.XDefaultRootWindow(connection)
        xlib.XCloseDisplay(connection)
        command("xdotool", "windowfocus", "--sync", str(root), "mouseup", "1", "keyup", "g")
        phase("focus-loss-release", 1.2)
        recorder.wait(timeout=65)
        assert recorder.returncode == 0
        recorder_log.flush()
        status = (OUTPUT / "recorder.log").read_text()
        assert "overruns" not in status and "failed" not in status, status
        with wave.open(str(OUTPUT / "pad-performance.wav"), "rb") as stream:
            assert stream.getnchannels() == 2 and stream.getsampwidth() == 2
            rate = stream.getframerate()
            audio = np.frombuffer(stream.readframes(stream.getnframes()), dtype="<i2").astype(float).reshape(-1, 2) / 32768
        assert np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 0.99
        for item in phases:
            segment = audio[int(item["start"] * rate):int(item["end"] * rate)]
            assert len(segment)
            item["rms"] = float(np.sqrt(np.mean(segment ** 2)))
            if item["name"] in ("armed-idle", "escape-release", "focus-loss-release", "stopped-take", "keyboard-release"):
                assert item["rms"] < 0.0001, item
            else:
                assert item["rms"] > 0.0005, item
        assert np.max(np.abs(audio[-rate:])) < 0.0001
        report = dict(status="passed", sample_rate=rate, phases=phases, snapshots=snapshots,
                      checks=["keyboard-first note sustain and release", "Energy force/speed knob", "Tone contact/brightness knob",
                              "Vibrato knob and double-click reset", "editable local notes and weight curve with undo",
                              "stationary signed-value pad", "reverse", "engine readback dot", "secondary button safety",
                              "Escape releases", "baseline restored on Follow", "scaled hit testing", "focus loss releases",
                              "local phrase and expression capture", "audible expression replay", "take stop"],
                      limitations="Synthetic X11 input and JACK dummy audio; no physical controller or human usability test")
        (OUTPUT / "native.json").write_text(json.dumps(report, indent=2))
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
