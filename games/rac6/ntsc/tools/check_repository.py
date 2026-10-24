#!/usr/bin/env python3
"""Guard the public repository against game data and build output.

The check is structural: it looks at the files Git is about to publish and
refuses the ones that must never be published.  It cannot judge whether a
source file leaks something in a comment, so it is a floor, not a substitute
for reading the diff.

Usage:
    python3 tools/check_repository.py
    python3 tools/check_repository.py --staged
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Never publishable: extensions of retail or build artifacts.
FORBIDDEN_SUFFIXES = {
    ".iso", ".zip", ".7z", ".rar", ".bin", ".img", ".cue", ".mdf", ".ecm",
    ".elf", ".o", ".a", ".so", ".dll", ".exe", ".dvd", ".pkg", ".app",
    ".pyc", ".pdb", ".map", ".dmp", ".core",
}
# Never publishable: directories that hold derived or private material.
FORBIDDEN_PARTS = {
    "baserom", "build", "modules", "bounds", "preuves", "dumps", "asm",
    "disasm", "logs", "gate", "receipts", "scratch", "abandonnes",
}
# A file larger than this is a red flag for a repository of written source.
SIZE_LIMIT = 4 * 1024 * 1024
# Instruction listings and raw dumps, by name.
FORBIDDEN_NAMES = {"disasm.txt", "disassembly.txt", "dump.txt", "listing.txt"}


def tracked(staged: bool) -> list[str]:
    command = ["git", "ls-files", "-z"]
    if staged:
        command = ["git", "diff", "--cached", "--name-only", "-z"]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        print("git ls-files failed:", result.stderr.strip())
        raise SystemExit(2)
    return [name for name in result.stdout.split("\0") if name]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--staged", action="store_true",
                        help="check the staged files instead of the whole index")
    args = parser.parse_args()

    problems: list[str] = []
    for name in tracked(args.staged):
        path = ROOT / name
        suffix = path.suffix.lower()
        parts = set(Path(name).parts)
        if suffix in FORBIDDEN_SUFFIXES:
            problems.append(f"{name}: forbidden file type")
        if parts & FORBIDDEN_PARTS:
            problems.append(f"{name}: forbidden directory")
        if path.name.lower() in FORBIDDEN_NAMES:
            problems.append(f"{name}: instruction listing")
        if path.is_file() and path.stat().st_size > SIZE_LIMIT:
            problems.append(f"{name}: file larger than {SIZE_LIMIT} bytes")

    if problems:
        print("repository check failed:")
        for problem in problems:
            print(f"  {problem}")
        return 1
    print("repository check: no game data, build output or toolchain in the index")
    return 0


if __name__ == "__main__":
    sys.exit(main())
