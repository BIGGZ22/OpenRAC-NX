# scene

A level and its live mobys, ready to draw: the level the editor's port export wrote, loaded into
memory (meshes, textures, placements, the baked camera visibility), the poses and shadows of the
mobys a running game holds in its memory, the level drawn as one renderer bucket per layer, and a
frame written to a PNG. The viewer program (`viewer/`) and the game programs (`game/common`) draw
with it. It is not the renderer: the level is drawn as buckets of `renderer/`'s frame, with the
port's OpenGL calls (`renderer/gl.h`), which the Vulkan backend carries out in a Vulkan build. What
the renderers read of a running game besides the mobys (the camera, which mobys are live) is the
game state, which each game keeps in its own folder (rac1's `game/rac1/game_state.h`) and the game
programs read through the game's profile (`game/common/profile.h`).

Libraries `openrac_scene` (the CPU side: the level's loading, the poses, the shadow arithmetic,
the PNG; the probe tools link it without a graphics context) and `openrac_scene_renderer` (the
level renderer and its embedded shaders, `scene_shaders.h`), in the namespace `openrac::scene`. The
files came from `viewer/` (restructure packages P2-02 and P2-37); the viewer program's own code
(`viewer/camera.h`) keeps `openrac::viewer`. The enhanced renderer's probe and early lighting is
not the scene's: it registers with the scene's lighting hook (`level_lighting.h`, restructure
package P2-39b).

## Interface

| Header | Declares |
|---|---|
| [`level.h`](level.h) | `LevelData`, one level of the editor's export loaded whole on the CPU (one vertex and one index array, every texture as an RGBA8 image, the placements of each `Layer`); `load_level`, the sky's textures, `kManifestFormat`, the export format this port reads |
| [`pvs.h`](pvs.h) | `Pvs`, rac1's baked camera visibility (`pvs.json`, `pvs.bin`): the grid's lookup of which instances a camera position can see |
| [`level_renderer.h`](level_renderer.h) | `LevelScene`, a loaded level on the GPU (shared buffers, instanced draws per class and material, skinned live mobys, culling, the enhanced lighting and shadows when they are on), and `LayerRenderer`, one of its layers as a bucket of `renderer::Renderer` |
| [`level_lighting.h`](level_lighting.h) | `LevelLighting`, the level renderer's lighting hook: what a `LevelScene` asks of the probe and early lighting registered with `set_level_lighting` (the shaders' lighting GLSL, the bake's load, the uniforms, the early pass); the enhanced renderer's `lighting::SceneLighting` implements it |
| [`emission.h`](emission.h) | `EmissionMask` and `kAuthoredEmissions`: the reviewed materials that emit, which `load_level` applies to a level's materials and the probe baker reads |
| [`moby_pose.h`](moby_pose.h) | `moby_palette` and `moby_pose_matrices`: a live moby's joint matrices, evaluated from its animation fields in game memory as the game's VU0 evaluator did |
| [`sun_shadow.h`](sun_shadow.h) | `sun_cascades` and `SunCascades`: the sun's four cascades over the camera's view, their snapped matrices and caster volumes; `StaticShadowKey`, what the static shadow cache compares |
| [`hero_shadow.h`](hero_shadow.h) | `hero_shadow_projection`, the view Ratchet's shadow map is drawn from, and `hero_airborne`, read from game memory |
| [`shadow_atlas.h`](shadow_atlas.h) | the shadow atlas's size and its rectangles: the four cascades and the hero's map |
| [`moby_shadow.h`](moby_shadow.h) | `moby_shadow_state` and `moby_shadows`: a moby's stencil shadow (direction, ground slab, fade) and its capsules, from game memory |
| [`screenshot.h`](screenshot.h) | `write_png`: an RGBA frame written to a PNG file |

## Depends on

- `renderer` (math, textures, buckets, framebuffers, shaders, the GPU draw interface, shadow
  volumes), `renderer/world` (`gl_world.h`) and, in a Vulkan build, `renderer/vulkan` for the
  GPU-driven instance batches.
- `assets` (`assets/geometry`: the moby animation and shadow arithmetic, to the game's bit).
- `common` (the log, the statistics and their switches, the JSON reader).
- cgltf and stb, compiled once in `openrac_viewer_third_party` (`viewer/third_party.c`): the GLB
  meshes, the PNG textures, the screenshots.
- Nothing above it: the probe and early lighting reach it through `level_lighting.h`, registered by
  whoever makes a `LevelScene` (the game programs in `game/common/frontend/world.cpp`, the viewer,
  the tests, with `lighting::use_scene_lighting` of `enhanced/viewer/scene_lighting.h`). A scene
  with none registered lights as the game does: the mesh shader without the probe GLSL, as when
  enhanced rendering is unavailable, and no probe data, so neither the probes, the early lighting
  nor the shadows that need them are drawn. The shadow arithmetic the level renderer draws with
  (`sun_shadow.h`, `hero_shadow.h`, `shadow_atlas.h`) is the scene's own since P2-39a, and the
  emissions table its level data since P2-39b.

## State and threads

- A `LevelData` belongs to its caller; loading holds no state between calls.
- A `LevelScene` owns its GPU objects (buffers, textures in the caller's `renderer::TexturePool`,
  programs, and the lighting registered with it). It is created, uploaded, drawn and released on the
  thread that holds the graphics context, the program's main thread.
- The pose and shadow functions are pure: they read the game memory they are handed and return
  values. The game programs call them from the thread that draws.
- Between two calls of `set_instances` for the mobys a `LevelScene` keeps where each moby went
  (its group and place): a call with the same mobys in the same order writes their poses there
  instead of regrouping them (`moby-pose-in-place`). It also keeps a static shadow cache when
  `shadow-static-cache` is on, and the registered static instances of the GPU culling
  (`gpu-resident-instances`), all let go by `release`.

## Errors

- `load_level`, `LevelScene::upload` and the enhanced passes' switches (`enable_hero_shadow`,
  `enable_sun_shadow`) return `false` and say why in their `std::string& error`.
- `load_level` logs a warning and goes on for what it can do without: a texture or a sky panorama
  that does not decode, a placement of an unknown class.
- The pose and shadow functions return an empty result for a moby whose fields do not describe one
  (an address outside the memory, a class beyond the joint limit).
- `write_png` returns `false` when the file cannot be written.

## Configuration

The level renderer reads its profiling and reference switches through `stats::probe`
(`common/stats.h`), so the debug overlay lists them: `OPENRAC_PVS_OFF`,
`OPENRAC_CAMERA_INSTANCE_CULL_OFF`, `OPENRAC_MOBY_SPHERE_CULL_OFF`, `OPENRAC_SHADOW_SOLID_OFF`,
`OPENRAC_SHADOW_BATCH_OFF`, `OPENRAC_SHADOW_ATLAS_OFF`, `OPENRAC_OPAQUE_COLOUR_BATCH_OFF`,
`OPENRAC_SHADOW_LAYER_REFERENCE`, `OPENRAC_SUN_RECEIVER_CULL_OFF`, `OPENRAC_SUN_INSTANCE_CULL_OFF`
and `OPENRAC_SUN_TIMING_DETAIL`. [docs/port/DEBUGGING.md](../../docs/port/DEBUGGING.md) documents
each one. The enhanced lighting and shadows follow the `enhanced_rendering` setting
([enhanced/lighting/README.md](../enhanced/lighting/README.md)).

Its render optimisations are entries of the optimisation registry (`renderer/render_opt.h`,
switched off with `OPENRAC_RENDER_OPT_OFF`): `texture-prepare`, `gpu-resident-instances`,
`shadow-static-cache` (off by default), `moby-pose-in-place` and `shadow-atlas-discard`
([docs/port/RENDER_PERF.md](../../docs/port/RENDER_PERF.md)).

## Tests

| ctest | Proves |
|---|---|
| `viewer`, `viewer_es`, `viewer_vulkan` | a level written by the test loads (meshes, textures, placements, overrides, the baked visibility and its lookup), and draws on OpenGL, OpenGL ES and Vulkan |
| `renderer_shader`, `renderer_shader_es`, `renderer_shader_es_bare`, `renderer_shader_vulkan` | the level renderer's embedded shaders compile and link on each API, with the ES fallbacks |
| `probe_scene`, `bake_liquids` | the probe tools turn a `LevelData` into the scene they bake |
| `hero_shadow_cpu`, `sun_shadow_cpu` | the shadow arithmetic: the cascades' split, snapping and caster volumes, the static cache's key, the hero's projection and airborne flag (`tests/sun_shadow_test.cpp`, `tests/hero_shadow_test.cpp`) |
| `hero_shadow_vulkan`, `sun_shadow_vulkan`, `sun_shadow_filter_reference_vulkan`, `probe_gpu_vulkan`, `early_lighting_vulkan`, `gtao_vulkan`, `enhanced_rendering_compat` | the enhanced passes the level renderer draws, and that a build with them switched off draws as one without |
| `geometry_moby_animation` | the pose arithmetic `moby_pose.h` runs, against the game's own (in `assets/geometry`) |

`moby_pose.h` and `moby_shadow.h` have no test of their own: what they read of game memory is
checked by the maintainers' regression comparison of the game programs
([docs/port/REGRESSION.md](../../docs/port/REGRESSION.md)).

## Further reading

- [docs/port/RAC1_GPU_DRIVEN.md](../../docs/port/RAC1_GPU_DRIVEN.md): the Vulkan instance culling,
  batching and shadows of the level renderer.
- [docs/engine/formats/OCCLUSION.md](../../docs/engine/formats/OCCLUSION.md): rac1's baked camera
  visibility, which `pvs.h` reads.
- [viewer/README.md](../viewer/README.md): the export the level comes from and how the viewer draws
  it.
