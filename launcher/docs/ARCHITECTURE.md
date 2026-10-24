# How the launcher is built

```
 ┌─────────────── UI (src/, Svelte 5, in the webview) ───────────────┐
 │ pages ─► lib/app.svelte.ts (state, queue) ─► lib/api.ts ──┐       │
 │                                    browser preview: lib/mock.ts   │
 └───────────────────────────────────────────────────────────┼───────┘
                          invoke("library"), invoke("run_action"), ...
                          events: job-output, job-exit        │
 ┌──────────── src-tauri/src/ (Tauri commands) ───────────────▼───────┐
 │ a thin layer: settings file location, events, opener               │
 │ lib.rs: the commands; android.rs: the Android app's own            │
 └──────────────────────────────┬──────────────────────────────────────┘
 ┌──────────── core/ (openrac-launcher-core, no window) ─▼─────────────┐
 │ catalog ◄─ games/*/game.json, progress/summary.json                 │
 │ actions ◄─ launcher/actions.json        status ◄─ baserom/, inputs  │
 │ install ◄─ <install folder>/active/<game>/data (the player's discs) │
 │ library: all of these for the UI; plan(scope, id) -> command line   │
 │ jobs: child processes, output lines, cancel     detect, config      │
 │ discord: Rich Presence over Discord's local socket                  │
 │ desktop: Linux menu entries, icons    progress_sync: bytematch.dev  │
 └──────────────────────────────┬──────────────────────────────────────┘
                                ▼
       the checkout's own tools: tools/extractor.py, tools/openrac.py,
       each game's build, editor/extract.py, Godot (never through a shell)
```

## The rules it keeps

1. **The checkout is the source of truth.** Games, versions, discs, inputs and
   progress come from `games/*/game.json` and `progress/summary.json`, the
   files `tools/openrac.py` reads and writes. The launcher hard-codes nothing
   about a game except its colours (`src/lib/themes.ts`). A new version in a
   game.json appears in the launcher by itself.
2. **The launcher runs the project's tools; it does not reimplement them.**
   Identifying discs is `openrac.py discs`, building is each game's own build.
   What the launcher checks by itself (`core/src/status.rs`) is cheap: names,
   sizes, links.
3. **The UI names actions; Rust builds the command line.** The webview sends
   `run_action({kind: "version", key: "rac1/pal"}, "build")`; `core` looks the
   action up in `actions.json`, fills its placeholders and runs the program
   with its arguments directly, never through a shell. A page cannot run
   anything that is not in `actions.json`.
4. **Everything but the window is in `core/`.** It builds and tests on any
   machine (`cargo test -p openrac-launcher-core`), without WebKit, a display
   or a game. `src-tauri/` only exposes it.
5. **Nothing personal or disc-derived is written outside the tools.** Disc
   data is written only by the extractor, into the install folder; the
   launcher's own files are `launcher.json` in the per-user config folder,
   the progress copy and the Linux desktop files (`core/src/desktop.rs`)
   ([README.md](../README.md#what-it-writes-and-what-it-connects-to)).
6. **No emulation.** Nothing the launcher runs interprets the console's
   code or models its hardware; Play is the native port
   ([docs/port](../../docs/port/README.md)).

## Data flow

**Start-up.** `App.svelte` calls `start()` (`lib/app.svelte.ts`): subscribe to
job events, `app_info`, `get_config`, then `library`. With no settings yet
(`setupComplete` false) the first-run screen (`pages/Settings.svelte` with
`setup`) shows; it calls `detect` and fills in what was found.

**The library.** `library` (`core/src/library.rs`) reads the catalogue, works
out each version's status, and for each action whether it can run now and,
if not, why (`blockers`). The UI shows exactly that; it never decides on its
own whether something can run. It asks again after every job, when the window
gets focus, and when settings change.

**Running an action.** `run(scope, action)` in `lib/app.svelte.ts`:

- A _detached_ action (the game, Godot) is started at once through
  `run_action`, which returns id 0. The launcher does not follow it.
- Anything else joins the queue (`lib/queue.ts`). Jobs run one at a time in
  order, so two builds never share the build container or a folder. When it
  is a job's turn, `run_action` starts it and returns its id; output arrives
  as `job-output` events and the end as `job-exit`. Events that arrive before
  `run_action` has returned are kept until the id is known.
- A job's end is acted on once, wherever its event landed: the queue keeps
  the jobs that ended (`takeEnded`) and the app takes them, so an end that
  came before the start call returned is acted on just like one that came
  after it.
- `run_action` checks the action again against the checkout as it is now; a
  job whose needs went missing in the meantime fails with the reason.

**Setting a game up.** `installGame(game)` in `lib/app.svelte.ts` asks for
the disc image and queues a job whose start is the `install_game` command
instead of `run_action`: `core/src/install.rs` plans `tools/extractor.py`
with the game's install folder, and the job runs like any other. The
library reports each game's set-up state (`GameView.install`), read from
`iso_data/<game>/buildinfo.json`; `uninstall_game` removes what set-up made
([INTEGRATION.md](INTEGRATION.md#2-setting-a-game-up)).

**When the game stops.** Play is a job like any other, so its end arrives as
`job-exit`. An exit status other than 0, or none (a signal), is not a failed
task: `gameStopped` in `lib/app.svelte.ts` asks `latest_crash` for the report
the port wrote since the job started and sets `app.stopped`, which the game's
page shows as `components/CrashNotice.svelte`. A game that ended **without**
such a report was closed, not crashed: the notice says so, in plain words,
and asks for nothing. `components/ReportsBlock.svelte` lists the game's logs
and reports under its saves, and `app.crashes` (counted after every refresh
and every game's end) marks the game in the list after a recent crash.
Every command run and every exit is written to `launcher.log`
(`core/src/launcher_log.rs`), which goes in the report's .zip.

**Cancelling.** `cancel_job` stops the job's whole process tree (its own
process group on Linux and macOS, `taskkill /T` on Windows): a build's
container or compiler goes with it.

## Commands

In `src-tauri/src/lib.rs`, but for the twelve that only the Android app
implements, which are in `src-tauri/src/android.rs` (marked Android below;
on a desktop they answer with nothing to do or an error). They call into
`core/src`, where the logic is (rule 4); the types are there too (serde,
camelCase) and generated from there into `src/lib/generated/` (below).

| Command                                                                                            | Arguments                                                                          | Returns                                                                                                                                                                     |
| -------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `app_info`                                                                                         |                                                                                    | `{version, platform, configFile}`                                                                                                                                           |
| `relaunch_app`                                                                                     |                                                                                    | restarts the launcher (after an update installed)                                                                                                                           |
| `launcher_update_check`                                                                            | `{channel}`                                                                        | a newer launcher on that channel, from Tauri's updater (desktop), or null                                                                                                   |
| `get_config` / `save_config`                                                                       | `{config}`                                                                         | `Config` (`core/src/config.rs`)                                                                                                                                             |
| `detect`                                                                                           |                                                                                    | candidates for the checkout, Python, Godot, Docker                                                                                                                          |
| `check_root`                                                                                       | `{path}`                                                                           | `{ok, version, message}`: is it an OpenRAC checkout                                                                                                                         |
| `check_tool`                                                                                       | `{tool, path}`                                                                     | the same, after running the tool's version flag                                                                                                                             |
| `adopt_godot`                                                                                      |                                                                                    | `Config`, with a Godot that works when none was set or the one set is gone (run at each start)                                                                              |
| `library`                                                                                          |                                                                                    | `Library` (`core/src/library.rs`)                                                                                                                                           |
| `install_game`                                                                                     | `{game, image}`                                                                    | the extractor's job, `{id, title, command, detached}`                                                                                                                       |
| `prepare_disc`                                                                                     | `{image}`                                                                          | the plain image set-up reads: an .iso as it is, a CHD unpacked once into `<install>/discs/<name>.iso` (`core/src/chd.rs`, `disc-copy` events)                               |
| `uninstall_game`                                                                                   | `{game}`                                                                           | removes the game's `iso_data`, `decompiler_out` and `out`                                                                                                                   |
| `scan_discs`                                                                                       |                                                                                    | `{found, choices}`: the games folder's discs, and (developers) the games with two to choose from                                                                            |
| `choose_disc`                                                                                      | `{version}`                                                                        | `Config`, after putting that disc's set-up in place (developers only; below)                                                                                                |
| `run_action`                                                                                       | `{scope, id, level, path, option}`                                                 | `{id, title, command, detached}`; `level`, `path`, `option` fill `{level}`, `{path}`, `{option}`                                                                            |
| `open_level_in_godot`                                                                              | `{version, level}`                                                                 | the version's `editor-open` action for that level, started as a job                                                                                                         |
| `open_output`                                                                                      | `{path}`                                                                           | opens a folder a job wrote (a file: its folder); only the checkout's `assets/` and `build/` and the install folder                                                          |
| `overrides_list`                                                                                   | `{game}`                                                                           | the game's overrides packs (`core/src/overrides.rs`), in order                                                                                                              |
| `overrides_set_enabled` / `overrides_move` / `overrides_remove`                                    | `{game, id, enabled}` / `{game, id, up}` / `{game, id}`                            | the packs again, changed                                                                                                                                                    |
| `plugins_list` / `plugins_set_enabled` / `plugins_move` / `plugins_set_setting` / `plugins_remove` | `{game, version}`, then `{id, enabled}` / `{id, up}` / `{id, key, value}` / `{id}` | the game's gameplay mods (native and script) in load order, as they stand for that disc (`core/src/plugins.rs`, `PluginView`), changed                                      |
| `enhanced_view` / `enhanced_set_enabled` / `enhanced_set_setting`                                  | `{game, version}`, then `{enabled}` / `{key, value}`                               | the Mods tab's Enhanced graphics entry for that version (`core/src/enhanced.rs`), or null; changed                                                                          |
| `plugins_install`                                                                                  | `{game, version, path}`                                                            | a .zip or folder added: `{kind: "plugin", id, name, version, replaced, enabled}`, or `{kind: "pack"}` for the page to add as an overrides pack                              |
| `plugins_status`                                                                                   | `{game}`                                                                           | what the port did with the mods at its last start (`status.json`), or null                                                                                                  |
| `plugins_preview`                                                                                  | `{game, version, url}`                                                             | what a repository's address holds (`core/src/remote.rs`, `Preview`): each mod, where it comes from, whether it can be installed here and why not                            |
| `plugins_install_remote`                                                                           | `{game, version, url, id}`                                                         | installs one of them: `{kind: "plugin", ...}` (checked against the release's `SHA256SUMS`), or `{kind: "pack", id, version, path}` for the page to add as an overrides pack |
| `plugins_check_updates`                                                                            | `{game, version}`                                                                  | `{updates, errors}`: newer releases (or a pack's newer commit) of the mods added from a link, but those kept at their version                                               |
| `plugins_update`                                                                                   | `{game, version, id}`                                                              | installs the newest, keeping its place, switch and settings                                                                                                                 |
| `plugins_sources` / `plugins_set_pin`                                                              | `{game}` / `{game, id, pin}`                                                       | where the mods added from a link came from (`sources.json`); keeps one at its version or lets it update                                                                     |
| `open_game_folder`                                                                                 | `{game, path}`                                                                     | opens a folder inside the game's set-up folder (an export); refuses anything else                                                                                           |
| `inspect_saves` / `backup_saves` / `restore_backup`                                                | `{serial}` / `{serial, note}` / `{serial, backupName}`                             | the card's slots and backups; a backup; a restore (refused while the game uses the card)                                                                                    |
| `open_saves_folder`                                                                                | `{serial}`                                                                         | opens the game's memory card folder, made if it is not there                                                                                                                |
| `read_save`                                                                                        | `{serial, slot}`                                                                   | one slot for the save editor (`core/src/save_edit.rs`, `SaveView`)                                                                                                          |
| `write_save`                                                                                       | `{serial, slot, fingerprint, change}`                                              | writes the editor's fields or raw chunks, safely (below); the backup and the slot as it now is                                                                              |
| `reports_list`                                                                                     | `{game}`                                                                           | the game's session logs and crash reports, newest first (`core/src/reports.rs`)                                                                                             |
| `latest_crash`                                                                                     | `{game, since}`                                                                    | the newest crash report since a time: after the game stopped                                                                                                                |
| `report_text` / `report_save`                                                                      | `{game, file}`                                                                     | the text to copy; the .zip saved in Downloads and shown (its path, home as `~`)                                                                                             |
| `report_delete` / `reports_delete_all`                                                             | `{game, file}` / `{game}`                                                          | deletes one log or crash report; all of them, and says how many went                                                                                                        |
| `open_reports_folder`                                                                              | `{game, crashes}`                                                                  | opens the game's data folder, or its `crashes/`                                                                                                                             |
| `differences` / `set_game_fix`                                                                     | `{game, version}` / `{game, version, id, on}`                                      | what the port does otherwise than the PS2 game (`core/src/differences.rs`); a fix switched                                                                                  |
| `cancel_job`                                                                                       | `{id}`                                                                             |                                                                                                                                                                             |
| `open_path`                                                                                        | `{path}` relative to the checkout                                                  | opens it with the system; refuses paths outside the checkout                                                                                                                |
| `open_url`                                                                                         | `{url}`                                                                            | opens an `https://` address in the browser                                                                                                                                  |
| `sync_progress_from_web`                                                                           |                                                                                    | fetches bytematch.dev's progress (`core/src/progress_sync.rs`), keeps it in the launcher's progress copy (never the checkout's `progress/summary.json`), returns `Library`  |
| `apply_progress_json`                                                                              | `{json}`                                                                           | the same from JSON the page fetched                                                                                                                                         |
| `set_discord_status`                                                                               | `{status}`                                                                         | sets Discord Rich Presence (idle, viewing, playing)                                                                                                                         |
| `pick_disc` (Android)                                                                              |                                                                                    | the system's picker; the image copied into the app (`disc-copy` events), its path or null                                                                                   |
| `gpu_drivers` (Android)                                                                            |                                                                                    | the GPU driver packages, the one chosen, the device (`OpenracPlugin.kt`)                                                                                                    |
| `gpu_driver_import` / `_select` / `_remove` (Android)                                              | - / `{id}` (null: the system's) / `{id}`                                           | adds a package's zip; chooses the one the game loads; removes one                                                                                                           |
| `gpu_renderer_select` (Android)                                                                    | `{renderer}`                                                                       | what the game draws with from its next start: `"vulkan"` or `"gles"`                                                                                                        |
| `background_status` (Android)                                                                      | `{title, text, percent}`                                                           | the set-up's notification while the launcher is in the background; nulls when the set-up is over                                                                            |
| `tools_ready` (Android)                                                                            |                                                                                    | waits until the tools the launcher carries are unpacked; an error says why they could not be                                                                                |
| `update_check` / `update_install` (Android)                                                        | `{channel}` / `{url, signature}`                                                   | a newer launcher on the releases (`core/src/update.rs`), or null; downloads its APK, checks the signature and hands it to Android's installer                               |
| `game_status` / `game_window` (Android)                                                            | - / `{stop}`                                                                       | `{running, library}`: whether the game runs; brings it back in front, or ends it                                                                                            |

| Event        | Payload                                                    |
| ------------ | ---------------------------------------------------------- |
| `job-output` | `{type: "output", id, stream: "stdout" \| "stderr", line}` |
| `job-exit`   | `{type: "exit", id, code, cancelled}`                      |
| `disc-copy`  | `{name, copied, total}` (Android: a picked disc's copy)    |

### Adding a command

1. The logic in `core/src/<module>.rs`, with a test.
2. A `#[tauri::command]` in `src-tauri/src/lib.rs` (`android.rs` for one only
   the Android app implements) that calls it, and its name in
   `generate_handler!`. The command only exposes the core: it holds no logic
   of its own. Use `spawn_blocking` for anything that touches the
   disk or runs a program.
3. `ts_rs::TS` on each new type the command sends or takes, the type added to
   the roots in `core/tests/generated_types.rs`, the files regenerated
   (below), and a wrapper in `src/lib/api.ts`.
4. A case in `src/lib/mock.ts`, so the browser preview keeps working.
5. If the page needs a new permission (a plugin), add it to
   `src-tauri/capabilities/default.json`, and only that one.

### The page's types

The TypeScript types of everything the commands send or take, and of the job
events, are generated from the Rust types in `core/src` with
[ts-rs](https://github.com/Aleph-Alpha/ts-rs) and live in
`src/lib/generated/`, one file per type. Nobody edits them. They are a
proposal, not a settled choice: the maintainers have not picked a generator
(restructure PROGRESS, question 5).

- **Derive.** A type the page receives derives `ts_rs::TS` beside its serde
  derives; ts-rs reads `rename_all`, `tag`, `flatten` and `skip`, so the
  TypeScript follows the JSON serde writes. Where Rust is looser than the page
  needs (a `String` that holds one of a few words), a `#[ts(type = "...")]` on
  the field says the words; where serde leaves a field out when it is empty
  (`skip_serializing_if`), `#[ts(optional)]` makes it optional. A type that
  would clash with another module's name gets a `#[ts(rename = "...")]`
  (`Kind` in `actions.rs` is `ActionKind`). A `serde_json::Value` is `unknown`.
- **Roots.** `core/tests/generated_types.rs` lists the types the commands
  send or take; what they contain is generated with them. It sets the one
  option ts-rs needs: integers past 2^53 become `number`, not `bigint`
  (no id, size or time here is that large).
- **Regenerate** after changing such a type:
  `OPENRAC_UPDATE_TYPES=1 cargo test -p openrac-launcher-core --test generated_types`,
  and commit the files.
- **Checked in, and why.** The page is checked and built (`yarn verify`,
  `yarn build`) without Rust, so the files are committed. The same test,
  run by `cargo test -p openrac-launcher-core` (CI's launcher job), fails when a
  committed file differs from what the Rust types generate now, or is no
  longer generated.
- **Not generated.** The types of commands that live in `src-tauri` (`AppInfo`,
  `DiscScan`, the Android commands' and the updater's) are written in
  `api.ts`: `src-tauri` is a thin layer and does not own the types a test in
  `core` could reach. `DiscordStatus` is written there too, because the Rust
  enum reads `game_id`, `progress_pct` and `start_time` where the page sends
  camelCase (restructure PROGRESS, follow-ups).
- **Using them.** `api.ts` re-exports the generated types the page names, so
  components import every type from `lib/api`; `mock.ts` imports them from
  `generated/` and types its answers on them.

### The two views

`Config.developer` (Settings, "Developer tools") chooses what the pages show.
Off, which is the default, a player's view. `lib/player.svelte.ts` answers
one question, what can the player do with this game now (`playerState`), and
does it (`playerAct`); `pages/GamePlayer.svelte` shows that answer as its main
button, and the game list (`components/GameList.svelte`, on `SideList.svelte`)
the same state in a few words, so the two never disagree. Every game has the
same page; the Games tab (`pages/Library.svelte`) opens on the last one used
(`config.lastVersion`) or the first, as OpenGOAL's launcher does. The secondary actions are in `components/Menu.svelte`
beside it. Each is a button over actions found by id in `actions.json`:

| Button                | What it runs                                                                                                                    |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Set up from your disc | `installGame`: the extractor, OpenGOAL's way (above)                                                                            |
| Play                  | the `play` action: the native port (planned for every version)                                                                  |
| Edit levels           | `editor-open`, after `editor-extract` and `editor-import` when the project is not there yet (`runChain` in `lib/app.svelte.ts`) |

On, the game's page gains one row of chips that lead to the Develop tab
(`pages/Develop.svelte`): a list of the checkout, every version and the
console, and beside it `components/DeveloperPanel.svelte`: the version's actions as rows
(`ActionRow.svelte`), grouped by the action's `area` in `actions.json` so the
game's two builds are never side by side: "PC port" (`port`), "Decompilation"
(`decomp`, the default, with the progress), "Level editor" (`editor`: its
checks; editing is under Mods), then the files and the facts of the version.
Develop's list shows each version's progress, and "This checkout"
(`components/DevCheckout.svelte`) the checkout's own actions. On Android,
Develop has no list and no checkout: it shows the game picked in Games (named
by a chip, as on Mods), with its versions and the console as tabs. Mods,
Activity, Develop and the save editor load the first time they are opened
(`App.svelte`); motion is in `lib/motion.ts` and stops with the system's
reduced-motion setting. A version whose `play` or `editor-extract` is `planned`
shows as "not playable yet" or "cannot open this game yet" to a player. A
first start asks nothing when the checkout and a Python are found
(`firstRun`).

### A developer's two discs

With developer tools on and two versions of a game among the games folder's
discs, `scan_discs` returns a `DiscChoice` for it (`core/src/disc_choice.rs`)
and the game's page shows `components/DiscSwitch.svelte` under Play.
`playerVersion` then plays the chosen version (`lib/discChoice.ts`), so
Play, the levels, Mods, the saves (the card is the version's serial) and the
logs (`ReportsBlock` lists only that version's program) follow it. The
choice is `Config.discChoice`, by game; without one, the disc set up is
played, as for a player. The levels, the build and the mods already have a
folder per version; the set-up (`iso_data/<game>`, which the port reads, and
`decompiler_out`, `out`) is one per game, so `choose_disc` moves the one in
place to `<game>@<version>` beside it and the chosen version's back, and a
developer's set-up of the other disc keeps the first aside the same way
(`before_set_up`). Nothing is extracted again or deleted. Off, or with one
disc, none of this runs.

### Adding a page

A component in `src/pages/`, a `Page` name in `lib/app.svelte.ts`, a tab in
`components/Header.svelte` and a branch in `App.svelte`. Pages read `app` and
call the functions of `lib/app.svelte.ts`; they do not call `api` for anything
that changes state.

## The save editor

On the Play page, **Edit** beside a slot of the Save files list opens
`components/SaveEditor.svelte` over that slot. The format and every field
are in `core/src/save_edit.rs`, with unit tests on saves built in code (no
real save file is kept in the repository).

**The format.** Both Ratchet & Clank discs (PAL `SCES_509.16`, US
`SCUS_971.99`) write the same `saveN.bin`: their descriptor tables list the
same chunks with the same ids and sizes, so one decoder serves both, picked
by the card's serial; the page never says which disc. The file is two section
sizes (0x1530, 0xaa4), a global section and 20 per-planet sections; a section
is `{size, crc16}` then `{id, size, bytes}` chunks padded to 4 and an end
marker. The checksum is the game's `memcard_Checksum` (CRC-16, polynomial
0x1F45, seed 0xEDB88320 of which the low 16 bits count, nothing above 0x1800
bytes). On load the game checks the two sizes, each section's checksum (non-zero
and equal) and every chunk's id and size; the editor keeps ids, sizes and
padding as they were and writes a fresh checksum for each section (a checksum
that would come out 0 is avoided through a padding byte the game never reads),
so an edited save loads. It came from the game's own code: `memcard.c`,
`pause.c`, `vendor.c` and the level code of the PAL decomp, and
`src/storage/memory_card/` of the US one; the item names and limits from the
executable's item and vendor tables; checked against ReRAC's notes
([THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md#rerac)).

**The fields**, only those whose meaning the code makes plain: the planet the
save loads on, bolts (to
9,999,999, the game's own limit), health (Premium Nanotech for 5, Ultra for
8: the two story flags and the maximum together), weapons and gadgets owned,
ammo (to each weapon's limit, the vendor's "full" test), the ten gold weapons,
the 40 gold bolts by planet, the planets unlocked for the ship (Novalis to
Kalebo III; the others come with the story) and the 30 skill points. Giving
or taking an item keeps the related chunks as the game does: owned and
acquired, the vendor's stock, and the quick select and equipped slots, which
let go of an item taken away. Unlocking a planet adds it to the galaxy map
after those unlocked before it.

**The page.** The editor is a large panel over the Play page (most of the
window, at any size it can take). Its title says what the slot holds ("Slot 2 ·
Kerwan · 4,890 bolts · 3h 12m", time played from chunk 3), and a row of links
to its sections stays at the top of the Player view. Quick actions sit in the
section headings: give all and take all weapons, fill all ammo, give all and
take all gadgets and items (taking asks first, with the story-item warning),
unlock or lock all ship destinations (the planet the save loads on stays
unlocked), all or no gold bolts, all or no skill points. They change the same
fields as the single edits, so the Rust side applies the same rules (vendor
stock, acquired flags, quick select). The planet choice heads the Planets
section, beside the Unlocked boxes it affects.

**Editing.** What differs from the save as read carries an amber dot where it
is and on its section's link; the link of the section in view is lit. Above
Save, "3 changes" opens the list of them in a player's words, a line per kind
of change ("Given: Devastator, Visibomb Gun", `saveEdit.ts`'s `editList`),
each leading to its section. Reset puts the slot back as read without
closing. Leaving with changes asks in the editor (Keep editing, Discard, Save
and close), as does taking away every gadget; no browser box. A save leaves
the editor open: Undo puts the backup back, and a new change brings Save
back. While the launcher runs the game, which writes the card too, Save waits
and says why. The editor is a dialog: focus goes into it, the page behind is
`inert`, Escape steps back one thing at a time, Ctrl+S saves, and focus
returns to the slot's Edit button.

**Skill point names** come from the game's own text, read at run time from the
set-up disc (`<install>/active/rac1/data/iso_data/rac1/disc.iso`):
`save_edit::skill_point_names` reads the table of contents at LBA 1500, the
`all_text` lump it points to (+0x1528) and the English block's ids
20444..20473, the names the pause menu's Skill Points page shows, in the
save's order (the executable's skill-point description table, ids 20474..,
is in the same order). Both rac1 discs hold the same names at the same ids.
Without a readable disc the page numbers them 1 to 30. No game text is kept
in the repository.

**The planet.** On Load Game the game restores the save and runs
`DoSpaceTransition` (PAL `func_00233308`) with the saved planet (chunk 0) as
both the planet left and the one reached; the level start then unlocks the
planet, marks it visited and puts Ratchet at the level's start point (where
the ship lands; position is never saved). Moving a save is therefore chunk 0,
with the planet unlocked and added to the galaxy map as the level start would.
A planet never visited plays its landing first. The default mode offers the
ship's destinations, Novalis to Kalebo III; Veldin, Drek's Fleet and the last
Veldin come with their story scenes and the state those leave, so they are
offered only in the advanced mode (a save already on one keeps it).

**Advanced mode** (Settings, "Advanced save editing", off by default) adds an
Advanced tab (`components/AdvancedSaveEditor.svelte`) for debugging: every
decoded field (the game's numbers and options, every item's owned, ever-had,
gold and ammo, the quick select, equipped and vendor slots, the 32 skill-point
bytes, the story and cheat flags, the galaxy map order, and each planet's
section: unlocked, visited, gold bolts, missions, bolts, hits and deaths), any
planet as the current one, and a raw view of every chunk by section and id
with its size and its bytes as hex. What is not decoded is marked "raw" or
"not decoded". The tab edits bytes only and applies no rules; it saves as raw
chunk patches (`SaveChange::Raw`): each keeps its size, the checksums are
written fresh, and the same guards apply. `write_save` refuses raw patches
unless the setting is on. Switching tabs waits until the changes are saved or
cancelled.

**Safety.** `write_save` refuses to write when a game program (by its name,
`openrac-rac1-pal` or `Ratchet & Clank (PAL)`, and the same for NTSC) runs
with that card: its `--cards` folder, or its default card when it has none,
and also when the process list cannot be read. It refuses when the file
changed since the page read it (a digest), and when a value is out of range
(checked on the page and again in Rust). It backs the whole card up to
`.backup_<time>_before_edit_slotN` (in the Backups list, never overwriting
an earlier backup), writes a temporary file beside the save, reads it back,
and replaces the save in one rename. After a save the editor offers Undo,
which restores that backup. Restoring any backup is refused while the game
runs with the card, too.

**Checking it with the game.** Copy the card folder somewhere else, edit a
slot there (a few lines of Rust over `save_edit::write_save`), and start the
port with `--cards <copy>/<serial>`: Load Game lists the slot with the new
bolts, and loading it logs `bolts N` and shows the weapons in the pause menu;
a save moved to Kerwan or Rilgar, never visited, lands there and leaves
Ratchet at the start
([docs/port/RAC1_PAL_TESTING.md](../../docs/port/RAC1_PAL_TESTING.md) for
scripted presses and pictures). Never point a test at the player's own card.

## Security

- The page gets `core:default` and the open-file dialog only
  (`src-tauri/capabilities/default.json`). Opening links and folders is the
  Rust side's job, with its own checks (https only; paths inside the checkout).
- The content security policy (`tauri.conf.json`) allows only the app's own
  scripts, styles and fonts; the fonts are bundled. The page's own `fetch`
  of openrac.dev is outside that policy's `connect-src`, so the progress is
  fetched by the Rust side (`sync_progress_from_web`, with `curl`:
  `core/src/progress_sync.rs`) when the page's attempt fails.
- `actions.json` comes from the user's own checkout, which they already trust
  to build and run. Its arguments are passed as an argument list, and only
  the placeholders in `core/src/actions.rs` (`PLACEHOLDERS`) are filled; the
  tests reject any other.

## Platforms

Tauri builds for Linux (AppImage, deb, rpm), macOS (app, dmg) and Windows
(NSIS, MSI): `"targets": "all"` in `tauri.conf.json`. The games' builds are not
equally portable: each action lists the platforms it runs on, and the game
page tucks the others away ("N more for other platforms"). The platform the
launcher runs on comes from `Platform::current()` in `core/src/actions.rs`.
Android has no shell, Python, Godot or compiler: there an action that names no
platform runs only if it is a `play` action (`runs_on`), and the others are not
listed.

### What Linux gets

**The AppImage's environment.** Every AppImage carries AppImageKit's `AppRun`,
which sets a list of variables before the launcher starts: `PYTHONHOME`,
`PYTHONPATH`, `PERLLIB`, `GST_PLUGIN_SYSTEM_PATH`, `QT_PLUGIN_PATH` and
`LD_LIBRARY_PATH`, all pointing into the AppImage. This AppImage carries no
Perl, no GStreamer and no Qt, so those variables point at nothing: a Qt
program told to look for its platform plugin in ten directories that do not
exist says "Could not find the Qt platform plugin" and stops. The launcher
takes them back (`core/src/appimage.rs`): the ones that mean nothing go, and
what the launcher starts (the game, the tools, the program that opens a link
or a folder) gets the system's own environment, not the AppImage's. The
launcher's own webview keeps the AppImage's libraries, as its helper processes
(WebKitWebProcess, WebKitNetworkProcess) find the AppImage's WebKit nowhere
else.

**The webview.** Two of WebKitGTK's settings draw nothing on some systems, and
are turned off there, before GTK starts (`core/src/webkit.rs`): its DMA-BUF
renderer under Wayland with NVIDIA's own driver, and its accelerated
compositing mode on a Steam Deck, where the window opens empty unless the
launcher is started with `WEBKIT_DISABLE_COMPOSITING_MODE=1`. A value the
player set is left as it is.

Android (`Platform::Android`) is an APK built by `tools/build_android.py`
(docs/port/ANDROID.md): the same UI and Rust, a project of its own in
`src-tauri/gen/android` (kept in git), and Kotlin for what only Android does
(`OpenracPlugin.kt`, reached from `src-tauri/src/android.rs`): the system's
file picker, the game's activity, the GPU driver packages. It always plays
from the copy it carries, unpacked from the APK by `OpenracSetup.kt`; Play
starts the game's activity instead of a program, and the updater is the
desktop's only (`capabilities/updater.json`).
