#!/usr/bin/env python3
"""Module gate: rebuild a module with its proved bodies and compare it.

For every module that carries proved bodies, the gate

  1. rebuilds each body from its **published source**, with the pinned chain
     and the measured relocation context, and takes the linked bytes;
  2. writes those bytes at their measured offsets in a copy of the retail
     module;
  3. compares the rebuilt module with the retail module, byte for byte, and
     compares their SHA-256.

With ``--baseline``, the code interval of the module is not copied from the
retail image but **rebuilt from the assembly baseline**: the interval is
rendered as R5900, assembled and linked, and the proved bodies are written
over it.  The module that comes out is a rebuild -- of its code -- and not a
copy; its data, which no C source covers yet, is still the retail's.

With ``--image``, the gate goes one step further: it writes the rebuilt
modules back into a copy of the disc image, at their recorded extents, and
requires the image to hash to the pinned retail hash.

What it proves: the bodies belong where they are placed, they do not
disturb their surroundings, and the module -- and the image that carries
it -- that contains them is the retail one.  What it does **not** prove:
that the parts of the program that have no accepted source yet are rebuilt
from source -- with ``--baseline`` they are rebuilt from the *assembly*
baseline, and without it they are still the retail bytes.  The full rebuild of
a module, and then of the disc, is the stage that remains.

Usage:
    python3 tools/module_gate.py --modules <private dir> --harness <private harness.py>
    python3 tools/module_gate.py --modules <private dir> --harness <private harness.py> \
                                 --baseline --assembler <Ps2EeAs.exe> --linker <ld.exe>
    python3 tools/module_gate.py --modules <private dir> --harness <private harness.py> \
                                 --image <private image>
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

from disc import IsoDisc  # noqa: E402
from placement_table import join  # noqa: E402
from code_intervals import code_end_from_relocations  # noqa: E402
from r5900_assembly import render_assembly, run_tool  # noqa: E402

TABLE = ROOT / "config/levels/bodies.tsv"
SOURCES = ROOT / "config/levels/body_sources.tsv"
BOOT_TABLE = ROOT / "config/boot/bodies.tsv"
BOOT_SOURCES = ROOT / "config/boot/body_sources.tsv"
TARGET = ROOT / "config/target.json"
BOOT_NAME = "SCUS_976.23"
CODE_START = 0x80


def records(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        return []
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--modules", type=Path, required=True)
    parser.add_argument("--harness", type=Path, required=True)
    parser.add_argument("--boot-harness", type=Path, default=None)
    parser.add_argument("--profiles", type=Path, default=None,
                        help="private table of the compiler profiles")
    parser.add_argument("--boot", type=Path, default=None,
                        help="the boot executable; it is rebuilt with its proved "
                             "bodies and its hash is compared with the pinned one")
    parser.add_argument("--baseline", action="store_true",
                        help="rebuild the code interval from the assembly "
                             "baseline instead of copying the retail bytes")
    parser.add_argument("--assembler", type=Path, default=None)
    parser.add_argument("--linker", type=Path, default=None)
    parser.add_argument("--baseline-work", type=Path, default=Path("/tmp/rac-sac-baseline"))
    parser.add_argument("--image", type=Path, default=None,
                        help="disc image; the rebuilt modules are written back "
                             "into it and the image hash is compared with the "
                             "pinned retail hash")
    args = parser.parse_args()

    rows = join(TABLE, SOURCES)
    profiles: dict[str, dict] = {}
    if args.profiles and args.profiles.is_file():
        profiles = json.loads(args.profiles.read_text(encoding="utf-8"))
    per_module: dict[str, list[dict]] = {}
    for row in rows:
        per_module.setdefault(row["module"], []).append(row)

    problems: list[str] = []
    rebuilt_bytes = 0
    rebuilt_modules: dict[str, bytes] = {}
    for module, entries in sorted(per_module.items()):
        path = args.modules / module
        if not path.is_file():
            problems.append(f"{module}: not extracted")
            continue
        retail = path.read_bytes()
        rebuilt = bytearray(retail)
        if args.baseline:
            if not args.assembler or not args.linker:
                problems.append(f"{module}: --baseline needs --assembler and --linker")
                continue
            interval = baseline_interval(module, retail, args.assembler, args.linker,
                                         args.baseline_work, problems)
            if interval is None:
                continue
            rebuilt[CODE_START:CODE_START + len(interval)] = interval
        for row in entries:
            offset = int(row["offset"], 0)
            size = int(row["size"])
            source = ROOT / row["source"]
            with tempfile.TemporaryDirectory() as directory:
                emit = Path(directory) / "body.bin"
                command = [sys.executable, str(args.harness), module, row["offset"],
                           str(source), "--one-line", "--emit", str(emit)]
                profile = profiles.get((row.get("profile") or "").split(":")[0])
                if profile:
                    command += ["--cpp", profile["cpp"], "--cc1", profile["cc1"],
                                "--flags", profile["flags"], "--work", profile["work"]]
                result = subprocess.run(command, capture_output=True, text=True)
                if "\tEXACT\t" not in result.stdout:
                    problems.append(f"{module} {row['offset']}: the harness refuses "
                                    f"the body ({result.stdout.strip()})")
                    continue
                body = emit.read_bytes()
            if len(body) != size:
                problems.append(f"{module} {row['offset']}: linked body is "
                                f"{len(body)} bytes, the table says {size}")
                continue
            rebuilt[offset:offset + size] = body
            rebuilt_bytes += size
        before = hashlib.sha256(retail).hexdigest()
        after = hashlib.sha256(bytes(rebuilt)).hexdigest()
        status = "identical" if rebuilt == retail else "DIFFERENT"
        if rebuilt != retail:
            problems.append(f"{module}: the rebuilt module differs from the retail "
                            f"module ({status})")
        rebuilt_modules[module] = bytes(rebuilt)
        print(f"  {module:16s} {len(entries):3d} placement(s), "
              f"{sum(int(row['size']) for row in entries):6d} rebuilt byte(s)  "
              f"{status}  sha256 {after[:16]}")

    print(f"{rebuilt_bytes} byte(s) rebuilt from published sources, "
          f"{len(rows)} placement(s)")

    boot_rows = join(BOOT_TABLE, BOOT_SOURCES)
    if boot_rows and args.boot is None:
        print("  boot             not measured: give --boot and --boot-harness")
    elif boot_rows:
        if args.boot_harness is None:
            problems.append("the boot has proved bodies but no boot harness was given")
        else:
            rebuilt_boot = rebuild_boot(args.boot, args.boot_harness, boot_rows,
                                        problems, profiles)
            if rebuilt_boot is not None:
                target = json.loads(TARGET.read_text(encoding="utf-8"))
                digest = hashlib.sha256(rebuilt_boot).hexdigest()
                if digest != target["boot_sha256"]:
                    problems.append("the rebuilt boot does not hash to the pinned "
                                    "retail hash")
                print(f"  boot             {len(boot_rows):3d} placement(s), "
                      f"{sum(int(row['size']) for row in boot_rows):6d} rebuilt "
                      f"byte(s)  sha256 {digest[:16]} "
                      f"({'matches' if digest == target['boot_sha256'] else 'DIFFERS'}"
                      " the pinned hash)")

    if args.image:
        rebuilt_image = rebuild_image(args.image, rebuilt_modules, problems)
        if rebuilt_image is not None:
            target = json.loads(TARGET.read_text(encoding="utf-8"))
            digest = hashlib.sha256(rebuilt_image).hexdigest()
            if digest != target["iso_sha256"]:
                problems.append("the rebuilt image does not hash to the pinned "
                                "retail hash")
            print(f"  image             {len(rebuilt_modules)} module(s) written "
                  f"back at their extents, sha256 {digest[:16]} "
                  f"({'matches' if digest == target['iso_sha256'] else 'DIFFERS'} "
                  "the pinned hash)")

    if problems:
        print("module gate failed:")
        for problem in problems:
            print(f"  {problem}")
        return 1
    print("module gate: every rebuilt module is the retail module")
    return 0



def baseline_interval(module: str, data: bytes, assembler: Path, linker: Path,
                      work: Path, problems: list[str]) -> bytes | None:
    """Rebuild the code interval of a module from the assembly baseline."""
    from elf import inspect_elf

    end = code_end_from_relocations(data)
    size = end - CODE_START
    words = struct.unpack_from("<" + "I" * (size // 4), data, CODE_START)
    assembly, _stats = render_assembly(words, CODE_START, end)
    folder = work / module.replace(".REL", "").lower()
    folder.mkdir(parents=True, exist_ok=True)
    (folder / "module.s").write_text(assembly, encoding="ascii", newline="\n")
    (folder / "module.ld").write_text(
        f"T_ZERO = 0;\nSECTIONS {{ . = 0x{CODE_START:08X}; .text : {{ *(.text) }} }}\n",
        encoding="ascii", newline="\n")
    try:
        run_tool([str(assembler), "-o", "module.o", "module.s"], folder,
                 folder / "as.log")
        run_tool([str(linker), "-T", "module.ld", "-o", "module.elf", "module.o"],
                 folder, folder / "ld.log")
    except ValueError as error:
        problems.append(f"{module}: the assembly baseline failed ({error})")
        return None
    linked = (folder / "module.elf").read_bytes()
    section = next(item for item in inspect_elf(folder / "module.elf")["sections"]
                   if item["name"] == ".text")
    actual = linked[int(section["file_offset"]):
                   int(section["file_offset"]) + int(section["size"])]
    if actual != data[CODE_START:end]:
        problems.append(f"{module}: the assembly baseline differs from the retail "
                        "code interval")
        return None
    return actual


def rebuild_boot(boot: Path, harness: Path, rows: list[dict],
                 problems: list[str],
                 profiles: dict[str, dict] | None = None) -> bytes | None:
    """Rebuild the boot executable with its proved bodies."""
    profiles = profiles or {}
    from elf import inspect_elf
    report = inspect_elf(boot)
    section = next(item for item in report["sections"] if item["name"] == ".text")
    base = int(str(section["virtual_address"]), 16)
    file_offset = int(section["file_offset"])
    data = bytearray(boot.read_bytes())
    for row in rows:
        address = int(row["offset"], 0)
        size = int(row["size"])
        source = ROOT / row["source"]
        with tempfile.TemporaryDirectory() as directory:
            emit = Path(directory) / "body.bin"
            command = [sys.executable, str(harness), row["offset"], str(source),
                       "--one-line", "--emit", str(emit)]
            profile = profiles.get((row.get("profile") or "").split(":")[0])
            if profile:
                command += ["--cpp", profile["cpp"], "--cc1", profile["cc1"],
                            "--flags", profile["flags"], "--work", profile["work"]]
            result = subprocess.run(command, capture_output=True, text=True)
            if "\tEXACT\t" not in result.stdout:
                problems.append(f"boot {row['offset']}: the harness refuses the body "
                                f"({result.stdout.strip()})")
                continue
            body = emit.read_bytes()
        if len(body) != size:
            problems.append(f"boot {row['offset']}: linked body is {len(body)} "
                            f"bytes, the table says {size}")
            continue
        at = file_offset + (address - base)
        data[at:at + size] = body
    return bytes(data)


def rebuild_image(image: Path, modules: dict[str, bytes],
                  problems: list[str]) -> bytes | None:
    """Write the rebuilt modules back into a copy of the disc image."""
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    with IsoDisc(image) as disc:
        entries = {}
        for name in modules:
            path = f"BIN/{name}"
            try:
                entries[path] = disc.find(path)
            except KeyError:
                problems.append(f"{path}: not on the disc")
                return None
        data = bytearray(image.read_bytes())
    for path, entry in entries.items():
        body = modules[Path(path).name]
        start = entry.extent * 2048
        data[start:start + len(body)] = body
    return bytes(data)


if __name__ == "__main__":
    raise SystemExit(main())
