# Lighting

How RAC1 lights its geometry. There are no dynamic per-pixel lights: the EE
and VU0 compute vertex colours from a bank of directional light sets and a
bank of point lights, per vertex for terrain and mobys, per light slot for
ties, per normal cluster for shrubs. The sky is not lit. Everything here is
ReRAC's reading of the NTSC-U code (`SCUS_971.99`), checked by ReRAC on all
19 levels (every tfrag vertex, all 44,712 tie instances, every shrub
instance) against bit-exact ports; no emulator memory dump has confirmed the
exact bytes yet.

**Games.** RAC1. The VU0 program the tie and shrub passes call (436083) is
not the same in RAC2 ([VU_PROGRAMS.md](../../port/VU_PROGRAMS.md)); nothing
else is measured in the sequels.

## Functions (NTSC-U and PAL)

| Role | NTSC-U | PAL |
|---|---|---|
| Terrain | boot 0x234F98 | `LightTfrags` `func_002362B0` |
| Ties | boot 0x237370 | `LightTies` `func_00238688` |
| Shrubs | level 01 0x29E7E8 | `LightShrubs` `func_0022B8F8` |
| Mobys (with skinning) | boot 0x1EE650 | `func_001EE9F8` |
| Point-light slot write | level 01 0x2525F8, 0x252750 | `WritePointLight_A` `func_L00_0023EF78`, `WritePointLight_B` `func_L00_0023F0D0` |
| Point lights, once per tick | boot 0x201A28 | `UpdateAllPointLights` `func_00202260` |
| Attach a light to instances | boot 0x201BA8 | `CreatePointLight` `func_002023E0` |
| Detach | boot 0x201F88 | `DetachPointLight` `func_002027C0` |
| Detach and attach again | level 01 0x252DD8 | `RefreshPointLight` `func_00202790` |
| Free a slot | level 01 0x252850 | `FreePointLight` `func_L00_0023F1D0` |

## The banks

- **Directional sets**: 16 sets of 0x40 bytes (NTSC-U 0x180340 in level 01,
  0x19BDC0 in the boot): colour A (w = back factor), direction A (the way the
  light travels; a sun has z < 0), colour B, direction B. The level loader
  zeroes them and copies the gameplay file's light section (at most 12
  sets; [LEVEL.md](../formats/LEVEL.md#8-the-gameplay-file)), so sets 12–15 add
  nothing. Every retail back factor is ≤ 0.
- **Point lights**: 8 slots of 0x20 bytes after them: colour and intensity
  (w), position and radius.
- **Selector** (u16, per vertex or per instance): when bits 8–15 are 0, set
  `bits 0–3`; otherwise sets `bits 0–3` and `bits 4–7` blended by
  t = `bits 8–15` / 256, the blended directions renormalised.
- **Colour units**: a byte c is carried as the float with bits
  `0x47800000 + c` (65536 + c / 128), so one float step is 1/128 and adding a
  light of colour 1.0 at full strength adds 128 to the byte; MODULATE then
  treats 0x80 as 1.0. Results are the low byte of each lane.
- **Normals**: a 256-entry (cos, sin) table in the executable (NTSC-U boot
  0x165500; level 01 0x166500), copied to scratchpad 0x3800; 296 of its 512
  values differ from correctly rounded cos and sin, so a port reads the
  table rather than computing it. Index a gives angle 2πa / 256.
- **Leaky clamp**: every light's dot product d becomes max(d, d × w) with w ≤ 0,
  so a back face gets |w| × |d|.

## 1. Terrain (`LightTfrags`)

Runs at level load for every fragment and every frame for the visible ones
with point lights or a relight flag. It rebuilds each fragment's colour array
from scratch and DMAs it over the stored array, which is never shown.

Per vertex, from its light record ([TFRAG.md](../formats/TFRAG.md#7-colour-and-lighting)):

    base = expand 5:5:5:1 colour (c5 << 3; alpha bit → 0x80)
    pick set A, B (whole fragment if header 0x34 ≥ 0, else per vertex)
    N = (cos az cos el, sin az cos el, sin el);  n = −N
    dA = ((n.x·dirA.x + n.y·dirA.y) + n.z·dirA.z);  dA = max(dA, dA·wA)   (same for B)
    rgb = base + ⌊128·colA·dA⌋ + ⌊128·colB·dB⌋, clamped to 255; alpha = base alpha

Then the fragment's point lights, in nibble order (at most four), on the
already-lit bytes: for a vertex at P, v = P − L; skipped when |v| > r; d = n ·
v · (1 − |v| / r) / |v|; leaky clamp; `rgba += ⌊128 · (colour, w) · d⌋`, rgb
clamped at 255.

## 2. Ties (`LightTies`)

Per instance, 64 colours, one per light slot, from the class's 64 slot normals
and the instance's 64 RGBA5551 ambient colours ([TIE.md](../formats/TIE.md#7-instances-and-lighting)).
The EE turns each light direction into the instance's object space using the
unit columns of its matrix (`L = −Nᵀ · dir`), merges the point lights reaching
the instance's sphere centre into one third light, and calls VU0 program
436083 four slots at a time:

    n = slot normal (s16 / 32768)
    d_k = L_k · n;  f_k = max(d_k, d_k·w_k)          k = A, B, P
    c = ambient + colA·f_A + colB·f_B + colP·f_P
    rgb clamped at 243 (not 255); alpha = ambient bit 15 ? 0x80 : 0

**Merged point light** (ties and shrubs): for each listed light within its
radius of the centre, direction += unit (centre − L) and colour += colour × (1
− dist / r), back factor += w × (1 − dist / r); the direction is renormalised
only when two or more contribute.

VU1 colours a dinky vertex with its slot's colour and blends a fat vertex's
three slots with the morph factor ([TIE.md](../formats/TIE.md#5-lod-and-morph)).

## 3. Shrubs (`LightShrubs`)

Per instance, 24 colours, one per class normal ([SHRUB_SKY.md](../formats/SHRUB_SKY.md)).
The same arithmetic as ties, with these differences: the instance columns
are normalised with rsqrt; a blended set scales xyz only, so the blended back
factor is the sum; the ambient is the instance colour (three bytes, alpha
0x80); clamp at 243. After the load-time pass the loader stores the average
of the 24 colours, which the billboard sprite uses.

## 4. Mobys

In the same VU0 pass as skinning ([MOBY_RENDERING.md](MOBY_RENDERING.md#6-skinning-and-lighting-on-vu0)),
per vertex, two directional lights (one set, or two blended by the moby's
cross-fade byte) and one light merged from the point lights in range of the
moby's centre:

    L_k = −Rᵀ · dir_k                (model space, per moby)
    n = (cos a cos e, sin a cos e, sin e);  n′ = M_skin · n   (not normalised)
    f_k = max(L_k · n′, −|K_k| · (L_k · n′))
    c = ambient + (Σ C_k · f_k) · rsqrt(|n′|²)
    rgba = min(255, (⌊128 c⌋ × multiplier) >> 7)

The ambient is the moby's +0x3C–0x3E (the instance colour from the
gameplay file; default 0x40); alpha is the moby's vertex alpha. The merged
point light: weights a_j = 1 − δ_j / R_j for each light whose radius covers
the centre, colour Σ a_j · colour_j, direction the normalised weighted sum,
K = 0. Items on Ratchet copy his light sets, cross-fade and ambient every
frame.

## 5. Point lights on the world

Explosions and similar effects light the level as well as the mobys.

- **Writing a light**: the first free slot, and only while the frame load is
  at most 0.8.
- **Attaching**: once per tick after the mobys, a slot whose position has
  moved more than 8 units from where it was attached is attached again.
  `CreatePointLight` tests a sphere of radius + 8 against every tie, then every
  terrain fragment, then every shrub, and adds the slot to each hit's
  four-nibble list. The slot's list of instances holds 0x200 entries for all
  three kinds together, so a big light near many ties attaches no terrain or
  shrubs (the Novalis arrival crash lights 512 ties and no terrain).
- **Detaching** removes the nibble; an instance whose list becomes empty is
  flagged for one more relight, which restores its baked colours.
- Each relight starts again from the baked inputs, so what is on screen is
  always baked + the currently listed lights.

## 6. Arithmetic and fidelity

The passes are VU0 macro code: products and sums truncate, no NaN, infinity
or denormals, dot products as `(x + y) + z`. Because every contribution lands
on the 1/128 grid of 65536, the results are close to
`base + Σ ⌊128 · colour · d⌋`, and an IEEE port in the same operation order
differs by at most one byte in rare cases. ReRAC's notes on the float model
and on what a native port can drop are in [HARDWARE.md](HARDWARE.md).

## Open

- Ground truth: a memory dump of the colour arrays after level load.
- Which fragments, ties and shrubs a light attaches at run time against
  the 0x200 cap, in levels other than Novalis.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/tfrag_lighting.md`,
`docs/plan/tie_lighting.md`, `docs/plan/shrub_lighting.md`,
`docs/plan/moby_skinning_lighting.md`, `docs/plan/moby_render_notes.md`,
`docs/formats/tfrag_rac1.md` section 4.1; PAL names from
`games/rac1/config/overlays/us_map.tsv` and
[RENDERER.md](../../port/RENDERER.md).
