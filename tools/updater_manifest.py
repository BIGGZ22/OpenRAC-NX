#!/usr/bin/env python3
"""Generates Tauri v2 latest.json updater manifest from release build artifacts."""

import argparse
import datetime
import json
import os
import sys
from typing import Any, Dict, Optional


def _is_arm(name: str) -> bool:
    return "aarch64" in name or "arm64" in name


def _linux_installer(name: str) -> str:
    if ".deb" in name:
        return "deb"
    if ".rpm" in name:
        return "rpm"
    if "appimage" in name:
        return "appimage"
    if name.endswith(".tar.gz") or ".tar.gz" in name:
        return "appimage"
    return "appimage"


def _windows_installer(name: str) -> str:
    if ".msi" in name or "msi" in name:
        return "msi"
    return "nsis"


def generate_manifest(
    artifacts_dir: str,
    tag: str,
    repo: str = "OpenRAC/OpenRAC",
    notes: str = "",
    pub_date: Optional[str] = None,
) -> Dict[str, Any]:
    clean_version = tag.removeprefix("launcher-v").removeprefix("v")
    if not pub_date:
        pub_date = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    platforms: Dict[str, Dict[str, str]] = {}

    for root, _, files in os.walk(artifacts_dir):
        for f in sorted(files):
            if not f.endswith(".sig"):
                continue
            base = f[:-4]
            sig_path = os.path.join(root, f)
            with open(sig_path, "r", encoding="utf-8") as s:
                sig_content = s.read().strip()

            # GitHub Release asset names cannot have spaces: GitHub converts spaces to dots.
            archive_file = base.replace(" ", ".")
            asset_url = f"https://github.com/{repo}/releases/download/{tag}/{archive_file}"

            rel = os.path.relpath(sig_path, artifacts_dir).lower()
            name = f.lower()

            if "darwin" in rel or "macos" in rel or ".dmg" in rel or ".app.tar.gz" in rel or "macos" in name or "darwin" in name:
                platforms["darwin-aarch64-app"] = {"signature": sig_content, "url": asset_url}
                platforms["darwin-x86_64-app"] = {"signature": sig_content, "url": asset_url}
            elif "windows" in rel or ".exe" in rel or ".msi" in rel or "nsis" in rel or "setup" in rel or "windows" in name or ".exe" in name or ".msi" in name:
                arch = "aarch64" if _is_arm(rel + name) else "x86_64"
                installer = _windows_installer(rel + name)
                platforms[f"windows-{arch}-{installer}"] = {"signature": sig_content, "url": asset_url}
            else:
                arch = "aarch64" if _is_arm(rel + name) else "x86_64"
                installer = _linux_installer(rel + name)
                platforms[f"linux-{arch}-{installer}"] = {"signature": sig_content, "url": asset_url}

    return {
        "version": clean_version,
        "notes": notes or f"OpenRAC Launcher release {clean_version}",
        "pub_date": pub_date,
        "platforms": platforms,
    }


def main():
    parser = argparse.ArgumentParser(description="Generate Tauri v2 latest.json for OpenRAC Launcher")
    parser.add_argument("artifacts_dir", help="Directory containing release artifacts and .sig files")
    parser.add_argument("--tag", required=True, help="GitHub release tag (e.g. launcher-v0.5.0)")
    parser.add_argument("--repo", default="OpenRAC/OpenRAC", help="GitHub repo (default: OpenRAC/OpenRAC)")
    parser.add_argument("--notes", default="", help="Release notes")
    parser.add_argument("--output", default="latest.json", help="Output path (default: latest.json)")
    args = parser.parse_args()

    manifest = generate_manifest(args.artifacts_dir, args.tag, args.repo, args.notes)
    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)
    print(f"Wrote updater manifest with {len(manifest['platforms'])} platform(s) to {args.output}")


if __name__ == "__main__":
    main()
