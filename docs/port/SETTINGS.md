# The PC settings

What the port adds to a game's own options for a PC's display: the settings,
their keys in the settings file, how the window paces its frames against the
game's tick, what was found about the PAL game's 50/60 Hz option, the graphics
card checks, and where a frame's time goes. Section 6 compares the
settings with what OpenGOAL and ReRAC give their players; section 8 the game
fixes, which switch the port's changes back to the console's behaviour.

Code: [settings.h](../../port/game/common/settings.h) (the settings and the
file), [options_menu.cpp](../../port/game/common/options_menu.cpp) (the rows
on the game's Video options page),
[frontend.cpp](../../port/game/common/frontend.cpp) (the window, pacing,
`apply_settings`), [window.h](../../port/platform/window.h) (SDL: display
modes, the context, the graphics card).

## 1. The settings

The file is `<data home>/openrac/settings/<id>.ini` (`--settings FILE` names
another), `key = value` a line, `#` for a comment. It is read at start and
written whenever a setting changes. An unknown value is reported in the log
and the default used; an unknown key is ignored.

In Ratchet & Clank (PAL) every setting but the last two is a row of the title
menu's **Options, Video** page. The page shows four rows at a time and moves
with the cursor. **Cross** or **Right** moves a row to its next value,
**Left** to the one before. A change is applied and saved at once: no setting
needs the game started again.

| Row | Key | Values (default first) | What it does |
|---|---|---|---|
| Display mode | `display_mode` | `windowed`, `borderless`, `fullscreen` | A window; a window covering the desktop at the desktop's own mode; or the display taken for the game at `resolution`. F11 and Alt+Return go between the window and the fullscreen kind last used |
| Monitor | `monitor` | `0`, `1`, `2`, ... | The display borderless and fullscreen cover, and the one the window opens on: `0` is wherever the window is, `1` the first display SDL lists (the primary one) |
| Resolution | `resolution` | `desktop`, `WIDTHxHEIGHT` | The display's mode in `fullscreen`. The page offers the modes the chosen display lists; a size it does not have becomes the closest it has |
| (the window) | `window_size` | `WIDTHxHEIGHT` | The window's size when last sized, kept for the next start. Absent: 1280 x 960, or as wide as `aspect` makes it |
| Picture | `aspect` | `4:3`, `16:10`, `16:9`, `21:9`, `32:9`, `window`, `custom` | The picture's shape. A wider one keeps the game's vertical field of view and shows more of the world to the sides; the seas, lava and sand the game's draw callbacks send are culled against the wider view too ([rac1/host/wide_view.c](../../port/game/rac1/host/wide_view.c)). `window`: whatever the window's shape is (never narrower than 4:3). `custom`: `aspect_custom` |
| | `aspect_custom` | `16:9`, any `X:Y` | The shape `aspect = custom` uses, `64:27` for one; a shape narrower than 4:3 is held to 4:3 |
| HUD | `hud` | `centre`, `edges` | In a picture wider than 4:3: the HUD where the game put it, in the 4:3 middle, or the elements the game anchors to the left or right of the screen (the weapon, the bolts, the side meters) out at the picture's sides. Menus and text stay in the middle either way |
| Render scale | `render_scale` | `100`, `50`, `75`, `125`, `150`, `200` (a file may hold 25 to 400) | The frame's resolution as a percentage of the picture's pixels in the window. Over 100 is supersampling; under it is for a slow graphics card. The frame is never larger than 8192 pixels a side |
| Anti-aliasing | `msaa` | `1` (`off`), `2`, `4`, `8`, `16` | Samples a pixel (MSAA). The page offers what the card gives (`GL_MAX_SAMPLES`); a file's larger number is held to it |
| Enhanced rendering (experimental) | `enhanced_rendering` | `0` (default), `1` | Whether the game makes the enhanced renderer at all (probe lighting, modern sun/Ratchet shadows, GTAO; [port/enhanced](../../port/enhanced/README.md)). Read at start; Vulkan only. Off, none of it is made and the game draws exactly as a build without it. The launcher's Mods tab writes it (**Enhanced graphics**, a built-in entry with the keys below on its page) and then plays with Vulkan (`OPENRAC_GL_API=vulkan`) and the player's probe bakes (`--probes <data>/probes/<version>`), which it makes from their own extracted levels. From a shell, `OPENRAC_ENHANCED=1` does the same as the key; `OPENRAC_ENHANCED=0` keeps it off |
| Enhanced graphics (F4) | `enhanced_graphics` | `1` (default), `0` | With enhanced rendering on: the in-game master switch for probe lighting, modern sun/Ratchet shadows and GTAO. Turning it off restores original lighting/stencil shadows and bypasses AO without changing individual preferences |
| Probe lighting | `probe_lighting` | `1` (default), `0` | Uses the verified level bake when loaded; toggle in the Enhanced menu. Disabling retains the bake for immediate switching back |
| Probe spheres | `probe_spheres` | `0` (default), `1` | Enhanced menu placement overlay: cyan sparse, green dense, gold relocated, magenta invalid. Dense sphere opacity follows the region fade; only the two selected dense regions are shown |
| Modern shadows | `enhanced_shadows` | `1` (default), `0` | Independently toggles sun cascades and Ratchet's shadow map in the Enhanced menu. Off restores original stencil shadows while keeping GI and GTAO preferences |
| Probe normal bias | `probe_normal_bias` | `0.0` to `2.0` world units (default `0.5`) | Offsets the GI visibility receiver along the camera-facing surface normal; updates immediately |
| Probe view bias | `probe_view_bias` | `0.0` to `2.0` world units (default `0.5`) | Offsets the GI visibility receiver toward the camera; updates immediately |
| Probe GI multiplier | `probe_gi_multiplier` | `0.0` to `5.0` (default `1.0`) | Scales baked indirect irradiance only; direct lights are unchanged |
| Ambient occlusion | `gtao` | `0` (off, default), `1` (GTAO) | In the Enhanced menu alongside GTAO settings. Adds optional contact shading from the visible world depth. Half-resolution analytic horizons by default, with edge-aware filtering; fixed samples, no temporal jitter or FSR dependency. Runs before particles and the HUD |
| GTAO quality | `gtao_quality` | `performance` (default), `quality` | Performance uses three silhouette-refinement steps; Quality uses seven for more accurate contacts and silhouettes. Older settings files default to Performance |
| GTAO directions | `gtao_slices` | `1` to `12` (default `4`) | Number of horizon-search directions; adjustable in ImGui's GTAO settings |
| GTAO steps | `gtao_steps` | `4` to `24` (default `6`) | Depth samples per side of each direction |
| GTAO half resolution | `gtao_half_resolution` | `0` (full resolution), `1` (half resolution, default) | A faster half-resolution evaluation with edge-aware upsampling |
| GTAO radius | `gtao_radius` | `0.05` to `8.0` (default `1.25`) | Occlusion radius in scene units |
| GTAO strength | `gtao_strength` | `0.0` to `3.0` (default `1.0`) | Multiplies indirect-light occlusion; zero leaves the picture unchanged |
| Texture filter | `anisotropy` | `1` (`off`: trilinear), `2`, `4`, `8`, `16` | The anisotropic filter of the world's textures, held to what the driver offers |
| VSync | `vsync` | `on`, `off`, `adaptive` | Whether a frame waits for the display's refresh. `adaptive` waits, but shows a frame that comes late at once (it tears) rather than a refresh later; a driver without it gives `on` |
| Frame rate | `frame_rate` | `display`, `original`, a number, `unlimited` | How many frames are drawn (section 2). The page offers 30, 60, 75, 90, 120, 144, 165, 240 and 360; a file may hold any number from 20 to 1000 |
| Frame cap | `frame_rate` | | The same key from another side: turns the cap on and moves it five frames a second at a time, for a rate the list does not have |
| Frame counter (F2) | `show_fps` | `0`, `1` | Frames shown a second and what drawing one costs, in the picture's corner |
| (Alt) | `menu_bar` | `1`, `0` | The menus along the window's top on Windows and Linux (macOS has them in its own menu bar) |
| Mute in background | `mute_unfocused` | `0`, `1` | Silence while another window has the keyboard |
| Game fixes: one row each | `fix_...` | `1` (the port's fix), `0` (the console's behaviour) | What the port changes from the console that a player may switch back (section 8). Ratchet & Clank: `fix_wide_moby_effects`, `fix_wide_nanotech_orbs`, `fix_wide_liquids` |
| (F9, F10) | `volume` | `100`, `0` to `100` | How loud everything the port plays is |
| Touch controls: Button size | `touch_scale` | `75`, `50` to `150` (percent) | The on-screen pad's size against its own ([touch_pad.h](../../port/platform/touch_pad.h)). Smaller controls sit nearer the screen's edges as well |
| Touch controls: Camera speed | `touch_camera` | `100`, `25` to `300` (percent) | How short a push of the finger on the right of the screen, from where it landed, turns the camera at full speed: 200 halves the push |
| Touch controls: Edit layout | `touch_layout` | empty, `GROUP DX DY, ...` | Where the player moved each group of the pad from its own place, in screen heights (`dpad 0.0200 -0.0500, face ...`; the groups are `stick`, `dpad`, `face`, `l1`, `l2`, `r1`, `r2`, `select`, `start`, `menu`). Empty: every control at its place. Written only when something was moved |
| (the launcher's Detailed logs) | `verbose_log` | `0`, `1` | The detailed log, as `OPENRAC_LOG=verbose` ([DEBUGGING.md](DEBUGGING.md#logs-and-traces)). Written only when on |

**Old files.** A file from before these keys loads as it did: `fullscreen = 1`
is `display_mode = borderless`; `frame_rate = 60` (and 120, 144, 240) is that
cap; and a file with no `vsync` key waits for the display only at
`frame_rate = display`, which is what that rate meant before vsync was a
setting of its own. The next save writes the new keys.

**Environment.** `OPENRAC_SOFTWARE_GL=1` runs on a software rasteriser
(section 4). `OPENRAC_GPU=integrated` leaves the choice of graphics card to
the system on Linux. For development: `OPENRAC_PACING=1` logs every frame
shown, `OPENRAC_FRAME_COST=N` where a frame's time goes (section 5),
`OPENRAC_SLOW_FRAME=MS` makes every frame that much slower.

## 2. The tick and the frames

The game's logic counts ticks, not time: a tick is one field of the
television it was written for, 50 a second for a PAL game. The port never
changes that. What it draws is its own business:

- `original`: one frame a tick.
- anything else: frames **between** ticks as well, the camera, the mobys and
  their poses moved from the last tick's to this one's by the time each frame
  is shown (half a tick behind the game, so that the next tick is ready
  before the frame that needs it). `display` draws one a refresh, a number
  that many a second, `unlimited` as many as the machine manages.
- `vsync` decides only whether a frame waits for a refresh. With it off and
  `frame_rate = display`, the port's own clock keeps the display's rate.
  With it on and a cap under the display's rate, both hold: 60 frames a
  second on a 120 Hz display are every other refresh.

The game's time is kept apart from the drawing in both directions:

- **A fast machine does not speed the game up.** The tick is scheduled on a
  clock (`main.cpp`, `openrac_game_vsync`): however many frames are drawn,
  the next tick runs at its time.
- **A slow one does not slow it down.** The frame cap never sleeps in the
  tick's path (a frame not yet due is drawn between ticks, when it is).
  Without vsync, a frame that would not be finished by the next tick's time
  is not started. And a tick that comes more than a tick late is not drawn at
  all, three in a row at most: the machine shows fewer pictures, and the game
  runs at its speed. Only a machine that cannot draw one frame in four ticks
  (80 ms) slows the game, and says so in the log.

Measured in level 0 (Apple M5 Max, 120 Hz display, 640 x 480 window, 400
ticks from tick 900):

| `frame_rate`, `vsync` | Frames shown a second | Game ticks a second |
|---|---|---|
| `original` | 50.1 | 50.00 |
| `display`, on | 120.1 | 49.98 |
| `display`, off | 120.2 | 50.00 |
| `30`, off | 30.1 | 50.00 |
| `60`, off | 60.1 | 49.99 |
| `60`, on | 60.1 | 49.95 |
| `144`, off | 144.2 | 49.91 |
| `unlimited`, off | 269.4 | 50.00 |
| `display`, off, every frame 27 ms slower | 26.6 | 50.05 |
| `display`, on, every frame 27 ms slower | 24.7 | 49.95 |

With VSync on or adaptive, a numeric cap is an upper bound: a 120 FPS cap on a
60 Hz display shows at most 60 frames per second. Refresh and frame-cap limits
share one deadline, so asynchronous Vulkan presentation does not add a second
refresh-length sleep to the game tick.

The display's refresh rate is read when the window says it moved to another
display or the display changed mode, not polled.

## 3. The game's 50 Hz / 60 Hz option

The PAL game has a "Video mode" row on this same page: 50 Hz, or 60 Hz for a
television that can show it. In the game's code
([games/rac1](../../games/rac1)):

- `InitOnce` reads the disc's region and sets `D_0015EE80` (1: PAL, 50 Hz),
  then calls `sceGsResetGraph` with video mode 3 (PAL) or 2 (NTSC).
- The row (list update `func_00221E60`, a row with flag 1) flips
  `D_0016044C`, after a confirmation dialog on the way to 60 Hz
  (`func_001FD3E8`, which goes back by itself after ten seconds).
- The title loop `func_001EBB48` sees `D_0016044C` differ from `D_0015EE80`,
  fades, makes it the mode and calls `sceGsResetGraph` again (the display
  becomes 512 x 448 from 512 x 512).
- The next level's start, `func_001E9EC8`, sets the game's **time base** from
  it with `func_002157C0`: at 60 Hz a tick is 1/60 s and the tick scale
  (`D_0015EE60`) 1.0; at 50 Hz 1/50 s and 1.2. Some 500 places read those,
  and a few lay things out by the mode (`D_0015EE80 ? 0x50 : 0x60`).

So the option changes how long a tick is, not only the picture. The port
ticked 50 times a second whatever the game had chosen: in the game's 60 Hz
mode everything ran at five sixths of its speed.

What the port does now:

1. **The row is gone.** The page's rows are the port's; the game's is not
   among them, and nothing else sets the mode. The game stays in the mode its
   disc starts in, 50 ticks a second for PAL, which is the one the
   decompilation and the port have been checked in.
2. **The tick follows the mode anyway.** `openrac_game_set_video_mode` takes
   the tick rate from the mode the game last set (60 for mode 2, 50 for mode
   3), so nothing depends on the row being absent: forcing the switch at the
   title (`OPENRAC_WRITE=300:16044C:0:3`) takes the tick from 50.00 to 60.00
   a second with it.
3. **The display's rate is the display's.** Whatever the tick, frames are
   shown at the rate section 2 describes.

A 60-tick PAL game is therefore possible (the game supports it), but it is a
different layout and time base from the one checked so far, and it would be a
setting of the port's own, not the television's. It is not offered.

## 4. The graphics card

At start the log says what draws the game:

```
graphics: Apple M5 Max (Apple), OpenGL 4.1 Metal - 91.7, GLSL 4.10
graphics: hardware context through SDL's cocoa driver; the window's buffer 8 bits a channel, depth 32, stencil 8, 0 samples, double buffered
graphics: up to 8 samples a pixel, anisotropy up to 16; the swap waits for the display
```

- **The request** is the same on every system: OpenGL 4.1, core profile,
  forward compatible (macOS gives a core context only so, and 4.1 is its
  most; Mesa and Windows' drivers give 4.1 or a later version that runs it),
  double buffered, 8 bits a channel with alpha, 24 bits of depth and 8 of
  stencil, no multisampling and no sRGB on the window's own buffer. The game
  is drawn into a frame of the port's own (which holds the samples and the
  depth) and scaled to the window. sRGB conversion is asked off and
  `GL_FRAMEBUFFER_SRGB` disabled: the renderer blends in the console's own
  colour space, as the console did.
- **Hardware first.** The context is first asked for with
  `SDL_GL_ACCELERATED_VISUAL` (WGL's full acceleration, macOS's accelerated
  pixel formats), and without the condition only if that fails.
- **A software rasteriser is refused.** If `GL_RENDERER` or `GL_VENDOR` names
  llvmpipe, softpipe, swrast, SwiftShader, Apple's software renderer, GDI
  Generic or Microsoft's basic render driver, the game's window does not
  open: the log and a message box say that OpenGL is being drawn on the CPU
  and what to do about it. `OPENRAC_SOFTWARE_GL=1` runs anyway. The headless
  checks, which draw through llvmpipe on purpose, are not held to it
  (`SoftwareRenderer::Allow`).
- **Two graphics cards.** On Windows the program exports
  `NvOptimusEnablement` and `AmdPowerXpressRequestHighPerformance`, which
  NVIDIA's and AMD's drivers read to run a program on the fast card. On Linux
  `prefer_discrete_gpu()` sets `DRI_PRIME=1` (Mesa), and NVIDIA's offload
  variables when its kernel driver is loaded, before SDL starts and only when
  the player has not set them. macOS gives an OpenGL program the discrete
  card already, and Apple silicon has one.

Checked on an Apple silicon Mac only. The Windows exports, the Linux
variables, the retry without the acceleration condition and the refusal of a
software rasteriser were not run on a machine that needs them.

## 5. Where a frame's time goes

`OPENRAC_FRAME_COST=N` logs, every N frames, what a frame cost and where:
`scene` (the tick's mobys and poses moved between ticks and sent to their
buffers, CPU), `submit` (the renderers' OpenGL calls, CPU), `gpu` (the wait
for the card to finish them; the switch calls `glFinish` so that the card's
time is not counted into a later call) and `show` (the scale to the window
and the swap).

Level 0, standing at the start, 2560 x 1920 frame, `frame_rate = unlimited`,
Apple M5 Max:

| | Before | After |
|---|---|---|
| Frames a second (`OPENRAC_PACING`, without `glFinish`) | 260 | 242 |
| A frame's cost before its swap (`OPENRAC_PACING`) | 3.44 ms | 2.99 ms |
| Time from one tick to the next: standard deviation | 1.62 ms | 0.51 ms |
| `scene` | 0.09 ms | 0.06 ms |
| `submit` | 3.0 to 3.3 ms | 2.6 to 2.9 ms |
| `gpu` | 2.1 to 3.6 ms | 3.8 to 3.9 ms |
| Draw calls, triangles | 2,069, 1,122,234 | 2,010, 1,122,234 |

Fewer frames a second at `unlimited` is meant: a frame that would run past
the next tick's time is no longer started, which is what steadied the tick.
A frame itself costs about a tenth less on the CPU. The card's work is the
same (the `gpu` figure moves between runs by more than its two columns
differ): what was fixed was real waste, but it is not where most of the time
is.

What the CPU already leaves to the GPU: skinning (the vertex shader skins
from a palette texture; the CPU only blends two ticks' joint matrices), the
lighting, the fog, instancing (one draw a class and part, whatever the
number of instances), the textures (converted once and kept; images the game
sends are cached by content).

Fixed:

- The live mobys' vertex arrays and instance buffers were deleted and made
  again every frame; a class now keeps them and only its instances are sent.
- The joint palette texture got new storage every frame; its texels are now
  sent into the storage it has.
- Every uniform was looked up by name in the driver at every use; `Shader`
  keeps the locations.
- A material's texture and uniforms are set only when they change, and
  primitives that follow one another with one material are one draw.
- A frame that shows the same picture again (the scene has arrived at the
  tick's) is not drawn.
- `SDL_GetCurrentDisplayMode` is no longer asked every 120 swaps.

Left, with what it costs:

| What | Cost | Why it is left |
|---|---|---|
| The terrain is 1,269 of the 2,010 draw calls for 28,000 triangles: one model whose primitives are a few triangles each, by material and tfrag | about 2 ms of the 3.3 ms `submit` on Apple's OpenGL (1.6 µs a call) | Joining them means ordering a model's primitives by material, which changes the order coplanar faces and translucent texels are drawn in. It wants a check of every level's picture, not a blind change |
| No culling: every tie, shrub and terrain part of the level is drawn every frame, 1.1 million triangles | most of the 3 ms `gpu` at this size | The game's own visibility is not carried over yet; the world renderers in `renderer/world` replay it and are not the ones drawing |
| The mobys are drawn in three passes (solid, glow, translucent) and every part is sent in the first two | 81 draw calls here | The glow faces are told apart in the shader, not on the CPU |
| The frame is read back with `glReadPixels` when the game reads its draw buffer (a page menu opening) and scaled on the CPU | one full frame, once as a menu opens | Not per frame |
| The direct renderer sets four texture parameters before every 2D draw | not measured apart from `submit` | A sampler object per state would do it; it is in a file another branch is changing |
| `Kept`'s instance and palette vectors are copied every frame between ticks | inside `scene`, 0.07 ms | Small |

## 6. OpenGOAL and ReRAC beside OpenRAC

OpenGOAL: `goal_src/jak1/pc/pckernel-h.gc`, `pckernel-common.gc`,
`progress-pc.gc`, `goal_src/jak2/pc/progress/progress-static-pc.gc`,
`game/system/hid/display_manager.cpp`, `game/graphics/pipelines/opengl.cpp`,
`game/settings/settings.cpp`, as of its commit efb21c3e8. ReRAC:
`crates/rc-engine/src/graphics.rs`, `display.rs`, `render_settings.rs`,
`frame_pace.rs`, `crates/rc-game/src/menus/pause/port.rs`, as of a46d260.
Both are ISC licensed. Nothing of either is copied here; the HUD placement
follows ReRAC's idea and says so in
[rac1/host/hud.c](../../port/game/rac1/host/hud.c).

"Now" is this work. "Had" was there before it.

### Display

| Setting | OpenGOAL | ReRAC | OpenRAC | |
|---|---|---|---|---|
| Windowed / borderless / exclusive fullscreen | all three | borderless only | all three | **now** (had: borderless only) |
| Monitor | yes (not while windowed) | no | yes | **now**; with one display here, a second was not tried |
| Fullscreen resolution | the display's modes at its refresh rate | no | the display's modes | **now** |
| Game (internal) resolution | a resolution list in fullscreen | 416p to 2160p, or the window's | a scale of the window's, 50% to 200% | **now**, as a scale: one setting that stays right when the window or display changes |
| Window size remembered | yes, and its position | no | size | **now**; position **later** (SDL's own placement is kept; it needs a check that the saved place is still on a display) |
| VSync | on / off (off by itself when the display is slower than the frame rate) | always (Bevy's default) | off / on / adaptive | **now**, apart from the frame rate |
| Frame rate | 60 to 150 (Jak 1), 60 to 240 (Jak 2); the game's tick changes with it | none: 60 ticks, one a frame | original, display, 30 to 360, any number, unlimited; the tick never changes | **had** in part; **now** any number and unlimited beside vsync |
| Frame-rate counter | in its debug overlay | a line in the log | yes | **now** |
| Aspect ratio | fit, 4:3, 16:9, 16:10, 21:9, 64:27, custom | 4:3, 16:10, 16:9 | 4:3, 16:10, 16:9, 21:9, 32:9, window, custom | **had**; 32:9 and custom **now** |
| Letterbox on or off | in the file | always | always: the picture keeps its shape with bars | **never** as a switch: stretching the picture is `aspect = window` done wrong |
| HUD placement in a wide picture | pulled toward the middle by the aspect | side-anchored slots out to the frame's sides | centre or edges | **now** |
| HUD scale, sharp or smooth HUD | no | original / sharp pixels | no: the 2D layer is drawn at the frame's resolution, glyphs from a distance field | **never** (nothing to choose between) |
| Brightness, contrast | Jak 3 | no | no | **later**: a pass over the frame as it is shown |
| Auto-hide the cursor | yes | no | no | **later**, with mouse input |

### Graphics

| Setting | OpenGOAL | ReRAC | OpenRAC | |
|---|---|---|---|---|
| MSAA | 2, 4, 8, 16 | 2, 4, 8, and the game's own softening | 2, 4, 8, 16 | **now** |
| The game's own anti-aliasing blits | not applicable | "Original" | no | **later**: the port does not draw the console's two soft blits at all yet |
| Texture filtering | no (keys parsed, unused) | original / smooth / sharp (anisotropic) | trilinear, anisotropic 2 to 16 | **now**; the console's own mip rule **later**, with the world renderers that replay it |
| Level of detail | high / low / PS2, background and foreground | original / far / farther / maximum | no | **later**: the port draws every model at full detail now, so there is nothing to raise; it comes with the game's own LOD and culling |
| Shadows | Jak 2: normal / extended | on / off | no | **later**: the port has no shadow renderer yet |
| Presets | no | original / enhanced / custom | no | **later**, when there are enough graphics settings to group |
| PS2 parity switches (particles, actor visibility, envmap, hi-res sky) | yes | no | no | **never** as such: they undo OpenGOAL's own enhancements, which the port does not have |
| Renderer choice | parsed, unused | `WGPU_BACKEND` | no | **never**: one renderer, OpenGL 4.1 |

### Input

| Setting | OpenGOAL | ReRAC | OpenRAC | |
|---|---|---|---|---|
| Choose the controller, port mapping | yes | first pad only | first two gamepads, in order | **later** |
| Rebind controller, keyboard, mouse | yes | no | no: the table in `input.cpp` | **later**: its own page and its own file section; input is not this work's |
| Stick deadzone, sensitivity | yes | fixed scale 1.33 | the game's own | **later**, with rebinding |
| Vibration on or off | yes | no rumble | rumble as the game drives it, no switch | **later**: a row on the game's own options |
| Camera inverted, speed | its own page | the game's Camera page | the game's Camera page | **never** in the port: the game has it |
| Keyboard, mouse camera and movement | yes | mouse as right stick | keyboard as a pad, no mouse | **later** |
| Ignore the controller while unfocused | yes | no | no | **later**, beside `mute_unfocused` |
| Controller LED, trigger effects, pressure | yes | no | no | **never** for the LED and triggers (nothing in the game drives them); pressure **later** |

### Other

| Setting | OpenGOAL | ReRAC | OpenRAC | |
|---|---|---|---|---|
| Music, effects and speech volume, mono | its own | the game's Sound page | the game's Sound page, and `volume` over everything | **had** |
| Mute in the background | no | no | yes | **now** |
| Subtitles, languages | its own pages | the game's | the game's Language page | **never** in the port: the game has it |
| Discord presence | on / off | no | the launcher's | **never** here: it is the launcher's setting |
| Speedrunner mode, cutscene skips, fast elevators | yes | no | `--skip-movies` for development | **later**, if wanted: they change the game, not the display |
| Territory | Jak 2 | no | the version built | **never** |
| The game's 50 Hz / 60 Hz mode | `video-mode` follows the frame rate | none (NTSC only) | removed (section 3) | **now** |
| Graphics card: prefer the discrete one | Windows exports | no | Windows exports, Linux variables | **now** |
| Graphics card: refuse a software rasteriser | no (the strings are logged) | no | yes | **now** |

## 7. Strafe (Ratchet & Clank)

The strafe of the later games (Going Commando's Hold L2-R2, Up Your
Arsenal's lock-strafe), which was a row of this page until 2026-10-21, is
a mod now: **lock-strafe**, in [OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods),
installed and switched in the launcher's Mods tab, with its mode (Lock with
weapons, or Hold L2-R2) among its settings ([MODS.md](MODS.md)). Its
README says what each mode does, how it is built from the game's own strafe
in state 0x81, and its limits. A settings file from before keeps loading;
its `strafe` (or `lock_strafe`) line is left out, and the log says so once.
The mod's `trace` setting logs what `OPENRAC_STRAFE_TRACE` did: a line each
pad report with the mode, what the strafe does, the hero's state and the
yaws.

What the port keeps for it are six hook points
([port/game/rac1-pal/libraries.tsv](../../port/game/rac1-pal/libraries.tsv),
`hook`), which run the game's own C while no mod takes them: the pad's
report into its state (`Pad_ProcessReport`), the hero's motion input
(`Hero_UpdateMotionInput`), his turn toward his move yaw
(`Hero_TurnToHeading`), his velocity along an angle
(`Hero_SetPlanarVelocity`), the follow camera's requests
(`FollowCamera_ApplyOverrides`) and his animation's advance
(`Hero_AdvanceAnimation`). Without the mod the game strafes only as the
console does.

## 8. Game fixes and the differences from the original

Players see everything the port does otherwise than the PS2 game, and a
purist can switch some of it back. One list per game holds it:
[port/game/rac1/differences.json](../../port/game/rac1/differences.json)
(another game's goes at `port/game/<game>/differences.json`). The port's
build reads it ([cmake/Differences.cmake](../../port/cmake/Differences.cmake)),
and so does the launcher ([core/src/differences.rs](../../launcher/core/src/differences.rs),
which also carries a copy for a launcher without a checkout): nothing is
written twice.

Each entry:

| Field | |
|---|---|
| `id` | lower-case words joined by hyphens: `wide-liquids` |
| `kind` | `fix`: the player may switch it back. `guard`: keeps the game from crashing where the console carried on unnoticed; shown, never switched (off would only mean the crash). `port`: a difference that comes with being a PC program, for the record |
| `name` | a few words. A fix's is its row on the game's Options page: 22 letters at most |
| `discs` | `["pal"]`, `["ntsc"]` or both |
| `original` | what the PS2 game does |
| `port` | what the port does, and why; for a fix, what off brings back |
| `setting` | a fix only: `{"key": "fix_...", "default": true}`. On is the port's fix |
| `refs` | where it is: commits, files, docs. Shown with the launcher's developer tools on |

Write only what the code or a commit shows. A fix is:

- **a setting**: `fix_... = 1` or `0` in the settings file (section 1),
  written with the others, read at start;
- **a row** of the Options page (title and pause menu), after the port's own
  under a **Game fixes** heading: On, or Original. The page scrolls, four or
  five rows at a time. The in-game drawer has a **Game fixes** card too;
- **a switch** in the launcher, under the game's **Differences from the
  original**, which edits that one line of the file (it counts from the
  game's next start);
- **a flag** the code reads. The build writes the disc's fixes into a table
  ([fixes.h](../../port/game/common/fixes.h)); host C asks for its flag once
  and reads it through the pointer from then on, so the test costs a load:

  ```c
  static const unsigned char* fix;
  if (!fix) fix = openrac_game_fix("fix_wide_liquids");
  if (*fix) { /* the port's fix */ } else { /* the console's behaviour */ }
  ```

To add one: the entry in the file, the flag at the code it changes (off
must give the console's behaviour, exactly), and its key in
`tests/game_fixes_test.cpp` if the test should hold it to its code. The
configure refuses a file that breaks a rule (a fix without a setting, a
setting on anything else, a key twice, a name too long), the launcher's
tests the same, and `game_fixes` checks that every key the rac1 host C asks
for is in the table.

Ratchet & Clank's fixes now, all on by default and all for both discs:

| Key | Off |
|---|---|
| `fix_wide_moby_effects` | MobyProc's "drawn" test ([render_native.c](../../port/game/rac1/host/render_native.c), [moby_view.h](../../port/game/rac1/host/moby_view.h)) against the 4:3 view: mobys near the sides of a wide picture lose their effects |
| `fix_wide_nanotech_orbs` | the nanotech cluster's sphere test ([wide_view.c](../../port/game/rac1/host/wide_view.c)) against the 4:3 view: crates near the sides lose their orb |
| `fix_wide_liquids` | the draw callback lists and the liquid grid's cull ([wide_view.c](../../port/game/rac1/host/wide_view.c)) against the 4:3 view: seas, lava and sand are missing near the sides |

At 4:3 none of them changes anything.
