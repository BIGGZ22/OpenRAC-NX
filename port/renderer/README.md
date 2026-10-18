# renderer

The port's GPU renderer, library `openrac_renderer`: OpenGL 4.1 core,
shaped after OpenGOAL's renderer
([docs/port/OPENGOAL_NOTES.md](../../docs/port/OPENGOAL_NOTES.md), sections 3
to 5) and built on what [docs/port/RENDERER.md](../../docs/port/RENDERER.md)
records about how Ratchet & Clank draws. **No emulation**: nothing here models
the console's GS, vector units, VIF or DMA. Each of the game's subsystems has
a renderer that draws its kind of thing on the GPU.

| File | What | OpenGOAL's |
|---|---|---|
| `renderer.h` | `Renderer` runs a list of `BucketRenderer`s each frame in the game's drawing order (sky, terrain, ties, shrubs, mobys, particles, HUD); `FrameInput` carries each bucket's packets and the camera | `OpenGLRenderer`, `BucketRenderer`, `SharedRenderState` |
| `direct.h` | the direct renderer: GIF packets (PACKED, REGLIST, IMAGE, A+D) of the 2D path turned into draw calls. `GifInterpreter` (CPU) assembles primitives and groups them by GPU state; `DirectRenderer` translates that state to GL: blend equation, depth test, alpha test with the double draw for fail modes, scissor, fog, texture | `DirectRenderer` |
| `texture_pool.h` | every texture as RGBA8 on the GPU, found by the base pointer in the game's TEX0 used as an **identifier**: textures converted ahead of time are `place`d there; images the game sends with IMAGE transfers are converted when first drawn and cached by content | `TexturePool` |
| `texture.h` | conversions to RGBA8: PSMCT32/24/16, PSMT8 and PSMT4 (and their "H" forms) with CLUTs in CSM1 order, TEXA alpha; the chip's block layout only for a texture uploaded in one format and read in another | `texture_conversion.h` |
| `gs.h` | the GS registers and the GIF tag as fields (public hardware documentation) | `common/dma/gs.h` |
| `subsystems.h` | placeholders for terrain (tfrag), ties, shrubs, mobys, sky and particles, each documenting what it must draw | `TFragment`, `Tie3`, `Shrub`, `Merc2`, `Sky`, `Sprite3` |
| `gl.h` | the GL functions used, loaded through the context's loader; no generated loader | glad |
| `shader.h`, `framebuffer.h`, `math.h` | GLSL programs (`#version 410 core`, samplers `tex_T<n>` bound to unit n), off-screen targets, column-major matrices with a reversed-depth projection | `Shader`, `FramebufferTexturePair` |

Shaders are in `shaders/` and compiled into the library
([cmake/Window.cmake](../cmake/Window.cmake)).

## Conventions (RENDERER.md, section 5)

- **Reversed depth**: the depth buffer is cleared to 0 and tested GEQUAL; the
  chip's Z, bigger nearer, goes straight to GL's window depth.
- **Colour 0x80 = 1.0** when modulating: the direct shader works in the chip's
  units (texel × vertex / 128, alpha 0x80 opaque) and writes alpha / 128.
- **No sample, no pixel**: the chip fills a pixel when its integer coordinates
  are inside the primitive and the scissor; a 2D primitive whose box holds no
  such point (between two rows, or slid just past the screen's edge) is left
  out, as the chip draws nothing of it. Drawn bigger than 512 x 448, GL would
  fill the window pixels between the samples (the air meter's line at the
  right edge after it slid away).
- **Alpha-test fail modes** (FB_ONLY, ZB_ONLY, RGB_ONLY) by drawing twice:
  passing pixels with every write, then failing ones with only what the mode
  keeps; ATST NEVER draws once with the fail mode's writes.
- **Textures** are converted once and identified by their base pointer, never
  by an address in a model of the chip's 4 MB memory.

## Placeholders and gaps

`world/` holds the world renderers converted from ReRAC: terrain (tfrag) and
ties, with the game's level of detail, culling and fog replayed on the CPU
(`tfrag_lod.h`, `tie_lod.h`, `fog.h`); `draw_data.h` also defines the inputs
of shrubs, mobys, sky, particles and water, which have no renderer yet. The
placeholders (`subsystems.h`) draw nothing yet: they need the game's
camera, visibility and lights, which come with the decompiled frame loop.
The level viewer ([viewer/](../viewer/README.md)) already draws the extracted
terrain, ties, shrubs, moby bind poses and sky.

The direct renderer covers the registers the 2D path uses. Not drawn
differently yet: region clamp and repeat (drawn as clamp), CSM2 CLUTs,
uploads into the middle of a buffer, local-to-local GS copies (ignored, as in
OpenGOAL), partial frame-buffer masks (a channel is masked only when all its
bits are), DATE, dithering. The hand-off from the game (which bytes reach
`FrameInput`, DESIGN.md section 4, item 3) is still open.

## Tests

`renderer_texture` (conversions, layout, pool), `renderer_gif` (the
interpreter) run anywhere; `renderer_direct` draws packets into a framebuffer
and checks pixels, through SDL's offscreen driver (Mesa's llvmpipe without a
GPU), and reports itself skipped when no OpenGL 4.1 context can be made.
