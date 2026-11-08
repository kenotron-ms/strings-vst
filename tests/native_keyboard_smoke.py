import ctypes
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time
import wave

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/live-keyboard-validation"


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / "native-keyboard.json").unlink(missing_ok=True)
    processes, streams, phases = [], [], []
    try:
        display = subprocess.Popen(["Xvfb", "-displayfd", "1", "-screen", "0", "1280x1000x24"],
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        processes.append(display)
        number = display.stdout.readline().strip()
        assert number
        env = {**os.environ, "DISPLAY": f":{number}", "LIBGL_ALWAYS_SOFTWARE": "1",
               "JACK_DEFAULT_SERVER": f"strings-keys-{os.getpid()}"}
        for name, arguments in (("jack", ["jackd", "--no-realtime", "--name", env["JACK_DEFAULT_SERVER"],
                                        "-d", "dummy", "-r", "48000", "-p", "1024"]),
                                ("standalone", [str(ROOT / "build/bin/Strings")])):
            stream = (OUTPUT / f"{name}.log").open("w")
            streams.append(stream)
            processes.append(subprocess.Popen(arguments, env=env, stdout=stream, stderr=stream))
            time.sleep(2)

        def command(*arguments):
            return subprocess.check_output(arguments, env=env, text=True).strip()

        window = command("xdotool", "search", "--pid", str(processes[-1].pid)).splitlines()[-1]
        command("xdotool", "windowmove", window, "0", "0", "windowfocus", "--sync", window)
        outputs = [port for port in command("jack_lsp").splitlines() if port.startswith("Strings:") and "out" in port.lower()]
        assert len(outputs) == 2
        stream = (OUTPUT / "recorder.log").open("w")
        streams.append(stream)
        recorder = subprocess.Popen(["jack_rec", "-f", str(OUTPUT / "keyboard.wav"), "-d", "30",
                                     "-b", "16", "-B", "1048576", *outputs], env=env, stdout=stream, stderr=stream)
        processes.append(recorder)
        origin = time.monotonic()

        def phase(name, audible, duration=0.8):
            start = time.monotonic() - origin
            time.sleep(duration)
            phases.append(dict(name=name, audible=audible, start=start + 0.35, end=time.monotonic() - origin - 0.08))

        def snapshot(name, position, active):
            path = OUTPUT / f"{name}.png"
            command("import", "-window", window, str(path))
            pixel = command("convert", str(path), "-format", f"%[pixel:p{{{position[0]},{position[1]}}}]", "info:")
            assert ("186,145,79" in pixel.replace(" ", "")) == active, (name, pixel)

        def screen(name):
            path = OUTPUT / f"{name}.png"
            command("import", "-window", window, str(path))
            return path

        screen("00-live-keyboard")
        phase("idle", False)
        command("xdotool", "mousemove", "473", "650", "mousedown", "1")
        phase("white-key", True)
        snapshot("01-clicked-key", (466, 650), True)
        command("xdotool", "mousemove", "482", "575")
        phase("black-key-drag", True)
        snapshot("02-black-key", (478, 574), True)
        command("xdotool", "mouseup", "1")
        phase("mouse-release", False, 1.1)
        snapshot("03-released-key", (478, 574), False)
        command("xdotool", "keydown", "w")
        phase("computer-accidental", True)
        command("xdotool", "key", "x")
        phase("octave-change-releases", False, 1.1)
        command("xdotool", "keyup", "w", "keydown", "a")
        phase("new-octave", True)
        command("xdotool", "key", "Escape", "keyup", "a")
        phase("escape-release", False, 1.1)
        command("xdotool", "key", "z", "keydown", "a", "keydown", "s")
        phase("computer-chord", True)
        command("xdotool", "mousemove", "935", "535", "click", "1", "keyup", "a", "keyup", "s")
        phase("release-button", False, 1.1)
        command("xdotool", "mousemove", "473", "650", "mousedown", "1", "mousemove", "200", "480")
        phase("drag-out-release", False, 1.1)
        command("xdotool", "mouseup", "1", "windowsize", window, "832", "544")
        time.sleep(0.4)
        command("xdotool", "mousemove", "378", "520", "mousedown", "1")
        phase("scaled-key", True)
        snapshot("04-scaled-key", (373, 520), True)
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
        command("xdotool", "windowfocus", "--sync", str(root), "mouseup", "1")
        phase("focus-loss-release", False, 1.1)
        command("xdotool", "windowsize", window, "1040", "680", "windowfocus", "--sync", window)
        time.sleep(0.25)
        command("xdotool", "mousemove", "350", "465", "mousedown", "1", "mousemove", "400", "480", "mouseup", "1", "key", "space")
        phase("removed-note-plot-is-inert", False)
        command("xdotool", "mousemove", "965", "84", "click", "1")
        screen("05-instrument-details")
        command("xdotool", "keydown", "a")
        phase("keyboard-in-details", True)
        command("xdotool", "keyup", "a")
        phase("details-key-release", False, 1.1)
        command("xdotool", "mousemove", "760", "84", "click", "1")
        screen("06-live-keyboard-final")
        recorder.wait(timeout=50)
        assert recorder.returncode == 0
        stream.flush()
        assert "overruns" not in (OUTPUT / "recorder.log").read_text()
        with wave.open(str(OUTPUT / "keyboard.wav"), "rb") as recording:
            assert recording.getnchannels() == 2 and recording.getsampwidth() == 2
            rate = recording.getframerate()
            audio = np.frombuffer(recording.readframes(recording.getnframes()), dtype="<i2").astype(float).reshape(-1, 2) / 32768
        for item in phases:
            segment = audio[int(item["start"] * rate):int(item["end"] * rate)]
            assert len(segment)
            item["rms"] = float(np.sqrt(np.mean(segment ** 2)))
            if item["audible"]:
                assert item["rms"] > 0.0005, item
            else:
                assert item["rms"] < 0.0001, item
        report = dict(status="passed", standalone_sha256=hashlib.sha256((ROOT / "build/bin/Strings").read_bytes()).hexdigest(),
                      phases=phases, screenshots=[str(path) for path in sorted(OUTPUT.glob("*.png"))],
                      scope="Actual native Linux standalone with synthetic X11 events and JACK dummy capture; no MIDI device or DAW")
        (OUTPUT / "native-keyboard.json").write_text(json.dumps(report, indent=2) + "\n")
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
        for stream in streams:
            stream.close()


if __name__ == "__main__":
    main()
