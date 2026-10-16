#!/usr/bin/env python3
"""Generates Tauri v2 latest.json updater manifest from release build artifacts."""

import argparse
import datetime
import json
import os
import sys
from typing import Any, Dict, Optional


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

            f_lower = f.lower()
            if "nsis" in f_lower or "windows" in f_lower or "setup" in f_lower:
                if "aarch64" in f_lower or "arm64" in f_lower:
                    platforms["windows-aarch64"] = {"signature": sig_content, "url": asset_url}
                else:
                    platforms["windows-x86_64"] = {"signature": sig_content, "url": asset_url}
            elif "appimage" in f_lower or "linux" in f_lower:
                if "aarch64" in f_lower or "arm64" in f_lower:
                    platforms["linux-aarch64"] = {"signature": sig_content, "url": asset_url}
                else:
                    platforms["linux-x86_64"] = {"signature": sig_content, "url": asset_url}
            elif "darwin" in f_lower or "macos" in f_lower or "app" in f_lower:
                platforms["darwin-aarch64"] = {"signature": sig_content, "url": asset_url}
                platforms["darwin-x86_64"] = {"signature": sig_content, "url": asset_url}

    return {
        "version": clean_version,
        "notes": notes or f"OpenRAC Launcher release {clean_version}",
        "pub_date": pub_date,
        "platforms": platforms,
    }


def main():
    parser = argparse.ArgumentParser(description="Generate latest.json for Tauri v2 updater")
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
