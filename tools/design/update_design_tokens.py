#!/usr/bin/env python3
"""Vendor ArcticWorks design tokens from the npm registry.

Downloads the pinned @arcticworks/design tarball, verifies its shasum against
the registry metadata, and refreshes tools/design/vendor/.

Usage: python3 tools/design/update_design_tokens.py [version]
"""

import hashlib
import json
import sys
import tarfile
import tempfile
import urllib.request
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
VENDOR_DIR = REPO_ROOT / "tools" / "design" / "vendor"
PIN_FILE = VENDOR_DIR / "package.json"

KEEP_PATHS = ("package/tokens/", "package/themes/", "package/LICENSE", "package/package.json", "package/README.md")


def fetch_registry_metadata(version):
    url = f"https://registry.npmjs.org/@arcticworks%2fdesign/{version}"
    with urllib.request.urlopen(url) as response:
        return json.load(response)


def main():
    with open(PIN_FILE, encoding="utf-8") as f:
        pin = json.load(f)
    version = sys.argv[1] if len(sys.argv) > 1 else pin["version"]
    metadata = fetch_registry_metadata(version)
    tarball = metadata["dist"]["tarball"]
    shasum = metadata["dist"]["shasum"]

    print(f"Downloading @arcticworks/design@{version} ...")
    with tempfile.NamedTemporaryFile(suffix=".tgz", delete=False) as tmp:
        with urllib.request.urlopen(tarball) as response:
            content = response.read()
        digest = hashlib.sha1(content).hexdigest()
        if digest != shasum:
            raise RuntimeError(f"Checksum mismatch: expected {shasum}, got {digest}")
        tmp.write(content)
        tmp_path = Path(tmp.name)

    print(f"Updating {VENDOR_DIR} ...")
    with tarfile.open(tmp_path, "r:gz") as tar:
        for member in tar.getmembers():
            if not any(member.name.startswith(prefix) for prefix in KEEP_PATHS):
                continue
            if member.isfile():
                target = VENDOR_DIR / Path(member.name).relative_to("package")
                target.parent.mkdir(parents=True, exist_ok=True)
                src = tar.extractfile(member)
                if src is None:
                    raise RuntimeError(f"Could not extract {member.name}")
                with src, open(target, "wb") as dst:
                    dst.write(src.read())

    tmp_path.unlink()
    print(f"Vendored @arcticworks/design@{version} ({shasum[:12]}...)")


if __name__ == "__main__":
    sys.exit(main())
