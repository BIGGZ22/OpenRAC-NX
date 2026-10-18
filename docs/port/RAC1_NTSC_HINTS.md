# Ratchet & Clank (NTSC-U): known trouble spots for the native port

Investigation notes for [port/game/rac1-ntsc](../../port/game/rac1-ntsc/README.md),
written 2026-10-18. They collect places where the US game's own code is known
to behave in a way a native build has to meet, so that whoever plays the port
in a window knows where to look first. Nothing here was reproduced in OpenRAC:
each entry is a lead (a symptom a player would see, the cause in the game's
code as far as it is understood, and the files and functions in this tree
where the cause would show up or is already dealt with). Where the cause is
only a guess, the entry says so. The practical side (build, run, find the
cause of a gameplay bug) is [RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md);
the switches for looking inside a running game are [DEBUGGING.md](DEBUGGING.md).

## How addresses are given

US addresses are those of the level overlays' shared engine code as the NTSC
decomp lays it out for level 0 (`games/rac1/ntsc/config/overlays/us/`,
`functions.tsv`: a row's places are `level:address`). The generated
`functions.c` of the build (`port/build/release/games/rac1-ntsc/gen/`) lists
the same address against the name hostgen gave it, so `0x1F9838` is
`game_FUN_001f4a58` there. Names in `FUN_L<level>_<address>` form are the
NTSC decomp's own for code that exists in one level's overlay. A PAL address
for the same function comes from `build/versionmap/rac1-ntsc--rac1-pal.json`
([versionmap](../../port/tools/versionmap/README.md)); a `near` pair changed
between the versions, so PAL's behaviour is a hint there, not a guarantee.

Levels are numbered as the discs number them. The ones named here: 0 Veldin,
2 Aridia, 3 Kerwan, 8 Batalia, 9 Gaspar, 12 Hoven, 13 Gemlik Base, 15 Quartu,
16 Kalebo III, 17 Drek's fleet.

## Where the port stands on each, in brief

| Mark | Meaning |
|---|---|
| handled | the port has code for it today (named in the entry); not proven on NTSC-U in play |
| partly | something is in place, something the entry names is not |
| open | nothing in the port deals with it yet |
| unknown | the cause is not understood; the entry says what to look at |

## 1. Planets

### 1.1 Quartu: a crash under water when the air runs low (level 15)

- **Symptom.** The game stops when the air meter gets low underwater.
- **Cause.** The hero's low-air beep asks the sound system for sound id -1,
  which no bank holds. The console's sound driver ignores an id it does not
  have and answers with a null handle. A host that indexes the bank with it
  reads outside the table.
- **Where.** The request travels through the game's command batch, which
  `game_FUN_0012dc80` (`port/game/rac1-ntsc/host/hand.c`, the replacement of
  snd_FlushSoundCommands) hands to `openrac_sound_command` and then to
  `openrac_sound_play` (`port/game/common/lib/sound_engine.cpp`). Set
  `OPENRAC_SOUND_LOG=1` to see each command and its reply.
- **Port today.** Handled: `openrac_sound_play` returns 0 for a negative id
  or an unknown bank, and `SoundPlayer::play` (`port/audio/sound_player.cpp`)
  checks the id against the bank's size. The game then sets parameters on
  handle 0, which `openrac_sound_set_params` also refuses. Worth keeping a
  test of "play id -1" in `port/tests/audio/`.

### 1.2 Kalebo III: a crash in the landing scene with a widened view (level 16)

- **Symptom.** The scene where the ship lands stops the game when the picture
  is wider than 4:3 and the level of detail has been raised.
- **Cause.** Not understood. The scene is drawn from lists the game builds
  from its own 4:3 visibility decisions (see 3.3); a wider or farther view
  hands the draw code more entries, or entries it never meets at 4:3, than
  that scene's code was written for. A fixed-size buffer in the scene's
  overlay code is the first suspect.
- **Where.** `aspect` in [SETTINGS.md](SETTINGS.md) and the picture-shape
  code in `port/game/common/frontend.cpp`; the scene itself is level 16's
  arrival code, found from `FUN_L00_00298840` (cutscene start, the
  level-specific copy via `functions.tsv`) and the level's ship-sequence
  functions.
- **Port today.** Open for NTSC-U. The port draws every tie, shrub and
  terrain part and has no level-of-detail raise yet
  ([SETTINGS.md](SETTINGS.md), section 5), so the trigger may not exist until
  the game's own visibility is carried over. Try the scene at 4:3 and at
  `aspect = 21:9` with `OPENRAC_GUARD` (RAC1_PAL_BUGFIX_GUIDE) when it can be
  reached.

### 1.3 Hoven: the ice around Ratchet is cut open (level 12)

- **Symptom.** The ice cube Ratchet is frozen in is missing faces or lets the
  water show through.
- **Cause.** One part of the cube's model writes colour but no depth. Whether
  it is covered depends on the order in which the game's lists are drawn:
  the draw lists the frame runs before the moby pass (Hoven's water) belong
  before the mobys, the rest after. Mobys drawn first, or all lists drawn
  last, cover it wrongly.
- **Where.** The order of passes in `port/renderer/renderer.cpp`
  and the moby drawing under `port/renderer/`; the game's side is the
  moby pass `FUN_0020d460` (US 0x250038) and the lists drawn around it.
- **Port today.** Unknown: the renderer draws mobys in separate passes for
  solid, glow and translucent parts ([SETTINGS.md](SETTINGS.md)); nobody has
  looked at a moby part with a colour write and no depth write.

### 1.4 Water, lava and sand surfaces at the sides of a wide picture (levels 1, 2, 5, 7, 9)

- **Symptom.** In a picture wider than 4:3 tiles of a lake, lava or sand sea
  are missing near the left and right edges.
- **Cause.** Each of these surfaces has a per-level draw callback, and it
  decides which tiles to draw with the game's own clip test, which knows only
  the 4:3 picture; nothing in the port widens that test.
- **Where.** The callbacks are level code (`FUN_L<level>_...`) registered
  into the game's draw callback lists
  (`port/game/rac1-ntsc/host/callback_lists.c` has the registration
  routines the levels carry). The matrix is the one `FUN_001f2d98` (US 0x1F7980, update_view_context)
  writes.
- **Port today.** Open. PAL's port draws the 3D itself from the extracted
  level and does not use these callbacks for the world, so check whether the
  NTSC window's levels hit this at all once the NTSC levels exist
  (README item 2).

### 1.5 Kerwan: bullets frozen in the air, cables not caught (level 3)

- **Symptom.** A helicopter's bullets stop where they were fired; the
  hanging cables cannot be grabbed.
- **Cause.** Bullets are particles, and a particle whose update does not run
  stays where it is (see 2.5). The cables are found by a rail search that
  exists in one build on Kerwan and another elsewhere: a routine that looks
  the same in the function catalogue but differs on one level.
- **Where.** `split_places.tsv` and `host/variants.c` in
  `port/game/rac1-ntsc/` exist for exactly this (the NTSC decomp's catalogue
  folds places by fingerprint). `FUN_00217b88` (`hand/game_particle_update.c`)
  is the particle step.
- **Port today.** Partly: the particle step is carried from PAL, not checked
  on NTSC-U. The catalogue's false folds are only listed where PAL had them;
  a level whose behaviour differs from the others may still hide one
  (see 2.6).

### 1.6 Drek's fleet and Quartu: frozen laser beams and streaks (levels 17, 15)

- **Symptom.** Laser beams that should move hang in place; Giant Clank's
  attack leaves streaks on the screen.
- **Cause.** Each is a particle type with its own update and draw code; the
  ship's engines use a type shared by several levels, Quartu's attack another.
  A type that is never stepped freezes; one whose old frames are not cleared
  streaks.
- **Where.** `level_particle53.c` (`FUN_L00_002721f0`) is the one such type
  the port has hand-written C for; the others are in the level's
  `FUN_L17_...` and `FUN_L15_...` functions. `port/game/rac1-ntsc/hand/hand.tsv`
  lists what is carried.
- **Port today.** Partly, as 1.5.

### 1.7 Batalia: a cutscene and its dialogue go out of step (level 8)

See 4.3.

### 1.8 Force fields, hologram gates and oxygen barriers drawn black (Orxon 10, Quartu 15 and other levels)

- **Symptom.** A see-through effect appears as a solid black shape.
- **Cause.** The surface is drawn with an alpha blend and a colour tint set
  in the graphics chip's registers (ALPHA, TEST); a renderer that reads the
  texture's alpha but not the blend mode draws it opaque or black.
- **Where.** The state arrives in the game's packets as the GS register
  words `port/renderer/gs.h` decodes; the blend choice is in
  `port/renderer/direct.cpp` and `port/renderer/shaders/direct.frag`.
- **Port today.** Partly: the direct renderer applies the GS blend modes for
  the 2D and for effects; whether the three level effects use only modes it
  knows has not been checked.

## 2. Gameplay systems

### 2.1 The Vendor hangs the game (every level with a Vendor)

- **Symptom.** The game freezes when a Vendor opens or closes, or the
  Vendor's box does not appear.
- **Cause.** The Vendor opens by adding 0.1 to a scale while it is below 1
  and shrinks it by 0.1 a frame until it drops under 0 when it closes. The
  console's FPU rounds towards zero, so these sums end at 1.0999998 and about -4e-7. Rounded to
  nearest (the host's mode) they end at 1.0000001 and -0.1: the scale goes
  negative, the Vendor's collision rectangle turns inside out, and the
  routine that moves a moby between the grid's cells (US 0x250B20,
  `FUN_0020dc20`, update_moby_grids; every level has a copy) walks a cell
  range that never ends.
- **Where.** `FUN_0020dc20` in `port/game/rac1-ntsc/hand/game_moby_grid.c`
  (listed in `hand/hand.tsv`); the Vendor itself is level and shared code:
  `FUN_L00_002d19b0` is drawn by `hand/level_vendor_draw.c`.
- **Port today.** Handled in two layers. `run_game_as_the_ee` in
  `port/game/common/main.cpp` sets `FE_TOWARDZERO` for the game's thread
  (`OPENRAC_ROUND_NEAREST=1` turns it off to compare), so the sums end where
  the console's end. And the hand-written grid routine does not loop on an
  inside-out rectangle: `exe_grid_valid` rejects it and `exe_grid_trap` stops
  there, so a regression shows up as a trap with the moby, not as a hang. `port/tests/exe_moby_grid_test.c` exercises the grid routine.
  **Not handled:** the console's FPU also flushes tiny results to zero (no
  denormals) and its divide rounds to nearest while its other operations
  truncate; the port sets neither, so a divide under `FE_TOWARDZERO` truncates
  where the console may not. The vector hand code (`host/collision.c`,
  `fesetround(FE_TOWARDZERO)` around its blocks) follows the VU rule only.
  If positions drift from a reference run by one unit in the last place after
  a divide, start there.
- **Related.** The Vendor's code is all translated now: the old "skip" entries
  for it in `port/game/rac1-ntsc/hostgen.json` named files the NTSC decomp had
  renamed, and were removed (221d707c, which also made the salesman show).
  If the Vendor still hangs, the rounding above is the place to look.

### 2.2 A NaN where the console has a number

- **Symptom.** The camera or an object jumps or disappears for a frame, and
  keeps doing so.
- **Cause.** The console's floats are never NaN: a square root of a negative
  takes the magnitude, a division by zero gives the largest number,
  0 times infinity is 0. The host's give NaN, which then compares false with
  everything.
- **Where.** `fsqrt` in `port/game/rac1-ntsc/host/collision.c` already takes
  `fabsf`; the same file drops a moby whose bounding sphere is not finite
  from the queries. `port/assets/ps2_float.h` has the model of the console's
  float rules for the tools. A `div` by zero in carried C is the other place.
- **Port today.** Partly: the vector hand code follows the rule; plain C
  from the decomp does not, and nothing clamps. For a jumpy camera, trace
  the camera block's words (`FUN_001edaa8`, US 0x1ED0A8, update_camera, and its
  collision sweep `FUN_L00_001ed6a8`, US 0x1ED6A8) for a NaN first
  (RAC1_PAL_BUGFIX_GUIDE: `OPENRAC_WATCH` and `OPENRAC_GUARD`).

### 2.3 The camera stops following for a second after a strafe

- **Symptom.** After the player releases a strafe, the follow camera stays
  where it was for about a second before it catches up.
- **Cause.** Unknown. It looks like a timer in the follow camera that the
  strafe state sets and nothing clears, or a hero state that the camera reads
  and that keeps its value one state too long.
- **Where.** The follow camera's per-frame update, `FUN_001edaa8`, and the
  hero state machine (`FUN_L00_002070d0`, hero_frame_update, and
  `FUN_L00_00217970`, the state update); the steering helper
  `game_FUN_L00_001eb328` in `host/camera.c` is the camera code the port
  replaces by hand.
- **Port today.** Unknown.

### 2.4 A cutscene that starts during a strafe keeps the strafe pose

- **Symptom.** Ratchet stays bent in the strafe pose through a cutscene.
- **Cause.** The cutscene start does not return the hero to a neutral state,
  and the strafe's pose belongs to a state that nothing leaves while the
  hero is not updated.
- **Where.** `FUN_L00_00298840` (US 0x298840, cutscene start) and the hero
  state setter near `FUN_L00_002223f8`.
- **Port today.** Open; no strafe run has been done. Low priority.

### 2.5 Objects that stand still or never appear

- **Symptom.** A class of object (a pickup, an enemy, a gadget) does nothing,
  or the Pilot's Helmet cannot be picked up, on every level.
- **Cause.** Each class's update is a level function reached through the
  class table. A function without C in the port is a stub that returns at
  once, so the moby is simply not updated.
- **Where.** `port/build/release/games/rac1-ntsc/gen/report.md`, "The
  frontier" lists what the boot, title and level loops reach without C; the
  skipped files in the NTSC README's table are stubs for every function they
  hold. Run with `--stop-on-missing` to stop at the first.
- **Port today.** Partly. The report lists one function without C on the
  boot, title and level 0 path now; every level beyond 0 is untried.

### 2.6 Level copies of one routine that differ

- **Symptom.** A behaviour is right on most planets and wrong on one.
- **Cause.** The NTSC decomp's function catalogue names by fingerprint; two
  levels' copies of a routine can differ in a constant or a field offset and
  still be catalogued as one. Whatever C is written for the name then runs
  on a place it does not fit (a rail search, a hero routine, a Vendor update).
- **Where.** `port/game/rac1-ntsc/split_places.tsv` (23 rows carried from
  PAL) and `host/variants.c`; the catalogue is
  `games/rac1/ntsc/config/overlays/us/functions.tsv`.
- **Port today.** Handled where PAL found it. Anything new needs the same
  treatment: list the place in `split_places.tsv`, write the variant.

### 2.7 Aiming up and down

- **Symptom.** None in the original; a player used to modern controls may
  expect to aim a weapon up and down, and the gloves' throw to land nearer
  or farther.
- **Cause.** The original has no vertical aim input of its own; weapons fire
  as the hero's state and the lock-on direct them (not verified here).
- **Where.** The weapon firing functions in the hero code (`FUN_L00_00229b70`,
  hero input, and the per-weapon class code).
- **Port today.** Out of scope: faithful behaviour is the goal
  ([DESIGN.md](DESIGN.md)). Do not treat missing aim as a bug.

## 3. Rendering

### 3.1 Fades

- **Symptom.** A fade to black cuts, or runs for a different number of
  frames, or the picture under it flickers.
- **Cause.** The fade routine (US 0x1F9838, `FUN_001f4a58`, fade_to_black)
  runs its n + 2 frames inside one call, waiting for the vertical blank in
  the middle, and draws nothing of its own: each of its frames sends the last
  display list again with a black quad over it. It has dozens of callers
  (the story cards, level exits, scene changes).
- **Where.** `openrac_game_vsync` in `port/game/common/main.cpp` (the frame
  ends where the game waits for the blank), `show_picture` and its `black`
  argument in `port/game/common/frontend.h` for pictures the game puts
  straight in its display buffer, and the comment at the list-resend in
  `frontend.cpp` (the "named quads" step).
- **Port today.** Handled in design: the game runs on its own thread and
  blocks at the wait, so the fade's frames are shown one by one. Two things
  to check on NTSC-U: that the card between planets keeps its two lines of
  text during the fade (the PAL fix is the quads kept from the list's own
  tick), and that the frame counter the fade reads moves by the same amount
  as on the console (`g_frame` in `main.cpp`).

### 3.2 The Vendor's screens and page menus

- **Symptom.** A Vendor page or a menu opens on a black or stale picture, or
  with the wrong items, only on some planets.
- **Cause.** Those screens put a stored picture of the frame back instead of
  running the world passes; they read the draw buffer back from the graphics
  chip when they open. On four planets (2 Aridia, 4 Eudora, 7 Umbris and 14
  Oltanis, whose Tesla Claw page) a page drew wrongly.
- **Where.** `openrac_game_store_image` (`main.cpp`) calls
  `frontend::store_image`; the Vendor mode is game mode 5 in the main loop
  (`FUN_L00_00245c28`, US 0x245C28). `hand/level_vendor_draw.c` builds the
  reflective mesh.
- **Port today.** Partly: the read-back is answered with the last frame
  drawn, scaled; the Vendor's 74-quad mesh is hand-written for NTSC-U from
  PAL's reconstruction and not checked in play.

### 3.3 The game decides visibility for a 4:3 picture

- **Symptom.** In a wide picture objects pop in at the sides, thin far things
  (antennas, spires, plants) are missing, or a moby behaves as if it were not
  visible while the player sees it.
- **Cause.** The game culls and picks levels of detail inside its own code,
  from a 4:3 projection (`FUN_001f2d98`, US 0x1F7980, builds it; the view
  setup is `FUN_001f2260`, US 0x1F6E48). A moby's "was drawn" flag is game
  state that class code reads (an enemy that only acts when drawn), so a
  host that draws more than the game decided leaves the logic on the old
  answer.
- **Where.** The renderer's own world passes in `port/renderer/world/`
  (`tie_lod.cpp`, `tfrag_lod.cpp`, `world_frame.cpp`) and the aspect in
  `frontend.cpp`.
- **Port today.** Open: the port draws everything and does not touch the
  game's flags, which keeps the logic faithful but means a wide picture shows
  objects whose class never ran. The vertical field-of-view factor is still
  PAL's 0.756 ([README](../../port/game/rac1-ntsc/README.md), item 4); a
  raised field of view also needs the sky and the top and bottom bands
  checked.

### 3.4 Effect textures that change every frame

- **Symptom.** After some tens of seconds, glows, smoke and sand lose their
  texture or turn black; on some graphics cards the game then stops.
- **Cause.** Several effects (Aridia's sand sea is one) upload new texture
  pixels each frame. A host that keeps every upload it ever converted runs
  out of space on the card.
- **Where.** `port/renderer/texture_pool.cpp` and `.h`: uploads are converted
  by base pointer in `m_uploads`.
- **Port today.** Handled in design: conversions of game uploads that no
  draw has used are dropped once a frame (the "may be dropped" mark in the
  header). Check with a long run on level 2 and `OPENRAC_OVERLAY=renderer`
  that the texture count stays flat.

### 3.5 Particles and effects drawn in the wrong order or not at all

See 1.3 and 1.6. The particle program and its packets go through the
direct renderer's effect path (`port/renderer/effects.cpp`); the game's own
particle code is `FUN_00217a30` (allocate) and `FUN_00217b88` (update),
carried from PAL in `hand/`.

## 4. Sound

### 4.1 A sound that does not exist

See 1.1. The same guard covers any bank-less request: a sound asked for
before its bank has loaded returns handle 0 and the game goes on.

### 4.2 Music and effects through a load

- **Symptom.** The music cuts out when a level loads, a checkpoint is
  restored or a movie opens, and comes back late.
- **Cause.** On the console the sound runs on a separate processor, so the
  music goes on while the main one is busy for a long frame. A port whose
  sound is advanced by the game's own frames goes silent when the game
  thread stalls.
- **Where.** `port/platform/audio.h` (the pull callback model) and
  `port/game/common/lib/sound_engine.cpp`: `mix_streams` and
  `SoundPlayer::render` are called from the output's own thread, and disc
  reads (`openrac_sound_load_bank_from_disc`, the VAG decode) happen outside
  `g_lock`.
- **Port today.** Handled by structure: the audio thread mixes on the
  device's clock, whatever the game thread is doing. Two things to watch.
  A game held by the debug overlay (F5) may keep its music mixing
  unless the streams are paused too; and what the game reads back
  (whether a voice has finished, a stream's position: 4.4) now follows
  the host's clock, not the game's frames.

### 4.3 The pause menu and dialogue

- **Symptom.** Dialogue and music keep playing under the pause menu, so a
  cutscene's speech and its picture drift apart (seen on Batalia, level 8).
- **Cause.** The pause menu pauses its sounds by group
  (`snd_PauseAllSoundsInGroup`, US row `FUN_0012e3e8` in `libraries.tsv`) and
  the streams by handle (`snd_PauseVAGStream`, `FUN_0012ec70`).
- **Where.** `openrac_sound_pause_groups` in `sound_engine.cpp` pauses the
  player's groups only; `openrac_stream_pause` pauses one stream by handle;
  a `Stream` has a `group` field the group call does not read.
- **Port today.** Partly: if the game pauses dialogue by group alone, the
  stream goes on. Test by pausing in a cutscene with speech and watching
  `OPENRAC_SOUND_LOG` for which calls the pause makes.

### 4.4 Read-backs of sound state

- **Symptom.** Music changes (a fade between tracks, a cue waiting for a
  stream to end) happen early or late when the machine runs slow or fast.
- **Cause.** The game asks how much of a stream is left and whether a sound
  is still playing, and acts on the answer; the answer depends on whose time
  the sound runs in. `FUN_00216290` (US 0x265AA8, music_update) is the main
  reader.
- **Where.** `openrac_lib_snd_GetVAGStreamTimeRemaining_CB` and
  `openrac_lib_snd_SoundIsStillPlaying_CB` in `port/game/common/lib/snd.c`.
- **Port today.** Unknown: both answer from the audio engine's real-time
  state. Fine at 60 ticks a second; a lagging or uncapped run could change
  the order of two music events.

## 5. Saves

### 5.1 The save folder name comes from the disc

- **Symptom.** The game finds no save, or saves under a name a console would
  not use; two discs share a card.
- **Cause.** The card folder's name is the game's serial, which it takes from
  the boot file on the disc (US 0x24B410, `FUN_00209030`,
  memcard_get_name). The US disc gives the code SCUS-97199 and so a folder
  of its own; the PAL disc's is different.
- **Where.** `game_FUN_00209030` and the disc replacement
  `port/game/common/lib/cdvd.c` (`sceCdRead` by sector, reading the image
  `openrac_game_disc_image`). The card folder is
  `default_cards()` in `main.cpp`, per `openrac_game.serial`
  (`SCUS_971.99`).
- **Port today.** Partly: the port's card folder is per game id, and the
  folder inside it is the game's own. Loading an existing save works on NTSC-U
  since the card's change-directory call was fixed (35a38680); a first save on
  a fresh card has not been tried. Make
  sure the boot file is readable through the sector reads, and that the name
  the game builds is what the launcher's save manager looks for.

### 5.2 Save blocks and checksums

- **Symptom.** "Damaged data" when loading a save the port just wrote.
- **Cause.** A save holds a game block and one block per level, each built
  by walking a table that names which of the game's variables go in it, and
  closed by a 16-bit checksum;
  loading recomputes it. Any change in how the lists are walked, or in the
  byte order of the checksum, fails every load.
- **Where.** US `FUN_0020abb0` (0x24CE68, memcard_make_whole_save),
  `FUN_0020acc0` (0x24CF78, calculate_crc16), `FUN_0020ad38` (0x24CFF0,
  validate_data_crc), `FUN_0020ad78` (0x24D030, memcard_prepare_data). All
  are matched C in the NTSC decomp, so they run as written.
- **Port today.** Handled in design (the code is the game's own); the file
  I/O under it is `port/game/common/lib/mc.c`. Round-trip a save through
  Save and Load before trusting it.

### 5.3 The card's timing and size

- **Symptom.** The card screens hang waiting, show "no card", or refuse a
  save for lack of space.
- **Cause.** The card state machine (US 0x24B690, `FUN_002093d8`,
  memcard_update_state, about 6 KB) starts a library call and polls for its
  end over several frames, and checks the free space before saving.
- **Where.** `openrac_lib_sceMcSync` and `openrac_lib_sceMcGetInfo` in
  `mc.c`.
- **Port today.** Partly: every command finishes when it is issued and
  `sceMcSync` reports it on the first poll; `sceMcGetInfo` answers 7000 free
  clusters and a formatted card. Fine if the machine tolerates an answer
  that is always ready; if a screen waits for a state that never shows
  "busy", add a frame of delay in `finish`. Check that a save needs fewer
  clusters than 7000.

### 5.4 When the game saves

- **Symptom.** Progress lost after leaving a planet, or a save written when
  none was chosen.
- **Cause.** The game saves on leaving a planet into the slot the player
  chose (US 0x24D430, `FUN_0020b178`, memcard_save_data; called from the
  level exit, `FUN_00231ff0`, do_space_transition), and from the Save pages
  (`FUN_002235b8`, US 0x281200, the new-game slot menu). Checkpoints are in
  memory and never reach the card.
- **Where.** The same functions; the folder is the card folder of 5.1.
- **Port today.** Handled in design. When testing, note that a checkpoint
  restore does not touch the card, so a missing save after a death is not a
  card bug.

## 6. Most urgent for the NTSC-U port

In this order, because each blocks something that otherwise works:

1. **The Vendor and its stubbed files (2.1).** The shared Vendor class and
   level 13's are not built; the grid routine and the rounding mode are in
   place but unproven on this version.
2. **Division and flush-to-zero (2.1, 2.2).** The one place where the
   console's float rules are not copied; it affects every level at once.
3. **The card path (5.1 to 5.3).** The first New Game on NTSC-U writes a
   save; the folder name, the always-ready card and the free-space answer all
   sit on that path.
4. **Sound cases (4.3, 4.4).** Pausing streams by group and read-backs
   against the host's clock matter for every cutscene.
5. **Fades and cards (3.1).** The story cards between planets are the first
   thing a new game shows.
6. **Wide-picture visibility (3.3, 1.2, 1.4).** Not urgent until the NTSC
   levels exist (README item 2), but the port's choice to draw everything
   changes what the game's logic sees, and that is better decided early.
