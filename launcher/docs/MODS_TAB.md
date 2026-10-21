# The Mods tab: every state

What the Mods tab can show, and where. Its words are all in
[src/lib/modsText.ts](../src/lib/modsText.ts) (named below as `T.<key>`); how
the mods work is [docs/port/MODS.md](../../docs/port/MODS.md).

The tab is written for players: one short line where a sentence is needed, the
longer "why" in a tooltip (`T.KIND_HINT`, the move buttons' titles), and what
only a contributor reads under developer tools (the last section).
`modsText.test.ts` keeps the page's lines under 120 characters.

## Where things are

| Part                                                     | Component                                               | Shown when                                                                |
| -------------------------------------------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------- |
| The tab itself                                           | `components/Header.svelte`                              | every desktop launcher, installed or on a checkout; **not on Android**    |
| The game list on the left                                | `pages/Mods.svelte` (`modState`) on `GamesFrame.svelte` | always                                                                    |
| The head: the game's name, a lead line, Play with mods   | `pages/Mods.svelte`                                     | a game is picked; Play with mods when the level editor can open the game  |
| The tabs: My mods, Level editor, Models and textures     | `pages/Mods.svelte`                                     | the level editor can open the game                                        |
| My mods → Add a mod (a link, a file, a folder, a drop)   | `components/AddMod.svelte`                              | My mods; the link field when the game has a port that plays               |
| My mods → Gameplay mods (native and script mods, Purist) | `components/PluginsPanel.svelte`                        | the game has a port that plays (its `play` action is not `planned`)       |
| My mods → Textures, models and levels (data packs)       | `components/PacksPanel.svelte`                          | the level editor can open the game                                        |
| My mods → "Want to make your own?" (to the Level editor) | `pages/Mods.svelte`                                     | under the lists                                                           |
| Level editor: Open a level in Godot, Pack your changes   | `pages/Mods.svelte` (tab `editor`)                      | the level editor can open the game                                        |
| Models and textures: Export, Import your edits           | `components/AssetsPanel.svelte` (tab `assets`)          | the level editor can open the game                                        |
| For developers                                           | `pages/Mods.svelte`                                     | developer tools on, under My mods and Level editor                        |
| The crash notice's mods line                             | `components/CrashNotice.svelte`                         | on a game's page, after a crash, when the last start loaded gameplay mods |
| Purist mode in Settings                                  | `pages/Settings.svelte` (General)                       | desktop only                                                              |

## The game list (left)

| State                                                        | Words                      | Tone |
| ------------------------------------------------------------ | -------------------------- | ---- |
| The editor cannot open it, but the game plays                | `T.GAME_LIST.gameplayMods` | ok   |
| The editor cannot open it and the game does not play         | `T.GAME_LIST.notModdable`  | info |
| Early access (the editor reads it; no packing back, no play) | `T.GAME_LIST.early`        | info |
| A level or asset job runs                                    | `T.GAME_LIST.working`      | warn |
| No disc set up                                               | `T.GAME_LIST.setUp`        | warn |
| Ready                                                        | `T.GAME_LIST.ready`        | ok   |

## The page, per kind of game

| Kind of game                                                           | What shows                                                                                                                                                |
| ---------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Playable, editor available (Ratchet & Clank in a checkout)             | the head with Play with mods, the three tabs; My mods has Add a mod (link, file, folder), Gameplay mods and the packs                                     |
| Playable, no editor (an installed launcher without the editor's tools) | the head without Play with mods (the game page plays with the gameplay mods), Add a mod and Gameplay mods; no tabs                                        |
| Early access (Going Commando, Up Your Arsenal, Deadlocked)             | the head (Play with mods off, `T.PAGE.playEarly`), a dashed line `T.PAGE.earlyTitle` / `earlyLine`, the tabs; My mods adds by file only, no Gameplay mods |
| Neither                                                                | `T.PAGE.noneTitle`, `noneLine` and a link to openrac.dev                                                                                                  |

Play with mods is the page's one orange button, in the head on every tab. When
it cannot start, the line under it says why: `T.PAGE.playEarly`,
`playMissing` (no play action on this computer) or `playBuild`.

## Add a mod (AddMod.svelte)

One panel for every kind: the player never says which kind a mod is.
`lib/addMods.ts` sends a native or script mod to the gameplay mods and a
texture and model pack to the packs.

| Element      | Words                                                                                                 | Notes                                                                                                               |
| ------------ | ----------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Title        | `T.ADD.title`                                                                                         | Browse mods (`T.ADD.browse`, opens `T.ADD.modsUrl`) beside it for Ratchet & Clank                                   |
| Link field   | placeholder `T.ADD.linkPlaceholder`, label `T.ADD.linkLabel`, button `T.ADD.go`                       | accepts https, http, no scheme, `.git`, `/tree/<branch>/<folder>`, `/releases/tag/<v>`, `git@github.com:o/r`, `o/r` |
| File, Folder | `T.ADD.file`, `T.ADD.folder`; pickers `T.ADD.pickZip`, `T.ADD.pickFolder`                             | a format-1 pack chosen here goes to the packs                                                                       |
| Drop hint    | `T.ADD.drop`, while a file is held over the window `T.ADD.dropHere` (the panel's border dashes amber) | in the app only                                                                                                     |
| Result       | a line under the bar                                                                                  | `T.added.*`, `T.installedFrom.*`, or the error in `--err`                                                           |

### A link's preview

| State                                   | What shows                                                                                | Words                                                                                                                                                                                                                                               |
| --------------------------------------- | ----------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Loading** (reading the repository)    | spinner on Find                                                                           |                                                                                                                                                                                                                                                     |
| **Error**                               | a line in `--err` under the field                                                         | the core's reason; a 404 is `NOT_FOUND` in core/src/remote.rs ("This repository is private or doesn't exist. OpenRAC can only install from public repositories."); others: not a repository's address, no branch, no mod in that folder, no release |
| **Ready**                               | a box on the darker ground: where it is from, a close button, the repository's note, rows | `T.fromPublisher`; `T.PREVIEW.nativeWarning` with an info mark when a native mod can be installed (informs, does not alarm)                                                                                                                         |
| A mod in the preview: kind              | a dot and words, its tooltip what it means                                                | `T.KIND_LABEL.*`, `T.KIND_HINT.*`                                                                                                                                                                                                                   |
| A mod in the preview: already installed | after its kind                                                                            | `T.installedAlready(version)`                                                                                                                                                                                                                       |
| A mod in the preview: fits              | its button: Install, Reinstall (same version), Update to X (older installed)              | `T.PREVIEW.install`, `T.PREVIEW.reinstall`, `T.updateTo`                                                                                                                                                                                            |
| A mod in the preview: does not fit      | its name dimmed, `T.COMPATIBLE_LABEL.no` in place of the button, the reason under it      |                                                                                                                                                                                                                                                     |
| **Installing**                          | spinner on that mod's button, all install buttons off                                     | then `T.installedFrom.on` / `.off` / `.replaced`, or `T.withName(name, error)`                                                                                                                                                                      |

## Gameplay mods (PluginsPanel.svelte)

### The panel's head

| Element           | Words                                                                   | Notes                                                                               |
| ----------------- | ----------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| Title, line       | `T.PANEL.title`, `T.PANEL.sub` (+ `T.PANEL.orderHint` with two or more) | the hint's tooltip is `T.PANEL.moveDownTitle`                                       |
| Check for updates | `T.PANEL.checkUpdates`                                                  | only when a mod came from a link; also runs by itself once a day, silently          |
| Purist mode       | `T.PURIST.title`, `T.PURIST.panelLine`                                  | the panel's last row; the launcher-wide setting (Settings: `T.PURIST.settingsLine`) |

### States of the panel

| State                        | What shows                                                           | Words                                   |
| ---------------------------- | -------------------------------------------------------------------- | --------------------------------------- |
| **Empty** (no mod installed) | a line; Purist mode under it                                         | `T.PANEL.empty`                         |
| **List cannot be read**      | the error, in `--err`                                                | the error text from core (plugins.rs)   |
| **Purist mode on**           | a notice above the list with Turn off; rows dimmed, no state on them | `T.PURIST.onNotice`, `T.PURIST.turnOff` |
| **Checking for updates**     | spinner on Check for updates                                         | answer `T.updatesFound(n, errors)`      |
| **Updating a mod**           | spinner on its Update button                                         | `T.updated`, or the error               |
| A change refused             | a line under the head                                                | the core's reason                       |

**Why a mod from a link cannot be installed** (core/src/remote.rs and
plugins.rs, `why`): not for this game's disc; a pack for another game; needs a
newer mod interface; not built for this computer; native mods do not run on this
system (Android); no release yet (native mods install built); no release has it
built for this computer; its release has no SHA256SUMS. At install, refused:
SHA256SUMS does not list the file, its SHA-256 differs, the archive's manifest
is not the one shown.

### An installed mod's row

| Element                            | Words                                                                                                                                                                                                       | Notes                                                                            |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| Switch                             | aria `T.useAria(name)`                                                                                                                                                                                      | off and disabled when it cannot be switched on                                   |
| Name, version, author, description | from the manifest                                                                                                                                                                                           |                                                                                  |
| Kind                               | `T.KIND_LABEL.native` / `.script` / `.pack`, tooltip `T.KIND_HINT.*` (lib/plugins.ts `modHint`)                                                                                                             | a dot and words, under the description                                           |
| State                              | `T.STATE_LABEL.*` via lib/plugins.ts `pluginState`                                                                                                                                                          | only `refused` and `failed`: the switch already says on or off                   |
| Move up / down                     | aria `T.moveUpAria`, `T.moveDownAria`; titles `T.PANEL.moveUpTitle`, `T.PANEL.moveDownTitle`                                                                                                                |                                                                                  |
| Menu                               | aria `T.moreAria`; items `T.PANEL.keepVersion` / `T.PANEL.letUpdate` (from a link only), `T.PANEL.homepage` (when it has one), `T.PANEL.remove` (in `--err`, confirms with `T.removeConfirm`)               |                                                                                  |
| Source (from a link)               | `T.fromSource(owner/repo)`, `T.keptAt(version)` when pinned                                                                                                                                                 | after the kind                                                                   |
| Update button                      | `T.updateTo(version)`                                                                                                                                                                                       | after the source, when the last check found a newer release and it is not pinned |
| Shared hook point                  | `T.sharesWith(names)`, in amber                                                                                                                                                                             | when another on mod takes the same hook point                                    |
| Settings                           | one row each: a switch (bool), a segmented control (2 or 3 choices), radios (more), a number field (int, float) with its range `T.rangeText`, a text field (string); a refused number says `T.settingTakes` | folded under a Settings button (`T.PANEL.settings`), only while the mod is on    |
| Developer details                  | id, folder, hooks (`T.PANEL.hooks`), license (`T.PANEL.license`), source (`T.PANEL.source`)                                                                                                                 | developer tools on                                                               |

**States of an installed mod** (`T.STATE_LABEL`, decided in lib/plugins.ts):

| Key       | Words              | When                                                          | The line under it                                     |
| --------- | ------------------ | ------------------------------------------------------------- | ----------------------------------------------------- |
| `on`      | On                 | on, and it loaded (or will); not shown, the switch says it    |                                                       |
| `off`     | Off                | switched off; not shown, the switch says it                   | `T.cannotSwitchOn(why)` when it cannot be switched on |
| `purist`  | Not in purist mode | on, and Purist mode is on; not shown, the notice says it      |                                                       |
| `refused` | Cannot load        | on, and a check refuses it now                                | the reason, in `--err`                                |
| `failed`  | Not loaded         | on, and the game's last start did not load it (`status.json`) | "Not loaded at the last start: …"                     |

**Reasons it cannot load or be switched on** (core/src/plugins.rs): it is not for
this game; not for this disc; needs a newer mod interface; not built for this
computer; native mods do not run on this system; cannot be read; its manifest
names another id; needs a mod that is not installed, cannot be read, is too old,
is off, or is below it; conflicts with another mod. From the game's last start
(status.json): its library could not be opened, its load refused, a hook point
it asked for is missing, a script error or a script over its time budget.

## Textures, models and levels (PacksPanel.svelte)

Title `T.PACKS.title`, line `T.PACKS.sub` (+ `T.PACKS.orderHint`); each pack a
switch, its name, author and what it replaces, up, down and remove
(`T.PACKS.moveUpTitle`, `moveDownTitle`, `remove`). Empty: `T.PACKS.empty`.

## Level editor (tab `editor`, Mods.svelte)

A lead line (`T.EDITOR.lead`, early access `leadEarly`) and two steps.

| Step                  | State              | Line                                            | Controls                                                                |
| --------------------- | ------------------ | ----------------------------------------------- | ----------------------------------------------------------------------- |
| Open a level in Godot | no disc            | `T.EDITOR.noDisc`                               | Set up the disc                                                         |
|                       | no Godot           | `T.EDITOR.noGodot`                              | Get Godot (the download page), Choose Godot… (checked as Settings does) |
|                       | reading the levels | the job's last line                             | spinner                                                                 |
|                       | ready              | `T.EDITOR.ready` / `readyEarly`                 | the level, Open in Godot (`api.openLevelInGodot`)                       |
|                       | not read yet       | `T.EDITOR.firstTime` / `firstTimeEarly`         | the level, Prepare and open (extract, then open)                        |
| Pack your changes     | early access       | `T.EDITOR.packEarly`                            | Pack, off                                                               |
|                       | packing            | the job's last line                             | spinner                                                                 |
|                       | packed             | `T.EDITOR.packedSome` / `packedNone`, in `--ok` | Open folder (`api.openOutput`, the job's output) and Pack               |
|                       | failed             | `T.EDITOR.packStopped`, in `--err`              | Pack                                                                    |

## Models and textures (AssetsPanel.svelte)

A lead line with How it works (`T.ASSETS.lead`, `howItWorks`), then Export (a
level, what, Export; Read the levels first for a game in early access; Open
folder for the last export, `api.openOutput`) and Import your edits
(`T.ASSETS.importLine`; the result `imported`, `importedRefused`, or the error).

## Android

The Mods tab is not shown on Android (Header.svelte), and Settings hides
Purist mode there. The core treats script mods as compatible on Android, but
the tab would also need a way to download from a link (the launcher fetches
with `curl`) and a `.zip` picker there.

## Developer view

With developer tools on (Settings), each gameplay mod adds its id, folder, hook
points, license and source, and a link's preview adds the commit read, each
mod's folder, release asset and tag, interface version and discs. Under My mods
and Level editor a dashed "For developers" panel (`T.PAGE.developerTitle`)
holds the version, its folders, the command lines behind the buttons
(CommandBox) and buttons for the level projects, the packed levels and the
editor's documentation; Models and textures shows each step's command under it.
The player's view never shows these.
