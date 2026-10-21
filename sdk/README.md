# The OpenRAC mod SDK

What you need to write a **native mod** for OpenRAC's native port: a small
shared library the game loads at start, which changes what the game does
through named hook points, a callback each game tick, the pad's reports and
its own settings. The design, the rules and how the port and the launcher
treat a mod are in [docs/port/MODS.md](../docs/port/MODS.md); Ratchet &
Clank's mods are in [OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods),
starting with the lock-strafe.

A mod of textures, models or level edits needs none of this: that is a data
pack, made with the level editor ([editor/MODDING.md](../editor/MODDING.md)).

| | |
|---|---|
| [include/openrac/openrac_mod.h](include/openrac/openrac_mod.h) | the interface: the entry points a mod exports and the table (`openrac_api`) the port hands it |
| [include/openrac/games/rac1.h](include/openrac/games/rac1.h) | Ratchet & Clank's hook points: names, signatures, replacement types |
| [cmake/](cmake/OpenRACModConfig.cmake) | `find_package(OpenRACMod)` and `openrac_add_mod()` |
| [template/](template) | a mod to copy |
| [examples/frame-counter/](examples/frame-counter) | a mod that uses each part of the interface |
| [tools/package_mod.py](tools/package_mod.py) | checks the manifest and writes the mod's `.zip`; joins the archives of several platforms |

## A mod in brief

A mod is a folder with a manifest, `openrac-mod.json` (MODS.md, section 3:
its id, name, version, the games and discs it is for, the interface version,
its settings, dependencies and conflicts), and its library. The library
exports two functions:

```c
#include "openrac/openrac_mod.h"
#include "openrac/games/rac1.h"

OPENRAC_MOD_DEFINE("my-mod", "0.1.0")          /* openrac_mod_query: the interface and the id */

static openrac_rac1_Hero_TurnToHeading s_turn; /* what the hook goes on to */

static void turn(int32_t mode, float accel, float damping, float limit) {
    /* ... before ... */
    s_turn(mode, accel, damping, limit);        /* the game's own, or a mod's loaded earlier */
}

OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) {
    if (OPENRAC_RAC1_HOOK(api, mod, Hero_TurnToHeading, turn, &s_turn) != OPENRAC_HOOK_OK) {
        api->fail(mod, "no Hero_TurnToHeading hook point here");
        return 1;                                /* not loaded; the launcher shows why */
    }
    return 0;
}
```

What the table offers (MODS.md, section 6): the game's memory
(`api->memory + address`), the log, the mod's settings, hook points, a
callback after each game tick and one for each pad report, the game's
functions by name or by code address, the relocation of globals for the
level loaded, and room on the game's stack. Nothing else of the port is
reachable, and a mod links against nothing of OpenRAC's: the build refuses a
library that leaves a symbol undefined.

Several mods on one hook point chain: each calls the original it was given,
and the mod loaded last runs first. A mod for several discs keeps a table of
the addresses and names it uses on each and picks it by `api->version`
(the lock-strafe does: `lock-strafe/src/discs.h` in rac1-mods).

## Building

You need CMake 3.20 or newer, a C compiler (Clang, GCC or MSVC) and Python 3
for the packaging. Ninja is the quickest generator; any works.

```sh
cp -r <OpenRAC>/sdk/template my-mod      # then set the id, the name, the target
cd my-mod
cmake -S . -B build -G Ninja -DOPENRAC_SDK=<OpenRAC>/sdk
cmake --build build
```

(`-DOpenRACMod_DIR=<OpenRAC>/sdk/cmake` does the same as `OPENRAC_SDK`.)

- **macOS**: Xcode's command-line tools (`xcode-select --install`) and
  `brew install cmake ninja`. A build is for the Mac's own processor;
  `-DCMAKE_OSX_ARCHITECTURES=x86_64` makes an Intel one (one architecture per
  build: join the two archives afterwards).
- **Linux**: `sudo apt install build-essential cmake ninja-build python3`
  (or your distribution's). Clang works too: `-DCMAKE_C_COMPILER=clang`.
- **Windows**: Visual Studio 2022 with "Desktop development with C++". In a
  "x64 Native Tools Command Prompt", `cmake -S . -B build -G Ninja` (or
  without `-G`, Visual Studio's own generator, and `cmake --build build
  --config Release`). Clang works through `-T ClangCL`, or with LLVM's
  `clang-cl` as `-DCMAKE_C_COMPILER=clang-cl`.

`openrac_add_mod(<target> MANIFEST openrac-mod.json SOURCES ...)` builds the
library as `<target>.dylib`, `<target>.so` or `<target>.dll` (no `lib` in
front, on any system), C11, with only the entry points visible, and lays the
mod out as its archive holds it in `build/package/<target>/`: the manifest
with its `plugin` naming this platform's library (`lib/<platform>/<file>`),
the library, and the README and LICENSE beside the manifest. The platforms are
`macos-arm64`, `macos-x86_64`, `linux-x86_64`, `linux-aarch64`,
`windows-x86_64` and `windows-arm64`.

## Packaging

```sh
python3 <OpenRAC>/sdk/tools/package_mod.py build/package/my_mod        # dist/my-mod-0.1.0-macos-arm64.zip
python3 <OpenRAC>/sdk/tools/package_mod.py --merge dist/my-mod-0.1.0-*.zip -o dist/my-mod-0.1.0.zip
python3 <OpenRAC>/sdk/tools/package_mod.py --check openrac-mod.json     # the manifest only
```

The archive is checked by the rules the launcher and the port apply (MODS.md,
section 3) and is not written when the manifest breaks one. `--merge` joins
the archives each platform's build made into one for every platform, which is
what a release publishes: their manifests must be the same but for `plugin`.
rac1-mods' workflow does both on GitHub's runners for macOS, Linux and
Windows; copy it for a repository of your own.

The tests: `python3 -m unittest discover -s sdk/tools`.

## Installing and trying a mod

In the launcher, **Mods**, then **Add a mod**, and choose the `.zip` (or the
`build/package/<target>` folder while you work on it). Switch it on, set its
settings, and Play: the game's log (Logs, under the game's saves) says
`mods      my-mod 0.1.0` at its top, and your mod's lines after its id. A mod
that was not loaded says why on its row in the Mods tab. By hand, without the
launcher: copy the folder to `<data>/plugins/my-mod/` and list it in
`<data>/plugins/plugins.json` (MODS.md, section 5).

**Purist mode** (Settings) starts the game with no mods at all; so does the
port's `--no-mods`.

## License

The SDK is under the GNU GPL, version 3 or later, as the rest of OpenRAC's
top level. A mod runs inside the GPL'd port; whether the SDK's headers should
be under a permissive license so that mods can choose their own is an open
question for the maintainers (MODS.md, section 10). Until it is settled,
write mods under the GPL, as rac1-mods does.
