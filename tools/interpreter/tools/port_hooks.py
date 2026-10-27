#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Carries a hooks table from one game's program to another's.

    port_hooks.py FROM.iso FROM_EXE FROM.hooks TO.iso TO_EXE > TO.hooks

A hooks table says at which address of a program each replaced library
function starts. Another version of the game, or another game built with the
same libraries, has the same functions at other addresses. This finds each one
again by the shape of its first instructions: the instruction words with
everything that depends on where things were linked (jump targets, the
immediates of address-forming and memory instructions) masked out.

Both programs are read from your own disc images. The output holds addresses
and the names from the table given, nothing of either program.

A function found in more than one place is taken at the same distance from
its old address as the function before it, when one of the places is there;
otherwise the line is written as a comment that starts "# NOT FOUND" and
lists the places, to be settled by hand.
"""
import struct
import sys

SECTOR = 2048

# How many instructions to compare, longest first: a long shape is surer, a
# short one still finds a short function.
LENGTHS = (48, 32, 24, 16, 12, 8)


def iso_file(iso, name):
    """Returns the bytes of a file in the root directory of a disc image."""
    with open(iso, "rb") as disc:
        # The primary volume descriptor is sector 16; its root record is at byte 156.
        disc.seek(16 * SECTOR)
        root = disc.read(SECTOR)[156:190]
        sector, size = struct.unpack_from("<I", root, 2)[0], struct.unpack_from("<I", root, 10)[0]
        disc.seek(sector * SECTOR)
        records = disc.read(size)
        at = 0
        while at < len(records):
            length = records[at]
            if length == 0:
                # Records do not cross sectors: go on at the next one.
                at = (at // SECTOR + 1) * SECTOR
                continue
            sector, size = struct.unpack_from("<I", records, at + 2)[0], struct.unpack_from("<I", records, at + 10)[0]
            found = records[at + 33:at + 33 + records[at + 32]].decode("ascii", "replace")
            if found.split(";")[0] == name:
                disc.seek(sector * SECTOR)
                return disc.read(size)
            at += length
    raise SystemExit(f"{name} is not in the root directory of {iso}")


def largest_segment(elf):
    """Returns (address, bytes) of the largest loaded segment of an ELF file."""
    table, = struct.unpack_from("<I", elf, 28)
    entry_size, count = struct.unpack_from("<HH", elf, 42)
    segments = []
    for n in range(count):
        kind, offset, address, _, size, _ = struct.unpack_from("<IIIIII", elf, table + n * entry_size)
        if kind == 1 and size:
            segments.append((address, elf[offset:offset + size]))
    return max(segments, key=lambda segment: len(segment[1]))


def shape(word):
    """Masks out of an instruction what depends on where the program was linked."""
    op = word >> 26
    # J and JAL: the target.
    if op in (2, 3):
        return word & 0xFC000000
    # ADDI, ADDIU, ORI, LUI, the 64-bit loads and stores, and every load and store: the immediate.
    if op in (8, 9, 13, 15, 26, 27, 30, 31) or 32 <= op <= 63:
        return word & 0xFFFF0000
    return word


def main():
    if len(sys.argv) != 6:
        raise SystemExit(__doc__)
    from_iso, from_exe, hooks, to_iso, to_exe = sys.argv[1:6]
    from_address, from_bytes = largest_segment(iso_file(from_iso, from_exe))
    to_address, to_bytes = largest_segment(iso_file(to_iso, to_exe))
    to_shapes = [shape(word) for word in struct.unpack_from(f"<{len(to_bytes) // 4}I", to_bytes, 0)]
    distance = None
    for line in open(hooks):
        text = line.rstrip("\n")
        fields = text.split()
        if not fields or fields[0].startswith("#"):
            print(text)
            continue
        address = int(fields[0], 16)
        found = None
        for length in LENGTHS:
            offset = address - from_address
            if offset < 0 or offset + length * 4 > len(from_bytes):
                continue
            wanted = [shape(word) for word in struct.unpack_from(f"<{length}I", from_bytes, offset)]
            places = [
                to_address + n * 4
                for n in range(len(to_shapes) - length)
                if to_shapes[n] == wanted[0] and to_shapes[n:n + length] == wanted
            ]
            if len(places) == 1:
                found = places[0]
                break
            if len(places) > 1:
                near = [place for place in places if place - address == distance]
                found = near[0] if len(near) == 1 else [f"{place:08X}" for place in places]
                break
        if isinstance(found, int):
            distance = found - address
            print(f"{found:08X}" + text[8:])
        else:
            print(f"# NOT FOUND {found} {text}")


main()
