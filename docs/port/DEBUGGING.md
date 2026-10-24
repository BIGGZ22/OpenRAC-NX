# Debugging the port

What the native port ([port/](../../port/README.md)) offers for looking inside a
running game: an overlay over the game's picture, a per-frame dump for
automated checks, pause, single step and slow motion, and the `OPENRAC_*`
environment variables. All of it is in every build; none of it costs
anything worth measuring while it is off.

```sh
OPENRAC_DIRECT=1 OPENRAC_OVERLAY=frame,renderer \
  port/build/release/openrac-rac1-pal --data <data> --window --levels <levels> \
  --cards <scratch dir> --settings <scratch file> --skip-movies --level 0
```

## The overlay

**F3** shows and hides it. `--debug-overlay` or `OPENRAC_OVERLAY` shows it
from the start. It is a panel in the window's top left corner, drawn with
[Dear ImGui](https://github.com/ocornut/imgui) over the finished frame at
the window's own resolution. Until it is first shown nothing of it exists
(no ImGui context, no measurements beyond a few counters).

| Key | |
|---|---|
| F3 | show or hide the overlay |
| F4 | toggle enhanced graphics (probe GI, modern shadows and GTAO); preserves individual preferences |
| F5 | pause or resume the game |
| F6 | run one tick, then pause again |
| F7 / F8 | slower / faster: the game at 1/2, 1/4, 1/8, 1/16 of its speed |
| F12 | write the window as it is shown to `openrac_window_<tick>.png` in the working directory |

F5 to F8 and F12 work whether the overlay is shown or not. While the game is
paused or slowed, a line at the top of the picture says so and names the keys
that undo it; a screenshot says where it went.

The same keys are menus. On Windows and Linux a bar along the window's top
has View, Graphics, Enhanced, Sound and Developer, with a tick beside what is on
(Graphics lists the picture's settings with their values; a click moves one
to its next value, and the menu stays open); Alt by itself
hides it and shows it again, and View > "Hide these menus" hides it (the
`menu_bar` setting keeps the choice). On macOS the system's menu bar has
View, Sound and Developer instead (the game's own Video options page has the
picture's settings there); `OPENRAC_MENU_BAR=1` shows the in-window bar too.
On Android they are a drawer that slides in from the left (docs/port/ANDROID.md);
`OPENRAC_DRAWER=1` puts it in place of the bar elsewhere, opened with the
on-screen pad's options button or Back.

F1, F2, F9, F10 and F11 keep their meanings (the level menu of developer
mode, the frame counter, volume, fullscreen). Escape pauses, as Start does;
the program ends when its window is closed (Cmd+Q on a Mac). On a Mac
laptop the F keys need Fn held, unless "Use F1, F2, etc. keys as standard
function keys" is on in the keyboard settings. The mouse works on the overlay;
the game does not use it. Keys go to the game unless a text field of the
overlay is being typed in.

The first lines are always there: game, level, tick, ticks and frames per
second, the pause, step and speed controls, a screenshot button and a
switch that starts and stops a frame dump (`openrac_frames.jsonl` in the
working directory). Below are six sections, each folded or open.
`OPENRAC_OVERLAY=frame,renderer,memory,mobys,game,shaders` names the ones
open at the start (`all`: every one; anything else, such as `1`: Frame).

### Frame

- The tick rate measured against the game's own (50 for PAL), and the
  frames shown per second.
- The frame time: average and worst of the last second, a plot of the last
  240 frames, and a plot of the game's own time per tick.
- Where the time goes, averaged over the last second:

  | Row | What is timed |
  |---|---|
  | game tick | the game's own code, the decompiled C, between two of its vertical blanks. Library replacements it calls (disc, sound, pad) count as the game's |
  | scene read from memory | the window reading the tick from game memory: camera, mobys and their poses, the 2D path's packets |
  | blend between ticks | moving the camera, mobys and poses part way from the last tick to this one, for a frame drawn between ticks |
  | render submission (CPU) | the renderer's passes issuing their OpenGL commands |
  | GPU | the same passes timed on the GPU with timer queries (`GL_TIME_ELAPSED`). "not measured" when the context has none. Results come back a few frames late |
  | present + overlay | the frame put in the window; the overlay itself |
  | swap and frame pacing | the buffer swap and whatever the frame pacing waits for after it |
  | wait for the next tick | the sleep until the next tick is due |

- How many frames are drawn per tick, the kind of the last one (`tick`,
  `between`, `held`, `picture`) and its blend alpha; whether frames are
  drawn between ticks; vsync; the display's refresh interval; the
  process's processor load.

### Renderer

- The OpenGL renderer, vendor and version as the context names them, and
  in red when the name is a software rasteriser's (llvmpipe, SwiftShader,
  ...): whether a GPU is drawing is the first thing to rule out when frames
  are slow.
- The last frame's draw calls, triangles, texture uploads (count and
  bytes), buffer uploads, program changes and framebuffer binds.
- A row per pass, in drawing order (the 2D before the world, sky, terrain,
  ties, shrubs, mobys, effects, hud): draw calls, triangles, CPU and GPU
  time, and a switch that stops the pass being drawn.
  Moby GPU time is split into `mobys opaque`, `mobys glow`, `mobys translucent`,
  `mobys effects` and `mobys metal`; the parent row reports CPU time without
  double-counting the nested GPU timings.
- Wireframe; the sizes of the window, the game's picture in it and the
  frame buffers; the GL textures alive and their size; the texture pool's
  entries; the shader programs in use; what the game sent for the 2D path.

The counts are taken where every renderer's GL calls pass, the function
table of `renderer/gl.h` ([gl_stats.h](../../port/renderer/gl_stats.h)), so
a new renderer is counted without doing anything. The overlay's own draws
are not counted. Texture sizes are what was asked of the driver (4 bytes a
pixel, a third more with mipmaps).

### Memory

- The host process: resident memory, the footprint the system charges
  (macOS), the peak.
- The game's 32 MB of main memory: how much holds anything but zero, and a
  map of it, 128 KB a cell: brighter with more in use, orange where it
  changed in the last half second. A click opens that address in the
  memory viewer.
- The game stack in use; a button that writes main memory to
  `ram_<tick>.bin` (as `OPENRAC_DUMP_RAM` does at a chosen frame).
- The memory viewer: 256 bytes from an address, as bytes with their text or
  as words with their float values; `follow` goes to the address the first
  word holds.

The game's own heaps are not shown: their headers are not identified yet.

### Mobys

The live mobys, nearest the hero first, within a radius or all of them:
address, class number and name, state, mode bits, animation sequence and
frame, position, distance and update function by name. The names are the
editor's ([editor/moby_classes.tsv](../../editor/moby_classes.tsv)), built
into the program.

- A click on a row selects the moby: a ring marks it in the picture and
  the inspector opens with its fields (collision sphere, position, state,
  scale, draw bytes, mode bits, lights, rotation, animation, update
  function, pvars, glow colour, class) and all of its 0x100 bytes as words
  and floats, with buttons to open it or its pvars in the memory viewer.
- "labels in the picture" names every listed moby where it stands.
- "click picks": a click in the game's picture selects the moby whose
  origin is nearest (within 40 pixels).

`OPENRAC_OVERLAY=...,inspect` selects the hero and opens the inspector;
`labels` turns the labels on; `memview` opens the memory viewer.

### Game

The level, the tick, the game mode word, the hero's position and state, the
camera (position, forward, field of view, fog), pad 0 as the game reads it
(scripted input included), and the functions without C that were called,
with their counts. A fault in the game's code ends the program, so it is
not shown here: the crash handler's report is in the log.

"probes" lists the on/off `OPENRAC_*` switches the program has read so far
and turns them on and off while it runs: `OPENRAC_DEBUG`,
`OPENRAC_CAM_LOG`, `OPENRAC_PACING`, `OPENRAC_SHOT_BETWEEN`, `OPENRAC_FPS`,
`OPENRAC_HITCH`. A switch appears once the code that reads it has run.

### Shaders

Every shader program: its number, the files it was built from, whether it
linked, how often it was bound this frame, and what failed to compile or
link since the start.

`reload` reads a program's two files again and links them into the same
program, so the renderers holding it draw with the new code at once. The
shaders are built into the program (cmake/Window.cmake), so the files are
read from the source tree the build used, or from `OPENRAC_SHADER_DIR`
(a folder laid out as `port/`: `renderer/shaders/direct.frag`, ...). The
program is only touched once the new sources compile and link in a trial;
otherwise it stays as it was and the compiler's log is shown. Uniform
values are put back by name. A uniform whose location changed, because
uniforms were added or removed, is found again only by renderers that look
locations up each frame.

## Pause, single step, slow motion

The game calls the port once a tick, at its vertical blank. Paused, the
port does not return from that call: it draws the last tick again (frames
of kind `held`) and keeps reading the window's events, so the overlay
stays live. A step lets one tick through. Slow motion makes each tick last
2 to 16 times longer; with frames drawn between ticks (the `frame_rate`
setting other than `original`) the motion stays smooth.

The sound is not paused: it is rendered on the audio thread, so a held
note goes on.

## The frame dump

`--debug-dump FILE`, or `OPENRAC_DEBUG_DUMP=FILE`, writes one JSON object
per line for every frame, with or without a window. Without a window a
frame is a tick and only the game's time and the wait are measured.

```json
{"frame":1299,"tick":1299,"kind":"tick","alpha":1.000,"t":26.770880,"frame_ms":19.994,
 "game_ms":0.309,"gpu_ms":1.801,"draw_calls":2069,"triangles":1122234,"texture_uploads":1,
 "texture_upload_bytes":49152,"buffer_upload_bytes":45600,"program_binds":4,"framebuffer_binds":17,
 "sections":[{"name":"scene","depth":0,"cpu_ms":0.738,"draw_calls":0,"triangles":0},
             {"name":"render","depth":0,"cpu_ms":3.685,"draw_calls":2069,"triangles":1122234},
             {"name":"terrain","depth":1,"cpu_ms":1.450,"gpu_ms":0.438,"draw_calls":1320,"triangles":25748}],
 "gauges":{"mobys":291,"mobys_drawn":288,"ui_quads":2,"effect_quads":81,"packet_bytes":16880,"level":0}}
```

| Key | |
|---|---|
| `frame` | counts every frame measured, from 0 |
| `tick` | the window's frame counter when the frame was drawn, the one `OPENRAC_SHOT` and `OPENRAC_PRESS` count in; without a window, the tick |
| `kind` | `tick`: a tick's own picture. `between`: drawn between two ticks. `held`: the last tick again, paused. `picture`: a full-screen picture the game shows itself (a movie's frame, a boot still) |
| `alpha` | how far the scene is from the last tick's to this tick's (1: this tick's) |
| `t` | seconds since the first frame measured |
| `frame_ms` | from this frame's start to the next one's |
| `game_ms` | the game's own code since the last tick; on `tick` frames only |
| `gpu_ms` | the sum of the sections timed on the GPU, or `null` when none was (no window, no timer queries, or the result did not come back within 16 frames) |
| `draw_calls` ... `framebuffer_binds` | the frame's counters ([stats.h](../../port/common/stats.h)) |
| `sections` | the timed parts in order, `depth` 0 at the top: `scene`, `prepare`, `render` (its passes at depth 1, each with `gpu_ms` when timed), `present`, `overlay`, `swap`, `wait` |
| `gauges` | values of the tick by name: live and drawn mobys, native 2D quads, effect quads, the 2D path's bytes, the level |

A frame is written once its GPU times are back, so the file runs up to 16
frames behind; everything held is written when the program ends.

A scripted session that shows the overlay, pauses, steps one tick and goes
on, with pictures of the window:

```sh
OPENRAC_KEYS="1000:F3,1160:F5,1161+20:F6,1162+20:F5" OPENRAC_OVERLAY_SHOT=100:shots/w_ ...
```

For a performance check: run a fixed number of frames with `--frames N`
and scripted input, then read `frame_ms` and the sections' times, for
example the 99th percentile of `frame_ms` over the `tick` frames of a
level, or the sum of `draw_calls` per pass.

## Measuring from code

[common/stats.h](../../port/common/stats.h) is what the overlay and the dump
read, and any code of the port may use it:

```cpp
stats::count(stats::Counter::DrawCalls);        // a counter of the frame: always on, one add
{ stats::Scope s("mobys", stats::kGpu); ... }   // a timed section; kGpu: timed on the GPU too
stats::lap("scene");                            // a top-level section since the last one ended
stats::gauge("ui quads", quads.size());         // a value of the frame, by name
static const bool& log_it = stats::probe("OPENRAC_MY_LOG", "what it logs");  // a switch the overlay lists
```

Sections, gauges and the frame history are kept only while the overlay is
shown or a dump is open; otherwise each costs the test of one bool. The
section tree follows OpenGOAL's frame profiler.

## Environment variables: the debugging tools

Every variable on this page has a row in the port's table of them,
[port/common/env.cpp](../../port/common/env.cpp) (read through `env.h`), with its kind and what
holds when it is unset. The port's `env` test fails when this page and the table do not name the
same variables, so a new variable gets a row in both.

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_OVERLAY` | `1`, or a list of `frame`, `renderer`, `memory`, `mobys`, `game`, `shaders`, `all`, `inspect`, `labels`, `memview`, `wireframe` | Shows the overlay from the start (`0`: not). The section names open those sections; `inspect` selects the hero and opens the moby inspector, `labels` names the mobys in the picture, `memview` opens the memory viewer, `wireframe` draws the frame in lines. | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_OVERLAY_SHOT` | `N:PREFIX` | With the overlay shown: every `N` ticks, the window with the overlay to `PREFIX<tick>.png`, from `OPENRAC_SHOT_FROM` on; while paused one more, `PREFIX<tick>_held.png`. (`OPENRAC_SHOT` writes the game's frame without the overlay.) | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_KEYS` | `FRAME[+HELD]:KEY,...` | Keys and clicks by script, put in the window's event queue at those window frames, as `OPENRAC_PRESS` presses pad buttons. `KEY` is SDL's name of a key (`F3`, `F5`, `Return`) or `click@X,Y`, a left click at that place of the window (in the window's own units, not pixels). `+HELD`: that many pictures later while the game is paused at `FRAME`, where frames do not count. An event is read on the frame after it was put. | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_MENU_BAR` | any (set) | macOS: the overlay's menus in a bar along the window's top as well, as on Windows and Linux (macOS has them in its own menu bar). | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_DRAWER` | any (set) | The overlay's menus in the drawer, as on Android, in place of the menu bar. | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_GL_API` | `es`, `desktop`, `vulkan` | The kind of OpenGL context: `es` asks for OpenGL ES 3.2, what Android draws with (the default there), and runs the renderer's ES path on a desktop driver; `desktop` the 4.1 core profile (the default elsewhere); `vulkan` the renderer's GL carried out on Vulkan 1.3 (`port/renderer/vulkan`; on macOS through MoltenVK). Where Vulkan cannot start, the log says why and the game draws with OpenGL. | `port/platform/window.cpp` |
| `OPENRAC_VULKAN_VALIDATION` | `1` | With `OPENRAC_GL_API=vulkan`: Khronos' validation layer, where it is installed, its messages in the log. On macOS it needs a Vulkan loader (Homebrew's `vulkan-loader` and `vulkan-validationlayers`, with `VK_LAYER_PATH` set), which is then opened in place of the bundle's MoltenVK. | `port/renderer/vulkan/vkgl.cpp`, `port/platform/window.cpp` |
| `OPENRAC_VULKAN_PIPELINE_CACHE` | path, or `0` / `off` / `none` | With `OPENRAC_GL_API=vulkan`: where graphics pipelines are cached to disk between runs; defaults to `.cache/openrac/vulkan_pipelines_<vendor>_<device>_<key>.bin` (or macOS / Android equivalent), `<key>` naming the code that makes the pipelines so that a changed build starts afresh and removes the older files. `0`, `off` or `none` turns caching off. A file that is not one whole save (cut short, mixed from two programs, or of an older format) is logged and dropped, and the next save replaces it. Beside it, `<file>.manifest` lists the pipelines made, which the next run makes again ahead on a worker thread (`pipeline-prewarm`), and `<file>.spirv` what glslang made, so a warm start compiles nothing (`spirv-disk-cache`, [RENDER_PERF.md](RENDER_PERF.md)); with caching off there is neither. | `port/renderer/vulkan/vkgl_device.cpp`, `pipeline_cache_file.cpp` |
| `OPENRAC_VULKAN_LIBRARY` | path | macOS: the Vulkan library to open (a loader or MoltenVK) in place of the one the application bundle carries. | `port/platform/window.cpp` |
| `OPENRAC_GL_HIDE_EXTENSIONS` | `NAME,...` | The renderer acts as if the driver lacked these GL extensions, to try the fallbacks a phone's driver needs on one that has them (`GL_NV_shader_noperspective_interpolation`, `GL_EXT_clip_cull_distance`). | `port/renderer/gl.cpp` |
| `OPENRAC_HIDE_PASSES` | `NAME,...` | The renderer's passes of those names are not drawn: the names of the Renderer section's rows (`sky`, `terrain`, `ties`, `shrubs`, `mobys`, `effects`, `hud`). | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_DEBUG_DUMP` | `FILE` | The frame dump, as `--debug-dump FILE`. | `port/game/common/main.cpp` |
| `OPENRAC_PAUSE_AT` | `TICK[:FRAMES]` | Pauses the game at that window frame; with `FRAMES`, goes on by itself after that many held pictures. | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_TIME_SCALE` | `X` (1/64 to 1) | The game's speed from the start: `0.25` is quarter speed. | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_SHADER_DIR` | folder | Where a shader reload reads the files, laid out as `port/`; the build's source tree when unset. | `port/game/common/debug/shaders.cpp` |
| `OPENRAC_SHADER_RELOAD` | `TICK` | Reloads every shader program at that window frame, with the result in the log. | `port/game/common/debug_overlay.cpp` |
| `OPENRAC_SCRIPT_BUDGET_MS` | `MS` (a positive number) | The time a script mod may run each time the game calls into it (a hook, a callback, its first run), not counting the game's own code it calls on to; over it, the mod stops ([MODS.md](MODS.md), section 11). Default 2. | `port/game/common/mod_scripts.cpp` |

## Environment variables: the probes

These variables are read by `openrac-<game>` at run time. Unless said otherwise, a variable is on when it is set to anything (including the empty string); the value is not looked at.

### Running and scripting a session

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_DATA` | folder path | The extracted data folder, used when `--data` is not given. Without either, the program prints usage and stops. | `port/game/common/main.cpp` |
| `OPENRAC_DATA_HOME` | folder path | Where the player's memory cards (`memcard/<serial>`) and settings (`settings/<id>.ini`) live, unless `--cards` or `--settings` says otherwise. The launcher sets it to its install folder on Windows, Linux and macOS. Unset: `%USERPROFILE%\OpenRAC` on Windows, `~/Library/Application Support/OpenRAC` on macOS, `$XDG_DATA_HOME/openrac` or `~/.local/share/openrac` elsewhere. | `port/game/common/program/options.cpp` |
| `OPENRAC_DIRECT` | any (set) | Together with `--level N`, goes straight into level N from the title world, as the save page's "continue without saving" starts a new game, skipping the title wait and menus. Without `--level` it does nothing. | `port/game/rac1/host/boot.c` |
| `OPENRAC_PRESS` | `FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],...` | Holds pad buttons on port 0 for scripted runs. `FRAME` is the window frame to start at (world, 2D and movie frames all count), `MASK` is hexadecimal in the pad's own bit values (0x4000 cross, 0x8 start), `FRAMES` the number of frames (default 5). The optional sticks are 0-255, 128 centred, 0 left or up; left stick `LX:LY`, right stick `RX:RY`. | `port/game/common/frontend.cpp` |
| `OPENRAC_WRITE` | `FRAME:ADDRESS:VALUE[:FRAMES],...` | Writes a 32-bit word into the game's memory at each vertical blank from `FRAME` for `FRAMES` blanks (default 1). `ADDRESS` and `VALUE` are hexadecimal; `FRAME` and `FRAMES` decimal. | `port/game/common/main.cpp` |
| `OPENRAC_HIDDEN` | any (set) | With `--window`: the window is made and drawn as always but never shown, so scripted runs open nothing on the screen and several can run at once. Screenshots still work. Without `--window` the game has no pad, terrain collision or disc movies, so it cannot be played by script. | `port/game/common/frontend.cpp` |
| `OPENRAC_SOFTWARE_GL` | any (set) | Runs on a software OpenGL rasteriser instead of refusing it. Such a renderer shows a level at a few frames a second, which looks like a broken port, so without this the game stops with a message. | `port/game/common/frontend.cpp` |
| `OPENRAC_AUDIO_TICKED` | any (set) | No sound device: the game's sounds advance by one tick's samples each tick (rendered and dropped) and streams are read when they start, so what the game gets back from the sound engine lands on the same tick however fast the run goes, and `port/tools/regress`'s runs repeat. | `port/game/common/main.cpp` |
| `OPENRAC_CONTROL_FRAMES` | `N` (decimal) | The run ends `N` window frames after the player gets control (a route's length in `port/tools/regress`). Unset means never. | `port/game/common/frontend.cpp` |
| `OPENRAC_DEBUG_MENU` | `N` (decimal frame) | Opens the developer menu by script at window frame `N` (or later), if the menu is available: `OPENRAC_DEVELOPER` on, and the boot program (title and main menu) loaded. Unset means never. | `port/game/common/debug_menu.cpp` |
| `OPENRAC_DEVELOPER` | `1` | Turns the launcher's developer mode on. The value matters: unset, empty or `0` is off; anything else is on. Without it the menus are the game's own. | `port/game/common/debug_menu.cpp` |

### Screenshots and dumps

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_SHOT` | `EVERY:PATH` | Writes a PNG of each window frame whose number is a multiple of `EVERY` (a positive decimal) to `PATH<frame>.png`; `PATH` is the text after the first colon, so it is a prefix. Without a colon, or with `EVERY` of 0, nothing is written. Only ticks' own frames are written unless `OPENRAC_SHOT_BETWEEN` is set. | `port/game/common/frontend.cpp` |
| `OPENRAC_SHOT_BETWEEN` | any (set) | Also writes the frames drawn between ticks, as `PATH<frame>_b<n>.png`. Needs `OPENRAC_SHOT`. | `port/game/common/frontend.cpp` |
| `OPENRAC_SHOT_FROM` | `FRAME` (decimal) | No tick-path shots before that frame. Default 0. | `port/game/common/frontend.cpp` |
| `OPENRAC_DUMP_SNAPSHOT` | `FILE.png` | Writes the frame as read back for the game's own screen capture (the snapshot the game requests) to that PNG. | `port/game/common/frontend.cpp` |
| `OPENRAC_DUMP_RAM` | `FRAME` (decimal) | At that frame, writes the game's 32 MB of main memory to `ram_FRAME.bin` in the working directory. | `port/game/common/frontend.cpp` |
| `OPENRAC_DUMP_VIF` | `FILE` | Writes the VIF data collected from the frame's display list (the 2D path's DMA chain) to that file, each time the chain is followed. | `port/game/common/frontend.cpp` |
| `OPENRAC_DUMP_TEXTURE` | `BLOCK` (hexadecimal) | Each conversion of a texture whose `TBP0` is that block is written to the working directory as `texture_<block>_<n>.ppm` (RGB) and `.pgm` (alpha). | `port/renderer/texture_pool.cpp` |
| `OPENRAC_DUMP_DRAWS` | `N` or `N-M` (decimal) | Turns on the logging of the 2D path's draws for ticks `N` to `M` (or only `N`). | `port/game/common/frontend.cpp` |

### Logs and traces

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_LOG` | `verbose`, `debug` | `verbose`: the detailed log a player can turn on (the launcher's Detailed logs; `verbose_log = 1` in the settings): level program loads, the first pad's changes, the callers of functions with no C, repeated calls to unknown code addresses. `debug`: everything, each frame's DMA chains and texture uploads too. Unset: info. | `port/common/log.cpp` |
| `OPENRAC_CRASH_TEST` | `KIND:FRAME` | For testing the crash report only: stops the program on purpose at that frame. `segv` (game memory that is not mapped), `host` (a host address), `abort`, `fpe`, `fatal` (`log::fatal`), `trap` (the game's trap). The report goes to `<data>/crashes/` (port/common/crash.h). | `port/game/common/main.cpp` |
| `OPENRAC_TRACE` | `FILE` | Writes one JSON line per tick to that file for `port/tools/regress` ([REGRESSION.md](REGRESSION.md)): the window frame, the frames since control, the level, the game mode, the camera, the hero's position, rotation and state, the live mobys and the layers the game drew. | `port/game/common/frontend.cpp` |
| `OPENRAC_REACH` | `FILE` | Writes the functions without C (and the zero-filled calls) the run reached, with their calls, to that file at exit and every 600 frames, so a run that crashes still leaves most of it (`port/tools/regress reach`). | `port/game/common/main.cpp` |
| `OPENRAC_DEBUG` | any (set) | Every 100th frame, logs the game's state: level, camera, mobys, layers drawn, menu and pad words, fog, load stage, for bring-up. Also gates `OPENRAC_FX_DUMP`. | `port/game/common/frontend.cpp` |
| `OPENRAC_FX_DUMP` | `FRAME` (decimal) | Only read inside the `OPENRAC_DEBUG` report (so every 100th frame): for frames from `FRAME` up to `FRAME`+99, logs each effect quad. | `port/game/common/frontend.cpp` |
| `OPENRAC_MOBY_LOG` | `N[:RADIUS]` | Every `N` ticks (`N` > 0), logs the mobys within `RADIUS` game units of the hero (default 20): class, address, state, mode bits, animation, position, distance, collision radius and update function. | `port/game/common/frontend.cpp` |
| `OPENRAC_FRAME_LOG` | `FROM-TO` or `N` (decimal) | For ticks in that range, logs what each drew: draw flags, camera, 2D bytes, quad, effect and moby counts. | `port/game/common/frontend.cpp` |
| `OPENRAC_CAM_LOG` | any (set) | Each drawn frame, logs the camera position, forward and up vectors. | `port/game/common/frontend.cpp` |
| `OPENRAC_PACING` | any (set) | Each frame shown, logs when (ms), how far between ticks it is (alpha), whether it is a tick or in-between frame, the refresh interval and the draw cost. | `port/game/common/frontend.cpp` |
| `OPENRAC_FPS` | any (set) | Every 300 frames, logs the frames per second over that span. | `port/game/common/main.cpp` |
| `OPENRAC_HITCH` | any (set) | Logs each frame that took more than 1.25 times the frame time, with its duration in ms. | `port/game/common/main.cpp` |
| `OPENRAC_WATCH` | `ADDRESS:COUNT[:i],...` | Each frame, logs `COUNT` values (at most 32) from each hexadecimal game `ADDRESS`: floats by default, words with the third field `i`. `COUNT` is decimal. | `port/game/common/main.cpp` |
| `OPENRAC_GUARD` | `FRAME:ADDRESS:BYTES[:SKIP]` | From window frame `FRAME` on, the first write (after `SKIP` of them) into `BYTES` bytes of game memory at `ADDRESS` ends the program with the host's stack, to find what corrupts them. `FRAME` and `SKIP` are decimal, `ADDRESS` and `BYTES` hexadecimal (default 4). | `port/game/common/main.cpp` |
| `OPENRAC_TRACE_INPUT` | any (set) | Logs a port's buttons and stick values, at most every 25 frames per port. | `port/game/common/frontend.cpp` |
| `OPENRAC_TRACE_CD` | any (set) | Prints each disc read (sector count, start sector, destination address) on stderr. | `port/game/common/lib/cdvd.c` |
| `OPENRAC_TRACE_WAD` | any (set) | Prints each WAD decompression on stderr: its addresses, first four bytes and output size (rac1), or its addresses, compressed and output sizes (rac2, rac4). | `port/game/rac1/host/hand.c`, `port/game/common/lib/wad_routine.c` |
| `OPENRAC_TRACE_MISSING` | any (set) | On the first call to each function that has no C, also prints a host stack trace. Without it only the warning is logged. | `port/runtime/guest.cpp` |
| `OPENRAC_TRACE_RELOCATION` | any (set) | Reports level-code relocation: level addresses with no paired address nearby, searches of the executable's code in a level, and calls and hooks resolved inside level copies. | `port/runtime/guest.cpp` |
| `OPENRAC_TRACE_SIF` | any value but the empty string | Prints the first 32 SIF RPC binds and the first 32 SIF RPC calls on stderr, with their clients and servers. | `port/game/common/lib/kernel.c` |
| `OPENRAC_SOUND_LOG` | any (set) | Prints each command the game sends its sound driver, with its first four words and the reply, on stderr. | `port/game/rac1/host/hand.c` |
| `OPENRAC_HIT_LOG` | any (set) | Prints each hit sphere with a template, and the mobys it listed, on stderr. | `port/game/rac1/host/collision.c` |
| `OPENRAC_COLL_TRACE` | any (set) | Counts collision queries and hits by kind (line, sphere, capsule, sphere-mobys, hero-groups, boot line) and prints the totals on stderr every 2000 queries. | `port/game/rac1/host/collision.c` |
| `OPENRAC_UI_LOG` | any (set) | Logs the caller of each 2D quad routine call and each emitted quad. | `port/game/rac1/host/ui2d.c` |

### Renderer and timing switches

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_UNCAPPED` | any (set) | The frame loop does not wait for the next frame's time, so the game runs as fast as the machine allows; also forces one frame per tick (no interpolation) and no vertical sync. A measure, not a way to play. | `port/game/common/main.cpp`, `port/game/common/frontend.cpp` |
| `OPENRAC_NO_BLEND` | any (set) | Forces one frame per tick (no interpolation between ticks) and no vertical sync, whatever the frame rate setting. | `port/game/common/frontend.cpp` |
| `OPENRAC_FRAME_COST` | `N` (decimal) | Every `N` frames drawn (200 when `N` is 1 or less), logs what a frame cost on average and where: `scene`, `submit`, `gpu` (the switch calls glFinish after each frame, which lowers the frame rate a little) and `show`, with the last frame's draw calls and triangles. | `port/game/common/frontend.cpp` |
| `OPENRAC_SLOW_FRAME` | `MS` | Every frame drawn takes that many milliseconds longer, as on a graphics card too slow for the settings, to check that the game's time does not slow with it. | `port/game/common/frontend.cpp` |
| `OPENRAC_ANIM` | `0` | Draws every moby in its bind pose. Only the value `0` has an effect; unset or anything else animates. | `port/game/common/frontend.cpp` |
| `OPENRAC_ENHANCED` | `1` | Enhanced rendering (probe GI, sun shadows, GTAO: `port/enhanced`), chosen at start for scripts and tests: `1` turns it on, any other value off. Unset or empty, it is on when `OPENRAC_PROBE_GI` is set to something other than `0`. The `enhanced_rendering` setting turns it on as well. | `port/renderer/gl.cpp` |
| `OPENRAC_PROBE_GI` | path to a local `.orpg` bake (`0`: off; unset: the level's `sparse.orpg` in the `--probes` folder, when there is one) | Experimental terrain and moby fragment GI. Rejects a different region/level, changed scene or different bake settings; uncovered fragments retain original lighting. See [probe GI](../../port/enhanced/lighting/README.md). | `port/enhanced/viewer/probe_lighting.cpp`, `port/renderer/gl.cpp` |
| `OPENRAC_PROBE_DENSE` | path to a local `.orpg` bake (`0` or unset: none) | Registers one finer local region. Camera-distance and one-cell spatial fades; sparse fallback on invalid local samples. Bad assets leave the level grid active. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_PROBE_REGIONS` | path to a local JSON manifest; `0` disables sidecar discovery | Registers finer regions via `{"regions":["relative.orpg",...]}`. When unset, discovers `<OPENRAC_PROBE_GI>.regions.json`. Compute builds complete 16×16 tile masks; each fragment chooses up to two containing volumes. Supported resident sets upload once. See [region residency](../../port/enhanced/lighting/README.md#dense-regions). | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_PROBE_SKY` | `level` or `uniform` | The sky the probe bake must have been made with, in place of the bake profile's. Any other value leaves the GI off, with the reason in the log. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_PROBE_SKY_INTENSITY` | a number, 0 or more | The sky intensity the probe bake must have been made with, in place of the bake profile's. A value that is not wholly a finite number of 0 or more leaves the GI off, with the reason in the log. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_VK_UNIFORM_REUSE_OFF` | any (set; unset for the optimised path) | Vulkan profiling reference: redundant uniform writes dirty the default block; ordinary uniform descriptors include each upload offset. Default uses byte comparisons, dynamic uniform offsets and bounded per-frame descriptor reuse. Set before startup. | `port/renderer/vulkan/vkgl_internal.h` |
| `OPENRAC_GPU_MARKERS` | any (set; unset for off) | Names timed GPU passes in AMD RGP and other Vulkan profilers. Requires configuring with `-DOPENRAC_GPU_MARKERS=ON` (default OFF) and active stats collection, such as `--debug-dump`. Label lookup and Vulkan label calls are compiled out otherwise. | `port/renderer/vulkan/vkgl_commands.cpp`, `port/renderer/vulkan/vkgl_device.cpp` |
| `OPENRAC_GPU_DRIVEN_OFF` | any (set) | Reference (Vulkan): instance visibility on the CPU and direct mesh draws, in place of the GPU-driven path. See [GPU rendering](RAC1_GPU_DRIVEN.md). | `port/renderer/vulkan/vkgl.cpp` |
| `OPENRAC_GPU_MATERIALS_OFF` | any (set) | Reference (Vulkan): each mesh's material uniforms and textures bound one by one; takes effect at the next shader reload. | `port/renderer/vulkan/gpu_materials.inc` |
| `OPENRAC_VK_EFFECT_UPLOAD_REUSE_OFF` | any (set) | Reference (Vulkan): every effect texture upload is copied and hashed, and identical ones are not reused. | `port/renderer/effects.cpp` |
| `OPENRAC_VK_PARTICLE_TEXTURE_REUSE_OFF` | any (set) | Reference (Vulkan): the texture of every visible particle is hashed, and none is reused. | `port/game/common/frontend.cpp` |
| `OPENRAC_RENDER_OPT_OFF` | `NAME,...` or `all` | Switches the listed render optimisations of the registry off (`all`: every one), so their reference paths draw; an unknown name is warned of and ignored. Read once, at the first question the draw code asks. The names, what each lowers and how its frames are compared are in [RENDER_PERF.md](RENDER_PERF.md). | `port/renderer/render_opt.cpp` |
| `OPENRAC_PVS_OFF` | any (set) | Profiling reference (Vulkan): no baked camera visibility; the F3 overlay has the switch too. See [Baked camera PVS](#baked-camera-pvs-vulkan-only). | `port/scene/level_renderer.cpp` |
| `OPENRAC_DEPTH_REUSE_OFF` | any (set) | Profiling reference: the GTAO prepass's depth is discarded before the world's colour pass instead of being reused. | `port/renderer/renderer.cpp` |
| `OPENRAC_RENDER_THREAD` | `0` disables | Vulkan world-recording worker with one owned snapshot in flight, enabled by default. Set to `0` for synchronous rendering. UI and presentation stay on the window thread; OpenGL stays synchronous. | `port/game/common/frontend/interpolation.cpp` |
| `OPENRAC_CAMERA_INSTANCE_CULL_OFF` | any (set) | Profiling reference: camera culling of aggregate groups only, with no per-instance frustum planes. | `port/scene/level_renderer.cpp` |
| `OPENRAC_PROBE_TILED_REFERENCE` | `1` | Evaluates every resident volume per fragment instead of tile masks, retaining the same ranking and two-sample GI limit. Correctness reference for tiled culling. | `port/enhanced/viewer/probe_selection.cpp` |
| `OPENRAC_PROBE_COMPUTE_OFF` | `1` | Uses the CPU/frustum selector and two uploaded dense slots instead of compute coverage selection. Also used automatically when compute or full residency is unavailable. | `port/enhanced/viewer/probe_selection.cpp` |
| `OPENRAC_HERO_SHADOW` | any (set) | Enables the separate Ratchet map independently; also enabled automatically with sun cascades. Main light grounded, downward cue airborne. See [hero shadow](../../port/enhanced/docs/RAC1_HERO_SHADOW.md). | `port/game/common/frontend.cpp` |
| `OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE` | any (set; unset for default) | Compiles the original probe sampler for comparison: valid zero-weight corners still read atlas textures. Set before startup. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_SHADOW_FILTER_REFERENCE` | any (set; unset for default) | Compiles the original 36-read sun/hero PCF filter instead of the normal 16-texel kernel. Set before startup; the default path has no depth-tie fallback. See [filtering](../../port/enhanced/docs/RAC1_SUN_SHADOW.md#unique-texel-pcf-filtering). | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_MOBY_SPHERE_CULL_OFF` | any (set; unset for default) | Leaves animated instances unbounded for native camera/cascade culling. Re-evaluated on the next instance upload; rigid meshes keep their native boxes. | `port/scene/level_renderer.cpp` |
| `OPENRAC_SHADOW_SOLID_OFF` | any (set; unset for default) | Uses the original alpha-capable shader for opaque static sun casters. Profiling counters `sun solid draws` / `sun alpha draws` show cascade shader use. See [sun shadows](../../port/enhanced/docs/RAC1_SUN_SHADOW.md#opaque-static-caster-shader). | `port/scene/level_renderer.cpp` |
| `OPENRAC_SHADOW_BATCH_OFF` | any (set; unset for default) | Submits shadow casters in model order instead of grouping GPU commands by shader and skinning mode. Use to compare indirect-call counts and shadow output. | `port/scene/level_renderer.cpp` |
| `OPENRAC_SHADOW_ATLAS_OFF` | any (set; unset for default) | Uses separate shadow targets instead of the indexed-viewport atlas; retains batching across layers. | `port/scene/level_renderer.cpp` |
| `OPENRAC_OPAQUE_COLOUR_BATCH_OFF` | any (set; unset for default) | Restores opaque colour in per-layer order. The default Vulkan material path groups solid and alpha-capable opaque colour across enabled layers when reusing the world depth prepass. | `port/scene/level_renderer.cpp` |
| `OPENRAC_OPAQUE_SPECIALISE_OFF` | any (set; unset for default) | Restores the combined opaque and general static translucent shaders. The specialised Vulkan material path separates static solid, static cutout, opaque moby and static translucent programs, retaining layer-uniform skinning and probe controls. | `port/scene/level_renderer.cpp` |
| `OPENRAC_SHADOW_LAYER_REFERENCE` | any (set; unset for default) | Uses per-layer shadow buckets and separate targets. Detailed sun timing also uses separate targets. | `port/scene/level_renderer.cpp` |
| `OPENRAC_SHADOW_GATHER_OFF` | any (set) | Reference: 16 individual shadow texel fetches in place of gathers; takes effect when the level is loaded again. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_SUN_RECEIVER_CULL_OFF` | any (set) | Reference: square shadow-map caster bounds for the sun's cascades. | `port/scene/level_renderer.cpp` |
| `OPENRAC_SUN_INSTANCE_CULL_OFF` | any (set) | Profiling reference: every instance of a cascade group that survives culling is kept. | `port/scene/level_renderer.cpp` |
| `OPENRAC_SUN_TIMING_DETAIL` | any (set) | GPU timers per cascade and per layer for the sun's shadows, at a higher profiling cost; otherwise the four whole-map timers, which include the clears. | `port/scene/level_renderer.cpp` |
| `OPENRAC_EARLY_LIGHTING_OFF` | any (set) | Reference: full-resolution GI and cascades; takes effect when the level is loaded again. | `port/enhanced/viewer/early_lighting.cpp` |
| `OPENRAC_SUN_SHADOWS` | `0`: off; otherwise on in gameplay | Four sun cascades (nearest 2048, others 1024), 256-unit range, filtered comparisons. Fully shadowed primary and directional fill retain 10%; GI stays independent. Suppresses stencil shadows. See [sun shadows](../../port/enhanced/docs/RAC1_SUN_SHADOW.md). | `port/game/common/frontend.cpp` |
| `OPENRAC_PROBE_RAYS` | integer, default `1024` | Expected bake ray budget for activation. Must match the ray budget used by the bake. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_PROBE_VIEW` | `original`, `combined`, `indirect`, `direct`, `coverage` | Comparison view (default combined). Coverage is green for a usable sample and magenta for fallback. Requires a valid bake. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_PROBE_BACKLIGHT` | `1` | Adds original artistic back-light factors to experimental direct lighting. Default off to match the Lambertian bake. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_PROBE_SPHERES` | `1` or `xray` | Actual probe positions: cyan valid, gold relocated valid, magenta invalid. `1` is depth-tested within five grid spacings; `xray` shows hidden portions translucently within ten spacings. Status colours, independent of irradiance. | `port/enhanced/viewer/probe_lighting.cpp` |
| `OPENRAC_UI_REPLAY` | any (set) | Replays the game's own GS packets for the 2D layer instead of drawing its quads natively, for comparing. | `port/game/common/frontend.cpp` |
| `OPENRAC_UI_SKIP` | `NAME,...` | The listed 2D routines (matched by substring against the list, for example `func_001F5800`) send the game's own packets instead of native quads, for bisecting. | `port/game/rac1/host/ui2d.c` |
| `OPENRAC_UI_RESIDUAL` | any (set) | Logs, once each, the kinds of 2D primitive still drawn from the game's GS packets rather than as native quads. | `port/renderer/direct.cpp` |
| `OPENRAC_SHOW_MISSING` | any (set) | Draws textured primitives whose texture has not been uploaded with the magenta placeholder; by default such draws write nothing. | `port/renderer/direct.cpp` |
| `OPENRAC_NO_PUMP_FIBER` | any (set) | Windows only: runs the game and the window pump in one loop instead of in separate fibers, for comparing. | `port/platform/pump.cpp` |

### Other

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_STRICT` | `1` | Stops with a fatal error on a call to a code address that no function of the port has, instead of going to the unknown-function handler. Only a value starting with `1` counts. | `port/runtime/guest.cpp` |
| `OPENRAC_ROUND_NEAREST` | any | The game's thread keeps the host's rounding to nearest instead of the EE's toward zero, to compare ([HARDWARE_SEMANTICS.md](HARDWARE_SEMANTICS.md), item 1). | `port/game/common/main.cpp` |
| `OPENRAC_DENORMALS` | any | The game's thread keeps the host's denormal floats instead of reading and making them as zero, as the EE does, to compare (item 1). | `port/game/common/main.cpp` |
| `OPENRAC_GPU` | `integrated` | Linux: leaves the choice of graphics card to the system. Otherwise the port asks for the discrete card (NVIDIA's PRIME offload when its driver is loaded, else Mesa's `DRI_PRIME` for the card with the most memory) unless `DRI_PRIME` is already set. | `port/platform/window.cpp` |
| `OPENRAC_GPU_DRIVER_DIR` | folder | Android: the folder of the GPU driver package chosen in the launcher ([ANDROID.md](ANDROID.md)); set by the launcher. | `port/android/gpu_driver.cpp` |
| `OPENRAC_GPU_DRIVER_LIBRARY` | file name | Android: the package's Vulkan library, in `OPENRAC_GPU_DRIVER_DIR`. Without both, the game uses the system's driver. | `port/android/gpu_driver.cpp` |
| `OPENRAC_UI_COLORS` | `NAME=#RRGGBB[AA],...` | The launcher's theme for the options drawer, in app.css's names without their dashes (`panel=#1d1d28,amber=#ffb443,...`); set by the launcher from Settings, Theme, and on Android the game's accent (`launcher/core/src/theme.rs`). Unknown names and malformed values keep the launcher's own colours. | `port/game/common/ui_colors.cpp` |
| `OPENRAC_NATIVE_LIBRARY_DIR` | folder | Android: the app's native library folder, where libadrenotools finds its hook libraries; set by the launcher. | `port/android/gpu_driver.cpp` |

### Build-time

| Variable | Value | What it does | Read in |
| --- | --- | --- | --- |
| `OPENRAC_CLANG` | path to a `clang` binary | The Clang that hostgen uses to read the decompilation's C. Without it, hostgen takes the first of `clang`, `clang-18`, `clang-17`, `clang-19`, `clang-20` on `PATH`. | `port/tools/hostgen/clangast.py` |

### Vulkan uniform and descriptor submission

Default uniforms are compared byte for byte before becoming dirty. Changed
constants receive immutable frame-ring uploads; a dynamic uniform-buffer offset
selects each draw's snapshot without changing resource descriptors. Named
uniform/storage blocks keep their binding semantics. Descriptor keys include
buffers, block offsets/ranges, sampler views and sampler state. Each program
keeps up to 256 combinations per frame plus its latest set. Frame changes retire
keys; relinking clears the cache. Frame-slot fences control pool and ring reuse.
Genuine constant changes upload the whole default block.

`--debug-dump` includes `vk_uniform_uploads`, `vk_uniform_upload_kb`,
`vk_uniform_writes_skipped`, `vk_descriptor_sets` and `vk_descriptor_reuses`.
`vkgl::submission_counters()` exposes cumulative counters to native fixtures.

```sh
ctest --test-dir port/build/release -R '^vulkan_uniform(_reference)?$'
```

The synthetic checks cover snapshots, texture switches, arrays/transposes,
ignored locations, frame slots, readback flushes and upload-ring rollover.

### Vulkan GPU timings

Vulkan scope boundaries both use pipeline BOTTOM timestamps. They measure
completion intervals and can include dependency waits; they do not isolate
active shader execution. Adjacent work can overlap inside the GPU pipeline.
CPU and GPU times also overlap and should not be added. See
[Vulkan timestamp semantics](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdWriteTimestamp.html).

Moby/tie GPU subpasses have separate timers; their parent rows report CPU time
to avoid double-counting. Optional GPU labels require an
`OPENRAC_GPU_MARKERS=ON` build, the runtime switch and active stats collection.

### Probe sampling

The sampler skips zero trilinear weights before metadata reads, atlas reads
after a zero behind-surface guard, and irradiance reads when distance visibility
reduces weight to zero. Valid/facing support is accumulated before visibility
rejection: blocked dense samples must stay dark instead of restoring sparse GI.

Use original, indirect and coverage views to distinguish compatibility, placement
and visibility problems. Green coverage is not proof of correct illumination.
See the [GI guide](../../port/enhanced/docs/RAC1_PROBE_GI.md#limits-and-troubleshooting).

### Game moby bounds in native culling

The frontend converts the game's animation-maintained sphere at moby +0x00 from
packed to world units, including radius. Skinned instances use its padded box
for camera/cascade tests; rigid meshes keep tighter transformed boxes. Sphere
metadata stays on the CPU. Camera compaction preserves opaque, glow, translucent
and additive order. Metal keeps the full list because its shader shifts depth.
Cascade culling uses its own light frustum, not camera survivors.

Missing, nonpositive or nonfinite bounds stay visible. Replacement meshes,
debug stand-ins and objects with their own cameras do not receive authored
bounds. Interpolation encloses both endpoint poses and the displayed transform's
stretch, including nonuniform scale/shear. Missing endpoints or singular
transforms retain the instance. `OPENRAC_MOBY_SPHERE_CULL_OFF` selects unbounded
animated instances for comparison.

### Baked camera PVS (Vulkan only)

`OPENRAC_PVS_OFF=1` disables baked camera occlusion; F3 also exposes the switch.
OpenGL/ES do not use it. RAC1 exports contain optional `pvs.json` / `pvs.bin`;
add them to an existing export with:

```sh
python editor/occlusion.py <owned-disc.iso> <port-levels-directory>
```

Terrain mappings check counts/triangle keys for staleness. Ties use placement
keys; live mobys use +0x36, with invalid keys/spawned objects visible. Shrubs have
no mapping. Geometry overrides and edited tie/moby placements disable baked PVS.

Each camera selects a 128-byte mask for its 4-unit cell, conservatively unioning
adjacent cells when the exact cell is absent. Missing/invalid data keeps objects
visible; no mask carries across levels. UI/render-to-texture mobys use their own
camera. PVS does not affect gameplay, animation or shadow-caster submission.

Camera passes use separate compact buffers, including transparent/metal passes.
Terrain indices are cached by mask, preserving opaque material runs and blended
order; shadow maps keep the original indices. PVS data is excluded from GI
fingerprints, so adding it does not require rebaking lighting.

Frame dumps expose `pvs enabled`, `pvs <layer> instances culled` and
`pvs <layer> primitive passes culled`. The last counts primitive/pass pairs,
not unique fragments. Counts precede frustum rejection and may overlap it.

### Repeatable rendering comparisons

Keep scene, camera, inventory, resolution, settings and probe inputs matched.
Wait for loading, automatic movement and pipeline creation to settle. Exclude
capture/readback frames from timing, and compare both run orders. Report geometry,
CPU submission and per-pass GPU time alongside frame intervals.

`port/tools/perf/orxon_start.py` automates a neutral-stick RAC1 NTSC Orxon ship-exit
scene, grants the required inventory and checks stable hero/camera state:

```sh
python port/tools/perf/orxon_start.py --exe <game.exe> --data <game-data> --levels <port-levels> --probes <orxon.orpg> --out <new-results-directory>
```

Set `--sky-intensity` to match the supplied bake. The tool checks bake activation,
PVS, depth reuse and timing availability. Use the same warmup/sample window for
both paths and keep generated output untracked. For backend switches, counters
and resource lifetime, see [GPU rendering](RAC1_GPU_DRIVEN.md).
