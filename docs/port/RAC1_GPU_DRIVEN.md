# Vulkan GPU rendering

Vulkan uses compute instance culling, shared geometry/materials and indirect
batching for terrain, ties, shrubs, mobys and modern shadows. OpenGL/ES keep
CPU visibility and direct draws. Sky, sprites, particles and stencil shadows
keep their existing renderers.

See the [lighting guide](../../port/enhanced/lighting/README.md) for setup,
[probe GI](../../port/enhanced/docs/RAC1_PROBE_GI.md) for light sampling and
[sun shadows](../../port/enhanced/docs/RAC1_SUN_SHADOW.md) for map generation.

## Submission and lifetime

The CPU owns scene traversal, pass order, materials, draw ranges and conservative
group bounds. Compute tests individual instances against PVS/frustum planes,
then compacts them with an ordered 64-lane scan. Transparent instance order is
preserved. Unknown bounds stay visible; camera UI bypasses world culling.
Shadow views use their own planes and never camera PVS. Terrain also retains
its CPU-cached PVS index compaction to preserve material runs. Hi-Z geometry
occlusion is not implemented.

A second dispatch builds indexed indirect arguments from immutable ranges and
visible-instance counts. Graphics consumes these and the compacted instance
stream without CPU visibility readback. Compatible adjacent commands merge
when pipeline, descriptors, uniform offset, geometry buffers and dynamic state
match. Render-pass boundaries remain. Unsupported multi-draw devices use
individual indirect calls; missing indirect first-instance support uses CPU draws.

Each frame slot owns immutable input pages until its fence retires. A page has
8 MiB of inputs, 32,768 120-byte instance slots, 4,096 group counts and 32,768
indirect commands. Growth adds pages instead of moving referenced storage.
Upload-command-buffer compute precedes graphics with shader-write dependencies
for indirect/vertex reads. Readback flushes start a new recording and invalidate
old batch handles. Pages are reused after retirement.

Static instances (terrain, ties, shrubs) are registered once at the level's
upload (`gpu_register_instances`): their bounds and instance data, in the
candidates' layout, live in one device-local buffer, and a view's batch of a
registered group writes only the view (58 words) and a group record whose first
word carries the resident bit; the cull shader reads that group's candidates
from a fourth storage binding. Each recording binds one resident copy; a change
of it waits for the next recording, and an old copy is destroyed only after the
frame slot that last bound it retires. The level's release forgets every
registration. The moby layer, regrouped each frame, keeps the payload copy
(registry entry `gpu-resident-instances`, [RENDER_PERF.md](RENDER_PERF.md), R-15).

## Shared materials

Bindless mesh, cutout-depth and alpha-shadow shaders use material records and
four-byte draw lookups. A push constant plus `gl_DrawIDARB` selects each record,
including across batch boundaries and zero-visible commands. Each 80-byte
record contains material values, texture indices and per-draw flags.
Global lighting, probes, palettes, shadow maps and GTAO remain in descriptor set
0; set 1 holds materials and texture/sampler descriptors. Opaque depth/shadow
shaders with no material reads stay small.

This needs draw parameters, nonuniform sampled-image indexing, partially bound
descriptors and sampled-image update-after-bind. Otherwise individual bindings
remain. Pages hold up to 1,024 image-view/sampler entries (subject to device
limits) and 32,768 lookups/records. Appends never change in-flight entries;
fences retire pages before reuse. See
[Khronos descriptor indexing](https://docs.vulkan.org/guide/latest/descriptor_arrays.html).

## Shadows and opaque colour

Shadow submission groups opaque, alpha-capable static and skinned commands,
removing model/layer shader alternation. Depth-only buckets may reorder because
blending, stencil and colour writes are off. CPU fallback consumes reusable
instance buffers immediately.

Four cascades and Ratchet's map share a 3072-square atlas, retaining their
2048/1024/512 resolutions. A per-draw view index selects projection and indexed
viewport/scissor via
[ViewportIndex](https://docs.vulkan.org/refpages/latest/refpages/source/ViewportIndex.html).
This needs material tables, multi-viewport and vertex viewport output
(Vulkan 1.2/SPIR-V 1.5); other devices keep separate targets. Filters stay local
to each rectangle, treating outside taps as lit. Detailed per-view timing also
uses separate targets so its intervals contain the corresponding draws.

Opaque colour batches across layers when GPU visibility/materials and reusable
prepass depth are available. It tests `EQUAL` with depth writes off. Runs stop
at translucent, glow, metal or effect work to preserve pass order. Visibility
is calculated once per layer. Otherwise original scheduling applies.
Blended receivers select from all resident probe regions because their surfaces
can lie outside the opaque depth bounds used to build tile masks.

Specialised static solid, static cutout, opaque moby and static translucent
programs reduce unused controls. Solid shaders have no discard; static shaders
compile out skinning. Shared run controls stay uniforms; fallback lighting stays
per draw. Alpha coverage and blend order are preserved. Dedicated depth shaders
handle the prepass.

## Counters and reference paths

Stats collection reduces GPU arguments to 16 bytes per page: visible instances,
nonempty draws and a 64-bit triangle count. Cached, persistently mapped staging
memory is read only after the existing fence; no extra wait/submission is added.
[VMA's readback pattern](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html)
guides allocation/invalidation. The reduction is independent code, informed by
[parallel reduction patterns](https://gpuopen.com/fidelityfx-parallel-sort/).
With stats off there is no reduction, copy or readback; allocations are lazy.

Completed counters can lag. Empty recordings publish zero; stale slots cannot
replace newer summaries, and stats off/on transitions reject old data. Immediate
scene counts are candidate upper bounds, not CPU-filtered geometry totals.

| Gauges | Meaning |
| --- | --- |
| `gpu driven mesh` | Active GPU instance path |
| `vk_draw_api_calls` | All native draw API calls |
| `vk_gpu_indirect_commands`, `calls`, `merged` | Logical arguments, native indirect calls and merges (all use the `vk_gpu_indirect_` prefix) |
| `vk_gpu_instance_candidates` | Instance/view pairs sent to compute |
| `vk_gpu_completed_recording`, `draws`, `triangles`, `visible_instances` | Last completed recording and its geometry (all use `vk_gpu_completed_`) |
| `vk_gpu_counter_readback_bytes` | 16 bytes per nonempty argument page |
| `vk_material_draws`, `records`, `textures` | Draw lookups, unique records and image/sampler entries (all use `vk_material_`) |
| `vk_gpu_batch_break_*` | Pipeline/resource/state incompatibilities; reasons overlap and omit pass/direct-draw/timing flushes |

These switches isolate rendering paths for comparison. Shader-changing switches
need startup or scene reload.

| Switch | Reference path |
| --- | --- |
| `OPENRAC_GPU_DRIVEN_OFF=1` | CPU visibility/direct meshes |
| `OPENRAC_GPU_MATERIALS_OFF=1` | Individual material bindings |
| `OPENRAC_SHADOW_BATCH_OFF=1` | Model-order shadows |
| `OPENRAC_SHADOW_ATLAS_OFF=1` | Separate shadow targets |
| `OPENRAC_SHADOW_LAYER_REFERENCE=1` | Per-layer shadow buckets |
| `OPENRAC_OPAQUE_COLOUR_BATCH_OFF=1` | Per-layer opaque colour |
| `OPENRAC_OPAQUE_SPECIALISE_OFF=1` | Combined/general colour shaders |
| `OPENRAC_VK_EFFECT_UPLOAD_REUSE_OFF=1` | Uncached effect uploads |
| `OPENRAC_VK_PARTICLE_TEXTURE_REUSE_OFF=1` | Per-particle texture resolution |
| `OPENRAC_SHADOW_GATHER_OFF=1` | Scalar shadow texel fetches |
| `OPENRAC_SUN_RECEIVER_CULL_OFF=1` | Square cascade culling |
| `OPENRAC_EARLY_LIGHTING_OFF=1` | Full-resolution fragment lighting |
| `OPENRAC_RENDER_OPT_OFF=gpu-resident-instances` | Static instances copied into each view's page every frame |
| `shadow-static-cache` (registry, off by default) | With it on: a sun cascade's static casters copied from a cache while its key is unchanged |

`OPENRAC_SUN_TIMING_DETAIL=1` enables per-layer shadow timings; normally the
four complete-map timers include clears.

## Work reduction

- Effects reuse identical texture uploads after checking shape, addressing and
  bytes. Upload sequence and overlapping asset-placement semantics remain.
- Particle extraction resolves each pixel/palette address pair once per pass.
  The cache dies after extraction, so later animation/address reuse is validated.
- Four shadow `textureGather` operations fetch the same 16 depths with unchanged
  comparisons, weights and accumulation; GL/ES retain scalar fetches. Component
  order follows the [GLSL specification](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html).
- Cascade culling extrudes the receiver's light-XY convex hull along the light
  direction. Blend overlap and three-texel + 0.05-unit padding preserve receiver
  coverage. Unknown/degenerate bounds fall back to square culling; off-camera
  casters remain eligible. Hero and GL/ES culling are unchanged.

### Early lighting

An 8x8 compute pass evaluates GI and cascade visibility at half width/height.
Opaque/cutout shaders reconstruct four samples using surface-plane and normal
agreement instead of repeating probe/shadow work. This is spatial filtering;
subpixel detail and shadow edges can soften. Translucent/effect and secondary
camera paths retain surface lighting. Cache identity includes camera, target,
viewport, frame and dimensions.

The cache uses four-byte RGBA8 lighting (`log2(1 + 16*E)/20` RGB, linear cascade
visibility A) plus an eight-byte surface guide: 12 bytes per half-resolution cell.
Hero-shadow visibility uses spare guide bits. GTAO's 3x3 filter is fused
into early lighting and attenuates GI before encoding; colour shaders do not
apply AO again. Albedo, GI strength, direct light and emission stay in materials.
The 10% directional/fill shadow floor remains.

### Compute GTAO and depth reuse

An 8x8 compute dispatch writes packed four-byte AO/source-pixel/normal records;
no AO framebuffer is needed. Odd dimensions round up and barriers protect
producer/consumer access. The unchanged-horizon shortcut uses an analytic
integral only when the complete search leaves both horizons open. It does not
skip searching or use temporal history. Performance quality uses three
silhouette-refinement steps; Quality uses seven. Search directions, depth steps,
radius and resolution are configurable in the Enhanced menu. See
[settings](SETTINGS.md) for defaults and ranges.

Enhanced single-sample targets expose their depth texture directly to compute
and separate visibility targets. GTAO borrows it without owning/deleting it.
MSAA, unsampleable targets and legacy colour-composite rendering retain copies
or resolves, avoiding framebuffer feedback. Existing Vulkan transitions return
depth to attachment layout before later depth-tested draws.

## Validation and troubleshooting

CPU/GPU fixtures compare complete framebuffer output, stable compaction,
PVS/frustum changes, transparency, animation, independent shadow visibility,
alpha coverage and descriptor rollover. Counter fixtures cover frame retirement,
stale recordings, stats transitions and direct-reference geometry totals.
Effect/particle checks cover overlapping transfers, palette animation and
address reuse. Run the relevant native checks after changing these systems.

When investigating rendering:

- Confirm the intended backend and feature path through the overlay gauges.
- Compare reference switches one at a time, with the same scene, camera, settings
  and compatible probe bake. Reload shaders when the switch requires it.
- Distinguish candidate geometry, completed geometry and native API calls.
  A lower call count does not necessarily lower frame time.
- Treat GPU timestamps as completion intervals that may include dependency
  waits. CPU and GPU work overlap; do not add their timings together.
- Check opaque/cutout coverage, transparent ordering, off-camera shadows and
  secondary cameras. Early lighting uses spatial reconstruction, so inspect
  thin surfaces, contact shading and moving edges as well as static output.

See [debugging](DEBUGGING.md) for frame dumps, GPU labels and profiling tools.

## Code map

| System | Main source |
| --- | --- |
| Scene passes, batching and geometry | [level_renderer.cpp](../../port/scene/level_renderer.cpp) |
| GPU instance pages and indirect commands | [gpu_draw.inc](../../port/renderer/vulkan/gpu_draw.inc) |
| Material tables and texture descriptors | [gpu_materials.inc](../../port/renderer/vulkan/gpu_materials.inc) |
| Vulkan state and submission | [vkgl_internal.h](../../port/renderer/vulkan/vkgl_internal.h), [vkgl_commands.cpp](../../port/renderer/vulkan/vkgl_commands.cpp) |
| Early lighting and surface reconstruction | [early_lighting.cpp](../../port/enhanced/viewer/early_lighting.cpp), [shaders](../../port/enhanced/viewer/shaders/early_lighting.comp) |
| GTAO evaluation and depth ownership | [gtao.cpp](../../port/enhanced/renderer/gtao.cpp), [compute shader](../../port/enhanced/renderer/shaders/gtao_evaluate.comp) |
