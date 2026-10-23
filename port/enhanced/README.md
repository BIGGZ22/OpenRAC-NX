# Enhanced rendering

Probe GI, modern shadows and GTAO use an enhanced Vulkan build. Normal builds
and OpenGL use original lighting. Embree is needed only for baking.

Start with the [lighting guide](lighting/README.md) for builds, bakes and controls.

| Directory | Contents |
| --- | --- |
| `lighting/` | Probe data, sampling, emission and transport |
| `renderer/` | GTAO and shaders |
| `viewer/` | Probe selection, early lighting and shadows |
| `game/` | Authored brightness tracking |
| `tools/probes/` | Offline baker and bake tools |
| `tests/`, `docs/` | Checks, design and experiments |

Shared integration and core Vulkan optimisations stay in `port/renderer/` and
`port/viewer/`. Editor bake inputs live in `editor/enhanced/`.
