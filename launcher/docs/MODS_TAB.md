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
| A gameplay mod's own page (its settings)                 | `components/ModPage.svelte`, `ModSetting.svelte`        | its row opened (`lib/modNav.svelte.ts`), in place of My mods' lists       |
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

| State                                   | What shows                                                                                                                                                    | Words                                                                                                                                                                                                                                               |
| --------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Loading** (reading the repository)    | spinner on Find                                                                                                                                               |                                                                                                                                                                                                                                                     |
| **Error**                               | a line in `--err` under the field                                                                                                                             | the core's reason; a 404 is `NOT_FOUND` in core/src/remote.rs ("This repository is private or doesn't exist. OpenRAC can only install from public repositories."); others: not a repository's address, no branch, no mod in that folder, no release |
| **Ready**                               | a box on the darker ground: where it is from, a close button, the repository's note, rows                                                                     | `T.fromPublisher`; `T.PREVIEW.nativeWarning` with an info mark when a native mod can be installed (informs, does not alarm)                                                                                                                         |
| A mod in the preview: kind              | a dot and words, its tooltip what it means                                                                                                                    | `T.KIND_LABEL.*`, `T.KIND_HINT.*`                                                                                                                                                                                                                   |
| A mod in the preview: already installed | after its kind                                                                                                                                                | `T.installedAlready(version)`                                                                                                                                                                                                                       |
| A mod in the preview: fits              | its button: Install, Reinstall (same version), Update to X (older installed)                                                                                  | `T.PREVIEW.install`, `T.PREVIEW.reinstall`, `T.updateTo`                                                                                                                                                                                            |
| A mod in the preview: does not fit      | its name dimmed, `T.COMPATIBLE_LABEL.no` in place of the button, the reason under it                                                                          |                                                                                                                                                                                                                                                     |
| **Installing**                          | spinner on that mod's button, all install buttons off; then the preview closes, or stays for a repository of several mods with that one's button at Reinstall | then `T.installedFrom.on` / `.off` / `.replaced`, or `T.withName(name, error)`                                                                                                                                                                      |

## Gameplay mods (PluginsPanel.svelte)

An overview: a row per mod, and each mod's settings on a page of its own
(ModPage.svelte), one click away. The main path reads left to right on a row:
switch it on, open it, play (Play with mods, in the page's head).

### The panel's head

| Element           | Words                                  | Notes                                                                               |
| ----------------- | -------------------------------------- | ----------------------------------------------------------------------------------- |
| Title             | `T.PANEL.title`                        |                                                                                     |
| Check for updates | `T.PANEL.checkUpdates`                 | only when a mod came from a link; also runs by itself once a day, silently          |
| Load order        | `T.PANEL.orderTitle`                   | a small line under the list, with two or more mods                                  |
| Purist mode       | `T.PURIST.title`, `T.PURIST.panelLine` | the panel's last row; the launcher-wide setting (Settings: `T.PURIST.settingsLine`) |

### States of the panel

| State                        | What shows                                                | Words                                   |
| ---------------------------- | --------------------------------------------------------- | --------------------------------------- |
| **Empty** (no mod installed) | a line; Purist mode under it                              | `T.PANEL.empty`                         |
| **List cannot be read**      | the error, in `--err`, its folders hidden (Reason.svelte) | the error text from core (plugins.rs)   |
| **Purist mode on**           | a notice above the list with Turn off; rows dimmed        | `T.PURIST.onNotice`, `T.PURIST.turnOff` |
| **Checking for updates**     | spinner on Check for updates                              | answer `T.updatesFound(n, errors)`      |
| A change refused             | a line under the head, its folders hidden                 | the core's reason                       |

### An installed mod's row

| Element    | Words                                                                                | Notes                                                                                      |
| ---------- | ------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------ |
| Switch     | aria `T.useAria(name)`                                                               | off and disabled when it cannot be switched on (its title says why)                        |
| Name, line | from the manifest: the description cut to one line                                   | the whole middle of the row opens the mod's page (aria `T.settingsAria(name)`)             |
| Badge      | one, most pressing first (lib/plugins.ts `modBadge`)                                 | `T.STATE_LABEL.refused` / `.failed`, else `T.BADGE.update(version)`, else `T.KIND_LABEL.*` |
| Settings › | `T.PANEL.settings`, or `T.PANEL.details` for a mod with no settings                  | the row's affordance; hovered, it turns amber                                              |
| Menu       | aria `T.moreAria`; `T.PANEL.moveUp`, `moveDown`, `openFolder`, `remove` (in `--err`) | Remove confirms with `T.removeConfirm`                                                     |

## A mod's own page (ModPage.svelte)

Opened from its row; `lib/modNav.svelte.ts` keeps which (a history entry
`#mod=<id>`, so Back, a mouse's back button and Escape return to the list,
and the row takes the focus back). `?mod=<id>` opens one at start. The game
list, the head and Play with mods stay; the tabs, Add a mod and the packs make
way for the page.

| Part          | What shows                                                                                                                                                | Words                                                                                                     |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| Where you are | a back button, then My mods › the mod's name                                                                                                              | `T.MOD_PAGE.back`, `crumbRoot`                                                                            |
| Head          | its name, version, author, source (`T.fromSource`, `T.keptAt`), what it runs, its description in two lines                                                | `T.KIND_LABEL.*`, tooltip `T.KIND_HINT.*`                                                                 |
| Actions       | Update (when the check found one and it is not kept), its switch with On / Off, the menu: keep this version, its page, its folder, Remove                 | `T.MOD_PAGE.update`, `on`, `off`, `T.PANEL.keepVersion` / `letUpdate`, `homepage`, `openFolder`, `remove` |
| Notices       | Purist mode on; why it cannot load (`--err`); why it cannot be switched on; off, with settings kept; a shared hook point                                  | `T.PURIST.onNotice`, the core's reason, `T.cannotSwitchOn`, `T.MOD_PAGE.offNote`, `T.sharesWith`          |
| About         | folded: the whole description, what it runs and what that means, source, license, its folder with Open its folder; with developer tools, its id and hooks | `T.MOD_PAGE.about`, `aboutKind`                                                                           |
| Presets       | a segmented row, Default first unless a preset is `default`; the one the values match is lit                                                              | `T.MOD_PAGE.presets`, `T.KEYBIND.defaults`                                                                |
| Saving        | beside the presets: Changes save as you make them; Saving…; Saved. Used from the game's next start; Not saved (and why, under it)                         | `T.SAVE.*`                                                                                                |
| Reset all     | every setting back to its default, after a question                                                                                                       | `T.MOD_PAGE.resetAll`, `resetAllConfirm`                                                                  |
| Sections      | a panel per manifest `group`, in the order each first appears, the ungrouped first                                                                        | `T.GROUP_TITLE.only` (one section), `general`, `controls(name)` (a group of keybinds)                     |
| No settings   | a line                                                                                                                                                    | `T.MOD_PAGE.noSettings`                                                                                   |

### A setting's row (ModSetting.svelte)

Its name, one line of its description (the info button unfolds the rest:
`T.SETTING.more` / `less`), its control on the right, Reset when it is not at
its default (`T.SETTING.reset(name, default)`), and "Saved" by its name for a
moment after each save. Every control applies as it changes, with no Enter
(lib/autosave.ts): a slider and typed text after it settles (350 and 600 ms),
a switch, a choice, a preset or a key at once; one save at a time, in order.

| Type                            | Control                                                                                                                                                                         |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `int`, `float` with min and max | a slider on the manifest's steps (lib/settingValues.ts), a mark at the default, its value in a field beside it: typed, it is snapped and clamped; the arrows step, Shift by ten |
| `int`, `float` otherwise        | the field alone                                                                                                                                                                 |
| `bool`                          | a switch                                                                                                                                                                        |
| `choice`                        | a segmented control (up to four short choices), else a select                                                                                                                   |
| `keybind`                       | the key caps (KeyBindSetting.svelte); waiting, the row's line says `T.KEYBIND.pressHint` with Cancel; a clash says `T.alsoBound`                                                |
| `string`                        | a field the row's width                                                                                                                                                         |

The values are kept where they always were, the per-user `plugins.json`
(core/src/plugins.rs): the page saves each setting with `plugins_set_setting`
as before, so the file's format does not change.

### No folder in plain view

Players share screenshots of this tab, so no path shows outside a folded
section: a mod's folder only in its page's About, opened with Open its
folder; the developer view folded under "For developers". A message that
names a path (a refusal, an error, a job's line, a toast) says the file's or
folder's last part instead (lib/paths.ts `hidePaths`), with the message as it
came under a folded Details (Reason.svelte).

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

With developer tools on (Settings), each gameplay mod's About adds its id and
hook points (its folder, license and source are there for everyone, folded),
and a link's preview adds the commit read, each
mod's folder, release asset and tag, interface version and discs. Under My mods
and Level editor a dashed, folded "For developers" panel (`T.PAGE.developerTitle`)
holds the version, its folders, the command lines behind the buttons
(CommandBox) and buttons for the level projects, the packed levels and the
editor's documentation; Models and textures shows each step's command under it.
The player's view never shows these.
