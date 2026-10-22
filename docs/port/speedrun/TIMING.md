# Timing: how the port's clock works

Everything a speedrunner needs to know about what the clock is doing while the
game runs. This is the document to read before trusting any time the port
reports.

## 1. The tick: 60 a second, for every version

The port paces the game with a tick, and the tick rate is a constant:

```cpp
// The game's ticks a second: 60 for every version. The PAL game runs in its own 60 Hz mode
// (options_menu.cpp keeps it there), so its time base is a sixtieth of a second too.
int tick_rate() {
    return 60;
}
```

— `port/game/common/main.cpp:138-142`

**This applies to the PAL disc as well as the NTSC-U disc.** A PAL PlayStation
2 runs its game at 50 Hz; this port runs the PAL game at 60. That is not a
rounding of the console's behaviour, it is a different time base, and it
matters for two reasons:

1. **The game logic itself runs differently.** The PAL game reads its own tick
   length from `D_0015EE7C` and its tick scale from `D_0015EE60`; about five
   hundred places in the game's code read those (`games/rac1/src/game/mobyutil.c:1115-1136`,
   the `SetVideoModeTiming` function the decompilation names). On a PAL disc at
   50 Hz the tick length is 0.02 s and the scale is 1.2; at 60 Hz they are
   0.0166… s and 1.0. The port keeps the PAL game in its own 60 Hz mode
   (`port/game/common/options_menu.cpp:716-721`, which holds the front end's
   video-mode byte at 0), so the PAL game in the port is running the *60 Hz*
   branch of its own code, exactly as it would in its Options → Video menu on
   a television that supports 60 Hz.
2. **Times are not comparable to a 50 Hz console run.** The community's own
   version-differences note says the PS2 at 50 Hz cannot do the Orxon 1 Clank
   proxy, while 60 Hz can; the port always behaves as 60 Hz.

> **A stale document.** `docs/port/SETTINGS.md` §2 and §3 still describe the
> port as ticking 50 times a second for PAL, and as taking the tick rate from
> the video mode the game last set. Both were true before commit `14e72695`
> ("feat(port): run every game at 60 Hz, PAL in its own 60 Hz mode",
> 2026-10-15); that commit changed the code and `port/game/rac1-pal/README.md`
> but not `SETTINGS.md`. The code is authoritative. The generated
> `openrac_game.frame_rate` is still 50 for PAL
> (`port/game/rac1-pal/hostgen.json:7`) but nothing reads it — grep for
> `openrac_game.frame_rate` in `port/` returns no consumer.

## 2. The tick is on the wall clock

The game's tick is scheduled against `std::chrono::steady_clock` on an
absolute deadline, so it neither drifts nor runs faster on a fast machine:

```cpp
g_next_frame += period;          // an absolute deadline, not a delay
...
std::this_thread::sleep_until(g_next_frame);
```

— `port/game/common/main.cpp:785-800`

The consequences, all of which matter:

| | |
|---|---|
| **A fast machine does not speed the game up.** | The tick is on the clock. A 4090 draws frames while a GTX 1080 drops them; the game ticks at the same rate on both. |
| **A slow machine does not slow it down either.** | A tick that arrives more than a tick late has its *frame* dropped instead, at most three in a row (`port/game/common/frontend.cpp:2824-2850`). Only a machine that cannot draw one frame in four ticks (80 ms) actually slows the game, and it says so in the log. |
| **Real time and game time agree.** | Because the tick is 1/60 s of wall clock and the game's own tick length is also 1/60 s, the two do not drift apart. A run's real time and its game time are the same number. |

### The exception: `OPENRAC_UNCAPPED=1`

Setting `OPENRAC_UNCAPPED=1` makes the loop return before the deadline
advance and the sleep (`port/game/common/main.cpp:778-780`), so the game
executes as many ticks a second as the machine can manage. It also forces one
frame a tick and no vsync (`port/game/common/frontend.cpp:544-548`).

The code calls it "a measure, not a way to play" (`main.cpp:722-724`). It is
useful for a **speedrun tool**: it turns the game into a deterministic
tick-stepper that can be driven far faster than real time. It is useless for
a recorded run, whose time is real time.

Note that it also skips the pause check at `main.cpp:804`, so the pause, step
and slow-motion controls do not work in uncapped mode.

## 3. Frames are not ticks

The picture and the game are decoupled, and the port offers four frame rates:

| `frame_rate` | Frames drawn | Interpolated? | VSync |
|---|---|---|---|
| `original` | exactly one per tick (60/s) | no | off (the tick paces the frames) |
| `display` | one per display refresh | yes | on → the swap waits; off → the port's clock holds the display's rate |
| a number N (20–1000) | N per second | yes | both hold |
| `unlimited` | as many as the machine draws | yes | on → refresh-bound; off → machine-bound |

— `port/game/common/frontend.cpp:1537-1553` (the cap), `:2892-2894` (interpolation),
`:544-560` (the swap's wait)

Between ticks the port draws frames interpolated from the previous tick's
scene to the current one, **half a tick behind** (`kLagTicks = 0.5`,
`frontend.cpp:1555-1557`), so the next tick is always ready before the
refresh that needs it.

**For speedrunning, `frame_rate = original` is the honest setting**: one
picture per tick, no interpolation, no vsync waiting, and the picture is
exactly the game's state at a tick boundary. Anything else is a smoother
picture of the same game.

## 4. What the game itself thinks the time is

The game keeps its own time base in memory. The `live-splits` mod reads it
(`rac1-mods/live-splits/src/live_splits.cpp:42-45`, `:59-68`):

| Disc | Address | Value in the port |
|---|---|---|
| PAL | `D_0015EE7C` | 0.0166… (1/60 s) |
| NTSC-U | `D_0015ED7C` | 0.0166… (1/60 s) |

The mod multiplies its tick count by this and sends the result to LiveSplit as
`setgametime`. Because the port runs at 60 Hz on both discs, **game time and
real time are the same in the port**, and a LiveSplit run using the Game Time
timing method is measuring the same thing as one using Real Time.

This is a genuine difference from the console. On hardware, PAL at 50 Hz has a
game time 20 % longer than its real time; the community's rules for `rac1`
list only `realtime` as a run type
(<https://www.speedrun.com/api/v1/games/y655j36e>, `ruleset.run-times`),
so no console leaderboard has a game-time column to compare against anyway.

## 5. Loading: there is none

This is the single biggest difference between a port run and a console run.

The port does not read the disc at runtime. Levels, textures, sounds and
models are extracted once by the player's own copy of the extractor into
files the port reads directly. There is no disc seek, no "loading" screen, no
IPL, no FMV buffering pause.

The community's own guide puts the console's loading cost at around **eight
minutes** for a PS2 run
(<https://www.speedrun.com/rac1/guides/gnz18>, "Getting started with
Ratchet 1 speedrunning"): "The loading times are extremely slow, estimated to
lose around 8 minutes."

So a port Any% is not a PS2 Any% with the loading removed. It is a different
race, and it needs a different leaderboard. See [RULES.md](RULES.md).

What the port *does* still spend time on:

| | |
|---|---|
| **Level transitions** | The scene between planets (the flight, the landing, the opening cutscene). These are still in the game and still take time. The flight world is level 98, the title world 99. |
| **Movies** | `--skip-movies` returns the skipped result immediately (`port/game/common/main.cpp:914-917`). The setting `skip_cutscenes = 1` (on by default) makes Start press the game's own skip buttons. Both are **practice** tools — see [PRACTICE.md](PRACTICE.md) for why skipping movies invalidates a run. |

## 6. Measuring the port itself

The port has a machine-readable frame log, which is what a verification tool
should read.

`--debug-dump FILE` writes one JSON object per frame
(`port/common/stats.cpp:83-121`). The keys, in the order they are written:

| Key | Meaning |
|---|---|
| `frame` | every frame measured, from 0 |
| `tick` | the window's frame counter when the frame was drawn |
| `kind` | `tick`, `between`, `picture` or `held` |
| `alpha` | 1.0 = this tick's scene, 0 = the last tick's |
| `t` | seconds since the first frame measured |
| `frame_ms` | this frame's start to the next one's |
| `game_ms` | **only on `kind == "tick"`**: the game's own code since the last tick |
| `gpu_ms` | the sum of the sections' GPU times, or null |
| `draw_calls`, `triangles`, `texture_uploads`, `texture_upload_bytes`, `buffer_upload_bytes`, `program_binds`, `framebuffer_binds` | render counters |
| `sections` | an array of `{name, depth, cpu_ms, gpu_ms, draw_calls, triangles}` |
| `gauges` | `mobys`, `mobys_drawn`, `ui_quads`, `effect_quads`, `packet_bytes`, `level` |

Two operational notes: the file **runs up to 16 frames behind** (a frame is
written only once its GPU times come back, `port/common/stats.cpp:33`), and
the same data can be produced from the debug overlay's "dump frames" checkbox,
which writes `openrac_frames.jsonl` in the working directory.

A second, coarser stream exists for the regression harness:
`OPENRAC_TRACE=FILE` writes one JSON line a tick with the frame, the frames
since the player got control, the level, the game mode, the camera, the hero
and the live moby count (`port/game/common/frontend.cpp:2280-2308`).

## 7. Summary for the runner

| Question | Answer |
|---|---|
| What is a tick? | 1/60 s, on every disc, always |
| Is game time the same as real time here? | Yes — the port's tick is the wall clock and the game's own time base agrees |
| Can I make the game run faster than real time? | Yes, `OPENRAC_UNCAPPED=1`, but that breaks the pause controls and changes nothing about the game's own tick count |
| Does the port load anything during a run? | No. No disc reads, no load screens |
| Are port times comparable to the console leaderboards? | **No.** They are faster by roughly the console's loading time, and they are at 60 Hz on PAL where the console is at 50 Hz |
| What is the right frame rate setting for a run? | `frame_rate = original` — one picture per tick, no interpolation |
| Where does the machine-readable timing live? | `--debug-dump FILE` (JSON lines) and `OPENRAC_TRACE=FILE` (JSON lines per tick) |
