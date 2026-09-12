#!/usr/bin/env python3
"""
Install a vendor SDK from manifest.yml (used by CI and by hand).

    install_sdk.py <sdk-name> [--manifest PATH] [--force]

Reads the SDK definition from the platform manifest (single source of versions,
ARCHITECTURE §5) and materialises every pinned repository inside `install_dir`,
honouring the optional `subdir` of each entry.

Layout example (stm32f1-hal):
    <install_dir>/                     <- stm32f1xx-hal-driver (root entry)
    <install_dir>/CMSIS/               <- CMSIS_5 (subdir: CMSIS)
    <install_dir>/CMSIS/Device/        <- cmsis_device_f1 (subdir: CMSIS/Device)

Nested targets are why cloning goes through a temporary directory: git refuses to
clone into a non-empty directory, and entries may be nested in any order (e.g.
`CMSIS/Device` already exists when `CMSIS` is installed). Merging from a temp
clone makes the result independent of the order in the manifest.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import yaml

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_MANIFEST = REPO_ROOT / "manifest.yml"


def merge_tree(source, target):
    """Copy the content of source into target, preserving existing siblings."""
    target.mkdir(parents=True, exist_ok=True)
    for entry in source.iterdir():
        destination = target / entry.name
        if entry.is_dir():
            shutil.copytree(entry, destination, dirs_exist_ok=True)
        else:
            shutil.copy2(entry, destination)


def clone_into(url, tag, target):
    """Clone url@tag and merge the working tree (without .git) into target."""
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp) / "repo"
        subprocess.run(
            ["git", "clone", "--depth", "1", "--branch", tag, url, str(tmp_path)],
            check=True,
        )
        merge_tree(tmp_path, target)


def main():
    parser = argparse.ArgumentParser(description="Install a vendor SDK from the platform manifest")
    parser.add_argument("sdk", help="SDK name as defined in manifest.yml (sdks.<name>)")
    parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="path to manifest.yml")
    parser.add_argument("--force", action="store_true", help="re-install even if present")
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

    # The SDK counts as installed only when every declared include dir exists:
    # a half-installed SDK must be repaired, not skipped.
    include_dirs = spec.get("include_dirs") or []

    def installed():
        if not dest.is_dir():
            return False
        if include_dirs:
            return all((dest / d).is_dir() for d in include_dirs)
        return any(dest.iterdir())

    if installed() and not args.force:
        print(f"SDK '{args.sdk}' already installed at {dest}")
        return

    print(f"Installing SDK '{args.sdk}' → {dest}")
    for repo in repos:
        subdir = repo.get("subdir", "")
        target = dest / subdir if subdir else dest
        print(f"  [git]  {repo['url']} @ {repo['tag']} → {target}", flush=True)
        clone_into(repo["url"], repo["tag"], target)

    missing = [d for d in include_dirs if not (dest / d).is_dir()]
    if missing:
        sys.exit(f"Error: SDK '{args.sdk}' installed but these include dirs are missing: "
                 f"{', '.join(missing)}")

    print(f"Done. Include dirs present: {', '.join(include_dirs) if include_dirs else '-'}")


if __name__ == "__main__":
    main()
