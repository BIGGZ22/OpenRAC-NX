# Ratchet & Clank (PAL): fixing gameplay bugs and backfilling C

The scripted playtest procedures (timelines, places, camera, pictures) are in
[RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md).

A working guide for an agent picking up the native port of Ratchet & Clank
(PAL) on Windows: how to build and run it, how to find out why something
in the game misbehaves, how to supply C for a function that has none, and
what is known to be open. It complements
[RAC1_PAL_CAMPAIGN.md](RAC1_PAL_CAMPAIGN.md), which holds the rules of the
campaign and its dated log; read that file's first three sections for what
counts as a fix and how to commit. Written 2026-10-12.

## 1. The pieces

| Piece | Where | What it is |
|---|---|---|
| The decompilation | sibling checkout `../rac1-decomp` (branch `main`) | Matching C in `src/`, assembly for the rest, and C for the port only in `nonmatching/` |
| The in-tree copy | `games/rac1/` | A copy of the decompilation synced from the sibling. **This machine's build does not read it** |
| hostgen | `port/tools/hostgen/` | Reads the decompilation's C with Clang (`--target=mipsel-linux-gnu`) and writes host C in which a game pointer is a `gaddr` |
| The port's own C | `port/game/rac1-pal/hand/` (game functions, translated by hostgen) and `port/game/rac1/host/` (host C, compiled as it is, shared with NTSC-U: [port/game/rac1](../../port/game/rac1/README.md)) | Functions the decompilation has no usable C for, and the console's side of things |
| The runtime and frontend | `port/runtime/`, `port/game/common/`, `port/platform/`, `port/viewer/`, `port/renderer/` | Guest memory, the window, input, sound output, drawing |
| A reference port | sibling checkout `../rerac` | The US release rewritten in Rust (ISC). Useful for what a function is meant to do |

Which decompilation a build reads is `OPENRAC_RAC1_PAL_SOURCE` in
`port/build/release/CMakeCache.txt`. Here it is the sibling `rac1-decomp`.
A fix to decompilation C therefore goes into `../rac1-decomp` first, and the
same change into `games/rac1/` so the two do not drift; commit each in
its own repository with the same message.

Where a function's C comes from, later entries winning:

1. matching C in the decompilation's `src/`;
2. `nonmatching/functional.tsv` in the decompilation ("candidates": C that
   does what retail does without matching its bytes; 323 entries);
3. `port/game/rac1-pal/hand/hand.tsv` (the port's own C, translated like
   the rest; 95 entries);
4. `port/game/rac1-pal/hand/overrides.tsv` (the port's own C for a function
   `src/` matches, where the port has to do otherwise: the matched C with
   that change, in a `hand/matched_*.c` that says why, translated like the
   rest in place of the matched definition; `src/` keeps its match);
5. a definition in `port/game/rac1/host/*.c` (or `port/game/rac1-pal/host/*.c`) replaces the function
   outright with host C.

A port fix to a function `src/` matches goes into an override (4), or into
host C (5) when it is host work, never into `src/`: its byte match is the
proof of the decompilation and cannot be checked without the original
toolchain.

`port/build/release/games/rac1-pal/gen/report.md` (and `report.json`) is
written by every hostgen run: 4,026 functions translated, 944 without C,
3 stubs, 345 taken from candidates, and "the frontier", the 33 functions
without C that the program can reach from its entry points, nearest first.

## 2. Build and run

Build (finds Visual Studio, Clang and Ninja itself; a few minutes when
hostgen has to run again, seconds otherwise):

```
python tools/build_port.py rac1-pal
python tools/build_port.py rac1-pal --target runtime_test
```

The link fails while the game is running, because Windows locks the
executable. Close the game (and do not use the launcher's Rebuild at the
same time) before building. To force hostgen to run again, delete
`port/build/release/games/rac1-pal/gen/hostgen.stamp`.

Run level 0 (Veldin) directly, from `games/rac1`:

```
OPENRAC_DIRECT=1 OPENRAC_PRESS=100:4000:5 \
  ../../../port/build/release/openrac-rac1-pal.exe \
  --data "C:/Users/<user>/AppData/Local/dev.openrac.launcher/active/rac1/data" \
  --window --no-card --level 0 --skip-movies --frames 1500
```

The hero's update starts around frame 959 on this path. `--frames N` ends
the run by itself. `--settings FILE` names another settings file; one that
does not exist gives the defaults and leaves the player's own untouched.
Always pass `--no-card` so a test never writes a player's save.

Two courtesies to whoever is at the machine:

- **Sound.** Set `SDL_AUDIODRIVER=dummy` for every scripted run. A test
  that plays the level's music over the user's speakers is not harmless:
  it was mistaken for a bug in the game once already.
- **Speed and windows.** `OPENRAC_UNCAPPED=1` with `OPENRAC_NO_BLEND=1`
  runs one frame a tick as fast as the machine goes (1,500 frames in a few
  seconds). Each run still opens a window. For a long unattended series,
  set `config.hidden = true` temporarily in `frontend::open`
  (`port/game/common/frontend.cpp`), mark the line, and remove it before
  handing the build back.

Tests: `python -m unittest test_hostgen` from `port/tools/hostgen` (18 of
its tests skip on Windows), `runtime_test.exe` in the build folder. Three
failures on Windows are older than this work and unrelated: the launcher
core's `plans_the_extractor`, `tools/test_mips`'s
`test_reports_missing_and_differing_copies`, and the editor's
`test_files_manifest_and_placements`.

## 3. Development switches

All are environment variables read by the game.

| Switch | Effect |
|---|---|
| `OPENRAC_DIRECT=1` | Start in the level named by `--level` |
| `OPENRAC_PRESS=FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],...` | Scripted pad. Mask in hex: `8` Start, `4000` Cross, `8000` Square (the wrench). Sticks 0 to 255, 128 centred, 0 is left or up. The stick is relative to the camera, which turns as the hero moves, so it is poor for reaching a place |
| `OPENRAC_WRITE=FRAME:ADDR:VALUE[:FRAMES],...` | Writes a 32-bit word (hex) into game memory. For getting somewhere in a test only, never as a fix |
| `OPENRAC_WATCH=ADDR:COUNT[:i],...` | Logs COUNT floats (or ints with `:i`) from each address every frame, at most 32 per address |
| `OPENRAC_GUARD=FRAME:ADDR:BYTES[:SKIP]` | From FRAME on, the first write (after SKIP) into those bytes of game memory stops the program with the host stack of the writer: who corrupts a value. Linux x86-64 |
| `OPENRAC_MOBY_LOG=N[:RADIUS]` | Every N frames, one line per live moby within RADIUS of the hero: class, address, state, mode bits, animation sequence, frame and fraction, position, update function, pvars |
| `OPENRAC_HIT_LOG=1` | Each damage sphere and the classes it listed |
| `OPENRAC_SOUND_LOG=1` | Each command the game sends its sound driver, with the reply |
| `OPENRAC_DEBUG=1`, `OPENRAC_TRACE_MISSING=1` | More log; the host stack of the first call to each function without C |
| `OPENRAC_SHOT=N:PREFIX`, `OPENRAC_SHOT_FROM` | Screenshots |
| `OPENRAC_NO_BLEND=1`, `OPENRAC_UNCAPPED=1` | One frame a tick; no wait between frames |
| `SDL_AUDIODRIVER=disk` with `SDL_AUDIO_DISK_OUTPUT_FILE=FILE` | Writes what would be played to FILE as raw 16-bit stereo at 48 kHz, in silence, for measuring levels |

Addresses worth knowing (PAL):

| Address | What |
|---|---|
| `0x13F450` | The hero (`struct Hero`, `include/hero.h` in the decompilation, which names it by its US address: PAL is 0x100 higher). `+0x80` position, `+0x90` rotation (z is the yaw), `+0x198` state timer, `+0xA90` animation speed, `+0xA94` animation rate, `+0x2080` its moby, `+0x2084` state id |
| `0x15EE98` | The bolt count |
| `0x16007C` | Free mobys left (level 0) |
| moby `+0x10` position, `+0x20` state, `+0x50..0x53` animation frame, next frame, sequence, next sequence, `+0x54` fraction, `+0x74` update, `+0x78` pvars, `+0xA6` class | |

Putting the hero beside something, facing it, and swinging the wrench:

```
# floats as hex words: python -c "import struct;print('%x'%struct.unpack('<I',struct.pack('<f',141.4))[0])"
OPENRAC_WRITE="1000:13f4d0:<x>:2,1000:13f4d4:<y>:2,1000:13f4d8:<z>:2,1000:13f4e8:<yaw>:50"
OPENRAC_PRESS="100:4000:5,1060:8000:3,1105:8000:3,1150:8000:3"
```

In level 0, crates (class 500) stand at about (141.9, 98.4, 32.1) and the
first creatures (class 749) around (156.5, 122.5, 29.5). Find others with
`OPENRAC_MOBY_LOG=1000:100000` and a script that keeps the classes whose
position or animation changes between samples.

## 4. Finding the cause of a gameplay bug

Measure before reading code. Each of the bugs below looked like something
else until it was a number.

1. **Reproduce it as a measurement** in a scripted run: a count of ticks
   with no movement, a bolt count, a moby's position per tick. Make the
   check a script that prints one line, so it can be run again after every
   change and by a bisect.
2. **Compare with a build that works**, if one exists. Watch the same
   addresses in both and take the first frame where they differ
   (`diff` of two `OPENRAC_WATCH` logs). The first difference is usually
   one event, and it names the subsystem.
3. **Bisect when a broad change caused it.** For a hostgen change that
   touches many functions, add a temporary filter that leaves out the
   functions named in a file, and halve the list with a script that
   rebuilds and runs the check each round (about eight rounds for 150
   functions, three to four minutes each). Remove the filter afterwards.
4. **Then read the code**, starting from the function the evidence names.
   `rerac` says what a function is for; the decompilation says what PAL
   does.

Be suspicious of a check that depends on the game's random numbers. Sounds
draw from the same generator as everything else, so any change to which
sounds play moves every later random event. A bisect for "the idle
animation is stuck" ran to completion on exactly that: the hero's standing
pose is static in every build, and only the moment of his first random
fidget had moved.

### Kinds of bug met so far

- **A candidate with a mistake in it.** C from `nonmatching/` that was
  never run because the path into it was dead. `func_0022DD68` (the
  per-tick voice update) copied a voice's position into its moby instead
  of the other way round; every sound attached to a moby held the moby
  still until the sound ended. It only showed once sounds started playing.
- **Making a dead path live exposes the next bug.** The hostgen change
  that lets a `void` definition ending in a call return that call's result
  (150 functions) was correct, and it is what made the bug above visible.
  When a fix turns on a subsystem, expect to debug that subsystem next
  rather than reverting the fix.
- **A translation that is not what the C says.** Clang keeps
  `__attribute__((mode(TI)))` on a declaration and drops it from a type
  name, so `*(int __attribute__((mode(TI))) *)dst = *(... *)src` copied 4
  bytes of 16. Bolts were created 80 units away under the level.
  `prep.py` now rewrites that type name (`name_wide_casts`). When a
  function's result is wrong in one field only, read its generated C under
  `port/build/release/games/rac1-pal/gen/` before doubting the source.
- **A result read from the wrong place.** The functional C for
  `func_L00_002D6610` tested the damage resolver's return value where
  retail tests its fifth argument.
- **Something the catalogue folded together.** Three different accessors
  share one fingerprint and were one function;
  `port/game/rac1-pal/split_places.tsv` separates them.
- **Not drawn right rather than not simulated right.** A deleted moby
  (state 0xFD) was still drawn; a moby that turns fast was not blended
  between ticks. Check the simulation with `OPENRAC_MOBY_LOG` first: if the
  numbers are right the fault is in `port/viewer` or the frontend.

## 5. Backfilling C for a function

When the game stops at a function without C, or behaves as if one did
nothing:

1. Confirm it is missing: the frontier in `report.md`, and
   `OPENRAC_TRACE_MISSING=1` for who called it.
2. Look for C that already exists: `nonmatching/shared/`,
   `nonmatching/game/` and the per-level folders in the decompilation hold
   near misses that are not in `functional.tsv` yet. A near miss with the
   same calls in the same order is usually right; read it against the
   assembly where it differs in size.
3. Otherwise write it from the function's instructions (the
   decompilation disassembles them from the owner's disc; its README says
   how), with `rerac` as a description of intent. The US
   release's addresses differ; map by the surrounding calls, not by
   arithmetic. Nothing from an SDK, and nothing the sourcing policy rules
   out ([SOURCING.md](../policy/SOURCING.md)).
4. Put it where it belongs:
   - into the decompilation as `nonmatching/functional/<name>.c`, one
     function a file, with a line in `nonmatching/functional.tsv`. This is
     the preferred home: the decompilation's own contributors can turn it
     into a match;
   - or into `port/game/rac1-pal/hand/` with a line in `hand.tsv`, when it
     has to differ from what could ever match (a change marked in the
     file) or is needed before the decompilation can take it.
5. Head the file with what the function does, that it is not a match, and
   where it came from. Code adapted from `rerac` carries its ISC notice
   and the file it came from. The third-party US decompilation is only
   ever "the NTSC decomp": no author's name, handle or address.
6. Rebuild, run the scenario that needed it, and check its neighbours
   still work.
7. Run the gate: `python3 tools/fcheck_gate.py run --only <name>`
   ([BACKFILL.md](BACKFILL.md#the-gate-done-means-fcheck-says-same)).
   The function is done only when its row says `same`; `unchecked` (with its
   reason) is delivered as unchecked, and `differs` is fixed first. A game
   run that looks right is not a verdict: the two constants bugs that
   shipped in backfilled C ran fine on the path anyone tried.

hostgen reads GNU C for a 32-bit MIPS target, so write what the rest of
the decompilation writes: explicit offsets or the decompilation's structs,
no host types, no `long` for a pointer. If hostgen cannot translate a
function it becomes a stub and `report.md` says why.

## 6. Editing safely on this machine

- Files here have CRLF line endings (`core.autocrlf=true`). A script that
  edits a file must keep them.
- Do not write code containing backslashes through a shell heredoc: `\n`
  became a newline and `\b` a backspace in three separate edits. Use the
  editor's own file tools, or a Python script saved to a file.
- Two shell commands run at once share a working directory.
- clangd or an antivirus scan can hold hostgen's prepared files open;
  `clangast._write` retries, and a build that still fails with
  `PermissionError` can simply be run again.
- Diagnostics the editor shows for `port/` files (`openrac/game_host.h`
  not found and what follows) come from clangd lacking the build's flags,
  not from the code.

## 7. Open at the time of writing

Understood but only guarded, with their proper fixes still to do:
[KNOWN_ISSUES.md](KNOWN_ISSUES.md) (the full moby pool on Gaspar, for one).

Reported by the player and not confirmed fixed:

- **The wrench against the hand.** "The hero moves slower than the
  wrench." The blend between ticks was changed
  (`blend_matrix` in `frontend.cpp`: translation always, rotation only for
  the same moby) and the hero no longer freezes, but nobody has looked at
  it since. Three mobys follow the hero in level 0 (classes 71, 601 and
  607; which is the wrench was not established). Compare them with the
  hero's hand joint per tick with `OPENRAC_NO_BLEND=1`, then with blending.
- **Music audible with the Windows mixer at zero.** Not explained. Known:
  the game's output peaks near half scale without clipping, it opens one
  ordinary stream on the default device (a second only while a movie
  plays), and the player's output device reports a range that ends at
  -65.25 dB rather than at silence. Not done: listing the audio sessions
  Windows holds for the process while it plays. The port now has a
  `volume` setting (0 to 100, F9 and F10) that does reach silence; it has
  no row on the Options page yet (`options_menu.cpp`; a fifth row may not
  fit the frame the page borrows, so look at it before committing).

Checked only by scripted runs, not by anyone playing: the hero's walk
after the voice fix, bolts from crates and from a class 749 creature, that
creature's death. Sound effects have never been listened to, only counted
as commands.

For the decompilation:

- Functions without C that play reaches: the 33 of the frontier, among
  them `func_L00_00254218` (the collision kernels' joint pose, which the
  port replaces with its own evaluator) and `func_L00_002788E0` (called
  while the pause menu's pages are open).
- Near misses in use that should become matches: `func_L00_0025A8E8`,
  `func_L00_0025AA20`, `func_L00_0025D6F0`, `func_L00_002D1E68`,
  `func_L00_00261B00`, `func_L00_002D4EC0`, `func_0022DD68`.
  `func_001F9BB0` and `func_001F9E58` were written from their callers,
  not from their instructions.
- The port's `hand/` still holds copies of the eight functions that were
  moved to `nonmatching/functional/`; they can go once the in-tree copy of
  the decompilation is synced.
- Every candidate that has never run is a suspect. Levels other than 0
  have not been tried since these fixes.

Modding (editor `Pack`, `--mods DIR`): adding and deleting mobys and
moving ties and shrubs work. Not done: custom models (planned as
port-native: a mod ships a `.glb` and a new class), collision for moved or
added scenery, terrain, and a check that added mobys fit the level's
capacity.

Neither repository's recent local commits are pushed. Pushing needs the
owner's word, and goes to `master` (OpenRAC) and `main` (rac1-decomp), not
to a branch.
