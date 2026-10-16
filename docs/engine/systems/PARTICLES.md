# Particles

RAC1's particle engine: a pool of 2,048 records of 0x40 bytes, 81 types each
with its own update function and spawner, a per-frame pass that culls,
depth-sorts and draws them through VU1 program 221571 as sprites, flat
quads, lines or ribbons. ReRAC read it from the NTSC-U code (`SCUS_971.99`);
95 of the engine's roughly 130 functions are identical in all 19 level
programs, and the boot executable carries only the allocator, `UpdateParts`
and `PartProc`.

**Games.** RAC1. VU1 program 221571 was edited for RAC2
([VU_PROGRAMS.md](../../port/VU_PROGRAMS.md)); nothing else is measured in the
sequels.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U | PAL |
|---|---|---|
| Allocate (stub to the real allocator) | boot 0x217A30 | `CreatePart` `func_00218928` |
| Free | level 01 0x27C5E8 | `KillPart` `func_L00_002688A8` |
| Per-tick dispatch | boot 0x217B88 | `UpdateParts` `func_00218A80` |
| Per-frame cull, sort, draw | boot 0x217C18 | `PartProc` `func_00218B10` |
| Queue a texture upload | boot 0x21880C | `_part_load_tex` `func_00219704` |
| Register the 81 update functions | level 01 0x27D4E0 | `RegisterPartTypes` `func_L00_002697A0` |
| Load particle textures and definitions | boot 0x2026C8 | `ParseParticleTexs` `func_00202F00` |
| Data-driven emitter (class 27) | level 01 0x2BD100 | `ParticleEmitterUpdate` `func_L01_002BE2C8` |
| Timer step | level 01 0x220EA8 | `FastDecTimer` `func_001F9938` |
| Tick scaling | level 01 0x220E30 | `scale_ticks` `func_001F9850` |
| Colour tween | level 01 0x2221A8 | `FastTweenColor` `func_001FA8A8` |
| Lens flare (not a particle) | level 01 0x20F480 | `LensFlareDraw` `func_001EDFF8` |

## 2. Pool and record

- **Pool**: 2,048 × 0x40 bytes, an allocation bitmap, a lowest-free hint, the
  highest index ever live and a count. Level init resets them but **does not
  clear the records**.
- **Allocate**: lowest free slot; clears only the first 0x20 bytes (one quadword
  at +0x00 and the same quadword three times at +0x10), so bytes 0x20–0x3F keep
  the previous occupant's data, which every spawner must overwrite (some types
  do not, and read stale values). Byte 0 = type.
- **Free**: byte 1 = 0x80, the bitmap bit cleared, the hint and highest index
  updated.

| Offset | Content |
|---|---|
| 0x00 | type, 0–80 |
| 0x01 | flags: 0x80 dead, 0x20 drawn last frame; bits 0–1 the render kind: 0 camera-facing sprite, 1 flat quad in world XY, 2 untextured line, 3 textured ribbon |
| 0x02 | particle texture index |
| 0x03 | ALPHA_1 low byte: 0x44 normal, 0x48 additive |
| 0x04 | colour 1 (0x80 = 1.0) |
| 0x08 | rotation, 256 steps per turn |
| 0x09 | near (low nibble / 4 units) and far (high nibble × 32 units) |
| 0x0A | s16, usually the life timer |
| 0x0C | size (kinds 0, 1; units of 1/210,000 world unit) or colour 2 (kinds 2, 3) |
| 0x10 | position (world units); +0x1C ribbon half-width at end 1 |
| 0x20 | velocity, or end 2 (kinds 2, 3); +0x2C ribbon width factor at end 2 |
| 0x30 | per type |

## 3. Types and simulation

- Each type has one update and usually one **spawner**, the only place that
  calls `CreatePart` for it; game code calls spawners, not the allocator. A
  particle follows nothing by itself: "attached" types store an offset from the
  hero position and re-add it, or keep a pointer to their owner's data, or are
  re-spawned by their owner. Each update frees its own record (timer, alpha,
  bounds).
- **Tick order**: the moby updates, the hero, then `UpdateParts`, which walks
  records 0 to the highest live index; a record created during the pass in a hole
  ahead of the cursor runs at once, one created above it waits a tick.
- **Time base** (`set_time_base`): speed 1.0 (PAL 1.2), timer scale 1.0 (PAL
  0.8333), dt 1/60 (PAL 1/50). `ticks(n) = (int)(n × scale + 0.5)`; gravity, where
  used, 9.8 × dt² per tick. So PAL runs the same particles at 50 Hz with scaled
  timers and speeds.
- **Type 6** is a data-driven emitter particle: everything (rate, life, size,
  spin, colour and alpha ramps with bounces between limits, gravity, a speed
  clamp, collision bounce, bounds) comes from the owning class-27 moby's 0xE0
  bytes of pvars. Novalis's ten class-27 emitters are the only particles at
  level start.
- **Randomness** comes from the shared newlib stream, in game order
  ([HARDWARE.md](HARDWARE.md#2-trigonometry-and-random-numbers)). Helpers (PAL):
  `random_integer_below` `func_002140B0` (`(rand() >> 16 & 0x7FFF) % n`),
  `random_float_between` `func_002140F8`, `random_angle_radians` `func_00214158`,
  and in the level programs `rand_range`, `randf_sym` and `rand_vec`.

ReRAC labelled about thirty types by what they draw (rain, sparks, smoke,
flashes, mist, flat ripples, the thruster ribbon of a flying enemy, and so on);
most labels are its own, not the game's.

## 4. Textures

From the core index ([TEXTURES.md](../formats/TEXTURES.md#5-particle-and-effect-textures)):
32 × 32 PSMT8 textures in the particle bank (not in `gs_ram`), and the particle
definitions: one frame list per type; each update knows its own frame count.
Textures are uploaded from EE memory every frame on demand, only for textures a
visible particle uses (TBW 1, PSMT8, TCC 1, MODULATE, CSM1).

## 5. Drawing (`PartProc`, VU1 program 221571)

Position in the frame: after the mobys, the first draw-callback list and the
lens flare; then CLAMP_1 = 5, **TEST_1 = 0x5380B** (alpha test ≥ 0x80, fail
writes RGB only, Z GEQUAL), ALPHA_1 = 0x44; the second draw-callback list
follows ([RENDER_PIPELINE.md](RENDER_PIPELINE.md#2-the-world-render-in-order)).

**Pass 1, cull**: camera-space position from the rotation-only view; kinds 0 and
1 culled outside their near and far (and the global far, 500 units, 64 units
underwater) and the frustum; lines and ribbons by their two ends. Survivors go
into 1,024 depth buckets of 0.25 units.

**Order**: buckets far to near (**back to front**); within a bucket, the higher
index first.

**Pass 2, packets**: a fade `m = min((far − z) >> 4, z − near, 0x1000)` scales the
alpha; then per kind:

| Kind | GS output |
|---|---|
| 0 sprite | triangle strip, textured, blended, flat, no fog; half-diagonal `406.35 × (size / 210000) / (z + 0.5)` pixels, rotated, y × 1.0625, all four corners at the centre's Z, affine texture (Q = 1) |
| 1 flat quad | the four world-XY corners projected one by one |
| 2 line | two projected points, Gouraud, one pixel |
| 3 ribbon | a strip across the segment, normal from the cross product, two colours |

Blend per record (0x44 or 0x48; lines 0x44). Depth and alpha are written only
where the modulated alpha reaches 0x80, colour always; no fog.

Sky stars use the same program and packet ([SHRUB_SKY.md](../formats/SHRUB_SKY.md#8-clears-and-rotation-level-code)).

## Open

- Most types' meaning; which spawners other levels call; the ribbon normal's
  sign.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/particles.md` sections 1–10 and "Update types";
PAL names from `games/rac1/config/overlays/us_map.tsv`.
