// OpenRAC Launcher: the desktop app. Every file and process operation lives
// in openrac-launcher-core (../core); this file only exposes it to the UI
// (../src) as Tauri commands and forwards job output as events. The list of
// commands and events is docs/ARCHITECTURE.md's; keep them in step with
// src/lib/api.ts and src/lib/mock.ts.
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

mod android;

use openrac_launcher_core::actions::{Platform, Scope};
use openrac_launcher_core::config::{self, Config};
use openrac_launcher_core::detect::{self, Check, Detected, Tool};
use openrac_launcher_core::disc_choice::{self, DiscChoice};
use openrac_launcher_core::discs::{self, FoundDisc};
use openrac_launcher_core::install;
use openrac_launcher_core::jobs::{Event, Jobs, Started};
use openrac_launcher_core::launcher_log;
use openrac_launcher_core::library::{self, Library};
use openrac_launcher_core::overrides::{self, PackView};
use openrac_launcher_core::plugins::{self, Installed, PluginView};
use openrac_launcher_core::remote;
use openrac_launcher_core::reports::{self, ReportEntry};
use serde::Serialize;
use tauri::{AppHandle, Emitter, Manager, State};
use tauri_plugin_opener::OpenerExt;

struct AppState {
    config: Mutex<Config>,
    config_file: Option<PathBuf>,
    jobs: Jobs,
    discord: Arc<Mutex<DiscordState>>,
    /// The copy of OpenRAC this launcher carries, if it was built with one (detect::bundled_root).
    bundled: Option<PathBuf>,
}

struct DiscordState {
    ipc: openrac_launcher_core::discord::DiscordIpc,
    app_start: u64,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct AppInfo {
    version: String,
    platform: Platform,
    /// Where launcher.json is.
    config_file: Option<PathBuf>,
    /// The launcher works on the copy of OpenRAC it carries: nothing to find or to build, and
    /// only what a player does is offered.
    standalone: bool,
    /// Where its updates come from: the repository that built it (core/src/update.rs).
    updates: openrac_launcher_core::update::UpdateSource,
    /// That repository's two channels, which the player chooses between (Settings, `updateChannel`).
    update_channels: UpdateChannels,
}

#[derive(Serialize)]
struct UpdateChannels {
    stable: openrac_launcher_core::update::UpdateSource,
    dev: openrac_launcher_core::update::UpdateSource,
}

/// A build of a branch: `dev.<run>`, which its updates are compared with (update::effective_version).
const LAUNCHER_BUILD: Option<&str> = option_env!("OPENRAC_LAUNCHER_BUILD");

/// The version updates are compared with, and the one shown.
fn launcher_version<R: tauri::Runtime>(app: &tauri::AppHandle<R>) -> String {
    openrac_launcher_core::update::effective_version(&app.package_info().version.to_string(), LAUNCHER_BUILD)
}

/// The update manifest's address (tauri.conf.json's `plugins.updater.endpoints`, which the build
/// sets to its own repository's releases).
fn update_source<R: tauri::Runtime>(app: &tauri::AppHandle<R>) -> openrac_launcher_core::update::UpdateSource {
    let endpoint =
        app.config().plugins.0.get("updater").and_then(|c| c["endpoints"][0].as_str()).unwrap_or_default().to_string();
    openrac_launcher_core::update::source(&endpoint)
}

/// Where the launcher looks for its updates: CHANNEL and a fork's BRANCH (the Settings page's,
/// before they are saved), else the ones the player chose, else the one it was built from.
fn chosen_update_source<R: tauri::Runtime>(
    app: &tauri::AppHandle<R>,
    channel: Option<openrac_launcher_core::update::Channel>,
    branch: Option<String>,
) -> openrac_launcher_core::update::UpdateSource {
    let built = update_source(app);
    let (channel, branch) = match channel {
        Some(channel) => (Some(channel), branch),
        None => app.try_state::<AppState>().map_or((None, None), |state| {
            let config = state.config.lock().unwrap();
            (config.update_channel, config.update_branch.clone())
        }),
    };
    built.chosen(channel.unwrap_or(built.kind()), branch.as_deref())
}

#[tauri::command]
fn app_info(app: AppHandle, state: State<'_, AppState>) -> AppInfo {
    use openrac_launcher_core::update::Channel;
    let updates = update_source(&app);
    AppInfo {
        version: launcher_version(&app),
        platform: Platform::current(),
        config_file: state.config_file.clone(),
        standalone: state
            .bundled
            .as_deref()
            .is_some_and(|copy| detect::on_the_copy(state.config.lock().unwrap().root.as_deref(), copy)),
        update_channels: UpdateChannels { stable: updates.on(Channel::Stable), dev: updates.on(Channel::Dev) },
        updates,
    }
}

/// A newer launcher on the player's channel, or None: Tauri's updater asked at that channel's
/// manifest, and the update kept for its own download and install (plugin-updater's `Update`,
/// rebuilt in the page from `rid`). A desktop's; Android's launcher has android::update_check.
#[tauri::command]
async fn launcher_update_check(
    webview: tauri::Webview,
    channel: Option<openrac_launcher_core::update::Channel>,
    branch: Option<String>,
) -> Result<Option<serde_json::Value>, String> {
    #[cfg(desktop)]
    {
        use tauri_plugin_updater::UpdaterExt;
        let source = chosen_update_source(webview.app_handle(), channel, branch);
        let url = tauri::Url::parse(&source.manifest).map_err(|e| format!("{}: {e}", source.manifest))?;
        let updater =
            webview.updater_builder().endpoints(vec![url]).and_then(|b| b.build()).map_err(|e| e.to_string())?;
        let Some(update) = updater.check().await.map_err(|e| e.to_string())? else { return Ok(None) };
        let date = update.raw_json.get("pub_date").cloned();
        let (current_version, version, body, raw_json) =
            (update.current_version.clone(), update.version.clone(), update.body.clone(), update.raw_json.clone());
        let rid = webview.resources_table().add(update);
        Ok(Some(serde_json::json!({
            "rid": rid,
            "currentVersion": current_version,
            "version": version,
            "date": date,
            "body": body,
            "rawJson": raw_json,
        })))
    }
    #[cfg(not(desktop))]
    {
        let _ = (webview, channel, branch);
        Err("on Android the launcher updates through update_check".into())
    }
}

#[tauri::command]
fn relaunch_app(app: AppHandle) {
    tauri::process::restart(&app.env());
}

#[tauri::command]
fn get_config(state: State<'_, AppState>) -> Config {
    state.config.lock().unwrap().clone()
}

/// A desktop's install folder holds the player's saves and settings too (core saves.rs): those
/// kept where earlier launchers kept them, or in the install folder `before` it (a folder chosen
/// in Settings), are copied in. Android keeps its own (OpenracSetup.kt).
fn settle_data_home(before: Option<&Path>, config: &Config) {
    use openrac_launcher_core::saves;
    let Some(install) = config.install_dir.as_deref().filter(|_| Platform::current() != Platform::Android) else {
        return;
    };
    let mut copied = before.filter(|before| *before != install).map_or(0, |before| {
        saves::adopt(before, install).unwrap_or_else(|failed| {
            launcher_log::line(&format!("saves: {failed} files could not be copied from {}", before.display()));
            0
        })
    });
    copied += saves::settle_data_home(install);
    if copied > 0 {
        launcher_log::line(&format!("saves: {copied} files copied into {}", install.display()));
    }
}

#[tauri::command]
fn save_config(state: State<'_, AppState>, mut config: Config) -> Result<Config, String> {
    let file = state.config_file.as_deref().ok_or("no per-user config folder")?;
    // Developer tools switched off: back to the carried copy (detect::settle).
    detect::settle(&mut config, state.bundled.as_deref());
    // The page sends the settings back without the paths the launcher keeps its own files
    // in; they are set again here.
    config.progress_cache = progress_cache(state.config_file.as_deref());
    config::save(file, &config)?;
    let before = std::mem::replace(&mut *state.config.lock().unwrap(), config.clone());
    if before.install_dir != config.install_dir {
        settle_data_home(before.install_dir.as_deref(), &config);
    }

    let discord = state.discord.clone();
    let target_id = config
        .discord_client_id
        .clone()
        .unwrap_or_else(|| openrac_launcher_core::discord::DEFAULT_CLIENT_ID.to_string());
    let rpc_enabled = config.discord_rpc;
    tauri::async_runtime::spawn(async move {
        let _ = tauri::async_runtime::spawn_blocking(move || {
            let mut discord = discord.lock().unwrap();
            discord.ipc.set_client_id(target_id);
            if !rpc_enabled {
                let _ = discord.ipc.clear();
            }
        })
        .await;
    });

    Ok(config)
}

/// Where to look for a checkout: next to the launcher and the folder it was started from.
fn start_folders() -> Vec<PathBuf> {
    let exe = std::env::current_exe().ok().and_then(|p| p.parent().map(PathBuf::from));
    exe.into_iter().chain(std::env::current_dir().ok()).collect()
}

/// The file the launcher keeps its own copy of the figures it fetched from bytematch.dev in:
/// in the per-user folder beside its settings, which the launcher can always write. A
/// checkout it does not own (the copy an installed launcher carries) is read-only.
fn progress_cache(config_file: Option<&std::path::Path>) -> Option<PathBuf> {
    config_file.and_then(|file| file.parent()).map(|dir| dir.join("progress.json"))
}

#[tauri::command]
async fn detect(state: State<'_, AppState>) -> Result<Detected, String> {
    let bundled = state.bundled.clone();
    Ok(tauri::async_runtime::spawn_blocking(move || detect::detect_all(&start_folders(), bundled.as_deref()))
        .await
        .unwrap_or_default())
}

#[tauri::command]
async fn check_root(path: PathBuf) -> Check {
    detect::check_root(&path)
}

#[tauri::command]
async fn check_tool(tool: Tool, path: PathBuf) -> Result<Check, String> {
    tauri::async_runtime::spawn_blocking(move || detect::check_tool(tool, &path)).await.map_err(|e| e.to_string())
}

/// Takes a Godot that works when none is set, or the one set is gone: the first run looks only
/// once, and a player often gets Godot later, from the Mods page's link. The config as it is.
#[tauri::command]
async fn adopt_godot(state: State<'_, AppState>) -> Result<Config, String> {
    if state.config.lock().unwrap().godot.as_deref().is_some_and(Path::is_file) {
        return Ok(state.config.lock().unwrap().clone());
    }
    let found = tauri::async_runtime::spawn_blocking(|| {
        let first = detect::tool_candidates(Tool::Godot).into_iter().next()?;
        detect::check_tool(Tool::Godot, &first.path).ok.then_some(first.path)
    })
    .await
    .map_err(|e| e.to_string())?;
    let mut config = state.config.lock().unwrap();
    if let (Some(godot), Some(file)) = (found, state.config_file.as_deref()) {
        if !config.godot.as_deref().is_some_and(Path::is_file) {
            let mut next = config.clone();
            next.godot = Some(godot);
            config::save(file, &next)?;
            *config = next;
        }
    }
    Ok(config.clone())
}

#[tauri::command]
async fn library(state: State<'_, AppState>) -> Result<Library, String> {
    let config = state.config.lock().unwrap().clone();
    tauri::async_runtime::spawn_blocking(move || library::library(&config)).await.map_err(|e| e.to_string())?
}

/// Runs action `id` of `scope`: a job whose output streams as `job-output`
/// and `job-exit` events, or a detached launch (id 0).
#[tauri::command]
fn run_action(
    app: AppHandle,
    state: State<'_, AppState>,
    scope: Scope,
    id: String,
    level: Option<u32>,
    path: Option<String>,
    option: Option<String>,
) -> Result<Started, String> {
    let config = state.config.lock().unwrap().clone();
    let inputs = openrac_launcher_core::actions::Inputs { level, path, option };
    let plan = library::plan(&config, &scope, &id, &inputs)?;
    let is_play = library::is_play(&config, &scope, &id);

    // The play job's Discord presence (core discord.rs): its status now, its game for its log.
    let play = match (is_play, &config.root) {
        (true, Some(root)) => {
            openrac_launcher_core::discord::play(root, &scope, openrac_launcher_core::discord::now_sec())
        }
        _ => None,
    };
    if let (Some(play), true) = (&play, config.discord_rpc) {
        let discord = state.discord.clone();
        let status = play.status.clone();
        tauri::async_runtime::spawn_blocking(move || {
            let mut discord = discord.lock().unwrap();
            let app_start = discord.app_start;
            let _ = discord.ipc.set_status(&status, app_start);
        });
    }
    let play_game = play.map(|p| p.game);
    // On Android the game is an activity of the launcher's own, not a program it runs.
    if is_play && Platform::current() == Platform::Android {
        launcher_log::line(&format!("play: {}", openrac_launcher_core::jobs::command_line(&plan)));
        return android::launch(&app, &plan);
    }
    let app_handle = app.clone();
    let title = plan.title.clone();
    launcher_log::line(&format!("run: {}", openrac_launcher_core::jobs::command_line(&plan)));
    let started = state.jobs.start(&plan, config.root.as_deref(), move |event| {
        let _ = match &event {
            Event::Output { line, .. } => {
                // The planet and bolts the game logs.
                if let Some(log) =
                    play_game.as_deref().and_then(|game| openrac_launcher_core::discord::read_game_log(game, line))
                {
                    if let Some(st) = app_handle.try_state::<AppState>() {
                        if st.config.lock().unwrap().discord_rpc {
                            let discord = st.discord.clone();
                            tauri::async_runtime::spawn_blocking(move || {
                                let _ = discord.lock().unwrap().ipc.on_game_log(log);
                            });
                        }
                    }
                }
                app.emit("job-output", &event)
            }
            Event::Exit { code, cancelled, .. } => {
                launcher_log::line(&format!(
                    "ended: {title}: {}",
                    if *cancelled {
                        "cancelled".to_string()
                    } else {
                        code.map_or("stopped by a signal (no exit code)".to_string(), |c| format!("exit code {c}"))
                    }
                ));
                if is_play {
                    if let Some(st) = app_handle.try_state::<AppState>() {
                        let cfg = st.config.lock().unwrap().clone();
                        if cfg.discord_rpc {
                            let discord = st.discord.clone();
                            tauri::async_runtime::spawn_blocking(move || {
                                let mut discord = discord.lock().unwrap();
                                let app_start = discord.app_start;
                                let _ = discord.ipc.end_play(app_start);
                            });
                        }
                    }
                }
                app.emit("job-exit", &event)
            }
        };
    });
    if let Err(e) = &started {
        launcher_log::line(&format!("could not start: {e}"));
    }
    started
}

// ---- Differences from the original (core/src/differences.rs) ----

/// The Mods tab's built-in Enhanced graphics entry for a version (core enhanced.rs), or None for a
/// version that has none.
#[tauri::command]
async fn enhanced_view(
    state: State<'_, AppState>,
    game: String,
    version: String,
) -> Result<Option<openrac_launcher_core::enhanced::EnhancedView>, String> {
    use openrac_launcher_core::enhanced;
    if !enhanced::supported(&game, &version) {
        return Ok(None);
    }
    let config = state.config.lock().unwrap().clone();
    let home = openrac_launcher_core::saves::data_home();
    tauri::async_runtime::spawn_blocking(move || {
        let bake = enhanced::version_bake_status(&config, &format!("{game}/{version}"));
        Some(enhanced::view(&home, &game, &version, bake))
    })
    .await
    .map_err(|e| e.to_string())
}

/// Switches Enhanced graphics on or off for a version, in its settings file.
#[tauri::command]
async fn enhanced_set_enabled(
    state: State<'_, AppState>,
    game: String,
    version: String,
    enabled: bool,
) -> Result<Option<openrac_launcher_core::enhanced::EnhancedView>, String> {
    let home = openrac_launcher_core::saves::data_home();
    let (g, v) = (game.clone(), version.clone());
    tauri::async_runtime::spawn_blocking(move || openrac_launcher_core::enhanced::set_enabled(&home, &g, &v, enabled))
        .await
        .map_err(|e| e.to_string())??;
    launcher_log::line(&format!("{game}/{version}: enhanced graphics {}", if enabled { "on" } else { "off" }));
    enhanced_view(state, game, version).await
}

/// Sets one of Enhanced graphics' settings for a version, in its settings file.
#[tauri::command]
async fn enhanced_set_setting(
    state: State<'_, AppState>,
    game: String,
    version: String,
    key: String,
    value: serde_json::Value,
) -> Result<Option<openrac_launcher_core::enhanced::EnhancedView>, String> {
    let home = openrac_launcher_core::saves::data_home();
    let (g, v) = (game.clone(), version.clone());
    tauri::async_runtime::spawn_blocking(move || {
        openrac_launcher_core::enhanced::set_setting(&home, &g, &v, &key, &value)
    })
    .await
    .map_err(|e| e.to_string())??;
    enhanced_view(state, game, version).await
}

/// The checkout a game's list of differences is read from: the one set, else the copy carried.
fn differences_root(state: &State<'_, AppState>) -> Option<PathBuf> {
    state.config.lock().unwrap().root.clone().or_else(|| state.bundled.clone())
}

/// What the port does otherwise than the PS2 game for one version, with the fixes' values.
#[tauri::command]
async fn differences(
    state: State<'_, AppState>,
    game: String,
    version: String,
) -> Result<openrac_launcher_core::differences::Differences, String> {
    let root = differences_root(&state);
    let home = openrac_launcher_core::saves::data_home();
    tauri::async_runtime::spawn_blocking(move || {
        openrac_launcher_core::differences::view(root.as_deref(), &home, &game, &version)
    })
    .await
    .map_err(|e| e.to_string())?
}

/// Switches a game fix on (the port's) or off (the console's behaviour) in the version's settings.
#[tauri::command]
async fn set_game_fix(
    state: State<'_, AppState>,
    game: String,
    version: String,
    id: String,
    on: bool,
) -> Result<openrac_launcher_core::differences::Differences, String> {
    let root = differences_root(&state);
    let home = openrac_launcher_core::saves::data_home();
    let what = format!("{game}/{version}: game fix {id} {}", if on { "on" } else { "off (the original's)" });
    let result = tauri::async_runtime::spawn_blocking(move || {
        openrac_launcher_core::differences::set_fix(root.as_deref(), &home, &game, &version, &id, on)
    })
    .await
    .map_err(|e| e.to_string())?;
    if result.is_ok() {
        launcher_log::line(&what);
    }
    result
}

// ---- Logs and crash reports (core/src/reports.rs) ----

/// A game's data folder, where its logs and crash reports are.
fn game_data(state: &State<'_, AppState>, game: &str) -> Result<PathBuf, String> {
    if game.is_empty() || game.contains(['/', '\\', '.']) {
        return Err(format!("{game} is not a game"));
    }
    let install = state.config.lock().unwrap().install_dir.clone().ok_or("no install folder (Settings)")?;
    Ok(install::data_dir(&install, game))
}

#[tauri::command]
async fn reports_list(state: State<'_, AppState>, game: String) -> Result<Vec<ReportEntry>, String> {
    let data = game_data(&state, &game)?;
    tauri::async_runtime::spawn_blocking(move || reports::list(&data)).await.map_err(|e| e.to_string())
}

/// Deletes one log or crash report of a game from its data folder (core checks the name).
#[tauri::command]
async fn report_delete(state: State<'_, AppState>, game: String, file: String) -> Result<(), String> {
    let data = game_data(&state, &game)?;
    let name = file.clone();
    let g = game.clone();
    tauri::async_runtime::spawn_blocking(move || reports::delete(&data, &g, &file))
        .await
        .map_err(|e| e.to_string())??;
    launcher_log::line(&format!("deleted a report of {game}: {name}"));
    Ok(())
}

/// Deletes every log and crash report of a game; returns how many files went.
#[tauri::command]
async fn reports_delete_all(state: State<'_, AppState>, game: String) -> Result<usize, String> {
    let data = game_data(&state, &game)?;
    let g = game.clone();
    let count = tauri::async_runtime::spawn_blocking(move || reports::delete_all(&data, &g))
        .await
        .map_err(|e| e.to_string())??;
    launcher_log::line(&format!("deleted all {count} reports of {game}"));
    Ok(count)
}

/// The newest crash report written since `since` (seconds since 1970): after the game stopped.
#[tauri::command]
async fn latest_crash(state: State<'_, AppState>, game: String, since: u64) -> Result<Option<ReportEntry>, String> {
    let data = game_data(&state, &game)?;
    let found = tauri::async_runtime::spawn_blocking(move || reports::latest_crash(&data, since))
        .await
        .map_err(|e| e.to_string())?;
    if let Some(e) = &found {
        launcher_log::line(&format!(
            "the game stopped unexpectedly: {} ({})",
            e.file,
            e.reason.as_deref().unwrap_or("?")
        ));
    }
    Ok(found)
}

#[tauri::command]
async fn report_text(app: AppHandle, state: State<'_, AppState>, game: String, file: String) -> Result<String, String> {
    let data = game_data(&state, &game)?;
    let version = app.package_info().version.to_string();
    tauri::async_runtime::spawn_blocking(move || reports::copy_text(&data, &file, &version))
        .await
        .map_err(|e| e.to_string())?
}

/// Saves the .zip for `file` in the player's Downloads folder and shows it there; returns its path.
#[tauri::command]
async fn report_save(app: AppHandle, state: State<'_, AppState>, game: String, file: String) -> Result<String, String> {
    let data = game_data(&state, &game)?;
    let version = app.package_info().version.to_string();
    // Android: made in the launcher's cache, then saved where the player chooses.
    let folder = if cfg!(target_os = "android") {
        let cache = app.path().app_cache_dir().map_err(|e| e.to_string())?.join("reports");
        std::fs::create_dir_all(&cache).map_err(|e| e.to_string())?;
        cache
    } else {
        reports::save_folder().ok_or("no folder to save in")?
    };
    let name = reports::bundle_name(&file);
    let out = folder.join(&name);
    let target = out.clone();
    tauri::async_runtime::spawn_blocking(move || {
        reports::bundle(&data, &file, launcher_log::path().as_deref(), &version, &target)
    })
    .await
    .map_err(|e| e.to_string())??;
    if cfg!(target_os = "android") {
        let saved = android::save_as(&app, &out, &name).await;
        let _ = std::fs::remove_file(&out);
        return match saved? {
            Some(name) => {
                launcher_log::line(&format!("saved a report: {name}"));
                Ok(name)
            }
            None => Ok(String::new()), // backed out: nothing to say
        };
    }
    launcher_log::line(&format!("saved a report: {}", out.display()));
    let _ = app.opener().reveal_item_in_dir(&out);
    Ok(reports::tidy(&out.to_string_lossy(), reports::home().as_deref()))
}

/// Opens the folder a game's logs are in (its crash reports' folder when `crashes`).
#[tauri::command]
fn open_reports_folder(app: AppHandle, state: State<'_, AppState>, game: String, crashes: bool) -> Result<(), String> {
    let data = game_data(&state, &game)?;
    let dir = if crashes && data.join("crashes").is_dir() { data.join("crashes") } else { data };
    if !dir.is_dir() {
        return Err("nothing has been logged for this game yet".into());
    }
    open_path_with_system(&app, &dir.to_string_lossy())
}

#[tauri::command]
fn set_discord_status(
    state: State<'_, AppState>,
    status: openrac_launcher_core::discord::DiscordStatus,
) -> Result<(), String> {
    let config = state.config.lock().unwrap().clone();
    let discord = state.discord.clone();
    tauri::async_runtime::spawn(async move {
        let _ = tauri::async_runtime::spawn_blocking(move || {
            let mut discord = discord.lock().unwrap();
            if !config.discord_rpc {
                let _ = discord.ipc.clear();
                return;
            }
            let app_start = discord.app_start;
            let _ = discord.ipc.set_status(&status, app_start);
        })
        .await;
    });
    Ok(())
}

#[tauri::command]
fn cancel_job(state: State<'_, AppState>, id: u32) -> Result<(), String> {
    state.jobs.cancel(id)
}

/// Opens a file or folder of the checkout (a README, baserom/) with the
/// system's default app. Paths outside the checkout are refused.
#[tauri::command]
fn open_path(app: AppHandle, state: State<'_, AppState>, path: String) -> Result<(), String> {
    let (root, install) = {
        let c = state.config.lock().unwrap();
        (c.root.clone().ok_or("the OpenRAC folder is not set")?, c.install_dir.clone())
    };
    let rel = path.split('#').next().unwrap_or_default();
    // An installed launcher's own folder is read-only (an AppImage, the macOS app): the folders a
    // checkout keeps under assets/ are the game's, in the per-user install folder.
    let on_copy = state.bundled.as_deref().is_some_and(|copy| detect::on_the_copy(Some(&root), copy));
    if let (true, Some(install)) = (on_copy, install.as_deref()) {
        if let Some(folder) = install::player_folder(install, rel) {
            std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
            return open_path_with_system(&app, &folder.to_string_lossy());
        }
    }
    let root = std::fs::canonicalize(&root).map_err(|e| e.to_string())?;
    let wanted = openrac_launcher_core::under(&root, rel);
    if rel.starts_with("assets/") && !rel.split('/').any(|part| part == "..") && !wanted.exists() {
        // A folder the tools write (the level editor's projects, packed levels) before they first
        // have: opened empty rather than refused.
        std::fs::create_dir_all(&wanted).map_err(|e| format!("{path}: {e}"))?;
    }
    let target = std::fs::canonicalize(wanted).map_err(|e| format!("{path}: {e}"))?;
    if !target.starts_with(&root) {
        return Err(format!("{path} is outside the OpenRAC folder"));
    }
    open_path_with_system(&app, &target.to_string_lossy())
}

/// Opens a web page in the browser: https only.
#[tauri::command]
fn open_url(app: AppHandle, url: String) -> Result<(), String> {
    if !url.starts_with("https://") {
        return Err(format!("not an https address: {url}"));
    }
    open_url_with_system(&app, &url)
}

/// Sets `game` up from the image of the player's own disc, OpenGOAL's way:
/// runs the extractor (extract and validate) as a job whose output streams
/// like any other job's. Returns the job.
#[tauri::command]
fn install_game(app: AppHandle, state: State<'_, AppState>, game: String, image: PathBuf) -> Result<Started, String> {
    let config = state.config.lock().unwrap().clone();
    let install_dir = config.install_dir.clone().ok_or("no install folder (Settings)")?;
    let plan = install::plan_extract(&config, &install_dir, &game, &image)?;
    // A developer's other disc set up before is kept aside, not replaced (disc_choice.rs).
    if config.developer {
        let catalog = openrac_launcher_core::catalog::load(config.root()?)?;
        disc_choice::before_set_up(&config, &catalog, &game, &image)?;
    }
    state.jobs.start(&plan, config.root.as_deref(), move |event| {
        let _ = match &event {
            Event::Output { .. } => app.emit("job-output", &event),
            Event::Exit { .. } => app.emit("job-exit", &event),
        };
    })
}

/// The plain image setting a game up reads: an .iso as it is; a CHD unpacked once into
/// `<install>/discs/<name>.iso` (core/src/chd.rs), with `disc-copy` events as it goes.
#[tauri::command]
async fn prepare_disc(app: AppHandle, state: State<'_, AppState>, image: PathBuf) -> Result<PathBuf, String> {
    use openrac_launcher_core::chd;
    if !chd::is_chd(&image) {
        return Ok(image);
    }
    let install_dir = state.config.lock().unwrap().install_dir.clone().ok_or("no install folder (Settings)")?;
    let name = image.file_name().map(|n| n.to_string_lossy().into_owned()).unwrap_or_default();
    let dest = install_dir.join("discs").join(chd::iso_name(&name));
    let unpacked = dest.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let file = std::fs::File::open(&image).map_err(|e| format!("{}: {e}", image.display()))?;
        chd::to_iso(file, &unpacked, |copied, total| {
            let _ = app.emit("disc-copy", android::DiscCopy { name: name.clone(), copied, total });
        })
    })
    .await
    .map_err(|e| e.to_string())?
    .map_err(|e| format!("cannot unpack the CHD image: {e}"))?;
    Ok(dest)
}

/// The games folder looked through: the games OpenRAC knows among its images, and, for a
/// developer, the games with more than one of their discs there (disc_choice.rs).
#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct DiscScan {
    found: Vec<FoundDisc>,
    choices: Vec<DiscChoice>,
}

fn scan(config: &Config) -> Result<(openrac_launcher_core::catalog::Catalog, Vec<FoundDisc>), String> {
    let catalog = openrac_launcher_core::catalog::load(config.root()?)?;
    let found = config.discs_dir.as_deref().map(|dir| discs::scan(dir, &catalog)).unwrap_or_default();
    Ok((catalog, found))
}

/// The games OpenRAC knows among the images in the player's games folder (none when no folder
/// is chosen), and the discs a developer can choose between.
#[tauri::command]
async fn scan_discs(state: State<'_, AppState>) -> Result<DiscScan, String> {
    let config = state.config.lock().unwrap().clone();
    if config.discs_dir.is_none() || config.root.is_none() {
        return Ok(DiscScan { found: Vec::new(), choices: Vec::new() });
    }
    tauri::async_runtime::spawn_blocking(move || {
        let (catalog, found) = scan(&config)?;
        let choices = disc_choice::choices(&config, &catalog, &found);
        Ok(DiscScan { found, choices })
    })
    .await
    .map_err(|e| e.to_string())?
}

/// Developers only: plays `version`'s disc of its game from now on. Its set-up is put in place
/// and the other kept aside (disc_choice.rs); the choice is saved in the settings.
#[tauri::command]
async fn choose_disc(state: State<'_, AppState>, version: String) -> Result<Config, String> {
    let file = state.config_file.clone().ok_or("no per-user config folder")?;
    let mut config = state.config.lock().unwrap().clone();
    let config = tauri::async_runtime::spawn_blocking(move || {
        let (catalog, found) = scan(&config)?;
        disc_choice::choose(&mut config, &catalog, &found, &version)?;
        config::save(&file, &config)?;
        Ok::<_, String>(config)
    })
    .await
    .map_err(|e| e.to_string())??;
    *state.config.lock().unwrap() = config.clone();
    Ok(config)
}

/// Removes what setting `game` up made (OpenGOAL's uninstall); the player's
/// disc image and saves are left alone.
#[tauri::command]
async fn uninstall_game(state: State<'_, AppState>, game: String) -> Result<(), String> {
    let install_dir = state.config.lock().unwrap().install_dir.clone().ok_or("no install folder (Settings)")?;
    tauri::async_runtime::spawn_blocking(move || install::uninstall(&install_dir, &game))
        .await
        .map_err(|e| e.to_string())?
}

#[tauri::command]
async fn sync_progress_from_web(state: State<'_, AppState>) -> Result<Library, String> {
    let config = state.config.lock().unwrap().clone();
    let root = config.root.clone().ok_or("the OpenRAC folder is not set")?;
    let cache = config.progress_cache.clone().ok_or("no per-user folder for the launcher's own files")?;

    // Every project's latest progress from bytematch.dev, with curl (core progress_sync.rs).
    // Offline, or when the site cannot be read, the kept copy is read as it was.
    tauri::async_runtime::spawn_blocking(move || {
        let _ = openrac_launcher_core::progress_sync::sync(&root, &cache, &remote::QuickCurl);
    })
    .await
    .map_err(|e| e.to_string())?;

    tauri::async_runtime::spawn_blocking(move || library::library(&config)).await.map_err(|e| e.to_string())?
}

#[tauri::command]
async fn apply_progress_json(state: State<'_, AppState>, json: String) -> Result<Library, String> {
    let config = state.config.lock().unwrap().clone();
    let root = config.root.clone().ok_or("the OpenRAC folder is not set")?;
    let cache = config.progress_cache.clone().ok_or("no per-user folder for the launcher's own files")?;
    openrac_launcher_core::catalog::update_progress_from_bytematch(&root, &cache, &json, &Default::default())?;
    tauri::async_runtime::spawn_blocking(move || library::library(&config)).await.map_err(|e| e.to_string())?
}

#[tauri::command]
fn inspect_saves(serial: String) -> openrac_launcher_core::saves::GameSaveStatus {
    openrac_launcher_core::saves::inspect_saves(&serial)
}

#[tauri::command]
fn backup_saves(serial: String, note: Option<String>) -> Result<openrac_launcher_core::saves::SaveBackupInfo, String> {
    openrac_launcher_core::saves::backup_saves(&serial, note.as_deref())
}

/// Whether the game runs with a memory card (the name to show when it does), for an edit or a
/// restore: on a desktop from the process list when asked, on Android from the game's activity,
/// asked here before (android.rs, game_holding_card).
async fn card_check(app: &AppHandle) -> Result<impl Fn(&Path, &str) -> Result<Option<String>, String>, String> {
    #[cfg(target_os = "android")]
    let running = android::game_holding_card(app).await?;
    #[cfg(not(target_os = "android"))]
    let _ = app;
    Ok(move |card: &Path, serial: &str| {
        #[cfg(target_os = "android")]
        {
            let _ = (card, serial);
            Ok(running.clone())
        }
        #[cfg(not(target_os = "android"))]
        {
            openrac_launcher_core::save_edit::card_in_use(card, serial)
        }
    })
}

/// Restores a backup of the card; refused while the game runs with it, as an edit is.
#[tauri::command]
async fn restore_backup(app: AppHandle, serial: String, backup_name: String) -> Result<(), String> {
    let in_use = card_check(&app).await?;
    tauri::async_runtime::spawn_blocking(move || {
        let card = openrac_launcher_core::saves::get_memcard_dir(&serial);
        if let Some(game) = in_use(&card, &serial)? {
            return Err(format!("{game} is running with this memory card: close it first"));
        }
        openrac_launcher_core::saves::restore_backup(&serial, &backup_name)
    })
    .await
    .map_err(|e| e.to_string())?
}

/// The skill points' names from the set-up rac1 disc (`<install>/active/rac1/data/iso_data/rac1/
/// disc.iso`; both rac1 discs hold the same names at the same text ids), or none.
fn skill_names(install: Option<PathBuf>) -> Vec<String> {
    install
        .map(|i| install::data_dir(&i, "rac1").join("iso_data").join("rac1").join("disc.iso"))
        .and_then(|iso| openrac_launcher_core::save_edit::skill_point_names(&iso))
        .unwrap_or_default()
}

/// One save slot for the save editor (core/src/save_edit.rs), with the skill points' names.
#[tauri::command]
async fn read_save(
    state: State<'_, AppState>,
    serial: String,
    slot: usize,
) -> Result<openrac_launcher_core::save_edit::SaveView, String> {
    let install = state.config.lock().unwrap().install_dir.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let card = openrac_launcher_core::saves::get_memcard_dir(&serial);
        let mut view = openrac_launcher_core::save_edit::read_save(&card, &serial, slot)?;
        view.skill_names = skill_names(install);
        Ok(view)
    })
    .await
    .map_err(|e| e.to_string())?
}

/// Writes the save editor's change into a slot: refused while the game runs with the card or if
/// the file changed since it was read; the card is backed up first. Raw chunk edits only with the
/// advanced save editor switched on in Settings.
#[tauri::command]
async fn write_save(
    app: AppHandle,
    state: State<'_, AppState>,
    serial: String,
    slot: usize,
    fingerprint: String,
    change: openrac_launcher_core::save_edit::SaveChange,
) -> Result<openrac_launcher_core::save_edit::WriteResult, String> {
    use openrac_launcher_core::save_edit;
    let (advanced, install) = {
        let c = state.config.lock().unwrap();
        (c.save_editor_advanced, c.install_dir.clone())
    };
    if matches!(change, save_edit::SaveChange::Raw { .. }) && !advanced {
        return Err("raw edits need the advanced save editor (Settings)".into());
    }
    let in_use = card_check(&app).await?;
    tauri::async_runtime::spawn_blocking(move || {
        let card = openrac_launcher_core::saves::get_memcard_dir(&serial);
        let mut result = save_edit::write_save(&card, &serial, slot, &fingerprint, &change, || in_use(&card, &serial))?;
        result.view.skill_names = skill_names(install);
        Ok(result)
    })
    .await
    .map_err(|e| e.to_string())?
}

/// A game's overrides folder (`<data>/overrides`), from the install folder in Settings.
fn overrides_dir(state: &State<'_, AppState>, game: &str) -> Result<PathBuf, String> {
    let install = state.config.lock().unwrap().install_dir.clone().ok_or("no install folder (Settings)")?;
    Ok(overrides::dir(&install, game))
}

/// The game's overrides packs, in the order they apply.
#[tauri::command]
fn overrides_list(state: State<'_, AppState>, game: String) -> Result<Vec<PackView>, String> {
    overrides::list(&overrides_dir(&state, &game)?)
}

#[tauri::command]
fn overrides_set_enabled(
    state: State<'_, AppState>,
    game: String,
    id: String,
    enabled: bool,
) -> Result<Vec<PackView>, String> {
    overrides::set_enabled(&overrides_dir(&state, &game)?, &id, enabled)
}

#[tauri::command]
fn overrides_move(state: State<'_, AppState>, game: String, id: String, up: bool) -> Result<Vec<PackView>, String> {
    overrides::move_pack(&overrides_dir(&state, &game)?, &id, up)
}

#[tauri::command]
fn overrides_remove(state: State<'_, AppState>, game: String, id: String) -> Result<Vec<PackView>, String> {
    overrides::remove(&overrides_dir(&state, &game)?, &id)
}

/// A game's native mods' folder (`<data>/plugins`, docs/port/MODS.md), from the install folder in
/// Settings.
fn plugins_dir(state: &State<'_, AppState>, game: &str) -> Result<PathBuf, String> {
    let install = state.config.lock().unwrap().install_dir.clone().ok_or("no install folder (Settings)")?;
    Ok(plugins::dir(&install, game))
}

/// Runs a change to a game's native mods away from the window's thread.
async fn on_plugins<T: Send + 'static>(
    state: State<'_, AppState>,
    game: &str,
    work: impl FnOnce(&Path) -> Result<T, String> + Send + 'static,
) -> Result<T, String> {
    let dir = plugins_dir(&state, game)?;
    tauri::async_runtime::spawn_blocking(move || work(&dir)).await.map_err(|e| e.to_string())?
}

/// The game's native mods in load order, as they stand for its disc `version` (`pal`).
#[tauri::command]
async fn plugins_list(state: State<'_, AppState>, game: String, version: String) -> Result<Vec<PluginView>, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| plugins::list(dir, &g, &version)).await
}

#[tauri::command]
async fn plugins_set_enabled(
    state: State<'_, AppState>,
    game: String,
    version: String,
    id: String,
    enabled: bool,
) -> Result<Vec<PluginView>, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| plugins::set_enabled(dir, &g, &version, &id, enabled)).await
}

#[tauri::command]
async fn plugins_move(
    state: State<'_, AppState>,
    game: String,
    version: String,
    id: String,
    up: bool,
) -> Result<Vec<PluginView>, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| plugins::move_mod(dir, &g, &version, &id, up)).await
}

#[tauri::command]
async fn plugins_set_setting(
    state: State<'_, AppState>,
    game: String,
    version: String,
    id: String,
    key: String,
    value: serde_json::Value,
) -> Result<Vec<PluginView>, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| plugins::set_setting(dir, &g, &version, &id, &key, value)).await
}

#[tauri::command]
async fn plugins_remove(
    state: State<'_, AppState>,
    game: String,
    version: String,
    id: String,
) -> Result<Vec<PluginView>, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| {
        let left = plugins::remove(dir, &g, &version, &id)?;
        remote::forget(dir, &id)?;
        Ok(left)
    })
    .await
}

/// Adds a mod from a .zip or a folder: a native mod to the store, or the answer that it is a data
/// pack (the page then adds it to the overrides with `overrides-add`).
#[tauri::command]
async fn plugins_install(
    state: State<'_, AppState>,
    game: String,
    version: String,
    path: PathBuf,
) -> Result<Installed, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| plugins::install(dir, &g, &version, &path)).await
}

/// The mods a chosen folder holds (one mod, or a repository's checkout of several).
#[tauri::command]
fn plugins_mod_folders(path: PathBuf) -> Result<Vec<PathBuf>, String> {
    plugins::mod_folders(&path)
}

/// What the port did with the game's mods at its last start (`status.json`), if it wrote it.
/// A game's mods from a repository's address (docs/port/MODS.md, section 10): what it holds,
/// installing one, newer versions, keeping one at its version. Through curl, no git needed; on
/// Android, which has no curl, through the Kotlin side's download (android::KotlinFetch).
fn with_remote<T>(
    app: &AppHandle,
    dir: &Path,
    game: &str,
    version: &str,
    work: impl FnOnce(&remote::Context) -> T,
) -> T {
    #[cfg(target_os = "android")]
    let fetch = android::KotlinFetch { app: app.clone() };
    #[cfg(not(target_os = "android"))]
    let fetch = {
        let _ = app;
        remote::Curl
    };
    work(&remote::Context {
        dir,
        game,
        version,
        platform: plugins::platform_key(),
        fetch: &fetch,
        hosts: remote::Hosts::default(),
    })
}

#[tauri::command]
async fn plugins_preview(
    app: AppHandle,
    state: State<'_, AppState>,
    game: String,
    version: String,
    url: String,
) -> Result<remote::Preview, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| with_remote(&app, dir, &g, &version, |c| remote::preview(c, &url))).await
}

#[tauri::command]
async fn plugins_install_remote(
    app: AppHandle,
    state: State<'_, AppState>,
    game: String,
    version: String,
    url: String,
    id: String,
) -> Result<remote::RemoteInstalled, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| with_remote(&app, dir, &g, &version, |c| remote::install(c, &url, &id))).await
}

#[derive(serde::Serialize)]
#[serde(rename_all = "camelCase")]
struct UpdateCheck {
    updates: Vec<remote::Update>,
    errors: Vec<String>,
}

#[tauri::command]
async fn plugins_check_updates(
    app: AppHandle,
    state: State<'_, AppState>,
    game: String,
    version: String,
) -> Result<UpdateCheck, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| {
        let (updates, errors) = with_remote(&app, dir, &g, &version, remote::check_updates);
        Ok(UpdateCheck { updates, errors })
    })
    .await
}

#[tauri::command]
async fn plugins_update(
    app: AppHandle,
    state: State<'_, AppState>,
    game: String,
    version: String,
    id: String,
) -> Result<remote::RemoteInstalled, String> {
    let g = game.clone();
    on_plugins(state, &game, move |dir| with_remote(&app, dir, &g, &version, |c| remote::update(c, &id))).await
}

#[tauri::command]
async fn plugins_sources(state: State<'_, AppState>, game: String) -> Result<remote::Sources, String> {
    on_plugins(state, &game, |dir| Ok(remote::read_sources(dir))).await
}

#[tauri::command]
async fn plugins_set_pin(
    state: State<'_, AppState>,
    game: String,
    id: String,
    pin: bool,
) -> Result<remote::Sources, String> {
    on_plugins(state, &game, move |dir| remote::set_pin(dir, &id, pin)).await
}

#[tauri::command]
async fn plugins_status(state: State<'_, AppState>, game: String) -> Result<Option<plugins::Status>, String> {
    on_plugins(state, &game, |dir| Ok(plugins::read_status(dir))).await
}

/// Opens a folder inside a game's set-up folder (an export, the overrides): folders only, and
/// only there, so the page cannot open anything else with it.
#[tauri::command]
fn open_game_folder(app: AppHandle, state: State<'_, AppState>, game: String, path: String) -> Result<(), String> {
    let install = state.config.lock().unwrap().install_dir.clone().ok_or("no install folder (Settings)")?;
    let data = install::data_dir(&install, &game);
    let _ = std::fs::create_dir_all(data.join("exports"));
    let data = std::fs::canonicalize(&data).map_err(|e| format!("{}: {e}", data.display()))?;
    let wanted = Path::new(&path);
    if !wanted.exists()
        && !wanted.components().any(|c| c == std::path::Component::ParentDir)
        && detect::plain(wanted).starts_with(detect::plain(&data))
    {
        // A folder of the set-up a tool writes (the level editor's projects, packed levels)
        // before it first has: opened empty rather than refused.
        std::fs::create_dir_all(wanted).map_err(|e| format!("{path}: {e}"))?;
    }
    let target = std::fs::canonicalize(wanted).map_err(|e| format!("{path}: {e}"))?;
    if !target.starts_with(&data) || !target.is_dir() {
        return Err(format!("{path} is not a folder of the game's set-up"));
    }
    open_path_with_system(&app, &target.to_string_lossy())
}

/// Opens a folder a job wrote (a `job-exit`'s `outputs`, a `Started`'s `output`) with the system's
/// file manager; a file opens its folder. Only the folders the launcher's tools write: the
/// checkout's assets/ and build/, and the per-user install folder with the games' set-up folders.
#[tauri::command]
fn open_output(app: AppHandle, state: State<'_, AppState>, path: String) -> Result<(), String> {
    let (root, install) = {
        let c = state.config.lock().unwrap();
        (c.root.clone(), c.install_dir.clone())
    };
    let target = std::fs::canonicalize(&path).map_err(|e| format!("{path}: {e}"))?;
    let target = if target.is_dir() { target } else { target.parent().map(Path::to_path_buf).unwrap_or(target) };
    let mut allowed = Vec::new();
    if let Some(root) = root.and_then(|r| std::fs::canonicalize(r).ok()) {
        allowed.push(root.join("assets"));
        allowed.push(root.join("build"));
    }
    if let Some(install) = install.and_then(|i| std::fs::canonicalize(i).ok()) {
        allowed.push(install);
    }
    if !allowed.iter().any(|folder| target.starts_with(folder)) {
        return Err(format!("{path} is not a folder the launcher's tools write"));
    }
    open_path_with_system(&app, &target.to_string_lossy())
}

/// Opens one level's Godot project in the Godot editor (the version's `editor-open` action, with
/// its checks: Godot set up, the level extracted), for a job's result or a level's row.
#[tauri::command]
fn open_level_in_godot(state: State<'_, AppState>, version: String, level: u32) -> Result<Started, String> {
    let config = state.config.lock().unwrap().clone();
    let inputs = openrac_launcher_core::actions::Inputs { level: Some(level), path: None, option: None };
    let plan = library::plan(&config, &Scope::Version(version), "editor-open", &inputs)?;
    launcher_log::line(&format!("run: {}", openrac_launcher_core::jobs::command_line(&plan)));
    state.jobs.start(&plan, config.root.as_deref(), |_| {})
}

#[tauri::command]
fn open_saves_folder(app: AppHandle, serial: String) -> Result<(), String> {
    let dir = openrac_launcher_core::saves::get_memcard_dir(&serial);
    if !dir.exists() {
        let _ = std::fs::create_dir_all(&dir);
    }
    open_path_with_system(&app, &dir.to_string_lossy())
}

/// The launcher's icons in each size the desktop's icon theme takes (core desktop.rs).
#[cfg(target_os = "linux")]
const LAUNCHER_ICONS: &[openrac_launcher_core::desktop::Icon] = &[
    ("32x32", include_bytes!("../icons/32x32.png")),
    ("64x64", include_bytes!("../icons/64x64.png")),
    ("128x128", include_bytes!("../icons/128x128.png")),
    ("256x256", include_bytes!("../icons/128x128@2x.png")),
    ("512x512", include_bytes!("../icons/icon.png")),
];

/// What an AppImage's AppRun hands over: its own PYTHONHOME, its own paths,
/// its own libraries. They are of no use to anything the launcher starts (a
/// Qt program told to look for its platform plugin in the AppImage's
/// directories, which hold no Qt, stops with "Could not find the Qt platform
/// plugin"), so they go. The launcher's own webview keeps the AppImage's
/// libraries; everything else gets the system's (core/src/appimage.rs).
#[cfg(target_os = "linux")]
fn clean_appimage_env() {
    openrac_launcher_core::appimage::clean_process_env();
}

/// What WebKitGTK needs turned off on this system, before GTK starts: its
/// DMA-BUF renderer under Wayland with NVIDIA's own driver, and its
/// accelerated compositing mode on a Steam Deck, where the window opens empty
/// unless the launcher is started with WEBKIT_DISABLE_COMPOSITING_MODE=1. A
/// value the player set is left as it is (core/src/webkit.rs).
#[cfg(target_os = "linux")]
fn tune_webkit() {
    for (name, value) in openrac_launcher_core::webkit::webkit_env() {
        if std::env::var_os(name).is_none() {
            std::env::set_var(name, value);
        }
    }
}

/// GTK 3's Wayland backend aborts on startup with an assertion failure in
/// _gdk_wayland_display_get_scaled_cursor_theme (`display_wayland->cursor_theme_name`) if no
/// cursor theme name can be resolved from GSettings (common in AppImages on GNOME Wayland or
/// when cursor themes are misconfigured). Set safe fallbacks for XCURSOR_THEME and XCURSOR_SIZE.
#[cfg(target_os = "linux")]
fn ensure_wayland_cursor_fallback() {
    let wayland = std::env::var_os("WAYLAND_DISPLAY").is_some();
    if wayland {
        if std::env::var("XCURSOR_THEME").map_or(true, |v| v.trim().is_empty()) {
            std::env::set_var("XCURSOR_THEME", "default");
        }
        if std::env::var("XCURSOR_SIZE").map_or(true, |v| v.trim().is_empty()) {
            std::env::set_var("XCURSOR_SIZE", "24");
        }
    }
}

/// Opens a path or a folder with the system's own program.
///
/// On Linux the program that opens it (xdg-open, and on KDE the Qt program it
/// calls) would get the AppImage's environment, in which a Qt program cannot
/// find its platform plugin and says so. So xdg-open is started here, with the
/// system's environment instead of the AppImage's.
#[cfg_attr(target_os = "linux", allow(unused_variables))]
fn open_path_with_system(app: &AppHandle, path: &str) -> Result<(), String> {
    // Android's own file manager shows no app's private folders: the Files app, through OpenRAC's
    // place in it, does.
    if Platform::current() == Platform::Android {
        return android::show_folder(app, Some(path));
    }
    #[cfg(target_os = "linux")]
    {
        open_with_system(path)
    }
    #[cfg(not(target_os = "linux"))]
    {
        app.opener().open_path(path, None::<&str>).map_err(|e| e.to_string())
    }
}

/// Opens a web page in the system's own browser, the way
/// [`open_path_with_system`] opens a folder.
#[cfg_attr(target_os = "linux", allow(unused_variables))]
fn open_url_with_system(app: &AppHandle, url: &str) -> Result<(), String> {
    #[cfg(target_os = "linux")]
    {
        open_with_system(url)
    }
    #[cfg(not(target_os = "linux"))]
    {
        app.opener().open_url(url, None::<&str>).map_err(|e| e.to_string())
    }
}

/// Starts the system's own opener for a path or a link, in the environment the
/// system expects: the AppImage's libraries are ours, not the system's.
#[cfg(target_os = "linux")]
fn open_with_system(target: &str) -> Result<(), String> {
    let mut cmd = std::process::Command::new("xdg-open");
    cmd.arg(target);
    openrac_launcher_core::appimage::sanitize(&mut cmd);
    let mut child = cmd.spawn().map_err(|e| format!("cannot start xdg-open: {e}"))?;
    // Reaped when it ends (xdg-open hands over to the desktop's program and exits): a child
    // that is not waited for stays a zombie until the launcher exits.
    std::thread::spawn(move || child.wait());
    Ok(())
}

/// On a screen too small for the window (a 1080p screen at 150 % or 200 %), the window opens at
/// what the screen has room for and can be made that small (core/src/window.rs).
#[cfg(desktop)]
fn fit_to_screen(window: &tauri::WebviewWindow) {
    let (Ok(Some(monitor)), Ok(size)) = (window.current_monitor(), window.inner_size()) else { return };
    let scale = monitor.scale_factor();
    let area = monitor.work_area().size.to_logical::<f64>(scale);
    let size = size.to_logical::<f64>(scale);
    let floor =
        window.config().app.windows.first().and_then(|w| w.min_width.zip(w.min_height)).unwrap_or((960.0, 560.0));
    if let Some(((width, height), (min_width, min_height))) =
        openrac_launcher_core::window::fit((size.width, size.height), floor, (area.width, area.height))
    {
        let _ = window.set_min_size(Some(tauri::LogicalSize::new(min_width, min_height)));
        let _ = window.set_size(tauri::LogicalSize::new(width, height));
        let _ = window.center();
    }
}

// On Android and iOS the launcher is a library the app's activity starts through this entry point.
// It starts in steps, in this order: the process's environment, the plugins, the app's setup once
// it exists, then the commands the page calls (docs/ARCHITECTURE.md lists them).
#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    prepare_environment();
    with_plugins(tauri::Builder::default())
        .setup(setup)
        .invoke_handler(tauri::generate_handler![
            app_info,
            relaunch_app,
            get_config,
            save_config,
            detect,
            check_root,
            check_tool,
            adopt_godot,
            library,
            run_action,
            cancel_job,
            open_path,
            open_url,
            install_game,
            prepare_disc,
            uninstall_game,
            scan_discs,
            choose_disc,
            sync_progress_from_web,
            apply_progress_json,
            set_discord_status,
            inspect_saves,
            backup_saves,
            restore_backup,
            open_saves_folder,
            read_save,
            write_save,
            overrides_list,
            overrides_set_enabled,
            overrides_move,
            overrides_remove,
            plugins_list,
            enhanced_view,
            enhanced_set_enabled,
            enhanced_set_setting,
            plugins_set_enabled,
            plugins_move,
            plugins_set_setting,
            plugins_remove,
            plugins_install,
            plugins_mod_folders,
            plugins_status,
            plugins_preview,
            plugins_install_remote,
            plugins_check_updates,
            plugins_update,
            plugins_sources,
            plugins_set_pin,
            open_game_folder,
            open_output,
            open_level_in_godot,
            reports_list,
            latest_crash,
            report_text,
            report_save,
            report_delete,
            reports_delete_all,
            open_reports_folder,
            differences,
            set_game_fix,
            android::pick_disc,
            android::pick_zip,
            android::show_files,
            android::gpu_drivers,
            android::gpu_driver_import,
            android::gpu_driver_select,
            android::gpu_driver_remove,
            android::gpu_renderer_select,
            android::background_status,
            android::tools_ready,
            launcher_update_check,
            android::update_check,
            android::update_install,
            android::game_status,
            android::game_window,
        ])
        .run(tauri::generate_context!())
        .expect("the launcher failed to start");
}

/// What the process's environment must say before GTK and WebKit start, on Linux: the AppImage's
/// variables gone, WebKit's renderers tuned to the system, a cursor theme for Wayland.
fn prepare_environment() {
    #[cfg(target_os = "linux")]
    {
        clean_appimage_env();
        tune_webkit();
        ensure_wayland_cursor_fallback();
    }
}

/// The launcher's plugins, registered in this order: one instance, the file dialogs, the system's
/// opener, the launcher's own updates on a desktop, and on Android the app's own plugin
/// (android.rs).
fn with_plugins(builder: tauri::Builder<tauri::Wry>) -> tauri::Builder<tauri::Wry> {
    // A second start while the launcher is open shows the open one's window and goes. Registered
    // first, as the plugin asks. Without it, on Linux the GTK application id (enableGTKAppId)
    // handed the second start to the first launcher, which set its window up again and stopped:
    // "a webview with label `main` already exists".
    #[cfg(desktop)]
    let builder = builder.plugin(tauri_plugin_single_instance::init(|app, _args, _cwd| {
        if let Some(window) = app.get_webview_window("main") {
            let _ = window.unminimize();
            let _ = window.show();
            let _ = window.set_focus();
        }
    }));
    let builder = builder.plugin(tauri_plugin_dialog::init()).plugin(tauri_plugin_opener::init());
    #[cfg(desktop)]
    let builder = builder.plugin(
        tauri_plugin_updater::Builder::new()
            // A build of a branch carries the release's plain version and its run (LAUNCHER_BUILD):
            // compared as X.Y.Z-dev.<run>, so each build of the branch updates the one before.
            .default_version_comparator(|current, release| {
                let current = openrac_launcher_core::update::effective_version(&current.to_string(), LAUNCHER_BUILD);
                openrac_launcher_core::update::is_newer(&current, &release.version.to_string())
            })
            .build(),
    );
    #[cfg(target_os = "android")]
    let builder = builder.plugin(android::init());
    builder
}

/// What the launcher sets up once the app exists, before the page asks for anything, in this
/// order: the desktop's entries and the window, launcher.json, the install folder, the copy of
/// OpenRAC it carries, the folder and Python it works with, Discord, its log; then the state every
/// command shares ([`AppState`]).
fn setup(app: &mut tauri::App) -> Result<(), Box<dyn std::error::Error>> {
    set_up_desktop(app);
    let config_file = app.path().app_config_dir().ok().map(|dir| dir.join("launcher.json"));
    let mut config = config_file.as_deref().map(config::load).unwrap_or_default();
    config.progress_cache = progress_cache(config_file.as_deref());
    settle_install_dir(app, &mut config, config_file.as_deref());
    let bundled = bundled_copy(app, &mut config);
    settle_root(&mut config, bundled.as_deref());
    let discord = start_discord(&config);
    start_log(app, config_file.as_deref());
    settle_data_home(None, &config);
    app.manage(AppState { config: Mutex::new(config), config_file, jobs: Jobs::default(), discord, bundled });
    Ok(())
}

/// The menu entry and icons the desktop finds the launcher by, and Discord's socket where a
/// Flatpak Discord hides it, on Linux (core desktop.rs, discord.rs); the window's icon and its size
/// on the screen, on a desktop.
#[cfg_attr(mobile, allow(unused_variables))]
fn set_up_desktop(app: &tauri::App) {
    #[cfg(target_os = "linux")]
    {
        openrac_launcher_core::desktop::ensure_launcher(LAUNCHER_ICONS);
        openrac_launcher_core::discord::link_flatpak_socket();
    }

    #[cfg(desktop)]
    if let Some(window) = app.get_webview_window("main") {
        if let Some(icon) = app.default_window_icon() {
            let _ = window.set_icon(icon.clone());
        }
        fit_to_screen(&window);
    }
}

/// The install folder: ~/OpenRAC, made at first start, with what older launchers set up in the
/// data folder moved into it (core install.rs). Android keeps the games' set-up, the saves and the
/// settings in the app's own folders, the ones the Files app shows (FilesProvider.kt), and asks
/// nothing at the first start (the Games page asks for the disc): a set-up from an earlier version
/// (in the launcher's data folder, or OpenRAC/ in the shared storage) moves into place.
fn settle_install_dir(app: &tauri::App, config: &mut Config, config_file: Option<&Path>) {
    let home_dir = app.path().home_dir().ok().or_else(detect::home_dir);
    let local_data = app.path().app_local_data_dir().ok();
    if Platform::current() != Platform::Android {
        install::settle_install_dir(config, home_dir.as_deref(), local_data.as_deref(), config_file);
        return;
    }
    if let Some(games) = android::games_dir() {
        let earlier: Vec<PathBuf> = [config.install_dir.clone(), local_data.clone()]
            .into_iter()
            .chain([home_dir.as_ref().map(|home| home.join("OpenRAC"))])
            .flatten()
            .collect();
        config.install_dir = Some(install::settle_games_dir(&games, &earlier));
    } else if config.install_dir.is_none() {
        config.install_dir = local_data;
    }
    config.setup_complete = true;
    if let Some(file) = config_file {
        let _ = config::save(file, config);
    }
}

/// The copy of OpenRAC the launcher carries: what a player plays from (detect::settle).
/// A copy whose Python does not run here is not for this machine (an Intel Mac has the
/// Apple silicon one): the launcher then works on a checkout, as it did.
/// On Android the copy is unpacked from the APK into the app's files (OpenracSetup.kt),
/// maybe still now: the page waits for it (tools_ready) before it reads anything there.
/// `tauri dev` works on its checkout: a copy in its resources is a leftover of an
/// installer built there, as old as that build (detect::leave_copy). OPENRAC_STANDALONE
/// set makes a debug build an installed launcher, to try the copy.
fn bundled_copy(app: &tauri::App, config: &mut Config) -> Option<PathBuf> {
    let resources_copy = app.path().resource_dir().ok().and_then(|dir| detect::bundled_root(&dir));
    let contributor = cfg!(debug_assertions) && std::env::var_os("OPENRAC_STANDALONE").is_none();
    if Platform::current() == Platform::Android {
        android::bundled_root()
    } else if contributor {
        if let Some(copy) = &resources_copy {
            detect::leave_copy(config, copy, &start_folders());
        }
        None
    } else {
        resources_copy.filter(|dir| {
            detect::bundled_python(dir).is_some_and(|python| detect::check_tool(Tool::Python, &python).ok)
        })
    }
}

/// The folder and Python the launcher works with (detect::settle), and on Linux the games' entries
/// and icons in that folder, so the taskbar shows a game's own icon (core desktop.rs).
fn settle_root(config: &mut Config, bundled: Option<&Path>) {
    // Android's picker gives no paths: the disc images the player picks are copied into
    // one folder of the app's, which is then the games folder.
    if config.discs_dir.is_none() && Platform::current() == Platform::Android {
        config.discs_dir = android::discs_dir();
    }
    detect::settle(config, bundled);

    #[cfg(target_os = "linux")]
    if let Some(root) = &config.root {
        openrac_launcher_core::desktop::ensure_games(root);
    }
}

/// Discord's connection, shared by the commands that change the status (`set_discord_status`,
/// `run_action`). Discord can take seconds to answer: with the setting on, it is connected and
/// shown idle without holding up the window.
fn start_discord(config: &Config) -> Arc<Mutex<DiscordState>> {
    let client_id = config
        .discord_client_id
        .clone()
        .unwrap_or_else(|| openrac_launcher_core::discord::DEFAULT_CLIENT_ID.to_string());
    let ipc = openrac_launcher_core::discord::DiscordIpc::new(client_id);
    let app_start = openrac_launcher_core::discord::now_sec();
    let discord = Arc::new(Mutex::new(DiscordState { ipc, app_start }));
    if config.discord_rpc {
        let discord = discord.clone();
        tauri::async_runtime::spawn_blocking(move || {
            let mut discord = discord.lock().unwrap();
            let _ = discord.ipc.set_status(&openrac_launcher_core::discord::DiscordStatus::Idle, app_start);
        });
    }
    discord
}

/// The launcher's own log, beside launcher.json (core launcher_log.rs), opened with the version and
/// the platform it started on.
fn start_log(app: &tauri::App, config_file: Option<&Path>) {
    if let Some(dir) = config_file.and_then(Path::parent) {
        launcher_log::init(dir);
        launcher_log::line(&format!(
            "the launcher {} started on {:?}",
            app.package_info().version,
            Platform::current()
        ));
    }
}
