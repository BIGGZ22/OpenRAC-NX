# Occlusion data

RAC1's precomputed visibility: a grid of 4-unit cells, each naming a
1,024-bit mask of what can be seen from it, an optional eight-mask override
for cameras outside the grid, and the gameplay file's mappings that give
each terrain fragment, tie and moby its bit. ReRAC read all three from the
game code of the NTSC-U disc (`SCUS_971.99`); Wrench's picture of the grid
agrees, its reading of the mappings does not (section 3). How the game uses
them each frame: [OCCLUSION.md (systems)](../systems/OCCLUSION.md).

**Games.** RAC1, measured. Wrench describes the same 4 × 4 × 4 octants and
128-byte masks for the sequels (**reference**); not measured in OpenRAC. RAC1
also keeps a sector-padded copy of the mappings as the level header's
`occlusion` range ([DISC.md](DISC.md#5-the-level-header-0x2434-bytes)).

## 1. The grid (core index +0x0C)

In the decompressed core data; no size stored; ends in 0–48 bytes of padding.

| Offset | Type | Content |
|---|---|---|
| 0x00 | s32 | offset of the mask array |
| 0x04 | u16 | z base |
| 0x06 | u16 | z count |
| 0x08 | u16 × count | z slots: offset of a y node in 4-byte units, 0 empty |
| y node | u16, u16, u16 × n | y base, count, slots: offset of an x node in 4-byte units |
| x node | u16, u16, u16 × n | x base, count, slots: **mask index**, 0xFFFF empty (0 is a valid mask) |
| masks | 0x80 × n | bit b is `mask[b >> 3] & (1 << (b & 7))` |

All bases and counts are unsigned. The mask count is not stored: it is the
highest index any cell names plus 1 (on every retail level, every mask is
used and the array fills the block). Several cells can share a mask.

**Cell of a camera**: `cell = (int)(camera × 0.25)` per axis, truncated
towards zero, so cell 0 covers (−4, 4).

The lookup is `ParseOcclGrid` (NTSC-U boot 0x1F2690, PAL `func_001F2A38`):
z, then y, then x, each `0 ≤ c − base < count`, then the slot, else no cell.

## 2. Octant override (core index +0xA8)

Present on levels 11, 13 and 17 only: 0x410 bytes, a centre (three floats
and 0) and eight masks indexed `(cam.x > cx) × 4 + (cam.y > cy) × 2 + (cam.z >
cz)`. Used only outside the grid with fallback mode 2
([systems](../systems/OCCLUSION.md)). Core index words +0xB0 and +0xB8 are
0xA000 and 0xB400 on every level and no occlusion code reads them.

## 3. Mappings (gameplay file +0x8C)

Three counts (terrain, ties, mobys), pad, then 8-byte records `{s32 bit,
s32 id}`: terrain records, then ties, then mobys. Bits run 0..1021 and may be
shared. At load the level loader turns each record into a u16 **occlusion
word** `(bit >> 3) << 8 | 1 << (bit & 7)` (byte index, bit mask):

| Kind | Stored in | Match rule |
|---|---|---|
| terrain | fragment header 0x3A | record i to fragment i, **only if** the counts agree and every fragment's triangle count (header 0x3D) equals the record's id, a staleness check; otherwise every fragment gets 0x7F80 and the game prints "occlusion out of date on tfrag" |
| tie | run-time tie +0x18 (first set to the instance's occlusion index) | positional when counts and keys agree, else the first record whose id equals the key |
| moby | run-time moby +0x36 | only mobys whose instance occlusion word (+0x5C) is 0: the first record whose id equals the spawn id (+0x0C) |

Anything unmatched, and every moby spawned later, gets 0x7F80: bit 1023,
which the per-frame builder always sets, so "always visible". Without a
grid or mappings the loader prints "no occlusion" and turns occlusion off.
Retail takes the simple paths on all 19 levels.

The on-disc fragment header word at 0x3A is 0 everywhere: it is a run-time
slot, and Wrench's reading of it as an occlusion index does not apply to RAC1.
Shrubs have no occlusion data.

**Totals** (NTSC-U, 19 levels): 230,109 cells, 139,110 masks. 83 fragments,
1,435 ties and 22 mobys are visible from no cell (probably hidden inside
geometry; not checked).

## Open

- Terrain fragment header byte 0x2B (`occl_index_stash`) and core index
  words +0xB0, +0xB8.
- The gameplay meaning of the octant override.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/occlusion_rac1.md`; PAL names from
`games/rac1/config/overlays/us_map.tsv`.
