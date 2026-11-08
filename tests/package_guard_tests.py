import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def main():
    cases = []
    with tempfile.TemporaryDirectory(prefix="strings-package-guard-") as directory:
        root = Path(directory)
        (root / "CMakeLists.txt").write_text("project(Strings VERSION 0.3.1 LANGUAGES C CXX)\n")
        report = root / "build/performance-validation/manifest.json"
        report.parent.mkdir(parents=True)
        source = root / "source.cpp"
        source.write_text("original\n")
        scenarios = [
            (dict(version="0.3.0", status="passed", sha256={}), "does not certify this version"),
            (dict(version="0.3.1", status="failed", sha256={}), "does not certify this version"),
            (dict(version="0.3.1", status="passed", sha256={"source.cpp": hashlib.sha256(b"old\n").hexdigest()}), "Validation is stale"),
            (dict(version="0.3.1", status="passed", sha256={"missing.cpp": "0" * 64}), "Validation is stale"),
        ]
        for manifest, expected in scenarios:
            report.write_text(json.dumps(manifest))
            result = subprocess.run([sys.executable, str(ROOT / "tools/package.py"), "linux-x86_64"],
                                    cwd=root, capture_output=True, text=True)
            assert result.returncode != 0 and expected in result.stderr, result.stderr
            assert not (root / "dist").exists(), "Failed validation still produced an archive"
            cases.append(expected)
        spec = importlib.util.spec_from_file_location("strings_package", ROOT / "tools/package.py")
        package = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(package)
        (root / "src").mkdir()
        (root / "src/model.cpp").write_text("fixture")
        (root / "src/platform.cpp").symlink_to("model.cpp")
        (root / "src/.git").mkdir()
        (root / "src/.git/private").write_text("do not export")
        (root / "dist").mkdir()
        previous = Path.cwd()
        try:
            os.chdir(root)
            archive = package.package_source("0.3.1")
            with tarfile.open(archive) as bundle:
                assert not any(".git" in Path(name).parts for name in bundle.getnames())
                member = bundle.getmember("Strings-0.3.1-source/src/platform.cpp")
                assert member.isfile() and bundle.extractfile(member).read() == b"fixture"
            (root / "src/keys.env").write_text("fake fixture, not a credential")
            try:
                package.package_source("0.3.1")
            except ValueError as error:
                assert "environment file" in str(error)
            else:
                raise AssertionError("Environment file was exported")
            (root / "src/keys.env").unlink()
            (root / "src/platform.cpp").unlink()
            (root / "src/platform.cpp").symlink_to(root.parent)
            try:
                package.package_source("0.3.1")
            except ValueError as error:
                assert "unsafe symlink" in str(error)
            else:
                raise AssertionError("Out-of-tree symlink was exported")
            cases += ["internal symlink materialized; Git metadata excluded", "environment files and out-of-tree symlinks rejected"]
        finally:
            os.chdir(previous)
    destination = ROOT / "build/performance-validation/package-guard.json"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(dict(status="passed", checks=cases,
                                          scope="Isolated temporary fixtures; production manifest/binary not modified"), indent=2) + "\n")
    print("PASS: stale/failed package guards, safe source export and secret-path exclusions")


if __name__ == "__main__":
    main()
