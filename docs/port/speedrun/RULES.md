# Rules: what a port run would need

What speedrun.com allows today, and what a native-port category would have to
say to be worth submitting.

The short answer: **a port run cannot go on the console leaderboards**, and
it should not try. It needs its own platform and its own rules.

## 1. The site rules that bind everyone

From speedrun.com's site rules
(<https://www.speedrun.com/support/learn/site-rules>):

> **The time of the run is the time in the video from starting condition to
> ending condition. It is not the time shown on the timer program, which is
> started and ended with human error.**

> **Tool-assisted emulator features such as save-states, re-recording, or
> frame advance are not allowed.**

> **Game memory must not be manipulated or monitored with outside tools during
> runs** (For example: Emulator RAM Watch, Cheat Engine). **Exceptions can be
> made on a community basis** for convenience. Examples of these conveniences
> might be item trackers or info HUDs for stream viewers or **auto-splitters**.
> These should not include information that would be unknown to the runner.

> **The game itself must not be altered or controlled with outside tools.**
> Exceptions can be made on a community basis.

> **Segmented runs** (runs performed in multiple sittings, often with each
> segment retried many times until it is made as optimal as possible) are
> most often disallowed. Communities may in rare cases choose to explicitly
> track these under a separate category.

Three of those matter directly to the port:

1. **The video is the run.** The timer is a convenience. A moderator watching
   a port run will time it from the video, not from LiveSplit.
2. **Memory watching during a run is out.** The port's own `live-splits` mod
   reads the game's memory — but it does so *inside* the game process, and
   only to detect level transitions the player can see. That is the same
   category as an autosplitter, which the rules explicitly allow. A mod that
   showed information the runner could not otherwise know would not.
3. **Save states are out.** Not "the port's save states" specifically — save
   states as a category of tool. See [SAVESTATES.md](SAVESTATES.md).

## 2. Why a port run cannot go on the console board

Two independent reasons.

### 2.1 The game is different

| | |
|---|---|
| **No loading.** | The port reads no disc at runtime. The console's loading costs a PS2 run around eight minutes (<https://www.speedrun.com/rac1/guides/gnz18>). |
| **60 Hz on PAL.** | The port runs the PAL game at 60 Hz (`port/game/common/main.cpp:138-142`); a PAL console runs it at 50 Hz. The game's own logic takes a different branch (`SetVideoModeTiming`, `games/rac1/src/game/mobyutil.c:1115-1136`) and different tricks become possible — the community's version-differences note says the Orxon 1 Clank proxy cannot be done at 50 Hz. |
| **The build is a different program.** | OpenRAC is a decompilation recompiled for the PC, not the retail program. It is a different executable with the same behaviour, and the community's rule is about the game, not the executable. |

So a port run is not a faster console run. It is a different race.

### 2.2 The site provides for exactly this

speedrun.com's emulator/native distinction is a **per-game, per-platform**
setting, not a global split. The tools are:

- a **Platform** entry (there is already a `PC` platform id, `8gej2n93`);
- an **"Emulator runs"** toggle per leaderboard
  (<https://www.speedrun.com/support/learn/editing-a-leaderboard>: *"Emulator
  runs (Shown by default): This option will determine whether Emulators are
  shown or hidden by default, or banned completely."*).

**The precedent is in this very series.** *Ratchet & Clank: Rift Apart* has
platforms **PC and PS5**, `emulators-allowed: false`, and run types
`[realtime, realtime_noloads]` with `realtime_noloads` as the default
(<https://www.speedrun.com/api/v1/games?name=Rift%20Apart>). A native PC
Ratchet & Clank entry with a load-removed ruleset is exactly the shape the
series already has.

## 3. What a port category should say

This is a proposal, not something that exists. It is written so that the
`rac1` moderators have something concrete to argue with.

### 3.1 The platform

**Option A — a new platform on the existing `rac1` game.**

Add a platform, e.g. "PC (OpenRAC)", and give it its own boards for the
existing categories. Simple, and it puts the port's times next to the
console's for comparison.

*Risk:* the categories' existing rules ("from a fresh file", "with the
wrench out") were written for the console and do not all transfer. The PS2
Any% rule about a pre-made save with 0 bolts and 10 Bomb Glove ammo is
console-specific.

**Option B — a separate game entry**, e.g. "Ratchet & Clank (OpenRAC)". Clean
rules, clean boards, no risk of contaminating the console records.

*Cost:* the community has to be convinced to maintain a second game.

**Option A is the better first step**, because it keeps the categories and
the records in one place and makes the comparison visible. But the categories'
rules must be rewritten for the platform.

### 3.2 The rules a port category needs

| Rule | Why |
|---|---|
| **"From a fresh game, with no mods loaded."** | The port has mods that change the game: `difficulty` (enemy count, damage, speeds, prices), `item-randomizer` (shuffles the game's contents), `lock-strafe` (changes the movement model), `keyboard-mouse` (input only, safe). The port's **Purist mode** (`--no-mods`, or the launcher's Mods switch under Play, off) starts the game with no mods at all — that is the clean way to express this rule. |
| **"Mods that only observe the game, such as an auto-splitter, are allowed."** | This mirrors the site's existing carve-out for autosplitters. The `live-splits` mod only observes. |
| **"The run is timed from the video."** | The site's own rule. Stating it removes any argument about the timer. |
| **"No save states, no re-recording, no frame advance."** | The site's rule, and it means the port's practice tooling is not a run tool. |
| **"Video required; verification required."** | As on the console boards. |
| **"The tick rate is 60 Hz; the PAL and NTSC-U discs are separate categories or a variable."** | The port runs both discs at 60 Hz, but the *discs* are different games in practice: different level scripts, different assets, different strats. The community's `Storage` variable (obsolete but still used) shows it already tracks such distinctions. |
| **"The in-game timer may not be used for the hoverboard races."** | The console's races use the in-game timer, which the category rules mandate. In the port the game's tick count is exact and the mod can read it, so a race's time should be **the game's tick count**, not the game's own race timer. This needs a decision, because the in-game timer and the tick count are not the same measurement. |

### 3.3 What the port can offer a moderator that an emulator cannot

This is the argument to make to the community.

A native port can emit a **machine-readable event log**: the level id, the
tick, the player's position, the live moby count — one JSON line per tick.
The port already writes two such streams
(`--debug-dump FILE` and `OPENRAC_TRACE=FILE`, see
[TIMING.md](TIMING.md) §6), and a mod can write a third, richer one with the
same events the `live-splits` mod detects.

That gives a moderator something no emulator can give:

- a **precise, per-tick trace** of the run to compare against the video;
- **replayable verification**: the log can be checked for monotonicity, for
  missing levels, and for the IGT/RTA relationship;
- **a defence against cheating**: the log shows what the game actually did.

An emulator's memory can be watched, but nothing it produces is as precise
or as easily shared.

The proposal, then: **accept the port's timing log as the primary timing
source for a native category**, with the video as the authority. That fits the
site's rule that "a good reason might be a game that uses an auto-splitter to
track time without loads".

## 4. What is and is not allowed on the port today

| Tool | During a run? | Why |
|---|---|---|
| **`live-splits`** | Yes | It only observes the game and talks to LiveSplit. The site's carve-out for autosplitters covers it. |
| **`keyboard-mouse`** | Yes | Input remapping only; it changes no game state. |
| **`lock-strafe`** | **No, without a category rule** | It changes the game's movement model. |
| **`difficulty`** | **No** | It changes the game's rules. |
| **`item-randomizer`** | **No** | It changes the game's contents. |
| **Save states** | **No** | The site's rule. |
| **`--skip-movies`** | **No** | It removes content from the run. |
| **Pause / step / slow motion** | **No** | Tool-assisted frame advance. |
| **`OPENRAC_UNCAPPED=1`** | **No** | It makes the game run faster than real time; a recorded run's time is real time. |

**The port's own Purist mode** (`--no-mods`, or the launcher's Mods switch under Play, off)
starts the game with no mods at all
(`port/game/common/main.cpp:240-241`, `:385`, and
`port/game/common/mods.cpp:545-548`). That is the cleanest way to satisfy a
"no mods" rule: it is a switch, not a promise.

## 5. The verification workflow a port run would use

1. **Record the video.** The video is the run.
2. **Run in Purist mode**, or with only observing mods.
3. **Keep the timing log.** `--debug-dump` or `OPENRAC_TRACE`, plus the
   `live-splits` mod's `trace` setting, which logs every planet reached and
   left.
4. **Submit** to the platform's boards with the video and the log.

The port can make step 3 better than that: a mod that writes a run-scoped
event log (level ids, tick numbers, positions) would give a moderator a
complete, machine-checkable trace. That is a small addition to the existing
mod system and it is the strongest argument for a native category.

## 6. Sources

- speedrun.com's site rules: <https://www.speedrun.com/support/learn/site-rules>
- Editing a leaderboard: <https://www.speedrun.com/support/learn/editing-a-leaderboard>
- Moderation rules: <https://www.speedrun.com/support/learn/moderation-rules>
- The `rac1` game's ruleset: <https://www.speedrun.com/api/v1/games/y655j36e>
- *Rift Apart*'s ruleset, the native-PC precedent:
  <https://www.speedrun.com/api/v1/games?name=Rift%20Apart>
- The port's own mod system: [MODS.md](../MODS.md),
  [sdk/docs/](../../../sdk/docs/README.md)
- The port's Purist mode: `port/game/common/mods.cpp:544-548`
