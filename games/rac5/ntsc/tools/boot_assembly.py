#!/usr/bin/env python3
"""Rebuild the pinned USA boot text and file-backed load image privately.

Requires optional rabbitizer 1.16.2 and privately supplied SN assembler/linker.
All game-derived output stays under ignored build/boot-assembly/.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

from compare_boot_slice import compare
from elf import inspect_elf
from r5900_assembly import (ASSEMBLER_SHA256, LINKER_SHA256, check_tool,
                            render_assembly, run_tool)

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "build/boot-assembly"
TEXT_START = 0x01E62500
TEXT_SIZE = 0x79CD4
LOAD_FILE_SIZE = 809872
DATA_SECTIONS = (
    (".data", 0x01EDC200, 191184),
    (".sndata", 0x01F0AD00, 70000),
    (".rodata", 0x01F1BE80, 49368),
    (".gcc_except_table", 0x01F27F80, 272),
)
def text_words(boot: Path, expected_hash: str) -> tuple[int, ...]:
    image = boot.read_bytes()
    if hashlib.sha256(image).hexdigest() != expected_hash:
        raise ValueError("boot ELF differs from pinned USA target")
    report = inspect_elf(boot)
    section = next((item for item in report["sections"] if item["name"] == ".text"), None)
    if (section is None or int(str(section["virtual_address"]), 16) != TEXT_START
            or section["size"] != TEXT_SIZE or not section["flags"] & 4):
        raise ValueError("pinned boot .text metadata differs from measured target")
    offset = int(section["file_offset"])
    return struct.unpack_from("<" + "I" * (TEXT_SIZE // 4), image, offset)


def render_data(boot: Path) -> tuple[str, str]:
    """Copy non-code sections into ignored assembly for a load-image baseline."""
    image = boot.read_bytes()
    report = inspect_elf(boot)
    program = next((item for item in report["programs"] if item["type"] == 1), None)
    if (program is None or int(str(program["virtual_address"]), 16) != TEXT_START
            or program["file_size"] != LOAD_FILE_SIZE):
        raise ValueError("pinned boot load segment differs from measured target")
    source: list[str] = []
    script = ["SECTIONS {", f"  . = 0x{TEXT_START:08X}; .text : {{ *(.text) }}"]
    for name, address, size in DATA_SECTIONS:
        section = next((item for item in report["sections"] if item["name"] == name), None)
        if (section is None or int(str(section["virtual_address"]), 16) != address
                or section["size"] != size or not section["flags"] & 2):
            raise ValueError(f"pinned boot {name} metadata differs from measured target")
        data = image[section["file_offset"]:section["file_offset"] + size]
        source.append(f".section {name}")
        for offset in range(0, size, 16):
            source.append(".byte " + ", ".join(f"0x{byte:02X}" for byte in data[offset:offset + 16]))
        script.append(f"  . = 0x{address:08X}; {name} : {{ *({name}) }}")
    script.append("}")
    return "\n".join(source) + "\n", "\n".join(script) + "\n"


def verify_load_image(boot: Path, rebuilt: Path) -> str:
    original = boot.read_bytes()
    reference_report = inspect_elf(boot)
    candidate = rebuilt.read_bytes()
    candidate_report = inspect_elf(rebuilt)
    program = next(item for item in reference_report["programs"] if item["type"] == 1)
    reference = original[program["file_offset"]:program["file_offset"] + LOAD_FILE_SIZE]
    load = bytearray(LOAD_FILE_SIZE)
    for name, address, size in ((".text", TEXT_START, TEXT_SIZE), *DATA_SECTIONS):
        section = next((item for item in candidate_report["sections"]
                        if item["name"] == name), None)
        if (section is None or int(str(section["virtual_address"]), 16) != address
                or section["size"] != size):
            raise ValueError(f"linked {name} placement differs from measured target")
        offset = address - TEXT_START
        load[offset:offset + size] = candidate[
            section["file_offset"]:section["file_offset"] + size]
    if load != reference:
        count = sum(left == right for left, right in zip(load, reference))
        raise ValueError(f"file-backed boot load image differs: {count}/{LOAD_FILE_SIZE} bytes equal")
    return hashlib.sha256(load).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("boot", type=Path)
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--linker", type=Path, required=True)
    args = parser.parse_args()
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    check_tool(args.assembler, ASSEMBLER_SHA256)
    check_tool(args.linker, LINKER_SHA256)
    words = text_words(args.boot, target["boot_sha256"])
    assembly, stats = render_assembly(words, TEXT_START, symbol="boot_text_start",
                                      raw_cop1_branches=True)
    if stats["cop1_branch_words"] != 6 or stats["sqrt_s"] != 50 or stats["zero_target_jumps"]:
        raise ValueError("R5900 special-case counts differ from measured boot")
    link_script = f"SECTIONS {{ . = 0x{TEXT_START:08X}; .text : {{ *(.text) }} }}\n"
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / "boot_text.s").write_text(assembly, encoding="ascii", newline="\n")
    (OUTPUT / "boot_text.ld").write_text(link_script, encoding="ascii", newline="\n")
    run_tool([str(args.assembler.resolve()), "-o", "boot_text.o", "boot_text.s"],
             OUTPUT, OUTPUT / "assembler.log")
    run_tool([str(args.linker.resolve()), "-T", "boot_text.ld", "-o",
              "boot_text.elf", "boot_text.o"], OUTPUT, OUTPUT / "linker.log")
    rebuilt = OUTPUT / "boot_text.elf"
    section = next(item for item in inspect_elf(rebuilt)["sections"] if item["name"] == ".text")
    if section["size"] != TEXT_SIZE:
        raise ValueError(f"linked .text size {section['size']} differs from {TEXT_SIZE}")
    exact, equal_count, digest = compare(args.boot, rebuilt, TEXT_START,
                                         TEXT_SIZE, target["boot_sha256"])
    if not exact:
        raise ValueError(f"boot .text differs: {equal_count}/{TEXT_SIZE} bytes equal")
    data_source, full_script = render_data(args.boot)
    (OUTPUT / "boot_data.s").write_text(data_source, encoding="ascii", newline="\n")
    (OUTPUT / "boot_load.ld").write_text(full_script, encoding="ascii", newline="\n")
    run_tool([str(args.assembler.resolve()), "-o", "boot_data.o", "boot_data.s"],
             OUTPUT, OUTPUT / "data_assembler.log")
    run_tool([str(args.linker.resolve()), "-T", "boot_load.ld", "-o",
              "boot_load.elf", "boot_text.o", "boot_data.o"], OUTPUT,
             OUTPUT / "load_linker.log")
    load_digest = verify_load_image(args.boot, OUTPUT / "boot_load.elf")
    print(f"boot .text: {equal_count}/{TEXT_SIZE} bytes exact; SHA-256 {digest}")
    print(f"boot file-backed load: {LOAD_FILE_SIZE}/{LOAD_FILE_SIZE} bytes exact; "
          f"SHA-256 {load_digest} (non-code sections copied from pinned input)")
    print("private assembly: " + ", ".join(f"{key}={value}" for key, value in stats.items()))


if __name__ == "__main__":
    main()
