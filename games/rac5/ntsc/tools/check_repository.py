#!/usr/bin/env python3
"""Reject game data, build output and proprietary binaries from the Git index."""

from __future__ import annotations

import subprocess
from pathlib import Path

from c_source_policy import policy_errors

ROOT = Path(__file__).resolve().parent.parent
PRIVATE_DIRS = ("baserom/", "asm/", "build/", "toolchain/", "work/", "assets/")
PRIVATE_SUFFIXES = (
    ".iso", ".elf", ".bin", ".wad", ".bwd", ".rel", ".irx", ".exe",
    ".dll", ".7z", ".zip", ".img", ".cnf", ".bik", ".vag",
    ".pss", ".o", ".a", ".map", ".log",
)


def forbidden_reason(name: str) -> str | None:
    path = name.replace("\\", "/")
    lower = path.lower()
    if lower.startswith("scratch/") or any(part.startswith("_scratch") for part in lower.split("/")):
        return "private scratch experiment directory"
    if path.startswith(PRIVATE_DIRS):
        return "private runtime directory"
    if lower.endswith(PRIVATE_SUFFIXES):
        return "game/toolchain/build file extension"
    if lower == ".env" or lower.startswith(".env.") or "/.env" in lower:
        return "environment file"
    return None


def check() -> list[str]:
    result = subprocess.run(["git", "ls-files", "--cached", "-z"], cwd=ROOT,
                            capture_output=True, check=True)
    problems: list[str] = []
    for raw in result.stdout.split(b"\0"):
        if not raw:
            continue
        name = raw.decode("utf-8", "replace")
        reason = forbidden_reason(name)
        if reason:
            problems.append(f"{name}: {reason}")
            continue
        path = ROOT / name
        if path.exists():
            if path.stat().st_size > 1_000_000:
                problems.append(f"{name}: unexpectedly large tracked file")
            elif path.read_bytes()[:4] == b"\x7fELF":
                problems.append(f"{name}: ELF bytes in a tracked file")
            if path.suffix.lower() in {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"}:
                problems.extend(f"{name}: {reason}" for reason in
                                policy_errors(path.read_text(encoding="utf-8")))
    return problems


if __name__ == "__main__":
    found = check()
    if found:
        raise SystemExit("repository check failed:\n" + "\n".join(found))
    print("repository check: no game data, build output or toolchain in Git index")
