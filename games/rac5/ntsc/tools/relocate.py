#!/usr/bin/env python3
"""Reconstruct a local SNR2 image at a measured load base.

Output contains game bytes and must stay under ignored build/. This is a
relocation baseline, not a C/C++ match, assembler rebuild or progress claim.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
from pathlib import Path
from typing import Mapping

from boot_symbols import read_boot
from disc import IsoDisc, verify_disc
from rel import (HEADER, RELOCATION, SYMBOL, iter_packed_relocations,
                 parse_module, unpack_relocation)

ROOT = Path(__file__).resolve().parent.parent
POINTER_FIELDS = (4, 12, 20, 24, 28, 32, 52, 56)
WORD = struct.Struct("<I")


def patched_word(word: int, kind: int, target: int) -> int:
    """Apply the boot helper's MIPS 32/26/HI16/LO16 instruction patch."""
    target &= 0xFFFFFFFF
    if kind == 2:
        return target
    if (word & 0xFC00003F) == 0x0D:
        raise ValueError("relocation targets a break instruction")
    if kind == 4:
        return (word & 0xFC000000) | ((target >> 2) & 0x03FFFFFF)
    if kind == 5:
        return (word & 0xFFFF0000) | (((target + 0x8000) >> 16) & 0xFFFF)
    if kind == 6:
        return (word & 0xFFFF0000) | (target & 0xFFFF)
    raise ValueError(f"unsupported MIPS relocation type {kind}")


def relocate_module(path: str, source: bytes, base: int,
                    external_symbols: Mapping[str, int]) -> tuple[bytes, tuple[str, ...]]:
    """Apply the pinned boot's two relocation stages to one SNR2 file.

    Missing external names resolve to zero and are reported to the caller;
    other modules may require a wider symbol set than the boot provides.
    The linked-list field is zero, as it is for the measured standalone loads.
    """
    parse_module(path, source)
    head = HEADER.unpack_from(source)
    alignment, size = head[10], len(source)
    if alignment == 0 or alignment & (alignment - 1) or base % alignment:
        raise ValueError("load base does not satisfy SNR2 alignment")
    if base < 0 or base + size > 0x100000000:
        raise ValueError("load base or image exceeds the 32-bit address space")
    if head[9]:
        raise ValueError("SNR2 export-pointer table is not yet supported")

    image = bytearray(source)
    for offset in POINTER_FIELDS:
        value = WORD.unpack_from(source, offset)[0]
        if value:
            WORD.pack_into(image, offset, base + value)
    WORD.pack_into(image, 0x30, 0)  # next loaded SNR2 module

    symbol_values: list[int] = []
    missing: set[str] = set()
    for index in range(head[4]):
        offset = head[3] + index * SYMBOL.size
        name_at, address, _aux, kind, _flags = SYMBOL.unpack_from(source, offset)
        end = source.index(b"\0", name_at)
        name = source[name_at:end].decode("ascii")
        if kind == 0:
            value = base + address  # includes the zero-address sentinel
        elif kind == 1:
            value = external_symbols.get(name, 0)
            if name and not value:
                missing.add(name)
        elif kind == 2:
            value = base + address
        else:
            raise ValueError(f"unsupported SNR2 symbol kind {kind} ({name})")
        WORD.pack_into(image, offset, base + name_at)
        WORD.pack_into(image, offset + 4, value)
        image[offset + 11] = 1 if kind == 2 else 0
        symbol_values.append(value)

    for fixup in iter_packed_relocations(source, head[13], head[1]):
        site = fixup.site
        word = WORD.unpack_from(image, site)[0]
        if fixup.kind == 0:
            kind, target = 6, base + (word & 0xFFFF)
        elif fixup.kind == 1:
            if fixup.low_site is None:
                raise ValueError("packed HI16 has no paired LO16")
            lower = WORD.unpack_from(image, fixup.low_site)[0] & 0xFFFF
            if lower & 0x8000:
                lower -= 0x10000
            kind, target = 5, base + ((word & 0xFFFF) << 16) + lower
        elif fixup.kind == 2:
            kind, target = 4, base + ((word & 0x03FFFFFF) << 2)
        else:
            kind, target = 2, base + word
        WORD.pack_into(image, site, patched_word(word, kind, target))

    for index in range(head[2]):
        offset = head[1] + index * RELOCATION.size
        site, kind, symbol, addend = unpack_relocation(source, offset)
        WORD.pack_into(image, offset, base + site)
        word = WORD.unpack_from(image, site)[0]
        WORD.pack_into(image, site, patched_word(word, kind, symbol_values[symbol] + addend))
    return bytes(image), tuple(sorted(missing))


def output_path(module: str, base: int, requested: Path | None) -> Path:
    """Keep every reconstructed game byte below the ignored build/ root."""
    allowed = (ROOT / "build").resolve()
    output = (requested or allowed / "relocated" / f"{module}.{base:08X}.bin").resolve()
    if not output.is_relative_to(allowed):
        raise ValueError("relocated game bytes must be written under ignored build/")
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path, help="personally obtained pinned USA ISO")
    parser.add_argument("module", help="filename under LVL/, e.g. FRONTEND.REL")
    parser.add_argument("--base", required=True, type=lambda text: int(text, 0),
                        help="measured aligned module address, e.g. 0xF97B80")
    parser.add_argument("--boot", type=Path, default=ROOT / "baserom/SCUS_976.15")
    parser.add_argument("--out", type=Path, help="output under ignored build/ only")
    args = parser.parse_args()
    if not re.fullmatch(r"(?:FRONTEND|LEVEL_\d{2})\.REL", args.module):
        parser.error("module must be a LVL/ frontend or numbered REL filename")

    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    with IsoDisc(args.image) as disc:
        verify_disc(disc, target)
        source = disc.file_bytes("LVL/" + args.module)
    boot_report, boot_symbols = read_boot(args.boot)
    if (boot_report["sha256"] != target["boot_sha256"]
            or boot_report["file_size"] != target["boot_size"]):
        parser.error("boot does not match the pinned USA executable")
    addresses = {symbol.name: symbol.address for symbol in boot_symbols}
    result, missing = relocate_module(args.module, source, args.base, addresses)

    try:
        output = output_path(args.module, args.base, args.out)
    except ValueError as error:
        parser.error(str(error))
    if output.exists() and output.read_bytes() != result:
        raise FileExistsError(f"existing output differs: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(result)
    print(f"{args.module}: {len(result)} relocated bytes at 0x{args.base:08X}; "
          f"SHA-256 {hashlib.sha256(result).hexdigest()}")
    if missing:
        print("external symbols unresolved to zero: " + ", ".join(missing))
    print(f"private output: {output}")


if __name__ == "__main__":
    main()
