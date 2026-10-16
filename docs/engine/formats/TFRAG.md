# Terrain fragments (tfrags)

The level's terrain: the core data's first block, cut into fragments of up
to 255 positions, each a set of VIF command lists that the terrain VU1
program draws in three levels of detail with a smooth morph between them.
The record layouts are Wrench's as ReRAC describes them; what the game does
with them (texture setup, level-of-detail choice, morph, lighting) is
ReRAC's reading of the NTSC-U code (`SCUS_971.99`) and of VU1 program 55907,
checked on all 20,016 tfrags of the 19 NTSC-U levels. OpenRAC's editor reads
LOD 0 and 2 on PAL; PAL's `TfragProc` `func_002352C8` confirms the streams
and DMA sizes ([ASSETS.md](../../../games/rac1/docs/ASSETS.md#terrain-tfrags)).

**Games.** RAC1, measured. Wrench uses one reader for all four games and
branches on the game in two places only: Deadlocked's header slot 0x32 and
swizzled textures; the sequels also put tfrag blocks in streamed chunks
(**reference**). VU1 programs 55907 and 903379 are byte-identical in RAC2
([VU_PROGRAMS.md](../../port/VU_PROGRAMS.md)); RAC3 and RAC4 are not
measured.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Level-load init: LOD distances, pointers, texture setup | 0x2040E0 | `RelocateTfrags` `func_00204918` |
| LOD distances, every frame | 0x233068 | `SetTfragDists` `func_00234380` |
| Per-frame cull, LOD choice, chain | 0x233FB0 | `TfragProc` `func_002352C8` |
| Per-frame texture address patch | 0x233308 | `PatchTfragGifs` `func_00234620` |
| Per-frame texture paging DMA | 0x234D48 | `func_00236060` |
| Texture usage from texture spheres | 0x234BD8 | `ComputeTfragTextureUsage` `func_00235EF0` |
| Vertex lighting | 0x234F98 | `LightTfrags` `func_002362B0` |
| View context (feeds the LOD slope) | level 01 0x219580 | `UpdateViewContext` `func_001F3140` |

The level programs carry their own copies; ReRAC found level 01's at the boot
address + 0x74988 for the tfrag routines, with data shifted by +0xC0.

## 2. Block header and fragment table

Block header (16 bytes): table offset (0x40 in practice), fragment count,
**LOD base distance L** (f32, world units; 12 to 32 on retail levels), and an
unknown word.

Fragment header (0x40 bytes):

| Offset | Type | Field |
|---|---|---|
| 0x00 | 4 × f32 | bounding sphere, absolute, in raw units (1024 = 1 world unit), radius included |
| 0x10 | s32 | data offset, from the start of the fragment table |
| 0x14 | u16 | LOD-2 list (0 in RAC1) |
| 0x16 | u16 | common list |
| 0x18 | u16 | LOD-1 list |
| 0x1A | u16 | LOD-0+1 list |
| 0x1C | u16 | GS setup payload (after the V4-32 unpack code) |
| 0x1E | u16 | colour array; end of the last list |
| 0x20 | u8 | common list size, quadwords |
| 0x21–0x23 | u8 | LOD-2, LOD-1 and LOD-0 transfer sizes (section 4) |
| 0x24–0x26 | u8 | colour entries sent for LOD 2, 1, 0 |
| 0x27 | u8 | base only: always LOD 2 |
| 0x28 | u8 | texture count |
| 0x29 | u8 | colour array size, quadwords |
| 0x2A | u8 | VU address of the colour unpack |
| 0x2B | u8 | unknown (occlusion related) |
| 0x2C | u8 | texture sphere count |
| 0x2D | u8 | flags, unknown |
| 0x2E | u16 | texture spheres |
| 0x30 | u16 | the fragment origin (four s32), then the per-vertex light records |
| 0x32 | u16 | end of the light records (Deadlocked: another pointer) |
| 0x34 | s8 | light set for the whole fragment; −1 (all retail data) = per vertex |
| 0x35 | u8 | relight flag, set when the point-light list empties |
| 0x36 | u16 | point-light slots, four nibbles, 0xF ends; reset to 0xFFFF at load |
| 0x38 | u16 | eight clip-box corners, 4 × s16 each, × 64 raw units |
| 0x3A | u16 | occlusion index ([OCCLUSION.md](OCCLUSION.md)) |
| 0x3C | u8 | position count, at most 255 |
| 0x3D | u8 | LOD-0 triangle count |
| 0x3E | u16 | texture paging distance, raw units |

The data block holds, in order: the LOD-2, common, LOD-1, LOD-0+1 and LOD-0
lists, the colour array (four bytes per position), the origin quadword and
the light records (eight bytes per position), the texture spheres and the
clip box.

## 3. The command lists

The lists are VIF1 code: STROW, STMOD (mode 1 adds the row while
unpacking), STCYCL (1/2 writes positions two quadwords apart, leaving room
for a colour), and unmasked unpacks V3-16, V4-8, V4-16 and V4-32, addressed
relative to the double buffer (FLG set). No MSCAL is in the data.

| List | Content |
|---|---|
| LOD 2 | LOD-2 indices and strips |
| common | the 5-quadword VU header (V4-16), the GS setup (V4-32, 5 quadwords per texture), common vertex infos, the origin row, common positions |
| LOD 1 | LOD-1 strips and indices |
| LOD 0+1 | LOD-01 parent indices, extra-entry indices, vertex infos, positions (all optional) |
| LOD 0 | LOD-0 positions, strips, indices, parent indices, extra-entry indices, vertex infos |

STROW rows: indices get the vertex-info base address added, so an index byte
becomes a VU address; texture coordinates get 0x45000000 (2048.0) added, so
the program reads 2048 + s/4096; positions get the fragment origin.

**Records.**

- Position: three s16 offsets from the origin; world = (origin + offset) /
  1024. Z is up, right-handed.
- Vertex info: s, t (signed, /4096), the address of the second LOD parent's
  position, the address of its own position (position index = address / 2).
- Strip: count (signed), flag, GS setup offset (quadwords, setup index =
  offset / 5), pad. 0 ends the list.
- Index arrays: bytes into the concatenated vertex infos (common, LOD-01,
  LOD-0), padded to four.

**The VU header** (20 u16 values, five quadwords): position counts of the
three tiers, the extra vertex-info counts of each tier and their addresses,
the base addresses of positions, vertex infos per tier, indices, parent and
extra-entry indices per tier, strips and GS setup. Each vertex-info tier is
one entry per position followed by its extra entries (verified on all
retail tfrags). The strip and index arrays of the three LODs share one
address: the list sent is the one drawn.

**Empty regions alias.** An empty region gets the address of the next one;
a reader that classifies unpacks by address alone must skip regions whose
count is zero (18,117 of 20,016 tfrags have no LOD-01 extra entries).

**One buffer** is 0x148 quadwords; the VU1 program double-buffers it.

## 4. Strips to triangles

ReRAC's reading of program 55907's strip processor (it supersedes Wrench's
pseudocode):

- The first record always loads a GS setup (index z / 5); its count is x + 128.
- x > 0: a strip of x vertices.
- x = 0: end of list; the packet is kicked.
- x < 0, count x + 128: if y ≥ 0, load setup z / 5 into the open packet; if
  y < 0, kick the packet, then load setup z / 5 if z ≥ 0 (z = −1 keeps the
  last GS registers).

Each strip is written behind its own GIF tag with NLOOP = count, as a GS
triangle strip; walking each as count − 2 triangles gives the header's
triangle count on every retail tfrag. Counted on NTSC-U: 137,980 setup loads,
28,838 kicks, of which 12,221 also load a setup. Loading setups only on
y ≥ 0, as Wrench does, gives 198,052 triangles the wrong texture.

Winding is not consistent and no face flag exists: the GS draws both sides.
Wrench's exporter makes even runs into quads and halves negative texture
coordinates; neither is what the game does.

## 5. Textures: the GS setup

Each texture's setup is five A+D quadwords: TEX0_1, TEX1_1, CLAMP_1,
MIPTBP1_1, MIPTBP2_1 (verified on all 50,067 retail setups). On the disc
they hold packed fields: TEX0 low word = index into the terrain texture
table; TEX1 = LOD K (s16, 1/16 units) and MMIN; CLAMP = wrap S and T (0
repeat, 1 clamp). The w lane of the TEX1 quadword is 2048.0, which the
program reads as the texture-coordinate bias.

At level load the init (PAL `RelocateTfrags`) rewrites the data halves in
place from the texture entry: TEX0 with TBW = max(1, w >> 6), PSMT8, log2
sizes, TCC 1, MODULATE, CBP = palette + base, CLD 4; TEX1 with MXL = mip
count − 1, MMAG linear, MMIN from the disc (4, linear-mipmap-nearest), K;
CLAMP from the disc, with the texture index kept in MINV for the per-frame
patch; MIPTBP1 with mip 2 and 3 addresses. What this establishes:

- The texture entry's `type` is the mip level count (4 for 64 to 256 pixels,
  3 for 32); its `mipmap` field is mip 2's GS block and `pad` mip 3's (−1 on
  three-level textures). Mips 2 and 3 stay resident; the base level and mip
  1 are paged.
- Mip 1 follows the base level in the core data (offset + w × h). All 1,639
  retail terrain textures are square, and each mip is the 2 × 2 average of
  the one above.
- K ranges from −8.5625 to −5.5625; the GS picks mip `round(log2(1/|Q|) + K)`
  clamped to 0..MXL, bilinear within it.

Every frame `TfragProc` lists the visible fragments whose near distance is
within the paging distance, the paging DMA uploads their textures, and
`PatchTfragGifs` writes the paged TBP0 and TBP1 into each listed setup.
Paging is per texture sphere (16 bytes: centre, u16 radius, u8 m, u8
texture): base and mip 1 when near ≤ m × 64, mip 1 only when near ≤ m × 128.
A renderer with every texture resident can use wrap from CLAMP, linear
magnification, nearest-mip minification and the K bias.

## 6. Level of detail

**Distances.** From L: D0 = 6L (LOD 1 to 2), D1 = 4L (LOD 0 to 1), D2 = 2L
(LOD 0 starts to morph). `SetTfragDists` turns them into raw thresholds
(× 1024) for `TfragProc` and into clip-w values (× the view's w-per-unit
slope) for VU1 constants at quadwords 666–669: morph slopes and intercepts
for the LOD-01 and LOD-0 tiers and two collapse thresholds.

**Choice, per fragment.** `TfragProc` takes the view-space depth of the
bounding sphere, near = depth − r and far = depth + r (truncated), and culls
against the frustum. A fragment crossing the guard band is retested with its
eight clip-box corners; if one is outside the guard band, it is drawn by the
clipping program 903379 at LOD 0 with no morph. Otherwise the first match:

| Condition | MSCAL | Draws |
|---|---|---|
| base only, or near ≥ D0 | 6 | LOD 2, no morph |
| far ≥ D0 | 8 | LOD 1, LOD-01 morph and collapse at D0 |
| near ≥ D1 | 0xA | LOD 1, LOD-01 morph |
| far ≥ D1 | 0xE | LOD 0, LOD-01 morph, LOD-0 morph and collapse at D1 |
| far ≥ D2 | 0x10 | LOD 0, LOD-0 morph |
| otherwise | 0x14 | LOD 0, no morph |

LOD 2 is sent as one transfer from the data start, LOD 1 as one from the
common list, LOD 0 as two (common, then LOD-0+1 and LOD 0); colours follow,
the count from the header's per-LOD colour fields.

**Morph.** For each primary entry of the LOD-01 or LOD-0 tier, with its own
transformed position c and two parent positions P1 (from the parent index
array) and P2 (from the vertex info): `c' = lerp(c, (P1 + P2) / 2, u)` and
the same for the colour; texture coordinates do not morph. In depth terms,
`u_LOD01 = clamp((depth − 4L) / 2L, 0, 1)` and
`u_LOD0 = clamp((depth − 2L) / 2L, 0, 1)`. LOD-0 parents can be LOD-01
vertices already morphed this frame.

**Collapse** (modes 8 and 0xE): when both parents lie beyond the threshold,
the entry's vertex info is replaced by parent 1's, so its triangles become
degenerate. The extra entries use the extra-entry index array as their
parent 1 for the same test; that array is their collapse replacement. At the
mode boundaries the weights are exactly 0 or 1, so switching modes is
seamless.

## 7. Colour and lighting

The stored colour array is a placeholder the game never shows: at level
load, and every frame for fragments with point lights, `LightTfrags`
computes each vertex's colour from its light record and writes it over the
array; VU1 copies it. The light record:

| Offset | Field |
|---|---|
| 0x0 | u16 byte offset of the vertex's position (for point lights) |
| 0x2 | u8 azimuth, u8 elevation: indices into a 256-entry (cos, sin) table |
| 0x4 | u16 base colour, 5:5:5:1 |
| 0x6 | u16 light set selector: set, or two sets and a blend weight |

The computation is in [LIGHTING.md](../systems/LIGHTING.md#1-terrain-lighttfrags).
Lit colours are 0..255 with 0x80 = 1.0 under GS MODULATE.

## 8. The VU1 program, in short

Program 55907 transforms each position by a 4 × 4 matrix the EE sends per
fragment, divides by w, adds a screen offset, and derives fog from w: the GS
fog value is clamp(w + offset, low, high), and adding 2048 sets the ADC bit
on triangles touching the guard band (they are dropped, not clipped).
Texture coordinates are sent as (s·Q, t·Q, Q). Output is double-buffered GS
packets of ST, RGBAQ, XYZF2 per vertex. The program's entry points, constant
block and the clipping program are in [VU1.md](../systems/VU1.md#terrain-55907-and-903379).

## Open

- RAC1: the block header's last word; the header flags; the w lane of a
  position after a V3 unpack; the exact sign of the w slope (inferred from
  the fog arithmetic, not measured at run time).
- RAC2: the VU1 programs are the same; whether the tfrag records are is not
  measured. RAC3, RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/tfrag_rac1.md`,
`docs/plan/vu1_tfrag_analysis.md`, `docs/plan/tfrag_lighting.md`; PAL names
from `games/rac1/config/overlays/us_map.tsv` and
[RENDERER.md](../../port/RENDERER.md).
