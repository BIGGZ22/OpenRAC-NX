# The native port

OpenRAC's goal for playing the games is a **native PC port**, built the way
[OpenGOAL](https://github.com/open-goal/jak-project) ported Jak and Daxter:

- the game's own code, decompiled to C, compiled for the PC and run as a
  normal program;
- a renderer of OpenRAC's own that draws what the game asks for on a modern
  GPU, written per renderer (terrain, ties, shrubs, mobys, sky, particles,
  2D), not by modelling the PlayStation 2's graphics hardware;
- the game's assets (levels, textures, models, sound) extracted once from the
  player's own disc into formats the port reads.

**No emulation.** OpenRAC does not interpret the console's processors, run the
retail program, or model the GS, the vector units or the DMA controller to
play a game. An earlier experiment that did, `runtime/` (2026-10-08 to
2026-10-09), was removed on 2026-10-09; git history keeps it, and the
studies it produced about the games are in this folder.

Nothing from the games is ever committed: no code bytes, data, microcode or
media taken from a disc ([SOURCING.md](../policy/SOURCING.md)). The player
supplies their disc; the port reads what was extracted from it.

The port starts with Ratchet & Clank (PAL), [games/rac1/pal](../../games/rac1/pal),
whose matching decompilation is the furthest along. The sequels follow,
sharing the renderer where their engines agree
([docs/engine](../engine/README.md)).

The code is in [port/](../../port/README.md).

## Documents

| Document | For |
|---|---|
| [DESIGN.md](DESIGN.md) | What is decided, the shape of the port, the open decisions |
| [ROADMAP.md](ROADMAP.md) | The order of work, and whether the decompilation must be finished first |
| [SETTINGS.md](SETTINGS.md) | The PC settings and their file, frame pacing against the game's tick, the game's 50/60 Hz option, the graphics card checks, where a frame's time goes, and the settings beside OpenGOAL's and ReRAC's |
| [DECOMP_STATUS.md](DECOMP_STATUS.md) | How much of Ratchet & Clank (PAL) is C a PC could build, and what is not |
| [RENDERER.md](RENDERER.md) | How the game builds a frame, everything it draws, the VU programs, the graphics chip's conventions |
| [PORTABILITY.md](PORTABILITY.md) | What the matching C assumes about the console, and the choices for a PC build |
| [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md) | The first survey of the game for a native build (2026-10-08): frame loop, display list, menu path, disc, memory card, library surface |
| [RAC1_PAL_CAMPAIGN.md](RAC1_PAL_CAMPAIGN.md) | The rules and the dated log of the campaign for a playable Ratchet & Clank (PAL) |
| [RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md) | For whoever continues it: building and running, the development switches, how to find the cause of a gameplay bug, how to supply C for a function without any, and what is open |
| [VU_PROGRAMS.md](VU_PROGRAMS.md) | The games' vector unit programs by name, and which the games share |
| [OPENGOAL_NOTES.md](OPENGOAL_NOTES.md) | How OpenGOAL's renderer and platform layer work, and what carries over |
| [tools/extractor.py](../../tools/extractor.py) | The disc extractor (OpenGOAL's `extractor`, for OpenRAC) |
