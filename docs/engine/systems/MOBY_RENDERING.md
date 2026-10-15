# Drawing mobys: cull, LOD, skinning, metal and glow

How RAC1 draws a moby each frame: `MobyProc` culls it, picks a level of
detail, fades it, builds the VIF1 chain and a job record; after the chain is
built, the EE and VU0 skin and light every vertex and write the results into
holes in the chain; VU1 program 13859 then transforms, clips and draws. This
is ReRAC's reading of the NTSC-U code (`SCUS_971.99`) and of VU programs
104691 and 13859. The class format is in [MOBY.md](../formats/MOBY.md),
animation in [MOBY_ANIMATION.md](MOBY_ANIMATION.md), the lighting
arithmetic in [LIGHTING.md](LIGHTING.md#4-mobys).

**Games.** RAC1. VU0 program 104691 has one chunk of two unchanged in RAC2
and VU1 program 13859 none ([VU_PROGRAMS.md](../../port/VU_PROGRAMS.md)), so
RAC2's moby path differs in ways not measured. RAC3 and RAC4: not measured.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Draw entry | 0x20D460 | `DrawMobys` `func_0020E2B0` |
| Set-up: VU1 program 13859, VU0 program 104691, TEST_1 | 0x20D278 | `DrawMobysSetup` `func_0020E0C8` |
| Cull, LOD, fade, chain, job records | 0x211808 (level 01: 0x26A7A0) | MobyProc `func_00212658` |
| After the chain: textures, animation, skinning | 0x20D3B0 | `DrawMobysCleanUp` `func_0020E200` |
| Copies the trig table, runs the jobs | 0x20D1A8 | `ProcessMobyAnimData` `func_0020DFF8` |
| Job loop | 0x211728 | `MobyAnimProc` `func_00212578` |
| Joint palette | 0x20E0E0 | `func_0020EF30` |
| Skin and light driver for VU0 104691 | 0x1EE650 | `func_001EE9F8` |
| Glow recolour | 0x2116B8 | `func_00212508` |
| Instance init (defaults) | 0x20C5F0 | `InitMobyInstance` `func_0020D440` |
| Rotation rows and sphere | 0x20DEF8 | `moby_build_rotation` `func_0020ED48` |

[RENDERER.md](../../port/RENDERER.md) section 4 lists `func_001EE9F8` among
the collision helpers; ReRAC's reading makes it the moby skinning and
lighting driver.

## 2. Runtime fields MobyProc reads

| Offset | Use |
|---|---|
| 0x00 | bounding sphere: centre × 1024, radius (from the sequence's sphere, × scale, rotated) |
| 0x10 | position, world units |
| 0x20 | a dead byte; 0x36, 0x37 occlusion bits ([OCCLUSION.md](OCCLUSION.md)) |
| 0x23 | alpha byte, default 0x80 |
| 0x24 | class pointer |
| 0x2C | scale (class scale × instance scale) |
| 0x31 | set when drawn |
| 0x32 | s16 draw distance |
| 0x34 | u16 mode: 0x81 skip, 0x10 glow, 0x200 additive, 0x400 and 0x800 deferred and shadow |
| 0x38, 0x39, 0x3A | light sets 0 and 1, cross-fade |
| 0x3C–0x3E | ambient RGB (default 0x40) |
| 0x40 | Euler angles |
| 0x72 | LOD switch (class byte 0x0E) |
| 0x73 | metal distance: 0x18 when the class has metal packets |
| 0x90 | glow colour |
| 0xC0, 0xD0, 0xE0 | rotation rows |

**Rotation.** `moby_build_rotation` calls VU0 program 28259, which applies,
for each non-zero angle, x first, then y, then z: R = Rz · Ry · Rx,
right-handed. Row i is the image of model axis i. The sine is a polynomial
to x⁹ after folding the angle (valid for |a| ≤ 3π/2), so its low bits differ
from a C library's. Mode 0x8000 negates row 1; mode 0x100 keeps the old rows.
World position of a vertex: scale / 1024 · R · packed + position.

## 3. Per moby, in order

1. **Skip** on mode & 0x81 or a clear occlusion bit.
2. **Draw distance**: dd = min(+0x32, 500); culled when the sphere centre's
   view depth exceeds dd (the radius cancels).
3. **Near plane**: culled when the sphere is wholly in front of n = 32 raw
   units. **Side planes**: as for ties.
4. **Fade**: `fade = min(((dd × 1024 − r) − (z − r)) >> 7, 0x80)`, the last 16
   units of dd. The vertex alpha is `(fade × +0x23) >> 7`.
5. **GS state per moby**, sent as one A+D quadword before its first packet
   and restored after its last: a fading moby uses TEST_1 0x5308B (alpha
   reference 0x08 instead of 0x60); mode 0x200 uses ALPHA_1 0x48, additive
   (Cs · As + Cd). Explosion flashes, for example, are mobys with +0x23 below
   0x80 that fade to 0; their pixels below the alpha reference blend over the
   scene without writing Z.
6. **LOD**: low detail when clamp(z − r, 0, 0x30000) > +0x72 × 1024: the low
   packets and the low joint count (class byte 9; 0 means the bind pose). No
   hysteresis, no cross-fade.
7. **Metal**: shine = min((+0x73 × 1024 − (z − r)) >> 7, 0x80); none when ≤ 0
   (full within 8 units, gone at 24).
8. **Glow**: with mode 0x10, the packets from the class's first glow packet
   to the end of the list are recorded for recolouring (section 6).

The per-moby VU1 matrix is `M = V · [s · r0; s · r1; s · r2; 1024 · (p − camera)]`,
where V is the view-projection: VU1 maps skinned packed integers to clip
space with the camera position taken out.

## 4. The chain and the job

Per packet, MobyProc writes:

1. for the first two packets of a moby: STCYCL 4/4 and the matrix M
   (V4-32 to VU 0x129 or 0x28D, the two buffers);
2. REF to a hole for the positions (V3-16 signed at TOPS + 0) and NEXT to a
   hole for the colours (V4-8 at TOPS + 0x61): **the skinning driver fills
   these later**;
3. REF to the class's VIF list (texture coordinates at 0xC2, indices at 0x12D,
   GS setups);
4. MSCAL 0x0E when the sphere is inside the guard band (no clipping) or
   MSCAL 0x0A (with the clip test and clipper).

The chain is kicked next frame, so filling it after it is built is safe. The
chain preamble sets the ST q lane to 1.0 through STMASK and STCOL, BASE 0,
OFFSET 0x164, and uploads the six-quadword constant block of section 5.

The **job record** for each drawn moby holds the packet count, the animation
inputs (t, pose layers, frame pointers), the joint count, the metal word
(shine alpha and metal record size), the class hierarchy and skeleton
pointers, the three light rows, three light colours, the back factors, the
ambient (as `0x47800000 | byte`, i.e. 65536 + c / 128) and per packet the
vertex table address and the chain hole.

## 5. VU1 program 13859 constants

Uploaded per frame from the view context to VU 0x123–0x128 (buffer A) and
0x287–0x28C (buffer B):

| Quadword | Content |
|---|---|
| 0x123 | fog and w floor; an A+D GIF tag template |
| 0x124 | the perspective numerator (Q = it / w); the strip GIF tag: PRE, PRIM 0x7C (strip, Gouraud, textured, fogged, blended), NREG 3, ST RGBAQ XYZF2 (a fan variant for clipped polygons) |
| 0x125 | clip-test scale |
| 0x126 | guard-band scale (×4) |
| 0x128 | post-divide offset: 2048, 2048, 8388096.0, the fog offset |
| 0x129–0x12C | M, per moby |

Per-vertex input, index i: position at TOPS + i − 1, colour at 0x61 + i − 1,
ST at 0xC2 + i − 1, indices at 0x12D. GS state per moby: ALPHA_1 (Cs − Cd) ·
As + Cd, TEST_1 0x5360B (alpha GEQUAL 0x60 with keep-colour on fail, Z GEQUAL;
OpenGOAL's double draw reproduces it). The moby Z offset is 16 units below
terrain's.

## 6. Skinning and lighting on VU0

`ProcessMobyAnimData` copies the 256-entry (cos, sin) table to scratchpad
0x3800 and runs every job: the joint palette (F_j = P_j · S_j at scratchpad
0x70000000 + 0x40 · j, [MOBY_ANIMATION.md](MOBY_ANIMATION.md)), then the
driver for VU0 program 104691.

- **Scratchpad**: 0x0000 palette, 0x1C00 job constants, 0x1C80 packet list,
  0x2000 and 0x2800 vertex tables (double-buffered), 0x3000 and 0x3400 output,
  0x3800 trig table.
- **Per vertex**, the EE hands VU0 the record's halves, the trig values and
  the scheduled transfer matrix; VU0 (entry 0, a pipelined loop over the
  two-joint, three-joint and single vertices) blends the matrices, transforms
  the position (truncated to s16) and the normal (not normalised), lights it
  ([LIGHTING.md](LIGHTING.md#4-mobys)) and returns position and colour
  through a register handshake, two vertices behind.
- **Pack**: colour = min(255, (⌊128 c⌋ × multiplier) >> 7); positions are not
  saturated. Results also go to the 512-entry vertex cache; duplicates are
  copied from it after the loop and are **not** relit.
- **VU0 programs.** 104691 is uploaded every frame by `DrawMobysSetup`;
  436083 later in the frame by `DrawWorld` (it lights ties and shrubs); 28259's
  upper part once at boot by `InitOnce`. They never overlap in use.

**Metal pass** (the class has metal packets and the shine alpha is above 0):
after the moby's regular packets, the same chain gets the metal packets with
M's translation row + (0, 0, 1000, 0) (a pull towards the camera of 1000 · Q
in GS Z), always MSCAL 0x0A, no GS state change. VU0 entries 0x111, 0x121 and
0x12D (three, two, one joint, palette read directly) output the position,
a colour of `0x70 + Σ ⌊32 · C_k · max(L_k · n′, 0)⌋` (no back light, no
normalisation, no multiplier; alpha = the shine alpha) and a sphere-map
coordinate `st = (E · n′ + 1) / 2`, where E is the camera rotation times the
moby rotation, re-based towards the view direction to the moby. Chrome and
glass differ only in the map.

**Glow**: after skinning, for each recorded glow packet, the first n colour
words of its output are overwritten with the glow colour (+0x90) and the
vertex alpha: the packet draws unlit, in that colour. The list is rebuilt by
each `MobyProc` call and applied once, so in a batch of several moby lists
only the last list's glow packets are recoloured. On the disc 175 classes
have a glow colour (vendor lamps, eyes, a packet of Ratchet and of Clank).

## 7. A native renderer

What ReRAC's port does, as a model for OpenRAC's: resolve the VU0 slot
machine at load into per-vertex joints and weights; upload the palette per
moby per frame and skin in the vertex shader (at most 3 joints, 111
matrices), truncating the skinned position before the transform; compute the
light rows, colours and point-light merge on the CPU per moby; reproduce the
lighting formula in the shader (ReRAC measured 0 differing vertices out of
315,913 on Novalis against a bit-exact CPU pass); draw the metal pass as a
second draw with the sphere-map coordinate and the depth pull; pick the blend
per moby from its alpha and mode bits (alpha 0x80: opaque; fading: alpha
tested; +0x23 below 0x80: blended, depth tested, no depth write, sorted; mode
0x200: additive).

## Open

- The deferred and shadow path (mode 0x400 and 0x800): [SHADOWS.md](SHADOWS.md).
- The exact cull sign conventions; whether anything writes the colour
  multipliers at run time.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/moby_skinning_lighting.md`,
`docs/plan/moby_render_notes.md`, `docs/plan/moby_untextured.md`,
`docs/formats/moby_rac1.md`; PAL names from
`games/rac1/config/overlays/us_map.tsv` and
[RENDERER.md](../../port/RENDERER.md).
