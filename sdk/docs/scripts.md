# Script mods (Luau)

New in interface 1.1 (SDK 1.1). A script mod is a folder of
[Luau](https://luau.org) files and a manifest. It needs no compiler, runs on
every platform OpenRAC runs on, Android included, and is sandboxed: it sees
only the `openrac` table and the safe parts of Luau's library.

Start from the SDK's `template-script/` ([getting started](getting-started.md)).

## How it runs

At the game's start, each file the manifest's `scripts` lists runs once, in
order. That first run is when the mod takes its hooks and callbacks; after it,
the game calls them.

```lua
--!strict
local swap = openrac.setting("pair") :: string

openrac.on_pad(function(port: number, report: buffer)
    local buttons = buffer.readu16(report, 0)   -- a bit is clear while its button is held
    -- ... change buttons ...
    buffer.writeu16(report, 0, buttons)
end)

openrac.on_frame(function()
    -- after each game tick: 50 a second on PAL, 60 on NTSC-U
end)
```

## The `openrac` table

| | |
|---|---|
| `openrac.game`, `.version`, `.platform`, `.api` | `"rac1"`, `"pal"` or `"ntsc"`, `"macos-arm64"` ..., the game's interface version (`"1.1"`) |
| `openrac.log(...)`, `openrac.warn(...)` | a line in the game's log, after your mod's id (`print` does the same as `log`) |
| `openrac.setting(key)` | one of your [settings](settings.md) |
| `openrac.hook(id, fn)` | wrap or replace a hook point: `fn(original, ...)` gets its arguments; `original(...)` goes on down the chain and returns its result; `original()` with no arguments passes the call's own; return nothing to keep the original's result |
| `openrac.before(id, fn)`, `openrac.after(id, fn)` | `fn(...)` before or after the rest of the chain, which runs whatever `fn` does |
| `openrac.on_frame(fn)` | `fn()` after each game tick |
| `openrac.on_pad(fn)` | `fn(port, report)` for each pad report before the game reads it; `report` is a `buffer` you may change |
| `openrac.fields` | game state by name, read and written as plain values: `openrac.fields.bolts`, `openrac.fields.hero.position.x` (experimental: see the [reference](reference/hooks-rac1.md#fields)) |
| `openrac.read(type, address)`, `openrac.write(type, address, value)` | game memory; `type` is `u8 i8 u16 i16 u32 i32 f32`; addresses in game RAM only (below 32 MB) |
| `openrac.pvars(moby)` | with the behaviour catalogue: the per-class variables of the moby at that address, by name (experimental) |
| `require(name)` | another `.luau` file of your mod's own folder: `require("util")`, `require("lib/util")` |

Hook points are named by their [ids](hooks.md#stable-ids)
(`"rac1.player.turn_to_heading"`); the older names work too. The full list,
with each one's arguments, is the [reference](reference/luau-api.md).

## The pad report

`on_pad`'s buffer is the controller's report, 18 bytes:

| Bytes | |
|---|---|
| 0-1 | the buttons, a little-endian 16-bit word; a bit is **clear** while its button is held. Bits 0-7: Select, L3, R3, Start, Up, Right, Down, Left; bits 8-15: L2, R2, L1, R1, Triangle, Circle, Cross, Square |
| 2-5 | right stick x, y; left stick x, y (0-255, 128 at rest) |
| 6-17 | pressures (0-255): right, left, up, down, triangle, circle, cross, square, L1, R1, L2, R2 |

## The sandbox

Available: `math`, `string`, `table`, `buffer`, `bit32`, `utf8`, `vector`,
`coroutine`, `os.clock`, `debug.traceback`, `print`. Not available: `io`, the
rest of `os`, `load`, `loadstring`, `dofile`, `getfenv`, `setfenv`, and files
outside your mod's folder. Library tables and `openrac` are read-only, and each
mod has a Luau state of its own, so mods cannot see each other's globals.

## The time budget

Each call into your script (the first run, a hook, a callback) may take at most
2 ms (time spent in the game's own code through `original` does not count).
A script that runs longer, or raises an error, is stopped:

- the error and its traceback go to the game's log, after your mod's id;
- the mod is marked `failed` with the reason in `status.json`, and the
  launcher shows it ([troubleshooting](troubleshooting.md));
- its hooks pass straight to the original from then on, and its callbacks stop.

The game carries on. Keep per-tick work small, and spread long work over
several ticks.

## Autocomplete

The SDK's `types/rac1.d.luau` declares the `openrac` table with every hook
point and field. With [luau-lsp](https://github.com/JohnnyMorganz/luau-lsp),
pass it as a definitions file; the script template's `.vscode/settings.json`
does that for VS Code.

## Packaging

A script mod has nothing to build: its archive is the folder,
`<id>-<version>.zip`, for every platform ([packaging](packaging-and-releases.md)).
From a repository link, the launcher can even install the repository's own
files with no release at all ([install from a link](install-from-link.md)).
