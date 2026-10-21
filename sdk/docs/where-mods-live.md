# Where mods live

Everything is in the player's own folders, never in the launcher's (which is
read-only when installed: an AppImage, the macOS app, the Windows installer's
folder). The launcher keeps its games in an **install folder**, shown in its
Settings; by default:

| System | Install folder | A game's mods |
|---|---|---|
| Linux | `~/OpenRAC` | `~/OpenRAC/active/rac1/data/plugins/` |
| macOS | `~/OpenRAC` | `~/OpenRAC/active/rac1/data/plugins/` |
| Windows | `%USERPROFILE%\OpenRAC` | `%USERPROFILE%\OpenRAC\active\rac1\data\plugins\` |

(If that folder cannot be made, the launcher's own data folder:
`~/.local/share/dev.openrac.launcher`, `~/Library/Application
Support/dev.openrac.launcher`, `%LOCALAPPDATA%\dev.openrac.launcher`.)

## Inside `plugins/`

```
plugins/
├── plugins.json     the mods' order, which are on, their settings (the launcher writes it)
├── status.json      what the game did with each mod at its last start (the game writes it)
├── sources.json     where mods installed from a link came from, for updates
└── <id>/            one mod, as installed: openrac-mod.json, scripts or lib/<platform>/...
```

`plugins.json`:

```json
{
  "format": 1,
  "mods": [
    { "id": "lock-strafe", "enabled": true, "settings": { "mode": "lock" } },
    { "id": "button-swap", "enabled": false, "settings": {} }
  ]
}
```

The order of `mods` is the load order ([hooks](hooks.md)). A folder that is not
listed shows in the launcher, off.

## How the game finds them

The launcher starts the game with `--data <the game's data folder>` and
`--plugins <data>/plugins/plugins.json`, or `--no-mods` in purist mode. Neither
side guesses a path. Run by hand without `--plugins`, the game reads
`<data>/plugins/plugins.json`.

## Without the launcher

Copy your mod's folder to `plugins/<id>/` and add it to `plugins.json`. This is
also the quickest way to try a change: edit the files in place and restart the
game.

## Android

The game runs inside the app there, and Android does not let an app load code it
wrote itself, so native mods are not loaded on Android. Script mods are.
