# Gameplay regression runs

`port/tools/regress` plays every level of Ratchet & Clank, PAL and US, by
script in hidden windows, records what the game does tick by tick, and
compares that with another build, with the other version or with a saved
baseline. It also lists what each run reached that the port has no right C
for yet. A full run of 19 levels takes about a minute per version (six
levels at once on an 18-core Mac). The runs use the port's own scripting
switches ([RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md)); nothing emulates the
console.

Everything it writes stays under `build/regress/` (ignored by git): runs in
`runs/`, baselines in `baselines/`. Never commit them.

## Running

From the repository root, with the port built
(`cd port && cmake --preset release && cmake --build --preset release`) and
Homebrew's Python on macOS:

```sh
python3 port/tools/regress/regress.py run --game rac1-pal              # all 19 levels
python3 port/tools/regress/regress.py run --game rac1-ntsc --levels 1,3,5-7
python3 port/tools/regress/regress.py run --game rac1-ntsc --build ../other/port/build/release --out build/regress/runs/other
```

- `--data` and `--level-data` default to the checkout's `build/data/pal` and
  `assets/port/rac1/pal` (PAL) or `build/ntsc-data` and `build/ntsc-levels`
  (US); in a worktree, the main checkout's.
- `--jobs N` runs N levels at once (default 6).
- A run without `--out` becomes `latest:<game>`, the default of `baseline
  save` and `check`; one with `--out` (another build's) is kept apart.

Each level runs with `OPENRAC_HIDDEN=1` (a window never shown),
`SDL_AUDIODRIVER=dummy`, `OPENRAC_UNCAPPED=1`, `OPENRAC_AUDIO_TICKED=1`,
`--no-card`, `--skip-movies` and a settings file of its own (the defaults,
whatever the player chose). The run prints one line per level and writes
`RUN/run.json` with each level's outcome:

| Outcome | |
|---|---|
| `ok` | the route played to its end |
| `no-control` | the run ended (`max_frames`) before the player had control |
| `crash` | the process stopped on a fault or an error code; `notes` has the crash report's reason and the game functions on top of its backtrace |
| `hang` | no end before the route's `timeout` (seconds); the process is stopped |
| `exit` | it ended on its own between control and the route's end |

`RUN/LL/` keeps level LL's `trace.jsonl`, `reach.json`, `run.log` and
settings. A crash report also goes to the data folder's `crashes/`, as for
any run.

### Repeatable runs

`OPENRAC_AUDIO_TICKED=1` makes the game's sounds advance by one tick's
samples each tick instead of at the sound device's pace, and reads streams
when they start: the game reads its sounds' state, and an uncapped run with
a device parted from itself on several levels (moby counts, then the hero).
With it, two runs of one build are the same tick for tick on every level
that reaches control. A level that still parts between runs of one build is
itself a finding.

## Comparing

```sh
python3 port/tools/regress/regress.py compare latest:rac1-pal latest:rac1-ntsc
python3 port/tools/regress/regress.py compare build/regress/runs/old latest:rac1-ntsc --fields cam,fw
```

Ticks are paired by `c`, the frames since control, so two builds and the two
versions (which reach control at other window frames) are read at the same
moments of the route. Per level it prints the first tick and field that
differ, the other fields that differ at that tick, and the largest gap of
each field over the whole route, which tells a drift from a broken routine:

```
level 14 Oltanis         diverges at c=47 (frames 510/570): cam [252.593, 149.646, 48.156] vs [252.561, 149.590, 48.156]; largest gap cam 14.451, fw 1.094
```

| Field | | Tolerance |
|---|---|---|
| `lv`, `m` | the level (-1 the boot program), the game mode (0 play, 2 scene, 3 pause, 6 loading or the flight) | exact |
| `cam`, `fw` | the camera the world was drawn from: position, forward row | `position` 0.05, `direction` 0.01 |
| `hp`, `hr`, `hs` | the hero: position, rotation (z the yaw), state id | `position`, `angle` 0.01 (wrapping at a turn), exact |
| `mb`, `dr` | the live mobys, the layers drawn (1 sky, 2 terrain, 4 ties, 8 shrubs, 16 mobys) | exact |

`--fields` picks fields, `--tol position=0.2` changes a tolerance. A level
without control on one side is "not compared". The exit code is 1 when
anything diverges.

## Baselines

```sh
python3 port/tools/regress/regress.py run --game rac1-pal
python3 port/tools/regress/regress.py run --game rac1-ntsc
python3 port/tools/regress/regress.py baseline save master          # the latest run of each version
# ... change the port, build, run both again ...
python3 port/tools/regress/regress.py check master
```

`baseline save NAME RUN...` takes runs by folder too; a second run of the
same version (the same build again) is kept as its twin (`<game>~2`), and
the levels where the two part are written to `noise.json`. `check` shows a
divergence on such a level as noise and does not fail on it. It fails on any
other divergence and on a level whose outcome changed (ok to crash).

## What a run reached

```sh
python3 port/tools/regress/regress.py reach latest:rac1-pal latest:rac1-ntsc [--top 25] [--json FILE]
```

Every stub hostgen writes and every call it had to fill an argument of with 0
counts its calls in a site of its own (`openrac_reach_site`,
[guest.h](../../port/runtime/include/openrac/guest.h)); `OPENRAC_REACH=FILE`
writes the sites a run called. `reach` merges a run's levels into one list
per version, ranked by how many levels reached a function, then by its
calls:

| Kind | |
|---|---|
| `asm` | still assembly in the decompilation: its stub did nothing |
| `no-c` | declared, defined nowhere |
| `untranslated` | C that hostgen cannot write yet (its report says why) |
| `missing` | hand C's own report of a function without C |
| `zero-args` | the function made a call with an argument filled with 0 (the callee and argument follow the arrow): on the console that register held whatever was last there |

It is the order to backfill or fix in for gameplay
([BACKFILL.md](../workflow/BACKFILL.md)), next to hostgen's static frontier.

## Adding or changing a route

Routes are data, in [port/tools/regress/routes.json](../../port/tools/regress/routes.json):

- `defaults`: Cross through the boot screens (`boot`, window frames), Start
  every 30 frames while a scene or the flight can be skipped and control has
  not come (`skip`), the `sequence` played from control, its `length`,
  `max_frames` and `timeout`.
- `sequences`: steps with `at` and `hold` in frames from control, `buttons`
  (`cross`, `square`, `r1+square`, ...), `left` and `right` sticks (0 to 255,
  128 centred, 0 left or up; the left stick is relative to the camera), and a
  `what` for the reader.
- `levels`: each level's `name` and anything it changes: another `sequence`
  or its own `steps`, a longer `length`. Keys under `rac1-pal` or `rac1-ntsc`
  apply to that version only (keep the versions' routes the same unless a
  version needs otherwise, or they no longer compare).

A route becomes the port's switches: `OPENRAC_DIRECT=1` with `--level N`,
`OPENRAC_PRESS` (window-frame entries, `k` entries for the skips, `c`
entries counted from control) and `OPENRAC_CONTROL_FRAMES`, which ends the
run the route's length after control. Control is a level's game mode 0 held
for 30 ticks. Read a level's `trace.jsonl` to see what a step did (`hp`
moving, `hs` changing, `fw` turning), and change one route at a time:
existing baselines no longer compare with a changed route.

## The port's switches

| Switch | |
|---|---|
| `OPENRAC_TRACE=FILE` | one JSON line a tick: `f` window frame, `c` frames since control (null before), `lv`, `m`, `cam`, `fw`, `hp`, `hr`, `hs`, `mb`, `dr` (the table above). The addresses are each version's: the camera and mobys from `viewer::rac1_addresses`, the game mode the loaded level's copy (`openrac_game_mode`) or the boot program's (`openrac_front_end`), the hero from `RAC1_ADDR(0x0013F450)` in rac1's `boot.c` (US position 0x13F3D0) |
| `OPENRAC_REACH=FILE` | the reached sites as JSON, at exit and every 600 frames |
| `OPENRAC_PRESS` `cF:...`, `kP:...` | a press F frames from control; a press held the first frames of every P while Start can skip what plays, until control |
| `OPENRAC_CONTROL_FRAMES=N` | ends the run N frames after control |
| `OPENRAC_AUDIO_TICKED=1` | no sound device; sounds advance with the ticks |

## Tests

```sh
python3 -m unittest discover -s port/tools/regress    # comparing, merging reach files, routes, outcomes
python3 -m unittest discover -s port/tools/hostgen     # the reach sites in the generated C
```
