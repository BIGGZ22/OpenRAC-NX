# Deko3D shader catalog (Switch)

Checked 2026-10-29 from a Switch Vulkan run with `OPENRAC_DUMP_SHADERS` (title, Novalis, vendor,
pause/map, cutscene) and from the renderer source.

## Every program the Switch game builds

Programs are built when renderers initialise or a level uploads, never lazily at draw time, so the
list follows from the code. The frontend (`game/common/frontend/world.cpp`) installs
DirectBackground/DirectRenderer (HUD and 2D), ShadowRenderer, scene LayerRenderer and two
EffectRenderers. The GS world renderers (tfrag/tie, `renderer/world`) are not installed.

| Program | Sources | Built by | In the dump |
|---|---|---|---|
| direct | renderer/shaders direct.vert, direct.frag | DirectRenderer | yes |
| effect | effect.vert, direct.frag + OPENRAC_UPSCALED_TEXTURE_COLOURS | EffectRenderer | yes |
| shadow volumes | shadow.vert, shadow.frag | ShadowRenderer | yes |
| shadow composite | shadow_composite.vert, shadow.frag | ShadowRenderer | yes |
| mesh | viewer mesh.vert, mesh.frag | LevelScene::upload | yes |
| mesh (solid) | mesh.vert, mesh.frag + OPENRAC_SOLID | LevelScene::upload | yes |
| sky | sky.vert, sky.frag | LevelScene::upload | yes |
| mesh depth | mesh_depth.vert/.frag | LevelScene::upload | yes |
| mesh depth solid | mesh_depth + OPENRAC_DEPTH_SOLID | LevelScene::upload | yes |
| GPU culling x3 | vkgl gpu_draw compute (MAKE_ARGUMENTS, SUMMARIZE, base) | Vulkan backend only | yes |

Only built when a feature is on (none are on in the Switch build):
- Bindless material variants (opaque colour batch, 3 colour-specialised, static translucent):
  `vkgl::gpu_materials_active()`; off on Switch by default (`OPENRAC_SWITCH_BINDLESS`).
- Enhanced rendering (early lighting, surface depth, sun/hero/atlas shadow depth, GTAO, probes,
  FSR, texture upscale): `OPENRAC_ENHANCED_RENDERING=OFF` in Switch builds.
- imgui (desktop debug tools, stubbed on Switch), touch overlay (Android).

Turning any of these on later adds programs; the runtime UAM fallback must cover that.

## Conversion check

`port/tools/deko3d/dk_check.py DUMP OUT --spirv-cross PATH --uam PATH`: each `.prog`'s SPIR-V ->
SPIRV-Cross GLSL 460 -> uam DKSH. Result 2026-10-29: 12/12 programs, 21/21 stages, no errors.

Observations for the backend:
- glslang's GL resolver gives the loose-uniform block binding 0 and samplers 1-3; storage
  buffers 0-3. All fit uam's per-stage limits (16 UBO, 32 samplers, 16 SSBO) unchanged.
- The mesh vertex stage samples textures (palette_texture, light_texture): vertex texture fetch.
- Vertex stages already end with `z = (z + w) / 2` (vkgl's vertex_source): use deko3d's default
  [0,1] depth; no DepthMinusOneToOne.
- Compiled only, not yet executed on deko3d.
