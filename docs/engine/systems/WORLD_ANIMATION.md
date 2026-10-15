# World animation: water, liquids, scrolling surfaces

What moves in a RAC1 level besides mobys. There is **no texture animation
in terrain, ties or shrubs**: their passes never read the frame counter.
Every animated surface (water, seas, lava, fire fields, reflective overlays)
is level code in a moby update that registers a per-frame **draw callback**.
ReRAC read these on the NTSC-U Novalis program (`SCUS_971.99`) and surveyed
the other levels. Sky rotation is in [SHRUB_SKY.md](../formats/SHRUB_SKY.md#8-clears-and-rotation-level-code),
shrub sway in [SHRUB_SKY.md](../formats/SHRUB_SKY.md#5-wind-sway), fog zones in
[CAMERA_FOG.md](CAMERA_FOG.md#5-fog-zones).

**Games.** RAC1. Not measured in the sequels.

## 1. Draw callbacks

`AddDrawCallback` (NTSC-U level 01 0x21AFE0, PAL `func_001F49B0`) registers
a function for this frame. The lists are cleared at the start of every tick
and drained by `ExecuteDrawCallbacks` (`func_001F4A00` and siblings) at fixed
points of the world render ([RENDER_PIPELINE.md](RENDER_PIPELINE.md#2-the-world-render-in-order)):
after the ties (levels 02, 05, 07, 09, 12, 14), after the mobys, after the
particles. On a frame with a catch-up tick only the last tick's registrations
are drawn, so a scroll advanced in the callback moves once per rendered frame.

Effect textures come from `GetEffectTex(n)` (`func_001F4868`): the core
index's effect textures ([TEXTURES.md](../formats/TEXTURES.md#5-particle-and-effect-textures)).

## 2. Static strip water (Novalis classes 676, 678, 761, 1225)

Each strip is one GS triangle strip from a 0x60-byte descriptor in the level
program's data: vertices (x, y, two z values), base texture coordinates, a
bounding sphere, two layer scroll speeds and directions, a wobble amplitude,
a vertex colour (alpha 0), two effect textures and two FIX blend factors.

- **Per draw** (`DrawStaticStrips`, level 01 0x2B96E0, PAL
  `func_L01_002BA898`): each layer's scroll advances and wraps at ±1; the
  strip is culled by sphere.
- **Per vertex**: a wobble angle from a hash of the position's float bits (only
  eight phase groups survive the mask), w = A · (sin θ, cos θ); layer 1 uses
  uv + s1 + w, layer 2 uv + s2 − w; z blends z0 and z1 with a per-class weight
  (761 bobs ±0.05 on a 64-tick sine).
- **GS**: drawn through the sprite VU1 program (57843, entries 0xC and 0xE),
  two passes (contexts 1 and 2, the second resending only ST), `C = Cd + (Cs −
  Cd) · FIX / 128`, repeat, bilinear. The vertex alpha of 0 always fails the
  alpha test: **colour blended, Z never written**.

## 3. The ripple module (levels 01, 05, 07, 11, 12, 13)

A shared engine module: a wave simulation on 16 × 16-unit patches.

| Role | NTSC-U level 01 | PAL |
|---|---|---|
| Init | 0x2B7A48 | `RipplePatchesInit` `func_L01_002B8C00` |
| Step one patch | 0x262038 | `RipplePatchStep` `func_L01_00262FE8` |
| Clock | 0x2B7FE0 | `RippleSimClock` `func_L01_002B9198` |
| Disturbance | 0x2B82A8 | `RippleDisturb` `func_L00_002A5158` |
| Height query | 0x2B8910 | `RippleHeightQuery` `func_0023B210` |
| Draw | 0x2B91C8 | `RippleDraw` `func_L01_002BA380` |
| Vertex shading | 0x261DF8 | `RippleVertex` `func_L01_00262DA8` |

- **Patch** (0x1190 bytes): centre (z = water level), eight neighbour links
  for edge exchange, two effect textures and FIX values, a 16-bit mask of
  active 4 × 4 sub-blocks, texture scroll and wobble, edge pin flags, three
  height buffers of 16 × 16 floats plus halos.
- **Activation**: on Novalis, while the camera is inside one of seven zone
  cuboids, that zone's patches are simulated and drawn, with random drops (1 in
  200 or 1 in 2,000 per tick); elsewhere patches follow their moby's visibility.
  The highest active zone picks the underwater colours.
- **Clock**: an accumulator starting at 1.1, +0.1 per tick, steps when ≥ 0.9.
  With the console's truncating adds this is **every 9 ticks** (8 under IEEE);
  between steps the draw lerps the last two states.
- **Step**: `n = 0.25 · Σ(eight neighbours) − previous`, damped by 15/16 when
  |n| ≥ 0.16; edges copied to the neighbours' halos.
- **Height query**: bilinear in an active patch, else a flat plane some levels
  set, else nothing; the hero, floats and the underwater test use it.
- **Draw**: per active sub-block, 46-vertex strips; per vertex a normal from
  the heights, a grey of `64 + 128 · clamp(3.25 · (L · n − 0.573) + 0.573, 0,
  1)` with L from light set 0, and a reflection coordinate `0.5 + 0.45 · R.xy`;
  pass 1 the water texture with scrolling wobbling coordinates, pass 2 the
  environment map. No Z write.
- **Disturbance**: cells within r of a point get amp (or amp / d); hero
  splashes, drips and random drops call it.

## 4. Seas and other liquids

- **Liquid grid module** (levels 03, 05, 07, 08, 09, 14): a grid of 7 × 7-cell
  blocks at a fixed z, culled by distance and frustum, drawn as 124-vertex strips
  through the sprite VU1 program, with a 64 × 64 texture blended between two
  effect frames texel by texel on the stored palette entries, and its own fog
  that fades the liquid to a colour with distance. One class per level owns the
  0x40-byte state record (the sea on level 05, lava on level 09, and others).
- **Camera-following ocean** (levels 11 and 16): a band of quads from behind
  the camera to ahead of it, two additive layers whose texture scrolls with the
  camera's motion.
- **Hoven's liquid** (level 12): static strips gated by a cuboid, two scrolling
  layers.
- The water Ratchet swims in is collision: surface-0 faces of the patch mobys
  ([COLLISION_QUERIES.md](COLLISION_QUERIES.md#4-surface-types)).

## 5. Other animated effects on Novalis

- **Fire and smoke fields** (class 760, four instances): camera-facing additive
  quads with a scrolling flame texture over a smoke curtain whose scroll is a
  global advanced by class 809.
- **Reflective overlay** (class 1848): five meshes with a sphere-mapped
  environment texture at a 25 % FIX blend, the map offset scrolling slowly.
- **Moby glow colours** pulsed by updates (the vendor, the ship): drawn by the
  moby glow list ([MOBY_RENDERING.md](MOBY_RENDERING.md#6-skinning-and-lighting-on-vu0)).
- **Point lights** moved by classes relight the world through the lighting
  passes ([LIGHTING.md](LIGHTING.md#5-point-lights-on-the-world)).

## Open

- Levels 02, 09 and 12's remaining liquid and lava meshes.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/world_animation.md`; PAL names from
`games/rac1/config/overlays/us_map.tsv`.
