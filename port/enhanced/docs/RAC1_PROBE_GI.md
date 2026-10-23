# RAC1 baked probe GI

See the [lighting guide](../lighting/README.md) for setup, controls and formats.

## Runtime lighting

Probe GI needs an enhanced Vulkan build and a valid level bake; otherwise
original lighting applies. It replaces ambient light on terrain and lit mobys,
including Ratchet, enemies and attachments:

```text
indirect = probe_irradiance / pi
diffuse = albedo * (direct_light + indirect * ambient_visibility)
surface = diffuse + emission + separate_specular_effects
output = existing_fog_and_output_conversion(surface)
```

GTAO mainly affects indirect light. Direct light, emission, metal effects, fog
and alpha remain separate. Camera-facing normals handle double-sided surfaces
but can change lighting when the camera crosses a surface's tangent plane.

## Offline bake

Embree traces terrain, ties, shrubs and selected static mobys using textures,
opacity, tint and shared runtime emission rules. Terrain vertex tint is kept
by default. Animated liquid emission is averaged; directly sampled emission
is not counted again on path hits.

Escaping rays default to 70% scaled skybox and 30% primary directional colour.
Static lights and emissive surfaces add diffuse bounces; runtime direct light
stays separate. Each volume traces the surrounding scene beyond its bounds.
Moving objects, destruction and doors do not update baked light or visibility.

## Probe placement

Bounce normals come from triangles and face the incoming ray. Inside tests
use ray crossings: at least 75% of 32 direction pairs must have odd counts in
both directions. Inside probes get up to four relocation attempts within 45%
of the smallest grid spacing; unusable probes become invalid.

Open surfaces, sealed single-shell rooms and intersecting shells remain
ambiguous. Correcting triangle winding alone cannot resolve them.

## Probe data and sampling

Probes store octahedral irradiance and distance-moment maps with filtering
borders, plus relocation and validity. Format 3 uploads packed texture planes
directly. Older formats and stale scene/settings hashes are rejected.

Sampling biases the receiver along its normal and view direction, then blends
eight neighbours using relocation, validity, position, surface direction and
visibility. Distance variance is clamped nonnegative; a receiver-plane guard
also limits leaks. Thin walls and sparse grids can still leak.

Irradiance is stored and texture-filtered as E^(1/5), raised to 2.5, blended as
sqrt(E), then squared back to E. Diffuse shading divides by pi once. CPU checks
use the same arithmetic.

## Sparse and dense volumes

A sparse grid covers the level. Dense regions follow authored lighting volumes
and whitelisted gameplay mobys, extending to nearby walls and ceilings. Large
regions split with overlap; planning reports record coverage limits. Halving
spacing on all axes costs about eight times as many probes.

Vulkan selects up to two resident regions per fragment using screen tiles and
depth. The CPU fallback uploads two camera-prioritised regions and fades changes.
Regions calculate visibility separately before blending light. Missing support
can use sparse GI; valid but blocked support stays dark. Boundary weights smooth
transitions, and weak visibility fades to black instead of a bright patch.

## Limits and troubleshooting

Green coverage means the sampler has usable support; it does not prove accurate
lighting or wall occlusion. Missing dense support may use sparse GI, but blocked
support must stay dark. Increasing density alone cannot fix invalid placement,
room/solid ambiguity or an incorrect visibility range.

Dense regions need to cover receiver surfaces, walls, ceilings and motion paths.
Enemy patrols and dynamic geometry can leave planned coverage. Check coverage
and GI-only views before adjusting brightness. Distance moments use a local
range so sky misses do not overwhelm nearby wall hits. Point lights are not
shadowed, and static baked emission remains after destruction.

A valid bake confirms scene/settings compatibility, not image quality. Rebake
sparse and dense volumes together after changing inputs or transport settings.

## Verification

CPU/GPU fixtures cover formats, placement, transport, sampling, region selection,
streaming and shadow/GTAO integration. Gameplay checks still need interiors,
doorways, thin walls, terrain LODs, skinning, attachments and emission. Compare
original, GI-only and combined views; record time, memory and coverage.

Exports and bakes stay local. PAL and NTSC need separate data and identities.

## References

- [Original lighting](../../../docs/engine/systems/LIGHTING.md) and
  [moby rendering](../../../docs/engine/systems/MOBY_RENDERING.md).
- [Terrain vertex shader](../../renderer/shaders/world/tfrag.vert),
  [fragment shader](../../renderer/shaders/world/tfrag.frag) and
  [point lights](../../renderer/shaders/world/world_lights.glsl).
- [Embree overview](https://github.com/RenderKit/embree/blob/master/doc/src/overview.md).
- NVIDIA DDGI [algorithms](https://github.com/NVIDIAGameWorks/RTXGI-DDGI/blob/main/docs/Algorithms.md)
  and [sampling](https://github.com/NVIDIAGameWorks/RTXGI-DDGI/blob/main/rtxgi-sdk/shaders/ddgi/Irradiance.hlsl).
