# Claude — session 53: game speed at higher throughput (2026-11-01)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

What stays as it was:
- the tested NROs (release `4262dad2…` / ELF `dc4513fc…`, diagnostic `0eff4249…` / ELF `69645dc8…`);
- the configs and logs;
- the graphics, pose, upstream and clock work.

Backups: `build/claude-scratch/backup-pre-claude-session53-2026-11-01/`.

## What the game's own code says about its speed (source, established)

**rac1/pal has two sets of timing constants**, set together:
- `func_002157C0` (`game/mobyutil.c`) and `func_L00_002623D0` (level copy) set
  `D_0015EE80` (the 50 Hz flag), `D_0015EE60` (per-tick factor), `D_0015EE68` (count scale) and
  `D_0015EE6C` (seconds a tick).
- **60 Hz:** factor 1.0, scale 1.0, 1/60 s.
- **50 Hz:** factor 1.2, scale 0.8333, 1/50 s. That is per-tick motion ×1.2 because there are fewer
  updates a second.

**Where the mode is chosen:**
- `initonce.c`: `D_0015EE80` = disc region (PAL → 1), video mode 3.
- **Title loop** (`bmain.c:310`): when `D_0015EE80` differs from the option byte `D_0016044C`, the
  flag takes the option, the video mode becomes 3 or 2, and `func_00121B78` runs.
- **The constants follow the flag only at the next level start** (`transition.c:216`,
  `loaders_00240398.c:612`).

**The port forces the 60 Hz option** (`options_menu.cpp`, `fe->video_mode = 0`). Session 53's log
shows mode 0x3, then 0x2. So from the first level start after the title the constants are 60 Hz,
and 60 ticks a second is real time, as on NTSC or PAL60.

**Before that point** the constants can be the 50 Hz set (factor 1.2) while the port ticked at 60,
which is **1.2 times too fast**.
- Session 53 loaded level 99 (the boot program) at log line 33. That was while the flag was still
  the PAL region's 1, before the title switched to mode 0x2 at line 135.
- A level start with the flag set selects the 50 Hz constants. So the title and menus most likely
  ran on them until the next level start.
- The exe's `.data` defaults for these globals are not checked here.
- The new log lines show the actual time base and its changes. Run 1 decides.

## Root cause vs hypothesis

**Proven (source):**
- The port's fixed 60 ticks a second does not follow the game's time base. Wherever the constants
  are 50 Hz, the game runs 1.2× fast.
- `tick_rate()` was hard-coded to 60. The video-mode hook only logged.

**Not proven, about gameplay in session 53:**
- At 58–59 ticks a second, with the 60 Hz constants a level start sets, gameplay time was real time.
  The audio trace agrees: 300 ticks = 5.00 s against 5.10 s of audio, with a small constant offset.
- Every earlier stock run played at 30–45 ticks a second, so 50–75% speed. Real speed after that may
  feel fast. That is a hypothesis.
- The new log lines show the actual time base, so the run decides it.

**Also checked:**
- Catch-up is bounded: a tick up to 100 ms late is made up by starting the next at once; past that
  the schedule restarts.
- The render worker presents but does not advance the game.
- Undrawn ticks are not extra simulation.

No other speed-up path was found.

## Change: the tick rate follows the game's time base (default on; reference switch)

- **`openrac/game_host.h`:** new hook `openrac_game_tick_rate(float* seconds_per_tick)`. It returns 50
  or 60, or 0 when unknown. It is defined in `main.cpp` (CRLF).
- **`rac1/host/boot.c`:** installs it when the version defines `RAC1_TIME_BASE`.
  - `rac1-pal/host/rac1_version.h`: `0x0015EE6C`.
  - Reads 1/50 → 50 and 1/60 → 60; anything else (not yet set) → 0.
  - NTSC defines nothing and stays at 60.
- **`program/vsync.cpp`:**
  - `tick_rate()` returns `g_tick_rate`.
  - Before each tick is paced, `refresh_tick_rate()` takes the hook's 50 or 60, logs the change
    (`tick rate N a second (was M): the game's timing constants are for X s a tick … (frame F)`) and
    restarts the schedule from now, with no burst and no wait.
  - `OPENRAC_TICK_RATE_60=1` keeps 60 (reference).
- **`program/tick_pacing.h`** (new): the schedule as a function, `next_due()`, identical to the old
  rule when the rate does not change, plus the restart on a rate change. `vsync.cpp` uses it.
- **`program/sound.cpp`:** the audio-ticked buffer is sized per call, at the rate now.
- **`program/game_host.cpp`:** comments and the mode-3 log text.
- **Downstream users of `tick_rate()`:**
  - frontend pacing via `tick_period()`;
  - `movie.cpp`, fixed for the length of a movie;
  - the sound trace, which takes the rate each tick;
  - the hitch log.
  - The frontend's `set_tick` gets the period at the rate now.

**What changes for the player:**
- Boot, intro and title run at 50 ticks a second while the game's constants are 50 Hz, which is real
  speed.
- From the first level on, 60, as before.
- Gameplay speed is unchanged by this, because it was already on the 60 Hz constants.

## Clocks label (from the request)

`switch clocks` lines with no game request now say "not requested by the game (the rates are the
system's, or another clock manager's)", not "(stock)". Session 53's Horizon OC rates were not stock.
The game adds no writes and no caps.

## Performance

No new optimization in this pass, deliberately.
- The timing fix changes what 60 ticks means during boot, and it needs its own validation run.
  Codex asked for it to stay separate.
- Stock performance work needs a clean stock baseline (config 02).
- Kept: the pose allocation reuse (7116 poses, 0 differences) and the helper off by default.
- Candidates for the next pass, measured first:
  - the capture's per-tick VIF and GIF copies and allocations;
  - the per-pass GPU logs for heavy windows;
  - a transfer-only late tick (designed in the session-49 doc).

## Files

| File | Change |
|---|---|
| `port/game/common/include/openrac/game_host.h` | `openrac_game_tick_rate` |
| `port/game/common/main.cpp` (CRLF) | its definition |
| `port/game/rac1/host/boot.c` | `tick_rate()` hook under `RAC1_TIME_BASE` |
| `port/game/rac1-pal/host/rac1_version.h` | `RAC1_TIME_BASE 0x0015EE6Cu` |
| `port/game/common/program/vsync.cpp` | `g_tick_rate`, `refresh_tick_rate`, `OPENRAC_TICK_RATE_60`, `tick_pacing::next_due` |
| `port/game/common/program/tick_pacing.h` | new |
| `port/game/common/program/sound.cpp` | buffer per call |
| `port/game/common/program/game_host.cpp` | comments and log text |
| `port/platform/switch/clocks.cpp` | label |
| `port/tests/tick_pacing_test.cpp` | new |
| `port/CMakeLists.txt` | `openrac_add_test(tick_pacing INCLUDES .)` |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | `OPENRAC_TICK_RATE_60` |

## Tests (Linux container)

- **`tick_pacing_test`:** g++/clang `-Wconversion` and ASan/UBSan, exit 0. It covers:
  - 600 ticks at 60 = 10 s;
  - 500 at 50 = 10 s;
  - 50 → 60 with no gap under 15 ms and a 10 s total;
  - late ticks inside the slack caught up on average;
  - a 500 ms stall with no burst after;
  - a slow machine runs slow, never fast.
- **`vsync.cpp`, `sound.cpp`, `game_host.cpp`:** `-fsyntax-only -Wall -Wextra -DOPENRAC_FRONTEND`,
  gcc and clang, clean.
- **`boot.c` (PAL `rac1_version.h`, generated `game_protos.h`):** gcc and clang `-fsyntax-only`,
  clean.
- **`switch_clocks_mock_test`:** exit 0 after the label change. `clocks.cpp` against the real libnx
  headers: clean.
- **`env_test`:** exit 0.

**Not covered:**
- the AArch64 build;
- the Windows tests;
- the generated-code side of `RAC1_TIME_BASE` (none: plain host C);
- hardware timing.

## Hardware (NEW build; `build/deko3d-release/session53-tests/`)

| Run | Config | Check |
|---|---|---|
| 1 | `01-speed-check` (any clocks) | From boot: `tick rate 50 a second` at boot or title if the constants are 50 Hz, then `tick rate 60 … (frame F)` at the first level; intro and title at normal speed; gameplay speed as before; audio trace offsets steady |
| 2 | `02-stock-benchmark` (stock clocks, same route as 47–49, no verify or owner trace) | A clean stock baseline: tick phases, pose µs a piece, GPU pass timing |

If gameplay still feels fast in run 1 while the log shows 60 with the 60 Hz constants, record what
looks fast: walking, animation, the camera, a cutscene, or a timer. That would be a new lead outside
the time base.
