# Reference: the port's knobs

Every switch a runner or a tool can use. Verified against the tree at commit
`b522692a`.

## 1. Command line

Parsed by `Options parse(int argc, char** argv)`
(`port/game/common/main.cpp:216-286`); the struct is at `:115-130`.

| Flag | Meaning | Default |
|---|---|---|
| `--data <dir>` | The extractor's data folder (the disc image and the game's executable). Falls back to `$OPENRAC_DATA`. | required |
| `--cards <dir>` | The memory-card folder. Port 0 is `<dir>`, port 1 is `<dir>.slot2`. A card is a plain folder of files. | `<data home>/openrac/memcard/<serial>` |
| `--no-card` | No memory card in either port. | off |
| `--window` | Open the window and run the game with graphics and input. Without it the game runs headless (no pad, no collision, no disc movies). | off |
| `--mods <dir>` | Levels edited in the editor and packed, read *instead of* the disc's. | none |
| `--settings <file>` | The player's settings INI. | `<data home>/openrac/settings/<id>.ini` |
| `--plugins <file>` | The native mods' list (the manifest). | `<data>/plugins/plugins.json` |
| `--no-mods` | **Purist mode**: load no native mod at all. | off |
| `--levels <dir>` | The extracted levels the window draws (`level_00` … `level_18`, `level_98` flight world, `level_99` title world). | `<data>/port/<version>` |
| `--frames N` | Run N ticks, then exit cleanly. | -1 (forever) |
| `--level N` | The level a new game starts in (0–18, the game's own numbering). | -1 (the game's own first level) |
| `--skip-movies` | Movie playback returns the skipped result immediately. Every skipped movie is logged. | off |
| `--keep-going` | Keep going when a function has no C yet (log a warning, return). | **on** |
| `--stop-on-missing` | Stop at the first function with no C and name it. | off |
| `--debug-dump <file>` | Write each frame's measurements as JSON lines. Works headless. | none |
| `--debug-overlay` | Show the F3 overlay from the start. | off |
| `--check-levels` | Read every extracted level as the window would, report problems, write `verified-<format>.json`. Needs a windowed build. | off |
| `--help`, `-h` | Print the usage and exit 64. | — |

> The usage string (`main.cpp:151`) is incomplete: it omits `--no-card`,
> `--window`, `--levels`, `--keep-going` and `--check-levels`. All work.

Separate binaries: `openrac-viewer` (a level with a free camera:
`--camera x y z [rx ry]`, `--state FILE`, `--screenshot FILE`, `--frames N`,
`--no-vsync`) and `openrac-extract` (the extractor).

## 2. Environment variables

Read with `std::getenv` throughout `port/`. The authoritative list is
`docs/port/DEBUGGING.md:247-346`; this is the code-verified one.

### Running and scripting a session

| Variable | Value | Meaning |
|---|---|---|
| `OPENRAC_DATA` | folder | The data folder when `--data` is absent. |
| `OPENRAC_DIRECT` | set | With `--level N`: straight into level N from the title. Without `--level` it does nothing. |
| `OPENRAC_PRESS` | `FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],...` | Scripted pad input on port 0. See [PRACTICE.md](PRACTICE.md) §4. |
| `OPENRAC_WRITE` | `FRAME:ADDRESS:VALUE[:FRAMES],...` | Write a 32-bit word into game RAM at each vertical blank. |
| `OPENRAC_CONTROL_FRAMES` | N | End the run N window frames after the player got control. |
| `OPENRAC_HIDDEN` | set | With `--window`: make and draw the window but never show it. |
| `OPENRAC_DEVELOPER` | `1` | The launcher's developer mode: the Options rows, the Planets page, the **F1 level select**. |
| `OPENRAC_AUDIO_TICKED` | set | No sound device: the sounds advance one tick's samples per tick. **Makes runs bit-reproducible** — the flag a deterministic save-state system wants. |
| `OPENRAC_REACH` | FILE | Write the functions-without-C the run reached, and their call counts. |
| `OPENRAC_DEBUG_MENU` | N | Open the developer level menu by script at window frame N. |

### The overlay and measurement tools

| Variable | Value | Meaning |
|---|---|---|
| `OPENRAC_OVERLAY` | `1`, or a list (`frame,renderer,memory,mobys,game,shaders`), or `all` | Show the overlay from the start, with those sections open. Also `inspect`, `labels`, `memview`, `wireframe`. |
| `OPENRAC_OVERLAY_SHOT` | `N:PREFIX` | Every N ticks, write the window to `PREFIX<tick>.png`. |
| `OPENRAC_KEYS` | `FRAME[+HELD]:KEY,...` | Scripted keyboard/mouse into the window's event queue. The only way to script the overlay itself. |
| `OPENRAC_PAUSE_AT` | `TICK[:FRAMES]` | Pause the game at that tick; with `FRAMES`, resume after that many held pictures. |
| `OPENRAC_TIME_SCALE` | X (1/64 … 1) | The game's speed from the start. |
| `OPENRAC_SHADER_DIR` | folder | Where a shader reload reads its files. |
| `OPENRAC_SHADER_RELOAD` | TICK | Reload every shader program at that tick. |
| `OPENRAC_HIDE_PASSES` | `NAME,...` | Do not draw the renderer's passes of those names (`sky, terrain, ties, shrubs, mobys, effects, hud`). |
| `OPENRAC_DEBUG_DUMP` | FILE | The frame dump, as `--debug-dump FILE`. |
| `OPENRAC_SCRIPT_BUDGET_MS` | MS | The time a script mod may run per call into it. **Default 2 ms.** |
| `OPENRAC_FRAME_COST` | N (default 200) | Every N frames, log where a frame's time went. |
| `OPENRAC_SLOW_FRAME` | MS | Every frame drawn takes that much longer. |

### Logs, traces and dumps

| Variable | Value | Meaning |
|---|---|---|
| `OPENRAC_LOG` | `verbose` \| `debug` | Log verbosity. |
| `OPENRAC_DEBUG` | set | Every 100th frame, log the game's state. |
| `OPENRAC_CAM_LOG` | set | Each drawn frame, log the camera's position and rows. |
| `OPENRAC_PACING` | set | Each frame shown: when, alpha, tick vs between, refresh interval, draw cost. |
| `OPENRAC_FPS` | set | Every 300 frames, log the frames per second over that span. |
| `OPENRAC_HITCH` | set | Log each tick that took over 1.25× the tick time. |
| `OPENRAC_WATCH` | `ADDRESS:COUNT[:i],...` | Each frame, log COUNT values (≤32) from each hex game address: floats by default, `i` for words. |
| `OPENRAC_TRACE_INPUT` | set | Log port 0's buttons and sticks, at most every 25 frames. |
| `OPENRAC_FRAME_LOG` | `FROM-TO` or `N` | For ticks in range, log what each drew. |
| `OPENRAC_MOBY_LOG` | `N[:RADIUS]` | Every N ticks, log the mobys near the hero. |
| `OPENRAC_FX_DUMP` | FRAME | Inside the `OPENRAC_DEBUG` report: log each effect quad for 100 frames from FRAME. |
| **`OPENRAC_TRACE`** | FILE | One JSON line per tick — the regress harness's trace. |
| **`OPENRAC_DUMP_RAM`** | FRAME | Write the game's 32 MB of main memory to `ram_FRAME.bin`. |
| `OPENRAC_DUMP_SNAPSHOT` | FILE.png | Write the frame as read back for the game's own screen capture. |
| `OPENRAC_DUMP_VIF` | FILE | Write the VIF data of the frame's display list. |
| `OPENRAC_DUMP_TEXTURE` | BLOCK (hex) | Write each conversion of a texture with that TBP0. |
| `OPENRAC_DUMP_DRAWS` | `N` or `N-M` | Log the 2D path's draws for those ticks. |
| `OPENRAC_SHOT` | `EVERY:PATH` | A PNG of each window frame whose number is a multiple of EVERY. |
| `OPENRAC_SHOT_BETWEEN` | set | Also write the frames drawn *between* ticks. |
| `OPENRAC_SHOT_FROM` | FRAME | No tick-path shots before that frame. |
| `OPENRAC_CRASH_TEST` | `KIND:FRAME` | Testing only: stop on purpose at that frame. |
| `OPENRAC_GUARD` | `FRAME:ADDRESS:BYTES[:SKIP]` | From FRAME on, the first write into those bytes ends the program with a host stack. |
| `OPENRAC_TRACE_MISSING` | set | On the first call to each function without C, print a host stack trace. |
| `OPENRAC_TRACE_RELOCATION` | set | Report level-code relocation details. |
| `OPENRAC_STRICT` | `1` | Fatal on a call to an unknown code address. |
| `OPENRAC_TRACE_CD` | set | Print each disc read on stderr. |
| `OPENRAC_TRACE_WAD` | set | Print each WAD decompression on stderr. |
| `OPENRAC_TRACE_SIF` | set | Kernel SIF tracing. |
| `OPENRAC_SOUND_LOG` | set | Print each sound-driver command and reply. |
| `OPENRAC_HIT_LOG` | set | Print each hit sphere with a template and the mobys it listed. |
| `OPENRAC_COLL_TRACE` | set | Count collision queries by kind, print totals every 2000 queries. |
| `OPENRAC_UI_LOG` | set | Log the caller of each 2D quad routine and each emitted quad. |

### The renderer and the clock

| Variable | Value | Meaning |
|---|---|---|
| **`OPENRAC_UNCAPPED`** | set | The frame loop does not wait for the next tick's time: **the game runs as fast as the machine allows.** Also forces one frame per tick and no vsync. "A measure, not a way to play." |
| `OPENRAC_NO_BLEND` | set | One frame per tick and no vsync, whatever the frame-rate setting. The tick still waits for its 60 Hz deadline. |
| `OPENRAC_ANIM` | `0` | Draw every moby in its bind pose. |
| `OPENRAC_UI_REPLAY` | set | Replay the game's own GS packets for the 2D layer instead of native quads. |
| `OPENRAC_UI_SKIP` | `NAME,...` | The listed 2D routines send the game's own packets instead of native quads. |
| `OPENRAC_UI_RESIDUAL` | set | Log the kinds of 2D primitive still drawn from the game's packets. |
| `OPENRAC_SHOW_MISSING` | set | Draw textured primitives whose texture is not uploaded with a magenta placeholder. |
| `OPENRAC_GL_API` | `es` \| `desktop` \| `vulkan` | The kind of OpenGL context. |
| `OPENRAC_GL_HIDE_EXTENSIONS` | `NAME,...` | Pretend the driver lacks these GL extensions. |
| `OPENRAC_VULKAN_VALIDATION` | `1` | Khronos validation layer messages. |
| `OPENRAC_VULKAN_PIPELINE_CACHE` | path, or `0`/`off` | Where Vulkan pipelines are cached. |
| `OPENRAC_VULKAN_LIBRARY` | path | macOS: the Vulkan library to open. |
| `OPENRAC_SOFTWARE_GL` | `1` | Allow a software rasteriser. |
| `OPENRAC_GPU` | `integrated` | Leave the graphics-card choice to the system (Linux). |
| `OPENRAC_NO_PUMP_FIBER` | set | Windows: run the game and the window pump in one loop. |
| `OPENRAC_MENU_BAR` | set | Show the in-window menu bar even on macOS. |
| `OPENRAC_DRAWER` | set | Use the touch drawer instead of the menu bar. |
| `OPENRAC_ROUND_NEAREST` | set | Keep the host's round-to-nearest instead of the EE's round-toward-zero. |
| `OPENRAC_DENORMALS` | set | Keep host denormals instead of the EE's flush-to-zero. |

**Probes**: `OPENRAC_CAM_LOG`, `OPENRAC_PACING`, `OPENRAC_SHOT_BETWEEN`,
`OPENRAC_FPS`, `OPENRAC_HITCH` and `OPENRAC_DEBUG` are registered as probes
and appear in the overlay's Game section with a checkbox to flip them while
running (`port/game/common/debug_overlay.cpp:1138-1149`).

## 3. The settings file

`<data home>/openrac/settings/<id>.ini` by default, or `--settings FILE`.
`data home` is `$XDG_DATA_HOME/openrac` or `~/.local/share/openrac` on Linux
and `~/Library/Application Support/OpenRAC` on macOS.

Syntax: `key = value` a line, `#` for a comment, whitespace trimmed. An
unknown *value* warns in the log and falls back to the default; an unknown
*key* is silently ignored (`port/game/common/settings.cpp:241-243`).

| Key | Values | Default | Note |
|---|---|---|---|
| `display_mode` | `windowed`, `borderless`, `fullscreen` | `windowed` | |
| `fullscreen` | `0`/`1` | — | Legacy: `1` means `borderless`. |
| `monitor` | `0`–`64` | `0` | |
| `resolution` | `desktop` or `WIDTHxHEIGHT` | desktop | Fullscreen mode. |
| `window_size` | `WIDTHxHEIGHT` | 1280×960 | |
| `aspect` | `4:3`, `16:10`, `16:9`, `21:9`, `32:9`, `window`, `custom` | `4:3` | A wider picture shows more to the sides. |
| `aspect_custom` | `X:Y` | `16:9` | Clamped to at least 4:3. |
| `hud` | `centre`, `edges` | `centre` | Where the HUD goes in a wide picture. |
| **`render_scale`** | 25–400 (%) | `100` | **Lower is faster.** Never above 8192 pixels a side. |
| **`msaa`** | `off`, 1, 2, 4, 8, 16 | `off` | Held to `GL_MAX_SAMPLES`. |
| **`anisotropy`** | `off`, 1, 2, 4, 8, 16 | 1 (trilinear) | |
| **`vsync`** | `off`, `on`, `adaptive` | `on` | |
| **`frame_rate`** | `display`, `original`, `unlimited`, or 20–1000 | `display` | **`original` = one frame a tick.** See [TIMING.md](TIMING.md) §3. |
| `show_fps` | `0`/`1` | `0` | The frame-rate counter (F2 toggles it). |
| `menu_bar` | `0`/`1` | `1` | |
| `mute_unfocused` | `0`/`1` | `0` | |
| `skip_cutscenes` | `0`/`1` | `1` | Start also presses the game's own cutscene-skip buttons. |
| `volume` | 0–100 | `100` | |
| `verbose_log` | `0`/`1` | `0` | |
| `fix_wide_moby_effects` | `0`/`1` | `1` | A port fix; `0` restores the console's behaviour. |
| `fix_wide_nanotech_orbs` | `0`/`1` | `1` | |
| `fix_wide_liquids` | `0`/`1` | `1` | |

Every one of these except `volume`, `verbose_log`, `menu_bar` and the three
fixes is a row of the game's own **Options → Video** page, drawn by the game's
widgets, and saved as it is changed
(`port/game/common/options_menu.cpp`, the rows enum at `options_menu.h:17-33`).

A file with `frame_rate` but no `vsync` gets `vsync = on` when
`frame_rate == display` and `off` otherwise (`settings.cpp:245-247`).

## 4. The keys

| Key | |
|---|---|
| **F1** | Level select (developer mode only) |
| **F2** | The frame-rate counter |
| **F3** | The debug overlay |
| **F5** | Pause / resume |
| **F6** | One tick forward |
| **F7** | Slower (down to 1/16) |
| **F8** | Faster (back to 1×) |
| **F9** / **F10** | Volume down / up |
| **F11** | Full screen |
| **F12** | Screenshot |
| **Alt** | Hide and show the in-window menus (on its own) |
| **Alt+Return** | Fullscreen |

## 5. The input map

`port/platform/input.cpp:20-39`, documented at `port/platform/input.h:9-13`.
There is no rebinding in the port; the `keyboard-mouse` mod adds one.

| PS2 | Keyboard |
|---|---|
| Left stick | `W` `A` `S` `D` |
| Right stick | `I` `J` `K` `L` |
| D-pad | The arrow keys |
| Cross | `Space` |
| Square | `F` |
| Circle | `E` |
| Triangle | `R` |
| Start | `Return` or `Escape` |
| Select | `Backspace` |
| L1 | Left Shift |
| R1 | `Q` |
| L2 | `1` |
| R2 | `3` |
| L3 | `C` |
| R3 | `V` |

A gamepad is mapped by position, so the bottom face button is always Cross
(`input.cpp:47-62`). The mouse is not a game control — the game does not use
it — but mods can read it, and the `keyboard-mouse` mod turns it into a
camera.

A PlayStation 5 DualSense works through the same path. The port does not ask
what kind of pad it is, does not use the touchpad, the gyroscope or the light
bar, and gives the game no rumble (the retail game asks for none either).
See [CONTROLLERS.md](../CONTROLLERS.md).

The PS2 pad's button word is **active low**: a held button reads 0
(`port/platform/pad.h:14-17`, `:48-65`).

| Bit | Button | Bit | Button |
|---|---|---|---|
| `0x0001` | Select | `0x0100` | L2 |
| `0x0002` | L3 | `0x0200` | R2 |
| `0x0004` | R3 | `0x0400` | L1 |
| `0x0008` | Start | `0x0800` | R1 |
| `0x0010` | Up | `0x1000` | Triangle |
| `0x0020` | Right | `0x2000` | Circle |
| `0x0040` | Down | `0x4000` | Cross |
| `0x0080` | Left | `0x8000` | Square |

## 6. The game's own addresses a tool may want

From the port's own host code (`port/game/rac1/host/front_end.c:35-54`),
PAL unless noted.

| Address | What |
|---|---|
| `0x0015EE84` | The level being played |
| `0x0015F6E8` | The front end's game-mode word |
| `0x0015EE98` | The player's bolts (NTSC-U: `0x0015ED98`) |
| `0x0013F450` | The hero (NTSC-U: `0x0013F350`) |
| `0x0013F4D0` / `D4` / `D8` | The hero's position x / y / z |
| `0x0013F4E8` | The hero's yaw |
| `0x0013CA40` | The pad the window reads |
| `0x0016044C` | The video-mode byte (the port holds it at 0) |
| `0x0015EE7C` | The game's tick length in seconds (NTSC-U: `0x0015ED7C`) |
| `0x0015EE60` | The game's tick scale (NTSC-U: `0x0015ED60`) |
| `0x0015EE80` | The game's region flag (1 = PAL) |

The hero's fields are also exposed by name to script mods:
`hero.position.x/y/z`, `hero.facing`, `hero.move_yaw`, `hero.state`,
`bolts` (`sdk/bindings/rac1.json`, the `fields` table).

## 7. Where the mod system stands

| | |
|---|---|
| **SDK version** | 1.3.0 (`sdk/VERSION`); the interface's major is 1, minor 3 |
| **Hook points, rac1** | **654** on both discs (`port/game/rac1-pal/libraries.tsv`) |
| **Hook points, rac2/3/4** | **0** — a mod for them is refused |
| **Script binding** | rac1 only; the others use `script_binding_none.c` and refuse script mods |
| **Script budget** | 2 ms per call, `OPENRAC_SCRIPT_BUDGET_MS` |
| **Per-tick cost with no mods** | One test of a bool (`mods.h:67-71`) |
| **The mod list** | <https://github.com/OpenRAC/rac1-mods> |

The full hook table with signatures and parameters is generated into
[`sdk/docs/reference/hooks-rac1.md`](../../../sdk/docs/reference/hooks-rac1.md),
and the API into
[`sdk/docs/reference/c-api.md`](../../../sdk/docs/reference/c-api.md) and
[`cpp-api.md`](../../../sdk/docs/reference/cpp-api.md).

## 8. The other port binaries

| Binary | For |
|---|---|
| `openrac-viewer` | A level, extracted from the player's disc, drawn natively with a free camera. `--camera x y z [rx ry]`, `--state FILE` (a memory image), `--screenshot FILE`, `--frames N`, `--size WxH`, `--no-vsync`, `--no-lighting`. |
| `openrac-extract` | The extractor: `--game`, `--proj-path`, `--extract`, `--validate`, `--decompile`, `--compile`, `--json`. |
| `port/tools/regress/` | The regression harness: plays every level by route in hidden windows and compares the traces. See [REGRESSION.md](../REGRESSION.md). |
