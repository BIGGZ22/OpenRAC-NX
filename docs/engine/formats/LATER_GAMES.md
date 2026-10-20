# Going Commando, Up Your Arsenal and Deadlocked levels

What the three later games' level data shares with Ratchet & Clank's and
what each changes, as the editor reads it ([editor/games.py](../../../editor/games.py),
[editor/wads.py](../../../editor/wads.py)). The layouts are Wrench's
(https://github.com/chaoticgd/wrench, GPL-3.0-or-later; read and
re-implemented, no code copied); each was checked on the retail disc named
below by reading every level with the editor's readers (2026-10-20).

**Games.** RAC2 = Going Commando `SCUS_972.68` v1.01, RAC3 = Up Your Arsenal
`SCUS_973.53`, RAC4 = Deadlocked `SCUS_974.65`, all NTSC-U (**retail**).
RAC1 is in [DISC.md](DISC.md) and [LEVEL.md](LEVEL.md).

## 1. What the four games share

The level core is one format across the four games: the readers written for
RAC1 read the others' with a handful of per-game values (section 3).

| Part | Shared | Per game |
|---|---|---|
| Core index header ([LEVEL.md](LEVEL.md#2-core-index-levelcoreheader-0xbc-bytes)) | every offset: GS RAM table, tfrags, occlusion, sky, collision, the class and texture tables, textures base at +0x60, chrome and glass maps at +0x90..+0x9c, core data size at +0x8c | +0x78 is Ratchet's sequences in RAC1–3, not in RAC4; +0x80 is RAC1's gadget table; +0x84 counts the moby texture stash from RAC2 on; +0xb4 starts a core block in RAC2 and RAC3 |
| Class entries | 32 bytes for mobys and ties, 48 for shrubs: core offset, class, 16 texture slots | |
| Texture entries, palettes | 16-byte entries, 256-colour RGBA32 palettes in the GS RAM lump, the CSM1 index swap, alpha 0x80 = opaque | RAC4's pixels are swizzled (section 2.4); RAC2–4 stash some moby textures in GS RAM |
| WAD compression ([WAD.md](WAD.md)) | the same stream, 0x2000-byte blocks | |
| Terrain ([TFRAG.md](TFRAG.md)) | the same fragments and VIF programs | RAC2–4 stream extra terrain in chunks (section 2.5) |
| Shrubs ([SHRUB_SKY.md](SHRUB_SKY.md)) | class blocks and 0x70-byte instance records | |
| Ties ([TIE.md](TIE.md)) | packets, strips and vertices | the class header (section 2.6); instance records 0xe0 in RAC1, 0x60 after |
| Mobys ([MOBY.md](MOBY.md)) | the 0x48-byte class header, packet table, VIF lists, index streams, vertex records, skins | the vertex table header (section 2.7), RAC4's skeleton (section 2.8), the instance record (section 2.9) |
| Sky | header, texture definitions, clusters, faces | the shell header (section 2.10) |
| Collision ([COLLISION.md](COLLISION.md)) | the same grid | RAC2–4 stream extra collision in chunks |

## 2. What differs

### 2.1 The disc

RAC1 keeps its table of contents at sector 1500 ([DISC.md](DISC.md)). The
three later discs keep theirs at sector 1001 (RAC2 also names it `RC2.HDR`):

1. Global WAD headers, each starting with its own size and its file's
   sector: 9 on RAC2's disc, 8 on RAC3's (the last padded with zeros, so the
   level table starts at +0x7418), 8 on RAC4's.
2. The level table: three (sector, sector count) pairs per level, each
   pointing at a header inside the table of contents. Which pair is which
   kind differs (RAC2: level, audio, scene; RAC3 and RAC4: audio, level,
   scene), so a header's kind is told by its size: level 0x60 (RAC2, RAC3)
   or 0xC68 (RAC4); audio 0x1018, 0x1818 or 0x2A0; scene 0x137C or 0x26F0.
   The table starts at +0x4FF8 (RAC2), +0x7418 (RAC3), +0xC430 (RAC4).
3. A header's word +4 is its file's first sector; its sector ranges are
   relative to it. On RAC2's disc the level files are also named
   (`G/LEVELn.WAD`, at the same sectors).

| | RAC2 | RAC3 | RAC4 |
|---|---|---|---|
| Table entries | 27 | 52 (one, the multiplayer menu's audio, without a level) | 47 |
| Levels | 27: 0–20, 22–26, 30 | 51: 1–14, 16–24, 26–36, 39–55 | 47: 0–2, 4–8, 10–11, 13–15, 21–22, 24–28, 30–31, 33–35, 41–42, 44–48, 50–51, 53–54, 61–62, 64–68, 70–71, 73–74 |

A level is numbered by its level header's word +8, as the games load it and
as Wrench names it: the table slot on RAC4's disc, the slot plus one on
RAC3's, and the slot on RAC2's but for the Insomniac Museum (slot 21,
level 30).

### 2.2 The level header

| Offset | RAC2, RAC3 (0x60) | RAC4 (0xC68) |
|---|---|---|
| +0x00 | header size | header size |
| +0x04 | the file's sector | the file's sector |
| +0x08 | level number | level number |
| +0x0C | reverb | reverb |
| | +0x10 level data, +0x18 sound bank, +0x20 gameplay, +0x28 occlusion | +0x10 the largest mission sizes, +0x18 level data, +0x20 sound bank |
| | +0x30 chunks (six ranges: three chunks, three sound banks) | +0x28 chunks; +0x58 gameplay; +0x60 128 mission ranges each for instances, data and sound banks; +0xC60 art instances |

Ranges are (sector, sector count). No RAC2 header on the retail disc has
the 0x68-byte form Wrench also knows (with separate NTSC and PAL gameplay
copies).

### 2.3 The level data

| | RAC1 | RAC2, RAC3 | RAC4 |
|---|---|---|---|
| Slots | 11 | 11 | 14 |
| Order | overlay, sound bank, core index, GS RAM, HUD header, 5 HUD banks, core data | overlay, core index, GS RAM, HUD header, 5 HUD banks, core data, transition textures | moby 8355's pvars, overlay, core index, GS RAM, HUD header, 5 HUD banks, core data, art instances, gameplay, global nav data |

In RAC4 the instances live in the level data: its gameplay file (mobys,
paths, shapes...) and its art instances (directional lights, ties, shrubs,
their groups and occlusion), both WAD-compressed. The editor reads those
two from the level data and does not use the level header's gameplay
range (+0x58).

### 2.4 Textures

- **Stash (RAC2–4).** The GS RAM table goes on past its count (+0x00) with
  index +0x84 more entries; the moby classes listed at +0xAC (s16, ending
  at a negative one) keep their texture pixels in the GS RAM lump from the
  first such entry's address, not in the core data.
- **Swizzle (RAC4).** An 8-bit texture's pixels are stored in the GS's
  PSMT8 block order. Wrench's `map_pixel_index_rac4` turns them back
  (written out in [level.py](../../../editor/level.py) `unswizzle`); its sky
  textures are turned the other way, as Wrench reads them. The level
  texture direction was checked by eye on one RAC4 terrain texture; the sky
  direction is Wrench's and not checked.
- **Chrome map.** None of the three games' levels has a chrome map in the
  core (+0x90 is 0), though their metal packets use one: those faces are
  drawn grey. The glass map (+0x98) is there.

### 2.5 Chunks

A large level keeps part of its terrain and collision in up to three
chunks, which the game swaps in as the player moves (Wrench's
`ChunkWadHeader`). Each is a sector range of the level file starting with
`{s32 tfrags, s32 collision}`, offsets of a WAD stream each. The editor
reads chunks 1 and 2
into the same terrain and collision meshes (chunk 0, as Wrench reads it,
is the core's). Levels with chunks: RAC2 1, 2,
4, 7, 8, 11, 19, 20; RAC3 4, 7, 8, 9, 10, 19, 24; RAC4 1, 5, 14, 15 and
their copies (21, 25, 34, 35, 41, 45, 54, 61, 65, 74).

### 2.6 Tie class header

| Field | RAC1 | RAC2–4 |
|---|---|---|
| packet table offsets (3 LODs) | +0x00 | +0x00 |
| packet counts (3 bytes) | +0x20 | +0x0C |
| texture count | +0x23 | +0x0F |
| AD GIF records | +0x2C | +0x1C |
| light-slot normals | +0x0C | +0x34 |
| scale | +0x40 | +0x40 |

From RAC3 on, a strip's last byte is a flag (any non-zero value turns the
strip's winding; Wrench's `rc34_winding_order`), where RAC1 and RAC2 store
0 or 1. The per-vertex light slots RAC1 keeps at packet bytes 0xA and 0xB
mean something else after it (`rgba_count`, `multipass_ofs`) and are not
read. RAC2–4 keep each tie instance's ambient colours in a block of their
own (gameplay +0x94, RAC4 art instances +0x20), variable-sized; not read.

### 2.7 Moby packets

The vertex table header is eight u16 in RAC2–4 (Wrench's
`GcUyaDlVertexTableHeader`), eight u32 in RAC1; after the vertices there is
no block of per-vertex colour multipliers, so the epilogue's length comes
from the packet's data size. In RAC2 a class whose header byte 0xB is set
keeps RAC1's tables (Wrench's `force_rac1_format`); in RAC3 and RAC4 that
byte is the team textures. RAC1's byte 0xA (the first glow packet) is not
read for the later games. Some classes leave texture slots empty that their
faces use (their pictures come from elsewhere at run time, not traced):
those faces are drawn untextured.

### 2.8 Skeletons and animation

- RAC1–3: 4×4 bind matrices, the parent named by its matrix's scratchpad
  address (0x70000000 + 0x40 × parent).
- RAC4: 3×4 matrices (three rows of four floats, the translation in the
  fourth column) and the parent named by its number (bit 7 sometimes set;
  0x7F for none), as Wrench's `recover_moby_joints` reads them.
- Animations: RAC2's and RAC3's sequences are read with RAC1's frame
  reader; frames flagged in their top nibble (Wrench's "special data") are
  not, and their class keeps its skeleton without animations. RAC4's
  sequences have a format of their own (Wrench's `read_dl_moby_sequence`)
  and are not read.

### 2.9 Instances

| | RAC1 | RAC2, RAC3 | RAC4 |
|---|---|---|---|
| Ties | gameplay +0x34, 0xE0 bytes | gameplay +0x34, 0x60 bytes | art instances +0x08, 0x60 bytes |
| Shrubs | +0x3C, 0x70 bytes | +0x40, 0x70 bytes | art instances +0x14, 0x70 bytes |
| Mobys | +0x44, 0x78 bytes | +0x4C, 0x88 bytes | gameplay +0x30, 0x70 bytes |

Each table is a count, 12 bytes of padding, then the records. The moby
records ([games.py](../../../editor/games.py) has every field):

| Field | RAC1 | RAC2, RAC3 | RAC4 |
|---|---|---|---|
| size | +0x00 | +0x00 | +0x00 |
| mission | | +0x04 | +0x04 |
| uid (spawn id) | +0x0C | +0x10 | +0x08 |
| bolts | | +0x14 | +0x0C |
| class | +0x18 | +0x28 | +0x10 |
| scale | +0x1C | +0x2C | +0x14 |
| position, rotation | +0x30, +0x3C | +0x40, +0x4C | +0x28, +0x34 |
| group | +0x48 | +0x58 | +0x40 |
| pvar index | +0x58 | +0x68 | +0x50 |
| colour, light | +0x64, +0x70 | +0x74, +0x80 | +0x5C, +0x68 |

The usual values the editor leaves out of a moby's metadata were measured
on every level: 17,422 RAC2 instances, 23,221 RAC3, 9,740 RAC4. Some RAC2
instances store an infinite rooted distance.

### 2.10 Sky

A shell starts with its cluster count and flags: two s32 in RAC1 and RAC2,
two s16 in RAC3 and RAC4, followed there by the shell's rotation and
angular velocity (three s16 each; not read).

## 3. What the editor reads, per game

Counts are from reading every level of each disc (`editor/extract.py port`,
2026-10-20): RAC2 27 levels, 9,442 textures, 4,444 moby models of 5,796
classes, 1,864 tie and 358 shrub classes, 17,422 mobys, 42,671 ties and
28,901 shrubs placed; RAC3 51 levels, 17,534 textures, 6,996 moby models of
9,630 classes, 2,743 tie and 688 shrub classes, 23,221 mobys, 59,067 ties,
57,061 shrubs; RAC4 47 levels, 13,693 textures, 5,685 moby models of 8,376
classes, 3,184 tie and 919 shrub classes, 9,740 mobys, 59,606 ties, 66,668
shrubs (each level counted on its own: a class or texture in several
levels counts once per level).

| | RAC2 | RAC3 | RAC4 |
|---|---|---|---|
| Levels read | 27 of 27 | 51 of 51 | 47 of 47 |
| Terrain (core and chunks) | yes | yes; one fragment of level 23 left out | yes |
| Ties, shrubs, their placements | yes | yes | yes |
| Moby class meshes and placements | yes | yes | yes |
| Moby skeletons | yes; one class left out | yes | yes; 50 class copies over the levels left out |
| Moby animations | yes (7,289) | yes (13,910), but 8 class copies with flagged frames | no |
| Textures | yes; the chrome map is missing | yes; the same | yes (swizzled); the same |
| Sky | yes | yes | yes |
| Collision (core and chunks) | yes | yes | yes |
| Lighting at load, tie ambient colours | no | no | no |
| Moby class names | no | no | no |
| Missions (RAC4's per-mission instances and classes) | | | no |
| Packing back | no | no | no |

What a level could not read is listed in its `level.json` (Godot) or
`manifest.json` (`stats.problems`).

## Open

- RAC4's moby animation format, and RAC2's and RAC3's flagged frames.
- The tie ambient colour blocks and the directional lights (gameplay +0x04
  in RAC2 and RAC3, art instances +0x00 in RAC4), for lighting as the games
  do: that also needs each executable's normal table, which is known for
  RAC1 only.
- Where the later games' chrome map is, and the pictures of the moby classes whose
  texture slots are empty.
- RAC4's missions: mission instances and classes, loaded on top of the
  level by game mode.
- The level packer for the later games: their gameplay records, the level
  header's ranges and RAC4's two instance files.
