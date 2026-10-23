# The native renderer: what Ratchet & Clank draws, and how

The facts a native, OpenGOAL-style renderer for Ratchet & Clank (PAL first)
needs: how the game builds a frame, each thing it draws, the vector unit
programs that do the drawing on the console, and the conventions of the
console's graphics chip a GPU renderer has to reproduce in its shaders.

The renderer described here does **not** model the console. It does what
each of the game's renderers does, on the GPU, from the data the game
prepares and the assets extracted from the player's disc. That is how
OpenGOAL draws Jak and Daxter ([OPENGOAL_NOTES.md](OPENGOAL_NOTES.md)).

Sources: [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md) (sections 1 to 4 and 10),
[VU_PROGRAMS.md](VU_PROGRAMS.md), [OPENGOAL_NOTES.md](OPENGOAL_NOTES.md),
[ASSETS.md](../../games/rac1/docs/ASSETS.md), the editor's readers
([editor/](../../editor/README.md)) and the C in `games/rac1/src/game`
(`vuchain.c`, `draw.c`, `framebuf.c`, `tfragfunc.c`, `tiefunc.c`,
`shrubfunc.c`, `mobyfunc.c`, `skyfunc.c`, `lights.c`, `effects.c`). Names
come from `config/names.tsv` and `include/names.h`; addresses are PAL. A
statement marked **(inferred)** is a conclusion, not something the code
states.

## 1. A frame

- **Boot:** `main` `func_0012DB18` (C), then the boot stage `func_001E99D8`
  and the title loop `func_001EBB48` (neither decompiled). In a level, the
  loop is `func_L00_002465F8` (not decompiled, 2,248 bytes).
- **One iteration of the loop:**
  1. Read timer 1, the frame clock.
  2. `VU1_sendChain` `func_002349B8` sends the display list built in the
     previous iteration; `VU1_swapChain` `func_00234948` swaps buffers.
  3. The new list starts with `PutDrawBufferSmall` `func_001FB598` (draw
     environment at `D_0015EFB8+0xC0`), `AA_BlurPass` `func_001FB848`, then
     `PutDrawBufferLarge` `func_001FB498`.
  4. The mode is dispatched through a jump table. In play (mode 0),
     `func_L00_00299250` updates the world and `func_L00_001F91B0` draws it
     through `DrawWorld` `func_001F3D78` (not decompiled).
  5. Memory card, debug drawing, `VU1_syncChain(1)` `func_00234AC8`.
  6. Frame skipping, `sceGsSyncV` `func_00122598`, then
     `ResetGsRegistersPr` `func_001F3D00`, which writes the display
     registers every frame.

  So the console draws frame N−1 while the game builds frame N. No flip of
  the displayed buffer was found.
- **The display list** is a VIF1 DMA chain built by the C in `vuchain.c`:
  write pointer `D_00161000`, double buffers `D_00160FF8[2]`, size per level
  from `D_001DE338`. The tags used: CNT with GIF DIRECT for register writes
  (`VU1_addGSregister` `func_00234C98`, `VU1_setScissor` `func_00234D58`),
  REF for data and for vector unit programs (`VU1_addDataRef`
  `func_00234B48`), CNT with STCYCL and UNPACK V4-32
  (`write_vif_unpack_packet` `func_00234BA0`), bare VIF codes such as
  FLUSHA, interrupt fences (`Vif1ChainCmd` `func_00235290`), END, and
  **NEXT**: every texture-upload function and `DoGifPaging` `func_001F4748`
  reserves a tag and later splices a sub-list in with NEXT (the survey's
  tag table leaves NEXT out).
- **The world's drawing order** inside `DrawWorld`: setup
  (`build_rotation_view_proj` `func_001F2608`, `UpdateOcclusion`
  `func_001F2FB8`, `ResetGsRegisters`), sky, terrain, fences, ties, texture
  paging, draw callbacks, shrubs, mobys, particles, light quads
  `func_001F4C30`, lens flare `func_001EDFF8`, then 2D: HUD `func_001FFFB8`,
  help `func_001FF1B0`, subtitles `func_001F4F90`, letterbox
  `func_001F5148`, screen effects `func_001F5368`. Each pass is switched by
  a word of `D_0018A3B0[3..10]`.

## 2. What it draws

| Subsystem | Entry (C) and core | Core is | VU1 program | Data, and what already reads it |
|---|---|---|---|---|
| Terrain (tfrag) | `DrawTfrag` `func_002346C0`, `TfragProc` `func_002352C8`, `ComputeTfragTextureUsage` `func_00235EF0` | hand-written asm | 55907, and 903379 for the second strip list | 0x40-byte fragments with level-of-detail and refinement VIF streams; [editor/terrain.py](../../editor/terrain.py) (LOD 0 and 2) |
| Ties (instanced scenery) | `DrawTies_1/_2` `func_00236BE0/00236CA8`, `TieProc` `func_00236F00`, `BuildTieTextureDma` `func_002383D8` | asm | 13507, and 224979 | [editor/ties.py](../../editor/ties.py) (LOD 0, 0xE0-byte placements) |
| Shrubs | `DrawShrubs` `func_00229E50`, `ShrubProc` `func_00229F00`, `BuildShrubTextureDma` `func_0022B648` | asm | 56467, and 912339 | [editor/shrubs.py](../../editor/shrubs.py) (the billboard flag at class +0x1C is not used yet) |
| Mobys (characters and objects) | `DrawMobys` `func_0020E2B0`, `DrawMobysSetup` `func_0020E0C8`, `DrawMobyList` `func_0020E180`, MobyProc `func_00212658`; animation `MobyAnimProc` `func_00212578`, `moby_anim_eval_chain` `func_00211808` | asm | 13859, VU0 104691 | [editor/moby_class.py](../../editor/moby_class.py) (high-detail packets), [editor/moby_anim.py](../../editor/moby_anim.py) (skeletons, animations); low-detail, metal and glow packets not read yet |
| Sky | `SkyDrawShell` `func_0022C9A8` (C), `SkyDrawShellTextured/Gouraud` (not decompiled), `skyproc` asm | EE side | none for the shells; sprites reuse the particle program | [editor/sky.py](../../editor/sky.py) |
| Particles | `PartProc` `func_00218B10`, `UpdateParts` `func_00218A80` | asm | 221571 | records come from level code; not read yet |
| Sprites, billboards, light quads | `DrawSpriteHelper_A` `func_001F7868` (C); `drawquad` asm | mixed | 57843 | the setup packet is built in C (section 3) |
| Shadows | `func_0020DEB0`, `shadowproc` asm | asm | not established | not read yet |
| Lighting | `LightTfrags` `func_002362B0`, `LightTies` `func_00238688`, `LightShrubs` `func_0022B8F8` (VU0 macro asm); `UpdateAllPointLights` `func_00202260` (C) | asm | none (EE side) | tie ambient colours (+0x50) and lights (+0xD0) read as fields; baked vertex colours not read |
| Water and effects | level code, e.g. Novalis's ripples `func_L01_002BA380`; glows `func_001EE6E0`; lens flare (C) | level code | sprite program | not read yet |
| HUD, 2D, text | `HudSprite` `func_00200468`, `DrawTexturedQuad` `func_001F5800`, glyphs `func_001F69F0`, `FontPrint` (not decompiled) | almost all C | none: GIF packets sent DIRECT | HUD banks and images not extracted |
| Screen effects | `FadeToBlack` `func_001F4E08`, letterbox, Z clear `func_001FB908`, render to texture `func_001FB608`, pause background (`GrabFrameSnapshot` `func_L00_001F9D40`, a frame buffer read-back) | C | none | |

**Level of detail and visibility.** Terrain: `SetTfragDists`
`func_00234380` stores three distances and a matrix that turns distance into
two morph weights for the refinement streams. Ties: packet counts per level
of detail in the class, a draw distance per instance. Mobys: high- and
low-detail packet counts and a per-class distance table on the scratchpad.
Occlusion: `BuildOcclVisibility` `func_001F2BC8` and `ParseOcclGrid`
`func_001F2A38` make a 1,024-bit visibility mask per 4-unit camera cell,
read by the renderer cores. The port exporter preserves this data as
optional `pvs.json` / `pvs.bin` sidecars. The Vulkan native renderer applies
the mask to camera terrain, ties and mobys; GL and ES keep their existing
visibility path. Shadow maps retain independently culled caster lists.

## 3. The VU1 programs

RAC1 has nine VU1 programs and three VU0 images, all in the executable's
`vutext` and sent on demand by REF; `D_0015F704` records which one is
loaded. [VU_PROGRAMS.md](VU_PROGRAMS.md) names them from the executable's
own table.

What is known of their inputs:

- **Terrain:** the camera matrix unpacked to VU addresses 5 and 0x14D;
  vertices as level packets of STROW, STMOD, STCYCL and V3-16, V4-8, V4-16,
  V4-32 unpacks.
- **Mobys:** texture coordinates at 0xC2, an index stream at 0x12D, GS
  texture blocks after it, a vertex cache seven deep; bit 7 of an index
  means "do not draw yet".
- **Sprites** (`DrawSpriteHelper_A`): 12 quadwords unpacked to 0x3A4: two
  camera matrices, a GIF tag (triangle fan, Gouraud, textured, fogged,
  blended; registers ST, RGBAQ, XYZF2), screen scale and offset, fog
  parameters; then BASE 0, OFFSET 0x1D2, MSCALF 0.
- **Shrubs:** GIF registers ST, RGBAQ, XYZF2.

Ties, mobys and the first shrub program do nine tenths of VU1's work in the
first level; terrain a twentieth ([VU_PROGRAMS.md](VU_PROGRAMS.md)). The
terrain and shrub programs are byte-identical in Going Commando; ties,
mobys and particles were edited; Going Commando adds program 56883.

**What a native renderer must learn for each program:** its memory map and
double-buffering, its entry points (those of terrain, particles and most of
the moby program were never found), the projection, clipping, culling and
near-plane rules, how fog and depth are derived, terrain's level-of-detail
morph, how lighting is applied (and the environment map for chrome mobys and
the second tie program), the halving of negative texture coordinates that
Wrench applies, and the GIF output. None of this is documented in OpenRAC;
it is learned by reading each program's disassembly made locally from the
player's own disc, and only the findings are written down (never the
microcode, [SOURCING.md](../policy/SOURCING.md)).

## 4. The EE's vector code

The game does its vector maths in VU0 macro instructions and 128-bit MMI,
none of it compiled C (171 and 107 functions):

- `fastfunc`: FastVecAdd, Sub, Scale, Dot, Cross, Length, Dist
  (`func_001F9BD8` to `func_001F9D10`), `matrix_mul_vec3` `func_001F9EC0`,
  `sce_vu0_mul_matrix` `func_001FA540`, `euler_to_matrix` `func_001FA218`,
  FastCos and FastSin `func_001F9F90/9FA8` (a VU0 polynomial: their low
  bits differ from a C library's), FastArcSin, the packed-colour
  conversions and FastTweenColor (MMI), FastMemCopy, Zero16, Or16, MemSet.
- libvu0's rotation matrices `func_001254A0/00125548/001253F8`.
- Mobys: `moby_build_rotation` `func_0020ED48`,
  `moby_anim_sphere_lerp` `func_0020EEE8`, joint matrices on the
  scratchpad at 0x70000000 + 0x40 · i.
- Collision `func_001EE9F8`, `func_001EFE10`; the three lighting functions.

31 of these helpers were written in C during the removed `runtime/`
experiment, operation for operation in the console's order
([PORTABILITY.md](PORTABILITY.md), section 4).

## 5. The graphics chip's conventions the shaders reproduce

- **Screen:** PAL 512 × 448. Vertices in 12.4 fixed point around
  (2048, 2048); the viewport record `D_0013E600` holds (0x800 ± w/2) << 4.
- **Depth:** 24-bit Z. Mobys test GEQUAL and the Z clear `func_001FB908`
  writes Z = 0: OpenGOAL's reversed depth (clear to 0, compare greater or
  equal) applies as it is.
- **Tests seen:** mobys alpha test GEQUAL 0x60 with "keep colour on fail"
  (needs OpenGOAL's double draw); quads alpha NEVER with "frame buffer
  only" (colour without depth); menus without depth test.
- **Blending:** the default is (Cs − Cd) · As + Cd, restored after effects;
  colour and alpha 0x80 = 1.0; bilinear filtering; clamping; FBA set.
- **Fog:** the fog colour from the view context (`UpdateFog`
  `func_001F2930`, regions `func_001EE858`); the per-vertex fog value is
  computed by the VU programs from four context words.
- **Textures:** 8-bit indexed with 256-entry 32-bit palettes in CSM1 order
  (bits 3 and 4 of the index swapped), alpha 0 to 0x80; 16- and 32-bit
  16 × 16 palettes too; textures carry mipmap levels (the editor exports
  the base level only).
- **Texture paging decides the design.** Every pass resets the paging
  pointer and streams its textures into the same area of the chip's memory,
  and the texture address is patched into each GIF packet when it loads.
  So **an address in the chip's memory is not a stable texture identity**:
  a native renderer identifies textures by pass and texture-table index,
  converts them once when the level loads, and never models the chip's
  memory ([OPENGOAL_NOTES.md](OPENGOAL_NOTES.md), section 9, item 2).

## 6. OpenGOAL's approach, applied

- **The hand-off.** OpenGOAL's game builds its display lists as on the
  console and hands them to a renderer thread at the console's sync points.
  For RAC1 those points are `VU1_sendChain`, `VU1_syncChain` and
  `sceGsSyncV`.
- **One native renderer per kind of drawing**, not a model of the
  hardware:
  - a direct renderer for the GIF packets the 2D path sends (HUD, fades,
    letterbox, clears, glyphs), all built by C already;
  - geometry converted at extraction time (terrain, ties, shrubs, mobys with
    GPU skinning), from the editor's readers, drawn with the camera,
    visibility bits, level-of-detail weights, lights and joint matrices the
    game computes;
  - VU programs whose maths fits a vertex shader (sprites, particles)
    ported into one;
  - render-to-texture registered where the game samples it (the pause
    background, `func_001FB608` targets), and the display's fades.
- **No interpreter of the VU programs or the VIF.** A program that is not
  understood yet is studied until it is; the port does not run microcode.

## 7. Order of work

What can start now, without waiting for the decompilation:

1. **An asset pipeline**: the editor's readers (level, terrain, ties,
   shrubs, moby classes and animations, sky) turned into a converter that
   writes port-ready meshes and RGBA8 textures, as OpenGOAL's extractor
   does when it installs a game.
2. **A native level viewer**: a GPU renderer that draws an extracted level
   (terrain, ties, shrubs, static mobys, sky) with the conventions of
   section 5 and a free camera. No game code runs.
3. **The renderer cores read and documented**, in order of what they draw:
   TieProc, MobyProc, ShrubProc (their inputs: visibility, level of detail,
   light colours), the three lighting functions, then each VU1 program.
   Terrain and shrubs carry over to Going Commando unchanged.
4. **What is not read yet**: occlusion data, moby low-detail, metal and
   shadow packets, HUD banks and images.
5. **The 2D path**: its producers are C already; a direct renderer can be
   written against them.

What has to wait for the decompiled code:

- Drawing a live frame of the game: the level loop, `DrawWorld`, the title
  loop and loader (`func_001EBB48`, `func_001EABE8`), `UpdateViewContext`
  `func_001F3140`.
- Particles, shadows, water and glows, whose records come from level code
  that is not decompiled.
- The loaders still in assembly: `RelocateTfrags`, `tie_ad_gif_convert`,
  `shrub_class_init`, `MobyClassRelocate`, `BuildMobyAdGif`.
- The game's texture paging tables, `FontPrint`, the effect textures.

To check before relying on any of this: the level programs carry their own
copies of the renderers at other addresses; the display's double buffering
is inferred from the memory layout; the roles of the VU programs are
ReRAC's names, still to be confirmed against OpenRAC's own reading.
