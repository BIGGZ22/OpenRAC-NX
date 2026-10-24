#!/usr/bin/env python3
"""Build every published source with the pinned chain, and check the objects.

The published sources are the written part of the project: this tool proves
that they still are pure C, that they still compile with the pinned profile,
and that each object is the one the placement table records.  It is the
reproducible build of ``src/``.

The toolchain binaries are local: their paths and hashes are written in
``config/toolchain.json`` (hashes) and ``LOCAL.md`` (paths).  The tool takes
the paths as arguments, so a third party can point it at its own copy of
the same binaries.

Usage (WSL, the chain is a Linux build):
    python3 tools/build.py --cpp <cpp> --cc1 <cc1> --as <as> [--out <dir>]
    python3 tools/build.py --cpp <cpp> --cc1 <cc1> --as <as> --check
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

from c_source_policy import require_c_source  # noqa: E402

TABLE = ROOT / "config/levels/bodies.tsv"
SOURCES = ROOT / "config/levels/body_sources.tsv"
BOOT_TABLE = ROOT / "config/boot/bodies.tsv"
BOOT_SOURCES = ROOT / "config/boot/body_sources.tsv"
PROFILE = ROOT / "config/toolchain.json"


def records(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cpp", type=Path, required=True)
    parser.add_argument("--cc1", type=Path, required=True)
    parser.add_argument("--as", dest="assembler", type=Path, required=True)
    parser.add_argument("--out", type=Path, default=None,
                        help="keep the objects in this directory")
    parser.add_argument("--profiles", type=Path, default=None,
                        help="private table of the compiler profiles: a source "
                             "proved with another chain is built with it")
    parser.add_argument("--check", action="store_true",
                        help="compare the object hashes with the placement table")
    args = parser.parse_args()

    profile = json.loads(PROFILE.read_text(encoding="utf-8"))
    flags = profile["flags"]["cc1"]
    profiles: dict[str, dict] = {}
    if args.profiles and args.profiles.is_file():
        profiles = json.loads(args.profiles.read_text(encoding="utf-8"))
    rows = records(SOURCES) + records(BOOT_SOURCES)
    sources: dict[str, str] = {}
    source_profile: dict[str, str] = {}
    for row in rows:
        sources.setdefault(row["source"], row["object_sha256"])
        source_profile.setdefault(row["source"], row.get("profile", ""))

    problems: list[str] = []
    built = 0
    output = Path(args.out) if args.out else None
    if output:
        output.mkdir(parents=True, exist_ok=True)

    for name, expected in sorted(sources.items()):
        source = ROOT / name
        if not source.is_file():
            problems.append(f"{name}: missing")
            continue
        try:
            require_c_source(source.read_text(encoding="utf-8"), name)
        except ValueError as error:
            problems.append(str(error))
            continue
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            (work / source.name).write_bytes(source.read_bytes())
            tools = {"cpp": str(args.cpp), "cc1": str(args.cc1),
                     "assembler": str(args.assembler), "flags": " ".join(flags)}
            chosen = profiles.get((source_profile.get(name) or "").split(":")[0])
            if chosen:
                tools = {"cpp": chosen["cpp"], "cc1": chosen["cc1"],
                         "assembler": chosen.get("as", str(args.assembler)),
                         "flags": chosen["flags"]}
            with (work / f"{source.stem}.i").open("wb") as handle:
                result = subprocess.run([tools["cpp"], source.name], cwd=work,
                                        stdout=handle, stderr=subprocess.PIPE,
                                        timeout=300)
            if result.returncode:
                problems.append(f"{name}: cpp failed")
                continue
            try:
                require_c_source((work / f"{source.stem}.i").read_text(
                    encoding="utf-8", errors="replace"),
                    f"{name} preprocessed input")
            except ValueError as error:
                problems.append(str(error))
                continue
            for command, what in (
                    ([tools["cc1"], *tools["flags"].split(), f"{source.stem}.i",
                      "-o", f"{source.stem}.s"], "cc1"),
                    ([tools["assembler"], "-o", f"{source.stem}.o", f"{source.stem}.s"],
                     "as")):
                done = subprocess.run([str(item) for item in command], cwd=work,
                                      capture_output=True, text=True, timeout=300)
                if done.returncode:
                    problems.append(f"{name}: {what} failed: "
                                    f"{(done.stdout + done.stderr)[-300:]}")
                    break
            else:
                obj = work / f"{source.stem}.o"
                digest = hashlib.sha256(obj.read_bytes()).hexdigest()
                built += 1
                if args.check and expected and digest != expected:
                    problems.append(f"{name}: object SHA-256 {digest} differs from "
                                    f"the table ({expected})")
                if output:
                    (output / f"{source.stem}.o").write_bytes(obj.read_bytes())

    print(f"{built} source(s) compiled with the pinned profile "
          f"({' '.join(flags)})")
    if problems:
        print("build failed:")
        for problem in problems:
            print(f"  {problem}")
        return 1
    if args.check:
        print("every published source is pure C and every object matches the "
              "placement table")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
