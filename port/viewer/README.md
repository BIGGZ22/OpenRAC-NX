# viewer

`openrac-viewer`, the native level viewer (roadmap P3,
[docs/port/ROADMAP.md](../../docs/port/ROADMAP.md)): one level extracted from
your own disc, drawn on the GPU through the port's renderer, with a free
camera. No game code runs.

```sh
python3 editor/extract.py port baserom/SCES_509.16.iso build/port --level 0
openrac-viewer build/port/level_00
```

The editor's `port` export ([editor/port.py](../../editor/port.py)) writes
the Godot export's meshes and textures (`terrain.glb`, `ties/`, `shrubs/`,
`mobys/`, `textures/`) plus `sky.glb` (the sky's shells),
`placements.json` (ties, shrubs, mobys: class, 4 × 4 matrix in game axes,
stored fields) and `manifest.json` (format 1: files, classes with moby class
scales and names, bounds). Never commit or share what it writes.

Controls: W A S D move, Q and E down and up, Shift faster, the mouse wheel
sets the speed; hold the right mouse button to look. 1 to 5 show or hide sky,
terrain, ties, shrubs and mobys; L toggles lighting; F11 borderless
fullscreen; Escape quits. Options: `--size WxH`, `--camera X,Y,Z,YAW,PITCH`
(game units, degrees), `--fullscreen`, `--borderless`, `--no-vsync`,
`--no-lighting`.

Headless check: `--screenshot out.png --frames N` draws N frames in a hidden
window (SDL's offscreen driver when there is no display) and writes the last.

## How it draws

Each layer is a `BucketRenderer` of the port's `Renderer`, in the game's
order: the sky's shells around the camera (rotation only, blended in order
over the level's background colour, no depth), then terrain, ties, shrubs
and mobys, instanced per class, with reversed depth (cleared to 0, GEQUAL)
and in the game's axes (Z up). Textures go through the renderer's texture
pool. A moby is its class mesh in the bind pose, scaled by the class scale
times its placement; a class without a mesh is a box coloured by class. In
the native port the live mobys are posed: each frame `moby_pose.h` evaluates
a moby's joint palette from its animation fields in game memory (the two keys
it points at, the blend between them, its class's skeleton), as the game's
VU0 evaluator did, and the vertex shader skins the class mesh with it (the
exporter's per-vertex joints and weights, the palettes in a float texture).

## Placeholder

The lighting is a fixed sun only so shapes read; the game's lights (tie and
shrub light indices, moby colours, the level's directional lights), fog,
level of detail, animation in the viewer itself (it has no game
to read poses from), the runtime pose layers and joint modifiers (moby +0x60
and +0x64: Ratchet's head and blinks, look-ats) and the collision layer are
not drawn yet.

RAC1 baked camera visibility is supported on Vulkan when the export has
`pvs.json` and `pvs.bin`. The normal exporter writes them; existing exports
can add just this metadata with `python editor/occlusion.py DISC.iso LEVELS`.
The command verifies the disc/export identities and leaves meshes, placements
and GI bakes untouched. Other games and exports without PVS remain visible
subject to the existing frustum/distance tests.
