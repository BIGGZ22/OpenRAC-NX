# Ratchet & Clank (PAL): playtesting the port by script

How to reproduce what a player reports without playing: start any level,
put Ratchet where the bug is, press buttons on a schedule, skip what is in
the way, take screenshots and read game memory, then check a fix the same
way. It complements [RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md)
(build, switches, addresses, finding a cause) with the procedures that work
and the traps met on the way. Everything here runs the port's own
development switches; nothing in it emulates the console.

## 1. Before a run

- **Build** from the repository root: `python3 tools/build_port.py rac1-pal`
  (on macOS use Homebrew's Python 3). A change to `hostgen.json`,
  `libraries.tsv`, `hand/hand.tsv` or `split_places.tsv` needs hostgen again:
  delete `port/build/release/games/rac1-pal/gen/hostgen.stamp` first.
- **The launcher's Play may run another copy.** It starts the build its
  saved root points at (a bundled copy, for an installed launcher), not
  `port/build/release`. When a player says a fix "did not go through", check
  which executable ran.
- **Levels.** The game reads extracted levels (`--levels DIR`) of its own
  format only ([levels-format](#7-when-the-export-changes)). For a test that
  needs fresh levels, export to a scratch folder under `build/` instead of
  overwriting the player's:
  `python3 editor/extract.py port games/rac1/baserom/SCES_509.16.iso build/NAME --level N`
  (writes level N plus the title world 99 and the flight world 98, which a
  direct start needs; about half a minute). Delete it afterwards.
- **The data folder** (`--data`) is the launcher's install:
  `~/Library/Application Support/dev.openrac.launcher/active/rac1/data` on
  macOS.

## 2. The run line

From `games/rac1`:

```sh
D="$HOME/Library/Application Support/dev.openrac.launcher/active/rac1/data"
SDL_AUDIODRIVER=dummy OPENRAC_UNCAPPED=1 OPENRAC_DIRECT=1 \
OPENRAC_PRESS=100:4000:5 \
  ../../port/build/release/openrac-rac1-pal --data "$D" --levels ../../assets/port/rac1 \
  --window --no-card --level 1 --skip-movies --frames 2300 > run.log 2>&1
```

- `OPENRAC_DIRECT=1` with `--level N` starts a new game straight in level N
  (the title's wait and menus skipped); `100:4000:5` presses Cross through
  the boot screens.
- `--skip-movies` skips every movie (fast; leave it out when the movies are
  under test). `--no-card` never touches the player's saves. `--frames N`
  ends the run.
- `SDL_AUDIODRIVER=dummy`: no sound on the player's speakers.
- `OPENRAC_UNCAPPED=1`: as fast as the machine goes (a few thousand frames
  in seconds). Add `OPENRAC_NO_BLEND=1` for one picture per tick.
- `--settings FILE`: another settings file (a missing one gives the
  defaults), for example `skip_cutscenes = 0` to test a setting off.
- Run one game at a time: several at once have run the machine out of
  memory. Kill leftover `openrac-rac1-pal` processes a crashed run leaves.
- In zsh, `for p in "1 2"` does not split `$p`: run loops under `bash -c`.

## 3. When things happen (direct start, uncapped)

Frames below are the window's frames (`g->index`), which `OPENRAC_PRESS`,
`OPENRAC_WRITE` and `OPENRAC_SHOT` all count. Times move a little between
builds; watch the game mode to be sure (section 5).

| Level | Timeline |
|---|---|
| 0 Veldin | Level up at ~939, its opening scene (mode 2) ~959 to ~1803, control from ~1803 |
| 1 Novalis | Arrival scene (mode 2) ~530 to ~2066, control from ~2066 |
| 3 Kerwan, other planets reached by ship | The flight between planets first (~360 to ~1009: its stage word `0x13E188` goes 1, 2, 4), then the landing, then control |
| 5 Rilgar | Flight, landing; control from about 1800 |
| 16 Kalebo III | Flight, landing; control from about 2000 |

Start (mask `8`) skips the scenes and the flight while "Skip cutscenes" is
on (the default), so `OPENRAC_PRESS=...,600:8:5` gets to a planet quickly.

## 4. Moving Ratchet and the camera

**Teleport** with `OPENRAC_WRITE=FRAME:ADDR:VALUE:FRAMES` (32-bit words in
hex) into the hero's position, after control is his:

```sh
f(){ python3 -c "import struct;print('%08X'%struct.unpack('<I',struct.pack('<f',$1))[0])"; }
OPENRAC_WRITE=2150:13F4D0:$(f 168):10,2150:13F4D4:$(f 135):10,2150:13F4D8:$(f 60.3):10
```

- `0x13F4D0/D4/D8` position, `0x13F4E8` yaw (hold it about 50 frames).
- Hold the position 10 frames: shorter writes were undone on Novalis.
- A write before control (during a scene or the flight) is lost.
- The camera keeps the direction it had: a teleport to the right spot can
  still face away. Try a few points around the target in one batch and keep
  the one that frames it.

**Pad**: `OPENRAC_PRESS=FRAME:MASK:FRAMES[:LX:LY[:RX:RY]]`, masks in hex,
active while held: `8` Start, `4000` Cross (jump), `8000` Square (wrench),
`1000` Triangle (talk, vendors), `2000` Circle (the equipped item: the
first press brings the bomb glove out, a second one about a second later
throws), `800` R1, `400` L1, `100` L2, `200` R2. Sticks 0 to 255, 128 centred,
0 left or up; the left stick is relative to the camera.

- **Turn the camera**: hold the right stick, `1900:0:150:128:128:255:128`
  turns it round (180 degrees in about 150 frames).
- **Throw the wrench**: hold R1 and press Square, `1940:800:40,1950:8800:5`.
- **Throw a bomb where the camera sees it**: turn Ratchet first,
  `1945:0:12:0:128`, then Circle twice a second apart.

**Places found so far**

| Level | Where | What |
|---|---|---|
| 0 | (171.43, 270.94, 30.5) | Clank at the crash site: walking up starts the chain of scenes that ends on Novalis |
| 0 | (151.0, 90.4, 32.6) after ~1900 | Crates (class 500) |
| 0 | ~(151, 125, 29.8) | Horny toads (class 749) |
| 0 | start, camera turned 180 degrees | The fence behind the start |
| 1 | (168, 135, 60.3) | Facing the Gadgetron vendor (class 11 at (169.95, 140.44, 60)) and its hologram (class 1143, 2.95 above) |
| 1 | (168.8, 137.6, 60.3), stick up briefly, Triangle | Opens the vendor's store (mode 5) |
| 16 | after landing, camera turned | The glass sheets (class 800) by the landing |

Find others: `OPENRAC_MOBY_LOG=N:RADIUS` lists every moby within RADIUS of
the hero each N frames (class, address, state, mode bits, animation, position,
distance, update function, pvars, draw bytes). Classes and their meaning are
in ReRAC's `docs/plan/class_census.md` and the planet pages.

## 5. Reading the game

- `OPENRAC_WATCH=ADDR:COUNT[:i],...` logs COUNT floats (or ints) at each
  address every frame. Collapse a long log with
  `grep watch run.log | cut -d' ' -f4- | uniq -c`.
- **The game mode** `0x15F6A8` (raw, watchable): 0 play, 1 movie, 2 in-game
  scene, 3 pause menu, 5 vendor, 6 level loading and the flight's end. It is
  level data: host C reads it as `OPENRAC_LDATA(0, 0x0015F6A8u)`.
- **Level data moves.** Each level program relocates its data from
  0x15F000 up; a raw watch of an address there reads the executable's copy,
  not the level's (they agree on level 0). Host and hand C read it through
  `OPENRAC_LDATA(0, addr)` (a level-0 name) or `OPENRAC_DATA(addr)` (an
  executable name), as the generated code does: look at how
  `build/release/games/rac1-pal/gen/...` addresses the symbol.
- Moby fields: `+0x10` position, `+0x20` state, `+0x23` alpha, `+0x30` update
  distance, `+0x32` draw distance, `+0x34` mode bits, `+0x50..0x54` animation,
  `+0x64` joint-modifier list, `+0x73` shine distance, `+0x78` pvars, `+0x90`
  glow colour, `+0xA6` class.
- **Game C cannot print**: its `printf` goes to the game's own library. To
  probe it, count into spare guest memory and watch that, or log from host C
  (`fprintf(stderr, ...)`; declare it yourself where the file has no
  `stdio.h`). Mark probes `TEMP probe` and remove them before committing.
- `[warning] func_X has no C in the port yet; it does nothing` and
  `[error] call to code address ...` in the log name the missing piece of a
  bug more often than not.
- Retail code to compare against: `baserom/overlays/level_NN/text.bin` with
  its `manifest.json` (the text record's address) gives each level's own
  instructions, for checking what a level's copy of a routine calls.

## 6. Pictures

- `OPENRAC_SHOT=N:PREFIX` with `OPENRAC_SHOT_FROM=F` writes PREFIX<frame>.png
  every N frames from F; a frame is written only when it is a multiple of N
  (a SHOT of 500 from 1300 writes nothing at 1300).
- The PNGs' alpha channel is meaningless: look at RGB only
  (`Image.open(f).convert("RGB")`). Crop and enlarge the part under test;
  a full frame at thumbnail size hides most bugs.
- Make one strip or grid per run (a few lines of Pillow) instead of
  reading frames one by one.
- `OPENRAC_HIDE_PASSES=sky,terrain,ties,shrubs,mobys,effects,hud` hides
  renderer passes: the fastest way to learn which one draws a wrong thing.
- `OPENRAC_SHOW_MISSING=1` draws textures the 2D path cannot find in magenta;
  `OPENRAC_DUMP_DRAWS=N[-M]` logs the 2D path's draws; `OPENRAC_FRAME_LOG`
  logs each tick's draw bits and 2D bytes; `OPENRAC_DUMP_TEXTURE=BLOCK`
  writes a GS block's converted texture; `OPENRAC_DUMP_VIF=FILE` writes a
  frame's VIF data.
- **The player's own window**: `screencapture -x -o -l<window id>` takes the
  running game's window even behind others; the id comes from
  `CGWindowListCopyWindowInfo` (a five-line Swift script; the window is
  owned by `openrac-rac1-pal`). Sending it keys is not allowed by macOS.

## 7. When the export changes

A change to what `editor/port.py` writes that the port relies on bumps the
export format: `FORMAT` there, `kManifestFormat` in `port/viewer/level.h`,
the stamp `verified-<format>.json` in `launcher/actions.json` and
`tools/bundle_standalone.py`, and the test fixtures. The launcher then
re-reads every player's levels before Play.

## 8. Checking a fix

1. Reproduce first, with the numbers or pictures that show the bug.
2. Change one thing; rebuild (hostgen too when tables changed).
3. Run the same script: the bug gone, and a second level or case where
   the same code runs unchanged.
4. Compare a setting or switch on and off when the fix is one.
5. `ctest --preset release` in `port/` (64 tests), the editor's and tools'
   unit tests when those changed, `yarn verify` for the launcher.
6. Remove probes and scratch exports, kill leftover game processes, commit
   with what was checked.
