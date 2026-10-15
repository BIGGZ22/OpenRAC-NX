# The native port: design

How OpenRAC gets from the decompiled games to games that run natively on a
PC. This page records what is decided, what is proposed and what is still
open. The evidence is in this folder: [DECOMP_STATUS.md](DECOMP_STATUS.md)
(what the decompilation holds), [RENDERER.md](RENDERER.md) (what the game
draws and how), [PORTABILITY.md](PORTABILITY.md) (what the C assumes about
the console), [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md) (the first survey of
the game), [VU_PROGRAMS.md](VU_PROGRAMS.md) and
[OPENGOAL_NOTES.md](OPENGOAL_NOTES.md) (how OpenGOAL did it).

Status words: **decided** (by the maintainers), **proposed** (the current
plan, open to change), **open** (needs a decision).

## 1. Decided

1. **A native port, the way OpenGOAL ported Jak and Daxter** (decided
   2026-10-09). The game's own code, decompiled, is compiled for the PC and
   runs as a normal program; a renderer of OpenRAC's own draws on the GPU;
   the platform services (disc, memory card, pads, sound, timing) are
   replaced at the game's library calls.
2. **No emulation** (decided 2026-10-09). OpenRAC does not interpret the
   console's processors, run the retail program, or model the GS, the vector
   units, the VIF or the DMA controller, to play a game or as a step towards
   playing one. The `runtime/` experiment, which did, was removed on
   2026-10-09.
3. **The player's disc is set up once, OpenGOAL's way** (decided
   2026-10-09): the launcher asks for the image of the player's own disc;
   OpenRAC's extractor copies its files out, checks them against the build
   it knows, then turns the assets into what the port reads (section 4).
   After that, the port reads extracted files and never the disc image.
4. **One port per game, one renderer for all where the engines agree.**
   The work lives at the top of OpenRAC, not in one game's tree.
5. **License**: GPL-3.0-or-later, as the rest of the top level
   ([LICENSE.md](../../LICENSE.md)). Nothing from the games (code bytes,
   data, microcode, media) is ever committed
   ([SOURCING.md](../policy/SOURCING.md)).
6. **First game: Ratchet & Clank (PAL)**, `games/rac1`, whose
   decompilation is the furthest along.
7. **Pointers are OpenGOAL's offsets** (decided 2026-10-09; open decision 1
   below). Game memory is one 4 GB reservation whose offsets are the
   console's addresses; a pointer in the game is a 32-bit offset into it.
   The decompiled C is written again for that model by
   [port/tools/hostgen](../../port/tools/hostgen/README.md), so the
   matching sources stay as they are and every 64-bit platform works,
   Apple Silicon included.
8. **The renderer starts on OpenGL 4.1 core, through SDL3** (decided
   2026-10-09; open decision 2), as OpenGOAL ships it everywhere. The
   renderer keeps the graphics API behind its own interface so that a
   second backend can follow.
9. **The port's own code is C++20, built with CMake, in
   [port/](../../port/README.md)** (decided 2026-10-09; open decision 5).
   The games stay the decompilation's C, compiled as C and linked in.

## 2. The shape of the port (proposed)

```
the player's disc ──► extractor ──► iso_data/   (the disc's files, checked)
                          │
                          └──────► assets       (levels, textures, models, sound,
                                                 in the port's own formats)
                                                     │
decompiled C (games/<game>/<version>/src) ──► the game, built for the PC
          + C written for the hand-written assembly     │
          + the data as C                               │ display lists or draw calls,
                                                        │ at the game's sync points
                platform layer ◄── library calls ───────┤
                (files, saves, pads, sound, time)       ▼
                                               native renderer (GPU):
                                               terrain, ties, shrubs, mobys,
                                               sky, particles, 2D, effects
```

- **The game.** The decompiled C of `games/<game>/<version>` is written
  again by hostgen for game memory (decided item 7) and compiled for the
  PC, with C written for the code that is hand-written assembly (the
  renderer cores' game side, collision, the vector helpers) and for the
  code not decompiled yet. hostgen's report lists, nearest first, what the
  program reaches that has no C. Every game version is built this way: a
  version is a directory in [port/game](../../port/game/README.md) (its
  hostgen configuration and the table of the libraries it calls), and the
  library replacements and the program are shared by all of them.
- **The platform layer** replaces the Sony and 989 libraries at their
  calls: disc reads become reads of the extracted files, the memory card
  becomes save files, the pad library reads SDL, 989snd is replaced at its
  API, `sceGsSyncV` and the DMA sends become the renderer's sync points.
  There is no model of the console's second processor; its services are
  answered on the EE side, as OpenGOAL did.
- **The renderer** is native per subsystem
  ([RENDERER.md](RENDERER.md), sections 6 and 7): geometry converted at
  extraction time and drawn from the camera, visibility, level-of-detail
  and lighting the game computes; a direct renderer for the 2D path; the
  maths of the VU programs, once understood, in shaders. It follows the
  conventions OpenGOAL measured (reversed depth, 0x80 = 1.0, the double
  draw for alpha-test fail modes, textures converted at load and identified
  by the game's own tables, not by the chip's memory addresses).
- **The hand-off** between game and renderer (proposed): the game keeps
  building its display list as it does today, and the renderer reads it at
  the sync points, one native renderer per kind of packet, the way
  OpenGOAL's renderer reads Jak's DMA chains. Where the game's side of a
  renderer is rewritten in C (the renderer cores are hand-written assembly
  today), it may hand the renderer a draw list instead (OpenGOAL's PC_PORT
  packets did the same).

## 3. The assets (proposed)

The extractor ([tools/extractor.py](../../tools/extractor.py)) works like
OpenGOAL's `extractor`, step by step:

| Step | OpenGOAL | OpenRAC |
|---|---|---|
| extract | copy every file out of the ISO 9660 image into `iso_data/<game>/`, hash them | the same, into `iso_data/<game>/` |
| validate | find the boot executable, hash it, look the serial and hash up in its database of known builds; refuse an unknown or damaged image with an error code | the same, against `games/<game>/game.json` (serial, size and SHA-1 of the boot executable); the same error codes |
| decompile | turn levels, textures, art and text into the port's formats (`decompiler_out/`, `out/<game>/fr3`) | planned: the editor's readers ([editor/](../../editor/README.md)) are the start |
| compile | build the game | the port is built from source today ([port/](../../port/README.md)); a player's copy will come built |

The launcher runs these steps for the player, as OpenGOAL's launcher does
([launcher/docs/INTEGRATION.md](../../launcher/docs/INTEGRATION.md)).

## 4. Open decisions

1. ~~The pointer model~~: decided, OpenGOAL's offsets (section 1, item 7).
2. ~~The graphics API~~: decided, OpenGL 4.1 core through SDL3 (item 8).
3. **The hand-off** (section 2): reading the game's display list, or draw
   lists from rewritten renderer cores, or the first and then the second.
4. **Floating point**: the console's arithmetic exactly, or IEEE with the
   cases the game relies on clamped ([PORTABILITY.md](PORTABILITY.md),
   section 2).
5. ~~Where the port's code lives~~: decided, [port/](../../port/README.md)
   (item 9).
6. **The asset formats** the extractor writes: OpenGOAL's own (`fr3`, a
   compressed level of converted geometry), glTF, or formats of OpenRAC's.

## 5. What the removed experiment leaves

`runtime/` (2026-10-08 to 2026-10-09) ran the retail program on a model of
the console. It is gone; git history keeps it (for example at `7954152`).
What it found about the games is kept here: the survey, the VU program
measurements, the OpenGOAL notes, and its findings about compiling the C
([PORTABILITY.md](PORTABILITY.md), section 4). Its C++ coding conventions
(`runtime/docs/CODING_CONVENTIONS.md` in history) are a starting point for
the port's own: the layout and comment rules carry over; the rules about
modelling hardware do not.
