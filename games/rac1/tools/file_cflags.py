#!/usr/bin/env python3
"""
Extra compiler flags for whole source files (config/file_cflags.txt).

  python3 tools/file_cflags.py src/path.c     # prints that file's extra flags, or nothing

GCC 2.95 takes its options per translation unit, so a source file is the
smallest thing a flag can apply to: retail could only have built a function
with other flags by building its object with them. When one function needs a
flag its neighbours do not, the function goes in a file of its own
(docs/BUILD_FIDELITY.md, "Flags"). Makefile.sn and tools/try_func.py both
read this list, so a file builds the same way in both.
"""
import sys
from pathlib import Path

CONFIG = Path(__file__).resolve().parent.parent / "config/file_cflags.txt"


def load() -> dict[str, list[str]]:
    out = {}
    if CONFIG.exists():
        for line in CONFIG.read_text().splitlines():
            line = line.split("#", 1)[0].strip()
            if line:
                path, *flags = line.split()
                out[path] = flags
    return out


def flags_for(path) -> list[str]:
    return load().get(str(path), [])


if __name__ == "__main__":
    print(" ".join(flags_for(sys.argv[1])))
