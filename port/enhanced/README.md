# Enhanced rendering

Probe GI, modern shadows, GTAO and edge-aware texture upscaling run through
Vulkan when enhanced rendering is enabled. With it off, or on OpenGL, the game
uses original lighting. Embree is needed only for offline baking.

Start with the [lighting guide](lighting/README.md) for builds, bakes and controls.

| Directory | Contents |
| --- | --- |
| `lighting/` | Probe data, sampling, emission and transport |
| `renderer/` | GTAO, cached texture upscaling and shaders |
| `viewer/` | Probe selection, early lighting and shadows, built with the level renderer into `openrac_scene_renderer`, in its namespace `openrac::scene` ([scene/README.md](../scene/README.md)) |
| `game/` | Authored brightness tracking |
| `tools/probes/` | Offline baker and bake tools |
| `tests/`, `docs/` | Tests and feature documentation |

GTAO (`renderer/gtao.h`) is the renderer's ambient occlusion hook
(`port/renderer/ambient_occlusion.h`): the game programs register it with the
renderer they create (`game/common/frontend/world.cpp`), and its settings type
is the renderer's (`port/renderer/gtao_settings.h`). Its source still builds
inside `openrac_renderer`, with its shaders.

Shared integration and core Vulkan optimisations stay in `port/renderer/` and
`port/scene/`. Editor bake inputs live in `editor/enhanced/`.

Texture upscaling and its 64 MiB cache live in `renderer/texture_upscale.*`.
The normal texture pool accepts an optional `UpscaledTextureColours` provider; it
keeps original alpha/mip sampling and notifies the provider when source textures
are retired. Only the enhanced texture library depends on the Vulkan copy helper.
