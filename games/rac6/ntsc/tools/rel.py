#!/usr/bin/env python3
"""Read the SNR2 modules of the disc and generate metadata-only catalogues.

The disc holds 33 SNR2 modules under ``BIN/``: one frontend (``FE.REL``) and 32
level modules (``LEVEL<n>.REL``).  An SNR2 module is an SN ProDG relocatable
module: a payload, a table of named-symbol relocations, a symbol table and a
packed stream of local fixups.  This tool reads that structure and counts it;
it does not split functions, does not apply relocations and does not claim any
byte match.

The field names follow the public BSD-3-Clause SNR2 loader in
``emoose/re4-research``; the parser was written for this project and checked
against every module of the pinned disc.

Adapted from ``OpenRAC/rac5-sm-decomp`` (GPL-3.0); see
``THIRD_PARTY_NOTICES.md``.

Usage:
    python3 tools/rel.py inspect <image> [--boot <elf>]
    python3 tools/rel.py catalogue <image> [--write]
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator

from disc import IsoDisc

ROOT = Path(__file__).resolve().parent.parent
HEADER = struct.Struct("<15I")
# The boot reads a 32-bit r_info word: low 8 bits type, high 24 the symbol.
# The signed addend is the third word (offset 8).
RELOCATION = struct.Struct("<IIi")
SYMBOL = struct.Struct("<IIHBB")
MAGIC = 0x32524E53  # ASCII SNR2, little endian
MODULE_DIR = "BIN"


def unpack_relocation(data: bytes, offset: int) -> tuple[int, int, int, int]:
    """Return (site, type, symbol index, signed addend) of one relocation."""
    site, info, addend = RELOCATION.unpack_from(data, offset)
    return site, info & 0xFF, info >> 8, addend


@dataclass(frozen=True)
class Export:
    name: str
    address: int  # offset in the SNR2 module, not a linked RAM address


@dataclass(frozen=True)
class Module:
    path: str
    size: int
    sha256: str
    relocation_count: int
    symbol_count: int
    imports: tuple[str, ...]
    import_references: tuple[tuple[str, int], ...]
    exports: tuple[Export, ...]
    relocation_types: tuple[tuple[int, int], ...]
    packed_relocation_types: tuple[tuple[int, int], ...]


@dataclass(frozen=True)
class PackedRelocation:
    kind: int
    site: int
    low_site: int | None = None


def iter_packed_relocations(data: bytes, start: int, payload_end: int) -> Iterator[PackedRelocation]:
    """Decode the local fixup sites of the packed stream, without applying them.

    Each descriptor byte carries two bits of operation and six bits of a
    four-byte-aligned site delta.  A zero delta is followed by a 32-bit
    absolute file offset, and an offset of zero ends the stream.  A HI16 site
    looks ahead for its LO16 partner but does not consume it.
    """
    if start == 0:
        return
    if not 0 < start < len(data):
        raise ValueError("invalid packed relocation stream start")

    def next_site(cursor: int, site: int) -> tuple[int, int, int, bool]:
        if cursor >= len(data):
            raise ValueError("unterminated packed relocation stream")
        descriptor = data[cursor]
        kind, delta = descriptor & 3, descriptor & 0xFC
        cursor += 1
        if delta == 0:
            if cursor + 4 > len(data):
                raise ValueError("short packed relocation offset")
            site = struct.unpack_from("<I", data, cursor)[0]
            cursor += 4
            if site == 0:
                return kind, site, cursor, True
        else:
            site += delta
        if site < HEADER.size or site + 4 > payload_end or site % 4:
            raise ValueError(f"invalid packed relocation site 0x{site:X}")
        return kind, site, cursor, False

    cursor, site = start, 0
    while True:
        kind, site, cursor, done = next_site(cursor, site)
        if done:
            if any(data[cursor:]):
                raise ValueError("nonzero data after packed relocation terminator")
            return
        low_site = None
        if kind == 1:
            look_cursor, look_site = cursor, site
            while True:
                look_kind, look_site, look_cursor, look_done = next_site(look_cursor, look_site)
                if look_done:
                    raise ValueError(f"packed HI16 at 0x{site:X} has no following LO16")
                if look_kind == 0:
                    low_site = look_site
                    break
        yield PackedRelocation(kind, site, low_site)


def packed_relocations_by_site(data: bytes, start: int,
                               payload_end: int) -> list[PackedRelocation]:
    """Index the fixups by address, preserving the stream HI16/LO16 links."""
    indexed = sorted(iter_packed_relocations(data, start, payload_end),
                     key=lambda item: item.site)
    if any(left.site == right.site for left, right in zip(indexed, indexed[1:])):
        raise ValueError("duplicate packed relocation site")
    return indexed


def packed_relocation_types(data: bytes, start: int, payload_end: int) -> tuple[tuple[int, int], ...]:
    """Count the packed local fixup operations, without publishing program bytes."""
    counts = Counter(item.kind for item in iter_packed_relocations(data, start, payload_end))
    return tuple(sorted(counts.items()))


def parse_module(path: str, data: bytes) -> Module:
    """Read one SNR2 module: its tables, its imports and its exports."""
    if len(data) < HEADER.size:
        raise ValueError(f"{path}: short SNR2 header")
    (magic, reloc_at, reloc_count, symbol_at, symbol_count, _image_name_at,
     _ctors_at, _dtors_at, _exports_at, _exports_count, _unknown,
     declared_size, _unknown2, packed_at, _unknown_at38) = HEADER.unpack_from(data)
    if magic != MAGIC:
        raise ValueError(f"{path}: missing SNR2 magic")
    if declared_size != len(data):
        raise ValueError(f"{path}: SNR2 file size does not match the header")
    if reloc_at < HEADER.size or reloc_at + reloc_count * RELOCATION.size != symbol_at:
        raise ValueError(f"{path}: invalid relocation table extent")
    if symbol_count == 0 or symbol_at + symbol_count * SYMBOL.size > len(data):
        raise ValueError(f"{path}: invalid symbol table extent")
    if packed_at and packed_at < symbol_at + symbol_count * SYMBOL.size:
        raise ValueError(f"{path}: packed relocations overlap the symbol table")
    try:
        packed_types = packed_relocation_types(data, packed_at, reloc_at)
    except ValueError as error:
        raise ValueError(f"{path}: {error}") from error

    types: Counter[int] = Counter()
    referenced: Counter[int] = Counter()
    for index in range(reloc_count):
        offset, kind, symbol, _addend = unpack_relocation(
            data, reloc_at + index * RELOCATION.size)
        if offset >= reloc_at or symbol >= symbol_count:
            raise ValueError(f"{path}: invalid relocation {index}")
        types[kind] += 1
        referenced[symbol] += 1

    imports: list[str] = []
    import_references: Counter[str] = Counter()
    exports: list[Export] = []
    for index in range(symbol_count):
        name_at, address, _aux, _kind, _flags = SYMBOL.unpack_from(
            data, symbol_at + index * SYMBOL.size)
        if name_at < HEADER.size or name_at >= reloc_at or address >= reloc_at:
            raise ValueError(f"{path}: invalid symbol {index}")
        end = data.find(b"\0", name_at, min(reloc_at, name_at + 256))
        if end < 0:
            raise ValueError(f"{path}: unterminated symbol name {index}")
        name = data[name_at:end].decode("ascii", "strict")
        if address:
            if not name:
                raise ValueError(f"{path}: unnamed export {index}")
            exports.append(Export(name, address))
        elif name:
            imports.append(name)
            if referenced[index]:
                import_references[name] += referenced[index]
    return Module(
        path, len(data), hashlib.sha256(data).hexdigest(), reloc_count,
        symbol_count, tuple(imports), tuple(sorted(import_references.items())),
        tuple(exports), tuple(sorted(types.items())), packed_types,
    )


def module_paths(disc: IsoDisc) -> list[str]:
    """Return the SNR2 module paths of the disc, in a stable order."""
    names = sorted(name for name in disc.entries(disc.find(MODULE_DIR))
                   if name.endswith(".REL"))
    if not names:
        raise ValueError(f"no {MODULE_DIR}/*.REL module found on the disc")
    return [f"{MODULE_DIR}/{name}" for name in names]


def modules_from_disc(image: Path) -> list[Module]:
    modules: list[Module] = []
    with IsoDisc(image) as disc:
        for path in module_paths(disc):
            modules.append(parse_module(path, disc.file_bytes(path)))
    return modules


def render_modules(modules: list[Module]) -> str:
    lines = ["# Generated by tools/rel.py; hashes and counts, no game bytes.",
             "path\tsize\tsha256\tnamed_relocations\tsymbols\timports\texports\t"
             "packed_hi16\tpacked_lo16\tpacked_26\tpacked_32"]
    for module in modules:
        packed = dict(module.packed_relocation_types)
        lines.append(f"{module.path}\t{module.size}\t{module.sha256}\t"
                     f"{module.relocation_count}\t{module.symbol_count}\t"
                     f"{len(module.imports)}\t{len(module.exports)}\t"
                     f"{packed.get(1, 0)}\t{packed.get(0, 0)}\t"
                     f"{packed.get(2, 0)}\t{packed.get(3, 0)}")
    return "\n".join(lines) + "\n"


def render_exports(modules: list[Module]) -> str:
    lines = ["# Generated by tools/rel.py; addresses are SNR2 offsets, not linked RAM addresses.",
             "module\tsymbol\toffset"]
    for module in modules:
        for symbol in module.exports:
            lines.append(f"{module.path}\t{symbol.name}\t0x{symbol.address:08X}")
    return "\n".join(lines) + "\n"


def catalogue(modules: list[Module], *, write: bool) -> None:
    """Write or verify the module catalogue.

    ``config/levels/modules.tsv`` is tracked: it is the identity of every
    module of the disc.  ``config/levels/exports.tsv`` lists every named
    symbol of every module -- more than 350 000 rows for this disc -- so it
    is generated on demand and stays out of the repository.
    """
    outputs = {
        ROOT / "config/levels/modules.tsv": render_modules(modules),
        ROOT / "config/levels/exports.tsv": render_exports(modules),
    }
    for path, content in outputs.items():
        if write:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8", newline="\n")
        elif path.name == "modules.tsv" and (
                not path.is_file() or path.read_text(encoding="utf-8") != content):
            raise ValueError(f"{path.relative_to(ROOT)} is missing or stale; "
                             "run catalogue --write")


def verify_target(disc: IsoDisc) -> dict[str, object]:
    """Refuse an image that is not the pinned target."""
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    info = disc.inspect()
    if info["boot_file"] != target["boot_file"]:
        raise ValueError("disc boot file differs from the pinned target")
    if info["actual_size"] != target["iso_size"]:
        raise ValueError("disc size differs from the pinned target")
    boot = disc.file_bytes(str(target["boot_file"]))
    if len(boot) != target["boot_size"]:
        raise ValueError("boot size differs from the pinned target")
    if hashlib.sha256(boot).hexdigest() != target["boot_sha256"]:
        raise ValueError("boot hash differs from the pinned target")
    return info


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=("inspect", "catalogue"))
    parser.add_argument("image", type=Path)
    parser.add_argument("--write", action="store_true", help="regenerate the tracked tables")
    args = parser.parse_args()

    with IsoDisc(args.image) as disc:
        verify_target(disc)
        modules = [parse_module(path, disc.file_bytes(path))
                   for path in module_paths(disc)]

    if args.command == "catalogue":
        catalogue(modules, write=args.write)
        print(f"SNR2 catalogues: {len(modules)} modules, "
              f"{sum(len(m.exports) for m in modules)} exports")
        return 0

    counts: Counter[int] = Counter()
    packed_counts: Counter[int] = Counter()
    for module in modules:
        counts.update(dict(module.relocation_types))
        packed_counts.update(dict(module.packed_relocation_types))
    print(f"SNR2: {len(modules)} modules, "
          f"{sum(m.relocation_count for m in modules)} named-symbol relocations, "
          f"{sum(len(m.exports) for m in modules)} exported symbols")
    print("packed local fixups: "
          + ", ".join(f"{kind}={count}" for kind, count in sorted(packed_counts.items())))
    print("named-symbol relocation types: "
          + ", ".join(f"{kind}={count}" for kind, count in sorted(counts.items())))
    for module in modules:
        print(f"  {module.path:16s} {module.size:9d} bytes, "
              f"{module.relocation_count:6d} named relocations, "
              f"{sum(count for _kind, count in module.packed_relocation_types):7d} packed fixups, "
              f"{module.symbol_count:6d} symbols, {len(module.exports):4d} exports")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
