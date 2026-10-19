# Connecting the games to the launcher

The launcher is scaffolding: the app, its look, the game catalogue, the job
runner, setting a game up from the player's disc and the first actions work,
and every game has its actions written down. This document is for whoever
wires the rest up: what "connected" means, the format of
`launcher/actions.json`, how a game is set up from a disc, and the work left,
in packages that can be taken one at a time.

Playing is the **native port** ([docs/port](../../docs/port/README.md)): the
launcher never runs the retail program or a model of the console.

Read [ARCHITECTURE.md](ARCHITECTURE.md) first for how the parts fit.

## What works today

| Part                                                                                          | State                                                                                                                                                                              |
| --------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| The app: window, header, Games, game pages, Tasks, Settings, first run                        | done; `yarn tauri dev`                                                                                                                                                             |
| Browser preview with mock data                                                                | done; `yarn dev`                                                                                                                                                                   |
| Catalogue from `games/*/game.json` and `progress/summary.json`                                | done, tested against this checkout                                                                                                                                                 |
| Disc and input status (names, sizes, links)                                                   | done; checksums are `openrac.py discs`'s, see [Disc verification](#disc-verification)                                                                                              |
| Detecting the checkout, Python, Godot, Docker                                                 | done                                                                                                                                                                               |
| Setting a game up from the player's disc (OpenGOAL's extract and validate)                    | done; the extractor is tested on synthetic images, not yet run on a real disc from the desktop app                                                                                 |
| Play (the native port)                                                                        | **unverified** for rac1/pal on Windows: `port-build` builds the port (tools/build_port.py), `play` runs it; **planned** for every other version                                    |
| Jobs: queue, live output, cancel (whole process tree)                                         | done, tested on Linux                                                                                                                                                              |
| Repository actions: Identify discs, Verify every checksum, Place inputs, Test OpenRAC's tools | **connected**: Identify discs and the tool tests run from the desktop app on Linux; the other two run with the same command lines from a shell. None yet with a disc in `baserom/` |
| `report-check` for rac1/pal and rac4                                                          | **connected**: the same command lines run from a shell on Linux (they need no disc)                                                                                                |
| Every other per-game action                                                                   | **unverified** or **planned**: the work below                                                                                                                                      |

## Action states

Every action in `actions.json` has a `state`. Change it only when its
definition is met:

| State        | Meaning                                                                            | The launcher                                                  | Done when                                                                                                            |
| ------------ | ---------------------------------------------------------------------------------- | ------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `planned`    | Wanted; no command yet                                                             | shows it greyed out, with its `todo` under "For contributors" | it has a `program` and `args`                                                                                        |
| `unverified` | The command is written from the game's docs but has not been run from the launcher | runs it, marked "Unverified"                                  | someone ran it from `yarn tauri dev` on each listed platform, with real inputs, and it did what its description says |
| `connected`  | Run from the launcher and known to work on its `platforms`                         | runs it, marked "Ready"                                       | (keep it working)                                                                                                    |

When you connect an action: set `state`, narrow `platforms` to what you ran it
on, delete its `todo` (or leave one for the platforms still unchecked), and
say in the commit body what you ran, where, and what it printed at the end.

## actions.json

```jsonc
{
  "format": 1, // this launcher reads 1
  "repository": [/* actions for the checkout as a whole */],
  "versions": {
    "rac1/pal": [/* actions for one version; the key is games/<game>/<version> */],
  },
}
```

One action:

| Field         |                                                                       | Meaning                                                                                                                                                |
| ------------- | --------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `id`          | required                                                              | unique within its list; what the UI sends back                                                                                                         |
| `label`       | required                                                              | the button's title, short ("Build")                                                                                                                    |
| `description` |                                                                       | one or two sentences for players: what it does, how long, what it needs                                                                                |
| `kind`        | required                                                              | `setup`, `build`, `check`, `play` or `edit`: the game page's section                                                                                   |
| `state`       | default `planned`                                                     | see above                                                                                                                                              |
| `program`     | for runnable states                                                   | `{python}`, `{godot}` or `{docker}` (the paths in Settings), a command on PATH (`bash`, `make`), or a path with placeholders (`{dir}/venv/bin/python`) |
| `args`        |                                                                       | the arguments, one per string; placeholders allowed; never a shell line                                                                                |
| `cwd`         | default: the version's folder, or the checkout for repository actions | relative to the checkout (`"."` for the checkout itself)                                                                                               |
| `platforms`   | default: all                                                          | `linux`, `macos`, `windows`; the others' actions are tucked away                                                                                       |
| `requires`    |                                                                       | what must be in place, each shown as a reason when missing: `disc`, `inputs`, `toolchains`, `python`, `godot`, `docker`, `artifact`                    |
| `artifact`    |                                                                       | a file the action makes (a build) or needs (play the build), relative to the checkout; `{artifact}` in `args`                                          |
| `detached`    | default false                                                         | start it and let it run (the game, an editor) instead of a job in Tasks                                                                                |
| `docs`        |                                                                       | the document for this step, relative to the checkout (`#anchor` allowed); the Docs button opens it                                                     |
| `todo`        | required while `planned`                                              | for contributors: what is left to do or check                                                                                                          |
| `options`     | required with `{option}`                                              | the values `{option}` may take; the UI picks one, anything else is refused                                                                             |

Placeholders (anything else is rejected by the tests):

| Placeholder           | Value                                                             |
| --------------------- | ----------------------------------------------------------------- |
| `{root}`              | the checkout, absolute                                            |
| `{dir}`               | the version's folder, absolute (`…/games/rac1`)                   |
| `{python}`, `{godot}` | the paths in Settings (refused while unset)                       |
| `{docker}`            | the path in Settings, or `docker` from PATH                       |
| `{disc}`              | the disc image found for the version (absolute)                   |
| `{boot}`              | the boot executable `openrac.py setup` placed for it              |
| `{artifact}`          | the action's `artifact`, absolute                                 |
| `{serial}`, `{key}`   | `SCES_509.16`, `rac1/pal`                                         |
| `{data}`              | the game's set-up folder, `<install>/active/<game>/data`          |
| `{level}`             | the level a run is for, two digits (00 while listed)              |
| `{path}`              | a file or folder the player chose for the run, absolute and there |
| `{option}`            | one of the action's `options`, chosen for the run                 |

`cargo test -p openrac-launcher-core` checks the file against the checkout:
known versions, unique ids, known placeholders (and no version placeholders in
repository actions), a program for every runnable action, a `todo` for every
planned one, and that every `docs` path exists. Run it after every change.

To list what is left:

```sh
python3 -c "import json; f = json.load(open('launcher/actions.json'))
for k, l in [('repository', f['repository'])] + list(f['versions'].items()):
    for a in l:
        if a.get('state', 'planned') != 'connected': print(f\"{k:10} {a['id']:18} {a.get('state', 'planned'):10} {a.get('todo', '')}\")"
```

## Work packages

Each package can be done on its own. Take one, say so where the group
coordinates, and keep each commit to one action or one package
(`feat(launcher): connect rac1/ntsc's build`).

### 1. Disc verification

This is about the developer's discs in `baserom/`, which the decompilations'
builds read; a player's disc is checked by the extractor when the game is set
up (next section). `core/src/status.rs` finds a disc by name or size and says
"found"; it never hashes a 4 GB image. The checksums are `tools/openrac.py discs`'s job, which the
launcher runs as a job and whose output the user reads.

To show "verified" on the cards: give `openrac.py discs` a machine-readable
mode (for example `--json`, printing `{serial: {"path", "ok", "checked":
"sha1"|"full", "differ": [...]}}`), run it from `status.rs` or as a job whose
result the launcher stores, and cache each result by path, size and
modification time so it is not recomputed. `openrac.py` is a top-level tool
with tests in `tools/test_openrac.py`: the flag and its test go there, in a
`feat(tools)` commit of its own.

### 2. Setting a game up

The player's side follows OpenGOAL's launcher exactly; its own steps and
OpenRAC's are:

| Step             | OpenGOAL                                                                        | OpenRAC                                                                                                                             |
| ---------------- | ------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| The player picks | "Install via ISO": their own disc image                                         | **Set up from your disc**: the same, `.iso` only                                                                                    |
| Extract          | `extractor <iso> --extract --validate --proj-path <install>/active/<game>/data` | `tools/extractor.py <iso> --game <game> --extract --validate --proj-path <install>/active/<game>/data`                              |
| Validate         | the boot executable's serial and hash against its database of known builds      | against `games/<game>/game.json` (serial, SHA-1 of the boot executable); file count and contents hash too once game.json lists them |
| Decompile        | the assets into the port's formats                                              | **not available yet** ([docs/port/ROADMAP.md](../../docs/port/ROADMAP.md), P2)                                                      |
| Compile          | build the game                                                                  | **not available yet** (P5)                                                                                                          |
| Play             | `gk -boot -fakeiso --proj-path …`                                               | the native port, when it exists: the `play` action (planned)                                                                        |
| Uninstall        | removes `iso_data`, `decompiler_out` and `out` for the game                     | **Remove the set-up**: the same three folders                                                                                       |

- The install folder is a setting (by default the launcher's per-user data
  folder); each game lives in `active/<game>/data/`, as in OpenGOAL.
- A failure ends the job with the extractor's message, `error NNNN: ...`,
  using OpenGOAL's number for the same failure (`tools/extractor.py`,
  `ERRORS`); the job's output is in Tasks.
- Ratchet & Clank names only a few files in the disc's ISO 9660 file system
  (RAC1: `SYSTEM.CNF`, the boot executable and `IOPRP243.IMG`) and reads the
  rest by sector. So, unlike OpenGOAL's, the extract step also keeps the
  image itself in `iso_data/<game>/disc.iso` (a hard link, a copy across
  file systems) for the decompile step to read.
- A disc set up this way counts as the version's disc for the level editor
  (`{disc}` in its actions), so a player needs no `baserom/`.
- Code: `core/src/install.rs` (the layout, the state on disk, the job's plan,
  uninstall), the Tauri commands `install_game` and `uninstall_game`, and
  `installGame` in `src/lib/app.svelte.ts`.

To do here: run the extractor from the desktop app with a real disc of each
game and record the result; add each disc's file count and contents hash to
its game.json (the extractor prints them for an unknown build); then the
decompile step, which is the port's asset pipeline.

### 3. Playing

Every version has a `play` action: the native port, built from the
decompiled C, drawn by OpenRAC's own renderer, from the assets the set-up
prepared. [docs/port/ROADMAP.md](../../docs/port/ROADMAP.md) says what each
needs and in which order. A version whose port can be started gets two
actions, as rac1/pal has on Windows:

- `port-build`, "Build the game": `tools/build_port.py <id> --disc {disc}`, which
  configures `port/` the first time and then rebuilds only what changed. The
  disc is read for the functions the decompilation has only as assembly:
  hostgen writes their C from the disc's own executable and level programs
  ([retail.py](../../port/tools/hostgen/retail.py)), which the player's
  machine has where the decompilation's assembly files are not.
  On Windows it sets up Visual Studio's environment itself and finds Clang
  and Ninja, so a player never types a compiler's command. The game page
  offers it in place of Play until the game is built. Each build that
  succeeds leaves `openrac-build.json` in `port/build/release`: the OpenRAC
  version and commit it built, and its games.
- `port-rebuild`, "Rebuild the game from scratch": the same with `--clean`,
  which deletes the build folder first and keeps the `--source` and `--disc`
  it held. The game page's Rebuild the game runs it, and so does Rebuild now
  in the notice the page shows when the game is outdated: built by another
  OpenRAC version than the stamp records, or at a commit before a change to
  what the game is built from (`port/`, `games/<game>/`, the build's own
  files: git decides, [port_build.rs](../core/src/port_build.rs)). The notice
  only recommends: Play never builds a game that is built.
- `port-levels`: `editor/extract.py port {disc} assets/port/rac1/pal`, the
  levels in the form the port draws (the worlds behind the menus too). Half
  a minute, once. Building runs it after the build the first time, and the
  game page offers it by itself when the game is built but they are missing.
- `play`: the port's executable with `--data {data} --window --levels
{root}/assets/port/rac1/pal`, as OpenGOAL's launcher starts `gk`. It `needs`
  the levels: an action's `needs` lists files an earlier step makes, and it
  stays blocked, naming that step, until they exist.

A `play` action also gets the environment Settings asks for
(`core/src/actions.rs`, `Action::plan`): `OPENRAC_DEVELOPER=1` with developer
tools on, `OPENRAC_LOG=verbose` with Detailed game logs, and, on Windows and
Linux with Renderer set to Vulkan (Advanced, `Config.renderer`),
`OPENRAC_GL_API=vulkan`: the port's experimental Vulkan renderer
(`port/renderer/vulkan`) in place of OpenGL 4.1. Where Vulkan 1.3 cannot
start, or the build has no Vulkan renderer, the game logs why and draws with
OpenGL (`port/platform/window.cpp`). It applies from the game's next start.
macOS builds have no Vulkan renderer, so the row is not shown there; Android
keeps its own choice under Graphics (`src-tauri/src/android.rs`).

Each version's levels have a folder of their own under one root, since the
versions of a game share its data folder: `assets/port/<game>/<version>` in a
checkout (`assets/port/rac1/pal`, `assets/port/rac1/ntsc`) and
`{data}/port/<version>` in an installed launcher (tools/bundle_standalone.py),
which is also where a game looks without `--levels`. Every action names its
folder. rac1/ntsc's `play` stays planned until the NTSC-U port plays; its
`todo` holds the command line to switch on.

The port needs each level's program to place the functions levels share; it
reads it from the disc image the set-up kept (`iso_data/<game>/disc.iso`)
when `decompiler_out` (from `openrac-extractor --decompile`) is not there, so
the set-up has no step for it.

To do here: a setting for the copy of the decompilation to build from, which
today is chosen once with `tools/build_port.py --source DIR`.

### 4. Each game's set-up, build and checks

The per-version actions carry their own `todo`s; in short:

- **rac1/pal**: `setup-asm` and `build` run `tools/setup_asm.sh` and
  `tools/build_sn.sh` directly (since 2026-10-09; they ran in the project's
  Docker image before). Check this against the project's own
  [CONTAINERS.md](../../games/rac1/docs/CONTAINERS.md), which says every
  build runs in the container on Linux and macOS, before marking them
  connected. Confirm the artifact and how a mismatch shows (exit code). Mind [BUILD_FIDELITY.md](../../games/rac1/docs/BUILD_FIDELITY.md):
  the launcher runs the build; it never changes how it builds.
- **rac1/ntsc**: `setup.sh` downloads and builds its toolchains (long; Linux,
  WSL, or Docker on macOS); `make elf`, `verify-baseline.sh`, `make iso`.
  Find `make elf`'s output for an artifact.
- **rac2**: needs settings the launcher lacks (package 4); the run id that
  `setup.py` creates feeds `build.py`.
- **rac3**: needs Wrench, wibo and a venv (package 4); then
  `tools/setup_asm.py`, `tools/build.py`, `tools/pr_check.py`.
- **rac4**: `setup_asm.sh`, the build (`tools/build.sh`, run directly since
  2026-10-09, as rac1/pal's), `audit_matches.py` with the project's venv.

Each game's own docs are the authority on its commands; the action's `docs`
field points at them. If a game's tool needs a change to be driven (a flag, a
machine-readable result), make it in that game's directory under its rules,
in a commit of its own, and verify the game as its docs say.

### 5. More settings

Some games need tools and folders the launcher does not know yet: rac2's
working folder for its builds (its "runtime" folder) (outside the tree; `build/rac2-runtime` works) and Wrench's
`wrenchbuild`; rac3's wibo and Wrench; per-game virtual environments. A design
that keeps `actions.json` the single place: a `tools` map in `Config`
(`{"wrench": "/path/to/wrenchbuild", "wibo": ...}`) with placeholders
`{tool:wrench}`, declared in a top-level `"tools"` list in `actions.json` (name,
label, hint, how to detect), so Settings lists them without UI changes. Add
the placeholder form to `PLACEHOLDERS` handling and its tests.

A "Download helpers" action (Wrench, wibo) must pin versions and checksums,
like rac1/ntsc's `setup.sh` does.

### 6. Windows

rac1/pal and rac4's scripts are bash; rac1/ntsc and rac2 use WSL for their
compilers. Decide per game between Git Bash (`bash` from Git for Windows) and
WSL (`wsl.exe --cd <path> -- <command>`, which needs paths converted to
`/mnt/c/...`: a `{wsl:dir}`-style placeholder). Test the job cancel on
Windows (`taskkill /T`) against a WSL process.

### 7. Updates

- **The launcher**: Tauri's updater plugin, as the T3SDK launcher does
  (signed `latest.json` on GitHub releases), plus a release workflow that
  builds the installers. Needs a signing key held by the maintainers.
- **The checkout**: the planned `update` repository action: `git pull` when the
  tree is clean, refusing otherwise. Players who do not use git need a
  different answer (open decision below).

### 8. The level editor and modding

rac1/pal and rac1/ntsc have the whole path written down, with the same ids,
in the order a user takes it. The Mods page shows it for the version the
game page plays (`playerVersion`: the disc the player set up, or the one a
developer with both discs chose on the game's page), so both discs go the
same way and no region is named; developer mode names the version.

| Action            | State (PAL / NTSC-U)   | What it does                                                                                           |
| ----------------- | ---------------------- | ------------------------------------------------------------------------------------------------------ |
| `editor-extract`  | unverified             | the disc's 19 levels into Godot projects, `assets/levels/rac1/pal` (NTSC-U: `assets/levels/rac1/ntsc`) |
| `editor-import`   | unverified             | Godot reads one level's meshes and textures once, headless, so the first open is quick                 |
| `editor-open`     | unverified             | the Godot editor on one level's project                                                                |
| `editor-pack`     | unverified             | edited scenes back into that disc's level data, `assets/packed/rac1/pal` (`assets/packed/rac1/ntsc`)   |
| `editor-preview`  | connected / unverified | Godot's player on an extracted level (Godot showing the editor's scenes, not the game)                 |
| `editor-tests`    | unverified (PAL)       | the unit tests of `editor/`                                                                            |
| `assets-export`   | unverified             | a level's models and textures from `assets/port/rac1/<version>` into `{data}/exports`                  |
| `assets-import`   | unverified             | an edited export back as an overrides pack in `{data}/overrides`                                       |
| `overrides-add`   | unverified             | a pack (.zip or folder) into `{data}/overrides`                                                        |
| `overrides-stack` | unverified             | the packed levels and the packs that are on into `{data}/mods/<version>`, for that disc's levels       |
| `play-edited`     | unverified / planned   | the game with `--mods {data}/mods/<version>`; NTSC-U's waits for its Play                              |

Exports and packs are the game's, shared by its versions: a pack records
what each file replaces, and `overrides-stack` (`editor/overrides.py stack
--levels`) puts a pack made from the other disc's levels where it matches
there ([editor/MODDING.md](../../editor/MODDING.md#packs-and-discs)). The
stacked folder is each version's own, `{data}/mods/<version>`, because what
it holds is made for one disc's levels (texture names, packed sectors); the
level packer's output is per version too, since it replaces that disc's
sectors. The installed launcher's bundle (tools/bundle_standalone.py) carries
none of these actions yet, and its header hides Mods.

The unverified ones ran from a shell on macOS with real discs and Godot
4.7.2 (extract into a new folder, the refusal of an existing one, the
headless import, packing on each disc, exporting, importing and stacking a
PAL pack onto both discs' levels, the tests); none has been run from the
desktop app yet. The extractor needs Python 3.10 or newer, which is why
Settings refuses an older one. It never overwrites a project folder, so once
one exists the page offers Open, not Extract. The other games carry a planned
`editor-extract` that says what the editor lacks for them
([editor/README.md](../../editor/README.md)).

### 9. Release and CI

CI runs the launcher's checks (the `launcher` job in
`.github/workflows/checks.yml`). Building installers in CI, and where they are
published, waits on package 7.

## Open decisions

These are not the launcher's to settle alone ([AGENTS.md](../../AGENTS.md), rule 11):

- **How players get OpenRAC.** Today the launcher needs a git checkout. A
  player-facing release could bundle `tools/` and the game folders instead,
  which changes what "update" means and where builds write.
- **Network.** Since 2026-10-09 the launcher fetches
  `https://openrac.dev/progress.json` at start-up and rewrites the
  checkout's `progress/summary.json` with it, and shows Discord Rich Presence
  unless switched off. `progress/summary.json` is a generated file that
  `tools/openrac.py progress` writes (AGENTS.md, rule 9): whether the
  launcher should keep its copy elsewhere is for the maintainers.
- **Which games and versions to put first.** The launcher shows every version
  in `games/`; the five games are rac1 (PAL and NTSC-U), rac2, rac3, rac4 and
  rac5 (Size Matters' PlayStation 2 port).

## Keeping things in step

| When this changes                       | Change this too                                                                                                      |
| --------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| A game.json gains a version             | an entry in `actions.json` (the tests insist) and, for a new game, a theme in `src/lib/themes.ts` (the tests insist) |
| A game's build commands or output paths | its actions' `args`, `artifact` and `docs`                                                                           |
| A Rust command or type                  | `src/lib/api.ts` and `src/lib/mock.ts`                                                                               |
| The website's colours or cards          | `src/app.css`, `src/lib/themes.ts` ([STYLE.md](STYLE.md))                                                            |
