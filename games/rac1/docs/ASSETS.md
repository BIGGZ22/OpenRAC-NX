# Asset formats

What the extractor (OpenRAC's [editor](../../../../editor/README.md)) knows
about the data on the PAL disc (`SCES_509.16` v2.00), and the evidence for
each piece. This file describes formats and measured metadata only; it
contains no game data.

## Sources and credits

- **The executable.** The functions named below are in our decompiled C or
  generated assembly, and they are the ground truth for everything the
  game reads directly.
- **[Wrench](https://github.com/chaoticgd/wrench)** by chaoticgd and
  contributors (GPL-3.0-or-later). Most of the extractor's format knowledge
  comes from reading its source at revision `1b48f4d`. Without it, each
  of these layouts would have had to be worked out from the game's code
  and VU microcode. The table below lists what came from where, and the
  sections further down say where a layout rests on Wrench alone.
- **[ReRAC](https://github.com/re-rac/rerac)** (ISC), a native PC port of
  the US release. Its format crate and notes give the moby instance record
  and what the game's level loader does with each field
  (`crates/rc-formats/src/gameplay.rs`, `docs/plan/moby_render_notes.md`),
  the moby class format (`docs/formats/moby_rac1.md` §1–§4,
  `crates/rc-formats/src/moby.rs` and `moby_anim.rs`,
  `docs/plan/moby_skinning_lighting.md`, `docs/plan/moby_untextured.md`,
  `docs/plan/moby_animation.md`), which ReRAC built from Wrench's moby
  reader and the game's code, and the collision block and what the game's
  code does with it (`docs/formats/collision_rac1.md`,
  `docs/plan/collision_queries.md`, in turn based on Wrench's collision
  reader, `collision.cpp`). The PAL data has the same layouts; our decoders
  were checked against the PAL disc (see "Mobys", "Moby classes" and
  "Collision").
- **[Lombyte](https://github.com/mateuszklysz/Lombyte)** (MIT): the moby class
  names the editor shows (`editor/moby_classes.tsv`), which it joins
  from each level's class dispatch table and Wrench's class names.
- **[Replanetizer](https://github.com/RatchetModding/Replanetizer)** by
  RatchetModding contributors (GPL-3.0-or-later). It reads the PS3 HD
  collection's files, whose layouts differ from the PS2 disc. The earlier
  prototype consulted it for what fields mean; none of its layouts are
  used.
- **[OpenGOAL's jak-project](https://github.com/open-goal/jak-project)**.
  Its extractor was the model for this one: extract from the user's own
  disc, check the version first, and keep the output out of the
  repository.
- **[Godot Engine](https://godotengine.org)** (MIT). The sky panorama
  follows the mapping in Godot's sky shader
  (`servers/rendering/renderer_rd/shaders/environment/sky.glsl`). Class
  meshes share textures because Godot's glTF importer resolves image URIs
  to imported textures.
- **[glTF 2.0](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)**
  by the Khronos Group, the format of the extracted meshes.

The extractor contains no code from these projects; it is newly written
Python based on the format knowledge above.

| Format | Wrench source |
|---|---|
| Table of contents groups, level audio and scene references | [`table_of_contents.h`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/iso/table_of_contents.h) |
| Level data header section names | [`level_data_wad.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/wrenchbuild/level/level_data_wad.cpp) |
| Core index fields and block boundaries | [`level_core.h`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/wrenchbuild/level/level_core.h), [`level_core.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/wrenchbuild/level/level_core.cpp) |
| WAD compression (checked against; the PAL code is the reference) | [`compression.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/compression.cpp) |
| Texture tables, palette swizzle and alpha | [`level_textures.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/wrenchbuild/level/level_textures.cpp), [`texture.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/core/texture.cpp), [`textures.md`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/docs/textures.md) |
| Terrain (tfrags) | [`tfrag_low.h`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/tfrag_low.h), [`tfrag_low.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/tfrag_low.cpp), [`tfrag_high.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/tfrag_high.cpp) |
| Ties | [`tie.h`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/tie.h), [`tie.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/tie.cpp) |
| Shrubs, and their winding fix | [`shrub.h`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/shrub.h), [`shrub.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/shrub.cpp), [`gltf.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/core/gltf.cpp) |
| Sky | [`sky.h`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/sky.h), [`sky.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/engine/sky.cpp) |
| Tie and shrub instances, gameplay block offsets | [`gameplay_impl_classes.inl`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/instancemgr/gameplay_impl_classes.inl), [`gameplay.cpp`](https://github.com/chaoticgd/wrench/blob/1b48f4d1ed02de9e05b57802ab518f9ec39c23df/src/instancemgr/gameplay.cpp) |

How the decoders were checked:

- An earlier prototype of the decoders was compared on level 0 with Wrench
  readers compiled separately. WAD decompression, terrain faces and tie
  packets all matched.
- The current decoders reproduce that prototype's output exactly on all
  19 levels: every vertex, UV, face, placement and texture pixel. The one
  exception is the untextured sky shells, whose colours the prototype
  misread as texture coordinates (see "Sky").
- The moby class decoder follows ReRAC's description and loader, and
  every check its loader makes holds on all 19 PAL levels (see "Mobys").

## The disc

The ISO 9660 file system names three files:

| File | LBA | Bytes | Role |
|---|---:|---:|---|
| `SYSTEM.CNF` | 289 | 58 | Boot configuration. `func_00201E88` reads its sector and checks the region. |
| `SCES_509.16` | 290 | 1,388,100 | The executable, SHA-1 `79956931…2e15e83`. |
| `IOPRP243.IMG` | 968 | 264,449 | IOP reboot image, loaded by `func_00201E88`. |

Everything else is addressed by absolute 2048-byte sector:

- **Table of contents.** `func_0012F3F8` reads six sectors at LBA 1500
  and keeps 0x2960 bytes in `D_00137C80`. The table starts with
  `(1, 0x2960)`.
- **Global groups.** The rest of the table is groups of `(LBA, size)`
  pairs, listed at the end of this file. Sizes are in sectors, except
  MPEG (bytes) and two audio groups that store LBAs only. The names are
  Wrench's; the loaders that use the debug font (`func_001E96B8`), video
  (`func_001E99D8`) and IOP modules (`func_00201E88`) confirm three of
  them.
- **Level table.** At +0x28c8 (`D_0013A548`): 19 entries of
  (header LBA, total sectors). The total is not a contiguous extent,
  because a level's audio can sit before its header.
- **Level header.** `func_0012F4A8` reads five sectors and keeps 0x2434
  bytes, starting `(id, 0x2434)`. The fields:
  - +0x08, +0x10, +0x18 and +0x20: sector ranges for the level data,
    NTSC gameplay, PAL gameplay and occlusion. `func_00204C60` loads the
    first three.
  - +0x28: 36 (LBA, bytes) pairs.
  - +0x148: 15 music LBAs, then 30 scenes of 6 audio and 68 WAD LBAs.
    This layout is Wrench's; the payloads are not decoded yet.

## WAD compression

`func_0020C468` decompresses. After a 16-byte header, `"WAD"` then the
stream size at +3 (header included), the stream is LZO-like:

- **Literal runs:** a tag below 0x10 copies `tag + 3` bytes; tag 0 copies
  `next + 18`. Two literal runs in a row trap (`teq`).
- **Matches:** tags of 0x40 and up, 0x20 and up, and 0x10 and up give
  short, medium and far copies. A far copy adds 0x4000 to its distance.
  The low two bits of a match's second-to-last byte are 0–3 literals that
  follow it.
- **Initial literals:** a first byte above 0x11 copies `byte - 0x11`
  literals at once.
- **Block markers:** a far match with distance 0 is a no-op if its length
  is 1. Any other length skips to the next 0x2000-byte block, which the
  game refills by DMA into the scratchpad.

Wrench's generic decoder aligns this skip to 0x1000 bytes, but the PAL
code uses 0x2000. On this disc both give the same output, because every
marker sits near a 0x2000 boundary.

## Level data

The data range starts with eleven (offset, size) pairs, named after
Wrench's `RacLevelDataHeader`:

1. code overlay;
2. sound bank;
3. core index;
4. GS RAM;
5. HUD header;
6. five HUD banks;
7. core data.

The core index and core data are WAD-compressed, and so is the gameplay
range. The decompressed core data is exactly the size recorded at core
index +0x8c on every level.

### Code overlays

The first section is code. Its records are
(load address, size, type, entry point) followed by the bytes to copy.

- **Loading:** ParseBin (`func_0012DA38`) copies records until the entry
  point changes and returns it. The main loop (`func_0012DB18`) then calls
  it.
- **Size:** on every level the records fill the section exactly: 1.65–1.91
  MB, uncompressed, in seven records of types 1, 8, 1, 1, 1, 1, 1.
- **Layout:** the records replace the executable's whole `main` segment
  (`config/splat.yaml`):

  | Record | Level 0 | Executable's `main` segment |
  |---|---|---|
  | literals | 0x15f000 | `lit` at 0x15f000 |
  | bss | 0x161f00 | `bss` |
  | data | 0x166100 | `data` |
  | level vtables | three small records | `lvl_vtbl`, `lvl_camvtbl`, `lvl_sndvtbl` |
  | text | 1,078,552 bytes | `text`: 0x1E9080–0x23E730, 349,872 bytes |

- **Size of the text:** across the 19 levels the text record is 1.07–1.21
  MB. The data records share only 44–84% of their bytes with the
  executable at the same addresses.

So each level brings its own, much larger build of the game program; the
executable's `main` is the program that runs before the first level loads.

`tools/overlay_scan.py` measures the overlap. It compares instructions with
their link-dependent fields masked and splits functions at calls, returns
and tail calls (so the figures are approximate). On 2026-09-27:

| | Bytes |
|---|---|
| The executable's game code (`text`), 1,036 functions | 348K |
| Of those, found in each level's text (about 979 functions) | 321K |
| Distinct overlay code also in the executable | 270K |
| Distinct overlay code shared by two or more levels, not in the executable | 1.19M |
| ...of which in all 19 levels | 594K |
| Distinct overlay code of a single level | 2.03M (about 107K per level) |

The executable's program is a subset of every level's. All distinct game
code comes to about 3.5 MB, ten times the executable's; with the resident
SDK code (`core_text`) the whole program is about 3.7 MB. Decompiling the
levels means splitting their overlays, as `config/splat.yaml` does for the
executable, and counting each distinct function once.

### Core index

The fields follow Wrench's `LevelCoreHeader`. The runtime tables in
`func_001EABE8` use the same relative layout, and every offset below was
checked against the decompressed sizes on all 19 levels.

| Offset | Contents |
|---|---|
| +0x00 | GS RAM table: count, offset. 16-byte entries (PSM, size, GS offset). |
| +0x08, +0x0c, +0x10, +0x14 | Core data offsets of the terrain, occlusion, sky and collision blocks. |
| +0x18, +0x20, +0x28 | Moby, tie and shrub class tables. Each entry is 32, 32 or 48 bytes and starts with a core offset and a class ID, and remaps 16 texture slots at +16. |
| +0x30…+0x48 | Terrain, moby, tie and shrub texture tables: count, offset. |
| +0x60 | Core offset of texture pixel data. |
| +0x78 | Index offset of 256 ratchet-sequence core offsets. |
| +0x80 | Gadget table: count, offset. 16-byte entries starting with a core offset. |
| +0x8c | Decompressed core size. |

Core blocks carry no sizes of their own. Each one ends at the next known
block start, as in Wrench:

- the four top-level blocks;
- the texture data;
- every class, gadget and ratchet-sequence offset;
- the end of the data.

## Textures

- **Entries:** each is 16 bytes: pixel offset from the texture data,
  width, height, type, palette in 256-byte units of GS RAM, mips and pad.
  `func_00203958` uploads PSMT8 pixels with 32-bit palettes.
- **Palette check:** the extractor requires every palette to be one the GS
  RAM table lists as a 32-bit palette (PSM 0).
- **Pixels:** 8-bit indices into a 256-colour palette. The GS reads these
  palettes with index bits 3 and 4 swapped (CSM1).
- **Alpha:** runs 0–0x80, which is opaque, and is doubled for PNG.
- **Mips:** only the base level is exported.

`func_001E94E8` handles a related standalone format (PIF), which is not
extracted yet.

## Terrain (tfrags)

A block header gives the fragment table's offset and count. The layouts
follow Wrench's tfrag reader. `func_002352C8` confirms which stream each
LOD uses and the qword sizes it sends by DMA, but not the VU program's
arithmetic.

- **Fragments:** 0x40 bytes each. The fields:
  - +0x10: data offset from the table;
  - +0x14, +0x16, +0x18: the LOD-2, shared and LOD-1 streams;
  - +0x1a to +0x1e: the refinement streams;
  - +0x22 and +0x23: their qword sizes;
  - +0x28: texture count.
- **Packets:** streams are VIF command lists. Only the exact packet
  sequences found on this disc are accepted: STROW, STMOD, STCYCL, and
  unmasked V3-16, V4-8, V4-16 and V4-32 unpacks, with every address
  checked against the fragment's VU memory map.
- **Positions:** signed 16-bit offsets from the STROW origin, /1024.
  Vertex infos hold (s, t, parent, position × 2).
- **Texture coordinates:** s and t are fixed point /4096. Negative values
  are halved, as Wrench does; this is not traced in VU code yet.
- **LOD 0:** two refinement stages each add absolute positions with two
  parents. They also add vertex infos for the new positions and for
  texture seams.
- **Faces:** strip descriptors are (count, packet end, material offset,
  pad). A count of 0 ends the list, a negative count switches material,
  and even counts are runs of quads.
- **Materials:** five-qword GS primitives. Their first word is the texture
  index, until `func_00204340` patches it at load time.

## Ties

Tie class meshes follow Wrench's tie reader. `func_00236A98` confirms the
AD GIF table pointer (+0x2c), the material count (+0x23) and the 80-byte
material stride.

- **Class header:** packet table at +0x00, packet counts per LOD at +0x20,
  scale at +0x40. Positions are signed 16-bit × scale / 1024; texture
  coordinates are /4096.
- **Packets:** each LOD-0 packet holds regular (16-byte) and extended
  (24-byte) vertices. Each vertex carries the GS address it is written to,
  sometimes two.
- **Replay:** the extractor replays each packet in GS address order:
  - 6 qwords per material;
  - 1 per strip tag;
  - 3 per vertex.

  It fails on any missing or conflicting write.
- **Placements:** PAL gameplay +0x34 points to a count, then 0xe0-byte
  instances:
  - class, draw distance, pad, occlusion index;
  - a column-major matrix at +0x10;
  - 0x80 bytes of ambient colours at +0x50;
  - directional lights at +0xd0 and UID at +0xd4.

## Shrubs

Shrubs follow Wrench's shrub reader.

- **Class header:** scale at +0x20, packet count at +0x28, and 24 stored
  normals at the offset at +0x2c. A class has a billboard if its offset
  at +0x1c is positive.
- **Packets:** a header, GIF tags, 64-byte material records, then two
  arrays of signed 16-bit vertex data.
- **Replay:** the extractor replays each packet in GS address order:
  - 1 qword per tag;
  - 5 per material;
  - 3 per vertex.

  Short packets repeat their last vertex as padding.
- **Winding:** the game's strips don't keep a consistent winding, so each
  face is turned towards the average of its vertices' stored normals.
- **Placements:** PAL gameplay +0x3c points to a count, then 0x70-byte
  instances:
  - class and draw distance (float);
  - a column-major matrix at +0x10;
  - colour, three integers, at +0x50;
  - directional lights at +0x60.

In both tie and shrub placements, the matrix's W component (+0x4c) holds
0.01 on 89% of ties and 99% of shrubs, and 0.0 on the rest. Its meaning is
unknown, so the Godot scenes record it when it isn't 0.01.

## Mobys

Mobys are the level's objects with behaviour: Ratchet's spawn point,
crates, bolts, enemies, NPCs, vendors, platforms. Their placements and
their classes' high-detail meshes, skeletons and animations are
extracted.

- **Placements:** PAL gameplay +0x44 points to a count, 12 bytes of
  padding, then 0x78-byte instances (16,232 on the 19 levels):
  - +0x00 record size (always 0x78), +0x18 class, +0x1c instance scale;
  - +0x08 spawn flags and +0x0c spawn id: the save-state tests that decide
    whether the moby is created;
  - +0x20 draw distance and +0x24 update distance, integers (64 on most);
  - +0x30 position and +0x3c Euler angles in radians, rotation
    R = Rz(z) · Ry(y) · Rx(x);
  - +0x48 group, +0x58 pvar index (the instance's class variables, -1 for
    none), +0x60 mode bits, +0x64 ambient colour (three integers, 128 =
    1.0), +0x70 light sets;
  - +0x04, +0x10, +0x14, +0x4c/+0x50 (rooting), +0x54, +0x5c (occlusion)
    and +0x74 as ReRAC describes them; +0x28 and +0x2c always hold 32 and
    64.

The Godot scenes keep every field a packer needs: the transform, plus
node metadata for the fields that differ from the value most instances
store (`editor/mobys.py`, `USUAL`).

### Moby classes

The layouts are ReRAC's (`docs/formats/moby_rac1.md` §1–§2 and its
corrections); `editor/moby_class.py` reads them.

- **Class table:** core index +0x18, 32-byte entries: the core offset of
  the class blob, the class number, two unknown words and 16 texture slots
  into the moby texture table (0xff unused). Offset 0 means no blob: spawn
  points, triggers and the 21 gadget classes, whose blobs are
  WAD-compressed in the gadget table and not decoded yet. The 19 levels
  have 2,972 blobs, 2,934 of them with a mesh.
- **Header** (0x48 bytes): the packet table at +0x00 (0: no mesh), the
  high-detail, low-detail and metal packet counts at +0x04–+0x06, the
  first metal packet at +0x07, the joint count at +0x08 and the class
  scale (float) at +0x24.
- **Scale:** a vertex is drawn at packed × class scale / 1024, then the
  instance's scale, rotation and position. The GLBs keep model units
  (packed / 1024), and each class scene's `Model` node carries the class
  scale.
- **Packets:** 16-byte entries: VIF list offset and size in qwords, vertex
  table offset and size, two qword counts that follow from the vertex
  count, and the number of vertices sent to VU1. Only the high-detail
  packets are read.
- **VIF list:** NOPs and UNPACKs only:
  - texture coordinates, V2-16 with the write mask, at VU address 0xc2:
    s and t / 4096;
  - the index stream, V4-8 at 0x12d, after a 4-byte header;
  - optionally GS texture blocks, V4-32 right after the indices: 64 bytes
    each, whose TEX0 data word is the class texture slot until the game
    patches it at load time (-1: untextured).
- **Vertex table:** eight words: matrix transfers, two-way, three-way and
  single-joint vertices, duplicates, their total, the offset of the
  vertices and the offset of a colour multiplier per vertex (0x80 on the
  whole disc). Then the 2-byte matrix transfers, the duplicates (8-byte
  aligned, a cache slot × 128) and 16-byte vertices:
  - bytes 0–7: skinning (VU0 matrix slots and weights);
  - bytes 8–9: the normal's azimuth a and elevation e in 256ths of a turn,
    (cos a cos e, sin a cos e, sin e);
  - bytes 10–15: the position, signed 16 bits.
- **Vertex cache:** each vertex record holds the 9-bit cache ID of the
  vertex seven places earlier, the depth of the VU1 program's pipeline;
  the last IDs are in one to six records after the vertices. A duplicate
  copies an earlier vertex, possibly from an earlier packet, out of the
  512-entry cache and takes its own texture coordinates.
- **Index stream:** 1-based indices, bit 7 suppresses the drawing kick.
  - A kicked index draws the triangle of the last three.
  - A 0 switches to the next GS block's texture and pushes an extra
    index: the first is in the index header, the rest at byte 12 of the
    GS blocks' successive qwords.
  - An extra index of 0 ends the packet, and its last three indices
    (1, 1, 1) only flush the pipeline.
  - A packet without GS blocks keeps the previous packet's texture.
- **Winding:** strips keep no consistent winding, so each face is turned
  towards its vertices' stored normals, as with shrubs. The normals are
  decoded as ReRAC does from the game's VU0 code; Wrench swaps x and y.
  On levels 0, 5 and 9 the faces agree with the stored normals at a mean
  |cos| of 0.98, against 0.59 with x and y swapped.
- **Skinning:** the VU0 matrix slots resolve to up to three joints per
  vertex, weights in 256ths. The extractor replays them and checks every
  load, weight sum and joint number; they become the GLB's joints and
  weights.
- **Untextured faces:** for a texture of -1 the game binds an 8×8 texture
  of 0x80 texels, which leaves the vertex colour. The extractor gives
  those faces `textures/moby_untextured.png`, flat grey.
- **Not read:** low-detail and metal (chrome, glass) packets, glow
  packets' colour, collision, sounds and sound triggers, and the shadow
  block.

On all 19 levels, every class decodes with every check passing: the
counts in each packet entry and vertex table agree, every packet has one
to six records after its vertices, every duplicate finds its cache slot,
and every index stream uses all its GS blocks and ends on the flush
trailer. The high-detail meshes have 1,942,238 triangles; ReRAC counts
1,933,983 on the US disc, which has four fewer class blobs. Renders of
vendors, crates, bolts, Ratchet, NPCs and enemies on levels 0, 3 and 5
show them textured and standing on their placements.

### Moby skeletons and animations

ReRAC traced these in the game's animation evaluator (`fun_0020e0e0`) and
per-tick step (`fun_0020d580`); `editor/moby_anim.py` reads them.

- **Skeleton** (class +0x14): one matrix per joint, four rows of four
  floats. Rows 0–2 are the images of the axes and row 3 the translation,
  in packed units; the w lanes are never read. The game skins with
  F = P · S for a joint pose P, so S is the inverse bind matrix and the
  bind pose is S⁻¹. Every joint's S_parent · S⁻¹ is a rotation and a
  scale without shear (residual below 3e-7; three joints are mirrored), so
  the GLB's joint nodes hold the bind pose exactly.
- **Hierarchy** (class +0x18, `common_trans`): 16 bytes per joint, the
  rest translation relative to the parent and a word: 0 for a root, else
  0x70000000 + 0x40 × parent, a scratchpad address. Parents come first.
- **Sequences:** class +0x48 lists `sequence count` offsets (0: empty
  slot). A sequence is a 0x1c-byte header (bounding sphere, frame count,
  loop sound, trigger count and pointer, rate override at +0x18), then
  frame offsets and trigger words.
- **Frames:** a 16-byte header (rate, time in 1/8 ticks, payload qwords,
  quaternion bytes = 8 × joints, scale count, translation offset and
  count), then:
  - one quaternion per joint, four signed 16-bit values / 32768;
  - scale records: three unsigned 16-bit values / 4096, a joint and a
    flag, 0x80 when the scale is inherited by the children, 0 when it
    scales only that joint after the whole chain;
  - translation records: three signed 16-bit values in packed units and a
    joint. Joints without one use their rest translation.
- **Local matrix:** [R(q)ᵀ · diag(s) | t], so the glTF rotation is the
  conjugate quaternion. A scale applied after the chain goes on a child
  node of the joint, which the joint's vertices follow.
- **Timing:** one tick is 1/60 s (ReRAC infers the rate). Each interval
  lasts 1 / rate ticks, from the sequence's rate override or else the
  earlier frame's rate; the last frame's rate leads back to the first.
  A rate of 0 there stops the sequence on its last frame; two classes on
  level 2 store a negative one, treated the same. Five intervals on the
  disc have zero length and are merged. Looping sequences get `_loop` in
  their glTF name, which Godot's importer turns into a looping
  animation.
- **Ratchet** (class 0) has empty sequence slots. His sequences are the
  core index's 256 ratchet sequence offsets (+0x78), each with frame
  offsets relative to itself; ReRAC takes the slot as the sequence
  number.
- **Output:** a channel that never leaves the bind pose in a sequence is
  left out of that animation (Godot then holds the bone at its rest), and
  rotation keys are normalized shorts. Classes 1 and 2 have joints and
  sequences but no skeleton matrices (ReRAC finds that their translation
  records drive another skeleton), and they stay static.

On all 19 levels, every class with skeleton matrices and sequences
decodes: 1,371 classes, and Ratchet on each level. In 1,123 of them some
joint leaves the bind pose, and their GLBs have 25,947 bones and 9,900
animations; the rest stay static meshes. For the five classes ReRAC
checked on its level 1 (11, 608, 724, 754, 1365), frame 0 of sequence 0
matches the bind pose decoded from S⁻¹ within 0.005° and 0.15 packed
units, which pins the quaternion convention, the parent chain and the
units. Renders of Ratchet, the robot dogs, Big Al, the horny toads (whose
tongue joints use the post-chain scale) and the Blarg paratroopers posed
by their animations show natural poses on undistorted meshes.

## Sky

`func_00203118` relocates the sky's pointers.

- **Header:** background colour at +0, shell count at +6, texture count at
  +0xc, texture definitions and data at +0x10, and eight shell offsets at
  +0x20.
- **Shells:** each shell is (cluster count, flags), with bit 0 meaning
  untextured. Its 0x20-byte cluster headers start at +0x10.
- **Clusters:** each points to its vertices (x, y, z, alpha: signed 16
  bits, positions /1024, alpha 0x80 opaque), a second 4-byte array per
  vertex, and faces (three indices and a texture, 0xff for none).
- **Second array:** texture coordinates (s, t, /4096) on textured shells,
  but an RGBA colour on untextured ones. Its alpha always equals the
  vertex alpha.
- **Winding:** faces are wound the opposite way to the other geometry.

The field meanings and scales are Wrench's, except the second array on
untextured shells. Wrench reads it as texture coordinates there too and
paints those shells white. On this disc they are colour gradients: level
1's runs from (53, 88, 139) to (120, 169, 201).

When a level has an untextured shell, it is shell 0: a dome over the whole
sphere, drawn first. Levels 5, 7, 10 and 15 have none, and the header
colour shows wherever no shell reaches.

The extractor composites the shells in order into a panorama. Each layer
is blended over the last by its alpha: the texture's alpha times the
vertex alpha, with 0x80 as full. That is the PS2's usual MODULATE and
alpha blend, inferred rather than traced in the game's code.

## Collision

The collision block is at the core offset in index +0x14 and ends at the
next core block. All the layouts here are from ReRAC's notes; the checks
are ours, on the PAL disc.

- **Header:** two offsets, the mesh (0x40) and the hero groups (0 if none).
  The mesh ends at the hero groups, or at the end of the block.
- **Tree:** a grid of 4-unit cells, three levels deep, indexed Z, then Y,
  then X. The root holds a signed base and a count, then 16-bit entries
  (offset from the mesh / 4) to Z slabs; a slab holds a base, a count and
  32-bit offsets to Y rows; a row holds a base, a count and a word per cell.
  Zero means empty. All offsets are from the start of the mesh. A cell's
  word is its leaf's offset in bits 8-31 and its size in 16-byte units in
  the low byte.
- **Leaf:** face count (16 bits), vertex count and quad count (bytes), the
  vertices, the faces, then one byte per quad. Total size is rounded up
  to 16. A vertex is one word: X in bits 0-9 and Y in 10-19, signed and in
  1/16 units, Z in bits 20-31, signed and in 1/64 units, all from the
  cell's centre (4 × cell + 2). A face is three vertex indices and a type
  byte; the first quad-count faces are quads and take their fourth index
  from the trailing bytes. The decoder checks each leaf's declared size
  against its contents and each index against its vertex count.
- **Faces:** the game's normal is (v2 − v0) × (v1 − v0), and faces are
  one-sided. Quads are split as (v0, v1, v2) and (v0, v2, v3). A face is
  stored in every cell it touches, so the decoder merges identical
  triangles (the same three world positions and type) and then winds each
  one the other way for glTF.
- **Type byte:** bits 0-4 are the surface id (0x1f is none), 5-6 a
  footstep class and bit 7 excludes the face from some queries. Surface
  ids have no table; see the legend in the extractor's README.
- **Hero groups:** a count and records of bounding sphere, triangle and
  vertex counts and an offset, with unsigned 1/64 vertices. Only the count
  is read, so far.

Measured on the PAL disc (the 19 levels' `level.json`): 429,500 cells,
2,374,105 stored faces and 1,165,544 distinct triangles, with surface ids
0-5, 7-14 and 31, and 518 hero groups. ReRAC reports slightly lower
cell and face totals on the NTSC-U disc; the two builds' levels differ.
Drawn over the terrain (level 0, by eye), the mesh coincides with the
terrain's footprint, which agrees with the axes and scales above.

## Table of contents groups

Offsets into the table at LBA 1500, with populated and total slots. The
exact LBAs and sizes of every reference are printed by
`extract.py survey`.

| Offset | Group | Used / slots | Size unit |
|---|---|---:|---|
| 0x0008 | debug font | 1 / 1 | sectors |
| 0x0010 | save game | 1 / 1 | sectors |
| 0x0018 | ratchet sequences | 28 / 28 | sectors |
| 0x00f8 | hud sequences | 17 / 20 | sectors |
| 0x0198 | vendor | 1 / 1 | sectors |
| 0x01a0 | vendor audio | 37 / 37 | sectors |
| 0x02c8 | help controls | 12 / 12 | sectors |
| 0x0328 | help moves | 15 / 15 | sectors |
| 0x03a0 | help weapons | 15 / 15 | sectors |
| 0x0418 | help gadgets | 14 / 14 | sectors |
| 0x0488 | help ss | 7 / 7 | sectors |
| 0x04c0 | options ss | 7 / 7 | sectors |
| 0x04f8 | frontbin | 1 / 1 | sectors |
| 0x0500 | mission ss | 81 / 81 | sectors |
| 0x0788 | planets | 19 / 19 | sectors |
| 0x0820 | unknown stuff2 | 38 / 38 | sectors |
| 0x0950 | goodies images | 10 / 10 | sectors |
| 0x09a0 | character sketches | 19 / 19 | sectors |
| 0x0a38 | character renders | 19 / 19 | sectors |
| 0x0ad0 | skill images | 31 / 31 | sectors |
| 0x0bc8 | epilogue images | 60 / 60 | sectors |
| 0x0da8 | sketchbook | 30 / 30 | sectors |
| 0x0e98 | commercials | 4 / 4 | sectors |
| 0x0eb8 | item images | 9 / 9 | sectors |
| 0x0f00 | qwark boss audio | 185 / 240 | LBA only |
| 0x12c0 | IOP modules (compressed bundle) | 1 / 1 | sectors |
| 0x12c8 | spaceships | 4 / 4 | sectors |
| 0x12e8 | unknown animation | 20 / 20 | sectors |
| 0x1388 | space plates | 6 / 6 | sectors |
| 0x13b8 | transition | 1 / 1 | sectors |
| 0x13c0 | space audio | 36 / 36 | sectors |
| 0x14e0 | sound bank | 1 / 1 | sectors |
| 0x14e8 | unknown wad | 1 / 1 | sectors |
| 0x14f0 | music | 1 / 1 | sectors |
| 0x14f8 | hud header | 1 / 1 | sectors |
| 0x1500 | hud banks | 5 / 5 | sectors |
| 0x1528 | all text | 1 / 1 | sectors |
| 0x1530 | unknown things | 28 / 28 | sectors |
| 0x1610 | post credits sequence | 1 / 1 | sectors |
| 0x1618 | post credits audio | 18 / 18 | sectors |
| 0x16a8 | credits images (NTSC) | 20 / 20 | sectors |
| 0x1748 | credits images (PAL) | 20 / 20 | sectors |
| 0x17e8 | unknown wads | 2 / 2 | sectors |
| 0x17f8 | MPEG video | 84 / 88 | bytes |
| 0x1ab8 | help audio | 669 / 900 | LBA only |

## Level ranges

| Level | Header LBA | Data LBA / bytes | PAL gameplay LBA / bytes | Occlusion LBA / bytes |
|---:|---:|---:|---:|---:|
| 0 | 1,886,014 | 1,886,019 / 13,453,312 | 1,892,799 / 432,128 | 1,893,010 / 14,336 |
| 1 | 1,893,017 | 1,893,022 / 18,663,424 | 1,902,385 / 512,000 | 1,902,635 / 24,576 |
| 2 | 1,902,647 | 1,902,652 / 16,439,296 | 1,910,937 / 528,384 | 1,911,195 / 38,912 |
| 3 | 1,911,214 | 1,911,219 / 17,018,880 | 1,919,841 / 638,976 | 1,920,153 / 53,248 |
| 4 | 1,920,179 | 1,920,184 / 16,115,712 | 1,928,273 / 450,560 | 1,928,493 / 18,432 |
| 5 | 1,928,502 | 1,928,507 / 18,470,912 | 1,937,808 / 577,536 | 1,938,090 / 28,672 |
| 6 | 1,938,104 | 1,938,109 / 19,079,168 | 1,947,709 / 581,632 | 1,947,993 / 30,720 |
| 7 | 1,948,008 | 1,948,013 / 16,097,280 | 1,956,090 / 444,416 | 1,956,307 / 20,480 |
| 8 | 1,956,317 | 1,956,322 / 15,804,416 | 1,964,399 / 737,280 | 1,964,759 / 34,816 |
| 9 | 1,964,776 | 1,964,781 / 16,939,008 | 1,973,337 / 583,680 | 1,973,622 / 26,624 |
| 10 | 1,973,635 | 1,973,640 / 16,660,480 | 1,982,049 / 561,152 | 1,982,323 / 26,624 |
| 11 | 1,982,336 | 1,982,341 / 18,524,160 | 1,991,642 / 524,288 | 1,991,898 / 24,576 |
| 12 | 1,991,910 | 1,991,915 / 17,915,904 | 2,001,021 / 733,184 | 2,001,379 / 49,152 |
| 13 | 2,001,403 | 2,001,408 / 18,524,160 | 2,010,685 / 475,136 | 2,010,917 / 28,672 |
| 14 | 2,010,931 | 2,010,936 / 18,182,144 | 2,020,076 / 536,576 | 2,020,338 / 24,576 |
| 15 | 2,020,350 | 2,020,355 / 18,636,800 | 2,029,711 / 524,288 | 2,029,967 / 40,960 |
| 16 | 2,029,987 | 2,029,992 / 17,207,296 | 2,038,759 / 747,520 | 2,039,124 / 43,008 |
| 17 | 2,039,145 | 2,039,150 / 16,918,528 | 2,047,618 / 423,936 | 2,047,825 / 20,480 |
| 18 | 2,047,835 | 2,047,840 / 17,858,560 | 2,056,917 / 731,136 | 2,057,274 / 43,008 |

NTSC gameplay has the same size as PAL gameplay and sits just before it.
