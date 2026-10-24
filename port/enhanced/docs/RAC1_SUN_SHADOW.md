# Cascaded sun shadows

The live RAC1 frontend enables four reverse-Z sun maps on gameplay levels
0-18: one 2048x2048 cascade and three 1024x1024 cascades. Set
`OPENRAC_SUN_SHADOWS=0` to disable them. Sun maps work with or without probes;
Embree is never needed at runtime. Viewer callers use
`LevelScene::enable_sun_shadow` and `FrameInput::sun_pixel_shadow`; the standalone
viewer does not enable them yet.

## Projection and casters

Directional set 0 supplies the sun direction, matching the bake. Cascades cover
256 camera-depth units with practical splits (lambda 0.85), sphere fits rounded
to 1/16 unit and texel-snapped light-space XY. The last 10% of each interval
blends into the next; the final cascade fades to lit. Receiver fits overlap.

Terrain, ties, shrubs and submitted opaque mobys cast using visible transforms
and skin palettes. Alpha cutouts are respected; glow, metal, blended, additive
and callback effects do not cast. Static groups are culled against each map's
XY planes independently of the camera. Their bounds extend cascade depth for
off-camera casters, with 32 units of live-object padding. Unsubmitted mobys
cannot cast. Vulkan batches maps in a shared atlas; the framebuffer API still
allocates colour alongside sampleable depth.

A static shadow cache can keep the cascades' static casters between frames
(registry entry `shadow-static-cache`, off by default): a second depth target in
the atlas's layout, redrawn per cascade only when its key changes (the snapped
matrix, the caster volume, the hero setting, the static layers' generation),
copied into the atlas before the mobys are drawn over it. The caster volume
follows the camera, so only a still camera hits, and on both measured GPUs the
copy costs more than the draws it saves
([RENDER_PERF.md](../../../docs/port/RENDER_PERF.md), R-18).

## Receiving shadows

Tent filtering uses reverse-Z comparisons, receiver-plane depth correction,
normal offset and a 0.01-world-unit depth bias. The next cascade is sampled
only during transitions. Full shadow retains 10% of primary directional and
secondary fill light; point lights and probe GI stay independent. GI-only and
coverage views omit modern shadows. Without usable probes, visibility affects
authored prelighting, whose direct and ambient components cannot be separated.

[Ratchet's map](RAC1_HERO_SHADOW.md) is enabled alongside cascades. When ready,
it removes him and attached equipment from world casters. Both projections combine
visibility using `min`, including outside cascade coverage. Only the airborne overlap cue
adds up to 45% diffuse darkening inside cascade shade, including GI, while excluding the hero rig. Unreadable hero
state falls back to world casting. Stencil shadows are suppressed whenever
either modern route is active, including diagnostic views.

## Opaque static caster shader

`OPENRAC_SHADOW_SOLID` variants handle safe terrain/tie/shrub casters with only
position and transforms: no alpha textures, skinning, colour varyings or discard.
They preserve the original position expression and invariant output. Mobys,
including Ratchet, retain alpha-capable shaders and existing exclusion rules.

Eligibility requires non-cutout, non-blended, non-metal material alpha >= 1;
finite indexed vertex alpha >= 0.5; and finite instance tint alpha >= 1 with
no glow-selecting flags. Mixed runs split at eligibility changes. Live uploads
refresh instance eligibility. Coverage and triangles must stay unchanged.

Set `OPENRAC_SHADOW_SOLID_OFF` before startup for the reference path.
`sun solid draws` / `sun alpha draws` in `--debug-dump` count cascade shader use.
Byte-comparison fixtures cover four cascade depths, alpha holes/thresholds,
instance fading, cutouts and mixed runs; shader checks reject unused material,
texture and skinning uniforms. OpenGL/ES checks run where contexts are available.

## Unique-texel PCF filtering

Both filters combine three bilinear taps per axis into four texel weights.
For fractional texel coordinate f:

| Filter | Axis weights | 2D normalisation |
| --- | --- | ---: |
| Sun tent | `[1-f, 2-f, 1+f, f]` | 16 |
| Ratchet box | `[1-f, 1, 1, f]` | 9 |

This fetches 16 depths instead of 36 without changing the footprint. Outside-map
samples stay lit; projection, receiver correction, bias and cascade blending
are unchanged. Receiver-plane arithmetic runs once per texel. There is no
depth-tie fallback; reassociation can differ near floating-point equality.

Set `OPENRAC_SHADOW_FILTER_REFERENCE` before startup for the nine-bilinear-tap
reference filter. Selection changes shader/pipeline identity without a per-pixel
mode branch.
`sun_shadow_vulkan` and `sun_shadow_filter_reference_vulkan` cover physical
shadows, slopes, transitions and hero cases. The filter fixture sweeps
1x1 through 1024x1024 maps, edges, fractional positions, four slopes and depths
within 1e-7 of coplanarity. Agreement must be within 0.00002 visibility and one
RGBA8 step; biased coplanar receivers must stay lit. These are synthetic checks.

## Checks and limits

CPU/Vulkan fixtures cover splits, containment, reverse depth, texel stability,
filtering, sloped receivers, light floors, transitions, off-camera/alpha casters,
missing probes and stencil suppression. Shared GI, hero, shader and viewer
checks cover integration.

Range, resolution and bias are fixed, with no per-lightset maps. Shadows fade
out at 256 units. Alpha-blended and callback effect geometry do not cast.
Live casters are limited to objects submitted by the game. Inspect fine edges,
sloped receivers and moving characters when changing projections or filtering.
