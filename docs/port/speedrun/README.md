# Speedrunning Ratchet & Clank on the port

The documents of the [speedrun guide](../SPEEDRUN.md), in reading order.

| Document | For |
|---|---|
| [TIMING.md](TIMING.md) | How the port's clock works: the 60 Hz tick, real time vs. game time, what "unlocked" means, and why the port's times are not comparable to a console's |
| [LIVESPLITTING.md](LIVESPLITTING.md) | Splitting a run with LiveSplit: the `live-splits` mod, the splits file, and how to extend it |
| [PRACTICE.md](PRACTICE.md) | Practice tooling: pause, single-step, slow motion, level select, scripted input, and what is missing |
| [SAVESTATES.md](SAVESTATES.md) | Save states: what exists, what a real one would take, and a design for it |
| [META.md](META.md) | The game's own speedrun meta: categories, records, routing tech, and the community's resources |
| [RULES.md](RULES.md) | What speedrun.com allows, and what a native-port category would need to say |
| [REFERENCE.md](REFERENCE.md) | Every knob: the command line, the environment variables, the settings file, the keys, the input map, the game's addresses |

## The three things worth knowing

1. **The port ticks 60 times a second, always** — the PAL disc included, which
   a PAL PlayStation 2 does not. The game's own time base agrees, so game time
   and real time are the same number here. See [TIMING.md](TIMING.md).
2. **Splitting works today.** The `live-splits` mod drives a LiveSplit timer
   over LiveSplit's own server and splits at every planet. See
   [LIVESPLITTING.md](LIVESPLITTING.md).
3. **There are no save states.** The port has pause, single-step, slow
   motion and level select, but not a savestate. [SAVESTATES.md](SAVESTATES.md)
   works out what one would take — and it is more feasible here than it looks,
   because the port's game state is one 32 MB array the port already knows how
   to dump and read back.

## Verified or not

Every claim about the port cites a file and line, checked against the tree at
commit `b522692a`. The speedrun.com leaderboard tables in
[META.md](META.md) were read from its public API, not from memory. Where a
claim could not be checked, the document says so.
