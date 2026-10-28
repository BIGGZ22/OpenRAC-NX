#!/usr/bin/env python3
"""Run the translated PAL save scan without an ISO or a console.

Generate rac1-pal first (a Switch build does this), then run:
  python port/tests/run_pal_memory_card_scan_test.py --cc gcc
An alternate generated-source folder can be selected with --generated.
The test uses synthetic files in a temporary build subfolder, never real saves.
"""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    port = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--generated", type=Path,
                        default=port / "build-switch/games/rac1-pal/gen")
    args = parser.parse_args()
    generated = args.generated.resolve()
    source = generated / "game/memcard.c"
    if not source.is_file():
        parser.error("PAL generated source missing; build rac1-pal first")
    build = port.parent / "build"
    build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="pal-save-scan-", dir=build) as tmp:
        executable = Path(tmp) / ("scan.exe" if os.name == "nt" else "scan")
        subprocess.run([
            args.cc, "-std=gnu11", "-O0", "-ffunction-sections", "-fdata-sections",
            "-DOPENRAC_COMPACT_GUEST_MEMORY=1",
            '-DOPENRAC_PAL_MEMCARD_SOURCE="game/memcard.c"',
            "-I" + str(port / "runtime/include"),
            "-I" + str(port / "game/common/include"), "-I" + str(generated),
            str(port / "tests/pal_memory_card_scan_test.c"),
            str(port / "game/common/lib/mc.c"), "-Wl,--gc-sections",
            "-o", str(executable),
        ], check=True)
        for mode in ("write", "read"):
            subprocess.run([str(executable), str(Path(tmp) / "card"), mode], check=True)


if __name__ == "__main__":
    main()
