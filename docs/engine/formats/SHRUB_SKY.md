# Shrubs and the sky

Two small formats: shrubs, instanced props with one mesh, an optional
billboard and a 24-colour lighting palette per instance; and the sky, up to
eight shells of indexed triangles the EE draws itself. Layouts are Wrench's
as ReRAC describes them; the draw rules are ReRAC's reading of the NTSC-U
code (`SCUS_971.99`) and VU1 programs 56467 and 912339. ReRAC checked them on
all 19 NTSC-U levels (551 shrub classes, 5,374 packets, 25,572 instances, 83
billboards; 75 sky shells, 1,700 clusters). OpenRAC's editor reads shrubs
and the sky on PAL ([ASSETS.md](../../../games/rac1/docs/ASSETS.md#shrubs)).

**Games.** RAC1, measured. Per Wrench (**reference**): the sky header is the
same in all four games; RAC1 and RAC2 shells have an 8-byte header with no
rotation, while RAC3 and Deadlocked add a starting rotation, an angular
velocity and a bloom flag. Shrub programs 56467 and 912339 are byte-identical
in RAC2 ([VU_PROGRAMS.md](../../port/VU_PROGRAMS.md)). Nothing else is
measured for the sequels.

## Functions (NTSC-U and PAL)

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Shrub class init: pointers, GS setups, paging distance | 0x203B08 | `shrub_class_init` `func_00204340` |
| Shrub cull, fade, billboards, sway, DMA chain | 0x228BE8 (level 01: 0x29CDF0) | `ShrubProc` `func_00229F00` |
| Shrub per-frame texture address patch | 0x228A30 | `PatchShrubGifs` `func_00229D48` |
| Shrub texture paging | 0x22A330 | `BuildShrubTextureDma` `func_0022B648` |
| Shrub lighting | level 01 0x29E7E8 | `LightShrubs` `func_0022B8F8` |
| Sky load: pointers, texture records | 0x2028E0 | `relocate_sky_definition` `func_00203118` |
| Sky per-level shell dispatch | level 01 0x252570 | `DrawSky` `func_L01_00252E80` |
| Sky shell, by flags | 0x22B690 | `SkyDrawShell` `func_0022C9A8` |
| Textured / Gouraud shell | 0x22B6E8 / 0x22B928 | `SkyDrawShellTextured` `func_0022CA00` / `SkyDrawShellGouraud` `func_0022CC40` |
| Sky vertex transform | 0x22BF94 | `sky_transform_vertices` `func_0022D2AC` |
| Sky GIF builders, textured / Gouraud | 0x22C208 / 0x22C0E0 | `func_0022D520` / `sky_build_gif_gouraud` `func_0022D3F8` |
| Cluster cull | 0x22C4C8 | `func_0022D7E0` |
| Star sprites | 0x22BBA0 | `SkySpriteProc` `func_0022CEB8` |

# Part 1. Shrubs

## 1. Class blob

All offsets relative to the blob. Header (0x40 bytes):

| Offset | Type | Field |
|---|---|---|
| 0x00 | 4 × f32 | bounding sphere, in units of `scale` world units (verified by the run-time centre computation) |
| 0x10 | f32 | texture paging distance (the init stores trunc(× 1024)); 5 on most classes |
| 0x14 | u16 | mode: bit 0 skips the class; `(mode & 6) >> 1` is the wind-sway mode (0 on 484 classes, 4 on 64, 2 on 3) |
| 0x16, 0x18 | | run time: instance count and list |
| 0x1C | s32 | billboard record, 0 if none |
| 0x20 | f32 | scale |
| 0x24 | s16 | class number |
| 0x26 | s16 | 0 everywhere |
| 0x28 | s16 | packet count |
| 0x2C | s32 | the 24-entry normal table |
| 0x34–0x38 | | run time: drawn, clipped and billboard counts |

Then packet entries (offset and byte size of each packet's VIF list), the
lists, the billboard record and 24 normals (s16 x, y, z, pad; ÷ 32768 as the
game reads them).

## 2. Packets

Each packet is one VIF list: STCYCL 4/4, STMOD 0, then exactly three
unpacks, FLG set:

1. V4-32 at 0: a header quadword (texture count, strip count, vertex count,
   VU address of the vertices), one GIF tag per strip, then four-quadword GS
   setups;
2. V4-16: per vertex s16 x, y, z and its GS packet slot;
3. V4-16: per vertex s, t (4.12), h (always 1.0), and the normal index (bits
   0–14, 0–23) with bit 15 the stop flag.

- **GIF tags**: NLOOP = strip length, PRE, PRIM 0x7C on every retail tag
  (triangle strip, Gouraud, textured, fogged, blended), PACKED, NREG 3, REGS
  ST, RGBAQ, XYZF2. The tag's fourth word is its slot.
- **GS setups** (four A+D quadwords): TEX1, CLAMP, MIPTBP1, TEX0, the first
  carrying its slot. On the disc TEX0's low word is the slot in the class's
  texture list; the class init rewrites all four by terrain's rule
  ([TFRAG.md](TFRAG.md#5-textures-the-gs-setup)). Retail: MMIN 4, K −142 to
  −110 (/16). The header's paging distance does not feed K; Wrench's
  formula for K is a fit.
- **Positions**: s16 × scale / 1024, then the instance matrix.
- **Slots**: tag 1 quadword, setup 5 (its own A+D tag and four entries),
  vertex 3. A packet with fewer than 6 vertices repeats its last vertex.

**What VU1 does** (program 56467, checked by ReRAC with an interpreter over
every retail packet): `ShrubProc` sends each packet's list into one of two
input buffers (0x76 quadwords, at VU 0x02 and 0x78) and calls it, then up to
five instances per batch, each 0x1C quadwords (four matrix columns, 24
palette colours), and MSCALF. The program copies tags and setups into two of
three 0xA8-quadword output buffers, then per instance writes every vertex's
ST·Q, the palette colour of its normal index, and screen XYZ with one fog
value per instance, and kicks. The stop flag is honoured from vertex 2 on and
three more vertices follow it, so a packet holds stop + 4 vertices. When the
stop flag is on vertex 2 (seven retail packets), vertex 3's colour is read
from the wrong address (another instance's data or an output buffer); only
the colour is affected. Program 912339 handles the instances clipped at the
guard band.

## 3. Instances

Instance record (0x70, gameplay section 0x3C): class, **f32** draw distance
(0 to 512, mostly 32), two zero words, column-major matrix (W of the last
column 0.01 on 25,367, 0.0 on 205), colour (three s32), zero, directional
light selector (0–8 or 15), zeros. No uid and no occlusion index: shrubs are
not occlusion-culled.

The loader builds per instance a 0x20-byte record (world sphere, draw
distance raised to at least 16 and, for billboard classes, to the fade
distance + 24, the fade distance as a byte, class index, relight flag, light
selector, point-light nibbles), a 0x40-byte matrix block (column 0 w = the
colour, column 1 w = the average lit palette colour, column 2 w = packed
column lengths, column 3 w = class scale) and a 0x60-byte palette of 24
colours. `LightShrubs` fills the palette:
[LIGHTING.md](../systems/LIGHTING.md#3-shrubs-lightshrubs).

## 4. Distance fade and billboards

With z the view depth of the sphere centre, D the run-time draw distance
(capped at 500) and F the billboard fade distance (`trunc(fade) & 0xFF`):

| Case | Mesh alpha | Billboard alpha |
|---|---|---|
| no billboard | min(16 × (D − z), 128): fades over the last 8 units | |
| z < F | 128 | |
| F ≤ z < F + 8 | 16 × (F + 8 − z) | 16 × (z − F) |
| z ≥ F + 8, or F = 0 | | min(8 × (D − z), 128): fades over the last 16 |

Alpha 128 goes to the opaque list, less to the translucent one (alpha test
reference 0x20 and 0x0C respectively). Instances clipped by the guard band are
never billboards.

**Billboard record** (0x40): fade distance, width, height, Z offset of the
bottom edge, then three A+D quadwords (TEX1, TEX0, MIPTBP1; register
addresses on the disc, data rewritten at load from the shrub class entry's
billboard texture). The sprite is not a VU1 packet: `ShrubProc` builds a
four-vertex strip (PRIM 0x7C) itself. It turns about world Z only: with d
the normalised direction from the eye to the instance origin, corner =
origin + y × W × (d.y, −d.x, 0) + (z × H + Zofs) × Z-up, for (y, z) in
(±½, 0 or 1), where W, H and Zofs are the record's sizes × class scale /
1024 × the instance's column lengths. Colour is the average lit palette
colour with the billboard alpha. Two passes: alpha ≥ 0x80 with alpha test
GEQUAL 0x60 writing Z, then the fading ones without Z write.

## 5. Wind sway

Classes with sway mode 1 or 2 shear the whole instance in world space in
proportion to height above its origin: columns (c.x + sx × c.z, c.y + sy × c.z,
c.z). Using a 256-entry table of −127 sin(2πi / 256), the frame counter T
and the instance's matrix block address A:

    g  = ((s(A·67 + T) · s((A·67 + T) >> 1)) · 0.2 + 0.8) · 0.1
    sx = (s(w + 64) · 0.02 + g) · k,   sy = s(w >> 1) · 0.02 · k
    w  = A·123 + T (mode 1) or A·123 + 2T (mode 2); mode 1 halves sx, sy
    k  = 1 − d² / 6000, none beyond d² ≥ 6000 from the camera

A lean of 6 to 10 % of the height towards +x with a slow gust and a ±2 %
wobble, fading out at about 77.5 units. The phase depends on the block's
address modulo 512, which is run-time heap state.

# Part 2. The sky

## 6. Block

A level may have no sky (core index +0x10 = 0). Header (0x40):

| Offset | Type | Field |
|---|---|---|
| 0x00 | 4 × u8 | colour; not used by the RAC1 shell code |
| 0x04 | s16 | clear flag: the loader sets it to 1, and the per-level dispatch zeroes it each frame on most levels (section 8) |
| 0x06 | s16 | shell count, at most 8 |
| 0x08 | s16 | sprite count: 0 on disc; set at run time for star levels |
| 0x0A | s16 | sprite capacity (0x20-byte run-time records) |
| 0x0C | s16 | texture count |
| 0x0E | s16 | effect texture count: the first textures, used by the star sprites |
| 0x10 | s32 | texture records |
| 0x14 | s32 | texture data base |
| 0x18 | s32 | effect index list |
| 0x1C | s32 | sprite area |
| 0x20 | 8 × s32 | shell offsets |

**Texture record** (16 bytes): palette offset, pixel offset (both from the
texture data base), width, height. 8-bit pixels and CLUT inside the sky block
itself, powers of two from 32 to 512, no mips. The loader rewrites each record
in place (a TEX0 cache, offsets >> 4, log2 sizes); textures are paged into GS
memory every frame.

**Shell** (RAC1: two s32): cluster count; flags, 0 textured and anything else
Gouraud. Cluster headers (0x20) start at shell + 0x10: bounding sphere, data
offset from the sky block, vertex count (at most 127), triangle count,
offsets of the vertices, texture coordinates and faces in the data, data size
(the game DMAs exactly that many quadwords to the scratchpad).

- **Vertex**: s16 x, y, z (used unscaled: only the direction matters), s16
  alpha (textured shells: the vertex alpha, 0x80 = 1).
- **Second array**, four bytes per vertex: textured shells, u16 s and t
  (/4096); **Gouraud shells, the vertex colour** R, G, B, A (Wrench reads it
  as texture coordinates; OpenRAC found the same on PAL).
- **Face**: three u8 indices and a texture (0xFF in Gouraud shells). The
  winding is the opposite of the other geometry; the GS does not cull.

## 7. How the sky is drawn

The sky has **no VU1 program**: the EE transforms the vertices with VU0
macro code and sends finished GIF packets through VIF1 DIRECT.

- **Order.** The frame render clears the screen (colour, Z = 0) only when
  there is no sky or the clear flag is set; then the sky, before terrain,
  ties, shrubs, mobys and particles. Shells in index order, clusters and faces
  in order.
- **Transform.** p = (x, y, z, 1) · SkyM · C, where C is the world
  view-projection built from the camera rotation only, so the sky is centred
  on the camera. XY uses the world projection's screen mapping; Z is sent as
  0 (the far end for a GEQUAL test), so all later geometry draws over it.
  Textured shells send Q = 1: affine texture mapping.
- **GS state per shell**:

  | | Gouraud | Textured |
  |---|---|---|
  | Z write | yes | no |
  | Test | none, Z ALWAYS | alpha GEQUAL 0x80 with "frame buffer only" on fail (inert), Z ALWAYS |
  | Blend | (Cs − Cd) · As + Cd | the same |
  | PRIM | 0x4B: triangles, Gouraud, blended, untextured, no fog | 0x5B: the same, textured |
  | Texture | | bilinear, clamped, no mips; RGBAQ = (0x80, 0x80, 0x80, vertex alpha) |

  After the sky: alpha test GEQUAL 0x60 with keep-colour, Z write on.
- **Culling**: whole clusters by bounding sphere, faces when all three
  vertices are outside the same plane.

## 8. Clears and rotation (level code)

- **Clear.** Because the dispatch zeroes the clear flag every frame, most
  levels clear only on the first frame; shell 0, a Gouraud dome writing Z = 0,
  is the colour and depth clear. Levels 05, 07, 10, 13, 14 and 15 keep the
  flag set and clear every frame; 05, 07, 10 and 15 have no Gouraud shell.
- **Rotation.** RAC1 data has none; per-level code gives chosen shells a
  rotation about Z-up, θ = (c mod P) × 2π / P − π, with c the 60 Hz game tick
  counter (frozen in pause):

  | Level | Rotating shells (period P in ticks) |
  |---|---|
  | 00 | s3: 0x8000 |
  | 01 | s2: 0x40000, s3: 0x20000, s4: 0x10000 |
  | 03, 04 | s1: 0x10000, s2: 0x20000 |
  | 05 | s1: 50,000 |
  | 07 | s2: 50,000 |
  | 08 | s1: 40,000 |
  | 09 | s3: Rz(θ, P 25,000) · Ry(0.3) |
  | 10 | s0: 2c, P 2^17; s1: 5c, P 2^17; s2: 10c, P 2^18; s3: 10c, P 2^17 |
  | 11, 12, 16 | s1: 0x40000, s2: 0x20000, s3: 0x10000 |
  | 14 | s1: 50,000, s2: 100,000 |
  | 18 | s4: 40,000, s5: 60,000 |

  The PAL names `DrawSkyShells_PAL/_NTSC` some projects used are wrong: the
  test that picks the animated or static loop is the level index.
- **Stars.** Levels 00, 02, 05, 06, 07, 13, 15 and 17 draw star sprites with
  the particle VU1 program between two shells (before s2 on 00, 02 and 13,
  before s1 on the others). They are generated with `rand()` on the first
  frame after load (a few hundred twinkling stars on a dome of radius 50 plus
  8 to 16 moving ones), get new random colour noise every frame, are blended
  additively, and are neither depth tested nor written.

## Open

- Shrubs: the 0.01 in the instance matrix; which instances go to 912339;
  the exact fog arithmetic of 56467.
- Sky: the sprite record layout in full; shell flags other than 0 and 1.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/shrub_sky_rac1.md`,
`docs/plan/shrub_lighting.md`, `docs/plan/sky_render_notes.md`; PAL names
from `games/rac1/config/overlays/us_map.tsv`.
