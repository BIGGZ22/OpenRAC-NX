#!/usr/bin/env python3
"""Audit and privately regenerate the pinned boot's VU overlay ELF sections.

The committed catalogue contains metadata only. Generated assembly and ELFs,
including names reconstructed from the user's pinned executable, stay in
ignored build/. This is a file-layout witness, not a C match or VU decomp.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from collections import Counter
from dataclasses import dataclass
from pathlib import Path

from elf import ELF_HEADER, SECTION_HEADER, inspect_elf
from r5900_assembly import (ASSEMBLER_SHA256, LINKER_SHA256, check_tool,
                            run_tool)

ROOT = Path(__file__).resolve().parent.parent
CATALOGUE = ROOT / "config/boot/dvp_overlays.tsv"
OUTPUT = ROOT / "build/dvp-overlays"
OVERLAY_TYPE = 0x7FFFF421
TABLE_TYPE = 0x7FFFF420
RECORD = struct.Struct("<III")


@dataclass(frozen=True)
class Overlay:
    name_offset: int
    name: str
    lma: int
    vma: int
    size: int


def parse_records(table: bytes, strings: bytes, sizes: dict[str, int]) -> list[Overlay]:
    if len(table) % RECORD.size:
        raise ValueError("VU overlay table has a partial record")
    rows: list[Overlay] = []
    for offset in range(0, len(table), RECORD.size):
        name_at, lma, vma = RECORD.unpack_from(table, offset)
        if name_at >= len(strings):
            raise ValueError("VU overlay name offset is out of range")
        end = strings.find(b"\0", name_at)
        if end < 0:
            raise ValueError("unterminated VU overlay name")
        name = strings[name_at:end].decode("ascii", "strict")
        if name not in sizes:
            raise ValueError(f"VU overlay name has no section: {name}")
        rows.append(Overlay(name_at, name, lma, vma, sizes[name]))
    return rows


def rebuild_strings(rows: list[Overlay], size: int) -> bytes:
    strings = bytearray(size)
    for row in rows:
        encoded = row.name.encode("ascii") + b"\0"
        if row.name_offset + len(encoded) > size:
            raise ValueError("VU overlay name exceeds string table")
        current = strings[row.name_offset:row.name_offset + len(encoded)]
        if any(a and a != b for a, b in zip(current, encoded)):
            raise ValueError("overlapping VU overlay names disagree")
        strings[row.name_offset:row.name_offset + len(encoded)] = encoded
    return bytes(strings)


def section_headers(image: bytes) -> list[tuple[int, ...]]:
    (_kind, _machine, _version, _entry, _phoff, shoff, _flags, _ehsize,
     _phentsize, _phnum, shentsize, shnum, _shstrndx) = ELF_HEADER.unpack_from(image, 16)
    return [SECTION_HEADER.unpack_from(image, shoff + index * shentsize)
            for index in range(shnum)]


def audit(boot: Path) -> tuple[list[Overlay], list[dict[str, object]], int]:
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    image = boot.read_bytes()
    if hashlib.sha256(image).hexdigest() != target["boot_sha256"]:
        raise ValueError("boot ELF differs from pinned USA target")
    report = inspect_elf(boot)
    sections = report["sections"]
    overlays = [section for section in sections
                if section["name"].startswith(".DVP.overlay.")]
    table_index = next(index for index, section in enumerate(sections)
                       if section["name"] == ".DVP.ovlytab")
    strings_index = next(index for index, section in enumerate(sections)
                         if section["name"] == ".DVP.ovlystrtab")
    table, names = sections[table_index], sections[strings_index]
    headers = section_headers(image)
    if (len(overlays) != 29 or table["type"] != TABLE_TYPE or table["flags"] != 1
            or names["type"] != 3 or names["flags"] != 0
            or headers[table_index][6] != strings_index
            or headers[table_index][9] != RECORD.size):
        raise ValueError("pinned VU overlay table layout differs from measured target")
    program = next(item for item in report["programs"] if item["type"] == 1)
    loaded_end = program["file_offset"] + program["file_size"]
    for section in overlays:
        content = image[section["file_offset"]:section["file_offset"] + section["size"]]
        if (section["type"] != OVERLAY_TYPE or section["flags"] != 5
                or section["virtual_address"] != "0x00000000"
                or section["file_offset"] < loaded_end or any(content)):
            raise ValueError(f"VU overlay is not a zero-filled non-load section: {section['name']}")
    table_bytes = image[table["file_offset"]:table["file_offset"] + table["size"]]
    strings_bytes = image[names["file_offset"]:names["file_offset"] + names["size"]]
    rows = parse_records(table_bytes, strings_bytes,
                         {str(section["name"]): int(section["size"]) for section in overlays})
    counts = Counter(row.name for row in rows)
    if (len(rows) != 30 or len(counts) != len(overlays)
            or sorted(counts.values()) != [1] * 28 + [2]
            or rebuild_strings(rows, len(strings_bytes)) != strings_bytes):
        raise ValueError("VU overlay records/string table differ from measured target")
    data = next(section for section in sections if section["name"] == ".data")
    data_start = int(data["virtual_address"], 16)
    if any(not (data_start <= row.lma and row.lma + row.size <= data_start + data["size"])
           for row in rows):
        raise ValueError("VU overlay LMA range lies outside loaded .data")
    if {row.vma for row in rows} != {0, 0x800}:
        raise ValueError("VU overlay VMA values differ from measured target")
    return rows, overlays, len(strings_bytes)


def render_catalogue(rows: list[Overlay]) -> str:
    lines = ["# Generated by tools/boot_dvp.py from the pinned USA boot; metadata only, no VU code bytes.",
             "index\tname_offset\tname\tlma\tvma\tsize"]
    for index, row in enumerate(rows):
        lines.append(f"{index}\t{row.name_offset}\t{row.name}\t0x{row.lma:08X}"
                     f"\t0x{row.vma:08X}\t{row.size}")
    return "\n".join(lines) + "\n"


def render_rebuild(rows: list[Overlay], overlays: list[dict[str, object]],
                   strings_size: int) -> tuple[str, str]:
    strings = rebuild_strings(rows, strings_size)
    source = [".set noreorder", ".section .text", ".globl test_entry",
              "test_entry:", "    jr $ra", "    nop"]
    for section in overlays:
        source.extend((f'.section {section["name"]},"wx"',
                       f'.space {section["size"]}'))
    source.append('.section .DVP.ovlytab,"w"')
    for row in rows:
        source.append(f".word 0x{row.name_offset:X}, 0x{row.lma:X}, 0x{row.vma:X}")
    source.append('.section .DVP.ovlystrtab,""')
    for offset in range(0, len(strings), 16):
        source.append(".byte " + ", ".join(str(value)
                      for value in strings[offset:offset + 16]))
    script = ["SECTIONS {", "  . = 0x00100000; .text : { *(.text) }", "  . = 0;"]
    for section in overlays:
        name = section["name"]
        script.append(f"  {name} : {{ *({name}) }}")
    script.extend(("  .DVP.ovlytab : { *(.DVP.ovlytab) }",
                   "  .DVP.ovlystrtab : { *(.DVP.ovlystrtab) }", "}"))
    return "\n".join(source) + "\n", "\n".join(script) + "\n"


def rebuild(boot: Path, rows: list[Overlay], overlays: list[dict[str, object]],
            strings_size: int, assembler: Path, linker: Path) -> None:
    check_tool(assembler, ASSEMBLER_SHA256)
    check_tool(linker, LINKER_SHA256)
    source, script = render_rebuild(rows, overlays, strings_size)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / "dvp.s").write_text(source, encoding="ascii", newline="\n")
    (OUTPUT / "dvp.ld").write_text(script, encoding="ascii", newline="\n")
    run_tool([str(assembler.resolve()), "-o", "dvp.o", "dvp.s"],
             OUTPUT, OUTPUT / "assembler.log")
    run_tool([str(linker.resolve()), "-N", "-s", "-e", "test_entry", "-T",
              "dvp.ld", "-o", "dvp.elf", "dvp.o"], OUTPUT, OUTPUT / "linker.log")
    reference_bytes = boot.read_bytes()
    candidate = OUTPUT / "dvp.elf"
    candidate_bytes = candidate.read_bytes()
    reference_sections = {s["name"]: s for s in inspect_elf(boot)["sections"]}
    candidate_sections = {s["name"]: s for s in inspect_elf(candidate)["sections"]}
    if len([name for name in candidate_sections if name.startswith(".DVP")]) != 31:
        raise ValueError("linked VU overlay section count differs")
    for name, original in reference_sections.items():
        if not name.startswith(".DVP"):
            continue
        linked = candidate_sections.get(name)
        if linked is None:
            raise ValueError(f"linked VU overlay section missing: {name}")
        old_data = reference_bytes[original["file_offset"]:original["file_offset"] + original["size"]]
        new_data = candidate_bytes[linked["file_offset"]:linked["file_offset"] + linked["size"]]
        if (old_data != new_data or
                any(original[key] != linked[key] for key in
                    ("type", "flags", "virtual_address", "size", "align"))):
            raise ValueError(f"linked VU overlay section differs: {name}")
    print("31/31 VU overlay sections rebuilt with exact bytes, type, flags, VMA and alignment")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("inspect", "catalogue", "rebuild"))
    parser.add_argument("boot", type=Path)
    parser.add_argument("--write", action="store_true", help="write metadata catalogue")
    parser.add_argument("--assembler", type=Path)
    parser.add_argument("--linker", type=Path)
    args = parser.parse_args()
    rows, overlays, strings_size = audit(args.boot)
    if args.command == "catalogue":
        content = render_catalogue(rows)
        if args.write:
            CATALOGUE.write_text(content, encoding="utf-8", newline="\n")
        elif not CATALOGUE.is_file() or CATALOGUE.read_text(encoding="utf-8") != content:
            raise ValueError("VU overlay catalogue missing or stale; rerun catalogue --write")
    elif args.command == "rebuild":
        if args.assembler is None or args.linker is None:
            parser.error("rebuild requires --assembler and --linker")
        rebuild(args.boot, rows, overlays, strings_size, args.assembler, args.linker)
    print(f"boot VU overlays: {len(overlays)} zero-filled sections, "
          f"{len(rows)} table records, {strings_size} string-table bytes; "
          "all LMAs lie in loaded .data")


if __name__ == "__main__":
    main()
