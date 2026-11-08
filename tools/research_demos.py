import json
import os
from pathlib import Path
import platform
import re
import subprocess


def main():
    root = Path(__file__).resolve().parents[1]
    output = root / "build/demos/research"
    output.mkdir(parents=True, exist_ok=True)
    cases = [("violin", 0, 1, 0), ("viola", 1, 1, 0), ("cello", 2, 1, 0),
             ("double-bass", 3, 1, 0), ("ensemble-8", 0, 8, 0),
             ("ensemble-16", 0, 16, 0), ("pizzicato", 0, 1, 1), ("tremolo", 0, 1, 2)]
    results = []
    for name, instrument, players, articulation in cases:
        path = output / f"{name}.wav"
        command = [str(root / "build/strings_render"), str(path), str(instrument),
                   str(players), str(articulation)]
        text = subprocess.check_output(command, text=True)
        print(text.strip(), flush=True)
        measurements = {key: float(value) for key, value in re.findall(
            r"(peak|rms|render_seconds|realtime_factor)=([0-9.eE+-]+)", text)}
        assert len(measurements) == 4 and measurements["peak"] > 0.001
        results.append(dict(name=name, players=players, seconds=8,
                            audio=str(path.relative_to(root)), **measurements))
    cpu_model = platform.processor()
    cpu_info = Path("/proc/cpuinfo")
    if cpu_info.exists():
        cpu_model = next((line.split(":", 1)[1].strip() for line in cpu_info.read_text().splitlines()
                          if line.startswith("model name")), cpu_model)
    report = dict(status="passed", sample_rate=48000, build="Release", platform=platform.platform(), cpu_model=cpu_model,
                  logical_cpus=os.cpu_count(),
                  scope="Single-threaded offline DSP plus WAV writing. Eight-second phrases, six seconds excitation; not live buffer deadlines or worst-case polyphony.",
                  measurements=results)
    destination = root / "build/research-validation/performance.json"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
