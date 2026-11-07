from pathlib import Path
import shutil
import sys


def main():
    platform = sys.argv[1]
    version = "0.1.0"
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
    shutil.copy2("assets/FONT-LICENSE.txt", root / "docs/FONT-LICENSE.txt")
    shutil.copy2("third_party/dpf/LICENSE", root / "docs/DPF-LICENSE.txt")
    shutil.copy2("build/host-smoke.json", root / "docs/audio-measurements.json")
    shutil.copytree("build/pluginval-logs", root / "Validation")
    shutil.copytree("build/demos", root / "Audio examples")
    Path("dist").mkdir(exist_ok=True)
    archive = shutil.make_archive(str(Path("dist") / name), "zip", root.parent, name)
    print(archive)


if __name__ == "__main__":
    main()
