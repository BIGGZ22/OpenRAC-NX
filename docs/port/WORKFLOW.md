# Testing and fixing the native port: the whole workflow

How a bug a player reports becomes a fix in a release: taking the report,
reproducing it without playing, finding the cause, choosing where the fix
belongs, checking it, and landing it. It is the method the port's fixes
have followed so far, written for anyone who wants to help, person or AI
agent.

The details live in the guides this one links:

| Guide | For |
|---|---|
| [RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md) | Running the game by script: starting a level, teleporting Ratchet, pressing buttons, screenshots, memory watches, the routes to every menu |
| [DEBUGGING.md](DEBUGGING.md) | The debug overlay and every `OPENRAC_*` switch |
| [RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md) | Build, the pieces of a level's code, measuring a gameplay bug, backfilling C for a function |
| [RENDERER.md](RENDERER.md) | How the game builds a frame and what each renderer draws |
| [../LAYOUT.md](../LAYOUT.md) | Where things go, and the direction for the port's per-version files |
| [../../launcher/docs/RELEASING.md](../../launcher/docs/RELEASING.md) | Cutting a release |

## 1. The loop

1. **Take the report** (section 2): which disc, which build, which screen,
   what happens.
2. **Reproduce it as a script** (section 3): a run that reaches the spot
   and shows the bug in a screenshot or a number.
3. **Find the cause** (section 4): measure first, read code second.
4. **Fix it where it belongs** (section 5): the game's C, the port's
   per-version files, the shared engine, or the level export.
5. **Check it** (section 6): the same script shows the bug gone, a second
   case still works, the tests pass.
6. **Land it** (section 7): a small commit that says what was checked; a
   release when the player asks for one.

Each step leaves something the next person can rerun: the script, the
number, the commit message.

## 2. Taking a report

Players describe what they see, not what broke. Before reproducing, know:

- **The disc and version.** PAL (SCES-50916, v2.00) plays; NTSC-U
  (SCUS-97199) is being brought up. The launcher's game page names the
  version it set up.
- **The build.** The launcher's version is in its header. A launcher may run
  a bundled copy rather than your checkout's build: when a fix "did not go
  through", check which executable ran.
- **Where.** The level, what Ratchet was doing, the menu page. A screenshot
  settles most questions; players can drop one in the repository root or
  send it.
- **Their save.** A memory card takes other paths through the menus than
  `--no-card`. Their cards are in `<data home>/OpenRAC/memcard/<serial>`:
  copy the folder to a scratch place and pass `--cards <copy>/<serial>`.
  Never run against the original.
- **Whether the game does it too.** Some reports describe how the game
  always behaved. The HUD is one: in play it stays hidden and shows only
  for a moment when it has news (Ratchet takes damage, picks up bolts,
  talks to a vendor, opens the weapon menu) or when the player opens and
  closes the pause menu (R3 opens the map, not the HUD). A hidden HUD is
  not a bug; one that stays hidden through those is.
- **The crash report or the log.** Ask for the launcher's report first:
  on the game's page, after a crash, **Copy report** gives a short text
  (what happened, where, the versions, the system and graphics card, the
  top of the backtrace) and **Save report** a .zip in the player's
  Downloads folder (`OpenRAC-crash-<date>_<time>.zip`: `summary.txt`, the
  crash report, that session's log and the launcher's `launcher.log`).
  The same is under **Logs and crash reports** on the game's page for any
  earlier session, crash or not; players post it in a
  [GitHub issue](https://github.com/OpenRAC/OpenRAC/issues) or on the
  Discord. The home folder reads `~` in all of it. Where the files are,
  in the data folder (`<install>/active/<game>/data`):
  - `openrac-<id>.log`: the latest session, one run only, starting with a
    header (OpenRAC's version and commit, the game and disc, the system,
    processor and memory, the command line); `openrac-<id>.1.log` to
    `.5.log` are the five runs before it. A session's log takes no more
    `[info]` lines past 32 MB.
  - `crashes/crash-<date>_<time>.txt`: one per crash (a fault, a fatal
    error, the game's own trap, an uncaught exception): the reason, the
    native backtrace (with the program's load address, for `atos -l` or
    `addr2line`), the level, the game mode word, the frame and the
    hero's position, the log's last 200 lines and the session header.
    The reason, backtrace and state are also at the end of the session's
    log. `[error]` and `[warning]` lines often name the cause (section 4).
  - For a bug that does not crash, ask the player to turn on **Detailed
    logs** (the game's page or Settings; `OPENRAC_LOG=verbose`, or
    `verbose_log = 1` in the settings file), play to the spot again and
    send that session: it adds level program loads, the controller's
    changes, who called a function with no C, and repeated calls to
    unknown code addresses. Check a report path with
    `OPENRAC_CRASH_TEST=segv:120` ([DEBUGGING.md](DEBUGGING.md#logs-and-traces)).

## 3. Reproducing it by script

Follow [RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md). The short version, from
`games/rac1`:

```sh
D="$HOME/Library/Application Support/dev.openrac.launcher/active/rac1/data"
SDL_AUDIODRIVER=dummy OPENRAC_UNCAPPED=1 OPENRAC_NO_BLEND=1 OPENRAC_DIRECT=1 \
OPENRAC_PRESS=100:4000:5 OPENRAC_SHOT=50:/tmp/run/f OPENRAC_SHOT_FROM=2000 \
  ../../port/build/release/openrac-rac1-pal --data "$D" --levels ../../assets/port/rac1/pal \
  --window --no-card --level 1 --skip-movies --frames 2400 > run.log 2>&1
```

What has paid off every time:

- **A helper for runs.** A small shell script that takes a name, a level, a
  frame count and a press list and writes the frames to a folder of that
  name saves retyping the line above dozens of times a session.
- **A contact sheet.** Ten screenshots read one by one hide the pattern; a
  grid of them (a few lines of Pillow, 640 x 480 tiles with the frame
  number on each) shows a whole menu walk or a fade at once. Crop and
  enlarge the part under test before judging a pixel-level bug.
- **Numbers for changes over time.** For a flicker, print each frame's mean
  brightness: `68, 53, 0, 0, 0, 76, 26` found a bright frame between black
  ones that no single screenshot showed.
- **Fresh levels when the export changed.** After a format bump the port
  reads only the new format and draws a black world from old exports. Export
  to a scratch folder under `build/` (`editor/extract.py port <disc>
  build/<name> --level N`) and delete it when done.
- **The game's state, set directly.** Teleport Ratchet, set bolts
  (`0x15EE98`), own items (`0x13D4C0 + id`, one byte each) with
  `OPENRAC_WRITE`. Some lists are built when a level or menu opens, so a
  write after that point may not show; buying an item at a vendor gives it
  as the game does.
- **One game at a time.** Several at once have run the machine out of memory;
  kill what a crashed run leaves (`pgrep -fl openrac-rac1`).

## 4. Finding the cause

Measure before reading code. In rough order of how often each found the
answer:

1. **The log's own words.** `X has no C in the port yet; it does nothing`
   (the function's C is missing; the count says how hot it is), `call to code
   address ..., which no function of the port has` (a pointer into code the
   port has not bound), `two functions at ...; keeping the first` (two names
   at one address, and the one kept may be the stub).
2. **hostgen's report**, `port/build/release/games/<id>/gen/report.md`: stubs,
   functions without C, and *calls short of arguments, filled with 0*. On the
   console a register still held the caller's value; in C the callee gets 0.
   That one bug made the US title menu show the Bomb Glove's tip for every
   entry.
3. **Hide and show.** `OPENRAC_HIDE_PASSES` says which renderer draws a wrong
   thing; `OPENRAC_SHOW_MISSING=1` paints textures the 2D path cannot find in
   magenta.
4. **Dumps.** `OPENRAC_DUMP_DRAWS=N` logs the 2D path's draws (sprites with
   their texel corners and filters, primitives with ALPHA and FIX),
   `OPENRAC_FRAME_LOG` each tick's draw bits and where the 2D list is split
   around the world, `OPENRAC_DUMP_TEXTURE=BLOCK` a GS block's texture,
   `OPENRAC_MOBY_LOG` the mobys near Ratchet, `OPENRAC_WATCH` any memory.
5. **A temporary probe.** When nothing above reaches the place, add a few
   lines that write what you need (a render target read back to a file, a
   value per call) and mark them `TEMP probe`. Remove every one before
   committing (`grep -rn "TEMP probe"`).
6. **What the game means to do.** ReRAC (`~/Projects/RAC/rerac`, ISC
   licence) is a native rewrite of the US game that documents nearly every
   system at US addresses: `docs/plan/*.md` for behaviour, `crates/` for
   code. `build-sn/fill/RERAC_XREF.tsv` in rac1-decomp maps PAL functions to
   it. Look there before reverse-engineering anything again; adapt its code
   with its notice where it helps.
7. **The decompilations.** PAL's names are in rac1-decomp's
   `include/names.h`; Lombyte's in `games/rac1/ntsc/config`. A level function
   has a copy in every level: `config/overlays/functions.tsv` lists where.
8. **The other version.** `port/tools/versionmap` pairs PAL and NTSC-U
   functions (`same`, `near`) and data; when one version works, watch the
   same thing in both and take the first frame where they differ.

### Kinds of cause met so far

From the week of 2026-10-16, each found by the steps above. The bug-fix
guide's own list covers gameplay code.

| What the player saw | The cause | How it showed |
|---|---|---|
| The vendor's screens black | A draw that reads a render target has texture handle 0, the placeholder's, and the "texture not uploaded yet" guard dropped it | The target read back held the panels; the monitors' draws had their writes masked |
| Screens under the vendor's monitors | The store's frame has no VU1 program, so all of its 2D was taken as coming before the world | `OPENRAC_FRAME_LOG`: "before world" equal to the whole list |
| No HUD on Kerwan, the HUD and sea flickering on Rilgar | The split at the first VU1 program: in the port that is the world text's sprite pass, absent while there is no world text. The terrain pass now puts a world marker in the list | `OPENRAC_FRAME_LOG`: "before world" equal to the whole list in play, PAL and US alike |
| The split point never found | The packet scan read a non-GIF word as a huge image and skipped the rest of the frame | Logging each DIRECT the scan saw |
| A flash on opening the store | The fly-in's first tick draws the world with an empty 2D list, so it showed unfaded between black ticks | Mean brightness per frame |
| Gaps around 2D panels | The half-pixel shift was half a *game* pixel, 2.5 screen pixels at 2560 across | Cropping the edge at full resolution |
| 3D items missing from menus | Mobys the game renders into a texture (vendor screens, pause previews) were drawn in the world or not at all | Their camera's zoom: 1.0 for a texture, 0.63 for the frame |
| The Pyrocitor and Blaster invisible | Their bind pose has a joint with zero scale; the export rejected the whole class | The manifest listed them with no mesh |
| Clank in front of Ratchet in a preview | Depth was cleared at each moby marker into the same panel | Two markers, one per moby |
| Ratchet's arms down with a gun | The pose layers on a moby's `+0x60` list were never applied | Reading the list: three nodes at weight 1 |
| The US menu showing help text | A call short of its argument (section 4, item 2) | The report listed it |
| The US level black | The animation advance has no C in the US decomp, and the stub won the address | "has no C", 215,018 calls |

## 5. Fixing it where it belongs

Where a change goes matters as much as the change ([LAYOUT.md](../LAYOUT.md)):

| The fault is in | Fix it in |
|---|---|
| The game's own C doing something only a PC trips on (an argument passed in a register, a call through an address) | The C in `games/<game>/<version>/src`, keeping the PS2 build byte-identical. In a tree OpenRAC follows from another project (the NTSC decomp), list the change in that tree's `OPENRAC_PATCHES.md` and offer it upstream |
| A function a decompilation has only as assembly | `port/game/<id>/hand/` (and `hand.tsv`), C carried from the other version where `versionmap` says `same` |
| Something the console's hardware or libraries did | The engine: `port/runtime`, `port/renderer`, `port/viewer`, `port/game/common`, shared by every game and version; data-driven per game where they differ, never forked |
| A routine the port must answer itself (drawing, a hook) | A `wrap` or `host` row in `port/game/<id>/libraries.tsv` with its host C in `port/game/<id>/host/` |
| Two routines the catalogue folded into one | `port/game/<id>/split_places.tsv` |
| What the level export writes | `editor/` (the port's levels come from `editor/extract.py port`). A change the port relies on bumps the export format: `editor/port.py FORMAT`, `port/viewer/level.h kManifestFormat`, the `verified-<format>.json` stamp in `launcher/actions.json` and `tools/bundle_standalone.py`, and the test fixtures, in one commit. Players' levels are then read again once |
| What the launcher runs | `launcher/actions.json` (`tools/bundle_standalone.py` derives an installer's actions from it) |

`port/game/<id>/` is meant to shrink: prefer the game's C when the fix is
to the game's code. Keep the rules that hold everywhere: no emulation, no
material [SOURCING.md](../policy/SOURCING.md) forbids, no game data in a
commit.

## 6. Checking it

1. **The same script** shows the bug gone, with the number or picture that
   showed it before.
2. **A second case** where the same code runs: another level, another menu
   page, the other version. A fix to shared code is smoke-tested on PAL
   (`tools/build_port.py rac1-pal`, a short run with `--level 1`) even when it
   was for NTSC-U.
3. **Nothing near it moved**: the other pages, Veldin's start, the HUD.
4. **The tests** of what you touched:
   - the port: `cmake --build --preset release && ctest --preset release` in `port/`;
   - after changing a game's C, its hostgen tables or its hand files: delete
     `port/build/release/games/<id>/gen/hostgen.stamp` first, or the old
     translation is kept;
   - Python: `python3 -m unittest discover -s editor` and `-s tools`, also on
     Python 3.12 (`uv run --python 3.12 --no-project python -m unittest
     discover -s editor`), which CI and the installers use;
   - the launcher: `yarn verify`, `cargo test -p openrac-launcher-core` in
     `launcher/`.
5. **Probes removed, scratch exports and processes cleaned up.**

## 7. Landing it

- **Commit small and say what you checked.** Conventional Commits
  (`fix(port): ...`, `feat(editor): ...`, `fix(rac1/ntsc): ...`); the body
  says what the player saw, the cause, the change, and the measurement
  that shows it fixed. Regenerated output gets its own commit. Stage only
  your own files when others work in the same tree.
- **Push and release only when asked.** Local commits are the default.
  Releases follow [RELEASING.md](../../launcher/docs/RELEASING.md): the
  version bumped in four files, a `launcher-vX.Y.Z` tag, the CI's draft
  checked, then published with notes written for players.
- **Before a sync or merge from another project**, run the removal-request
  check ([AGENTS.md](../../AGENTS.md), rule 2) on what came in.

## 8. Working with several people or agents

Fixes that do not touch the same files can run at once; the port's shared
build folder cannot. What has worked:

- Give each person or agent its own area (`editor/` and `launcher/`;
  `port/game/rac1-ntsc/`; the shared engine) and name the files it must not
  touch.
- Point each at this document, the guides above, and the exact command
  lines, data folders and levels to use.
- Ask for a report: commits, what was checked, what is left, what needs a
  decision.
- One build at a time in `port/build/release`; check for a running `ninja`
  or game first.
