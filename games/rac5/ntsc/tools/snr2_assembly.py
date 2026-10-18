#!/usr/bin/env python3
"""Rebuild pinned SNR2 candidate code intervals as private R5900 assembly.

Requires optional rabbitizer 1.16.2 and privately supplied measured SN tools.
Game-derived output stays under ignored build/snr2-assembly/.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
from pathlib import Path

from disc import IsoDisc, verify_disc
from elf import inspect_elf
from r5900_assembly import (ASSEMBLER_SHA256, LINKER_SHA256, check_tool,
                            render_assembly, run_tool)
from rel import HEADER, MAGIC, RELOCATION, iter_packed_relocations, unpack_relocation

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "build/snr2-assembly"


CODE_START = 0x80
CATALOGUE = ROOT / "config/levels/code_intervals.tsv"
# Stronger boundary observations already reviewed for these two placements.
BOUNDARY_ANCHORS = {
    "FRONTEND.REL": (0x27D00, 0x48, 0x27D00),
    "LEVEL_01.REL": (0x1EF180, 0x78, 0x1EF1A8),
}


def pinned_modules() -> dict[str, str]:
    rows = (ROOT / "config/levels/modules.tsv").read_text(encoding="utf-8").splitlines()[1:]
    return {Path(row["path"]).name: row["sha256"]
            for row in csv.DictReader(rows, delimiter="\t")}


def code_end_from_relocations(data: bytes) -> int:
    header = HEADER.unpack_from(data)
    sites: set[int] = set()
    for index in range(header[2]):
        site, kind, _symbol, _addend = unpack_relocation(
            data, header[1] + index * RELOCATION.size)
        if kind == 4:
            sites.add(site)
    sites.update(item.site for item in iter_packed_relocations(data, header[13], header[1])
                 if item.kind == 2)
    if not sites:
        raise ValueError("SNR2 module has no measured jump relocation sites")
    return (max(sites) + 8 + 0x7F) & ~0x7F


def code_words(data: bytes, module: str, expected_sha256: str, end: int) -> tuple[int, ...]:
    if hashlib.sha256(data).hexdigest() != expected_sha256:
        raise ValueError(f"{module} differs from the pinned USA module")
    header = HEADER.unpack_from(data)
    if (header[0] != MAGIC or header[11] != len(data)
            or header[10] != CODE_START or end >= header[1]):
        raise ValueError(f"{module} SNR2 header differs from measured target")
    size = end - CODE_START
    if size <= 0 or size % 4:
        raise ValueError("candidate code extent is not a nonempty aligned interval")
    return struct.unpack_from("<" + "I" * (size // 4), data, CODE_START)


def check_jump_relocations(data: bytes, words: tuple[int, ...]) -> tuple[int, int]:
    header = HEADER.unpack_from(data)
    named: set[int] = set()
    for index in range(header[2]):
        site, kind, _symbol, _addend = unpack_relocation(
            data, header[1] + index * RELOCATION.size)
        if kind == 4:
            named.add(site)
    local = {item.site for item in iter_packed_relocations(data, header[13], header[1])
             if item.kind == 2}
    zero_jumps: set[int] = set()
    local_jumps: set[int] = set()
    for index, word in enumerate(words):
        if word >> 26 not in (2, 3):
            continue
        site = CODE_START + index * 4
        if (word & 0x03FFFFFF) == 0:
            zero_jumps.add(site)
        else:
            local_jumps.add(site)
    if zero_jumps != named or local_jumps != local:
        raise ValueError("SNR2 jump sites disagree with relocation tables")
    return len(zero_jumps), len(local_jumps)


def trailing_zero_words(data: bytes, end: int) -> int:
    count = 0
    for offset in range(end - 4, CODE_START - 1, -4):
        if struct.unpack_from("<I", data, offset)[0] != 0:
            break
        count += 1
    return count


def verify_module(data: bytes, module: str, expected_sha256: str,
                  assembler: Path, linker: Path) -> tuple[str, int, str, int, int, int, int, int]:
    if hashlib.sha256(data).hexdigest() != expected_sha256:
        raise ValueError(f"{module} differs from the pinned USA module")
    end = code_end_from_relocations(data)
    words = code_words(data, module, expected_sha256, end)
    named_count, local_count = check_jump_relocations(data, words)
    slug = module.removesuffix(".REL").lower()
    assembly, stats = render_assembly(words, CODE_START,
                                      symbol=f"{slug}_text_start", allow_zero_jump=True)
    if stats["zero_target_jumps"] != named_count or stats["cop1_branch_words"] != 0:
        raise ValueError(f"{module} instruction counts differ from relocation evidence")
    padding_words = trailing_zero_words(data, end)
    if module in BOUNDARY_ANCHORS:
        anchor_end, anchor_padding, invalid_at = BOUNDARY_ANCHORS[module]
        if end != anchor_end or padding_words * 4 != anchor_padding:
            raise ValueError(f"{module} measured code boundary changed")
        import rabbitizer
        boundary_word = struct.unpack_from("<I", data, invalid_at)[0]
        if rabbitizer.Instruction(boundary_word, invalid_at,
                                  rabbitizer.InstrCategory.R5900).isValid():
            raise ValueError(f"{module} measured data-boundary probe changed")

    output = OUTPUT / slug
    output.mkdir(parents=True, exist_ok=True)
    (output / "module.s").write_text(assembly, encoding="ascii", newline="\n")
    (output / "module.ld").write_text(
        f"T_ZERO = 0;\nSECTIONS {{ . = 0x{CODE_START:08X}; .text : {{ *(.text) }} }}\n",
        encoding="ascii", newline="\n")
    run_tool([str(assembler.resolve()), "-o", "module.o", "module.s"],
             output, output / "assembler.log")
    run_tool([str(linker.resolve()), "-T", "module.ld", "-o",
              "module.elf", "module.o"], output, output / "linker.log")
    candidate = output / "module.elf"
    linked = candidate.read_bytes()
    section = next(item for item in inspect_elf(candidate)["sections"] if item["name"] == ".text")
    size = end - CODE_START
    if int(str(section["virtual_address"]), 16) != CODE_START or section["size"] != size:
        raise ValueError(f"{module} linked code placement differs from derived interval")
    actual = linked[section["file_offset"]:section["file_offset"] + size]
    reference = data[CODE_START:end]
    if actual != reference:
        count = sum(left == right for left, right in zip(actual, reference))
        raise ValueError(f"{module} code differs: {count}/{size} bytes equal")
    digest = hashlib.sha256(actual).hexdigest()
    print(f"{module}: {size}/{size} candidate code bytes exact; "
          f"local jumps={local_count}, named jumps={named_count}, SHA-256 {digest}")
    return (module, end, digest, named_count, local_count,
            stats["branch_targets"], stats["sqrt_s"], padding_words)


def render_catalogue(rows: list[tuple[str, int, str, int, int, int, int, int]]) -> str:
    lines = ["# Generated by tools/snr2_assembly.py after exact private builds; placement bytes, not C progress.",
             "module\tstart\tend_exclusive\tinterval_bytes\tsha256\tnamed_jumps\tpacked_jumps\tbranch_targets\tsqrt_s\ttrailing_zero_words"]
    for module, end, digest, named, local, branches, sqrt, padding in rows:
        lines.append(f"{module}\t0x{CODE_START:08X}\t0x{end:08X}\t{end - CODE_START}\t"
                     f"{digest}\t{named}\t{local}\t{branches}\t{sqrt}\t{padding}")
    return "\n".join(lines) + "\n"


def main(default_module: str | None = None) -> None:
    known = pinned_modules()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("module", nargs="?", choices=known)
    parser.add_argument("--all", action="store_true", help="verify all pinned SNR2 placements")
    parser.add_argument("--write-catalogue", action="store_true",
                        help="write metadata only after all placements pass")
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--linker", type=Path, required=True)
    args = parser.parse_args()
    if args.all and args.module is not None:
        parser.error("choose either one module or --all")
    if args.write_catalogue and not args.all:
        parser.error("--write-catalogue requires --all")
    names = sorted(known) if args.all else [args.module or default_module]
    if names == [None]:
        parser.error("choose one pinned module or --all")
    check_tool(args.assembler, ASSEMBLER_SHA256)
    check_tool(args.linker, LINKER_SHA256)
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    rows = []
    with IsoDisc(args.image) as disc:
        verify_disc(disc, target)
        for module in names:
            data = disc.file_bytes(f"LVL/{module}")
            rows.append(verify_module(data, module, known[module], args.assembler, args.linker))
    if args.all:
        content = render_catalogue(rows)
        if args.write_catalogue:
            CATALOGUE.write_text(content, encoding="utf-8", newline="\n")
        elif not CATALOGUE.is_file() or CATALOGUE.read_text(encoding="utf-8") != content:
            raise ValueError("code interval catalogue missing or stale; rerun with --write-catalogue")
        total = sum(end - CODE_START for _module, end, *_rest in rows)
        distinct = len({digest for _module, _end, digest, *_rest in rows})
        print(f"{len(rows)} placements: {total} cumulative interval bytes, "
              f"{distinct} distinct full-interval hashes; no C progress claimed")


if __name__ == "__main__":
    main()
