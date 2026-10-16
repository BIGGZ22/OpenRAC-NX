#!/usr/bin/env python3
"""Move generated original assembly into its tracked classification folders.

The assembly bytes remain local to each contributor. Compatibility symlinks
at asm/nonmatchings keep the differ and retail inventory tools working.
"""

import os
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LISTS = (
    ("config/handwritten_asm.txt", "handwritten"),
    ("config/linker_remnants.txt", "remnants"),
)
ENTRY = re.compile(r"(core_text|text)/(func_[0-9A-F]{8})")


def entries(path):
    for number, line in enumerate((ROOT / path).read_text().splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if not ENTRY.fullmatch(line):
            raise SystemExit(f"{path}:{number}: invalid entry {line!r}")
        yield line


def main():
    seen = set()
    counts = {}
    for manifest, folder in LISTS:
        count = 0
        for entry in entries(manifest):
            if entry in seen:
                raise SystemExit(f"duplicate assembly classification: {entry}")
            seen.add(entry)
            segment, name = entry.split("/")
            source = ROOT / "asm/nonmatchings" / segment / (name + ".s")
            target = ROOT / "asm" / folder / segment / (name + ".s")
            if source.is_symlink():
                if source.resolve() != target or not target.is_file():
                    raise SystemExit(f"stale assembly link: {source}")
            elif source.is_file() and not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                source.rename(target)
                source.symlink_to(os.path.relpath(target, source.parent))
            else:
                raise SystemExit(f"missing or duplicate generated assembly: {source}")
            count += 1
        counts[folder] = count
    print("organized assembly: " + ", ".join(f"{n} {folder}" for folder, n in counts.items()))


if __name__ == "__main__":
    main()
