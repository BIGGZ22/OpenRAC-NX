# Claude — Switch clocks: raised profile, verified read-back, resets answered (2026-11-01)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

Not touched: the tested NROs (release `4b310f16…` / ELF `9a6ea68f…`, diagnostic `74fcab10…` /
ELF `0ffbf7e4…`), the configs and the logs.

Backups: `build/claude-scratch/backup-pre-claude-session51-2026-11-01/`. The `clocks.cpp` backup
there is the one containing Codex's session-50 fixes (`b672523a`).

## Session 51: what the log shows

| Line | When | Reads (MHz) | Context |
|---|---|---|---|
| 15 | start | CPU 1020, GPU 307.2, memory 1331.2 | handheld |
| 16 | request | CPU 1785, GPU 460.8, memory 1600 | handheld GPU cap |
| 17 | read back at once | CPU 1785, GPU 460.8, memory 1600 | as requested |
| 873 | the next read, at the first minute | CPU 1020, GPU 307.2, memory 1331.2 | stock again |

- There was no service error, no put-back line and no focus or mode hook firing.
- **The request was made at platform start**, before deko3d opened, before the first frame, and
  before the launcher-to-game load and the level loads.
- **Root cause, established:** the request worked and then something reset all three domains. The
  status line kept saying "set by the game", because it reported the request, not a read-back.
- **Hypotheses, not proven:**
  - The applet manager or APM applied the program's own performance configuration when the program
    began displaying or loading. All three domains moved together, as a performance configuration
    does.
  - An external clock manager.

  The port has no other clock setter. The first reset happened somewhere in the minute between
  lines 17 and 873, and that log cannot say when.

## Changes (`port/platform/switch/clocks.cpp` rewritten, keeping Codex's fixes)

### The raised profile, as the user asked

`OPENRAC_SWITCH_CLOCK_PROFILE=raised` requests **CPU 1785, GPU 768 and memory 1600 MHz in both
handheld and docked**. There is no 460.8 MHz handheld GPU cap for this profile
(`clock_choice::raised_profile()`).
- It is not called a retail profile anywhere: 1785 is the boost (loading) CPU clock, and GPU 768
  handheld is above every official handheld profile.
- Handheld, the start line says so, with a heat and battery note.
- **It still needs hardware validation.**
- The per-domain settings keep their official caps, including 460.8 handheld, and are ignored when
  the profile is on.

### Verified, never assumed

- **Separate facts per domain:** requested, last request accepted or REFUSED (with the result),
  read back as requested or not (or not readable), and put back pending to the baseline.
- **One domain failing to read does not hide the others.**
- Every request is followed by a logged read-back.
- **Context on every line:** focus, mode, the applet's current performance configuration (7.0.0+),
  and APM's performance mode and configuration.

### Answering resets

**Read-back schedule while in focus:** 0.25, 1, 3 and 10 s after each request, then every 5 s.

**When a read-back no longer matches:**
- It is logged with the seconds since the request and the configurations. At most 24 detailed lines
  a run; a count goes in each minute line.
- The request is made again, so the profile persists while in focus.

**An external override:**
- Three resets in a row within 5 s of their requests are reported as one, naming APM or a manager
  such as sys-clk as possibilities.
- After that, requests back off to every 30 s, or every 2 s with `OPENRAC_SWITCH_CLOCK_KEEP=1`
  (explicit ownership).
- Requests are **never made every frame**.

**A refused request:** logged with its result, module and description, then retried after 1 s,
doubling to 30 s.

**A domain whose rate cannot be read** before its first request is left alone until the focus
returns.

**A reported sys-clk boot flag** (`atmosphere/contents/00FF0000636C6BFF/flags/boot2.flag`) is logged
as a hint, not proof.

### Lifecycle

**Nothing is raised** while out of focus or after an exit request.

**Focus lost** (HOME, sleep, a library applet):
- Each owned domain is put back to the baseline it had before this program's first request. That
  baseline is kept across re-requests.
- A refused put-back stays owed and is retried while out of focus (Codex's fix, kept).
- The GPU is never put back above 460.8 when handheld (Codex's clamp, kept).

**Mode change:**
- A domain the system has already changed for the new mode is dropped, with no write.
- A domain still at our rate is put back.
- Baselines are forgotten, and the profile is requested again 500 ms later for the new mode.

**Exit request:** put back, and never raised again.

**`stop()`:**
- Three bounded put-back attempts without waiting, then a warning if any remain.
- Unhook, close the sessions, `apmExit`.
- `atexit` is registered once, for the `std::exit` path.
- A killed process or a blocked main thread cannot be helped.

## Files

| File | Change |
|---|---|
| `port/platform/switch/clocks.cpp` | rewritten as above (Codex's no-set-without-baseline, pending put-back retry and handheld GPU clamp kept) |
| `port/platform/switch/clocks.h` | contract comments |
| `port/platform/switch/clock_choice.h` | `raised_profile()` |
| `port/tests/switch_clock_choice_test.cpp` | raised-profile check |
| `port/tests/switch_clocks_mock_test.cpp` (new), `port/tests/switch_clocks_fake/switch.h` (new) | lifecycle over fake services and a fake time |
| `port/CMakeLists.txt` | `switch_clocks_mock` test (host only, `if(NOT SWITCH)`) |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | `OPENRAC_SWITCH_CLOCK_PROFILE`, `OPENRAC_SWITCH_CLOCK_KEEP`, updated per-domain rows |

`graphics.cpp` is unchanged: the hook and `tick()` stay in the event pump.

## Tests (Linux container)

**`switch_clocks_mock_test`:** g++ `-O1` with ASan/UBSan and `-Wall -Wextra -Wshadow`, exit 0,
0 failures. clang: `-fsyntax-only` clean (no clang sanitizer runtime here). It covers:
- no request: no writes in 3 minutes, no hook;
- raised profile handheld: 768 GPU, uncapped;
- a reset 2 s after the request answered, and one reset counted;
- holding for 60 s with no extra requests;
- focus lost: back to stock and nothing raised out of focus; focus back: raised again;
- an override resetting every change 100 ms later, for 2 minutes:
  - default: override reported, and between 4 and 9 requests;
  - `KEEP`: between 40 and 65 requests;
- per-domain GPU 768: 460.8 handheld, 768 docked after a dock, never above 460.8 on undock, back to
  307.2 at stop;
- a refused GPU request beside a set memory clock and an unreadable CPU: the CPU is untouched, and
  retries are bounded until the GPU is accepted;
- every put-back refused out of focus, then retried until it succeeds;
- an exit request: put back, never raised again;
- `stop` with refusals: bounded, and services closed;
- the pcv path (system before 8.0.0);
- starting out of focus.

**Other checks:**
- `switch_clock_choice_test`: g++ and clang `-Wconversion`, exit 0.
- `clocks.cpp`: `-fsyntax-only -Wall -Wextra -Wshadow -D__SWITCH__` against the real libnx headers
  (clkrst, pcv, apm, applet incl. `appletGetCurrentPerformanceConfiguration`, hosversion), gcc and
  clang, clean.
- `env_test`: exit 0.

**Not done:**
- the AArch64 build;
- the Windows tests;
- anything about Horizon: the mock is not the system.

## Hardware plan (`build/deko3d-release/session51-tests/`, NEW build, sys-clk off, same route; handheld is fine, note the mode)

| Run | Config | Check |
|---|---|---|
| 1 | `01-stock-logged` | `switch clocks` lines: stock read-backs and the configuration values at start, at each minute and through the loads |
| 2 | `02-raised-profile` | see below |

Run 2 checks:
- the requested line reads back 1785/768/1600;
- **when the first reset happens** (the `reset #1 found X s after request` line) and what the
  configurations say then;
- whether it is re-requested and holds afterwards, or is reported as an external override;
- HOME and back, and sleep and wake: put-back and re-request lines, and the console wakes normally;
- the L+R+Minus exit: a put-back line.

Two caveats:
- **Do not compare performance** until run 2's read-backs show the rates holding during gameplay.
- Watch handheld heat and battery at GPU 768.
