# Ties (instanced scenery)

Ties are the level's instanced static scenery: one class mesh in three levels
of detail, placed many times by 0xE0-byte instance records, lit per
instance in 64 light slots. The layouts start from Wrench's reader as ReRAC
describes it; ReRAC corrected most of the RAC1 header, packet header and
vertex records from the game's `TieProc`, `LightTies` and VU1 program 13507,
and checked them on every class, packet and instance of the NTSC-U disc
(1,804 classes, 42,725 packets, 1,662,131 triangles, 44,712 instances).
OpenRAC's editor reads LOD 0 and the placements on PAL; PAL's
`func_00236A98` confirms the material table pointer, count and stride
([ASSETS.md](../../../games/rac1/docs/ASSETS.md#ties)).

**Games.** RAC1, measured. Wrench gives the sequels a larger class header
with different offsets and a 0x60-byte instance without the colours
(**reference**). VU1 programs 13507 and 224979 were edited for RAC2 (none of
their chunks is the same, [VU_PROGRAMS.md](../../port/VU_PROGRAMS.md)).
RAC3 and RAC4 are not measured.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Cull, LOD, morph factor, DMA chain | 0x235BE8 (level 01: 0x2A9A90) | `TieProc` `func_00236F00` |
| Lighting | 0x237370 (level 01: 0x2AB218) | `LightTies` `func_00238688` |
| GS setup conversion at load | 0x203730 | `tie_ad_gif_convert` `func_00203F68` |
| Texture paging | 0x2370C0 | `BuildTieTextureDma` `func_002383D8` |
| Level-load cap of the draw distance (720) | 0x1E9B10 | `func_001E9EC8` |

## 2. Class blob

All offsets are relative to the blob, so a class can be moved freely. The
RAC1 header is 0x80 bytes, followed by an extension up to the first packet
table:

| Offset | Type | Field |
|---|---|---|
| 0x00 | 3 × s32 | packet tables of LOD 0, 1, 2 |
| 0x0C | u32 | 64 light-slot normals: s16 (x, y, z, 0) in 1/32767, 0x200 bytes, just before the GS setups |
| 0x10 | 3 × f32 | near, mid, far distances (world units) |
| 0x1C | f32 | equal to 0x48 on disc; `TieProc` uses these bytes as per-frame LOD counters |
| 0x20 | 3 × u8 | packet counts per LOD |
| 0x23 | u8 | texture count |
| 0x24 | u16 | mode: `TieProc` skips the class when `& 9`; `(& 6) >> 1` selects its path (0, 2, 4 on disc) |
| 0x26, 0x28 | | 0 on disc; instance count and list at run time |
| 0x2C | u32 | GS setup table (0x50 bytes per texture) |
| 0x30 | 4 × f32 | bounding sphere |
| 0x40 | f32 | scale |
| 0x44 | s32 | the class number |
| 0x48 | f32 | a distance (5 to 20), unknown |
| 0x50 | 3 × 16 | per LOD: strip vertex total, triangle total, strip count, pad |
| 0x80 | | bounding box min and max, then 8 corners `TieProc` uses for frustum tests; the first packet table is at 0x140 (0xC0 on one class) |

Position in class space: s16 × scale / 1024. Packet header (16 bytes): data
offset from the LOD's packet table; GS setup count; 5 × that count; 3 +
strip count; control size in quadwords (unpack header and strips); vertex
region offset and size (quadwords); colour-index region offset and element
count; a slot step table offset and size (not used by 13507); strip count;
strip vertex total.

## 3. Packet data

| Offset | Content |
|---|---|
| 0x00 | four s32: GS packet positions of setups 1..3 (setup 0 is at 0) |
| 0x10 | four s32: byte offsets into the class's GS setup table (/ 0x50 = material) |
| 0x20 | unpack header, 12 bytes |
| 0x2C | strips, 4 bytes each: vertex count, pad, GIF tag position, winding (0 in RAC1) |
| vertex region | "dinky" vertices (16 bytes) then "fat" vertices (24 bytes) |
| colour-index region | two copies of one byte per vertex: the light slot; copy B = copy A + 0x40 |

At most four materials per packet.

- **Dinky vertex**: s16 x, y, z; GS slot; s, t (/4096); q; second GS slot
  (or 0).
- **Fat vertex**: an s16 **morph delta** (x, y, z), GS slot, s16 position,
  pad, s, t, q, second GS slot. Its colour index element is four bytes (c0,
  c1, c2, 0xFF).
- **Unpack header**: dinky-single-only flag, no-fat flag, an unknown byte,
  strip count, the GS slots that end the four write phases (dinky single,
  dinky double, fat single, fat double), dinky size + 4, 3 × fat count + 6,
  dinky count, fat count.

## 4. From packet to triangles

`TieProc` uploads per packet: the setup positions (VU 0), the GS setups
(VU 1 + 5k), MSCAL 4, the unpack header and strips (V4-8), the vertices
(V4-16, VU 0x32), MSCAL 6 (place setups and strip tags, convert vertices),
the two colour-index copies (VU 0xCC and 0xF8). Then per instance: six
quadwords of transform and LOD data at VU 0xC6, the instance's 64 lit
colours (16 quadwords at 0x346 or 0x386), MSCAL 0.

Program 13507 writes setup k to its GS slot, each strip's GIF tag with NLOOP =
the strip's stored vertex count (EOP on the last), and each vertex's ST,
RGBAQ, XYZ to its slot, in stored order. Vertices in a "double" phase are
written to their second slot as well; this is how one stored vertex serves
two strips. A reader can rebuild the packet the same way: fill a slot map
(the last writer wins; 93 retail slots are written twice with the same data),
then walk from slot 0: a setup advances 6 quadwords, a strip reads its
vertices from slots cursor + 1 + 3i and advances 1 + 3 × count. Strips are GS
triangle strips. Wrench's reconstruction (sort by slot, drop duplicates,
walk) gives the same result on every retail packet.

## 5. LOD and morph

**Cull** (per instance, world units): skipped when the distance is 0, when
the sphere is beyond min(draw distance, 720), behind the near plane, outside
a side plane, or when its occlusion bit is clear
([OCCLUSION.md](OCCLUSION.md)). An instance not wholly inside the guard band
is drawn by the clipping program 224979 at the same LOD.

**LOD** from the depth of the sphere centre (no radius, no instance scale):

| Condition, in order | LOD | Morph factor k |
|---|---|---|
| depth ≤ near | 0 | 0 |
| depth > far | 2 | 0 |
| depth ≤ mid | 0 | (depth − near) / (mid − near) |
| otherwise | 1 | (depth − mid) / (far − mid) |

VU1 draws a fat vertex at `position + k × delta` and its colour as
`c0 × (1 − k) + ((c1 + c2) / 2) × k`, truncated; dinky vertices take
`palette[c0]`. ReRAC checked on Novalis that at k = 1 every LOD-0 fat vertex
lies on the LOD-1 surface and every LOD-1 fat vertex on the LOD-2 surface,
so the switches at mid and far are seamless. LOD 2 has no fat vertices.
Typical distance sets are 10/20/30 and 10/20/1024.

**Fog** for a tie is one value per instance, from the centre depth:
`clamp(cf24 + depth × cf20, If, In)` with the view context's fog constants
([CAMERA_FOG.md](../systems/CAMERA_FOG.md)).

## 6. Textures

Five A+D registers per setup, in the order TEX0, TEX1, MIPTBP1, CLAMP,
MIPTBP2 (not the terrain's order). The load-time conversion and the paging
follow terrain's rules: TEX0 from the class's texture slot, MXL = type − 1,
MMIN and K from TEX1, CLAMP from the disc fields, mip 1 after the base level
([TFRAG.md](TFRAG.md#5-textures-the-gs-setup)).

## 7. Instances and lighting

Instance record (0xE0, gameplay section 0x34): class number, draw distance
(**an s32** the loader converts to float; 720 on every Novalis tie), pad,
occlusion index, column-major matrix (W of the last column: 0.01 on 39,954
retail instances, 0.0 on 4,758), **64 RGBA5551 ambient colours, one per light
slot**, directional light selector (0–8, 15, once 0xFF00), uid, two pads.

The loader builds a 0x20-byte run-time record per instance (world sphere,
pointer to a 0x1C0-byte record, draw distance, occlusion index, class
index, relight flag, light selector, point-light nibbles) and a 0x1C0-byte
record (the matrix with each column's w = 1 / its length and the class scale,
64 lit colours, the 64 ambient colours). `LightTies` computes 64 colours per
instance from the slot normals, the ambient colours and up to three lights;
see [LIGHTING.md](../systems/LIGHTING.md#2-ties-lightties). Colours clamp at
243, not 255.

## Open

- RAC1: the unpack header's byte 2; the slot step table's consumer; the
  meaning of the 0.01 in the matrix; the reflection pass; the vertex path of
  224979.
- RAC2–RAC4: not measured in OpenRAC.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/tie_rac1.md`, `docs/plan/tie_lighting.md`;
PAL names from `games/rac1/config/overlays/us_map.tsv` and
[RENDERER.md](../../port/RENDERER.md).
