# Native mods in C++

`openrac/mod.hpp` is a header-only C++17 layer over the C interface
([native-c.md](native-c.md)): hooks as lambdas, typed settings, RAII. It is
compiled into your mod only; the game still sees the C interface, so a C++ mod
works with any OpenRAC of its interface version. The SDK's `template/` is a
C++ mod.

## A whole mod

```cpp
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"

static void load(openrac::Mod& mod) {
    if (mod.game() != "rac1") throw openrac::Error("this mod is for Ratchet & Clank");

    const bool trace = mod.setting<bool>("trace");

    mod.hook<openrac::rac1::Hero_TurnToHeading>(
        [trace, &mod](auto original, int32_t m, float accel, float damping, float limit) {
            if (trace) mod.info("turning");
            original(m, accel, damping, limit);
        });

    mod.on_frame([] { /* after each game tick */ });
}

OPENRAC_MOD_CPP("my-mod", "0.1.0", load)
```

`OPENRAC_MOD_CPP(id, version, load)` writes both entry points. Throwing out of
`load` refuses the mod with the exception's message.

## `openrac::Mod`

| | |
|---|---|
| `game()`, `version()`, `platform()`, `openrac_version()`, `api_major()`, `api_minor()` | about the game running |
| `debug/info/warn/error(text)`, `log(level, text)` | the game's log |
| `fail(reason)` | refuse during load (or throw `openrac::Error`) |
| `setting<T>(key)` | `bool`; an integer type for `int`; `float` or `double`; `std::string` for `choice` and `string` |
| `hook<HookPoint>(callable)` | `callable(original, args...)`; throws `openrac::Error` if the hook point is missing, has another signature, or is taken twice |
| `original<HookPoint>()` | what your hook goes on to, to call it from elsewhere |
| `on_frame(callable)`, `on_pad(callable)` | `callable()`; `callable(port, report, size)` |
| `memory()` | an `openrac::Memory`: `read<T>(address)`, `write<T>(address, value)`, `ref<T>(address)` (experimental) |
| `function<Fn>(name)`, `required_function<Fn>(name)`, `function_at<Fn>(address)` | game functions (experimental) |
| `relocate(address)`, `relocate_level(level, address)`, `loaded_level()` | globals in the code loaded now (experimental) |
| `input()` (1.2) | an `openrac::Input` for the tick about to run: `held(bind)`, `pressed(bind)`, `mouse_dx()`, `mouse_dy()`, `wheel_x()`, `wheel_y()`, `focused()`, `captured()`, `overlay()`, `tick()`; 1.3: `has_pointer()`, `pointer_x()`, `pointer_y()`, `pointer_inside()` ([the keyboard and mouse](native-c.md#the-keyboard-and-mouse)) |
| `take_input(devices)` (1.2) | during load: `OPENRAC_INPUT_KEYBOARD` turns the port's own keyboard table off |
| `capture_mouse(on)` (1.2) | the cursor captured or let go |
| `openrac::bind_from_name(name)`, `openrac::bind_name(bind)` | a key, button or wheel direction by its name (`"KeyW"`, `"MouseLeft"`) |

The hook point types (`openrac::rac1::Hero_TurnToHeading`, ...) are in
`openrac/games/rac1.hpp`, generated from OpenRAC's hook table; see the
[reference](reference/hooks-rac1.md) and [hooks](hooks.md) for ids and chains.

`openrac::StackFrame frame(mod, 16);` gives 16 bytes on the game's stack at
`frame.address()`, given back when `frame` goes out of scope.

## Exceptions never cross into the game

Every hook and callback the layer installs catches whatever your code throws:

- in a hook, the exception is logged with your mod's id and the original runs,
  so the game goes on (if you threw after calling the original, it runs twice:
  do not throw there);
- in a callback, it is logged and the callback returns;
- in `load`, it becomes the refusal.

## Building

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_mod LANGUAGES C CXX)
find_package(OpenRACMod 1.1 REQUIRED)
openrac_add_mod(my_mod MANIFEST openrac-mod.json SOURCES src/mod.cpp)
```

C++17, with each platform's default runtime (libc++ on macOS, libstdc++ on
Linux, the DLL runtime `/MD` with MSVC); only the entry points are exported.
The full API is the [reference](reference/cpp-api.md).
