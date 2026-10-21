# The Mods tab: every state, for its redesign

An inventory of what the Mods tab can show, for whoever redesigns it. It
describes the tab as it is; it decides nothing. The gameplay mods' words are
all in [src/lib/modsText.ts](../src/lib/modsText.ts) (named below as
`T.<key>`); the level editor's and the packs' words are still in their
components. How the mods work is [docs/port/MODS.md](../../docs/port/MODS.md).

## Where things are

| Part                                                                 | Component                                               | Shown when                                                                |
| -------------------------------------------------------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------- |
| The tab itself                                                       | `components/Header.svelte`                              | every desktop launcher, installed or on a checkout; **not on Android**    |
| The game list on the left                                            | `pages/Mods.svelte` (`modState`) on `GamesFrame.svelte` | always                                                                    |
| The page's title and lead line                                       | `pages/Mods.svelte`                                     | a game is picked                                                          |
| Gameplay mods (native and script mods, Purist mode, Add from a link) | `components/PluginsPanel.svelte`                        | the game has a port that plays (its `play` action is not `planned`)       |
| Your mods (data packs: textures, models, levels)                     | `components/PacksPanel.svelte`                          | the level editor can open the game                                        |
| Play with your mods                                                  | `pages/Mods.svelte`                                     | the level editor can open the game                                        |
| Make your own: Edit the levels, Pack your edits                      | `pages/Mods.svelte` (tab `levels`)                      | the level editor can open the game                                        |
| Make your own: Models and textures                                   | `components/AssetsPanel.svelte` (tab `assets`)          | the level editor can open the game                                        |
| The crash notice's mods line                                         | `components/CrashNotice.svelte`                         | on a game's page, after a crash, when the last start loaded gameplay mods |
| Purist mode in Settings                                              | `pages/Settings.svelte` (General)                       | desktop only                                                              |

## The game list (left)

| State                                                        | Words                        | Tone |
| ------------------------------------------------------------ | ---------------------------- | ---- |
| The editor cannot open it, but the game plays                | `T.GAME_LIST.gameplayMods`   | ok   |
| The editor cannot open it and the game does not play         | `T.GAME_LIST.notModdable`    | info |
| Early access (the editor reads it; no packing back, no play) | "Early access" (Mods.svelte) | info |
| Others: the editor's own states (extracting, ready, ...)     | Mods.svelte, `modState`      |      |

## The page, per kind of game

| Kind of game                                                           | What shows                                                                                                        |
| ---------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Playable, editor available (Ratchet & Clank in a checkout)             | Gameplay mods, Your mods, Play with your mods, Make your own                                                      |
| Playable, no editor (an installed launcher without the editor's tools) | Gameplay mods only                                                                                                |
| Early access (Going Commando, Up Your Arsenal, Deadlocked)             | the Early access panel, Your mods, Play with your mods (says it cannot play yet), Make your own; no Gameplay mods |
| Neither                                                                | "Mods are not possible for this game yet" and a link to openrac.dev                                               |

## Gameplay mods (PluginsPanel.svelte)

### The panel's head and controls

| Element                 | Words                                                                                   | Notes                                                                                                               |
| ----------------------- | --------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Title                   | `T.PANEL.title`                                                                         |                                                                                                                     |
| Add a .zip              | `T.PANEL.addZip`, picker title `T.PANEL.pickZip`                                        | a format-1 pack chosen here goes to Your mods (lib/addMods.ts)                                                      |
| Add a folder            | `T.PANEL.addFolder`, picker title `T.PANEL.pickFolder`                                  |                                                                                                                     |
| Check for updates       | `T.PANEL.checkUpdates`                                                                  | only when a mod came from a link; also runs by itself once a day, silently                                          |
| Link field              | placeholder `T.PANEL.linkPlaceholder`, label `T.PANEL.linkLabel`, button `T.PANEL.look` | accepts https, http, no scheme, `.git`, `/tree/<branch>/<folder>`, `/releases/tag/<v>`, `git@github.com:o/r`, `o/r` |
| Purist mode switch      | `T.PURIST.title`, `T.PURIST.panelLine`                                                  | the launcher-wide setting; Settings has the same switch with `T.PURIST.settingsLine`                                |
| Footnote under the list | `T.PANEL.note`                                                                          | when the list is not empty                                                                                          |

### States of the panel

| State                        | What shows                                                   | Words                                                                                                                               |
| ---------------------------- | ------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------- |
| **Empty** (no mod installed) | a line with a link to rac1-mods                              | `T.PANEL.emptyBefore`, `T.PANEL.modsUrlLabel`, `T.PANEL.emptyAfter`                                                                 |
| **List cannot be read**      | the error, in `--err`                                        | the error text from core (plugins.rs)                                                                                               |
| **Adding** (.zip or folder)  | spinner on Add a .zip, both add buttons off                  |                                                                                                                                     |
| **Added**                    | a line under the head (`said`)                               | `T.added.on`, `T.added.off`, `T.added.replaced`, `T.added.pack`; failures `T.withName`, `T.added.packHereNot`, `T.added.packFailed` |
| **Purist mode on**           | every on mod's state reads "Not in purist mode"; rows dimmed | `T.STATE_LABEL.purist`                                                                                                              |
| **Checking for updates**     | spinner on Check for updates                                 | answer `T.updatesFound(n, errors)`                                                                                                  |
| **Updating a mod**           | spinner on its Update button                                 | `T.updated`, or the error                                                                                                           |

### Add from a link

| State                                   | What shows                                                                                                              | Words                                                                                                                                                                                                                                               |
| --------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Loading** (reading the repository)    | spinner on Look                                                                                                         |                                                                                                                                                                                                                                                     |
| **Error**                               | a line in `--err` under the field                                                                                       | the core's reason; a 404 is `NOT_FOUND` in core/src/remote.rs ("This repository is private or doesn't exist. OpenRAC can only install from public repositories."); others: not a repository's address, no branch, no mod in that folder, no release |
| **Ready**: the preview                  | the repository and its owner, a note about the whole repository (e.g. no release yet), warnings, one row per mod, Close | `T.fromPublisher`, `T.PREVIEW.nativeWarning` (when a native mod is listed), `T.PREVIEW.scriptNote` (when a script mod is listed), `T.PANEL.close`                                                                                                   |
| A mod in the preview: kind              | a dot and words                                                                                                         | `T.KIND_LABEL.native` / `.script` / `.pack`                                                                                                                                                                                                         |
| A mod in the preview: already installed | after its kind                                                                                                          | `T.installedAlready(version)`                                                                                                                                                                                                                       |
| A mod in the preview: fits or not       | a dot and words; when not, the reason under it                                                                          | `T.COMPATIBLE_LABEL.yes` / `.no`; reasons below                                                                                                                                                                                                     |
| A mod in the preview: its button        | Install, Reinstall (same version), Update to X (older installed)                                                        | `T.PREVIEW.install`, `T.PREVIEW.reinstall`, `T.updateTo`                                                                                                                                                                                            |
| **Installing**                          | spinner on that mod's button, all install buttons off                                                                   | then `T.installedFrom.on` / `.off` / `.replaced`, or `T.withName(name, error)`                                                                                                                                                                      |

**Why a mod from a link cannot be installed** (core/src/remote.rs and
plugins.rs, `why`): not for this game's disc; a pack for another game; needs a
newer mod interface; not built for this computer; native mods do not run on this
system (Android); no release yet (native mods install built); no release has it
built for this computer; its release has no SHA256SUMS. At install, refused:
SHA256SUMS does not list the file, its SHA-256 differs, the archive's manifest
is not the one shown.

### An installed mod's row

| Element                            | Words                                                                                                                                                                                                       | Notes                                                          |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| Switch                             | aria `T.useAria(name)`                                                                                                                                                                                      | off and disabled when it cannot be switched on                 |
| Name, version, author, description | from the manifest                                                                                                                                                                                           |                                                                |
| Kind                               | `T.KIND_LABEL.native` / `.script` / `.pack`                                                                                                                                                                 | a dot and words                                                |
| State                              | `T.STATE_LABEL.*` via lib/plugins.ts `pluginState`                                                                                                                                                          | below                                                          |
| Move up / down                     | aria `T.moveUpAria`, `T.moveDownAria`; titles `T.PANEL.moveUpTitle`, `T.PANEL.moveDownTitle`                                                                                                                |                                                                |
| Menu                               | aria `T.moreAria`; items `T.PANEL.keepVersion` / `T.PANEL.letUpdate` (from a link only), `T.PANEL.homepage` (when it has one), `T.PANEL.remove` (in `--err`, confirms with `T.removeConfirm`)               |                                                                |
| Source line (from a link)          | `T.fromSource(owner/repo)`, `T.keptAt(version)` when pinned                                                                                                                                                 |                                                                |
| Update button                      | `T.updateTo(version)`                                                                                                                                                                                       | when the last check found a newer release and it is not pinned |
| Shared hook point                  | `T.sharesWith(names)`, in amber                                                                                                                                                                             | when another on mod takes the same hook point                  |
| Settings                           | one row each: a switch (bool), a segmented control (2 or 3 choices), radios (more), a number field (int, float) with its range `T.rangeText`, a text field (string); a refused number says `T.settingTakes` | only while the mod is on                                       |
| Developer details                  | id, folder, hooks (`T.PANEL.hooks`), license (`T.PANEL.license`), source (`T.PANEL.source`)                                                                                                                 | developer tools on                                             |

**States of an installed mod** (`T.STATE_LABEL`, decided in lib/plugins.ts):

| Key       | Words              | When                                                          | The line under it                                     |
| --------- | ------------------ | ------------------------------------------------------------- | ----------------------------------------------------- |
| `on`      | On                 | on, and it loaded (or will)                                   |                                                       |
| `off`     | Off                | switched off                                                  | `T.cannotSwitchOn(why)` when it cannot be switched on |
| `purist`  | Not in purist mode | on, and Purist mode is on                                     |                                                       |
| `refused` | Cannot load        | on, and a check refuses it now                                | the reason, in `--err`                                |
| `failed`  | Not loaded         | on, and the game's last start did not load it (`status.json`) | "Not loaded at the last start: …"                     |

**Reasons it cannot load or be switched on** (core/src/plugins.rs): it is not for
this game; not for this disc; needs a newer mod interface; not built for this
computer; native mods do not run on this system; cannot be read; its manifest
names another id; needs a mod that is not installed, cannot be read, is too old,
is off, or is below it; conflicts with another mod. From the game's last start
(status.json): its library could not be opened, its load refused, a hook point
it asked for is missing, a script error or a script over its time budget.

## Android

The Mods tab is not shown on Android (Header.svelte), and Settings hides
Purist mode there. The core treats script mods as compatible on Android, but
the tab would also need a way to download from a link (the launcher fetches
with `curl`) and a `.zip` picker there.

## Developer view

With developer tools on (Settings), the panel adds each mod's id, folder,
hook points, license and source, and the preview adds the commit read, each
mod's folder, release asset and tag, interface version and discs. The rest of
the page adds the command lines behind its buttons (CommandBox) and folder
paths.
