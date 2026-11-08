from pathlib import Path
import hashlib
import json
import re
import shutil
import sys
import tarfile


def package_source(version):
    roots = [Path(name) for name in ("CMakeLists.txt", "README.md", "LICENSE", "THIRD_PARTY_NOTICES.md",
             ".gitignore", ".gitmodules", "src", "tests", "tools", "docs", "assets", "third_party", ".github", ".amplifier/goals")]
    files = set()
    for root in roots:
        for source in root.rglob("*") if root.is_dir() else [root]:
            if any(part in (".git", "__pycache__", ".DS_Store") for part in source.parts):
                continue
            if source.is_symlink():
                target = source.resolve()
                if not target.is_relative_to(Path.cwd().resolve()) or target.suffix == ".env" or target.name == ".env" or ".git" in target.parts:
                    raise ValueError(f"Refusing unsafe symlink in source export: {source}")
            if source.is_file():
                if source.suffix == ".env" or source.name == ".env":
                    raise ValueError(f"Refusing environment file in source export: {source}")
                files.add(source)
    archive = Path("dist") / f"Strings-{version}-source.tar.gz"
    with tarfile.open(archive, "w:gz", dereference=True) as bundle:
        for source in sorted(files):
            bundle.add(source, arcname=f"Strings-{version}-source/{source}", recursive=False)
    return archive


def main():
    platform = sys.argv[1]
    version = re.search(r"project\(Strings VERSION ([0-9.]+)", Path("CMakeLists.txt").read_text()).group(1)
    current_manifest = Path("build/performance-validation/manifest.json")
    if current_manifest.exists():
        manifest = json.loads(current_manifest.read_text())
        if manifest["version"] != version or manifest["status"] != "passed":
            raise ValueError("Current performance validation does not certify this version")
        for filename, expected in manifest["sha256"].items():
            source = Path(filename)
            if not source.is_file() or hashlib.sha256(source.read_bytes()).hexdigest() != expected:
                raise ValueError(f"Validation is stale for {filename}; validate the current build before packaging")
    name = f"Strings-{version}-{platform}"
    root = Path("build/package") / name
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    binary = Path("build/bin")
    for filename in ("Strings.vst3", "Strings.clap"):
        source = binary / filename
        if not source.exists():
            raise FileNotFoundError(source)
        if source.is_dir():
            shutil.copytree(source, root / filename)
        else:
            shutil.copy2(source, root / filename)
    standalone = next((binary / name for name in ("Strings.app", "Strings.exe", "Strings") if (binary / name).exists()), None)
    if standalone is None:
        raise FileNotFoundError("Standalone application missing")
    if standalone.is_dir():
        shutil.copytree(standalone, root / standalone.name)
    else:
        shutil.copy2(standalone, root / standalone.name)
    for document in ("README.md", "LICENSE", "THIRD_PARTY_NOTICES.md"):
        shutil.copy2(document, root / document)
    shutil.copytree("docs", root / "docs")
    shutil.copytree("assets", root / "assets")
    shutil.copy2("assets/FONT-LICENSE.txt", root / "docs/FONT-LICENSE.txt")
    shutil.copy2("third_party/dpf/LICENSE", root / "docs/DPF-LICENSE.txt")
    shutil.copy2("build/host-smoke.json", root / "docs/audio-measurements.json")
    if Path("build/build-info.json").exists():
        shutil.copy2("build/build-info.json", root / "docs/build-info.json")
    validation = Path("build/performance-validation/pluginval")
    if not validation.exists():
        validation = Path("build/pluginval-research-logs")
    if not validation.exists():
        validation = Path("build/pluginval-logs")
    shutil.copytree(validation, root / "Validation")
    performance = Path("build/performance-validation/performance.json")
    if not performance.exists():
        performance = Path("build/research-validation/performance.json")
    if performance.exists():
        audio = root / "Audio examples"
        audio.mkdir()
        for measurement in json.loads(performance.read_text())["measurements"]:
            source = Path(measurement["audio"])
            shutil.copy2(source, audio / source.name)
    else:
        shutil.copytree("build/demos", root / "Audio examples")
    research = Path("build/research-validation")
    performance_validation = Path("build/performance-validation")
    if (performance_validation / "manifest.json").exists():
        shutil.copytree(performance_validation, root / "Research validation", ignore=shutil.ignore_patterns("package-check.json"))
    elif research.exists():
        shutil.copytree(research, root / "Research validation")
    trial = Path("build/standalone-workflow-r2")
    if current_manifest.exists() and trial.exists():
        session = json.loads((trial / "session.json").read_text())
        audit = json.loads((trial / "audio-audit.json").read_text())
        if session["status"] != "passed" or audit["status"] != "passed":
            raise ValueError("Standalone trial is not validated")
        expected = hashlib.sha256((binary / "Strings").read_bytes()).hexdigest()
        if session["standalone_sha256"] != expected or audit["standalone_sha256"] != expected:
            raise ValueError("Standalone trial does not certify this binary")
        destination = root / "Standalone trial"
        destination.mkdir()
        for filename in ("session.json", "audio-audit.json", "score.json", "harbour-run.mid", *audit["audio_sha256"]):
            source = trial / filename
            if filename in audit["audio_sha256"] and hashlib.sha256(source.read_bytes()).hexdigest() != audit["audio_sha256"][filename]:
                raise ValueError(f"Standalone audio changed: {filename}")
            shutil.copy2(source, destination / filename)
        shutil.copytree(trial / "screenshots", destination / "screenshots")
    Path("dist").mkdir(exist_ok=True)
    archive = shutil.make_archive(str(Path("dist") / name), "zip", root.parent, name)
    print(archive)
    if "--source" in sys.argv[2:]:
        print(package_source(version))


if __name__ == "__main__":
    main()
