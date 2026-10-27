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

The port starts with Ratchet & Clank (PAL), [games/rac1](../../games/rac1),
whose matching decompilation is the furthest along. The sequels follow,
sharing the renderer where their engines agree
([docs/engine](../engine/README.md)).

The code is in [port/](../../port/README.md).

## Documents

| Document | For |
|---|---|
| [DESIGN.md](DESIGN.md) | What is decided, the shape of the port, the open decisions |
| [ROADMAP.md](ROADMAP.md) | The order of work, and whether the decompilation must be finished first |
| [SETTINGS.md](SETTINGS.md) | The PC settings and their file, frame pacing against the game's tick, the game's 50/60 Hz option, the graphics card checks, where a frame's time goes, the settings beside OpenGOAL's and ReRAC's, and the game fixes (what the port does otherwise than the original, and what a player switches back) |
| [SPEEDRUN.md](SPEEDRUN.md) | Speedrunning the port: how it is timed, how to split a run with LiveSplit, the practice tooling, what a save state would take, the game's own speedrun meta, and what a category on the port would have to say |
| [CONTROLLERS.md](CONTROLLERS.md) | Controllers: what the port does with a PlayStation 5 DualSense today, what Ratchet & Clank actually asks a controller for (analog button pressure, once, for swimming; no rumble at all, even on the console), what SDL3 exposes for adaptive triggers and the light bar, and what to build |
| [MODS.md](MODS.md) | Mods: native plugins behind a versioned C interface, their manifest, hook points, the launcher's list and the port's answer, purist mode, crash reports that name them, the SDK ([sdk/](../../sdk/README.md)) |
| [WEAPONS.md](WEAPONS.md) | Every weapon and how to test it in the port: the item ids, the one call that gives a weapon (`GiveItem`), the vendors, a verified `OPENRAC_WRITE` recipe for putting any weapon in Ratchet's hands, what to watch when one fires, the cheat codes and why not to arm a test with them, the I-5 debug menu, and what to watch (the fire flag, `button_mask`, the Walloper's state) |
| [DECOMP_STATUS.md](DECOMP_STATUS.md) | How much of Ratchet & Clank (PAL) is C a PC could build, and what is not |
| [HARDWARE_SEMANTICS.md](HARDWARE_SEMANTICS.md) | What the original code relies on in the PS2's processors (rounding, no NaN, 128-bit registers, VU0 flags, MMI, the GIF stream, ...) and how the port keeps each |
| [DEBUGGING.md](DEBUGGING.md) | The debug overlay, the frame dump, pause and slow motion, every `OPENRAC_*` environment variable |
| [RENDERER.md](RENDERER.md) | How the game builds a frame, everything it draws, the VU programs, the graphics chip's conventions |
| [RAC1_GPU_DRIVEN.md](RAC1_GPU_DRIVEN.md) | Vulkan visibility, compaction, indirect batching and resource lifetime |
| [RENDER_PERF.md](RENDER_PERF.md) | What drawing costs: the render counters and gauges, the timed passes, the optimisation registry, and each render package's numbers |
| [PERFORMANCE_CAPTURE.md](PERFORMANCE_CAPTURE.md) | Recording a stretch of play on a desktop or a phone into one short report to paste: frame times, a verdict, CPU and GPU time per pass, the device's clocks and heat |
| [RAC1_PROBE_GI.md](../../port/enhanced/docs/RAC1_PROBE_GI.md) | Baked probe GI, sparse and dense volumes, sampling and validation |
| [RAC1_HERO_SHADOW.md](../../port/enhanced/docs/RAC1_HERO_SHADOW.md) | Ratchet's shadow map: directional on the ground, downward in the air |
| [RAC1_SUN_SHADOW.md](../../port/enhanced/docs/RAC1_SUN_SHADOW.md) | Four sun cascades, a 10% directional-light floor and stencil suppression |
| [ANDROID.md](ANDROID.md) | The port and the launcher on Android: one APK, the game's activity, OpenGL ES 3.2, the tools on the device, GPU driver packages (Mali, Adreno); building and device tests in [port/android](../../port/android/README.md) |
| [PORTABILITY.md](PORTABILITY.md) | What the matching C assumes about the console, and the choices for a PC build |
| [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md) | The first survey of the game for a native build (2026-10-08): frame loop, display list, menu path, disc, memory card, library surface |
| [RAC1_PAL_CAMPAIGN.md](RAC1_PAL_CAMPAIGN.md) | The rules and the dated log of the campaign for a playable Ratchet & Clank (PAL) |
| [WORKFLOW.md](WORKFLOW.md) | **Start here to help:** the whole loop from a player's report to a release (taking the report, reproducing it by script, finding the cause, where a fix belongs, checking it, landing it), with the kinds of cause met so far |
| [RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md) | Playtesting by script: starting any level, teleporting Ratchet, moving the camera, pressing buttons on a schedule, skipping scenes and flights, screenshots and reading game memory, and checking a fix |
| [REGRESSION.md](REGRESSION.md) | Gameplay regression runs: every level of both versions played by route in hidden windows, a trace per tick compared with another build, the other version or a baseline, and what the runs reached without right C |
| [BACKFILL.md](BACKFILL.md) | Functions with no matching C yet: which ones to fill in for the port, how to write their C, and the fcheck gate that says when it is done |
| [RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md) | For whoever continues it: building and running, the development switches, how to find the cause of a gameplay bug, how to supply C for a function without any, and what is open |
| [RAC1_NTSC_HINTS.md](RAC1_NTSC_HINTS.md) | Where the US game's own code is known to trouble a native build (planets, gameplay systems, rendering, sound, saves), with the places in this tree to look and what the port already does |
| [VU_PROGRAMS.md](VU_PROGRAMS.md) | The games' vector unit programs by name, and which the games share |
| [OPENGOAL_NOTES.md](OPENGOAL_NOTES.md) | How OpenGOAL's renderer and platform layer work, and what carries over |
| [tools/extractor.py](../../tools/extractor.py) | The disc extractor (OpenGOAL's `extractor`, for OpenRAC) |
