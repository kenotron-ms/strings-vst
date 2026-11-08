import hashlib
import json
from pathlib import Path
import subprocess
import tarfile
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parents[1]
REPORTS = ROOT / "build/performance-validation"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def safe_name(name, prefix):
    path = Path(name)
    if path.is_absolute() or ".." in path.parts or path.parts[0] != prefix:
        raise ValueError(f"Unsafe archive member: {name}")
    if ".git" in path.parts or path.suffix == ".env" or path.name == ".env":
        raise ValueError(f"Private metadata/environment file in archive: {name}")
    return Path(*path.parts[1:])


def main():
    manifest = json.loads((REPORTS / "manifest.json").read_text())
    version = manifest["version"]
    binary_name = f"Strings-{version}-linux-x86_64"
    source_name = f"Strings-{version}-source"
    binary_archive = ROOT / f"dist/{binary_name}.zip"
    source_archive = ROOT / f"dist/{source_name}.tar.gz"
    archive_hashes = {str(path.relative_to(ROOT)): digest(path.read_bytes()) for path in (binary_archive, source_archive)}
    for name, expected in manifest["sha256"].items():
        if digest((ROOT / name).read_bytes()) != expected:
            raise ValueError(f"Manifest became stale: {name}")
    checked_binary = 0
    with zipfile.ZipFile(binary_archive) as bundle:
        for name in bundle.namelist():
            safe_name(name, binary_name)
        for name, expected in manifest["sha256"].items():
            relative = None
            path = Path(name)
            if name.startswith("build/bin/"):
                relative = path.relative_to("build/bin")
            elif name.startswith(("docs/", "assets/")) or name in ("README.md", "LICENSE", "THIRD_PARTY_NOTICES.md"):
                relative = path
            elif name.startswith("build/performance-validation/"):
                relative = Path("Research validation") / path.relative_to("build/performance-validation")
            elif name.startswith("build/standalone-workflow-r2/"):
                trial_relative = path.relative_to("build/standalone-workflow-r2")
                if trial_relative.suffix in (".wav", ".mid", ".png") or trial_relative.name in ("session.json", "audio-audit.json", "score.json"):
                    relative = Path("Standalone trial") / trial_relative
            if relative is not None:
                if digest(bundle.read(f"{binary_name}/{relative.as_posix()}")) != expected:
                    raise ValueError(f"Binary archive member changed: {relative}")
                checked_binary += 1
        if json.loads(bundle.read(f"{binary_name}/Research validation/manifest.json")) != manifest:
            raise ValueError("Packaged manifest differs")
        if any(name.endswith("/package-check.json") for name in bundle.namelist()):
            raise ValueError("Archive includes a self-referential or historical archive check")
    checked_source = 0
    with tempfile.TemporaryDirectory(prefix="strings-source-verification-") as directory:
        temporary = Path(directory)
        with tarfile.open(source_archive) as bundle:
            members = bundle.getmembers()
            actual_sources = set()
            for member in members:
                relative = safe_name(member.name, source_name)
                if not member.isfile():
                    raise ValueError(f"Source archive has a non-file member: {member.name}")
                source = ROOT / relative
                if not source.resolve().is_relative_to(ROOT) or digest(bundle.extractfile(member).read()) != digest(source.read_bytes()):
                    raise ValueError(f"Source archive member differs: {relative}")
                actual_sources.add(relative.as_posix())
                checked_source += 1
            required = {name for name in manifest["sha256"] if not name.startswith("build/")}
            if not required <= actual_sources:
                raise ValueError(f"Source archive omissions: {sorted(required - actual_sources)}")
            bundle.extractall(temporary, members=members)
        extracted = temporary / source_name
        build = temporary / "build"
        with (REPORTS / "package-source-build.log").open("w") as output:
            subprocess.run(["cmake", "-S", str(extracted), "-B", str(build), "-DCMAKE_BUILD_TYPE=Release"],
                           check=True, stdout=output, stderr=subprocess.STDOUT)
            subprocess.run(["cmake", "--build", str(build), "-j4"], check=True, stdout=output, stderr=subprocess.STDOUT)
        with (REPORTS / "package-source-ctest.log").open("w") as output:
            subprocess.run(["ctest", "--test-dir", str(build), "--output-on-failure"],
                           check=True, stdout=output, stderr=subprocess.STDOUT)
        if "100% tests passed, 0 tests failed out of 4" not in (REPORTS / "package-source-ctest.log").read_text():
            raise ValueError("Extracted-source CTest report is incomplete")
        if not (build / "bin/Strings.vst3/Contents/x86_64-linux/Strings.so").is_file():
            raise ValueError("Extracted sources did not produce VST3")
    report = dict(status="passed", version=version, archive_sha256=archive_hashes,
                  verified_binary_members=checked_binary, verified_source_members=checked_source,
                  extracted_source="Clean full Release build produced VST3/CLAP/standalone; CTest 4/4 passed",
                  exclusions="No Git metadata/environment files/unsafe links; no self-referential archive check",
                  scope="Archive identity and source reproducibility, not acoustic realism or new host validation")
    (REPORTS / "package-check.json").write_text(json.dumps(report, indent=2) + "\n")
    checksums = ROOT / f"dist/Strings-{version}-SHA256SUMS"
    checksums.write_text("".join(f"{value}  {Path(name).name}\n" for name, value in archive_hashes.items()))
    print(f"PASS: {checked_binary} binary-package members, {checked_source} source files, clean build and CTest 4/4")


if __name__ == "__main__":
    main()
