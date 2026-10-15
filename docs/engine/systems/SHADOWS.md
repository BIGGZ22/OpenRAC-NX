# Shadows

RAC1 has one dynamic shadow: selected characters (Ratchet, enemies,
critters, NPCs) cast a stencil shadow volume built from a proxy of capsules
and spheres on their joints, clipped to a thin slab around the ground below
them, and darkening what lies inside by 25 % with a hard edge. Everything
else (the shade under trees, roofs and overhangs) is baked into the terrain,
tie and shrub vertex colours ([LIGHTING.md](LIGHTING.md)); point lights cast
no shadows; there are no blob shadows. ReRAC read this from the NTSC-U code
(`SCUS_971.99`), checked the data on all 19 levels and the arithmetic against
PCSX2 memory. RENDERER.md's "Shadows `func_0020DEB0`, `shadowproc` asm" is this
system.

**Games.** RAC1. Not measured in the sequels.

## 1. Functions (NTSC-U and PAL)

ReRAC's suggested names, with the PAL addresses (none of them is named in
PAL's tables yet):

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Splice the shadow chain (from `DrawMobysCleanUp`) | 0x20D060 | `func_0020DEB0` |
| Direction 0, each tick | 0x20CFD0 | `func_0020DE20` |
| Key light direction of the hero's light set | 0x20D510 | `func_0020E360` |
| Build the posed list | 0x227740 | `func_00228A58` |
| Draw all shadows | 0x227548 | `func_00228860` |
| Set the direction | 0x226FB8 | `func_002282D0` |
| Sphere / capsule outline | 0x227D80 / 0x227ED0 | `func_00229098` / `func_002291E8` |
| Clip to the slab | 0x228520 | `func_00229838` |
| Project and classify faces | 0x227A08 | `func_00228D20` |
| Clear destination alpha | 0x2271D0 | `func_002284E8` |
| One pass / emit faces | 0x227140 / 0x228598 | `func_00228458` / `func_002298B0` |
| Resolve | 0x227378 | `func_00228690` |
| Ratchet's direction and slab | level 01 0x22A260 | `func_L00_002091D8` |
| Slab: around a height / probe down / probe along the direction | level 01 0x26EFF8 / 0x26F020 / 0x26F0E0 | `func_L01_0026FFC8` / `func_L00_0025B178` / `func_00214550` (`update_moby_shadow_range`) |

## 2. Which mobys cast

- A class casts when its header byte 0x0F (the shadow block size in
  quadwords) is non-zero; the block sits just before the skeleton.
  `InitMobyInstance` then sets mode 0x400, a range byte (+0x7F = 24), an empty
  slab (+0x84 = +0x88 = 0) and direction index 0 (+0xBD).
- **Shadow block**: records with a common header `u8 type, u8 last, u16 size`:
  - type 0, **sphere** (0x20): joint, segment count, offset (xyz in joint
    space, w = radius, model units);
  - type 1, **capsule** (0x30): joints A and B, segment counts A and B, two
    offsets with radii. Segment count 0 gives a flat end; a negative −k moves
    that end away from the other by k · 16 / 4096 of the segment and makes it
    flat.
- On the disc: 154 class entries (79 classes), 1,668 capsules and 45 spheres,
  at most 21 records (Ratchet). On Novalis: Ratchet, troopers, amoeboids,
  critters, NPCs and a few others. No crate, bolt, pickup or prop casts.
- A shadow is drawn when mode 0x400 is set, the slab is non-empty (+0x88 > 0,
  set each frame by the class's update), the shadow's sphere passes the view
  cull, and +0x7F ≠ 0. Mode 0x800 defers a moby without a shadow.

## 3. Direction and slab (game logic)

- **Direction 0** (everyone but Ratchet), each tick: `(0.14 cos φ, 0.14 sin φ,
  −0.99)`, φ the angle of the hero's key light: nearly straight down, leaning
  about 8°.
- **Direction 1** (Ratchet): `(cos p cos φ, cos p sin φ, −sin p)`, the pitch p
  moving at most 0.052 rad per tick towards 0.97 rad (56° below horizontal,
  clearly to the side) or 1.5 rad (under him) while in the air. Off while
  swimming on the surface, on the Hoverboard, and in two more state groups.
- **Slab** (+0x84 low, +0x88 high), by the class's choice:
  - around a given height, ±0.2;
  - a segment 16 units down from the moby, ±0.2 around the hit;
  - for scene actors, two probes along the direction (the second only reaches
    about 14 % of the way down, mixing two normalisations);
  - for Ratchet, four probes around his sphere (the fourth repeats the third,
    a data quirk) and his ground, water or liquid height: low = min − 0.12,
    high = min(max + 0.24, low + 3).
- **Size by distance**: full up to 16 units from the camera, shrinking to
  nothing at 24 (the range byte); the proxy shrinks towards the joints, it does
  not fade.

## 4. Geometry

1. In `DrawMobys`, `MobyProc` defers every moby with mode 0xC00 to a second
   pass after a placeholder tag, recording `{moby, size, guard flag}` for each
   shadow that survives the cull: **casters are drawn after the shadows**, so
   they are never darkened.
2. The posed list (8,064 bytes, about 7 Ratchet-sized casters) holds each
   record's points `P = position + R · ((J · p) · s / 1024 · size / 4096)` with
   joint matrices from the collision pose cache.
3. Per record: the outline perpendicular to the direction (an n-gon for a
   sphere, two half-arcs joined by tangents for a capsule; circles step by 6.28
   / n), each point paired with point + 1000 · direction, cut to the slab: a
   closed prism, the silhouette slid along the direction, as thick as the
   slab.
4. Projected by the EE (VU0 macro code, no VU1 program) and sent through VIF1
   DIRECT; side quads as strips, caps as fans, each face classified by the sign
   of its screen area.

## 5. The GS stencil in destination alpha

1. **Clear**: full-screen sprites leave every pixel's destination alpha at
   0x7F (alpha test NEVER, frame buffer only).
2. **Count**: FRAME_1 masks RGB so only alpha is written; TEX0_1 is **the frame
   buffer itself** (texture feedback); each face's ST is its own screen
   position, so it reads the destination alpha and MODULATEs it: front-class
   faces with vertex alpha 0x82 (0x7F → 0x80), back-class with 0x7F (0x80 →
   0x7F); Z GEQUAL, no Z write. For the counts that occur, bit 7 ends set
   exactly when the visible faces of one orientation outnumber the other: a
   z-pass stencil.
3. **Resolve**: FBMSK off, ALPHA_1 `(0 − Cd) · 0x20 / 128 + Cd` = 0.75 · Cd, and
   DATE so only pixels with alpha bit 7 pass: full-screen black sprites. Then
   the world state (TEST_1 0x5360B, ALPHA_1 0x44) is restored.

The result: a hard silhouette of rounded capsules that moves with the
skeleton, uniformly 25 % darker (overlaps do not darken more), wrapping the
ground and whatever stands inside the slab, never the casters.

**A native port**: a count target (+1 / −1 per face, depth tested against the
scene) and a multiply where the count is positive, with the casters drawn
afterwards; ReRAC found its multiply within 2 of the GS byte on 244 of 256
display bytes. Ordinary shadow maps would shadow everything, self-shadow, and
fight the baked shade.

## Open

- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/shadows.md` sections 0–5,
`docs/plan/hardware_fidelity_layers.md`; PAL addresses from
`games/rac1/config/overlays/us_map.tsv`.
