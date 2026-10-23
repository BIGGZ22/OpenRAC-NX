# Speedrunning OpenRAC

This is the guide to speedrunning Ratchet & Clank on OpenRAC's native PC
port: how the port is timed, how to split a run, what practice tools exist,
what the game's own speedrun meta is, and what a category on the port would
have to look like. The documents are in [speedrun/](speedrun/README.md).

| Document | For |
|---|---|
| [speedrun/TIMING.md](speedrun/TIMING.md) | How the port's clock works: the 60 Hz tick, real time vs. game time, what "unlocked" means, and why the port's times are not comparable to a console's |
| [speedrun/LIVESPLITTING.md](speedrun/LIVESPLITTING.md) | Splitting a run with LiveSplit: the `live-splits` mod, the splits file, and how to extend it |
| [speedrun/PRACTICE.md](speedrun/PRACTICE.md) | Practice tooling: pause, single-step, slow motion, level select, scripted input, and what is missing |
| [speedrun/SAVESTATES.md](speedrun/SAVESTATES.md) | Save states: what exists, what a real one would take, and a design for it |
| [speedrun/META.md](speedrun/META.md) | The game's own speedrun meta: categories, records, routing tech, and the community's resources |
| [speedrun/RULES.md](speedrun/RULES.md) | What speedrun.com allows, and what a native-port category would need to say |
| [speedrun/REFERENCE.md](speedrun/REFERENCE.md) | Every knob: the command line, the environment variables, the settings file, the keys, the input map, the game's addresses |

## The short version

- **The port is timed at 60 ticks a second, always** — PAL included
  (`port/game/common/main.cpp:138-142`). The game keeps its own time base
  (`D_0015EE7C` on PAL, `D_0015ED7C` on NTSC-U), which the port holds at a
  sixtieth of a second. A run's *game time* is therefore ticks × 1/60.
- **Splitting a run works today.** The `live-splits` mod
  ([OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods)) drives a
  LiveSplit timer over LiveSplit's own server and splits at every planet. See
  [speedrun/LIVESPLITTING.md](speedrun/LIVESPLITTING.md).
- **Practice tooling is good but not a save-state system.** Pause, single
  step, slow motion and level select exist; **save states do not**. See
  [speedrun/PRACTICE.md](speedrun/PRACTICE.md) and
  [speedrun/SAVESTATES.md](speedrun/SAVESTATES.md).
- **Times are not comparable to the console leaderboards.** The port has no
  disc reads and no load screens, so a PS2 Any% that takes ~36 minutes on
  hardware takes far less here. A port run needs its own category. See
  [speedrun/TIMING.md](speedrun/TIMING.md) and
  [speedrun/RULES.md](speedrun/RULES.md).
- **Only Ratchet & Clank can be modded today.** rac1 has 654 hook points on
  both discs; rac2, rac3 and rac4 have none and no script binding, so a mod
  for them is refused.

## A note on what is verified and what is not

Every claim about the port in this folder cites a file and line in the
repository, checked against the tree at commit `b522692a`. Claims about
speedrun.com were read from its public API; the leaderboard tables in
[speedrun/META.md](speedrun/META.md) are copied from it, not from memory.
Where a claim could not be checked, it says so.

## Sources of truth

| For | Read |
|---|---|
| The port's own design | [port/README.md](../../port/README.md), [DESIGN.md](DESIGN.md) |
| Frame pacing and settings | [SETTINGS.md](SETTINGS.md), [DEBUGGING.md](DEBUGGING.md) |
| Mods, the SDK, hook points | [MODS.md](MODS.md), [sdk/docs/](../../sdk/docs/README.md) |
| Playing the port by script | [RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md), [REGRESSION.md](REGRESSION.md) |
| The game's own knowledge | [games/rac1](../../games/rac1), [docs/engine](../engine/README.md) |
