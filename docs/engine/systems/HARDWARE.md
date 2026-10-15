# What the game relies on in the console, and what a native port keeps

RAC1's code and data assume the console's arithmetic, graphics chip, timing
and libraries in ways a native port has to decide about one by one: keep the
result, or drop it. This page collects what ReRAC found on the NTSC-U disc
(`SCUS_971.99`) while writing a native port, with the PCSX2 comparisons it
made, as engine facts. OpenRAC's own choices for the port are in
[docs/port/DESIGN.md](../../port/DESIGN.md) (open decision 4, floating point)
and [PORTABILITY.md](../../port/PORTABILITY.md); [RENDERER.md, section 5](../../port/RENDERER.md#5-the-graphics-chips-conventions-the-shaders-reproduce)
lists the GS conventions measured on PAL, which agree with what follows.

**Games.** RAC1. The sequels run on the same hardware; which of these effects
they depend on is not measured.

## 1. Floating point

The EE's FPU and the VU0 and VU1 multiply-add units are not IEEE:

- every operation truncates (rounds toward zero);
- no denormals (exponent 0 is zero), no infinities or NaNs: exponent 255 is a
  finite number, overflow clamps to the largest magnitude;
- the adder drops the bits of the smaller operand more than one place below
  the larger operand's last bit before adding (the guard-bit rule PCSX2's
  soft-float work measured); a difference of exponents of 25 or more returns
  the larger operand;
- division and square root truncate; the multiplier occasionally returns one
  unit in the last place less than the truncated product (not modelled by
  ReRAC).

Where it shows:

| Effect | Cause |
|---|---|
| Colour bytes from lighting can differ by 1 | colours are carried as `65536 + c / 128`, so one last bit is one colour step ([LIGHTING.md](LIGHTING.md)) |
| Water ripples step every 9 ticks, not 8 | truncation in a phase accumulator |
| A path follower samples 20 points per spline segment, not 19 | `t += 0.05` reaches 0.99999946 on the 20th add under truncation, 1.0000001 after 19 under IEEE |
| An animation jumps to its catch frames in one tick (the Comet-Strike wrench) | a rate of `0x7F800000` is the largest finite value, not infinity, so the next advance steps straight onto the key |
| Collision hits and hero paths drift over hundreds of ticks | the query kernels and hero physics in VU0 macro code ([COLLISION_QUERIES.md](COLLISION_QUERIES.md)) |

ReRAC measured, against PCSX2 at the Novalis spawn: tie palette colours equal
on 96,511 of 96,512 entries (one off by 1); hero rotation entries within 2
units in the last place; crate heights within 1 unit in the last place; the
follow camera within 2.5e-4 units.

## 2. Trigonometry and random numbers

The game has several sine and angle routines, each with its own last bits:
the VU0 program 28259 polynomial (to x⁹ after folding the angle; used for
moby rotations), the two fast helpers `FastCos` and `FastSin` (PAL
`func_001F9F90`, `func_001F9FA8`; ReRAC lists the same two addresses as
sine then cosine), `FastArcSin` (`func_001F9FC0`), an arctangent with an
octant table (PAL `func_L00_001FF860`), libvu0's cosine-and-sine, and the
(cos, sin) table in the executable used for normals (296 of 512 values differ
from correctly rounded ones).

**Random numbers**: newlib's `rand` (NTSC-U 0x1160D8, PAL `func_001160D8`):
`s = s × 0x41C64E6D + 0x3039`, result `s & 0x7FFFFFFF`, seeded with
`srand(1234)` (`func_001160C8`) at level init. One stream is consumed in game
order: level load, then each tick the mobys in slot order, the hero,
particles, camera, and in the render the sky stars. Particle bursts, debris,
bolt scatter and star fields match the console only if every earlier draw
happened in the same order. 989snd on the IOP has its own generator, not
reversed.

## 3. The graphics chip

- **Colour units**: vertex and texture colours are 0x80 = 1.0; MODULATE is
  `(Ct × Cv) >> 7`, so vertex colours above 0x80 brighten up to twice.
- **Blending on display bytes**: the GS blends the frame buffer's stored
  bytes: `Cd + ((Cs − Cd) × As >> 7)` for ALPHA 0x44 and `Cd + (Cs × As >> 7)`
  for 0x48, clamped per draw. Dim additive puffs therefore stack into white
  cores; a port blending in linear light gets a different look. ReRAC blends
  effects (particles, translucent and additive mobys, draw-callback effects)
  in a display-byte copy of the frame to keep it.
- **Alpha test with keep-colour on fail**: TEST_1 0x5360B (alpha GEQUAL
  0x60, fail writes RGB only, Z GEQUAL) is the world's default; a texel below
  the reference still writes colour but not depth. OpenGOAL's double draw
  reproduces it. Other passes: shrubs 0x5320B and 0x530CB, billboards 0x53001,
  sky 0x30000 and 0x3180B, HUD 0x5380B.
- **The frame buffer's alpha is never displayed.** On ties in their morph
  range, VU1's blend of a fat vertex's four lanes can give alpha 0x7F instead
  of 0x80, which on the GS is an invisible 1/128 blend; a port that lets
  frame alpha through shows flickering see-through walls.
- **Fog** is a byte per vertex from the VU programs: `clamp(depth × slope +
  offset)`, blended RGB only, `FOGCOL + (C − FOGCOL) × F / 255`
  ([CAMERA_FOG.md](CAMERA_FOG.md)). Ties and shrubs have one fog value per
  instance.
- **Interpolation**: colour and fog are interpolated linearly in screen
  space; only texture coordinates are perspective-correct (ST and Q). The sky
  sends Q = 1, so its textures are affine. Sprites and the HUD are flat.
- **Mip level**: `round(log2(1/|Q|) + K)` clamped to the texture's levels,
  nearest mip, bilinear within it; with Q = n/z this is `round(log2(z/32) + K)`
  from the camera depth ([TFRAG.md](../formats/TFRAG.md#5-textures-the-gs-setup)).

## 4. Screen and projection

- Draw buffer 512 × 416 (NTSC; PAL 512 × 448), centre (2048, 2048) in 12.4
  fixed point.
- Projection: horizontal tangent 0.63, vertical × 0.775 (NTSC), near 32 and
  far 745,472 raw units; GS Z = depth / 2²⁴ in reverse order, tested GEQUAL.
  ReRAC's derivation is in [CAMERA_FOG.md](CAMERA_FOG.md).
- 2D corners are sent as `x × 16 + offset − 8`; particle and star sprites
  scale y by 1.0625.

## 5. Timing

- Game logic ticks at 60 Hz (the NTSC field rate is 59.94). The main loop
  allows **at most one** extra tick per rendered frame, when timer 1 counted
  more than 0x2580 (one field): at 30 fps the game runs two ticks per frame,
  below that it slows down.
- Some state lags a frame by design: the drawn fog is the previous camera
  update's, particle culls read the previous frame's camera, the sky clears
  only on the first frame of most levels.

## 6. Pad

`libpad2` returns 18 bytes per pad (active-low buttons, four axis bytes
centred on 127/128, twelve pressures). The game's own code then applies a
dead zone of |b − 127| < 48, scales by 1/76, takes the length and the angle,
and keeps a 30-entry history for flicks. The dead zone and scale are game
feel, not the library.

## 7. Game quirks that show

Behaviours of the game's code a faithful port keeps:

- Point lights attach to at most 0x200 instances, ties first
  ([LIGHTING.md](LIGHTING.md#5-point-lights-on-the-world)).
- The animation post-scale list drops some joints' scale
  ([MOBY_ANIMATION.md](MOBY_ANIMATION.md)).
- The particle allocator clears only the first 0x20 bytes of a 0x40-byte
  record, and some particle types read the stale rest
  ([PARTICLES.md](PARTICLES.md)).
- The shrub VU1 program fetches one vertex colour from the wrong address in
  six-vertex packets ([SHRUB_SKY.md](../formats/SHRUB_SKY.md#2-packets)).
- C's `%` on a negative selection reads the entry before a table (the quick
  select ring).
- A sound slot without an owner uses a leftover stack value as its velocity.
- Moby shadow probes repeat one probe and step circles by 6.28 / n
  ([SHADOWS.md](SHADOWS.md)).

## Open

- The multiplier's last-bit behaviour and the VU divider, which ReRAC assumes
  exact-then-truncated.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/hardware_fidelity_layers.md` (sections A, B,
D, "Result-level reproductions", "Tolerances", "Reproduced game quirks"),
`docs/plan/tfrag_lighting.md` section 6, `docs/plan/decisions.md`; PAL names
from `games/rac1/config/overlays/us_map.tsv`.
