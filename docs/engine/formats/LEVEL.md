# Level data, the core index and the gameplay file

What a RAC1 level's sectors hold once the disc header has located them
([DISC.md](DISC.md)): the level data container, the core index that maps the
level's art, the block layout of the decompressed core data, and the
gameplay file of placed instances. The layouts are Wrench's as ReRAC
describes them, with ReRAC's corrections from the game's loader; where
OpenRAC checked them on the PAL disc, [ASSETS.md](../../../games/rac1/docs/ASSETS.md)
says so and is linked.

**Games.** RAC1 (NTSC-U by ReRAC, PAL by OpenRAC). The core index, class
and texture tables are the same in Going Commando, Up Your Arsenal and
Deadlocked, measured on their retail discs; what differs (the level data's
slots, the instance records, the moby texture stash) is in
[LATER_GAMES.md](LATER_GAMES.md). RAC2's level data header was also
measured on prerelease discs ([LEVEL-ARCHIVE-FORMAT.md](../../../games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md)).

**Addresses.** NTSC-U addresses are ReRAC's (`SCUS_971.99`); PAL names come
from `games/rac1/config/overlays/us_map.tsv`. A PAL name of the form
`func_L00_<address>` is level code, named by its address in level 00's
program; ReRAC usually cites the copy in level 01 (Novalis).

## 1. Level data container

`data` in the level header points at an uncompressed container whose 0x58-byte
header holds byte ranges relative to its start:

| Offset | Range | Compressed | Content |
|---|---|---|---|
| 0x00 | overlay | no | the level's code ([DISC.md](DISC.md#7-code-overlays-ratchet-executable)) |
| 0x08 | sound_bank | no | the level's 989snd bank ([SOUND_BANKS.md](SOUND_BANKS.md)) |
| 0x10 | core_index | no | the core index (section 2) |
| 0x18 | gs_ram | no | an image of GS memory: texture pixels and palettes ([TEXTURES.md](TEXTURES.md)) |
| 0x20 | hud_header | no | |
| 0x28 | 5 × hud_banks | each WAD | HUD graphics |
| 0x50 | core_data | one WAD stream (`coredata`) | geometry, collision, sky, classes, pixels |

Absent entries are `{-1, 0}`; Wrench aligns each lump to 0x40 when packing.
The sequels drop `sound_bank` and add `transition_textures` (RAC2, RAC3;
Deadlocked adds more), per Wrench.

## 2. Core index (`LevelCoreHeader`, 0xBC bytes)

Two address spaces meet here, the most common source of mistakes:
**index** offsets are from the start of the core index; **data** offsets are
from the start of the decompressed core data. Tables are `{s32 count; s32
offset}`, count first.

| Offset | Field | Space | Content |
|---|---|---|---|
| 0x00 | gs_ram | index | table of 16-byte GS RAM entries (section 5) |
| 0x08 | tfrags | data | terrain block ([TFRAG.md](TFRAG.md)) |
| 0x0C | occlusion | data | a copy of the occlusion data, 0 if none |
| 0x10 | sky | data | sky block, 0 if none ([SHRUB_SKY.md](SHRUB_SKY.md)) |
| 0x14 | collision | data | collision block ([COLLISION.md](COLLISION.md)) |
| 0x18 | moby_classes | index | 0x20-byte entries |
| 0x20 | tie_classes | index | 0x20-byte entries |
| 0x28 | shrub_classes | index | 0x30-byte entries |
| 0x30–0x48 | tfrag, moby, tie, shrub textures | index | 16-byte texture entries |
| 0x50 | part_textures | index | 16-byte particle texture entries |
| 0x58 | fx_textures | index | 16-byte effect texture entries |
| 0x60 | textures_base | data | base of all texture pixel offsets |
| 0x64 | part_bank | data | particle pixel bank |
| 0x68 | fx_bank | data | effect pixel bank |
| 0x6C | part_defs | index | particle definitions; not decoded by Wrench ([PARTICLES.md](../systems/PARTICLES.md)) |
| 0x70 | sound_remap | index | sound remap table (section 6) |
| 0x74 | unknown | | |
| 0x78 | ratchet_seqs | index | 256 data offsets of Ratchet's animation sequences; 0 unused |
| 0x7C | scene_view_size | data | a watermark after the class geometry; use unknown |
| 0x80 | gadget_count | | RAC1 only |
| 0x84 | gadget table | index | 16-byte entries (section 7) |
| 0x88 | compressed size of core data | | |
| 0x8C | decompressed size of core data | | equals the decoded size on all 19 PAL and NTSC-U levels |
| 0x90, 0x94 | chrome map texture, palette | | the environment map of the moby metal pass ([MOBY.md](MOBY.md)) |
| 0x98, 0x9C | glass map texture, palette | | Wrench packs 0x4000 and 0x400, which look like GS addresses |
| 0xA0 | unknown | data? | |
| 0xA4 | heightmap | | not decoded |
| 0xA8, 0xB0, 0xB8 | occlusion octant and radius data | | not decoded |
| 0xAC | moby GS stash | | not used in RAC1 |
| 0xB4 | moby sound remap | | Deadlocked in practice |

0x30 further bytes follow the header in the index; their meaning is
unknown. PAL's runtime tables in `func_001EABE8` (NTSC-U 0x1EA830) use the
same relative layout
([ASSETS.md](../../../games/rac1/docs/ASSETS.md#core-index)).

## 3. Blocks inside the core data

Blocks have start offsets but no sizes. A block ends at the smallest known
start above its own, from: the four top-level blocks, `textures_base`, the
decompressed size, every class blob offset, every non-zero Ratchet sequence
offset, every gadget offset and, when non-zero, the moby sound remap. The
terrain block opens the data and ends at the first non-zero of occlusion,
sky, collision. A start of 0 is an absent block.

RAC1 has no chunk streaming: the one implicit chunk is the core data's
terrain and collision. The sequels stream three chunks, each with its own
sound bank (Wrench).

## 4. Class tables

| Table | Entry | Fields |
|---|---:|---|
| Moby classes | 0x20 | data offset of the class blob (0: none), class number, two unknown words, 16 slots into the moby texture table |
| Tie classes | 0x20 | the same layout, slots into the tie texture table |
| Shrub classes | 0x30 | the same first 0x20 bytes, then a billboard record |

The shrub billboard record (16 bytes, s16 each): width (0: no billboard),
height, highest mip level, palette offset and texture offset in `gs_ram`, and
three mip offsets. It addresses `gs_ram` directly, not through a texture
entry. Class blob formats: [MOBY.md](MOBY.md), [TIE.md](TIE.md),
[SHRUB_SKY.md](SHRUB_SKY.md).

## 5. GS RAM and texture tables

GS RAM entries (16 bytes): pixel format (0x00 RGBA32 palette, 0x01 RGBA16
palette, 0x13 8-bit indexed), s16 width and height, GS address, byte offset
in `gs_ram`. RAC1 never appends a moby texture stash (the sequels do).
Texture entries, particle and effect texture entries: [TEXTURES.md](TEXTURES.md).

## 6. Sound remap table

An 8-byte header of s16 fields: second part offset and size, third part
offset and count. The table's size is found from the four bytes just before
the second part, read as `{s16 offset; s16 size}`: offset + size × 4. The
elements are not decoded.

## 7. Gadget table (RAC1 only)

16-byte entries: data offset of a **WAD-compressed** moby class blob, class
number, compressed size, pad. A gadget's textures are those of the moby
class entry with the same class number. The sequels keep gadgets in a global
`GADGET.WAD` (Wrench). PAL has 21 gadget classes per level, not decoded yet
([ASSETS.md](../../../games/rac1/docs/ASSETS.md#moby-classes)).

## 8. The gameplay file

`gameplay_ntsc` and `gameplay_pal` are each one WAD stream. Decompressed, the
file opens with 37 s32 offsets from its start (0: absent):

| Offset | Section | Records |
|---|---|---|
| 0x00 | level settings | section 8.1 |
| 0x04 | directional lights | 0x40: colour A, direction A, colour B, direction B (each four floats) |
| 0x08 | cameras | 0x20: type, position, rotation, pvar index |
| 0x0C | sound instances | 0x90: s16 class, s16 m-class, a runtime function pointer, pvar index, range, matrix, inverse matrix (3 rows), rotation |
| 0x10–0x2C | help messages | US English, UK English, French, German, Spanish, Italian, Japanese, Korean (section 8.2) |
| 0x30, 0x38, 0x40 | tie, shrub, moby class lists | `count`, then class numbers |
| 0x34 | tie instances | 0xE0 (section 8.4) |
| 0x3C | shrub instances | 0x70 (section 8.4) |
| 0x44 | moby instances | 0x78 (section 8.3) |
| 0x48 | moby groups | `count; data_size`, offsets into a u16 member array (index = offset / 2; negative: empty) |
| 0x4C | shared data | `data_size; pointer_count`, the data, then 8-byte `{u16 pvar, u16 offset, s32 shared offset}` |
| 0x50 | pvar moby-link fixups | `{s32 pvar, u32 offset}` until pvar < 0 |
| 0x54 | pvar table | `{s32 offset, s32 size}`; the count is 1 + the largest pvar index used by mobys, cameras and sounds |
| 0x58 | pvar data | |
| 0x5C | pvar relative-pointer fixups | as 0x50 |
| 0x60–0x6C | cuboids, spheres, cylinders, pills | 0x80: matrix, inverse matrix (3 rows), rotation |
| 0x70 | paths | `count; data_offset; data_size`, offsets, then count-prefixed runs of four-float points |
| 0x74 | grind paths | the same, with 0x20 records before: bounding sphere, unknown, closed-loop flag, inactive |
| 0x78 | point-light grid | RAC1 only; 64 × 64 grid (section 8.5) |
| 0x7C | point lights | 0x20: position, radius, RGB bytes |
| 0x80 | environment transitions | section 8.5 |
| 0x84 | camera collision grid | 64 × 64 grid (section 8.5) |
| 0x88 | environment sample points | 0x30 (section 8.5) |
| 0x8C | occlusion mappings | `{tfrag, tie, moby}` counts, 8-byte pairs ([OCCLUSION.md](OCCLUSION.md)) |
| 0x90 | unused | |

Sections are 16-byte aligned, except help messages (unpadded) and occlusion
mappings (0x40). Most tables start with `{s32 count; s32 pad[3]}`. The
pointer order differs from every sequel; RAC1 has no tie or shrub groups, no
separate tie colour section and no areas (Wrench).

### 8.1 Level settings (0x50 bytes)

Background colour (three s32, −1: none), fog colour (the same), fog near and
far distance, fog near and far intensity, death height, ship position and
rotation about Z, ship path, ship camera cuboids (first, last). Their use is
in [CAMERA_FOG.md](../systems/CAMERA_FOG.md).

### 8.2 Help messages

`{s32 count; s32 size}`, `size` counting the header. Entries of 16 bytes:
string offset from the section, then s16 id, short id, third-person id, co-op
id, voice clip, character. Korean uses another encoding. The runtime text
path is in [HUD_TEXT.md](../systems/HUD_TEXT.md).

### 8.3 Moby instances (0x78 bytes)

A 16-byte header `{static_count; spawnable_count; pad[2]}`; `spawnable_count`
is the engine's reserve of runtime mobys, not the array length. ReRAC traced
what the loader does with each field: the loop in NTSC-U level 01 0x255958
(PAL `func_L00_002422D8`) makes each record a 0x100-byte runtime moby,
calling `InitMobyInstance` (NTSC-U 0x20C5F0, PAL `func_0020D440`) first.

| Offset | Field | Where the loader puts it (moby +X) |
|---|---|---|
| 0x00 | record size, 0x78 | the loop's stride |
| 0x04 | unknown | +0xB0 (byte); an index into a table in the spawn test |
| 0x08 | spawn flags | the spawn test; flag 0x10 goes to +0xB1 |
| 0x0C | spawn id | +0xB2 (s16), the bit tested in the save state |
| 0x10, 0x14 | unknown | +0xB4, +0xB6 (s16) |
| 0x18 | class | `InitMobyInstance(moby, class)` |
| 0x1C | f32 scale | +0x2C = class scale (class +0x24) × scale |
| 0x20 | draw distance | +0x32 (s16). An integer (64 on most), not the float Wrench declares |
| 0x24 | update distance | +0x30 (byte) |
| 0x28, 0x2C | always 32, 64 | not used |
| 0x30 | f32 position[3] | +0x10 |
| 0x3C | f32 rotation[3], radians | +0x40; R = Rz · Ry · Rx |
| 0x48 | group | +0x21 (byte) |
| 0x4C, 0x50 | rooted flag, distance | when set, height = ground below + distance |
| 0x54 | unknown | |
| 0x58 | pvar index, −1 none | +0x78 |
| 0x5C | occlusion | 0 → +0x36 = 0, else 0x7F80 |
| 0x60 | mode bits | ORed into +0x34 and into the class header's +0x44 |
| 0x64 | s32 r, g, b | the ambient colour at +0x3C–0x3E; 128 = 1.0 |
| 0x70 | light | s32 at +0x38: light set 0, light set 1, cross-fade |
| 0x74 | unknown | not −1 → a per-moby hook |

The ground probe is NTSC-U level 01 0x26E618 (PAL `func_00214358`); the
rotation rows and bounding sphere are built next by NTSC-U 0x20DEF8 (PAL
`func_0020ED48`, `moby_build_rotation` in [RENDERER.md](../../port/RENDERER.md)).
The spawn test and the save bits it reads are in
[GAME_STATE.md](../systems/GAME_STATE.md); the runtime record in
[MOBY.md](MOBY.md).

### 8.4 Tie and shrub instances

Tie instance (0xE0): class, draw distance, pad, occlusion index, a
column-major matrix at 0x10, 0x80 bytes of per-vertex ambient colours inline
at 0x50, directional light set at 0xD0, uid at 0xD4. Wrench's comments put
the last fields at 0x50 by mistake; the record size settles it. In the
sequels the tie instance is 0x60 bytes and the colours are a separate
section (Wrench).

Shrub instance (0x70): class, f32 draw distance, matrix at 0x10, colour
(three s32) at 0x50, directional light set at 0x60.

In both, the matrix's W component (+0x4C) is 0.01 on most placements and 0.0
on the rest, meaning unknown (measured on PAL,
[ASSETS.md](../../../games/rac1/docs/ASSETS.md#shrubs)). Lighting from
these fields: [LIGHTING.md](../systems/LIGHTING.md).

### 8.5 Grids, transitions and sample points

- **Camera collision grid** and **point-light grid**: a 16-byte header, then
  64 × 64 s32 cells (index y × 64 + x), each 0 or an offset from the grid to a
  list starting with a count. Camera list: 0x30-byte primitives at list +
  0x10 (bounding sphere, volume type 3 cuboid, 5 sphere, 6 cylinder, 7 pill,
  shape index, flags, an integer and a float value). Point-light list: point
  light indices from list + 4. Wrench rebuilds the light grid instead of
  keeping it, so its packing rule is not certain.
- **Environment transitions**: a count, `count` 16-byte records (unknown;
  probably bounding spheres), then 0x80-byte records: inverse matrix, two hero
  colours and lights, flags, two fog colours, and near/far distance and
  intensity for each of the two fogs.
- **Environment sample points** (0x30): position, 1.0, hero colour (three
  s32), hero light, reverb depth, type, delay, feedback, enable byte, music
  track. RAC1's have no fog colour.

## Open

- RAC1: moby instance fields 0x04, 0x10, 0x14, 0x54, 0x74; the mode bits one
  by one (those ReRAC traced are in [MOBY.md](MOBY.md)); the unknown core
  index fields; the occlusion mapping pair's layout; the sound remap and
  particle definition records.
- RAC2–RAC4: not measured in OpenRAC.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/wad_layouts_rac1.md` sections 2–6 and
`docs/plan/moby_render_notes.md` section 1; PAL facts from
[ASSETS.md](../../../games/rac1/docs/ASSETS.md) and
`games/rac1/config/overlays/us_map.tsv`.
