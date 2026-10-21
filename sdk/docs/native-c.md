# Native mods in C

A native mod is a shared library (`.dylib`, `.so`, `.dll`) that the game opens
at start. It talks to the game only through one table of functions, so it is
built against the SDK's headers alone and links against nothing of OpenRAC's.
The C++ layer ([native-cpp.md](native-cpp.md)) is the same thing with less
code; this page is what is underneath.

## The two entry points

```c
#include "openrac/openrac_mod.h"
#include "openrac/games/rac1.h"

OPENRAC_MOD_DEFINE("my-mod", "0.1.0")   /* openrac_mod_query: the interface version, the id */

OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) {
    if (strcmp(api->game, "rac1") != 0) {
        api->fail(mod, "this mod is for Ratchet & Clank");
        return 1;                          /* not loaded; the reason is shown */
    }
    api->log(mod, OPENRAC_LOG_INFO, "hello");
    return 0;                              /* loaded */
}
```

- `openrac_mod_query` tells the game which interface you built for and your
  mod's id and version; they must match the manifest.
- `openrac_mod_load` gets the table (`openrac_api`) and your mod's handle.
  Return 0 to load; anything else refuses, with the reason you gave `fail`.
- `openrac_mod_unload` (optional) runs when the game exits.

Everything happens on the game's thread.

## Hooks

```c
static openrac_rac1_Hero_SetPlanarVelocity s_original;

static void my_velocity(float angle) {
    s_original(angle);   /* the game's own, or the mod before yours */
}

/* in openrac_mod_load: */
if (OPENRAC_RAC1_HOOK(api, mod, Hero_SetPlanarVelocity, my_velocity, &s_original) != OPENRAC_HOOK_OK) {
    api->fail(mod, "no Hero_SetPlanarVelocity in this OpenRAC");
    return 1;
}
```

`api->hook(mod, name, signature, replacement, &original)` takes a hook point
by its id or alias; the signature (`"void(f32)"`) must be the hook point's, or
the call fails. `original` is what your replacement goes on to. See
[hooks](hooks.md) for the chain and the ids, and the
[reference](reference/hooks-rac1.md) for every hook point's signature.

Results: `OPENRAC_HOOK_OK`, `OPENRAC_HOOK_UNKNOWN` (no such hook point here),
`OPENRAC_HOOK_SIGNATURE` (another signature), `OPENRAC_HOOK_TOO_LATE` (outside
load).

## Callbacks

```c
static void tick(void* user) { /* after each game tick */ }
static void pad(void* user, int port, uint8_t* report, int size) { /* change report[] */ }

api->on_frame(mod, tick, NULL);
api->on_pad(mod, pad, NULL);
```

The pad report's layout is in [scripts.md](scripts.md#the-pad-report).

## The keyboard and mouse

Interface 1.2 (`"api": "1.2"`) lets a mod read the keyboard and mouse and
press the pad's buttons from them, for a mod that plays the game with a
keyboard and mouse (rac1-mods' keyboard-mouse):

```c
#include "openrac/keys.h"   /* names for keys, mouse buttons and the wheel */

static int s_jump;          /* a bind: from a keybind setting (settings.md) */

static void pad(void* user, int port, uint8_t* report, int size) {
    const openrac_input* in = s_api->input();      /* this tick's keys and mouse */
    if (port == 0 && openrac_bind_held(in, s_jump)) {
        report[1] &= (uint8_t)~0x40;               /* Cross held (bit 14: a clear bit is held) */
    }
    /* in->mouse_dx, mouse_dy: the motion since the last tick; wheel_x, wheel_y in notches */
}

/* in openrac_mod_load: */
s_jump = openrac_bind_from_name(api->setting_string(mod, "jump"));  /* one bind; split a pair at ',' */
api->take_input(mod, OPENRAC_INPUT_KEYBOARD);  /* the port's own WASD table stops */
api->capture_mouse(mod, 1);                     /* hidden cursor, relative motion */
```

- `input()` is the state for the tick about to run, the same for every call
  in that tick (`in->tick` says which). Each key, `keys[usage]`, has
  `OPENRAC_KEY_HELD` and `OPENRAC_KEY_PRESSED` (it went down since the last
  tick, so a tap shorter than a tick still counts). Nothing is held while the
  window has no focus.
- `take_input` gives the keyboard to your mod: the port's table (WASD, IJKL,
  Space, ...) no longer presses pad 0's buttons. A gamepad still does, so
  merge your buttons into the report rather than overwrite it.
- `capture_mouse` asks; the port lets the cursor go anyway while the window
  has no focus or its overlay (F3) is open (`in->overlay`), and captures it
  again after.
- The game's state (playing, paused, a menu) is the game's own: read it from
  game memory.

## Settings and logging

`api->setting_bool/int/float/string(mod, key)` ([settings](settings.md)), and
`api->log(mod, level, text)` with `OPENRAC_LOG_DEBUG`, `_INFO`, `_WARN`,
`_ERROR`.

## Game memory and functions (experimental)

These name the game's internals: what is where changes with the disc and with
the decompilation's progress.

- `api->memory` is the base of game memory: address `a` is `api->memory + a`
  (`OPENRAC_GREF(api, uint32_t, a)` reads or writes a word).
- `api->relocate_data(a)` and `api->relocate_level_data(level, a)` give a
  global's address in the code loaded now; ask each time, since a level moves
  its globals. `api->loaded_level()` is the level loaded, -1 for none.
- `api->function(name)` and `api->function_at(address)` find a game function to
  call, by its name in this disc's build or by code address.
- `api->stack_push(size, &saved)` / `api->stack_pop(saved)` give room on the
  game's stack for a call that takes the address of a local.

A mod for both discs keeps a table of the addresses and names it uses on each
and picks one by `api->version` (`"pal"`, `"ntsc"`).

## Building

The SDK's CMake package builds the library with only the entry points visible
and no undefined symbols:

```cmake
find_package(OpenRACMod 1.1 REQUIRED)
openrac_add_mod(my_mod MANIFEST openrac-mod.json SOURCES src/mod.c)
```

The full list of the table's members is the [reference](reference/c-api.md).
