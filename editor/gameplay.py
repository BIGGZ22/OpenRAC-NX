"""The gameplay file's sections that hold mobys and what belongs to them, read and written
again, for a packer that adds and deletes mobys.

The file starts with one word per section, the section's offset or 0 (the order is the port's
port/assets/world/gameplay.h, after ReRAC, ISC). The ones here:

    +0x44  moby instances      count, the runtime mobys kept spare for what the game spawns
                               while it runs, pad[2], then 0x78-byte records (mobys.py)
    +0x48  moby groups         count, bytes of members, pad[2]; one s32 per group, the offset
                               of its members or -1; the members, u16 moby indices, the last
                               of a group with 0x8000 set
    +0x4c  shared data         size, count, pad[2]; a blob of `size` bytes; `count` records
                               {u16 pvar, u16 offset in its block, s32 offset in the blob}
    +0x50  pvar moby links     {s32 pvar, s32 offset} until a negative pvar: the s32 at that
                               offset of that block is a moby's index in the table
    +0x54  pvar table          {s32 offset, s32 size} per pvar, into the pvar data
    +0x58  pvar data           the blocks, each at a multiple of 16
    +0x5c  pvar pointer fixups as the links: that s32 is an offset in its own block

No section is moved unless it grows: a section that fits where it was is written there, and one
that does not goes to the end of the file with its header word changed, so every other section
stays where its own offsets expect it. Standard library only.
"""

import struct

from formats import FormatError
from mobys import SIZE

SECTIONS = 37
MOBYS, GROUPS, SHARED, LINKS, TABLE, DATA, POINTERS = 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C


def offset(g: bytes, pointer: int) -> int:
    return struct.unpack_from("<i", g, pointer)[0]


def extent(g: bytes, pointer: int) -> tuple[int, int]:
    """Where a section starts and where the next one (or the file) ends."""
    start = offset(g, pointer)
    starts = sorted(o for o in struct.unpack_from(f"<{SECTIONS}i", g, 0) if o > 0)
    later = [o for o in starts if o > start]
    return start, later[0] if later else len(g)


def records(g: bytes) -> list[bytes]:
    start = offset(g, MOBYS)
    if start <= 0:
        return []
    count, = struct.unpack_from("<i", g, start)
    return [bytes(g[start + 0x10 + i * SIZE:start + 0x10 + (i + 1) * SIZE]) for i in range(count)]


def write_records(g: bytes, items: list[bytes]) -> bytes:
    """The table with these records; the rest of its header (the spare mobys) as G has it."""
    start = offset(g, MOBYS)
    return struct.pack("<i", len(items)) + bytes(g[start + 4:start + 0x10]) + b"".join(items)


def groups(g: bytes) -> list[list[int] | None]:
    """Each group's members (moby indices), None for a group with no list."""
    start = offset(g, GROUPS)
    if start <= 0:
        return []
    count, size = struct.unpack_from("<2i", g, start)
    offsets = struct.unpack_from(f"<{count}i", g, start + 0x10)
    members = struct.unpack_from(f"<{size // 2}H", g, start + 0x10 + 4 * count)
    out: list[list[int] | None] = []
    for at in offsets:
        if at < 0:
            out.append(None)
            continue
        group, i = [], at // 2
        while i < len(members):
            group.append(members[i] & 0x7FFF)
            if members[i] & 0x8000:
                break
            i += 1
        out.append(group)
    return out


def write_groups(items: list[list[int] | None]) -> bytes:
    offsets, members = [], []
    for group in items:
        if not group:
            offsets.append(-1)
            continue
        offsets.append(2 * len(members))
        members += group[:-1] + [group[-1] | 0x8000]
    body = struct.pack(f"<{len(members)}H", *members)
    # The byte count takes in the zeros that end the section at a multiple of 16 (all 19 levels).
    body += bytes(-(0x10 + 4 * len(offsets) + len(body)) % 16)
    return struct.pack("<2i8x", len(items), len(body)) + struct.pack(f"<{len(offsets)}i", *offsets) + body


def fixups(g: bytes, pointer: int) -> list[tuple[int, int]]:
    """A list of {pvar, offset} that ends at a negative pvar."""
    at, out = offset(g, pointer), []
    if at <= 0:
        return out
    while True:
        pvar, where = struct.unpack_from("<2i", g, at)
        if pvar < 0:
            return out
        out.append((pvar, where))
        at += 8


def write_fixups(items: list[tuple[int, int]]) -> bytes:
    return b"".join(struct.pack("<2i", *item) for item in items) + struct.pack("<2i", -1, -1)


def shared(g: bytes) -> tuple[bytes, list[tuple[int, int, int]]]:
    """The shared blob and its records (pvar, offset in the block, offset in the blob)."""
    start = offset(g, SHARED)
    if start <= 0:
        return b"", []
    size, count = struct.unpack_from("<2i", g, start)
    at = start + 0x10 + size
    return bytes(g[start + 0x10:at]), [struct.unpack_from("<2Hi", g, at + 8 * i) for i in range(count)]


def write_shared(blob: bytes, items: list[tuple[int, int, int]]) -> bytes:
    return struct.pack("<2i8x", len(blob), len(items)) + blob + b"".join(struct.pack("<2Hi", *i) for i in items)


def pvar_count(g: bytes, used: list[int]) -> int:
    """How many pvars the table holds: the file has no count, so as many as its room in the
    file, and no fewer than the highest one something names."""
    start, end = extent(g, TABLE)
    if start <= 0:
        return 0
    return max((end - start) // 8, max(used, default=-1) + 1)


def pvar_table(g: bytes, count: int) -> list[tuple[int, int]]:
    start = offset(g, TABLE)
    return [struct.unpack_from("<2i", g, start + 8 * i) for i in range(count)]


def replace(g: bytes, sections: dict[int, bytes]) -> bytes:
    """The file with these sections (header pointer -> bytes) written: where each was when it
    fits there, the rest of its room zeroed, else at the end of the file, at a multiple of 16."""
    out = bytearray(g)
    for pointer, data in sections.items():
        start, end = extent(g, pointer)
        if start <= 0:
            raise FormatError(f"the gameplay file has no section at +{pointer:#x} to write")
        if len(data) <= end - start:
            out[start:end] = data + bytes(end - start - len(data))
            continue
        out += bytes(-len(out) % 16)
        struct.pack_into("<i", out, pointer, len(out))
        out += data
    return bytes(out) + bytes(-len(out) % 16)
