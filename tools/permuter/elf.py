# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The little of a relocatable MIPS ELF32 object the scorer reads: a function's code and which of
its instructions carry a relocation. Standard library only.

    obj = elf.read(Path("out.o"))
    code, relocs = obj.function("func_00123456")   # bytes, {offset in it: (reloc type, symbol)}
"""
from __future__ import annotations

import struct
from dataclasses import dataclass, field
from pathlib import Path

SHT_SYMTAB, SHT_RELA, SHT_REL = 2, 4, 9
R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 4, 5, 6, 7


@dataclass
class Section:
    name: str
    kind: int
    offset: int
    size: int
    link: int
    info: int
    entsize: int


@dataclass
class Object:
    data: bytes
    sections: list[Section]
    symbols: list[tuple[str, int, int, int]] = field(default_factory=list)  # name, value, size, shndx

    def body(self, section: Section) -> bytes:
        return self.data[section.offset:section.offset + section.size]

    def relocations(self, index: int) -> dict[int, tuple[int, str]]:
        """{offset: (relocation type, symbol name)} of every REL or RELA section that applies to
        section INDEX."""
        out = {}
        for s in self.sections:
            if s.kind in (SHT_REL, SHT_RELA) and s.info == index:
                step = 12 if s.kind == SHT_RELA else 8
                body = self.body(s)
                for at in range(0, len(body) - step + 1, step):
                    offset, info = struct.unpack_from("<II", body, at)
                    sym = info >> 8
                    out[offset] = (info & 0xFF, self.symbols[sym][0] if sym < len(self.symbols) else "")
        return out

    def function(self, name: str) -> tuple[bytes, dict[int, tuple[int, str]]]:
        """The code of function NAME and its relocations, keyed by offset from its start. A symbol
        of size 0 runs to the next symbol of its section or the section's end. Raises KeyError."""
        for sym, value, size, shndx in self.symbols:
            if sym == name and 0 < shndx < len(self.sections):
                break
        else:
            raise KeyError(name)
        text = self.body(self.sections[shndx])
        if not size:
            later = [v for _, v, _, i in self.symbols if i == shndx and v > value]
            size = (min(later) if later else len(text)) - value
        relocs = {o - value: t for o, t in self.relocations(shndx).items() if value <= o < value + size}
        return text[value:value + size], relocs


def read(path: Path) -> Object:
    """Parse the object at PATH. Raises ValueError when it is not a little-endian ELF32 file."""
    data = Path(path).read_bytes()
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        raise ValueError(f"{path}: not a little-endian ELF32 file")
    shoff, = struct.unpack_from("<I", data, 0x20)
    entsize, count, names = struct.unpack_from("<3H", data, 0x2E)
    raw = [struct.unpack_from("<10I", data, shoff + i * entsize) for i in range(count)]
    strings = raw[names][4] if count else 0

    def name_at(table: int, at: int) -> str:
        start = table + at
        return data[start:data.index(b"\0", start)].decode(errors="replace")

    sections = [Section(name_at(strings, r[0]), r[1], r[4], r[5], r[6], r[7], r[9]) for r in raw]
    obj = Object(data, sections)
    for s in sections:
        if s.kind == SHT_SYMTAB:
            table = sections[s.link].offset
            body = obj.body(s)
            for at in range(0, len(body) - 15, 16):
                name, value, size, _info, _other, shndx = struct.unpack_from("<IIIBBH", body, at)
                obj.symbols.append((name_at(table, name), value, size, shndx))
    return obj
