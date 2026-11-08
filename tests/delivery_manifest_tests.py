import importlib.util
import json
from pathlib import Path
import tempfile


ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("delivery_manifest", ROOT / "tools/delivery_manifest.py")
delivery = importlib.util.module_from_spec(spec)
spec.loader.exec_module(delivery)


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value) if isinstance(value, dict) else value)


def main():
    with tempfile.TemporaryDirectory(prefix="strings-manifest-") as directory:
        delivery.ROOT = root = Path(directory)
        delivery.REPORTS = reports = root / "build/performance-validation"
        delivery.TRIAL = trial = root / "build/standalone-workflow-r2"
        delivery.MODULE = module = root / "build/bin/Strings.vst3/Contents/x86_64-linux/Strings.so"
        delivery.STANDALONE = standalone = root / "build/bin/Strings"
        write(root / "CMakeLists.txt", "project(Strings VERSION 0.3.1 LANGUAGES C CXX)")
        for name in ("README.md", "LICENSE", "THIRD_PARTY_NOTICES.md", ".gitmodules", ".gitignore", "src/model.cpp"):
            write(root / name, "fixture")
        write(module, "module")
        write(standalone, "standalone")
        cases = [dict(instrument=instrument, sample_rate=rate, parameter=parameter)
                 for instrument in ("Violin", "Viola", "Cello", "Double bass") for rate in (44100, 48000, 96000)
                 for parameter in ("bow_pressure", "bow_position", "bow_direction", "bow_engaged", "vibrato_depth")]
        automation = dict(status="passed", measurements=cases, audio_examples=[],
                          plugin_sha256={str(module.relative_to(root)): delivery.sha256(module)})
        write(reports / "host.json", dict(status="passed", measurements=[{}] * 12))
        write(reports / "host-automation.json", automation)
        write(reports / "research-audio.json", dict(status="passed", auditions=[]))
        for name in ("native", "ux-interactions", "package-guard"):
            write(reports / f"{name}.json", dict(status="passed"))
        write(reports / "ux-bridge.json", dict(status="passed", results=[dict(view=view, audio="README.md")
              for view in ("bow", "mood", "phrase", "conductor", "material", "synthetic-gamepad")]))
        write(reports / "performance.json", dict(status="passed", measurements=[]))
        for name in ("ctest.log", "sanitizer.log"):
            write(reports / name, "100% tests passed, 0 tests failed out of 4")
        write(reports / "pluginval.log", "Strictness level: 5\nStrings v0.3.1\nNum plugins found: 1\nSUCCESS\n")
        write(reports / "completion.md", "manual acceptance audit fixture")
        write(trial / "session.json", dict(status="passed", standalone_sha256=delivery.sha256(standalone)))
        write(trial / "audio-audit.json", dict(status="passed", standalone_sha256=delivery.sha256(standalone),
                                             jack_processing_errors=0, audio_sha256={}))
        delivery.main()
        assert json.loads((reports / "manifest.json").read_text())["version"] == "0.3.1"
        cases.pop()
        write(reports / "host-automation.json", automation)
        try:
            delivery.main()
        except ValueError as error:
            assert "Missing held-note" in str(error)
        else:
            raise AssertionError("Missing automation coverage was accepted")
    print("PASS: manifest generation and missing automation coverage rejection")


if __name__ == "__main__":
    main()
