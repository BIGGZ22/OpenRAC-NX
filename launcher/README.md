# OpenRAC Launcher

[![checks](https://github.com/OpenRAC/OpenRAC/actions/workflows/checks.yml/badge.svg)](https://github.com/OpenRAC/OpenRAC/actions/workflows/checks.yml)
[![openrac.dev](https://img.shields.io/badge/community-openrac.dev-e8a317)](https://openrac.dev/)
[![Discord](https://img.shields.io/badge/Discord-join%20us-5865F2?logo=discord&logoColor=white)](https://discord.gg/Sfd2B54PDG)

OpenRAC's desktop launcher for the Ratchet & Clank games. It sets a game up
from your own disc, plays its native PC port, and opens its levels in Godot
for editing. It is OpenRAC's counterpart to
[OpenGOAL's launcher](https://github.com/open-goal/launcher) for Jak and
Daxter; nothing here emulates the console.

It covers all five games in OpenRAC: Ratchet & Clank (PAL and NTSC-U), Going
Commando, Up Your Arsenal, Deadlocked and Size Matters. Ratchet & Clank (PAL)
is the only version playable today. The others can be browsed and, where their
projects support it, set up, built and checked; each game's Play button says
what is still missing.

An installed launcher needs only your disc image to play. Its installer
carries a copy of the game built for your system, the set-up and level tools,
and a Python to run them
([tools/bundle_standalone.py](../tools/bundle_standalone.py); the
[launcher workflow](../.github/workflows/launcher.yml) puts it together). The
launcher uses that copy when it finds no OpenRAC checkout; a contributor's
checkout goes first. With the copy there is nothing to build: the steps are
the disc, the levels, Play.

It is a [Tauri 2](https://tauri.app/) app: Rust for every file and process
operation (`core/` and `src-tauri/`), and Svelte 5 for the UI (`src/`), styled
after [openrac.dev](https://openrac.dev).

| Games                                                                                 | A game                                                                                           |
| ------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| ![The games: one tile each, with what can be done next](docs/screenshots/library.jpg) | ![A game: the main button, its menu, save files and the level editor](docs/screenshots/game.jpg) |

## Read first

| You want to                                                    | Read                                         |
| -------------------------------------------------------------- | -------------------------------------------- |
| Connect a game's set-up, build, checks or play to the launcher | [docs/INTEGRATION.md](docs/INTEGRATION.md)   |
| Understand how the parts fit, add a command or a page          | [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) |
| Change the look, add a game's colours                          | [docs/STYLE.md](docs/STYLE.md)               |
| Know where the native port stands                              | [docs/port](../docs/port/README.md)          |
| Build, package and publish a new launcher release              | [docs/RELEASING.md](docs/RELEASING.md)       |

OpenRAC's own rules apply here too ([AGENTS.md](../AGENTS.md),
[CONTRIBUTING.md](../CONTRIBUTING.md), [SOURCING.md](../docs/policy/SOURCING.md)):
the launcher never downloads, uploads or bundles a game.

## Using the launcher

### Quick start

1. Start the launcher and choose the folder with your PlayStation 2 disc
   images. It looks two folders deep and recognises a disc by the boot program
   its `SYSTEM.CNF` names, whatever the image is called. A `.chd` is accepted
   and unpacked once into the install folder
   ([core/src/discs.rs](core/src/discs.rs), [core/src/chd.rs](core/src/chd.rs)).
2. The launcher sets the game up from the disc with
   [tools/extractor.py](../tools/extractor.py), then prepares its levels and
   checks that the game can draw each one. Play is offered after that check;
   a level that fails is read from the disc once more and checked again.
3. Press **Play**. In a checkout, **Build the game** first builds the native
   port ([tools/build_port.py](../tools/build_port.py)); an installed launcher
   already carries a build.

**Remove the set-up** deletes what set-up made, as OpenGOAL's uninstall does;
the disc image and saves are kept. The detailed flow is in
[docs/INTEGRATION.md](docs/INTEGRATION.md#2-setting-a-game-up).

### A game's page

Every game has the same page. The one main button always says the next thing
to do: **Set up from your disc**, **Build the game**, or **Play**. Less common
actions (rebuild, open the folder, the activity log, remove the set-up) are in
the menu beside it. The list on the left switches games, and the Games tab
opens on the one you used last.

- **Saves**: the port's memory card (`~/.local/share/openrac/memcard/<serial>/`
  on Linux) shows each slot's planet, bolts and save time, with backups you
  can restore. **Edit** opens a slot in the save editor: planet, bolts, health
  upgrades, weapons, gadgets, ammo, gold weapons, gold bolts, planets and
  skill points. **Advanced save editing** adds every field and the raw bytes.
  The editor refuses to write while the game runs, backs the card up before
  every write, and offers Undo
  ([docs/ARCHITECTURE.md](docs/ARCHITECTURE.md#the-save-editor)).
- **Crashes**: the game page says when the game stopped unexpectedly, and your
  saves are kept. **Copy report** gives you text for a
  [GitHub issue](https://github.com/OpenRAC/OpenRAC/issues) or Discord;
  **Save report** writes a `.zip` to Downloads. Home paths are shortened to
  `~`, so nothing personal goes in.
- **Logs**: every play session and crash is kept under **Help**, newest first,
  with the same Copy and Save actions. A recent crash marks the game in the
  list. **Detailed logs** makes the game write more when we ask for it.
- **Differences from the original**: for a game with a port, this lists what
  it does differently and why. **Game fixes** have switches; **Kept from
  crashing** and **Playing on a PC** are for the record. The list is
  `port/game/<game>/differences.json`
  ([docs/port/SETTINGS.md](../docs/port/SETTINGS.md#8-game-fixes-and-the-differences-from-the-original)).
- **Two discs of a game**: with developer tools on and both a game's PAL and
  NTSC-U images present, a small **Disc** switch under Play chooses which one
  the page uses. Each disc keeps its own set-up, levels and build; switching
  moves the other aside as `iso_data/<game>@<version>` and never deletes it
  ([core/src/disc_choice.rs](core/src/disc_choice.rs)).
- **Layout**: Settings → Launcher switches between Compact (the default) and
  Classic. Compact puts the game's name and its main button side by side
  (`src/layouts/compact.css`).

## Mods and level editing

**Gameplay mods** are native mods, small programs the game loads at start, or
script mods, Luau files run in a sandbox. Each row says which. Add a `.zip` or
a folder (or drop one on the page) to install it into
`<data>/plugins/<id>`; each mod has a switch, a place in the load order, its
settings and Remove, and says why it cannot load when it cannot. Changes count
from the game's next start. **Purist mode** starts the game with none of them.
Android loads no native mods; its launcher has no Mods tab yet.

**Add from a link** takes a GitHub repository or release. The launcher shows
the mods, their author and whether this computer's game can take them before
installing anything. A gameplay mod is installed from its release `.zip` only
when `SHA256SUMS` lists the same hash; nothing is compiled locally. A script
mod is its release `.zip` or the repository's files. A texture and model pack
comes from the repository and goes to the packs. A linked mod remembers its
source in `<data>/plugins/sources.json`; **Check for updates** (also once a
day) offers **Update to X**, and **Keep this version** stops it. Other git
hosts work for packs if git is installed
([docs/port/MODS.md](../docs/port/MODS.md#10-mods-in-a-git-repository)).

**Level editing** is three steps for each editable game: **Open the editor**
(choose a level; each is its own Godot 4 project under `assets/levels/<game>`),
**Pack** the edits into `assets/packed/<game>`, and **Play** with them. The
disc image is never changed. With developer tools on, the page also shows the
folders and commands behind the buttons
([editor/README.md](../editor/README.md#pack-your-edits)).

Going Commando, Up Your Arsenal and Deadlocked (NTSC-U) are early access:
their levels open in Godot and **Models and textures** works, but edits are
not packed back and the games are not playable yet. Their packs wait in Your
mods ([editor/README.md](../editor/README.md#the-later-games-early-access)).
**Models and textures** exports a level's models (glTF, with skeletons) and
textures (PNG), imports edited folders back as overrides, and manages the
packs: on or off, order, remove, add a `.zip` or folder, or drop one on the
page. **Play with your mods** puts the packed levels and the packs that are on
together first ([editor/MODDING.md](../editor/MODDING.md)).

## Develop

With **developer tools** on (Settings → Launcher), the app also shows
decompilation progress, disc serials, the Tasks tab, save file names and
sizes, and the checks' details.

You need Node 20+, Yarn (the version in `package.json`, through Corepack:
`corepack enable`) and Rust stable. On Linux, install Tauri's system libraries
(Debian and Ubuntu:
`libwebkit2gtk-4.1-dev libgtk-3-dev librsvg2-dev libsoup-3.0-dev`; see
[Tauri's prerequisites](https://tauri.app/start/prerequisites/) for others).
On Windows, use Rust's MSVC toolchain; if your default is GNU, run
`rustup override set stable-x86_64-pc-windows-msvc` in this folder.

```sh
cd launcher
yarn install
yarn dev           # UI in a browser at http://localhost:1430, with mock data
yarn tauri dev     # the desktop app, with the real Rust side
yarn tauri build   # an installer for this platform, in target/release/bundle/
```

The browser preview reads the real `games/*/game.json`,
`progress/summary.json` and `actions.json`, and makes up the rest (which games
are set up, what a job prints): see `src/lib/mock.ts`. Add `?setup` for the
first-run screen, `?version=rac1/pal` for a game page, or `?page=tasks` for
Tasks.

GitHub builds installers for Windows, Linux and macOS on every change to
`launcher/` on `master` and on pull requests; they are the run's artifacts
under Actions. A tag `launcher-v<version>` also puts them on a draft release.
The installers are not signed
([launcher.yml](../.github/workflows/launcher.yml)).

## Check

```sh
yarn verify                                            # svelte-check, ESLint, Prettier, Vitest
cargo test -p openrac-launcher-core                    # the Rust logic; no WebKit needed
cargo clippy --workspace --all-targets -- -D warnings  # needs Tauri's system libraries
cargo fmt --all --check
python3 -m unittest discover -s ../tools               # includes the extractor's tests
```

`cargo test -p openrac-launcher-core` also checks `actions.json` against the
checkout: every version key exists in a `game.json`, no duplicate ids, only
known placeholders, and every `docs` path exists. CI runs all of these in
`.github/workflows/checks.yml`.

## Layout

```
launcher/
├── actions.json        what the launcher can run, per version: the file to edit to connect a game
├── core/               Rust, no window: games, actions, status, set-up, settings, detection, jobs,
│                       Discord presence (+ tests)
├── src-tauri/          the desktop app: Tauri commands over core/, window and bundle config, icons
├── src/                the UI: Svelte 5 + TypeScript
│   ├── lib/            api.ts (commands and types), mock.ts (browser preview), app state, queue, themes
│   ├── components/     header, game card, action card, progress bar, path field, toasts, icons
│   └── pages/          Library, Game (player and developer views), Tasks, Settings (also the first run)
├── public/             the wrench logo and favicon (from openrac-site), game backdrops and previews
└── docs/               ARCHITECTURE, INTEGRATION, STYLE, RELEASING
```

## Files and network

| Where | What |
| --- | --- |
| **Settings** | `launcher.json` in the per-user config folder (`~/.config/dev.openrac.launcher/` on Linux, `~/Library/Application Support/dev.openrac.launcher/` on macOS, `%APPDATA%\dev.openrac.launcher\` on Windows). Settings shows the path. |
| **Progress cache** | `progress.json` beside `launcher.json`: the latest figures from [bytematch.dev](https://bytematch.dev), kept for the next start without a network. The launcher never writes the checkout's `progress/summary.json`. |
| **Game installs** | The install folder (Settings; by default `~/OpenRAC` or `%USERPROFILE%\OpenRAC`), with `active/<game>/data/` laid out as OpenGOAL does. A developer's other disc waits beside it as `iso_data/<game>@<version>`. |
| **Mods** | When you add, switch, order, update or remove one: `<data>/plugins/` (each mod's folder, `plugins.json`, and `sources.json` for linked mods, whose downloads it checks and then deletes). A pack's files wait in `.downloads/` until the packs take them. |
| **Logs** | `openrac-<id>.log` and the five before it, plus `crashes/`, in the game's data folder. The launcher's own log is `launcher.log` beside `launcher.json`, moved to `launcher.1.log` past 1 MB. |
| **Reports** | `OpenRAC-<report>.zip` in your Downloads folder, only when you press Save report. |
| **Memory card** | Backups (`.backup_<time>…` beside the game's folder in the card), restored backups and edited slots, always after a backup of the whole card. |
| **Tool output** | Whatever the tools it runs write in the checkout (`baserom/` inputs, build output, `assets/godot` for the level editor), all ignored by git. |
| **ByteMatch** | At start-up: the project list and each mapped project's JSON. This is how the launcher shows current progress; it works without a network from the cached figures. |
| **GitHub** | Only when adding a mod from a link or checking for updates: `api.github.com`, raw files and release downloads. |
| **Discord** | Unless switched off in Settings, Discord Rich Presence over its local socket (Linux, macOS) or named pipe (Windows). While a game plays it shows the game, planet, bolts and time, and links to openrac.dev and GitHub. |
| **Linux desktop** | At every start: icons in `~/.local/share/icons/hicolor/`, two `.desktop` files in `~/.local/share/applications/`, and refreshed desktop caches. |

## License and credits

The launcher is under the GNU GPL v3 or later, as the rest of OpenRAC's top
level ([LICENSE.md](../LICENSE.md), [COPYING](../COPYING)). It takes its look,
logo and favicon from the OpenRAC website (MIT) and bundles three fonts (SIL
Open Font License); see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Its
design follows the T3SDK launcher for Thief: Deadly Shadows (MIT), and two
small helpers are adapted from it; its disc set-up follows OpenGOAL's launcher
and extractor (ISC), from their documented behaviour, with no code taken.
