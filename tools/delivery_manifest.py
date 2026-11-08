import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
REPORTS = ROOT / "build/performance-validation"
TRIAL = ROOT / "build/standalone-workflow-r2"
MODULE = ROOT / "build/bin/Strings.vst3/Contents/x86_64-linux/Strings.so"
STANDALONE = ROOT / "build/bin/Strings"


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def main():
    version = re.search(r"project\(Strings VERSION ([0-9.]+)", (ROOT / "CMakeLists.txt").read_text())[1]
    reports = {}
    names = ("host", "host-automation", "research-audio", "native", "ux-bridge", "ux-interactions", "performance", "package-guard")
    for name in names:
        path = REPORTS / f"{name}.json"
        reports[name] = json.loads(path.read_text())
        require(reports[name]["status"] == "passed", f"Failed report: {path}")
    runtime_sources = [path for path in (ROOT / "src").rglob("*") if path.is_file()]
    binary_time = max(MODULE.stat().st_mtime, STANDALONE.stat().st_mtime)
    require(max(path.stat().st_mtime for path in runtime_sources) <= binary_time, "Runtime source is newer than tested binaries")
    for name in names[:-1]:
        require((REPORTS / f"{name}.json").stat().st_mtime >= binary_time, f"Report predates current binary: {name}")
    require(len(reports["host"]["measurements"]) == 12, "Missing host instrument/rate cases")
    automation = reports["host-automation"]
    require(automation["plugin_sha256"][str(MODULE.relative_to(ROOT))] == sha256(MODULE), "Host automation binary changed")
    expected = {(instrument, rate, parameter) for instrument in ("Violin", "Viola", "Cello", "Double bass")
                for rate in (44100, 48000, 96000)
                for parameter in ("bow_pressure", "bow_position", "bow_direction", "bow_engaged", "vibrato_depth")}
    actual = {(entry["instrument"], entry["sample_rate"], entry["parameter"]) for entry in automation["measurements"]}
    require(actual == expected, "Missing held-note automation cases")
    require({entry["view"] for entry in reports["ux-bridge"]["results"]} ==
            {"bow", "mood", "phrase", "conductor", "material", "synthetic-gamepad"}, "Missing UX routes")
    log_names = ("ctest.log", "sanitizer.log", "pluginval.log")
    for name in log_names:
        path = REPORTS / name
        require(path.stat().st_mtime >= binary_time, f"Log predates current build: {name}")
        text = path.read_text()
        if name == "pluginval.log":
            require(text.rstrip().endswith("SUCCESS") and "Strictness level: 5" in text and
                    f"Strings v{version}" in text and "Num plugins found: 1" in text, "pluginval did not validate this version")
        else:
            require("100% tests passed, 0 tests failed out of 4" in text, f"Incomplete CTest: {name}")
    session = json.loads((TRIAL / "session.json").read_text())
    audit = json.loads((TRIAL / "audio-audit.json").read_text())
    require(session["status"] == audit["status"] == "passed", "Standalone audit failed")
    require(session["standalone_sha256"] == audit["standalone_sha256"] == sha256(STANDALONE), "Standalone binary changed")
    require(audit["jack_processing_errors"] == 0, "Standalone capture has processing errors")
    for name, expected_hash in audit["audio_sha256"].items():
        require(sha256(TRIAL / name) == expected_hash, f"Standalone audio changed: {name}")
    inputs = {ROOT / name for name in ("CMakeLists.txt", "README.md", "LICENSE", "THIRD_PARTY_NOTICES.md", ".gitmodules", ".gitignore")}
    for directory in ("src", "tests", "tools", "docs", "assets", "third_party", ".github", ".amplifier/goals"):
        inputs.update(path for path in (ROOT / directory).rglob("*") if path.is_file() and not path.is_symlink()
                      and "__pycache__" not in path.parts and ".git" not in path.parts)
    inputs.update(path for path in (ROOT / "build/bin").rglob("*") if path.is_file())
    for name in names:
        inputs.add(REPORTS / f"{name}.json")
    inputs.update(REPORTS / name for name in log_names)
    inputs.add(REPORTS / "completion.md")
    inputs.update(path for path in TRIAL.rglob("*") if path.is_file())
    for report, key in ((reports["performance"], "measurements"), (reports["ux-bridge"], "results")):
        inputs.update(ROOT / entry["audio"] for entry in report[key])
    inputs.update(ROOT / entry["path"] for entry in reports["research-audio"]["auditions"])
    inputs.update(ROOT / name for name in automation["audio_examples"])
    manifest = dict(
        schema=1, version=version, status="passed", validated_at_utc=datetime.now(timezone.utc).isoformat(),
        scope="Local Linux x86-64 bounded research prototype; not published or acoustically calibrated",
        acceptance="docs/delivery-0.3.1.md records the manual requirement audit; this inventory is not a perceptual-realism test",
        sha256={str(path.relative_to(ROOT)): sha256(path) for path in sorted(inputs)},
        reports=[f"{name}.json" for name in names] + list(log_names),
        standalone_trial="build/standalone-workflow-r2/audio-audit.json",
        archives=[f"dist/Strings-{version}-linux-x86_64.zip", f"dist/Strings-{version}-source.tar.gz"],
        archive_verification="build/performance-validation/package-check.json is generated after packaging and deliberately excluded from the archives",
        limitations=["Synthetic body/string profiles; no measured calibration or human realism study",
                     "High violin register remains about 19.38 cents sharp in the strong-bow audit",
                     "Reduced physical model; full finite-width thermal/hair/FDTD and neural methods are not implemented",
                     "Synthetic controller tests through an adapter; no physical device or advanced DualSense verification",
                     "Eight-second scratch editor is not a persistent, host-synchronized composition timeline",
                     "Original Harbour Run cue, not the requested film melody; no claim that exact-theme task is finished",
                     "DPF lifecycle warnings remain; Linux validation is not all-host/platform certification"])
    (REPORTS / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Inventoried {len(inputs)} files for local {version}; source/build/reports and clean standalone hashes agree")


if __name__ == "__main__":
    main()
