# Claude — Switch clock control and clock logging (2026-11-01)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

This adds to the session-49 pass. That pass's `frame.cpp` owner-trace change is on the PC and is
not touched here.

Backups: `build/claude-scratch/backup-pre-claude-session50-2026-11-01/`.

## Why

- **Every performance figure so far ran at stock clocks** (CPU 1020 MHz). The user confirmed this,
  but no log recorded it.
- **The user wants the option of higher clocks in the game, as SwitchWakerHD has.** Two points from
  its own notes shaped this design:
  - it used clkrst for CPU steps 1020–1785 and GPU 614;
  - it removed the CPU option after "an Erista did not wake from sleep" at a raised CPU clock, and
    now leaves clocks to sys-clk.

## What it does

### Always: clock logging

`platform/switch/clocks.cpp` reads the clocks through clkrst (system 8.0.0 and later) or pcv
(earlier), the services sys-clk uses. It logs them at start and once a minute:

```
switch clocks (start): CPU 1020.0 MHz, GPU 768.0 MHz, memory 1600.0 MHz; docked; as the system has them
```

From the next build, every log states what the run ran at.

### Only when env.txt asks: setting the clocks

| Setting | Allowed values (MHz) |
|---|---|
| `OPENRAC_SWITCH_CPU_MHZ` | 1020, 1224, 1326, 1428, 1581, 1683, 1785 |
| `OPENRAC_SWITCH_GPU_MHZ` | 307, 384, 460, 537, 614, 691, 768 |
| `OPENRAC_SWITCH_MEM_MHZ` | 1331, 1600 |

**Limits** (`clock_choice.h`, host-tested):
- Only Nintendo's own clock steps are used.
- A value between steps takes the step below.
- Nothing above each official maximum: CPU 1785 (boost mode), GPU 768 docked and **460.8
  handheld**, memory 1600.
- Asking for more, for example 2295 or 1267, is held at the maximum. Going past these needs the
  overclock suites and their risks; the game will not do it.

**When the clocks are set and put back:**
- **Set** only while the game has the focus.
- **Put back** to what the system had whenever the focus goes. That covers HOME, **sleep**
  (libnx: `AppletFocusState_Background` means "HOME menu open / console is sleeping") and a library
  applet.
  - This answers SwitchWakerHD's Erista sleep failure: the console never sleeps at a clock the game
    set.
  - Caveat: the put-back runs in the applet hook from the game thread's event pump. If that thread
    stalls when sleep is requested, the put-back waits too.
- **Docked or undocked:** put back, then set again for the new mode, because the GPU ceiling
  changes.
- **On exit request, at the platform's end, and in `atexit`:** put back. The L+R+Minus exit goes
  through `std::exit`, which `atexit` covers.
- **When the process is killed outright** (closed from HOME): the focus loss came first, so the
  clocks were already put back.

**sys-clk:** sys-clk may set clocks too. If a clock the game set has moved, the minute log warns
("something else (sys-clk?) sets it too"). Use one or the other.

**Threads:**
- The hook runs inside `appletMainLoop`, in the event pump (`poll_event` in
  `platform/switch/graphics.cpp`).
- The minute log is called from the same place.
- All the clock state is on that one thread. `Window::swap`, which the render worker may call, is
  not used for this.

## Files

| File | Change |
|---|---|
| `port/platform/switch/clock_choice.h` | new: the steps and caps, constexpr |
| `port/platform/switch/clocks.h`, `clocks.cpp` | new: logging, opt-in setting, focus/mode/exit handling |
| `port/platform/switch/graphics.cpp` | `Platform` start/stop the clocks; `poll_event` calls `switch_clocks::tick()` after `appletMainLoop` |
| `port/cmake/Window.cmake` | `platform/switch/clocks.cpp` in the Switch `openrac_platform` |
| `port/CMakeLists.txt` | `openrac_add_test(switch_clock_choice INCLUDES .)` |
| `port/tests/switch_clock_choice_test.cpp` | new |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | the three settings |

## Tests (Linux container)

- **`switch_clock_choice_test`:** g++ and clang `-Wall -Wextra -Wconversion`, plus ASan/UBSan, all
  exit 0. It covers:
  - nothing asked;
  - the official maximum;
  - above-maximum asks held;
  - the handheld GPU cap;
  - between-step values;
  - too-low values left alone;
  - every step reachable by its MHz figure;
  - a constexpr check.
- **`clocks.cpp`:** `-fsyntax-only -Wall -Wextra -D__SWITCH__` against the real libnx headers (pcv,
  clkrst, applet, hosversion), gcc and clang, clean.
- **`env_test`:** exit 0.
- **Not run:**
  - **`graphics.cpp`:** not compiled here; the edit is three lines. Codex: compile check.
  - **Any clock call on hardware:** service access, rates and sleep behaviour are hardware-only.

## Hardware plan (new build; `build/deko3d-release/session50-tests/`, docked, sys-clk off)

| Run | Config | Check |
|---|---|---|
| 1 | `01-clocks-stock-logged` (session-49 `01-serial-clean` + owner trace) | `switch clocks` lines show stock (CPU 1020, GPU 768 docked, memory 1600); the corrected `image owner` lines; clean figures |
| 2 | `02-clocks-official-max` (01 + CPU 1785 / GPU 768 / memory 1600) | `set` line; ticks a second, game ms, prepare, capture and GPU spans against run 1; **press HOME and come back once, sleep and wake once**: the log must show "put back" and "set" each time, and the console must wake normally |

The session-49 runs (`01-serial-clean`, `02-helper-verify`) can still go first on the current NRO.

**Expected:** game CPU time should drop roughly in proportion in CPU-bound windows (1785/1020 =
1.75× the clock). Memory-bound parts gain less. The GPU is unchanged docked (768 is already the
docked clock). Not a claim; run 2 measures it.
