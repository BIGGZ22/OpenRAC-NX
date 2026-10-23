# Enhanced rendering

Probe GI, modern shadows, GTAO and edge-aware texture upscaling use an enhanced Vulkan build. Normal builds
and OpenGL use original lighting. Embree is needed only for baking.

Start with the [lighting guide](lighting/README.md) for builds, bakes and controls.

| Directory | Contents |
| --- | --- |
| `lighting/` | Probe data, sampling, emission and transport |
| `renderer/` | GTAO, cached texture upscaling and shaders |
| `viewer/` | Probe selection, early lighting and shadows |
| `game/` | Authored brightness tracking |
| `tools/probes/` | Offline baker and bake tools |
| `tests/`, `docs/` | Checks, design and experiments |

Shared integration and core Vulkan optimisations stay in `port/renderer/` and
`port/viewer/`. Editor bake inputs live in `editor/enhanced/`.

Texture upscaling and its 64 MiB cache live in `renderer/texture_upscale.*`.
The normal texture pool accepts an optional `UpscaledTextureColours` provider; it
keeps original alpha/mip sampling and notifies the provider when source textures
are retired. Only the enhanced texture library depends on the Vulkan copy helper.
