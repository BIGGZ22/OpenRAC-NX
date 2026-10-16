# Camera, projection, fog and underwater

How RAC1 turns its camera into the matrices and VU constants every renderer
uses, how the projection and depth map to the GS, how fog is computed (level
settings, fog zones, the underwater alternative) and what being underwater
changes. ReRAC's reading of the NTSC-U code (`SCUS_971.99`), checked by a
test that the port's projection, the VU pipeline and the game's own
world-to-screen function agree to under 1/1000 pixel.

**Games.** RAC1. Not measured in the sequels.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U | PAL |
|---|---|---|
| Defaults: near, far, field of view, screen, fog | boot 0x1F2C60 | `InitViewContext` `func_001F3008` |
| Projection, fog terms, all VU constant blocks; calls `SetTfragDists` | boot 0x1F2D98 | `UpdateViewContext` `func_001F3140` |
| Copies the level or underwater fog into the view context | boot 0x1F2588 | `UpdateFog` `func_001F2930` |
| View and view-projection matrices, guard-band and clip variants | boot 0x1F2260 | `build_rotation_view_proj` `func_001F2608` |
| One point, world to screen | boot 0x1F2070 | `projectWorldPoint` `func_001F2418` |
| FOGCOL into the GS | boot 0x1F3868 | `ResetGsRegisters` `func_001F3C10` |
| Render-to-texture view | boot 0x1F33B8 | `set_render_to_texture_view` `func_001F3760` |
| Usual camera update | boot 0x1ECCD8 | `camera_update` `func_001ED080` |
| Fog zones | boot 0x1EE4B0 (level 01: 0x2102B8) | `EnvFogUpdate` `func_001EE858` |
| Fog zone lookup | level 01 0x26BBC0 | `EnvZoneLookup` `func_L00_00257E18` |
| Hero light cross-fade in a zone | level 01 0x26BE04 | `HeroEnvLighting` `func_L00_0025805C` |
| Underwater test | level 01 0x20E9F0 | `update_camera_underwater_flag` `func_001EDB98` |
| Background colour into the clear packet | boot 0x1FB280 | `SetBackgroundColor` `func_001FB448` |

## 2. Camera and view

The camera record (NTSC-U 0x187080) holds the position (world units), Euler
angles and three rotation rows: **forward, left, up** in the right-handed
Z-up world. The view matrix gives camera x = −left · p (right), y = −up · p
(**down**, the GS screen direction), z = forward · p. Camera space is in raw
units (world × 1024): each renderer translates its integer positions by
−camera × 1024 itself.

## 3. Projection

| | Value |
|---|---|
| Near n, far f | 32 and 745,472 raw units (1/32 and 728 world units) |
| Horizontal tangent | 0.63 (64.4° field of view); cutscene cameras can change it |
| Vertical tangent | 0.63 × 0.775 (NTSC; PAL 0.756) = 0.48825 |
| Screen half sizes | 256 × 208 (a 512 × 416 draw buffer) |
| Z scale | −8,388,080.0 |

Per vertex, with z the camera depth: `Q = n / z`;
`X = 2048 + 256 · x / (0.63 z)`, `Y = 2048 + 208 · y / (0.48825 z)` (12.4
fixed point); `Z = 8388112 + Zs · ((f + n)/(f − n) − 2fn / ((f − n) z))`, from
16,776,192 at the near plane to 32 at the far: **reverse depth, tested
GEQUAL**, 24-bit. The projection's w column carries the **fog slope**, not 1
(section 4), so a level whose two fog intensities were equal would break Q;
none is. Beyond f, Z would wrap; distance culls keep geometry away. The guard
band is |x − 2048| ≤ 1024 and |y − 2048| ≤ 832 pixels.

The sky uses a variant with w = z / n and no translation
([SHRUB_SKY.md](../formats/SHRUB_SKY.md#7-how-the-sky-is-drawn)); the terrain
constant block the VU1 program reads is in [TFRAG.md](../formats/TFRAG.md#8-the-vu1-program-in-short)
and [VU1.md](VU1.md#terrain-55907-and-903379).

## 4. Fog

From the gameplay file's level settings ([LEVEL.md](../formats/LEVEL.md#81-level-settings-0x50-bytes)):
fog colour, near and far distance Dn, Df (raw units along the view axis),
near and far intensity In, If (GS fog values, 255 = no fog).

    cf10 = (If − In) · n / (Df − Dn)        (the projection's w column)
    cf14 = (In · Df − If · Dn) / (Df − Dn)
    F(z) = In + (If − In) · (z − Dn) / (Df − Dn), clamped to [If, In]

F is linear in camera **depth**, not distance. The GS fogs RGB after the
texture, alpha untouched: `C = FOGCOL + (C − FOGCOL) · F / 255`, F
interpolated in screen space. Novalis: fog colour (105, 127, 180), Dn 0, Df
240 world units, In 255, If 102. The level background colour goes into the
clear packet, which is only drawn when the sky does not clear
([SHRUB_SKY.md](../formats/SHRUB_SKY.md#8-clears-and-rotation-level-code)).

`UpdateFog` runs at the end of every frame render and copies the level fog
(or the underwater set) into the view context; `UpdateViewContext` then
recomputes the fog terms and the terrain morph constants, so fog changes
move terrain detail distances too. What is drawn lags the camera by one
update.

## 5. Fog zones

Gameplay section 0x80 ([LEVEL.md](../formats/LEVEL.md#85-grids-transitions-and-sample-points)):
a circle per record (x, y, r²), then 0x80-byte records (inverse matrix, two
hero colours, two hero light sets, flags, two fog colours, two sets of near
and far distance and density).

- **Lookup**: the first record whose circle contains the point (x, y); its
  inverse matrix gives l; if any |l| > 1, no zone (later records are not
  tried); else t = (l.x + 1) / 2.
- **Fog** (flag 2): with i = (int)(t × 255), colour `(c2 · i + c1 · (255 −
  i)) >> 8`; distances `(d1 · (1 − t) + d2 · t) × 1024`; intensities `255 −
  (k1 · (1 − t) + k2 · t) × 255`. Written to the level fog itself: **leaving a
  zone restores nothing**; records are built so their ends match the outdoor
  fog. The zones are small transition portals (cave and tunnel mouths, gas
  areas).
- **Hero light** (flag 1): the hero's colour and light-set cross-fade (moby
  +0x38) blend with t.

## 6. Underwater

- **Test**, each camera update: a segment ±0.75 around the camera, up to 6
  casts past non-water hits; on a water face (surface id 0), the flag is
  `camera z < water height + 0.04` (the height includes ripples). No hit leaves
  the flag unchanged. Cleared in some camera modes and states.
- **While set**: the alternate fog (per-zone colour, near 0, far 32 units,
  far intensity 48); the particle far distance drops from 500 to 64 units; a
  full-screen tint `Cd + (Cs − Cd) · A / 128` drawn **after the HUD** (so the
  HUD is tinted; 50 % or 37.5 % by water zone); sound pitch and volume
  changes ([AUDIO.md](AUDIO.md#4-per-frame-sound_update)). The sky is not
  fogged and not changed except by the tint.

## Open

- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/game_camera_fog.md`,
`docs/plan/world_animation.md` sections 5–6, `docs/plan/sky_render_notes.md`
section 5; PAL names from `games/rac1/config/overlays/us_map.tsv`.
