# How the launcher is built

```
 ┌─────────────── UI (src/, Svelte 5, in the webview) ───────────────┐
 │ pages ─► lib/app.svelte.ts (state, queue) ─► lib/api.ts ──┐       │
 │                                    browser preview: lib/mock.ts   │
 └───────────────────────────────────────────────────────────┼───────┘
                          invoke("library"), invoke("run_action"), ...
                          events: job-output, job-exit        │
 ┌──────────── src-tauri/src/lib.rs (Tauri commands) ─────────▼───────┐
 │ a thin layer: settings file location, events, opener               │
 └──────────────────────────────┬──────────────────────────────────────┘
 ┌──────────── core/ (openrac-launcher-core, no window) ─▼─────────────┐
 │ catalog ◄─ games/*/game.json, progress/summary.json                 │
 │ actions ◄─ launcher/actions.json        status ◄─ baserom/, inputs  │
 │ install ◄─ <install folder>/active/<game>/data (the player's discs) │
 │ library: all of these for the UI; plan(scope, id) -> command line   │
 │ jobs: child processes, output lines, cancel     detect, config      │
 │ discord: Rich Presence over Discord's local socket                  │
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
   the progress copy and the Linux desktop files
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
- `run_action` checks the action again against the checkout as it is now; a
  job whose needs went missing in the meantime fails with the reason.

**Setting a game up.** `installGame(game)` in `lib/app.svelte.ts` asks for
the disc image and queues a job whose start is the `install_game` command
instead of `run_action`: `core/src/install.rs` plans `tools/extractor.py`
with the game's install folder, and the job runs like any other. The
library reports each game's set-up state (`GameView.install`), read from
`iso_data/<game>/buildinfo.json`; `uninstall_game` removes what set-up made
([INTEGRATION.md](INTEGRATION.md#2-setting-a-game-up)).

**Cancelling.** `cancel_job` stops the job's whole process tree (its own
process group on Linux and macOS, `taskkill /T` on Windows): a build's
container or compiler goes with it.

## Commands

All in `src-tauri/src/lib.rs`; types in `core/src` (serde, camelCase) and
mirrored in `src/lib/api.ts`.

| Command                      | Arguments                         | Returns                                                                               |
| ---------------------------- | --------------------------------- | ------------------------------------------------------------------------------------- |
| `app_info`                   |                                   | `{version, platform, configFile}`                                                     |
| `get_config` / `save_config` | `{config}`                        | `Config` (`core/src/config.rs`)                                                       |
| `detect`                     |                                   | candidates for the checkout, Python, Godot, Docker                                    |
| `check_root`                 | `{path}`                          | `{ok, version, message}`: is it an OpenRAC checkout                                   |
| `check_tool`                 | `{tool, path}`                    | the same, after running the tool's version flag                                       |
| `library`                    |                                   | `Library` (`core/src/library.rs`)                                                     |
| `install_game`               | `{game, image}`                   | the extractor's job, `{id, title, command, detached}`                                 |
| `uninstall_game`             | `{game}`                          | removes the game's `iso_data`, `decompiler_out` and `out`                             |
| `run_action`                 | `{scope, id}`                     | `{id, title, command, detached}`                                                      |
| `cancel_job`                 | `{id}`                            |                                                                                       |
| `open_path`                  | `{path}` relative to the checkout | opens it with the system; refuses paths outside the checkout                          |
| `open_url`                   | `{url}`                           | opens an `https://` address in the browser                                            |
| `sync_progress_from_web`     |                                   | fetches openrac.dev's progress, writes `progress/summary.json`, returns the catalogue |
| `apply_progress_json`        | `{json}`                          | the same from JSON the page fetched                                                   |
| `set_discord_status`         | `{status}`                        | sets Discord Rich Presence (idle, viewing, playing)                                   |

| Event        | Payload                                                    |
| ------------ | ---------------------------------------------------------- |
| `job-output` | `{type: "output", id, stream: "stdout" \| "stderr", line}` |
| `job-exit`   | `{type: "exit", id, code, cancelled}`                      |

### Adding a command

1. The logic in `core/src/<module>.rs`, with a test.
2. A `#[tauri::command]` in `src-tauri/src/lib.rs` that calls it, and its name
   in `generate_handler!`. Use `spawn_blocking` for anything that touches the
   disk or runs a program.
3. The types and a wrapper in `src/lib/api.ts`.
4. A case in `src/lib/mock.ts`, so the browser preview keeps working.
5. If the page needs a new permission (a plugin), add it to
   `src-tauri/capabilities/default.json`, and only that one.

### The two views

`Config.developer` (Settings, "Developer tools") chooses what the pages show.
Off, which is the default, a player's view. `lib/player.svelte.ts` answers
one question, what can the player do with this game now (`playerState`), and
does it (`playerAct`); the games page's tiles (`components/GameTile.svelte`)
and `pages/GamePlayer.svelte` both show that answer as their main button, so
the two never disagree. The secondary actions are in `components/Menu.svelte`
beside it. Each is a button over actions found by id in `actions.json`:

| Button                | What it runs                                                                                                                    |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Set up from your disc | `installGame`: the extractor, OpenGOAL's way (above)                                                                            |
| Play                  | the `play` action: the native port (planned for every version)                                                                  |
| Edit levels           | `editor-open`, after `editor-extract` and `editor-import` when the project is not there yet (`runChain` in `lib/app.svelte.ts`) |

On, `pages/GameDeveloper.svelte`: every action by kind, the inputs and the
progress. A version whose `play` or `editor-extract` is `planned`
shows as "not playable yet" or "cannot open this game yet" to a player. A
first start asks nothing when the checkout and a Python are found
(`firstRun`).

### Adding a page

A component in `src/pages/`, a `Page` name in `lib/app.svelte.ts`, a tab in
`components/Header.svelte` and a branch in `App.svelte`. Pages read `app` and
call the functions of `lib/app.svelte.ts`; they do not call `api` for anything
that changes state.

## Security

- The page gets `core:default` and the open-file dialog only
  (`src-tauri/capabilities/default.json`). Opening links and folders is the
  Rust side's job, with its own checks (https only; paths inside the checkout).
- The content security policy (`tauri.conf.json`) allows only the app's own
  scripts, styles and fonts; the fonts are bundled. The page's own `fetch`
  of openrac.dev is outside that policy's `connect-src`, so the progress is
  fetched by the Rust side (`sync_progress_from_web`, with `curl`) when the
  page's attempt fails.
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
