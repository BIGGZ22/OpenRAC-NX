# Mods: plugins, the SDK and the launcher

How a mod changes what the native port does without being built into it: a
**native mod** is a shared library the port loads at start, through a small
versioned C interface; a **data mod** is a pack of textures, models or level
edits, which the level editor and the overrides packs already cover
([editor/MODDING.md](../../editor/MODDING.md)). The launcher installs both,
switches them on and off, orders them and shows why one was not loaded. A
player who wants the game as the console plays it switches **Purist mode** on
and nothing is loaded.

Mods live outside OpenRAC: [OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods)
holds Ratchet & Clank's, starting with the lock-strafe that was an option of
the port's own until 2026-10-21. The SDK to write one is [sdk/](../../sdk/README.md).

Status words as in [DESIGN.md](DESIGN.md): **decided**, **proposed**, **open**.

## 1. What is decided (2026-10-21)

1. **The port stays the console's game; changes of play are mods.** What a
   mod does is not an option of the port. The port carries only the hook
   points a mod needs, and with no mod hooked they run the game's own code,
   so the game without mods is the retail game (and the game fixes of the
   Differences page, which stay the port's: they are switched there).
2. **A native mod is a shared library with a C interface**: `.dylib` on
   macOS, `.so` on Linux, `.dll` on Windows, exporting `openrac_mod_query`
   and `openrac_mod_load`. Everything it uses of the port comes through one
   table of functions the port hands it (`openrac_api`), never by linking
   against the port's own symbols. So a mod is built once per platform
   against the SDK's headers alone, as Reloaded-II's and BepInEx's plugins
   are built against an interface and not against the game.
3. **A mod only changes the game where the port lets it: named hook
   points.** A hook point is a game function the port's build declares
   hookable (`hook` in a version's `libraries.tsv`, as `wrap` is today). Its
   C is kept as `<name>__game`, and hostgen writes the function itself: it
   calls the mod's replacement when one is hooked, the game's C otherwise
   (section 4). A mod asks for a hook point by its name and signature; one
   the port does not have, or whose signature differs, is refused at load
   with the reason. There is no patching of machine code.
4. **One file says what a mod is, `openrac-mod.json`**, the same name as the
   overrides packs' manifest, with `"format": 2` (section 3). The launcher and
   the port check it by the same rules.
5. **The launcher writes the list, the port reads it.** The mods a game loads,
   their order and their settings are `<data>/plugins/plugins.json`, written
   by the launcher; the port writes back what it did with each one,
   `<data>/plugins/status.json` (section 5).
6. **Versioned, refused rather than guessed.** An interface version the port
   does not have, another game or disc, a platform the mod has no library
   for, a missing dependency or a conflict: the mod is not loaded, the game
   starts without it, the log and `status.json` say why, and the launcher shows
   it beside the mod.
7. **A crash names the mods.** The session log's header lists the mods
   loaded, and a crash report lists them and names the one whose code was
   running where the address lets the port tell (section 7).
8. **Purist mode loads no mods**: `--no-mods`, which the launcher's switch
   passes.

## 2. Where things are

```
OpenRAC
├── sdk/                               the SDK (section 8)
│   ├── include/openrac/openrac_mod.h  the interface: openrac_api, the entry points
│   ├── include/openrac/games/rac1.h   Ratchet & Clank's hook points and their signatures
│   ├── cmake/                         find_package(OpenRACMod): openrac_add_mod()
│   ├── template/                      a mod to copy
│   ├── examples/frame-counter/        a mod that uses each part of the interface
│   └── tools/package_mod.py           a mod's .zip, with its manifest
├── port/game/common/mods.{h,cpp}      the loader: the list, the checks, dlopen, the hooks
├── port/game/common/mod_manifest.*    manifests and the list, read and checked (tested alone)
├── port/game/<version>/libraries.tsv  `hook` rows: the hook points of each disc
└── launcher/core/src/plugins.rs       the launcher's side: install, list, order, settings

<install>/active/<game>/data/          a game's set-up folder (launcher/docs/INTEGRATION.md)
├── plugins/
│   ├── plugins.json                   the launcher's list: order, on or off, settings
│   ├── status.json                    the port's answer at its last start
│   └── <id>/                          one native mod, as installed
│       ├── openrac-mod.json
│       └── lib/<platform>/<library>
├── overrides/                         the data packs (unchanged: editor/overrides.py)
└── mods/<version>/                    the packs stacked for one disc (unchanged)
```

`plugins/` is the game's, not a version's, as `overrides/` is: a mod lists the
discs it supports, and one mod folder serves both of a game's discs. The
folder is the player's own (in the install folder, a per-user folder by
default) and only the launcher writes it; the port reads it and writes
`status.json`.

**Where that is on each system.** The install folder is the launcher's
setting (Settings, "Install folder"); everything above lives in it, with the
player's games, and never beside a program. The launcher's own folder (an
AppImage, the macOS app, the Windows installer's folder) is read-only and
replaced by an update, so nothing a player adds goes there. Unless the player
chose another, the install folder is:

| System | Install folder | A game's mods |
|---|---|---|
| Linux (AppImage, deb, rpm, a checkout) | `~/OpenRAC` | `~/OpenRAC/active/rac1/data/plugins/` |
| macOS | `~/OpenRAC` | `~/OpenRAC/active/rac1/data/plugins/` |
| Windows | `%USERPROFILE%\OpenRAC` | `%USERPROFILE%\OpenRAC\active\rac1\data\plugins\` |

and when that folder cannot be made, the launcher's per-user data folder
(`~/.local/share/dev.openrac.launcher` on Linux, `~/Library/Application
Support/dev.openrac.launcher` on macOS, `%LOCALAPPDATA%\dev.openrac.launcher`
on Windows). The launcher's Settings show the folder in use.

**Neither side guesses.** The launcher starts the game with `--data <data>`
and `--plugins <data>/plugins/plugins.json` (or `--no-mods` in Purist mode),
the same in an installed launcher and a checkout; the port reads only what
those name and writes `status.json` beside the list. Run by hand without
`--plugins`, the port reads `<data>/plugins/plugins.json`. Nothing of a
source tree is read at run time.

**Android** carries the game inside the app and runs it there; the system
does not let an app load code from the files it writes (since Android 10), so
native mods are not loaded on Android: the Mods tab shows them as not for
this platform (no `plugin` entry for it). Data packs work there as
elsewhere.

## 3. The manifest: `openrac-mod.json`

```jsonc
{
  "format": 2,                           // 1 is a data pack (editor/MODDING.md)
  "id": "lock-strafe",                   // lower case, digits and hyphens; the folder's name
  "name": "Lock-strafe",
  "version": "1.0.0",                    // semantic version
  "author": "OpenRAC contributors",
  "description": "Strafe as in the later games: ...",
  "license": "GPL-3.0-or-later",
  "homepage": "https://github.com/OpenRAC/rac1-mods",
  "api": "1.0",                          // the interface it was built for (section 6)
  "games": { "rac1": ["pal", "ntsc"] },  // game id: the discs (game.json versions)
  "plugin": {                            // the library for each platform it was built for
    "macos-arm64": "lib/macos-arm64/lock_strafe.dylib",
    "macos-x86_64": "lib/macos-x86_64/lock_strafe.dylib",
    "linux-x86_64": "lib/linux-x86_64/lock_strafe.so",
    "windows-x86_64": "lib/windows-x86_64/lock_strafe.dll"
  },
  "hooks": ["Pad_ProcessReport", "Hero_TurnToHeading"],   // the hook points it takes
  "settings": [
    { "key": "mode", "name": "Mode", "type": "choice", "default": "lock",
      "choices": [ { "value": "lock", "name": "Lock with weapons" },
                   { "value": "hold", "name": "Hold L2 or R2" } ],
      "description": "..." }
  ],
  "requires": [ { "id": "some-library-mod", "version": ">=1.2.0" } ],
  "conflicts": [ { "id": "other-strafe", "reason": "both take L2 and R2" } ],
  "load": { "after": ["camera-tweaks"], "before": [] }
}
```

| Field | Required | Rule |
|---|---|---|
| `format` | yes | 2. A format-1 file is a data pack and goes to `overrides/` |
| `id` | yes | `[a-z0-9]+(-[a-z0-9]+)*`, at most 64 characters, the installed folder's name |
| `name`, `version` | yes | `version` is `MAJOR.MINOR.PATCH` with an optional `-suffix` |
| `author`, `description`, `license`, `homepage` | no | shown in the launcher; `homepage` only if `https://` |
| `api` | yes, with `plugin` | `MAJOR.MINOR`: the interface's version the mod was built against |
| `games` | yes | at least one game, each with at least one version |
| `plugin` | no | platform → path inside the mod. Platforms: `macos-arm64`, `macos-x86_64`, `linux-x86_64`, `linux-aarch64`, `windows-x86_64`, `windows-arm64`. Without it the mod is data only (not used yet: section 10) |
| `hooks` | no | the hook points the mod takes. The launcher shows which mods share one; the port warns about a hook a mod takes but does not list |
| `settings` | no | the mod's settings (below) |
| `requires` | no | mods that must be on and load before it; `version` is `>=X.Y.Z`, `=X.Y.Z` or absent |
| `conflicts` | no | mods it cannot run with; either side may list the other |
| `load` | no | `after` and `before`: ids it should load after or before when both are on (an order hint the launcher applies when a mod is installed or switched on) |

A setting is `{key, name, type, default, description?}` with `key` in
`[a-z0-9_]+`, and by type:

| `type` | Extra | Value in `plugins.json` |
|---|---|---|
| `bool` | | `true` / `false` |
| `int` | `min`, `max` | an integer |
| `float` | `min`, `max`, `step` | a number |
| `choice` | `choices`: `[{value, name}]`, two or more | one of the values (a string) |
| `string` | `max_length` | a string |

A value that is missing, of the wrong type or out of range is the default
(the port logs it). Settings count from the game's next start.

## 4. Hook points

A hook point is declared in a disc's `libraries.tsv`, in the `port` column:

```
func_L00_002124E8	hero	Hero_TurnToHeading	hook	eases the facing toward the move yaw: ...
```

and hostgen writes, in the generated `hooks.c`:

```c
static void* hook_func_L00_002124E8;
void func_L00_002124E8(int a0, float a1, float a2, float a3) {
    if (hook_func_L00_002124E8 != 0) {
        ((void (*)(int, float, float, float))hook_func_L00_002124E8)(a0, a1, a2, a3);
        return;
    }
    func_L00_002124E8__game(a0, a1, a2, a3);
}
```

with the table the loader binds (`openrac_game_hook_points`): the name of the
third column, the function's symbol, its signature and the slot. The name is
the hook point's name for mods, the same for every disc of a game (rac1/pal's
`func_L00_002124E8` and rac1/ntsc's `FUN_L00_00211e30` are both
`Hero_TurnToHeading`), so a mod names it once. With no mod on it the slot is
null and the game's C runs: one load and a branch the processor predicts.

**Signatures** are written the same way on both sides: the return type, then
the parameters, from `void`, `i32`, `u32` (a game address too), `i64`,
`u64`, `f32`, `i16`, `u16`, `i8`, `u8` (`void(i32,f32,f32,f32)`). The SDK's
game header gives each hook point's name, signature and a typedef
(`OPENRAC_RAC1_HOOK_Hero_TurnToHeading`). A mod passes the signature it was
written for; a different one is refused.

**Several mods on one hook point** chain, as hooks do in Detours or
MinHook: a mod's `hook()` call gets the function it replaces, the game's C
or the replacement of a mod loaded before it, and calls it to go on. So the
mod loaded **last** runs **first**, and the order on the Mods tab (first to
last) is the order they wrap the game in. A mod that does not call the
original takes the call for itself; two such mods on one hook point are
legal, but the launcher shows that they share it.

Hook points are added as mods need them; adding one changes nothing for a
game without mods. Which exist is the game header in the SDK, and the port
writes them in `status.json`. A version's table names a hook point only where
that disc has the function; a mod for both discs that needs one the other
disc lacks is refused there, with the reason.

## 5. The list and the answer

`<data>/plugins/plugins.json`, written by the launcher (also by hand: it is
plain JSON):

```json
{
  "format": 1,
  "mods": [
    { "id": "lock-strafe", "enabled": true, "settings": { "mode": "lock" } }
  ]
}
```

The order of `mods` is the load order. A mod's folder that is gone drops out;
a folder that is not in the list is shown, off, at the end.

The port, at start, unless `--no-mods`:

1. reads the list (`--plugins FILE` names another, for tests; default
   `<data>/plugins/plugins.json`). No file, or no mod on: nothing else runs;
2. reads each enabled mod's manifest and checks, in this order: the format,
   the game and the disc, the interface's version, a library for this
   platform, the dependencies (on, and earlier in the list), the conflicts
   (with any mod on, either side's list). A mod refused here is not opened;
3. opens each library that passed, in the list's order; checks that
   `openrac_mod_query` gives the same interface version and id as the
   manifest; calls `openrac_mod_load`. A mod that returns non-zero (or asks
   for a hook point that is not there) is closed again, with its hooks
   undone, and the mods after it that require it are refused;
4. writes `<data>/plugins/status.json`:

```json
{
  "format": 1,
  "time": "2026-10-21T14:02:11+02:00",
  "game": "rac1", "version": "pal", "platform": "macos-arm64",
  "api": "1.0", "openrac": "0.9.0",
  "purist": false,
  "mods": [
    { "id": "lock-strafe", "version": "1.0.0", "state": "loaded",
      "hooks": ["Pad_ProcessReport", "Hero_TurnToHeading"] },
    { "id": "other", "state": "refused", "reason": "needs interface 2.0; this OpenRAC has 1.0" }
  ],
  "hook_points": ["Pad_ProcessReport", "Hero_UpdateMotionInput", "..."]
}
```

States: `loaded`, `refused` (a check failed: the reason says which), `failed`
(its library could not be opened, or its load returned an error: the reason
is the system's or the mod's own), `off` is not written (the launcher knows).

## 6. The interface

`sdk/include/openrac/openrac_mod.h` is the contract; this is its shape.

```c
/* The mod's library exports two functions, and may export a third. */
OPENRAC_MOD_EXPORT void openrac_mod_query(openrac_mod_info* info);      /* the interface it was built for, its id */
OPENRAC_MOD_EXPORT int  openrac_mod_load(const openrac_api* api, openrac_mod* mod);  /* 0: loaded */
OPENRAC_MOD_EXPORT void openrac_mod_unload(void);                        /* optional, at exit */
```

`openrac_api` is a table of functions and facts that the port fills:

| Part | What |
|---|---|
| `size`, `api_major`, `api_minor` | the table's size and version: a mod built for 1.2 checks `api_minor >= 2` before using a 1.2 function; the port refuses a mod built for a minor above its own |
| `game`, `version`, `openrac_version`, `platform` | `"rac1"`, `"pal"`, `"0.9.0"`, `"macos-arm64"` |
| `memory` | the base of game memory: a game address `a` is `memory + a`, for the life of the program |
| `log(mod, level, text)` | a line in the session log, prefixed with the mod's id |
| `fail(mod, reason)` | during load: the reason `status.json` gives when load returns non-zero |
| `setting_bool/int/float/string(mod, key)` | the mod's settings, checked against its manifest |
| `hook(mod, name, signature, replacement, &original)` | take a hook point (section 4) |
| `on_frame(mod, fn, user)` | `fn(user)` after each game tick (50 or 60 a second), in load order |
| `on_pad(mod, fn, user)` | `fn(user, port, report, size)` for each pad report before the game reads it: the report as the console's pad library gives it, which the mod may change |
| `function(name)` | a game function by its name in this disc's build (`func_001F9F90`, `FUN_001f9dc8`), or null |
| `function_at(address)` | the function at a code address, of the level loaded (the game's own function pointers) |
| `relocate_data(a)`, `relocate_level_data(level, a)` | a global's address in the code loaded now (guest.h's `OPENRAC_DATA`, `OPENRAC_LDATA`) |
| `loaded_level()` | the level whose program is loaded, -1 for none |
| `stack_push(size, &saved)`, `stack_pop(saved)` | room on the game's stack for a call that takes a pointer to a local |

A mod for several discs carries its own table of the addresses and names it
uses on each, as the port's `rac1_version.h` does, and picks it by
`api->version`; one it has no table for it refuses (`fail`).

**Compatibility.** The major version changes when something in the table
changes or goes; a mod built for another major is refused. The minor version
grows when something is added at the end; a mod built for a minor at most
the port's loads. The port keeps every hook point it has had, with its
signature, for as long as the major version lasts.

**What a mod must not do**: keep pointers into game memory across levels
(the level's program moves its globals: ask `relocate_*` each time), call into
the port from another thread, or unload itself. Everything happens on the
game's thread.

## 7. Safety and crashes

- **Before the game starts**: the checks of section 5. The launcher runs the
  same checks on what it can see (format, game and disc, interface version
  against its port's, a library for this platform, dependencies and
  conflicts) and shows the result on the mod's row before Play; the port's
  `status.json` adds what only loading tells (a hook point missing, the mod's
  own refusal).
- **The log**: the session header gets a `mods` line (`lock-strafe 1.0.0`, or
  `none`, or `none (purist mode)`), and each refused mod a warning.
- **A crash**: the report (`<data>/crashes`) lists the loaded mods (id,
  version, library) in a section of its own, and when the faulting address,
  or one of the host frames, lies in a mod's library (`dladdr`;
  `GetModuleHandleEx` on Windows), it says so: `in mod lock-strafe
  (lock_strafe.dylib+0x1a2c)`. The launcher's crash notice suggests trying
  again in Purist mode when mods were loaded.
- **Trust**: a native mod is a program and runs with the game's rights. The
  launcher says so when one is installed and shows where it came from
  (`homepage`); OpenRAC does not sign or vet mods. On macOS the launcher
  takes the download quarantine off the libraries it installs, which the
  system would otherwise refuse to open.
- **No mods**: the port reads nothing but the list's absence, binds no hook,
  runs no callback, and the game is the retail game (with the port's fixes).

## 8. The SDK

`sdk/` in OpenRAC: the headers, a CMake package, a template and an example.
No copy of OpenRAC is needed to use it: each version is published as one
archive, `openrac-mod-sdk-<version>.zip`, on the release `sdk-v<version>`
of OpenRAC/OpenRAC, with its SHA-256 ([sdk.yml](../../.github/workflows/sdk.yml),
[package_sdk.py](../../sdk/tools/package_sdk.py)). Its version is the
interface's, `MAJOR.MINOR`, and a patch number for fixes to the SDK itself
(`sdk/VERSION`): the SDK 1.0.x builds mods for interface 1.0. The archive is
the same bytes wherever it is made, so a mod's build can pin the version and
its hash (rac1-mods does: `sdk.lock`).

```sh
unzip openrac-mod-sdk-1.0.0.zip
cmake -S my-mod -B build -DOpenRACMod_DIR=$PWD/openrac-mod-sdk-1.0.0/cmake
cmake --build build
python3 openrac-mod-sdk-1.0.0/tools/package_mod.py build/package/*   # dist/<id>-<version>-<platform>.zip
```

`openrac_add_mod(<target> MANIFEST openrac-mod.json SOURCES ...)` makes the
shared library with the interface's headers, hidden symbols but the entry
points, no undefined symbols left for the port to fill (each platform's
linker is told so), and a `package` folder laid out as the mod's archive.
`package_mod.py` checks the manifest by the rules of section 3, puts the
library at its platform's path and writes the `.zip`; `--merge` joins the
archives of several platforms into one, which is what a mod's release
publishes. [sdk/README.md](../../sdk/README.md) has the steps for macOS,
Linux and Windows (Clang or MSVC).

## 9. The launcher

The Mods tab shows, for each game with a port, a **Mods** panel above the
level editor's steps and the packs:

- each installed mod, in load order: its name, version, author, a switch, and
  its state in a few words (Ready, Not for this disc, Needs OpenRAC with
  interface 2.0, Needs mod X, Conflicts with Y, Not loaded at the last start:
  the reason). Up and down move it in the order;
- the mod's settings, from its manifest, as the launcher's settings rows
  (a switch, a segmented control or radios, a number, a text field:
  [STYLE.md](../../launcher/docs/STYLE.md)), written to `plugins.json` as
  they change;
- **Add a mod**: a `.zip` or a folder (or one dropped on the page). A format-2
  manifest goes to `plugins/<id>` (an older version of the same id is
  replaced, its settings kept); a format-1 one is a data pack and goes to the
  packs below. A mod for another game is refused;
- **Remove**, in the row's menu, set apart in `--err`;
- **Purist mode** (one switch for the launcher, in Settings and on the Mods
  tab): Play passes `--no-mods`.

Play passes nothing more: the port reads `plugins.json` from the data folder
it is given. The Rust side is `core/src/plugins.rs`, with tests on mods made
in a temporary folder.

## 10. What is proposed or open

- **Proposed: a mod with data.** A format-2 mod with an `assets` folder that
  is a data pack, added to the packs when the mod is installed and on when the
  mod is, so a mod can carry code and textures together.
- **Proposed: settings in the game.** The in-game drawer showing the loaded
  mods and their settings, written back to `plugins.json`.
- **Proposed: more hook points** as mods ask for them, and a hook on the
  renderer's frame for mods that draw.
- **Open: the license of the SDK and of mods.** The SDK is GPL-3.0-or-later
  like the rest of OpenRAC's top level. A mod runs inside the GPL'd port; a
  permissive license for the SDK's headers would let mods choose their own.
  For the maintainers to settle (AGENTS.md, rule 11).
- **Open: a mod index.** Mods are installed from files today; a list to
  browse and update them from, as OpenGOAL's launcher has for its mods,
  waits on where it would be hosted.

## 11. How others do it

- **OpenGOAL** (Jak and Daxter) ships mods as whole builds of its game from
  a fork, listed in a JSON index its launcher reads; a mod is its own copy
  of the game. OpenRAC's mods are smaller: plugins to one build. The launcher
  side (installed per game, switched from the launcher, the game started with
  them) follows OpenGOAL's.
- **Reloaded-II** (a loader for Windows games): each mod a folder with a
  `ModConfig.json` (id, name, version, author, dependencies, supported
  applications), the loader resolving dependencies and order; mods talk to
  the game through interfaces the loader hands them. The manifest and the
  checks here are of that shape.
- **BepInEx** (Unity games): plugins with a declared id, version and
  dependencies (`BepInDependency`, `BepInIncompatibility`) and a
  per-plugin configuration with defaults and descriptions, which the
  settings schema mirrors.
- **SKSE and Detours-style hooks**: an entry point queried for its version
  before it is loaded, and a hook that returns the original to call. Here the
  hook points are declared by the port, so nothing patches code at run time,
  which keeps Apple Silicon, Windows and Android alike.
