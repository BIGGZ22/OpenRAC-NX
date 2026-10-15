# Textures, palettes and GS RAM

How a RAC1 level stores its textures: 8-bit indexed pixels in the core data,
palettes and small images in a GS memory image (`gs_ram`), and tables in the
core index that tie them together. Also the standalone PIF images of the
global data. The layouts are ReRAC's, written from Wrench and checked by
ReRAC on all 9,104 level textures of the NTSC-U disc; OpenRAC's extractor
reads the same layout on PAL, where the game's upload is `func_00203958`
(NTSC-U 0x203120) ([ASSETS.md](../../../games/rac1/docs/ASSETS.md#textures)).

**Games.** RAC1, measured. Wrench's code paths say the sequels differ
(**reference**): RAC2, RAC3 and Deadlocked keep some moby textures resident
in GS memory (a "stash" appended to the GS RAM table), and Deadlocked's
pixels are swizzled. Neither is measured in OpenRAC.

## 1. Where things are

| Lump | Compressed | Holds |
|---|---|---|
| core index | no | texture tables, class texture slots, GS RAM table, particle definitions |
| core data | WAD | the shared pixel data from `textures_base` (core index +0x60), the particle bank (+0x64), the effect bank (+0x68) |
| `gs_ram` | no | palettes, reduced mip images, shrub billboards: an image the game sends to GS memory |

No texture has a header of its own; size, format and palette are all in the
tables.

## 2. Texture entries (16 bytes)

Four tables in the core index: terrain (+0x30), moby (+0x38), tie (+0x40),
shrub (+0x48).

| Offset | Type | Field |
|---|---|---|
| 0x0 | s32 | pixel offset from `textures_base` in the core data |
| 0x4 | s16 | width |
| 0x6 | s16 | height |
| 0x8 | s16 | type |
| 0xA | s16 | palette, in 0x100-byte blocks of `gs_ram` |
| 0xC | s16 | mip image, in 0x100-byte blocks of `gs_ram`; −1 none |
| 0xE | s16 | −1 |

- **Pixels**: width × height bytes, one palette index each, row-major from
  the top, not swizzled. Sizes are powers of two, at least 8 wide.
- **Type**: Wrench writes 3 for every RAC1 texture and never reads it; the
  NTSC-U disc has 4 (7,375 textures), 3 (1,012), 1 (623) and 2 (11). What
  the values select is not known.
- **Mips**: Wrench writes one quarter-size image (every fourth pixel) and
  never reads it back; the retail layout is not established.

## 3. Palettes

- 256 entries of four bytes R, G, B, A, read at `gs_ram + palette × 0x100`.
- **CSM1 order**: the stored palette has bits 3 and 4 of the index swapped
  where they differ: `linear[i] = raw[(bit3(i) != bit4(i)) ? i ^ 0x18 : i]`.
  In each group of 32 entries the middle two blocks of 8 trade places. The
  mapping is its own inverse.
- **Alpha** runs 0 to 0x80, 0x80 opaque. For an 8-bit image,
  `a < 0x80 ? a × 2 : 255`. Whether RAC1 uses alpha above 0x80 (the later
  games use it for bloom and reflectivity, per Wrench) is unknown.
- The GS RAM table (core index +0x00) lists palettes (format 0x00 RGBA32;
  0x01 RGBA16 is declared but never seen) and 8-bit images (0x13). OpenRAC's
  extractor requires every palette to be a format-0 entry; it holds on all
  19 PAL levels.

## 4. Which texture a mesh uses

- A moby, tie or shrub class entry has 16 bytes of indices into its kind's
  texture table, ending at the first 0xFF (so at most 15).
- Geometry selects a texture by the low word of a TEX0 register in its GS
  setup data, which is the **slot** in the class's list. The game patches
  that word into GS register values at load: terrain in `RelocateTfrags`
  (PAL `func_00204918`, NTSC-U 0x2040E0), ties in `tie_ad_gif_convert`
  (`func_00203F68`, NTSC-U 0x203730), shrubs in the shrub class init
  (`func_00204340`, NTSC-U 0x203B08), mobys in the moby class loader
  ([TFRAG.md](TFRAG.md#5-textures-the-gs-setup)).
- Mobys: −1 no texture, −2 chrome (environment map), −3 glass. The core index
  holds the chrome and glass map at +0x90–0x9C ([MOBY.md](MOBY.md)).
- Terrain: one material per terrain texture entry, in table order.
- Gadgets use the textures of the moby class entry with their class number.

## 5. Particle and effect textures

- **Particle texture entries** (core index +0x50, 16 bytes): palette offset,
  unknown, pixel offset (both from the particle bank), side length. Square,
  8-bit, CSM1 palette.
- **Particle definitions** (core index +0x6C): `{count; unknown; index
  offset; index count}`, then `count` offsets into a byte array of frame
  indices into the particle texture table; 0 is an unused particle. A
  particle's frames run to the next used particle's start. Wrench writes 81
  particles when it rebuilds RAC1. The particle system that uses them is in
  [PARTICLES.md](../systems/PARTICLES.md).
- **Effect texture entries** (core index +0x58): palette offset, pixel offset
  (from the effect bank), width, height; −1 for an absent one. Wrench has
  names for the sequels' effect slots but none for RAC1's.

## 6. Shrub billboards

The shrub class entry's billboard record ([LEVEL.md](LEVEL.md#4-class-tables))
gives width, height, palette and pixel offsets in 0x100-byte blocks of
`gs_ram`. The pixels live in `gs_ram`, not in the core data. The billboard
geometry record inside the shrub class is in [SHRUB_SKY.md](SHRUB_SKY.md).

## 7. PIF images (global data)

Help screens, planets, mission and option screenshots, item images, the
debug font, sketches and the epilogue are PIF files, most of them
WAD-compressed. PAL's `func_001E94E8` handles the format.

| Offset | Field |
|---|---|
| 0x00 | `"2FIP"` |
| 0x04 | file size (Wrench treats it as unused) |
| 0x08 | width (at most 2048) |
| 0x0C | height |
| 0x10 | format: 0x13 8-bit with 256 colours, 0x94 4-bit with 16 colours |
| 0x14, 0x18 | palette format and order, not interpreted |
| 0x1C | mip count |

Then the palette and the mips from largest to smallest. 4-bit pixels hold
two indices per byte, high nibble first. 8-bit palettes are in CSM1 order;
4-bit ones are not. RAC1's PIF pixels are linear.

Related global formats: **texture lists** (`count`, then offsets of PIFs in
the same blob; RAC1's `space_plates`), and RAC1's credits images, raw RGBA
with no header, 512 × 416 (NTSC) and 512 × 448 (PAL).

## 8. What a native renderer does with this

Textures are converted once at load and identified by table and index, not
by GS address, because the game streams textures through one area of GS
memory each pass ([RENDERER.md, section 5](../../port/RENDERER.md#5-the-graphics-chips-conventions-the-shaders-reproduce)).
ReRAC samples moby textures with repeat (every moby GS setup it read has
CLAMP_1 = 0) and bilinear filtering.

## Open

- RAC1: the `type` values; the retail mip layout and the TEX1 words that
  select mip levels; whether `gs_ram` entries carry real GS addresses; RGBA16
  palettes; the effect slots' roles.
- RAC2–RAC4: not measured in OpenRAC.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/textures_rac1.md` and
`docs/plan/moby_render_notes.md` section 4; PAL facts from
[ASSETS.md](../../../games/rac1/docs/ASSETS.md).
