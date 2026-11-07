import os
from pathlib import Path
import platform
import subprocess
import sys
import urllib.request
import zipfile


def main():
    system = platform.system()
    suffix = {"Linux": "Linux", "Darwin": "macOS", "Windows": "Windows"}[system]
    root = Path("build/pluginval").resolve()
    root.mkdir(parents=True, exist_ok=True)
    archive = root / "pluginval.zip"
    urllib.request.urlretrieve(
        f"https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_{suffix}.zip", archive
    )
    with zipfile.ZipFile(archive) as bundle:
        bundle.extractall(root)
    binary = next(path for path in root.rglob("pluginval.exe" if system == "Windows" else "pluginval") if path.is_file())
    binary.chmod(0o755)
    if system == "Darwin":
        subprocess.run(["xattr", "-dr", "com.apple.quarantine", str(root)], check=False)
    logs = Path("build/pluginval-logs").resolve()
    logs.mkdir(parents=True, exist_ok=True)
    command = [str(binary), "--validate", str(Path(sys.argv[1]).resolve()),
               "--strictness-level", "5", "--timeout-ms", "120000", "--output-dir", str(logs)]
    result = subprocess.run(command, timeout=600, env={**os.environ, "LIBGL_ALWAYS_SOFTWARE": "1"})
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
