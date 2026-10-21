# The manifest: `openrac-mod.json`

Every mod has one, at the top of its folder. The launcher reads it to show the
mod and decide whether it can run; the game reads it again at start. Both apply
the same rules, and the SDK's `tools/package_mod.py --check openrac-mod.json`
checks a manifest the same way.

## A script mod

```json
{
  "format": 2,
  "id": "button-swap",
  "name": "Button swap",
  "version": "1.0.0",
  "author": "Your name",
  "description": "Swaps two buttons of the pad.",
  "license": "MIT",
  "homepage": "https://github.com/you/your-mods",
  "type": "script",
  "api": "1.1",
  "scripts": ["main.luau"],
  "games": { "rac1": ["pal", "ntsc"] },
  "settings": [
    { "key": "pair", "name": "Buttons", "type": "choice", "default": "cross-square",
      "choices": [ { "value": "cross-square", "name": "Cross and Square" },
                   { "value": "l1-r1", "name": "L1 and R1" } ] }
  ]
}
```

## A native mod

```json
{
  "format": 2,
  "id": "lock-strafe",
  "name": "Lock-strafe",
  "version": "1.0.0",
  "author": "OpenRAC contributors",
  "description": "Strafe as in the later games.",
  "api": "1.0",
  "games": { "rac1": ["pal", "ntsc"] },
  "plugin": {
    "macos-arm64": "lib/macos-arm64/lock_strafe.dylib",
    "linux-x86_64": "lib/linux-x86_64/lock_strafe.so",
    "windows-x86_64": "lib/windows-x86_64/lock_strafe.dll"
  },
  "hooks": ["rac1.pad.process_report", "rac1.player.turn_to_heading"]
}
```

You do not write `plugin` by hand: the SDK's CMake (`openrac_add_mod`) fills in
the platform it built, and `package_mod.py --merge` joins the platforms
([packaging](packaging-and-releases.md)).

## Fields

| Field | Required | What it is |
|---|---|---|
| `format` | yes | `2`. (A file with `"format": 1` is a data pack of textures, models or levels, made by the level editor.) |
| `id` | yes | Lower-case letters, digits and single hyphens, at most 64 characters: `big-jump`. It never changes between versions: the launcher replaces an installed mod of the same id. |
| `name` | yes | What players see. |
| `version` | yes | `MAJOR.MINOR.PATCH`, optionally `-suffix` (`1.2.0`, `2.0.0-beta`). Raise it for each release: updates compare versions. |
| `author`, `description`, `license` | no | Shown in the launcher. |
| `homepage` | no | An `https://` address, shown as the mod's source. |
| `type` | no | `"native"` (the default) or `"script"`. |
| `api` | yes | The mod interface you wrote for, `MAJOR.MINOR`: `"1.0"` for native mods, `"1.1"` or later for script mods. A game whose interface has another major version, or a lower minor, does not load the mod and says why. |
| `games` | yes | The game and the discs the mod is for: `{"rac1": ["pal", "ntsc"]}` (PAL and NTSC-U). |
| `scripts` | script mods | The `.luau` files to run at start, in order, as paths inside the mod (forward slashes). |
| `plugin` | native mods | Platform to library path inside the mod. Platforms: `macos-arm64`, `macos-x86_64`, `linux-x86_64`, `linux-aarch64`, `windows-x86_64`, `windows-arm64`. |
| `hooks` | no | The hook points the mod takes, by id ([hooks](hooks.md)). The launcher shows which mods share a hook point. |
| `settings` | no | Options for players ([settings](settings.md)). |
| `requires` | no | Mods that must be on, and load before this one: `[{"id": "other-mod", "version": ">=1.2.0"}]` (`version` is `>=X.Y.Z`, `=X.Y.Z`, or left out). |
| `conflicts` | no | Mods it cannot run with: `[{"id": "other-strafe", "reason": "both take L2 and R2"}]`. |
| `load` | no | Order hints: `{"after": ["camera-tweaks"], "before": []}`. The launcher places the mod accordingly when it is switched on. |

## What gets a mod refused

The launcher shows the reason on the mod's row, and the game writes it to
`status.json` ([troubleshooting](troubleshooting.md)):

- another game or disc than the one played;
- an `api` the game's interface does not cover;
- a native mod with no library for this computer's platform;
- a mod it `requires` that is missing, off, too old, or later in the list;
- a conflict with a mod that is on (either mod may list the other).
