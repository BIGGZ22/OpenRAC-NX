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
anything diverges. Between a PAL and a US run, the fields a level's
`versions` (routes.json) lists are not compared from their tick on, and the
line says so; `--strict` compares them too.

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
  not come (`skip`; counted on window frames in the standard set, which
  keeps its baseline; `"from": "scene"` counts from the frame the scene
  became skippable, the same tick in both versions: see Route sets), the
  `sequence` played from control, its `length`, `max_frames` and `timeout`.
- `sequences`: steps with `at` and `hold` in frames from control, `buttons`
  (`cross`, `square`, `r1+square`, ...), `left` and `right` sticks (0 to 255,
  128 centred, 0 left or up; the left stick is relative to the camera), and a
  `what` for the reader.
- `levels`: each level's `name` and anything it changes: another `sequence`
  or its own `steps`, a longer `length`. Keys under `rac1-pal` or `rac1-ntsc`
  apply to that version only (keep the versions' routes the same unless a
  version needs otherwise, or they no longer compare).
- `versions` (under a level): what the two versions do differently there on
  the route, each `{"fields": [...], "from": c, "why": ...}`; `compare`
  leaves those fields out from tick `from` when one run is PAL and the other
  US ([RAC1_NTSC_HINTS.md](RAC1_NTSC_HINTS.md), section 7, says why each is
  there). Never for two runs of one version.

A route becomes the port's switches: `OPENRAC_DIRECT=1` with `--level N`,
`OPENRAC_PRESS` (window-frame entries, `k` entries for the skips, `c`
entries counted from control) and `OPENRAC_CONTROL_FRAMES`, which ends the
run the route's length after control. Control is a level's game mode 0 held
for 30 ticks. Read a level's `trace.jsonl` to see what a step did (`hp`
moving, `hs` changing, `fw` turning), and change one route at a time:
existing baselines no longer compare with a changed route.

## Route sets

The level routes above are the `standard` set, and its baseline stays
valid as long as they do not change. `routes.json` `sets` holds more sets,
each a table of named routes (`"11-swim": {"level": 11, ...}`), run, saved
and checked by name:

```sh
python3 port/tools/regress/regress.py sets                                # every set and its routes
python3 port/tools/regress/regress.py card --game rac1-pal                # the prepared card (below), once
python3 port/tools/regress/regress.py card --game rac1-ntsc
python3 port/tools/regress/regress.py run --game rac1-pal --set wide      # its 56 routes
python3 port/tools/regress/regress.py run --game rac1-ntsc --set wide --only 03-ship,title-load-2
python3 port/tools/regress/regress.py baseline save wide --set wide       # latest:<game>@wide of each version
python3 port/tools/regress/regress.py check wide --set wide
python3 port/tools/regress/regress.py compare latest:rac1-pal@wide latest:rac1-ntsc@wide
python3 port/tools/regress/regress.py reach latest:rac1-pal@wide latest:rac1-ntsc@wide --top 40
```

A set's run lives in `runs/<game>-<set>-<time>/`, one folder per route; its
latest run is `latest:<game>@<set>`. `--levels` keeps the routes on those
levels, `--only` the routes named. The two versions run at once with two
`run` commands (six routes each at once is what an 18-core Mac holds; a
route of the `wide` set's long kind takes two minutes or so, the whole set
about ten minutes per version).

A set has `defaults` over the standard ones, `sequences` and `lists` of its
own, and its routes' keys are the level routes' (`level`, `name`, `sequence`
or `steps`, `length`, `max_frames`, `timeout`, `versions`, a version's own
keys under its id) and:

| Key | |
|---|---|
| `start` | `direct` (the default: `OPENRAC_DIRECT` with `--level`) or `title`: from the boot screens, with the route's `boot` presses (window frames) through the title's menus |
| `until` | `control` (the default) or `frames`: a route that never gets control (the title's menus) plays to `max_frames` and is `ok` when it got there |
| `card` | `prepared`: the run plays on a fresh copy of the prepared card instead of `--no-card` |
| `points`, `lists` | named places (`[x, y, z]` or `[x, y, z, yaw]`) and lists for the steps |
| `then` | steps added to the sequence's (a death route's teleport) |

and the steps can do more than press. Each becomes `OPENRAC_WRITE` entries
counted from control, at the version's own addresses (`routes.json`
`addresses`, paired by the version map):

| Step key | |
|---|---|
| `teleport` | the hero to `[x, y, z]` (10 frames, `hold_place` to change it), and to the yaw when a fourth number is there (50 frames); or a point's name |
| `yaw` | the hero's yaw |
| `give` | a list of `items` (`all`): owned and acquired, the ammo full |
| `equip` | an item id into the hero's pending item (include/hero.h `pending_item[0]`): the game's own switch puts it in his hand |
| `health` | the hero's hit points (0 kills him) |
| `write` | `{"address": "hero+0x22A8" or "0x...", "value": ..., "frames": N}` |
| `each` | `{"at", "each": LIST, "every": N, "do": [steps]}`: the block once for every entry, `$` the entry |

A skip with `"from": "scene"` presses Start from the frame what plays became
skippable (`s` below), so a scene's skip falls on the same tick of it in
both versions; `"after_control": true` keeps skipping the scenes and flights
a route starts after control (`S`).

### The wide set

`wide` widens what a run reaches, for both versions alike:

| Routes | What they play |
|---|---|
| `LL-long`, one a level | every item given; the level's path (up to ten places from its own data, `points.py` below), at each a teleport, a walk, a jump, the wrench, Triangle and a thrown wrench; the first vendor (open, move, buy, leave); the pause menu and the map opened and closed; the quick select; then each weapon and gadget equipped and fired three times (the Suck Cannon last) |
| `01-pause-0` to `-6` | Novalis: the pause menu, then its page N opened, moved in and left; one route a page, so a page that stops the game leaves the others to run |
| `03-fall`, `09-fall`, `05-health`, `12-enemies`, `03-dogs`, `11-swim` | deaths and respawns through to control: a teleport below the level, the hit points to 0, one hit point among Hoven's troopers and Kerwan's robot dogs; Pokitaru's water |
| `00-clank`, `05-hoverboard-girl`, `05-bouncer`, `16-hoverboard`, `11-jet-fighter`, `13-qwark`, `07-snagglebeast`, `15-giant-clank`, `18-giant-clank`, `18-drek`, `06-clank`, `10-clank`, `12-turret`, `09-turret`, `17-turret`, `03-ship` | set pieces a direct start reaches: next to the moby that starts one, Triangle, then play on (Start skips the scenes they start). Orxon's Clank runs without items (the O2 mask would let Ratchet go) |
| `title-new`, `title-load-0` to `-4`, `title-options`, `title-idle` | the title: a new game (no card) into Veldin's routes, each prepared save loaded (Load Game, slot N) and played, the options pages, the title left alone |

`points.py LEVEL` prints a level's places from the extracted
`placements.json`: the hero's start, the respawn points (class 805, the
checkpoints), gold bolts, infobots, teleporters and vendors, nearest first
from the start, a vendor's point three units in front of it, facing it. The
`wide` routes hold what it printed; print them again after an export moves
things.

### The prepared card

`regress.py card --game G` copies a memory card (the player's,
`~/Library/Application Support/OpenRAC/memcard/<serial>` on macOS, only
read; `--from DIR` another) to `build/regress/cards/G/card` and rewrites the
copy's five slots from the first saved game on it: every weapon and gadget
owned with full ammo, Ultra Nanotech, the planets the ship reaches unlocked,
9,999,999 bolts, and slot 0 to 4 resuming on Rilgar, Orxon, Gemlik Base,
Hoven and Kalebo III (the save editor's planets: Veldin and Drek's Fleet are
reached by the story). It also marks the save notice seen (story flag
0x10): a level shows that notice once after Load Game, on every level but
the first Veldin, and holds the game in mode 4 until Cross, so a card whose first save
was made before it (on Veldin) would keep every loaded route from control.
`card.py` is the launcher's save format
(`launcher/core/src/save_edit.rs`) in Python: the checksums are written
again, so the game loads it. A route with `"card": "prepared"` plays on a
fresh copy of it in its run folder: the player's card is never touched.

## The port's switches

| Switch | |
|---|---|
| `OPENRAC_TRACE=FILE` | one JSON line a tick: `f` window frame, `c` frames since control (null before), `lv`, `m`, `cam`, `fw`, `hp`, `hr`, `hs`, `mb`, `dr` (the table above). The addresses are each version's: the camera and mobys from `viewer::rac1_addresses`, the game mode the loaded level's copy (`openrac_game_mode`) or the boot program's (`openrac_front_end`), the hero from `RAC1_ADDR(0x0013F450)` in rac1's `boot.c` (US position 0x13F3D0) |
| `OPENRAC_REACH=FILE` | the reached sites as JSON, at exit and every 600 frames |
| `OPENRAC_PRESS` `cF:...`, `kP:...`, `sP:...`, `SP:...` | a press F frames from control; a press held the first frames of every P while Start can skip what plays, until control: `k` on window frames, `s` from the frame what plays became skippable (the same tick of a scene in PAL and US, where window frames fall at other ticks of it); `S` as `s`, after control too |
| `OPENRAC_WRITE` `cF:ADDR:VALUE:N` | a word written F frames from control, for N frames |
| `OPENRAC_CONTROL_FRAMES=N` | ends the run N frames after control |
| `OPENRAC_AUDIO_TICKED=1` | no sound device; sounds advance with the ticks |

## Tests

```sh
python3 -m unittest discover -s port/tools/regress    # comparing, merging reach files, routes, sets, the card, outcomes
python3 -m unittest discover -s port/tools/hostgen     # the reach sites in the generated C
```
