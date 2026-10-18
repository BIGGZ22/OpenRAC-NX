#!/usr/bin/env python3
"""Rebuild the pinned boot ELF container from a linked load image and metadata.

The JSON catalogue contains numeric ELF layout facts, not retail code or
section payloads. The linked boot supplies PT_LOAD; DVP metadata is regenerated
from the public catalogue. All generated ELF bytes stay in ignored build/.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
from pathlib import Path

from boot_dvp import Overlay, audit, rebuild_strings, render_catalogue
from elf import ELF_HEADER, PROGRAM_HEADER, SECTION_HEADER, inspect_elf

ROOT = Path(__file__).resolve().parent.parent
CATALOGUE = ROOT / "config/boot/elf_layout.json"
DVP_CATALOGUE = ROOT / "config/boot/dvp_overlays.tsv"
OUTPUT = ROOT / "build/boot-elf-pack/boot_exact.elf"
REGINFO = struct.Struct("<6I")
RECORD = struct.Struct("<III")


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def dvp_rows() -> list[Overlay]:
    with DVP_CATALOGUE.open(encoding="utf-8", newline="") as file:
        records = csv.DictReader((line for line in file if not line.startswith("#")),
                                 delimiter="\t")
        rows = [Overlay(int(row["name_offset"]), row["name"],
                        int(row["lma"], 16), int(row["vma"], 16),
                        int(row["size"])) for row in records]
    if len(rows) != 30 or len({row.name for row in rows}) != 29:
        raise ValueError("DVP catalogue no longer has the measured layout")
    return rows


def expected_names(rows: list[Overlay]) -> list[str]:
    overlays = list(dict.fromkeys(row.name for row in rows))
    return (["", ".text", ".reginfo", ".data", ".sndata", *overlays,
             ".rodata", ".gcc_except_table", ".sbss", ".DVP.ovlytab",
             ".DVP.ovlystrtab", ".bss", ".shstrtab"])


def string_table(tokens: list[str]) -> bytes:
    if not tokens or tokens[0] != "" or any(not token or "\0" in token
                                                for token in tokens[1:]):
        raise ValueError("invalid ELF section-name tokens")
    return b"\0".join(token.encode("ascii", "strict") for token in tokens) + b"\0"


def layout_from_boot(boot: Path) -> dict[str, object]:
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    image = boot.read_bytes()
    if digest(image) != target["boot_sha256"]:
        raise ValueError("boot differs from pinned USA target")
    measured_rows, _overlays, strings_size = audit(boot)
    rows = dvp_rows()
    if (render_catalogue(measured_rows) != DVP_CATALOGUE.read_text(encoding="utf-8")
            or rows != measured_rows):
        raise ValueError("public DVP catalogue differs from pinned boot")
    header = ELF_HEADER.unpack_from(image, 16)
    shoff, shentsize, shnum, shstrndx = header[5], header[10], header[11], header[12]
    sections = inspect_elf(boot)["sections"]
    raw_sections = [list(SECTION_HEADER.unpack_from(image, shoff + i * shentsize))
                    for i in range(shnum)]
    if ([section["name"] for section in sections] != expected_names(rows)
            or shstrndx != shnum - 1 or shnum != 41):
        raise ValueError("retail section order differs from measured boot")
    reginfo = next(section for section in sections if section["name"] == ".reginfo")
    if reginfo["size"] != REGINFO.size:
        raise ValueError("retail MIPS reginfo size differs")
    names = sections[shstrndx]
    name_bytes = image[names["file_offset"]:names["file_offset"] + names["size"]]
    tokens = [item.decode("ascii", "strict") for item in name_bytes.split(b"\0")[:-1]]
    if (tokens[:4] != ["", ".symtab", ".strtab", ".shstrtab"]
            or tokens[4:] != expected_names(rows)[1:-1]
            or string_table(tokens) != name_bytes):
        raise ValueError("retail section-name strings differ from measured order")
    programs = [list(PROGRAM_HEADER.unpack_from(image, header[4] + i * header[8]))
                for i in range(header[9])]
    if len(programs) != 1 or programs[0][0] != 1:
        raise ValueError("retail boot must have one PT_LOAD")
    layout = {
        "target_sha256": target["boot_sha256"],
        "file_size": len(image),
        "elf_ident_hex": image[:16].hex(),
        "elf_header": list(header),
        "program_headers": programs,
        "section_headers": [{"name": section["name"], "words": raw}
                            for section, raw in zip(sections, raw_sections)],
        "section_name_tokens": tokens,
        "reginfo_words": list(REGINFO.unpack_from(image, reginfo["file_offset"])),
        "dvp_string_size": strings_size,
    }
    load = programs[0]
    reproduced = pack_file(layout, rows, image[load[1]:load[1] + load[4]])
    if reproduced != image:
        raise ValueError("numeric metadata and generated DVP do not reproduce pinned ELF")
    return layout


def pack_file(layout: dict[str, object], rows: list[Overlay], load: bytes) -> bytes:
    header = layout["elf_header"]
    programs = layout["program_headers"]
    sections = layout["section_headers"]
    if (len(header) != 13 or len(programs) != 1 or len(sections) != 41
            or [section["name"] for section in sections] != expected_names(rows)
            or header[9] != 1 or header[11] != len(sections)
            or header[8] != PROGRAM_HEADER.size or header[10] != SECTION_HEADER.size
            or header[12] != len(sections) - 1):
        raise ValueError("ELF layout shape differs from pinned boot")
    ident = bytes.fromhex(layout["elf_ident_hex"])
    if ident != b"\x7fELF\x01\x01\x01" + b"\0" * 9:
        raise ValueError("unsupported ELF identifier")
    program = programs[0]
    if program[0] != 1 or len(load) != program[4]:
        raise ValueError("linked load size differs from retail PT_LOAD")
    tokens = layout["section_name_tokens"]
    names = string_table(tokens)
    raw_sections = [section["words"] for section in sections]
    shstr = raw_sections[header[12]]
    if (shstr[5] != len(names) or layout["file_size"] != header[5] +
            len(sections) * SECTION_HEADER.size):
        raise ValueError("section-name or file extent differs from measured layout")
    name_positions = {token: names.find(token.encode("ascii") + b"\0")
                      for token in tokens if token}
    for section, raw in zip(sections, raw_sections):
        if (len(raw) != 10 or raw[0] != name_positions.get(section["name"], 0)
                or raw[4] > layout["file_size"]
                or raw[1] != 8 and raw[4] + raw[5] > layout["file_size"]):
            raise ValueError(f"invalid measured section header: {section['name']}")
    result = bytearray(layout["file_size"])

    def write(offset: int, data: bytes, label: str) -> None:
        if offset < 0 or offset + len(data) > len(result):
            raise ValueError(f"{label} lies outside ELF")
        result[offset:offset + len(data)] = data

    write(0, ident, "ELF identifier")
    write(16, ELF_HEADER.pack(*header), "ELF header")
    write(header[4], PROGRAM_HEADER.pack(*program), "program header")
    write(program[1], load, "linked PT_LOAD")
    by_name = {section["name"]: section["words"] for section in sections}
    reginfo = REGINFO.pack(*layout["reginfo_words"])
    if by_name[".reginfo"][5] != len(reginfo):
        raise ValueError("reginfo metadata differs from section size")
    write(by_name[".reginfo"][4], reginfo, "MIPS reginfo")
    table = b"".join(RECORD.pack(row.name_offset, row.lma, row.vma)
                     for row in rows)
    strings = rebuild_strings(rows, layout["dvp_string_size"])
    for name, data in ((".DVP.ovlytab", table), (".DVP.ovlystrtab", strings),
                       (".shstrtab", names)):
        section = by_name[name]
        if section[5] != len(data):
            raise ValueError(f"generated {name} differs from measured size")
        write(section[4], data, name)
    for index, raw in enumerate(raw_sections):
        write(header[5] + index * SECTION_HEADER.size,
              SECTION_HEADER.pack(*raw), f"section header {index}")
    return bytes(result)


def build(boot: Path, linked: Path) -> Path:
    OUTPUT.unlink(missing_ok=True)
    layout = layout_from_boot(boot)
    content = json.dumps(layout, indent=2) + "\n"
    if CATALOGUE.read_text(encoding="utf-8") != content:
        raise ValueError("ELF layout catalogue missing or stale; rerun catalogue --write")
    reference = inspect_elf(boot)
    candidate = inspect_elf(linked)
    if (candidate["entry"] != reference["entry"]
            or candidate["programs"] != reference["programs"]):
        raise ValueError("linked entry or PT_LOAD header differs from retail")
    source = linked.read_bytes()
    program = layout["program_headers"][0]
    output = pack_file(layout, dvp_rows(),
                       source[program[1]:program[1] + program[4]])
    if digest(output) != layout["target_sha256"]:
        raise ValueError("full C-linked ELF differs from pinned retail ELF")
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_bytes(output)
    print(f"complete boot ELF: {len(output)}/{len(output)} exact bytes; "
          f"SHA-256 {digest(output)}")
    return OUTPUT


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("catalogue", "build"))
    parser.add_argument("boot", type=Path)
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--linked", type=Path)
    args = parser.parse_args()
    if args.command == "catalogue":
        layout = layout_from_boot(args.boot)
        content = json.dumps(layout, indent=2) + "\n"
        if args.write:
            CATALOGUE.write_text(content, encoding="utf-8", newline="\n")
        elif not CATALOGUE.is_file() or CATALOGUE.read_text(encoding="utf-8") != content:
            raise ValueError("ELF layout catalogue missing or stale; rerun --write")
        print(f"boot ELF layout: {layout['file_size']} bytes, "
              f"{len(layout['section_headers'])} sections; metadata only")
    elif args.linked is None:
        parser.error("build requires --linked")
    else:
        build(args.boot, args.linked)


if __name__ == "__main__":
    main()
