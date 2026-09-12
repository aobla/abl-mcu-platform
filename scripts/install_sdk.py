#!/usr/bin/env python3
"""
Install a vendor SDK from manifest.yml (used by CI and by hand).

    install_sdk.py <sdk-name> [--manifest PATH]

Reads the SDK definition from the platform manifest (single source of versions,
ARCHITECTURE §5) and clones every pinned repository into `install_dir`, honouring
the optional `subdir` of each entry.

Example:
    ./scripts/install_sdk.py stm32f1-hal        # → ~/.local/share/abl-mcu-sdks/stm32f1-hal
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

import yaml

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_MANIFEST = REPO_ROOT / "manifest.yml"


def main():
    parser = argparse.ArgumentParser(description="Install a vendor SDK from the platform manifest")
    parser.add_argument("sdk", help="SDK name as defined in manifest.yml (sdks.<name>)")
    parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="path to manifest.yml")
    parser.add_argument("--force", action="store_true", help="re-clone even if the directory exists")
    args = parser.parse_args()

    manifest_path = Path(args.manifest)
    if not manifest_path.is_file():
        sys.exit(f"Error: manifest not found: {manifest_path}")

    manifest = yaml.safe_load(manifest_path.read_text(encoding="utf-8")) or {}
    sdks = manifest.get("sdks") or {}
    if args.sdk not in sdks:
        sys.exit(f"Error: SDK '{args.sdk}' is not defined in {manifest_path} "
                 f"(available: {', '.join(sorted(sdks)) or '-'})")

    spec = sdks[args.sdk]
    dest = Path(os.path.expanduser(spec["install_dir"]))
    repos = spec.get("git") or []
    if not repos:
        sys.exit(f"Error: SDK '{args.sdk}' has no git repositories in the manifest")

    print(f"Installing SDK '{args.sdk}' → {dest}")
    for repo in repos:
        subdir = repo.get("subdir", "")
        target = dest / subdir if subdir else dest

        # A repo may already be present *inside* another clone (e.g. CMSIS_5 lands
        # in <sdk>/CMSIS while cmsis_device_* uses <sdk>/CMSIS/Device): treat a
        # non-empty target as installed, unless --force is given.
        if target.exists() and any(target.iterdir()) and not args.force:
            print(f"  [skip] {target} (already present)")
            continue

        target.mkdir(parents=True, exist_ok=True)
        print(f"  [git]  {repo['url']} @ {repo['tag']} → {target}")
        subprocess.run(
            ["git", "clone", "--depth", "1", "--branch", repo["tag"], repo["url"], str(target)],
            check=True,
        )

    print("Done.")


if __name__ == "__main__":
    main()
