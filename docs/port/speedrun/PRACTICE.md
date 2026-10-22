# Practice tooling

What a runner can do today to practise a segment, and what is missing.

## 1. The debug overlay

Press **F3** for the overlay, or start with `--debug-overlay`, or set
`OPENRAC_OVERLAY=1` (`port/game/common/debug_overlay.cpp:2070-2074`,
`port/game/common/main.cpp:256-257`).

Its sections are `frame`, `renderer`, `memory`, `mobys`, `game` and `shaders`
(`debug_overlay.cpp:55-57`).

| Section | What is in it |
|---|---|
| **Frame** | Ticks a second measured against the game's own; frames shown a second; frame milliseconds; where a frame's time goes (`game tick`, `scene read from memory`, `blend between ticks`, `render submission`, `GPU`, `present`, `swap`, `wait for the next tick`); a 240-frame plot; a histogram of the game's own milliseconds per tick. |
| **Renderer** | GL renderer and version, draw calls, triangles, texture uploads; a row per pass (2D, world, sky, terrain, ties, shrubs, mobys, effects, hud) with CPU and GPU time and a per-pass checkbox. |
| **Memory** | Host memory use; the game's 32 MB map with the cells that changed in the last half second; stack use; a button that writes main memory to `ram_<tick>.bin`; a memory viewer (256 bytes, as bytes/words/floats, with follow). |
| **Mobys** | The live mobys nearest the hero: address, class number and name, state, position, distance, update function. Click a row for an inspector — collision sphere, scale, animation, pvars, the raw bytes. "Labels in the picture" and "click picks" are there too. |
| **Game** | Level and its name, tick, game mode word, hero position and state, camera, **pad 0 as the game reads it (scripted input included)**, the functions-without-C that were called, and a **probes tree**: every `OPENRAC_*` switch the program has read, with a checkbox to flip it while running. |
| **Shaders** | Every program, its files, whether it linked, and a "reload all from disk" button. |

## 2. Pause, step and slow motion

These work whether or not the overlay is showing, and say so in the picture
when they are (`debug_overlay.cpp:2076-2097`):

| Key | |
|---|---|
| **F5** | Pause / resume |
| **F6** | One tick forward (it pauses first) |
| **F7** | Slower — halves the speed each press, down to 1/16 |
| **F8** | Faster — back up to 1× |
| **F12** | Screenshot |
| **F2** | The frame-rate counter |
| **F9** / **F10** | Volume down / up |
| **F11** | Full screen |
| **F1** | Level select — **only with `OPENRAC_DEVELOPER=1`** (`debug_overlay.cpp:290-292`, `:1527-1529`) |

The slow motion is implemented as a **stretch of the tick period**
(`port/game/common/main.cpp:640-649`), so the game runs slower rather than
running normally and being sampled less often. There is no faster-than-1×
control in the overlay: `OPENRAC_UNCAPPED=1` is the only way to make the game
run faster than real time, and it is not a play mode.

Two things to know:

- **The sound does not pause.** It runs on the audio thread
  (`docs/port/DEBUGGING.md`).
- **`OPENRAC_UNCAPPED=1` disables all of this**, because the uncapped path
  returns before the pause check (`main.cpp:778-780`).

Scriptable equivalents, for a repeatable setup:

| | |
|---|---|
| `OPENRAC_PAUSE_AT=TICK[:FRAMES]` | Pause at that tick; with `FRAMES`, resume by itself after that many held pictures. |
| `OPENRAC_TIME_SCALE=X` | The game's speed from the start (1/64 … 1). |
| `OPENRAC_KEYS="1160:F5,1161+20:F6,1162+20:F5"` | Scripted keyboard input, the only way to script the overlay itself. |

## 3. Level select

Three ways to reach a level directly:

| | |
|---|---|
| **`--level N`** | The level a new game starts in (0–18, the game's own numbering). Without `OPENRAC_DIRECT` the title and menus still run. |
| **`OPENRAC_DIRECT=1` with `--level N`** | Straight into level N from the title: the title's wait and the menus are skipped. |
| **F1 in developer mode** | The in-game level menu. Needs `OPENRAC_DEVELOPER=1`. |

The levels, in the port's own order (`port/game/rac1/host/front_end.c:12-16`,
matching `port/tools/regress/routes.json`):

| | | | |
|---|---|---|---|
| 0 Veldin | 5 Rilgar | 10 Orxon | 15 Quartu |
| 1 Novalis | 6 Blarg Station | 11 Pokitaru | 16 Kalebo III |
| 2 Aridia | 7 Umbris | 12 Hoven | 17 Drek's Fleet |
| 3 Kerwan | 8 Batalia | 13 Gemlik Base | 18 Veldin (final) |
| 4 Eudora | 9 Gaspar | 14 Oltanis | |

The worked example from `docs/port/RAC1_PAL_TESTING.md`:

```sh
D="$HOME/Library/Application Support/dev.openrac.launcher/active/rac1/data"
SDL_AUDIODRIVER=dummy OPENRAC_UNCAPPED=1 OPENRAC_DIRECT=1 \
OPENRAC_PRESS=100:4000:5 \
  ../../port/build/release/openrac-rac1-pal --data "$D" --levels ../../assets/port/rac1/pal \
  --window --no-card --level 1 --skip-movies --frames 2300 > run.log 2>&1
```

`--no-card` never touches the player's saves; `--frames N` ends the run after
N ticks.

## 4. Scripted input

The port can be driven by script, which is the foundation of repeatable
practice setups and of the regression harness.

### `OPENRAC_PRESS` — pad input on a schedule

```
OPENRAC_PRESS=FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],...
```

Holds buttons (and optionally the sticks) for `FRAMES` window frames starting
at `FRAME`. The mask is in the pad's own bit values, **active-low as the pad
reports them**: `0x4000` cross, `0x8` start, `0x1` select, `0x10`/`0x20`/
`0x40`/`0x80` the d-pad, `0x100`/`0x200`/`0x400`/`0x800` L2/R2/L1/R1, `0x1000`
triangle, `0x2000` circle, `0x8000` square. Sticks are 0–255, 128 centred, 0
is left or up.

Four counting forms:

| Form | |
|---|---|
| `FRAME` | Window frames from the start. |
| `cFRAME` | Frames **from control** — the same moment in every version. |
| `kPERIOD` | Holds the buttons for the first `FRAMES` of every `PERIOD` frames while what plays can be skipped and control has not come yet (Start through the scenes and the flight). |
| `sPERIOD` | The same, counted from the frame the scene became skippable — the same tick of the scene in every version. |
| `SPERIOD` | `s`, and also after control. |

— `port/game/common/frontend.cpp:3082-3126`

### `OPENRAC_WRITE` — write the game's memory on a schedule

```
OPENRAC_WRITE=FRAME:ADDRESS:VALUE[:FRAMES],...
```

Writes a 32-bit word into game RAM at each vertical blank from `FRAME` for
`FRAMES` blanks (default 1). Hexadecimal address and value; decimal frame
counts. `cFRAME` counts from control.
(`port/game/common/main.cpp:610-637`)

This is how you teleport Ratchet. The hero's position is at `0x13F4D0` /
`0x13F4D4` / `0x13F4D8` (PAL) and his yaw at `0x13F4E8`.

### `OPENRAC_CONTROL_FRAMES=N` — end the run N frames after control

(`port/game/common/frontend.cpp:2309-2318`) — a route's length.

### The regression harness

`port/tools/regress/` plays every level of both versions by script in hidden
windows, records what the game does tick by tick, and compares that with
another build, the other version or a saved baseline. The routes are
declarative, in `port/tools/regress/routes.json`:

```json
"sequences": {
  "standard": [
    {"at": 30,  "hold": 120, "left": [128, 0],   "what": "walk forward"},
    {"at": 160, "hold": 150, "right": [255, 128], "what": "turn the camera round"},
    {"at": 320, "hold": 5,   "buttons": "cross",  "what": "jump"},
    {"at": 360, "hold": 5,   "buttons": "square", "what": "swing the wrench"}
  ]
}
```

A run is `ok`, `no-control`, `crash`, `hang` or `exit`, and each level keeps
its own `trace.jsonl`, `reach.json` and `run.log`. See
[REGRESSION.md](../REGRESSION.md).

**This is the closest thing the port has to a practice-route system**, and it
is a test harness, not a runtime feature: it is driven by environment
variables, not by a file the game reads while you play.

## 5. Controllers

The port reads any SDL3 gamepad: buttons by position (the bottom face button
is Cross on a DualSense and A on an Xbox pad), both sticks, and both
triggers. What the game does with the controller is the interesting part, and
it is documented in [CONTROLLERS.md](../CONTROLLERS.md):

- **Ratchet & Clank 1 uses analog button pressure in exactly one place** — the
  hero's swimming turn rate, scaled by the **Cross** and **Square** buttons.
- **The game has no rumble at all** — the retail executable links no motor
  command, so there is nothing to restore.
- The **stick dead zone is the game's own** (a `0x30` threshold with a
  `/0x4C` scale, `games/rac1/src/game/pad.c:346-362`); SDL3 applies none.
- The **touchpad click** and the **DualSense Edge's paddles** are two spare
  buttons the PlayStation 2 never had — useful for weapon switching once the
  port has its rebinding page.
- The port has **no rebinding** yet; the `keyboard-mouse` mod is the
  workaround.

## 6. The existing practice mods

From [OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods), all of which
work on both discs:

| Mod | What it does for practice |
|---|---|
| **`difficulty`** | A slider for everything: enemy count, damage both ways, enemy and Ratchet speed, weapon and ammo prices, ammo per shot, max ammo, bolt drops, health. Presets Easy / Retail / Hard / Brutal / a seeded Chaos. **This is the mod for practising a route** — it removes the deaths that cost you the segment. |
| **`item-randomizer`** | Shuffles gadgets, weapons, vendor stock, gold bolts and infobots, with logic so every seed can be finished. Not for a serious run; useful for learning the game. |
| **`live-splits`** | The timer. See [LIVESPLITTING.md](LIVESPLITTING.md). |
| **`lock-strafe`** | UYA-style lock-strafe and GC-style strafe. A movement change, not a practice tool — but it is the closest thing to a movement option the port has. |
| **`keyboard-mouse`** | WASD, mouse camera, click to fire, right-click for the Quick Select, number keys and wheel for weapons; every action rebindable. **The most comfortable option for a runner on a PC.** |
| **`button-swap`** | Swaps Cross/Square, Circle/Triangle or L1/R1. |

The strafe used to be a port setting and became a mod on 2026-10-21
(`docs/port/SETTINGS.md`).

## 7. What is missing

Ranked by how much they cost a practice session.

| Missing | Why it matters | What it would take |
|---|---|---|
| **Save states** | The single biggest gap. See [SAVESTATES.md](SAVESTATES.md). | A new subsystem in the port: dump and restore the guest memory plus a handful of host-side values. The groundwork is already there — see that document. |
| **A practice-route player** | The port can be *scripted* from the environment, but not from a file while you play. A route you can load, scrub to a timestamp, and replay would make segment practice repeatable. | The `OPENRAC_PRESS` and `OPENRAC_WRITE` parsers already exist (`frontend.cpp:3082-3126`, `main.cpp:610-637`). A mod could read the same format from a file and drive the pad through `on_pad` — the `keyboard-mouse` mod shows how. |
| **A free camera** | The overlay's camera is read-only. `openrac-viewer` has one (`--camera x y z [rx ry]`), but it is a separate binary that draws a level, not the game. | A mod that rewrites the camera each tick. `FollowCamera_ApplyOverrides` (`rac1.camera.follow.apply_overrides`) is exactly the hook point for it — `keyboard-mouse` takes it. |
| **An in-game overlay** | There is no drawing API for mods, so no in-game split list, no route display, no IGT counter in the picture. | A renderer hook point in the port, listed as *proposed* in `MODS.md`. A significant addition. |
| **Per-segment times visible in-game** | LiveSplit shows them, but not next to the game. | The same drawing API. |
| **A frame-step display of the input** | `OPENRAC_TRACE_INPUT=1` logs the pad every 25 frames; the overlay's Game section shows pad 0 as the game reads it. But there is no input history to scrub. | A ring buffer of pad reports in the overlay. |
| **Load-removal / pause the timer on load** | There is nothing to load, so this is moot in the port — but LiveSplit's `pausegametime` exists if a category ever wants it. | Nothing. |

## 8. A practical practice setup

What a runner would actually do today:

```sh
# Practice one segment, from the level, as fast as the machine goes.
D="$HOME/Library/Application Support/dev.openrac.launcher/active/rac1/data"
SDL_AUDIODRIVER=dummy OPENRAC_UNCAPPED=1 OPENRAC_DIRECT=1 \
OPENRAC_PRESS=100:4000:5 \
  ../../port/build/release/openrac-rac1-pal --data "$D" --levels ../../assets/port/rac1/pal \
  --window --no-card --level 5 --skip-movies --frames 3000
```

- `--level 5` puts you in Rilgar; `--frames 3000` ends it after 50 seconds of
  game time.
- `OPENRAC_UNCAPPED=1` makes it run as fast as the machine allows, so a
  3000-tick segment takes seconds. Pause and step do not work in this mode,
  so drop it when you want to step.
- `--no-card` keeps the player's saves untouched.
- The `difficulty` mod set to Easy removes the deaths.
- `live-splits` on, with a splits file, gives you the segment time.

And to check the result, the port can dump each frame's measurements
(`--debug-dump FILE`) or each tick's state (`OPENRAC_TRACE=FILE`) — see
[TIMING.md](TIMING.md) §6.
