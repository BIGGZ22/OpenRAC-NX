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

**Two kinds of documentation** (decided 2026-10-21). For modders, the
stable layer is documented in [sdk/docs](../../sdk/docs/README.md), shipped in
the SDK's archive: getting started, the manifest, settings, the hook model with
its stable ids, native mods in C and C++, script mods, packaging and releases,
install from a link, where mods live, troubleshooting, and an API reference
generated from the SDK's headers, bindings and type files
([sdk/tools/gen_reference.py](../../sdk/tools/gen_reference.py), checked in
CI). This page is the design and the internals: why things are as they are,
and how the port, the launcher and the generators do them. Marked
**experimental** in both: the behaviour catalogue and anything that names the
game's internals (moby classes and pvars, the fields exposed to scripts, game
functions and addresses), which follow the decompilation. Guides to the port's
internals wait for its move to C++.

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
| `keybind` (1.2) | `binds` (1 or 2, default 2), `group` | up to `binds` names of `openrac/keys.h`, comma-separated: `"KeyW,ArrowUp"`; `""` binds nothing |

A `keybind` names keys by where they are, as the web's `KeyboardEvent.code`
does (`KeyW`, `Space`, `ShiftLeft`, `Digit1`, `ArrowUp`), mouse buttons
(`MouseLeft`, `MouseRight`, `MouseMiddle`, `MouseBack`, `MouseForward`) and
the wheel (`WheelUp`, `WheelDown`, `WheelLeft`, `WheelRight`). The launcher
shows it as "press a key" buttons and marks an input bound to two keybinds of
the same `group` (settings without one share the group `""`). The manifest
may also list `"presets": [{"id", "name", "values": {key: value}}]`: buttons
over the settings that set several keybinds at once (a keybind a preset does
not name goes back to its default; the launcher adds a Default button of its
own).

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

**Ids and aliases.** Every hook point has a **permanent id**, in lower case:
`<game>.<area>.<name>` (`rac1.pad.process_report`, `rac1.player.turn_to_heading`,
`rac1.camera.follow.apply_overrides`, `rac1.moby.bolt.update`). Its other
names are **aliases**: the name in `libraries.tsv` (`Hero_TurnToHeading`),
each disc's symbol (`func_L00_002124E8`, `FUN_L00_00211e30`), and every name
it has had before. The port's loader, the C++ layer and the scripts take
either, and `status.json` and the launcher speak in ids. The decompilation's
names will change (the port's C++ refactor renames functions); ids do not:

- [sdk/bindings/rac1.json](../../sdk/bindings/rac1.json) gives each hook point
  of its own its id; the catalogue's get theirs by a rule (section 11);
- each hook point is **anchored** to its function's place on the first disc
  (`pal:L00:002124E8`), which a rename does not move;
- [sdk/bindings/rac1.ids.lock](../../sdk/bindings/rac1.ids.lock) keeps every
  id ever given, with its anchor and every alias it has had. The generator
  reuses an anchor's id and keeps its old aliases, refuses a table that gives
  a locked hook point another id or reuses an id, and fails when an id of the
  lock is no longer given. So **an id is never removed, reused or changed**,
  and a mod written today against `Hero_TurnToHeading` (or the id) binds
  after the function is renamed;
- the launcher carries the names-to-ids table
  ([launcher/core/src/hook_ids.json](../../launcher/core/src/hook_ids.json),
  generated) to tell two mods that name one hook point differently that they
  share it.

A new hook point (a hand-written `hook` row, for the item randomizer's item
grants for example) is a row in each disc's `libraries.tsv` and an entry in
the table with its id, as the six are; the generator adds it to the lock.

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

**The behaviour catalogue's hook points (experimental).** Every update
function of the behaviour catalogue is a hook point: 637 of them in
Ratchet & Clank, on both discs, covering 1,114 classes' updates (a function
several classes share is one hook point; its replacement tells them apart by
the moby's class, `oclass`, an `i16` at +0xA6). Their ids are permanent; what
is experimental is everything else about them: their names follow the
decompilation, and the catalogue's pvar names follow its structs, both of
which the C++ refactor will change. Section 11 says how they are made; the
catalogue itself, shipped in the SDK archive:

- **`sdk/catalogue/rac1/classes.json`**, the stable table for bindings (the Luau
  tier) and tools: per class, its update functions with their hook names
  (`Moby_<Class>_Update`, aliases of the ids), status and signature, the classes
  sharing each, and its pvar fields (name, offset, type, meaning). Its format is
  documented in [editor/catalogue.py](../../editor/catalogue.py) and changes only
  with its `format` number;
- `sdk/catalogue/rac1/behaviour.json` (everything, may grow), `behaviour.tsv`, and
  [BEHAVIOUR.md](../../sdk/catalogue/rac1/BEHAVIOUR.md) to read.

`python3 editor/catalogue.py write` regenerates them; `check` fails when stale.

Hook points are added as mods need them; adding one changes nothing for a
game without mods. Which exist is the game header in the SDK, and the port
writes them in `status.json`. A version's table names a hook point only where
that disc has the function; a mod for both discs that needs one the other
disc lacks is refused there, with the reason.

**What a hook point can be.** Any function of the game with C (translated
from the decompilation or from its assembly) can be made one, by a `hook` row
for its symbol; nothing else in the port changes. That covers the three ways
mods change behaviour:

- **wrap**: the replacement does something, calls `original`, and does more
  after (lock-strafe's camera and pad hooks);
- **replace**: the replacement never calls `original` (a moby class's
  behaviour written anew);
- **after** or **before** a named function: a wrap that does its work only on
  one side of the call.

A function the game reaches through a code address rather than by name (a
moby class's update in its class table, a callback the game registers) is a
hook point the same way: the registry of functions by address
(`functions.c`) points at the hook's function, so a call through the table
reaches the mod's replacement too. The behaviour catalogue's class updates are
hook points this way (above).

**A function the port wraps too** (a `wrap` row of its own beside the `hook`
row: NanotechBallUpdate, for the wide-screen orb fix) keeps the port's
function, and hostgen writes the hook as `<name>__hooked`, which the port's
function calls where it called `<name>__game`: the mod replaces the game's
code under the port's fix, and the fix holds for the mod's update too.

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
| `input()` (1.2) | the keyboard and mouse for the tick about to run (`openrac_input`): each key by its USB HID usage, held and pressed since the last tick; the mouse's buttons, its motion since the last tick (raw while captured) and its wheel; whether the window has focus, the cursor is captured, the port's overlay has the input. Never null: without a window nothing is held |
| `take_input(mod, devices)` (1.2) | during load: `OPENRAC_INPUT_KEYBOARD` stops the port's own keyboard table driving pad 0 (WASD, Space, ...), so the mod presses the pad's buttons through `on_pad`; a gamepad goes on as before, the port's window keys stay its own |
| `capture_mouse(mod, capture)` (1.2) | any time: ask for the cursor captured (hidden, held in the window, relative motion) or let go. The port lets it go anyway while the window has no focus or its overlay or debug menu is open |

A mod for several discs carries its own table of the addresses and names it
uses on each, as the port's `rac1_version.h` does, and picks it by
`api->version`; one it has no table for it refuses (`fail`).

`openrac/keys.h` (1.2) names every key, mouse button and wheel direction
("binds") and tests them against `openrac_input`; the C++ layer has
`mod.input()`, `mod.take_input()`, `mod.capture_mouse()`, and scripts
`openrac.key(name)`, `pressed(name)`, `mouse()`, `window()`, `take_input(...)`
and `capture_mouse(on)`. The game's own state (in play, paused, the Quick
Select open) is the game's: a mod reads it from game memory as it reads
anything else of the game (rac1-mods' keyboard-mouse reads the game mode).

**Compatibility.** The major version changes when something in the table
changes or goes; a mod built for another major is refused. The minor version
grows when something is added at the end; a mod built for a minor at most
the port's loads. A mod reports the interface its manifest's `api` names,
not the SDK's: `openrac_add_mod` passes it in (`OPENRAC_MOD_TARGET_MINOR`), so
a 1.1 mod rebuilt with SDK 1.2 still loads on a port of 1.1. 1.2 (SDK 1.2.0)
added the keyboard and mouse (`input`, `take_input`, `capture_mouse`, the
`keybind` setting and manifest presets). The port keeps every hook point it has had, with its
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

## 10. Mods in a git repository

A player pastes a repository's address in the Mods tab (**Add from a
link**), and the launcher finds the mods in it, shows them, and installs the
one chosen, built: players have no compilers, and nothing is ever built on
their machine. For that, a repository keeps to these conventions; the SDK's
template ([sdk/template](../../sdk/template)) and
[rac1-mods](https://github.com/OpenRAC/rac1-mods) follow them.

**Where the manifests are.** One mod: `openrac-mod.json` at the top of the
repository. Several: one folder per mod at the top, each with its
`openrac-mod.json` (rac1-mods: `lock-strafe/openrac-mod.json`). A top-level
`openrac-mods.json`, `{"format": 1, "mods": ["lock-strafe", ...]}`, may list
the folders instead, and then only those are read. In a mod's folder:

```
openrac-mod.json        the manifest (section 3)
README.md, LICENSE      shown to players before they install
CMakeLists.txt, src/    a native mod's sources (built by the workflow, never by the launcher)
tests/                  its tests
level_NN/, ...          a data pack's files, as editor/MODDING.md lays them out (format 1)
```

**Releases carry the built mods.** A tag `v<version>` (or, in a repository
of several mods released apart, `<id>-v<version>`) makes a GitHub release
with, for each mod:

| Asset | |
|---|---|
| `<id>-<version>-<platform>.zip` | the mod for one platform (`macos-arm64`, `linux-x86_64`, `windows-x86_64`, ...): what the launcher downloads |
| `<id>-<version>.zip` | every platform's joined, for installing by hand |
| `SHA256SUMS` | `sha256sum`'s format, a line per asset: the launcher refuses an asset whose hash is not listed or differs |

The SDK's template carries the workflow that makes them
([template/.github/workflows/build.yml](../../sdk/template/.github/workflows/build.yml):
build and test on macOS, Linux and Windows, package, join, publish on a tag),
so a modder pushes a tag and nothing else; rac1-mods uses the same file.

**What the launcher does** (`launcher/core/src/remote.rs`):

1. reads the address: `https://github.com/<owner>/<repo>` (with
   `/tree/<ref>` or `/releases/tag/<tag>` naming a version), or any other git
   host's address;
2. on GitHub, without git: the latest release (or the one named) through
   GitHub's HTTPS API, the repository's tree at its tag for the manifests, and
   each manifest's text. It shows each mod's name, version, author, the
   repository and its owner, the games and discs it is for, its interface
   version and whether this launcher's port can load it, before anything is
   installed, with the warning that a native mod runs code on this computer;
3. installs the chosen mod: a native mod is the release's
   `<id>-<version>-<platform>.zip` for this computer, checked against
   `SHA256SUMS`, then installed as a `.zip` is (section 9). A mod with no
   asset for this platform, no `SHA256SUMS`, a hash that differs, or a
   manifest in the archive that is not the one shown, is refused with that
   reason. A data pack (no `plugin`) comes from its asset when the release
   has one, else from the repository's archive at the release's tag (or the
   commit shown), its folder only;
4. other git hosts: a data pack through `git` (a shallow clone at the
   tag) when git is installed; a native mod needs a host with releases
   (GitHub today; GitLab, Gitea and Forgejo have the same and can follow);
5. remembers where each mod came from, `<data>/plugins/sources.json`:

```json
{ "format": 1,
  "mods": { "lock-strafe": { "url": "https://github.com/OpenRAC/rac1-mods",
                             "path": "lock-strafe", "version": "1.0.0",
                             "tag": "v1.0.0", "commit": null, "pin": false } } }
```

**Updates.** **Check for updates** (on the Mods tab, and once a day when it
opens) asks each source for its newest release with an asset for this
platform whose version is above the installed one (for a data pack with no
releases, a newer commit on its branch), and offers **Update** on the mod's
row. A pinned mod (**Keep this version**) is never offered one. An update
installs as a reinstall does: the mod's place, on/off and settings are kept.

## 11. Three ways to write a mod: C, C++ and Luau scripts

Decided 2026-10-21. The C interface (section 6) is the boundary that lasts:
the port's own code is to become C++, and the table, the entry points and the
hook points' signatures stay as they are through that. Two tiers sit on it.

### C++ (`sdk/include/openrac/mod.hpp`)

A header-only C++17 layer over `openrac_mod.h`, compiled into the mod, never
into the port: nothing of it crosses the boundary.

- `openrac::Mod` wraps the handle and the table: `log`, `fail`, typed settings
  (`setting<bool>("trace")`, `setting<std::string>("mode")`, ...).
- Hooks take lambdas or `std::function`, with the original as a typed callable:
  `mod.hook<openrac::rac1::Hero_TurnToHeading>([](auto original, int mode, float a, float d, float l) { ... original(mode, a, d, l); })`.
  Each hook point is a type in the generated `openrac/games/rac1.hpp`
  (its name, signature and function type), and a static thunk per hook point
  holds the callable, so the C slot still gets a plain function.
- `mod.on_frame(fn)`, `mod.on_pad(fn)`, `openrac::Memory` (typed reads and
  writes at game addresses), `openrac::StackFrame` (RAII over `stack_push` and
  `stack_pop`).
- **No exception crosses the C boundary**: every thunk catches, logs the error
  with the mod's id, and calls the original (a hook) or returns (a callback).
  `OPENRAC_MOD_CPP(id, version, load)` writes the two entry points, and turns
  an exception out of `load` into `fail()`.

The SDK template is C++ (`template/src/mod.cpp`); the lock-strafe in
rac1-mods is the reference C++ mod.

### Luau scripts

[Luau](https://luau.org) (Roblox's Lua, MIT), release 0.741 (commit
`2d88f9b5`), fetched and built by the port's CMake, its virtual machine and
compiler only ([port/cmake/Luau.cmake](../../port/cmake/Luau.cmake),
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md)). A script mod needs no
compiler and runs on every platform, Android included. The runtime is
[port/game/common/mod_scripts.cpp](../../port/game/common/mod_scripts.cpp).

```jsonc
{
  "format": 2, "id": "button-swap", "name": "Button swap", "version": "1.0.0",
  "type": "script",                 // "native" (the default) or "script"
  "api": "1.1",                     // scripts need interface 1.1 or later
  "scripts": ["main.luau"],         // run in this order at load, from the mod's folder
  "games": { "rac1": ["pal", "ntsc"] },
  "hooks": [], "settings": [ ... ]
}
```

Interface **1.1** is 1.0 with script mods: the C table is unchanged, and a port
of 1.1 loads mods built for 1.0. A script mod carries no `plugin`; its archive
is one for every platform, `<id>-<version>.zip`, and from a git repository the
repository's own files are the mod (no release asset, no build).

**What a script sees**: one global table, `openrac`, and the safe parts of
Luau's library: `math`, `string`, `table`, `buffer`, `bit32`, `utf8`,
`vector`, `coroutine`, `os.clock` and `debug.traceback`; `print` writes to the
log. There is no `io` (Luau has none), no other `os`, no `getfenv`, `setfenv`,
`load`, `loadstring` or `dofile`, and every library table and `openrac` itself
is read-only.

| | |
|---|---|
| `openrac.game`, `.version`, `.platform`, `.api` | as `openrac_api`'s (`api` is the port's, `"1.1"`) |
| `openrac.log(...)`, `openrac.warn(...)` | the session log, after the mod's id |
| `openrac.setting(key)` | the mod's setting, typed by the manifest (a boolean, a number or a string; nil for a key it does not declare) |
| `openrac.hook(name, fn)` | wrap or replace: `fn(original, ...)` gets the hook point's arguments as numbers; `original(...)` goes on down the chain and returns its result (`original()` with no arguments passes the call's own); `fn`'s return is the hook point's result (nil: the original's) |
| `openrac.before(name, fn)`, `openrac.after(name, fn)` | `fn(...)` with the arguments, before or after the rest of the chain, which runs whatever `fn` does |
| `openrac.on_frame(fn)` | after each game tick |
| `openrac.on_pad(fn)` | `fn(port, report)` for each pad report, `report` a `buffer` of the report's bytes, which the script may change |
| `openrac.read(type, address)`, `openrac.write(type, address, value)` | game memory, `type` one of `u8 i8 u16 i16 u32 i32 f32`; addresses in game RAM only (below 32 MB), anything else an error |
| `openrac.fields` | the game state exposed by name (`openrac.fields.hero.position.x`, `openrac.fields.bolts`): read and written as plain values; a read-only one, or one this disc does not have, is an error to write |
| `openrac.pvars(moby)` | with the behaviour catalogue (below): the pvars of the moby at that address by name (`.class` its class's name), nil for a class the catalogue does not have |
| `require(name)` | another `.luau` file of the mod's own folder (`"util"`, `"lib/util"`), run once; nothing outside the folder |

Hooks and callbacks are taken only while the scripts first run. Script and
native mods share one chain per hook point, in the list's order: a script mod
after a native one wraps it, and the other way round. At most eight script
mods take one hook point (`OPENRAC_SCRIPT_LINKS`): each place in a chain is a
C function of its own in the binding.

**The binding is generated**, so it follows the hook points and survives the
port's move to C++. Its source is a committed table per game,
[sdk/bindings/rac1.json](../../sdk/bindings/rac1.json):

```jsonc
{
  "format": 1, "game": "rac1", "discs": ["pal", "ntsc"],
  "catalogue": "sdk/catalogue/rac1/classes.json",       // the behaviour catalogue (optional)
  "catalogue_symbols": { "ntsc": "sdk/bindings/rac1.ntsc.tsv" },  // its functions on the other discs
  "catalogue_exclude": { "func_001E9768": "why it is not a hook point" },
  "hooks": [ { "id": "rac1.player.turn_to_heading",       // permanent (section 4)
               "name": "Hero_TurnToHeading", "aliases": [], "signature": "void(i32,f32,f32,f32)",
               "params": ["mode", "accel", "damping", "limit"],
               "symbols": { "pal": "func_L00_002124E8", "ntsc": "FUN_L00_00211e30" },
               "about": "..." } ],
  "bases": { "hero": { "pal": "0x0013F450", "ntsc": "0x0013F350" } },
  "fields": [ { "name": "hero.state", "type": "i32", "base": "hero", "offset": "0x2084",
                "readonly": true, "about": "..." },
              { "name": "bolts", "type": "i32", "address": { "pal": "0x0015EE98", "ntsc": "0x0015ED98" } } ]
}
```

A field's `type` is one of `u8 i8 u16 i16 u32 i32 f32`; its address is a
base's plus its offset, or its own per disc (none: not on that disc). The
table holds the six hook points and the fields the port's own C already
names: the hero's position, facing, move yaw and state, and the bolts.

[sdk/tools/gen_binding.py](../../sdk/tools/gen_binding.py) writes from it,
all committed:

- `port/game/<game>/script_binding.c`: for each hook point eight C
  functions of its signature that hand their arguments to the runtime
  (`openrac_script_dispatch`) and call what they go on to when no script
  handles the call, a caller of the signature for `original`, and the fields
  ([openrac/script_binding.h](../../port/game/common/include/openrac/script_binding.h));
  a game with none gets `script_binding_none.c`, which refuses script mods;
- `sdk/include/openrac/games/<game>.hpp`: the hook points as C++ types for
  the C++ layer;
- `sdk/types/<game>.d.luau`: the `openrac` global's types, for an editor's
  autocomplete (luau-lsp's `--definitions`).

`gen_binding.py --check` (ctest `mod_binding`) fails when an output is not what
the table gives, or when the table's hook points (with the catalogue's) are not
exactly the `hook` rows of every disc's `libraries.tsv`: a hook point added
there needs its line in the table, and the other way round.

**The behaviour catalogue** (the level editor's,
[sdk/catalogue/rac1/classes.json](../../sdk/catalogue/rac1/classes.json),
format 1 of [editor/catalogue.py](../../editor/catalogue.py)) is the
generator's other input, and experimental (section 4). From it:

- **hook points**: each update function, but those `catalogue_exclude` names
  with a reason, is a hook point (`void(u32)` with the moby, or `void()` for
  the one that gets none). The generator writes its `hook` row into each
  disc's `libraries.tsv`, in a block between `# ---- generated by
  sdk/tools/gen_binding.py ...` and `# ---- end of the generated hook points
  ----` that nothing else edits (hand-written rows stay above it). The other
  disc's symbol comes from `catalogue_symbols`
  ([rac1.ntsc.tsv](../../sdk/bindings/rac1.ntsc.tsv), made once with
  `port/tools/versionmap/carry.py names` from the two discs' map: 585 the
  same instructions, 60 changed at the same place); committed, so the
  generator needs no disc;
- **ids**: `rac1.moby.<name>.update`, `<name>` the function's own name less
  its `Update` when it ends so (`BlargTurretUpdate`: `rac1.moby.blarg_turret.update`),
  else its lowest class's name, else `class_<lowest class number>`; two new
  ones that would share an id each get `_<lowest class number>`. Once given,
  the lock keeps it (section 4). The catalogue's hook name
  (`Moby_<Class>_Update`) and the symbols are aliases;
- **pvars**: each class's typed pvar struct (the checked one where it has two)
  as `openrac.pvars(moby)`'s fields: numbers by their type, pointers as
  read-only `u32` addresses, a vector's lanes (`spot_x` ... `spot_w`), arrays
  of up to four numbers (`v_0` ...); larger arrays and nested structs are left
  out. 224 classes have some.

Left out, with the reason in the table: `Profile_Mark` (an empty routine 46
classes take as their update, which the engine also calls as a profiler mark:
a hook would run for every mark and never see a moby), and the seven update
functions the decompilation defines under another name (`..._r`, `..._impl`),
whose `hook` row hostgen does not check against the name it compiles.

What it costs, measured on the Mac (Apple clang, RelWithDebInfo):
643 hook points for Ratchet & Clank (6 + 637), each one load of its slot, a
compare and a branch the processor predicts while no mod takes it.
`hooks.c` is 0.22 MB of C per disc (from 3 KB) and compiles in 0.1 s;
`script_binding.c` is 0.45 MB (eight script links per hook point, one macro
line each) and compiles in 3.8 s per disc; the program's code (`__text`) grows
by 0.73 MB (10.59 to 11.31 MB, 6.9 %), the file by 1.8 MB with its debug
information. A fresh build of the whole port took 182 s at `-j 18`.
`games/rac1.hpp`, 0.31 MB of declarations, adds nothing a mod's build notices.

**Sandbox and budget.** Each script mod has its own Luau state, sandboxed
with `luaL_sandbox` and its scripts run in a thread of its own
(`luaL_sandboxthread`), so mods cannot see or change each other's globals.
Every call into a script (a hook, a callback, the first run) has a budget,
checked by Luau's interrupt: 2 ms by default (`OPENRAC_SCRIPT_BUDGET_MS`),
not counting the time spent in the game's own code that the script calls on to
(`original`). A script that runs over it, or raises an error, is stopped
there: the error and its traceback go to the log with the mod's id,
`status.json` marks the mod `failed` with the reason (`stopped while the game
ran: hook Hero_TurnToHeading: ...`), its hooks pass straight to the original
from then on, and its callbacks stop. A hook whose script stopped before
calling its original has the original run anyway. A crash report says which
script mod was running at the time, as it names a native one.

**Cost.** With no script mod on, no Luau state is made and nothing of it
runs. With one, a hook point the scripts did not take costs nothing more.

### In the launcher

A script mod installs from a `.zip`, a folder or a repository's address
(the repository's archive at the release's tag, or at the newest commit
without releases: its own files). Its row and its preview say **Runs scripts
(sandboxed)** where a native mod says it runs code on this computer.

### In the SDK

`sdk/template-script/` is a script mod's repository (manifest, `main.luau`,
the type definitions, the same workflow, which packages script mods without
building them), and rac1-mods' `button-swap` is the example: one `on_pad`
callback that swaps two buttons, chosen in its settings.

## 12. What is proposed or open

- **Proposed: a mod with data.** A format-2 mod with an `assets` folder that
  is a data pack, added to the packs when the mod is installed and on when the
  mod is, so a mod can carry code and textures together.
- **Proposed: settings in the game.** The in-game drawer showing the loaded
  mods and their settings, written back to `plugins.json`.
- **Proposed: more hook points** as mods ask for them, and a hook on the
  renderer's frame for mods that draw.
- **Decided (2026-10-21): the SDK is MIT.** Everything under `sdk/` (the
  headers, the CMake package, the templates, the tools, the guides) is under
  the MIT license ([sdk/LICENSE](../../sdk/LICENSE)), so mods can be under any
  license their authors choose; the port and the rest of OpenRAC stay
  GPL-3.0-or-later. rac1-mods is GPL-3.0-or-later.
- **Decided (2026-10-21): SDK releases stay in OpenRAC's release list**, as
  `sdk-v<version>`, never marked Latest (the workflow passes `--latest=false`,
  and the launcher reads only its own `launcher-v` releases). A repository of
  their own would need a token across repositories for the workflow.
- **Open: a mod index.** Mods are installed from files and repository
  addresses (section 10); a list to browse them from, as OpenGOAL's launcher
  has for its mods, waits on where it would be hosted.

## 13. How others do it

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
