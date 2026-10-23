# Ratchet's separate shadow map

[Sun cascades](RAC1_SUN_SHADOW.md) enable Ratchet's map automatically.
`OPENRAC_HERO_SHADOW=1` enables it independently. To restore stencil shadows,
set `OPENRAC_SUN_SHADOWS=0` and unset `OPENRAC_HERO_SHADOW`.

## Projection and casters

Grounded Ratchet casts along directional light set 0; airborne Ratchet casts
straight down in Z-up space. The unsigned `air_frames` halfword at Hero+0x30E
drives this choice through ascent, apex and falling. Hero+0x30A describes
ground-moby interactions and is not a grounded-state flag.
Unreadable state falls back to world cascades or the original stencil path.

The 512x512 reverse-Z map uses Ratchet's visible mesh, interpolated transform
and joint palette. It excludes glow, metal, additive passes, transparent texels
and other mobys. An instance tag also excludes Ratchet from world maps while
his separate map is ready. Attachments are not additional casters.

The orthographic region is eight units wide, with four units upstream and
72 units of light-space depth. Normal/depth biases limit self-shadowing;
out-of-map receivers stay lit. Filtering is described in the
[sun notes](RAC1_SUN_SHADOW.md#unique-texel-pcf-filtering).

## Receiving the shadow

Opaque early lighting stores eight-bit hero visibility in spare surface-guide
bits, keeping the cache at 12 bytes per half-resolution cell. Reconstruction
uses the same surface-aware weights as GI and cascades. Other draws sample
in the fragment shader; the caster map is always rasterised.

- Grounded visibility combines with cascades using `min`; both directional
  lights retain 10% in shadow. GI and point lights stay independent. An overlap
  cue adds up to 15% diffuse darkening, including GI, on other receivers.
- Airborne visibility is a platforming cue: at most 25% diffuse darkening,
  including GI. Ratchet and near-vertical receivers are excluded. Emission,
  alpha and fog are unchanged.

Ratchet, glow and metal do not receive the grounded overlap cue. GI-only and
coverage diagnostics omit modern shadows. Ties and shrubs use common fragment
lighting to separate direct light; authored-light fallback applies visibility
to existing prelighting. Stencil shadows are suppressed while either modern
route owns the shadow, and remain available if both fail to initialise.

## Checks and limits

CPU/Vulkan fixtures cover state reads, jump apex/landing, truncated buffers,
projection, reverse depth, skinning, alpha, caster exclusion and bounded cues.
Extent, range, bias and cue strength are fixed. Attachments do not cast through
this map. Check the whole jump arc and receivers already in cascade shadow
when changing projection or cue rules.
