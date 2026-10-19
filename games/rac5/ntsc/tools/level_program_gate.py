#!/usr/bin/env python3
"""Verify complete private SNR2 builds for measured frontend/level placements.

This replaces an assembly-built candidate code interval inside the pinned
module, checks every resulting file byte, then checks the measured relocated
load image. It is a baseline for later C replacements, not a C match.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from boot_symbols import read_boot
from disc import IsoDisc, verify_disc
from elf import inspect_elf
from r5900_assembly import ASSEMBLER_SHA256, LINKER_SHA256, check_tool
from relocate import relocate_module
from snr2_assembly import CODE_START, OUTPUT as ASSEMBLY_OUTPUT, pinned_modules, verify_module

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "build/level-program-gate"
REPORT = OUTPUT / "report.json"

# Paused EE memory immediately after snDllLoaded and before constructors.
MEASURED_LOADS = {
    "FRONTEND.REL": (0x00F97B80,
                     "9d97396dbc8dca5d26b139d303fefdc9a7001eadb308a819bf49fb35f545d6d6",
                     ("PAUSEMODE_GetCurrentPauseScreen__Fv",)),
    "LEVEL_01.REL": (0x00D4B380,
                     "a64703fde6edcd09007421bcc93a17fc02929477f6f07f5918d468db6aed3407",
                     ()),
}


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def replace_range(source: bytes, replacement: bytes, start: int, end: int) -> bytes:
    if not CODE_START <= start < end <= len(source) or len(replacement) != end - start:
        raise ValueError("assembled SNR2 code interval has the wrong extent")
    return source[:start] + replacement + source[end:]


def replace_code(source: bytes, replacement: bytes, end: int) -> bytes:
    return replace_range(source, replacement, CODE_START, end)


def verify_one(module: str, source: bytes, expected_sha: str,
               boot_symbols: dict[str, int], assembler: Path,
               linker: Path) -> dict[str, object]:
    end, code_sha = verify_module(source, module, expected_sha, assembler, linker)[1:3]
    linked_path = ASSEMBLY_OUTPUT / module.removesuffix(".REL").lower() / "module.elf"
    linked = inspect_elf(linked_path)
    section = next(item for item in linked["sections"] if item["name"] == ".text")
    assembled = linked_path.read_bytes()[section["file_offset"]:
                                        section["file_offset"] + section["size"]]
    rebuilt = replace_code(source, assembled, end)
    if sha(rebuilt) != expected_sha or sha(assembled) != code_sha:
        raise ValueError(f"{module} complete file or code interval differs")
    base, expected_load_sha, expected_missing = MEASURED_LOADS[module]
    relocated, missing = relocate_module(f"LVL/{module}", rebuilt, base, boot_symbols)
    if sha(relocated) != expected_load_sha or missing != expected_missing:
        raise ValueError(f"{module} relocated image differs from measured EE baseline")
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / module).write_bytes(rebuilt)
    print(f"{module}: {len(rebuilt)}/{len(rebuilt)} complete file bytes exact; "
          f"relocated SHA-256 {sha(relocated)}")
    return {"module": module, "size": len(rebuilt), "sha256": expected_sha,
            "code_interval_bytes": len(assembled), "code_interval_sha256": code_sha,
            "measured_base": f"0x{base:08X}", "relocated_sha256": expected_load_sha,
            "missing_imports_resolved_to_zero": list(missing), "counted_matches": 0}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path, help="personally obtained pinned USA ISO")
    parser.add_argument("--boot", type=Path, default=ROOT / "baserom/SCUS_976.15")
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--linker", type=Path, required=True)
    args = parser.parse_args()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({"status": "incomplete", "counted_matches": 0},
                                 indent=2) + "\n", encoding="utf-8", newline="\n")
    check_tool(args.assembler, ASSEMBLER_SHA256)
    check_tool(args.linker, LINKER_SHA256)
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    boot, symbols = read_boot(args.boot)
    if boot["sha256"] != target["boot_sha256"]:
        raise ValueError("boot differs from pinned USA target")
    addresses = {symbol.name: symbol.address for symbol in symbols}
    known = pinned_modules()
    results = []
    with IsoDisc(args.image) as disc:
        verify_disc(disc, target)
        for module in MEASURED_LOADS:
            source = disc.file_bytes("LVL/" + module)
            results.append(verify_one(module, source, known[module], addresses,
                                      args.assembler, args.linker))
    REPORT.write_text(json.dumps({"status": "passed_assembly_only",
                                  "modules": results, "counted_matches": 0},
                                 indent=2) + "\n", encoding="utf-8", newline="\n")
    print("two measured SNR2 programs exact; zero accepted level C matches")


if __name__ == "__main__":
    main()
