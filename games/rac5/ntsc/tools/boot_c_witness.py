#!/usr/bin/env python3
"""Link the accepted boot C object into the complete pinned boot baseline.

This is an object-level witness; acceptance still requires boot_trial_gate.py.
The caller builds the hash-pinned object from src/boot/memcard_busy.c;
all game-derived assembly and linked output stay under ignored build/.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from boot_assembly import (DATA_SECTIONS, ROOT, TEXT_SIZE, TEXT_START,
                           render_data, text_words, verify_load_image)
from compare_boot_slice import compare
from elf import inspect_elf
from r5900_assembly import (ASSEMBLER_SHA256, LINKER_SHA256, check_tool,
                            render_assembly, run_tool)

OUTPUT = ROOT / "build/boot-c-witness"
SOURCE = ROOT / "src/boot/memcard_busy.c"
SOURCE_SHA256 = "87b1494dad2d079980703f0275383e283a7eb000bf9d8d313d281175ed24705b"
OBJECT_SHA256 = "415800c82c13e302c09de6aa773a68f82414410e28375e6b726db591fbdeb674"
C_START = 0x01E627C0
C_END = 0x01E627D0
BUSY_STATE = 0x01F4A654
BSS_START = 0x01F28100
BSS_SIZE = 817644
LOAD_MEMORY_SIZE = 1627648
PREFIX_SIZE = 128
TRAIL_SIZE = 20


def check_source() -> None:
    normalized = SOURCE.read_text(encoding="utf-8").replace("\r\n", "\n")
    if hashlib.sha256(normalized.encode("utf-8")).hexdigest() != SOURCE_SHA256:
        raise ValueError("MEMCARD_Busy C source differs from measured trial")


def linker_script() -> str:
    lines = [f"busy_state = 0x{BUSY_STATE:08X};", "SECTIONS {",
             f"  . = 0x{TEXT_START:08X};",
             "  .text : { boot_prefix_align.o(.text.align) "
             "boot_split_trim.o(.text.before) memcard_busy.o(.text) "
             "boot_split_trim.o(.text.after) }"]
    for name, address, _size in DATA_SECTIONS:
        lines.append(f"  . = 0x{address:08X}; {name} : {{ boot_data.o({name}) }}")
    lines.append(f"  . = 0x{BSS_START:08X}; .bss : {{ boot_bss.o(.bss) }}")
    lines.append("  .tail (NOLOAD) : { boot_tail.o(.tail) }")
    lines.append("}")
    return "\n".join(lines) + "\n"


def split_aligned_prefix(assembly: str) -> tuple[str, str]:
    """Keep the original first 128 instruction bytes in an aligned input section."""
    lines = assembly.splitlines()
    header = [".set noat", ".set noreorder", ".section .text.before",
              ".globl boot_text_start", "boot_text_start:"]
    if lines[:len(header)] != header:
        raise ValueError("boot assembly header differs from measured layout")
    body = lines[len(header):]
    instructions = 0
    split = None
    for index, line in enumerate(body):
        if line.startswith("    "):
            instructions += 1
            if instructions == PREFIX_SIZE // 4:
                split = index + 1
                break
    if split is None or ".section .text.after" in body[:split]:
        raise ValueError("boot assembly lacks the measured 128-byte prefix")
    prefix = header[:2] + [".section .text.align", ".align 7"] + header[3:] + body[:split]
    remainder = header[:3] + body[split:]
    return "\n".join(prefix) + "\n", "\n".join(remainder) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("boot", type=Path)
    parser.add_argument("--c-object", type=Path, required=True)
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--linker", type=Path, required=True)
    args = parser.parse_args()
    check_source()
    check_tool(args.assembler, ASSEMBLER_SHA256)
    check_tool(args.linker, LINKER_SHA256)
    if hashlib.sha256(args.c_object.read_bytes()).hexdigest() != OBJECT_SHA256:
        raise ValueError("MEMCARD_Busy object differs from measured GNU EE trial")
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    words = text_words(args.boot, target["boot_sha256"])
    assembly, stats = render_assembly(words, TEXT_START, symbol="boot_text_start",
                                      raw_cop1_branches=True, exclude=(C_START, C_END))
    if (stats["excluded_words"] != 4 or stats["sqrt_s"] != 50
            or stats["cop1_branch_words"] != 6):
        raise ValueError("split assembly differs from measured boot layout")
    data_source, _baseline_script = render_data(args.boot)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    prefix, remainder = split_aligned_prefix(assembly)
    (OUTPUT / "boot_prefix_align.s").write_text(prefix, encoding="ascii", newline="\n")
    (OUTPUT / "boot_split_trim.s").write_text(remainder, encoding="ascii", newline="\n")
    (OUTPUT / "boot_data.s").write_text(data_source, encoding="ascii", newline="\n")
    (OUTPUT / "boot_bss.s").write_text(
        f".section .bss\n.space {BSS_SIZE}\n", encoding="ascii", newline="\n")
    (OUTPUT / "boot_tail.s").write_text(
        f'.section .tail,"aw",@nobits\n.space {TRAIL_SIZE}\n',
        encoding="ascii", newline="\n")
    (OUTPUT / "boot_c.ld").write_text(linker_script(), encoding="ascii", newline="\n")
    shutil.copyfile(args.c_object, OUTPUT / "memcard_busy.o")
    for stem in ("boot_prefix_align", "boot_split_trim"):
        run_tool([str(args.assembler.resolve()), "-o", f"{stem}.o", f"{stem}.s"],
                 OUTPUT, OUTPUT / f"{stem}_assembler.log")
    run_tool([str(args.assembler.resolve()), "-o", "boot_data.o", "boot_data.s"],
             OUTPUT, OUTPUT / "data_assembler.log")
    run_tool([str(args.assembler.resolve()), "-o", "boot_bss.o", "boot_bss.s"],
             OUTPUT, OUTPUT / "bss_assembler.log")
    run_tool([str(args.assembler.resolve()), "-o", "boot_tail.o", "boot_tail.s"],
             OUTPUT, OUTPUT / "tail_assembler.log")
    run_tool([str(args.linker.resolve()), "-N", "-T", "boot_c.ld", "-o", "boot_c.elf",
              "boot_prefix_align.o", "boot_split_trim.o", "memcard_busy.o",
              "boot_data.o", "boot_bss.o", "boot_tail.o"],
             OUTPUT, OUTPUT / "linker.log")
    candidate = OUTPUT / "boot_c.elf"
    candidate_report = inspect_elf(candidate)
    section = next(item for item in candidate_report["sections"] if item["name"] == ".text")
    if section["size"] != TEXT_SIZE:
        raise ValueError("C-linked boot text size differs from retail")
    exact, count, digest = compare(args.boot, candidate, TEXT_START, TEXT_SIZE,
                                   target["boot_sha256"])
    if not exact:
        raise ValueError(f"C-linked boot text differs: {count}/{TEXT_SIZE} bytes equal")
    load_digest = verify_load_image(args.boot, candidate)
    reference_report = inspect_elf(args.boot)
    reference_program = next(item for item in reference_report["programs"] if item["type"] == 1)
    candidate_program = next(item for item in candidate_report["programs"] if item["type"] == 1)
    reference_bytes = args.boot.read_bytes()[
        reference_program["file_offset"]:
        reference_program["file_offset"] + reference_program["file_size"]]
    candidate_bytes = candidate.read_bytes()[
        candidate_program["file_offset"]:
        candidate_program["file_offset"] + candidate_program["file_size"]]
    reference_bss = next(item for item in reference_report["sections"] if item["name"] == ".bss")
    candidate_bss = next(item for item in candidate_report["sections"] if item["name"] == ".bss")
    tail = next(item for item in candidate_report["sections"] if item["name"] == ".tail")
    if (reference_program["memory_size"] != LOAD_MEMORY_SIZE
            or candidate_program != reference_program
            or candidate_bytes != reference_bytes
            or reference_bss["virtual_address"] != candidate_bss["virtual_address"]
            or candidate_bss["virtual_address"] != f"0x{BSS_START:08X}"
            or reference_bss["size"] != candidate_bss["size"]
            or candidate_bss["size"] != BSS_SIZE or candidate_bss["type"] != 8
            or tail["type"] != 8 or tail["size"] != TRAIL_SIZE
            or int(tail["virtual_address"], 16) != BSS_START + BSS_SIZE):
        raise ValueError("C-linked PT_LOAD/BSS layout or raw bytes differ from measured boot")
    print(f"C-linked boot .text: {count}/{TEXT_SIZE} bytes exact; SHA-256 {digest}")
    print(f"C-linked raw PT_LOAD: {len(reference_bytes)}/{len(reference_bytes)} bytes exact; "
          f"SHA-256 {load_digest}")
    print(f"C-linked BSS section: 0x{BSS_START:08X}, {BSS_SIZE} zero-fill bytes")
    print("C-linked PT_LOAD program header: all fields equal pinned retail")
    print("Provisional witness only: runtime and compiler catalogue gates remain open")


if __name__ == "__main__":
    main()
