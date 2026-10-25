# OpenRAC Launcher

[![checks](https://github.com/OpenRAC/OpenRAC/actions/workflows/checks.yml/badge.svg)](https://github.com/OpenRAC/OpenRAC/actions/workflows/checks.yml)
[![openrac.dev](https://img.shields.io/badge/community-openrac.dev-e8a317)](https://openrac.dev/)
[![Discord](https://img.shields.io/badge/Discord-join%20us-5865F2?logo=discord&logoColor=white)](https://discord.gg/Sfd2B54PDG)

The launcher for OpenRAC's Ratchet & Clank games. It sets a game up from your
own disc, plays its native PC port, and opens its levels in Godot for
editing. It is OpenRAC's version of
[OpenGOAL's launcher](https://github.com/open-goal/launcher) for Jak and
Daxter. Nothing here emulates the console.

It covers all six games: Ratchet & Clank (PAL and NTSC-U), Going Commando, Up
Your Arsenal, Deadlocked, Size Matters and Secret Agent Clank. Only
Ratchet & Clank (PAL and NTSC-U) is playable today. The others can be browsed, and set
up, built and checked where their projects support it. Each game's Play
button says what is still missing.

An installed launcher needs only your disc image. Its installer carries the
game built for your system, the set-up and level tools, and a Python to run
them ([tools/bundle_standalone.py](../tools/bundle_standalone.py), put
together by the [launcher workflow](../.github/workflows/launcher.yml)). The
launcher uses this copy when it finds no OpenRAC checkout; a contributor's
checkout comes first. With the copy there is nothing to build: pick the
disc, prepare the levels, Play.

It is a [Tauri 2](https://tauri.app/) app. Rust does every file and process
operation (`core/` and `src-tauri/`); Svelte 5 draws the UI (`src/`), styled
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

OpenRAC's own rules apply here too ([CONTRIBUTING.md](../CONTRIBUTING.md),
[SOURCING.md](../docs/policy/SOURCING.md)):
the launcher never downloads, uploads or bundles a game.

## Using the launcher

### Quick start

1. Start the launcher and choose the folder with your PlayStation 2 disc
   images. It looks two folders deep. It knows a disc by the boot program its
   `SYSTEM.CNF` names, whatever the file is called. A `.chd` works too; it is
   unpacked once into the install folder
   ([core/src/discs.rs](core/src/discs.rs), [core/src/chd.rs](core/src/chd.rs)).
2. The launcher sets the game up from the disc with
   [tools/extractor.py](../tools/extractor.py). Then it prepares the levels
   and checks that the game can draw each one. Play appears after that check.
   A level that fails is read from the disc again and checked again.
3. Press **Play**. In a checkout, **Build the game** first builds the native
   port ([tools/build_port.py](../tools/build_port.py)). An installed
   launcher already carries a build.

**Remove the set-up** deletes what set-up made, like OpenGOAL's uninstall. It
keeps the disc image and the saves. The full flow is in
[docs/INTEGRATION.md](docs/INTEGRATION.md#2-setting-a-game-up).

### A game's page

Every game has the same page. Its main button always names the next step:
**Set up from your disc**, **Build the game** or **Play**. Rarer actions
(rebuild, open the folder, the activity log, remove the set-up) are in the
menu beside it. The list on the left switches games. The Games tab opens on
the game you used last.

- **Saves**: the port's memory card, in `memcard/<serial>/` of the data
  folder: `%LOCALAPPDATA%\openrac` on Windows,
  `~/Library/Application Support/OpenRAC` on macOS, `$XDG_DATA_HOME/openrac`
  or `~/.local/share/openrac` on Linux. Each slot shows its planet, bolts and
  save time, with backups you can restore. **Open folder** opens that
  folder. **Edit** opens a slot in the save editor: planet, bolts, health
  upgrades, weapons, gadgets, ammo, gold weapons, gold bolts, planets and
  skill points. **Advanced save editing** adds every field and the raw bytes.
  The editor will not write while the game runs, backs up the card before
  every write, and offers Undo
  ([docs/ARCHITECTURE.md](docs/ARCHITECTURE.md#the-save-editor)).
- **Crashes**: the game page says when the game stopped unexpectedly. Your
  saves are kept. **Copy report** gives text for a
  [GitHub issue](https://github.com/OpenRAC/OpenRAC/issues) or Discord.
  **Save report** writes a `.zip` to Downloads. Home paths are shortened to
  `~`, so nothing personal goes in.
- **Logs**: every play session and crash is kept under **Help**, newest
  first, with the same Copy and Save actions. A recent crash marks the game
  in the list. **Detailed logs** makes the game write more, for when we ask.
- **Differences from the original**: for a game with a port, what it does
  differently and why. **Game fixes** can be switched; **Kept from crashing**
  and **Playing on a PC** are listed for the record. The list is
  `port/game/<game>/differences.json`
  ([docs/port/SETTINGS.md](../docs/port/SETTINGS.md#8-game-fixes-and-the-differences-from-the-original)).
- **Two discs of a game**: with developer tools on and both the PAL and
  NTSC-U images of a game present, a small **Disc** switch under Play picks
  the one the page uses. Each disc keeps its own set-up, levels and build.
  Switching moves the other aside as `iso_data/<game>@<version>` and never
  deletes it ([core/src/disc_choice.rs](core/src/disc_choice.rs)).
- **Layout**: Settings → Launcher switches between Compact (the default) and
  Classic. Compact puts the game's name and its main button side by side
  (`src/layouts/compact.css`).

## Mods and level editing

**Gameplay mods** come in two kinds, and each row says which: native mods
(small programs the game loads at start) and script mods (Luau files run in a
sandbox). Add a `.zip` or a folder, or drop one on the page, to install it
into `<data>/plugins/<id>`. Each mod has a switch, a place in the load order,
its settings and Remove. When a mod cannot load, it says why. Changes apply
from the game's next start. **Purist mode** starts the game with no mods.
Android loads no native mods. Its Mods tab shows the script mods, Enhanced
graphics and the packs on one page, without the level editor.

**Enhanced graphics** (experimental) is built in. It is the first row of
Gameplay mods for Ratchet & Clank (PAL and NTSC-U) and adds bounced light,
modern sun and Ratchet shadows, and ambient occlusion, from the game's Vulkan
renderer. Its page has:

- the main switch;
- Probe lighting, Modern shadows, and Ambient occlusion with its quality;
- Texture upscaling (Normal or Edge-aware 2x);
- an Advanced fold with the Bounce light slider.

Each change is saved to the game's settings file at once. While it is on,
Play uses the Vulkan renderer. The bounced light is baked on this computer
from your own extracted levels, one level per thread. That happens when it is
first switched on, and again after the levels are read from the disc again.
**Lighting** on its page shows the progress, with **Prepare lighting**. A
level without a bake keeps its original lighting. Nothing derived from the
game is downloaded
([port/enhanced/lighting/README.md](../port/enhanced/lighting/README.md#in-the-launcher)).

**Add from a link** takes a GitHub repository or release. Before installing
anything, the launcher shows the mods, their author, and whether this
computer's game can run them.

- A gameplay mod is installed from its release `.zip`, and only when
  `SHA256SUMS` lists the same hash. Nothing is compiled locally.
- A script mod is its release `.zip` or the repository's files.
- A texture and model pack comes from the repository and goes to the packs.
  Packs also work from other git hosts if git is installed.

A linked mod remembers its source in `<data>/plugins/sources.json`. **Check
for updates** (also run once a day) offers **Update to X**; **Keep this
version** stops that
([docs/port/MODS.md](../docs/port/MODS.md#10-mods-in-a-git-repository)).

**Level editing** has three steps for each editable game:

1. **Open the editor** and choose a level. Each level is its own Godot 4
   project under `assets/levels/<game>`.
2. **Pack** the edits into `assets/packed/<game>`.
3. **Play** with them.

The disc image is never changed. With developer tools on, the page also shows
the folders and commands behind the buttons
([editor/README.md](../editor/README.md#pack-your-edits)).

**Models and textures** exports a level's models (glTF, with skeletons) and
textures (PNG), and imports edited folders back as overrides. It also manages
the packs: on or off, order, remove, add a `.zip` or folder, or drop one on
the page. **Play with your mods** first combines the packed levels with the
packs that are on ([editor/MODDING.md](../editor/MODDING.md)).

Going Commando, Up Your Arsenal and Deadlocked (NTSC-U) are early access.
Their levels open in Godot and **Models and textures** works, but edits are
not packed back and the games are not playable yet. Their packs wait in Your
mods ([editor/README.md](../editor/README.md#the-later-games-early-access)).

## Develop

With **developer tools** on (Settings → Launcher), the app also shows
decompilation progress, disc serials, the Tasks tab, save file names and
sizes, and the details of the checks.

You need:

- Node 20+;
- Yarn, the version in `package.json`, through Corepack (`corepack enable`);
- Rust stable. On Windows, use Rust's MSVC toolchain. If your default is GNU,
  run `rustup override set stable-x86_64-pc-windows-msvc` in this folder.
- On Linux, Tauri's system libraries. On Debian and Ubuntu:
  `libwebkit2gtk-4.1-dev libgtk-3-dev librsvg2-dev libsoup-3.0-dev`. Other
  systems: [Tauri's prerequisites](https://tauri.app/start/prerequisites/).

```sh
cd launcher
yarn install
yarn dev           # UI in a browser at http://localhost:1430, with mock data
yarn tauri dev     # the desktop app, with the real Rust side
yarn tauri build   # an installer for this platform, in target/release/bundle/
```

The browser preview reads the real `games/*/game.json`,
`progress/summary.json` and `actions.json`. It makes up the rest, such as
which games are set up and what a job prints (`src/lib/mock.ts`). Add
`?setup` for the first-run screen, `?version=rac1/pal` for a game page, or
`?page=tasks` for Tasks.

GitHub builds installers for Windows, Linux and macOS on every change to
`launcher/`, on `master` and on pull requests. They are the run's artifacts
under Actions. A `launcher-v<version>` tag also puts them on a draft release.
The installers are not signed
([launcher.yml](../.github/workflows/launcher.yml)).

## Check

```sh
yarn verify                                            # svelte-check, ESLint, Prettier, Vitest
yarn test:android                                      # the Android pages in a browser (e2e/); once before:
                                                       #   yarn playwright-core install chromium-headless-shell
cargo test -p openrac-launcher-core                    # the Rust logic; no WebKit needed
cargo clippy --workspace --all-targets -- -D warnings  # needs Tauri's system libraries
cargo fmt --all --check
python3 -m unittest discover -s ../tools               # includes the extractor's tests
```

`cargo test -p openrac-launcher-core` also checks `actions.json` against the
checkout: every version key is in a `game.json`, no id is repeated, only
known placeholders are used, and every `docs` path exists. CI runs all of
these in `.github/workflows/checks.yml`.

## Layout

```
launcher/
├── actions.json        what the launcher can run, per version: the file to edit to connect a game
├── core/               Rust, no window: games, actions, status, set-up, settings, detection, jobs,
│                       Discord presence (+ tests)
├── src-tauri/          the desktop app: Tauri commands over core/, window and bundle config, icons
├── src/                the UI: Svelte 5 + TypeScript
│   ├── lib/            api.ts (commands), generated/ (their types, from core/), mock.ts (browser preview), app state, queue, themes;
│   │                   the helpers pages share (format, clipboard, jobText, labels), each with its tests
│   ├── components/     header, game card, action card, progress bar, path field, toasts, icons
│   └── pages/          Library, Game (player and developer views), Tasks, Settings (also the first run)
├── public/             the favicon (the logo's mark), game backdrops and previews
└── docs/               ARCHITECTURE, INTEGRATION, STYLE, RELEASING
```

## Files and network

What the launcher writes, and when it goes online:

- **Settings**: `launcher.json` in the per-user config folder:
  `~/.config/dev.openrac.launcher/` on Linux,
  `~/Library/Application Support/dev.openrac.launcher/` on macOS,
  `%APPDATA%\dev.openrac.launcher\` on Windows. Settings shows the path.
- **Progress cache**: `progress.json` beside `launcher.json`, the latest
  figures from [bytematch.dev](https://bytematch.dev), kept for the next start
  without a network. The launcher never writes the checkout's
  `progress/summary.json`.
- **Game installs**: the install folder (Settings; by default `~/OpenRAC` or
  `%USERPROFILE%\OpenRAC`), with `active/<game>/data/` laid out as OpenGOAL
  does. A developer's other disc waits beside it as
  `iso_data/<game>@<version>`.
- **Saves and game settings**: `memcard/` and `settings/` in the data folder
  (see Saves above), shared with the game. Saves that some development builds
  kept in the install folder are moved to `migrated/` in the data folder at
  start, so nothing is lost.
- **Mods**: `<data>/plugins/`, when you add, switch, order, update or remove
  one: each mod's folder, `plugins.json`, and `sources.json` for linked mods.
  Their downloads are checked, then deleted. A pack's files wait in
  `.downloads/` until the packs take them.
- **Logs**: `openrac-<id>.log` and the five before it, plus `crashes/`, in
  the game's data folder. The launcher's own log is `launcher.log` beside
  `launcher.json`, moved to `launcher.1.log` past 1 MB.
- **Reports**: `OpenRAC-<report>.zip` in your Downloads folder, only when you
  press Save report.
- **Memory card**: backups (`.backup_<time>…` beside the game's folder in the
  card), restored backups and edited slots, always after a backup of the
  whole card.
- **Tool output**: whatever the tools it runs write in the checkout
  (`baserom/` inputs, build output, `assets/godot` for the level editor), all
  ignored by git.
- **Linux desktop**: at every start, the launcher's icons in
  `~/.local/share/icons/hicolor/` and its two `.desktop` files in
  `~/.local/share/applications/`. For each game of the checkout with an icon
  (`port/game/<id>/icon.png`), that icon and two entries named the way the
  game's own window names itself, from its `game.json` ("Ratchet & Clank
  (PAL)"). Then the desktop caches are refreshed.

Online:

- **ByteMatch**, at start-up: the project list and each mapped project's
  JSON, to show current progress. Without a network the cached figures are
  used.
- **GitHub**, only when adding a mod from a link or checking for updates:
  `api.github.com`, raw files and release downloads.
- **Discord**, unless switched off in Settings: Rich Presence over Discord's
  local socket (Linux, macOS) or named pipe (Windows). While a game plays it
  shows the game, planet, bolts and time, and links to openrac.dev and
  GitHub.

## License and credits

The launcher is under the GNU GPL v3 or later, like the rest of OpenRAC's top
level ([LICENSE.md](../LICENSE.md), [COPYING](../COPYING)). Credits, all in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md):

- its look follows the OpenRAC website (MIT); its logo and icons are
  OpenRAC's own;
- it bundles three fonts (SIL Open Font License);
- its design follows the T3SDK launcher for Thief: Deadly Shadows (MIT), and
  two small helpers are adapted from it;
- its disc set-up follows OpenGOAL's launcher and extractor (ISC), from their
  documented behaviour, with no code taken.
