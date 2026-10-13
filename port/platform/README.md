# platform

The port's platform layer on SDL3: what the game's Sony libraries did for
the screen, the pads, sound and time, replaced at the game's library calls
([docs/port/DESIGN.md](../../docs/port/DESIGN.md), section 2). Library
`openrac_platform`; SDL3 comes from the system or is fetched by CMake
([cmake/Dependencies.cmake](../cmake/Dependencies.cmake)).

| File | What |
|---|---|
| `window.h` | `Platform` (SDL for the life of the program) and `Window`: an OpenGL 4.1 core context, windowed, fullscreen at a resolution or borderless, vsync; a message box when OpenGL 4.1 is missing; SDL's offscreen driver for headless checks |
| `pad.h` | `Pad`: one DualShock 2 as the game's pad library reports it, and the 32-byte read buffer |
| `input.h` | keyboard and SDL gamepads into two pad ports, rumble |
| `audio.h` | `AudioOutput`: one SDL audio stream, 48 kHz stereo signed 16-bit, pulled by a callback or pushed by `queue()` |
| `clock.h` | a monotonic clock and `FramePacer`, which holds the loop to the console's 50 Hz (PAL) or 59.94 Hz (NTSC) field rate |
| `pump.h` | `run_pumped` and `poll_event`: the program beside the window's event pump on one thread (a fiber on Windows), so it goes on drawing while Windows holds the pump in its own loop (the window dragged by its title bar, sized, a menu open) |

As in OpenGOAL ([OPENGOAL_NOTES.md](../../docs/port/OPENGOAL_NOTES.md),
section 6), OpenGL 4.1 core is requested everywhere, so macOS gets the same
context as the other platforms, and the renderer loads its functions through
the context (`Window::gl_loader`), never through SDL headers.

## The pad

From public knowledge of the DualShock 2's protocol (homebrew documentation,
no Sony material). Sixteen buttons in one word, **active low** (a held button
reads 0):

| Bit | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| Button | Select | L3 | R3 | Start | Up | Right | Down | Left |
| **Bit** | **8** | **9** | **10** | **11** | **12** | **13** | **14** | **15** |
| Button | L2 | R2 | L1 | R1 | Triangle | Circle | Cross | Square |

Sticks are one byte per axis, 0x00 left or up, 0xFF right or down, 0x80
centred. Pressure bytes (right, left, up, down, triangle, circle, cross,
square, L1, R1, L2, R2) run from 0x00 to 0xFF. `Pad::write_read_buffer`
lays them out as libpad's read buffer: status, mode (0x79), buttons, right
then left stick, pressures. Default keys are listed in `input.h`; gamepad
buttons map by position (the bottom face button is Cross).

## Not done yet

The replacements of the game's library calls themselves (`scePadRead`,
989snd, `sceGsSyncV`) belong to the game's build; this layer is what they
call. Settings (key remapping, display mode) are not saved yet.
