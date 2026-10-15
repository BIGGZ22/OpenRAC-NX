# Level collision

The level's static collision: one mesh per level, baked in world space from
terrain, ties and shrubs, stored in a sparse grid of 4-unit cells so that a
query reads one cell; plus hero-only groups (invisible walls), the gameplay
file's shape volumes, and the camera collision grid. The layout is Wrench's
as ReRAC describes it, confirmed by ReRAC from the game's query code on the
NTSC-U disc (`SCUS_971.99`, all 19 levels). OpenRAC's editor reads it on PAL
with the same results ([ASSETS.md](../../../games/rac1/docs/ASSETS.md#collision)).
How the game queries it: [COLLISION_QUERIES.md](../systems/COLLISION_QUERIES.md).
Moby class collision is a separate format ([MOBY.md](MOBY.md#8-collision-bangles-gif-usage)).

**Games.** RAC1, measured. Wrench uses one reader and writer for all four
games, so the cell format is the same in its model (**reference**). In the
sequels a level has up to three collision blocks, chunk 0 in the core data
and chunks 1–2 in the streamed chunk files, and the core reserves room for
the largest. RAC2's and RAC3's shape and camera-grid sections sit at other
gameplay offsets, and their point lights carry a built-in mask grid instead
of RAC1's separate grid. None of this is measured in OpenRAC.

## 1. Block

At core index +0x14, in the decompressed core data, with no stored size.

| Offset | Content |
|---|---|
| 0x00 | offset of the mesh tree (0x40 on every level) |
| 0x04 | offset of the hero groups; non-zero on every RAC1 level, even with zero groups |
| 0x08–0x3F | zero |

The mesh runs from its offset to the hero groups.

## 2. The grid

Cells of 4 × 4 × 4 units, aligned to multiples of 4, looked up Z, then Y,
then X. All offsets are from the mesh start.

| Level | Content |
|---|---|
| root | s16 z base, u16 z count, then u16 entries: offset / 4 of a slab, 0 empty |
| slab | s16 y base, u16 y count, then u32 offsets of rows, 0 empty |
| row | s16 x base, u16 x count, then u32 cell words, 0 empty |

**Cell word**: bits 8–31 the leaf's offset, bits 0–7 its size in quadwords
(the DMA count the game uses to copy the leaf to the scratchpad; it equals
⌈(4 + 4V + 4F + Q) / 16⌉ on all 429,484 NTSC-U cells, so a leaf is under
0x1000 bytes). The game reads the bases unsigned and only answers queries
inside [0, 1024)³; all retail coordinates are ≥ 0 (up to cell 200, 220, 111).

## 3. Leaf

| Field | Content |
|---|---|
| u16 | face count (quads + triangles) |
| u8 | vertex count, at most 255 |
| u8 | quad count |
| u32 × V | packed vertices |
| 4 bytes × F | faces: three vertex indices and a type byte; the first quad-count faces are quads |
| u8 × Q | the fourth index of each quad |

Padded to 16 bytes.

**Vertex**: X in bits 0–9 and Y in 10–19 (signed, 1/16 unit), Z in bits 20–31
(signed, 1/64), from the cell centre (4 × cell + 2). The range ±32 units lets a
large face keep all its vertices in every cell it touches. The game's decode
confirms the axes and scales.

**Faces**: a face is stored in every cell it overlaps, so one cell answers a
query. The game's normal is (v2 − v0) × (v1 − v0); faces are one-sided
unless the query asks otherwise. Quads are (v0, v1, v2) and (v0, v2, v3).
Wrench reverses the winding in both directions.

**Type byte**: bits 0–4 the surface id (0x1F: none), bits 5–6 a footstep
sound class, bit 7 excludes the face from queries that ask. There is no table
of surface ids in the game; what the code does with them is in
[COLLISION_QUERIES.md](../systems/COLLISION_QUERIES.md#4-surface-types). ReRAC
counted 39 distinct bytes on NTSC-U; 31 (0x1F) and 12 dominate.

**Totals**, NTSC-U: 429,484 cells, 4,687,748 vertices, 2,373,683 faces
(1,312,557 quads). OpenRAC on PAL: 429,500 cells and 2,374,105 faces; the
two builds' levels differ slightly.

## 4. Hero groups

Player-only geometry, not in the grid: a count, padding to 16, then 16-byte
groups: bounding sphere (u16 x, y, z, radius at 1/64), triangle count (the
game reads a byte; at most 214 on the disc), vertex count, data offset from
the section. Data: vertices of u16 x, y, z at 1/64 (absolute, unsigned) and
a zero, then triangles of three u8 indices and a zero. No type byte. Hero
coordinates are confined to 0–1024 units. 516 groups on NTSC-U (518 on PAL);
six levels have none.

## 5. Shapes and the camera collision grid (gameplay file)

- **Shapes** (cuboids 0x60, spheres 0x64, cylinders 0x68, pills 0x6C): 0x80
  bytes each, a forward matrix (column-major, translation in column 3),
  columns 0–2 of the inverse, Euler angles. A cuboid is the [−1, 1]³ cube
  through the matrix. The other shapes' canonical forms are not established.
  The stored W of the forward matrix is 0.01, as in ties and shrubs.
- **Camera collision grid** (0x84): 64 × 64 cells of 16 units over X and Y
  (0–1024), each a list of 0x30-byte primitives: bounding sphere (z = 0),
  volume type (3 cuboid, 5 sphere, 6 cylinder, 7 pill), shape index, flags, an
  integer and a float whose meanings are unknown.
- **Point-light grid** (0x78, RAC1 only): the same 64 × 64 cells, each a
  count and point-light indices. Wrench rebuilds it from the lights instead
  of reading it, so its exact rule is not established.

## Open

- RAC1: the surface ids as behaviours; the camera primitive's flags and
  values; the canonical sphere, cylinder and pill.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/collision_rac1.md`,
`docs/plan/collision_queries.md`; PAL facts from
[ASSETS.md](../../../games/rac1/docs/ASSETS.md).
