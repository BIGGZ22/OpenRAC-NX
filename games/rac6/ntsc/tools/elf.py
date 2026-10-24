#!/usr/bin/env python3
"""Read the PS2 ELF metadata of a target program, and list its functions.

The tool reports structure only: sections, program headers, the entry point,
and the linked function symbols with their address and compiled size.  It
never prints executable bytes, so its output can be published as research
metadata.

Adapted from the ELF reader of ``OpenRAC/rac5-sm-decomp`` (GPL-3.0); see
``THIRD_PARTY_NOTICES.md``.

Usage:
    python3 tools/elf.py <executable> [--json]
    python3 tools/elf.py <executable> --functions [--tsv]
"""

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
SHT_PROGBITS = 1
SHT_SYMTAB = 2
SHT_STRTAB = 3
SHT_NOBITS = 8
PT_LOAD = 1
STT_FUNC = 2
EM_MIPS = 8
ET_EXEC = 2


def _headers(data: bytes) -> tuple[tuple, list[tuple], list[tuple]]:
    if len(data) < 52 or data[:6] != b"\x7fELF\x01\x01":
        raise ValueError("not a 32-bit little-endian ELF")
    (kind, machine, version, entry, phoff, shoff, flags, ehsize, phentsize,
     phnum, shentsize, shnum, shstrndx) = ELF_HEADER.unpack_from(data, 16)
    if machine != EM_MIPS or kind != ET_EXEC or version != 1 or ehsize != 52:
        raise ValueError("not a supported PS2 MIPS executable")
    if phnum and (phentsize != PROGRAM_HEADER.size or phoff + phnum * phentsize > len(data)):
        raise ValueError("invalid ELF program-header table")
    if shnum and (shentsize != SECTION_HEADER.size or shoff + shnum * shentsize > len(data)):
        raise ValueError("invalid ELF section-header table")
    if shnum == 0 or shstrndx >= shnum:
        raise ValueError("missing ELF section-name table")
    programs = [PROGRAM_HEADER.unpack_from(data, phoff + index * phentsize)
                for index in range(phnum)]
    sections = [SECTION_HEADER.unpack_from(data, shoff + index * shentsize)
                for index in range(shnum)]
    return (kind, machine, version, entry, phoff, shoff, flags, ehsize, phentsize,
            phnum, shentsize, shnum, shstrndx), programs, sections


def inspect_elf(path: Path) -> dict[str, object]:
    """Return the structure of an ELF: programs, sections, entry, hashes."""
    data = Path(path).read_bytes()
    (_kind, _machine, _version, entry, _phoff, _shoff, flags, _ehsize, _phentsize,
     _phnum, _shentsize, shnum, shstrndx), _programs, sections = _headers(data)

    programs: list[dict[str, object]] = []
    for index, (typ, offset, vaddr, paddr, file_size, mem_size, pflags, align) in enumerate(
            [PROGRAM_HEADER.unpack_from(data, off) for off in ()] or []):
        pass
    raw_programs = []
    for index in range(len(_programs)):
        typ, offset, vaddr, paddr, file_size, mem_size, pflags, align = _programs[index]
        if offset + file_size > len(data) or file_size > mem_size:
            raise ValueError(f"invalid ELF program header {index}")
        raw_programs.append({
            "type": typ, "file_offset": offset, "virtual_address": f"0x{vaddr:08X}",
            "physical_address": f"0x{paddr:08X}", "file_size": file_size,
            "memory_size": mem_size, "flags": pflags, "align": align,
        })
    programs = raw_programs

    names_offset, names_size = sections[shstrndx][4], sections[shstrndx][5]
    if names_offset + names_size > len(data):
        raise ValueError("invalid ELF section-name data")
    names = data[names_offset:names_offset + names_size]
    reported: list[dict[str, object]] = []
    for index, (name_at, typ, sflags, addr, offset, size, link, info, align, entsize) in enumerate(sections):
        if typ != SHT_NOBITS and offset + size > len(data):
            raise ValueError(f"invalid ELF section {index}")
        if name_at >= len(names):
            raise ValueError(f"invalid ELF section name {index}")
        end = names.find(b"\0", name_at)
        if end < 0:
            raise ValueError(f"unterminated ELF section name {index}")
        reported.append({
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
        "sections": reported,
    }


def read_symbols(path: Path) -> list[dict[str, object]]:
    """Return the linked symbols of an ELF, with their section and size."""
    data = Path(path).read_bytes()
    (_kind, _machine, _version, _entry, _phoff, _shoff, _flags, _ehsize, _phentsize,
     _phnum, _shentsize, shnum, _shstrndx), _programs, sections = _headers(data)

    section_names: list[str] = []
    names_offset, names_size = sections[_shstrndx][4], sections[_shstrndx][5]
    names = data[names_offset:names_offset + names_size]
    for name_at, *_rest in sections:
        end = names.find(b"\0", name_at)
        section_names.append(names[name_at:end].decode("ascii", "replace"))

    symbols: list[dict[str, object]] = []
    for index, header in enumerate(sections):
        if header[1] != SHT_SYMTAB:
            continue
        _name, _typ, _flags, _addr, offset, size, link, _info, _align, entsize = header
        if entsize != ELF_SYMBOL.size or size % entsize or link >= shnum:
            raise ValueError("invalid ELF symbol table")
        if sections[link][1] != SHT_STRTAB:
            raise ValueError("ELF symbol table has no string table")
        strings = data[sections[link][4]:sections[link][4] + sections[link][5]]
        for at in range(offset, offset + size, entsize):
            name_at, value, body_size, sym_info, _other, section = ELF_SYMBOL.unpack_from(data, at)
            if name_at >= len(strings) or b"\0" not in strings[name_at:]:
                raise ValueError("invalid ELF symbol name")
            end = strings.index(b"\0", name_at)
            name = strings[name_at:end].decode("ascii", "strict")
            symbols.append({
                "name": name,
                "address": value,
                "size": body_size,
                "type": sym_info & 15,
                "bind": sym_info >> 4,
                "section": section_names[section] if section < len(section_names) else "",
            })
    return symbols


def read_functions(path: Path) -> list[tuple[str, int, int]]:
    """Return (name, address, size) for every global function symbol."""
    functions: list[tuple[str, int, int]] = []
    seen: set[str] = set()
    for symbol in read_symbols(path):
        if symbol["bind"] != 1 or symbol["type"] != STT_FUNC:
            continue
        name = str(symbol["name"])
        if name in seen:
            raise ValueError(f"duplicate ELF function symbol: {name}")
        seen.add(name)
        functions.append((name, int(symbol["address"]), int(symbol["size"])))
    functions.sort(key=lambda item: item[1])
    return functions


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--json", action="store_true", help="print the full structure")
    parser.add_argument("--functions", action="store_true", help="list the function symbols")
    parser.add_argument("--tsv", action="store_true", help="print the function list as TSV")
    args = parser.parse_args()

    if args.functions:
        functions = read_functions(args.executable)
        if args.tsv:
            print("name\taddress\tsize")
            for name, address, size in functions:
                print(f"{name}\t0x{address:08X}\t{size}")
        else:
            for name, address, size in functions:
                print(f"0x{address:08X} {size:>8}  {name}")
        print(f"{len(functions)} function symbol(s)")
        return 0

    report = inspect_elf(args.executable)
    if args.json:
        print(json.dumps(report, indent=2))
        return 0
    print(f"MIPS ELF: {report['file_size']} bytes, entry {report['entry']}, "
          f"SHA-256 {report['sha256']}")
    print(f"{len(report['programs'])} program header(s), {len(report['sections'])} section(s)")
    for section in report["sections"]:
        print(f"  {section['name']:<16} address {section['virtual_address']} "
              f"file {section['file_offset']:#x} size 0x{section['size']:X}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
