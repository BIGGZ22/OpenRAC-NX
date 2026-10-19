#!/usr/bin/env python3
"""Report PS2 ELF metadata without publishing executable bytes."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_HEADER = struct.Struct("<HHIIIIIHHHHHH")
PROGRAM_HEADER = struct.Struct("<IIIIIIII")
SECTION_HEADER = struct.Struct("<IIIIIIIIII")
ELF_SYMBOL = struct.Struct("<IIIBBH")
SHT_NOBITS = 8
PT_LOAD = 1
EM_MIPS = 8


def inspect_elf(path: Path) -> dict[str, object]:
    data = path.read_bytes()
    if len(data) < 52 or data[:6] != b"\x7fELF\x01\x01":
        raise ValueError("not a 32-bit little-endian ELF")
    (kind, machine, version, entry, phoff, shoff, flags, ehsize, phentsize,
     phnum, shentsize, shnum, shstrndx) = ELF_HEADER.unpack_from(data, 16)
    if machine != EM_MIPS or kind != 2 or version != 1 or ehsize != 52:
        raise ValueError("not a supported PS2 MIPS executable")
    if phnum and (phentsize != PROGRAM_HEADER.size or phoff + phnum * phentsize > len(data)):
        raise ValueError("invalid ELF program-header table")
    if shnum and (shentsize != SECTION_HEADER.size or shoff + shnum * shentsize > len(data)):
        raise ValueError("invalid ELF section-header table")
    if shnum == 0 or shstrndx >= shnum:
        raise ValueError("missing ELF section-name table")

    programs: list[dict[str, object]] = []
    for index in range(phnum):
        typ, offset, vaddr, paddr, file_size, mem_size, pflags, align = PROGRAM_HEADER.unpack_from(
            data, phoff + index * phentsize)
        if offset + file_size > len(data) or file_size > mem_size:
            raise ValueError(f"invalid ELF program header {index}")
        programs.append({
            "type": typ, "file_offset": offset, "virtual_address": f"0x{vaddr:08X}",
            "physical_address": f"0x{paddr:08X}", "file_size": file_size,
            "memory_size": mem_size, "flags": pflags, "align": align,
        })

    raw_sections = [SECTION_HEADER.unpack_from(data, shoff + index * shentsize)
                    for index in range(shnum)]
    names_offset, names_size = raw_sections[shstrndx][4], raw_sections[shstrndx][5]
    if names_offset + names_size > len(data):
        raise ValueError("invalid ELF section-name data")
    names = data[names_offset:names_offset + names_size]
    sections: list[dict[str, object]] = []
    for index, (name_at, typ, sflags, addr, offset, size, link, info, align, entsize) in enumerate(raw_sections):
        if typ != SHT_NOBITS and offset + size > len(data):
            raise ValueError(f"invalid ELF section {index}")
        if name_at >= len(names):
            raise ValueError(f"invalid ELF section name {index}")
        end = names.find(b"\0", name_at)
        if end < 0:
            raise ValueError(f"unterminated ELF section name {index}")
        sections.append({
            "name": names[name_at:end].decode("ascii", "replace"),
            "type": typ, "flags": sflags, "virtual_address": f"0x{addr:08X}",
            "file_offset": offset, "size": size, "align": align,
        })

    return {
        "sha256": hashlib.sha256(data).hexdigest(),
        "file_size": len(data),
        "entry": f"0x{entry:08X}",
        "machine": "MIPS",
        "elf_flags": f"0x{flags:08X}",
        "programs": programs,
        "sections": sections,
    }


def read_global_functions(path: Path) -> dict[str, tuple[int, int]]:
    """Read linked ELF32 global function symbols as address/compiled-body size."""
    inspect_elf(path)
    data = path.read_bytes()
    (_kind, _machine, _version, _entry, _phoff, shoff, _flags, _ehsize,
     _phentsize, _phnum, shentsize, shnum, _shstrndx) = ELF_HEADER.unpack_from(data, 16)
    headers = [SECTION_HEADER.unpack_from(data, shoff + index * shentsize)
               for index in range(shnum)]
    functions: dict[str, tuple[int, int]] = {}
    for header in headers:
        if header[1] != 2:
            continue
        _name, _typ, _flags, _addr, offset, size, link, _info, _align, entsize = header
        if (entsize != ELF_SYMBOL.size or size % entsize
                or link >= shnum or headers[link][1] != 3):
            raise ValueError("invalid ELF function symbol table")
        strings = data[headers[link][4]:headers[link][4] + headers[link][5]]
        for at in range(offset, offset + size, entsize):
            name_at, value, body_size, sym_info, _other, section = ELF_SYMBOL.unpack_from(data, at)
            if sym_info >> 4 != 1 or sym_info & 15 != 2 or section == 0:
                continue
            if name_at >= len(strings) or b"\0" not in strings[name_at:]:
                raise ValueError("invalid ELF function symbol name")
            end = strings.index(b"\0", name_at)
            name = strings[name_at:end].decode("ascii", "strict")
            if name in functions:
                raise ValueError(f"duplicate ELF function symbol: {name}")
            functions[name] = (value, body_size)
    return functions


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--json", action="store_true", help="include every section and program header")
    args = parser.parse_args()
    report = inspect_elf(args.executable)
    if args.json:
        print(json.dumps(report, indent=2))
        return
    print(f"MIPS ELF: {report['file_size']} bytes, entry {report['entry']}, SHA-256 {report['sha256']}")
    print(f"{len(report['programs'])} program header(s), {len(report['sections'])} section(s)")
    for section in report["sections"]:
        if section["name"] in (".text", ".data", ".sndata", ".rodata", ".bss"):
            print(f"  {section['name']}: address {section['virtual_address']}, size 0x{section['size']:X}")


if __name__ == "__main__":
    main()
