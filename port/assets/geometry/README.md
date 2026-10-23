# assets/geometry

What the renderer draws, read from a level's decompressed core data and
gameplay file: library `openrac_assets_geometry`, namespace
`openrac::assets::rac1` (the layouts are Ratchet & Clank's; the other games'
are not known yet). Converted from [ReRAC](https://github.com/re-rac/rerac)'s
`rc-formats` and its specs in `docs/formats` and `docs/plan` (ISC License,
Copyright (c) 2026 ReRAC contributors); each file names its source.

The readers resolve at load what the console's vector units did each frame
(vertex caches, skinning slots, strip and texture state, level-of-detail
links), so a renderer never replays that machinery. Where the game's
arithmetic decides a colour or a matrix to the last bit (lighting, the
animation palette), it runs on a model of the PS2's float unit
([common/ps2_float.h](../../common/ps2_float.h): truncating multiply and
add, the adder's pre-truncation), in the game's order.

| File | |
|---|---|
| `core_records.h`, `gs_adgif.h`, `mesh.h`, `vif.h` | core index tables (textures, classes, gadgets), GS A+D quadwords and register fields, a plain triangle mesh, VIF code lists |
| `texture.h`, `particle_textures.h` | the level's textures (PSMT8 with CSM1 palettes) and mip chains, billboard textures, particle and FX textures |
| `tfrag.h`, `tfrag_lighting.h` | terrain: fragments, strips, ad-gifs and their GS registers, texture paging spheres, the three levels of detail with their morph and collapse links; the lighting pass that bakes vertex colours |
| `tie.h`, `tie_lighting.h` | ties, the instanced scenery: classes, packets (dinky and fat vertices, strips, GS slots), instances; per-instance lighting of the 64 light slots |
| `shrub.h`, `shrub_lighting.h` | shrubs: classes, packets, billboards, fading, wind sway, instances; per-instance lighting and the VU1 colour-address quirk |
| `sky.h` | the sky: shells (textured and Gouraud), clusters, textures |
| `moby.h` | moby classes: packets of the high, low and metal levels of detail, the vertex cache, the VU0 skinning slots resolved into per-vertex joint weights, the index stream into triangles, the skeleton |
| `moby_animation.h` | sequences and keyframes; playback (advance, hard cut) and the evaluator that builds the joint palette, without the runtime layers the game's own code adds |
| `moby_lighting.h` | instance rotation rows and vertex lighting as the game computes them |
| `moby_collision.h`, `moby_shadow.h` | a class's collision primitives and mesh; its shadow proxy shapes |
| `gadget.h` | Ratchet's hand-held gadget classes (WAD streams in the core data), class textures, joint lists |
| `lighting.h` | what the lighting passes share: the light bank, point lights, the normal table (read from the player's boot executable), the instance light registers |

Addresses in the comments are NTSC-U (SCUS_971.99) unless they say PAL.
OpenRAC's Python readers in [editor/](../../../editor) (terrain, ties,
shrubs, sky, moby classes and animation) read the same formats from the PAL
disc for the editor.

## Tests

`tests/geometry/`: ReRAC's unit tests, converted, on synthetic data; where
ReRAC's tests used values of real classes, these use values of their own
with the expected results worked out by hand. ReRAC's tests that need the
extracted disc are not converted yet.
