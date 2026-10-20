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

/// Where the launcher looks for its updates: CHANNEL (the Settings page's, before it is saved), else
/// the one the player chose, else the one it was built from.
fn chosen_update_source<R: tauri::Runtime>(
    app: &tauri::AppHandle<R>,
    channel: Option<openrac_launcher_core::update::Channel>,
) -> openrac_launcher_core::update::UpdateSource {
    let built = update_source(app);
    let chosen =
        channel.or_else(|| app.try_state::<AppState>().and_then(|state| state.config.lock().unwrap().update_channel));
    built.on(chosen.unwrap_or(built.kind()))
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
) -> Result<Option<serde_json::Value>, String> {
    #[cfg(desktop)]
    {
        use tauri_plugin_updater::UpdaterExt;
        let source = chosen_update_source(webview.app_handle(), channel);
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
        let _ = (webview, channel);
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

#[tauri::command]
fn save_config(state: State<'_, AppState>, mut config: Config) -> Result<Config, String> {
    let file = state.config_file.as_deref().ok_or("no per-user config folder")?;
    // Developer tools switched off: back to the carried copy (detect::settle).
    detect::settle(&mut config, state.bundled.as_deref());
    config::save(file, &config)?;
    *state.config.lock().unwrap() = config.clone();

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
    let exe = std::env::current_exe().ok().and_then(|p| p.parent().map(Path::to_path_buf));
    exe.into_iter().chain(std::env::current_dir().ok()).collect()
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
    let is_play = if let Scope::Version(_) = &scope {
        if let Some(root) = &config.root {
            if let Ok(file) = openrac_launcher_core::actions::load(root) {
                file.find(&scope, &id).map(|a| a.kind == openrac_launcher_core::actions::Kind::Play).unwrap_or(false)
            } else {
                false
            }
        } else {
            false
        }
    } else {
        false
    };

    if is_play && config.discord_rpc {
        if let Some(root) = &config.root {
            if let Ok(catalog) = openrac_launcher_core::catalog::load(root) {
                if let Scope::Version(key) = &scope {
                    if let Some(v) = catalog.version(key) {
                        let discord = state.discord.clone();
                        let play_status = openrac_launcher_core::discord::DiscordStatus::PlayingGame {
                            title: v.title.clone(),
                            region: v.region.clone(),
                            game_id: v.game.clone(),
                            start_time: Some(openrac_launcher_core::discord::now_sec()),
                        };
                        tauri::async_runtime::spawn_blocking(move || {
                            let mut discord = discord.lock().unwrap();
                            let app_start = discord.app_start;
                            let _ = discord.ipc.set_status(&play_status, app_start);
                        });
                    }
                }
            }
        }
    }

    // The game id whose planet names the play job's log gives (Discord).
    let play_game = match (&scope, is_play, &config.root) {
        (Scope::Version(key), true, Some(root)) => {
            openrac_launcher_core::catalog::load(root).ok().and_then(|c| c.version(key).map(|v| v.game.clone()))
        }
        _ => None,
    };
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
                if let Some(game) =
                    play_game.clone().filter(|_| line.contains("now playing level") || line.contains("] bolts "))
                {
                    if let Some(st) = app_handle.try_state::<AppState>() {
                        if st.config.lock().unwrap().discord_rpc {
                            let discord = st.discord.clone();
                            let line = line.clone();
                            tauri::async_runtime::spawn_blocking(move || {
                                let _ = discord.lock().unwrap().ipc.on_game_log(&game, &line);
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
                                let _ = discord.ipc.set_planet(None);
                                let _ = discord
                                    .ipc
                                    .set_status(&openrac_launcher_core::discord::DiscordStatus::Idle, app_start);
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
    app.opener().open_path(dir.to_string_lossy(), None::<&str>).map_err(|e| e.to_string())
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
    let root = state.config.lock().unwrap().root.clone().ok_or("the OpenRAC folder is not set")?;
    let root = std::fs::canonicalize(&root).map_err(|e| e.to_string())?;
    let rel = path.split('#').next().unwrap_or_default();
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
    app.opener().open_path(target.to_string_lossy(), None::<&str>).map_err(|e| e.to_string())
}

/// Opens a web page in the browser: https only.
#[tauri::command]
fn open_url(app: AppHandle, url: String) -> Result<(), String> {
    if !url.starts_with("https://") {
        return Err(format!("not an https address: {url}"));
    }
    app.opener().open_url(url, None::<&str>).map_err(|e| e.to_string())
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

    // Every project's latest progress from bytematch.dev, and each game's project JSON for its
    // functional C, with curl (on Linux, macOS and Windows 10+). Offline, the kept copy is read.
    let root_for_fetch = root.clone();
    tauri::async_runtime::spawn_blocking(move || {
        use openrac_launcher_core::catalog;
        // detect::quiet: each curl opened a console window of its own on Windows, at every start.
        let fetch = |url: &str| -> Option<String> {
            let out = detect::quiet(Path::new("curl"))
                .args(["-s", "-f", "--connect-timeout", "4", "--max-time", "10", url])
                .stdin(std::process::Stdio::null())
                .output()
                .ok()?;
            if out.status.success() {
                String::from_utf8(out.stdout).ok()
            } else {
                None
            }
        };
        let Some(list) = fetch(catalog::BYTEMATCH_PROJECTS) else { return };
        let mut jsons = std::collections::BTreeMap::new();
        for (_key, slug) in catalog::bytematch_projects_for(&root_for_fetch, &list).unwrap_or_default() {
            if let Some(text) = fetch(&catalog::bytematch_project_url(&slug)) {
                jsons.insert(slug, text);
            }
        }
        let _ = catalog::update_progress_from_bytematch(&root_for_fetch, &list, &jsons);
    })
    .await
    .map_err(|e| e.to_string())?;

    tauri::async_runtime::spawn_blocking(move || library::library(&config)).await.map_err(|e| e.to_string())?
}

#[tauri::command]
async fn apply_progress_json(state: State<'_, AppState>, json: String) -> Result<Library, String> {
    let config = state.config.lock().unwrap().clone();
    let root = config.root.clone().ok_or("the OpenRAC folder is not set")?;
    openrac_launcher_core::catalog::update_progress_from_bytematch(&root, &json, &Default::default())?;
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
    app.opener().open_path(target.to_string_lossy(), None::<&str>).map_err(|e| e.to_string())
}

#[tauri::command]
fn open_saves_folder(app: AppHandle, serial: String) -> Result<(), String> {
    let dir = openrac_launcher_core::saves::get_memcard_dir(&serial);
    if !dir.exists() {
        let _ = std::fs::create_dir_all(&dir);
    }
    app.opener().open_path(dir.to_string_lossy(), None::<&str>).map_err(|e| e.to_string())
}

/// Writes `bytes` to `path` unless it already holds them; true when it wrote.
#[cfg(target_os = "linux")]
fn write_if_changed(path: &std::path::Path, bytes: &[u8]) -> bool {
    if std::fs::read(path).map(|old| old == bytes).unwrap_or(false) {
        return false;
    }
    std::fs::write(path, bytes).is_ok()
}

/// The menu entry and icons the desktop finds the launcher by. On Wayland the window's app id
/// (the program's name, `openrac-launcher`) names the .desktop file the taskbar takes the icon
/// from; `dev.openrac.launcher` is the bundle's identifier. Both entries are kept so either id
/// finds the icon, but only one shows in the menu. Files are written, and the desktop's caches
/// refreshed, only when something changed: nothing to do on most starts.
#[cfg(target_os = "linux")]
fn ensure_linux_desktop_integration() {
    let sizes: &[(&str, &[u8])] = &[
        ("32x32", include_bytes!("../icons/32x32.png")),
        ("64x64", include_bytes!("../icons/64x64.png")),
        ("128x128", include_bytes!("../icons/128x128.png")),
        ("256x256", include_bytes!("../icons/128x128@2x.png")),
        ("512x512", include_bytes!("../icons/icon.png")),
    ];
    let Ok(home) = std::env::var("HOME") else { return };
    let home_path = PathBuf::from(home);
    let icons_base = home_path.join(".local/share/icons/hicolor");
    let app_dir = home_path.join(".local/share/applications");
    let _ = std::fs::create_dir_all(&app_dir);

    let mut icons_changed = false;
    for (size, bytes) in sizes {
        let dir = icons_base.join(size).join("apps");
        let _ = std::fs::create_dir_all(&dir);
        icons_changed |= write_if_changed(&dir.join("dev.openrac.launcher.png"), bytes);
        icons_changed |= write_if_changed(&dir.join("openrac-launcher.png"), bytes);
    }

    let exe_path = std::env::current_exe().unwrap_or_else(|_| PathBuf::from("openrac-launcher"));
    let entry = |wm_class: &str, hidden: bool| {
        format!(
            "[Desktop Entry]\nType=Application\nName=OpenRAC Launcher\nComment=Set up, build and play the decompiled Ratchet & Clank games\nExec=\"{}\"\nIcon=dev.openrac.launcher\nTerminal=false\nCategories=Game;Development;\nStartupNotify=true\nStartupWMClass={}\n{}",
            exe_path.to_string_lossy(),
            wm_class,
            if hidden { "NoDisplay=true\n" } else { "" }
        )
    };
    let mut entries_changed = false;
    entries_changed |= write_if_changed(
        &app_dir.join("dev.openrac.launcher.desktop"),
        entry("dev.openrac.launcher", false).as_bytes(),
    );
    entries_changed |=
        write_if_changed(&app_dir.join("openrac-launcher.desktop"), entry("openrac-launcher", true).as_bytes());

    if icons_changed {
        let _ = std::process::Command::new("gtk-update-icon-cache").arg("-f").arg("-t").arg(&icons_base).status();
    }
    if entries_changed || icons_changed {
        let _ = std::process::Command::new("update-desktop-database").arg(&app_dir).status();
        let _ = std::process::Command::new("kbuildsycoca6").status();
    }

    // If Flatpak Discord is running, ensure standard XDG_RUNTIME_DIR socket symlink exists
    if let Ok(runtime_dir) = std::env::var("XDG_RUNTIME_DIR") {
        let standard_sock = PathBuf::from(&runtime_dir).join("discord-ipc-0");
        let flatpak_sock = PathBuf::from(&runtime_dir).join("app/com.discordapp.Discord/discord-ipc-0");
        if flatpak_sock.exists() && !standard_sock.exists() {
            let _ = std::os::unix::fs::symlink(&flatpak_sock, &standard_sock);
        }
    }
}

/// Game desktop entries and icons so KDE Plasma and other Linux desktop taskbars show the game's
/// icon when launched from the launcher or directly.
#[cfg(target_os = "linux")]
fn ensure_linux_game_desktop_integration(root: &std::path::Path) {
    let Ok(home) = std::env::var("HOME") else { return };
    let home_path = PathBuf::from(home);
    let icons_base = home_path.join(".local/share/icons/hicolor");
    let app_dir = home_path.join(".local/share/applications");
    let _ = std::fs::create_dir_all(&app_dir);

    let games_dir = root.join("port/game");
    let Ok(entries) = std::fs::read_dir(games_dir) else { return };

    let mut icons_changed = false;
    let mut entries_changed = false;

    for entry in entries.flatten() {
        if !entry.path().is_dir() {
            continue;
        }
        let game_id = entry.file_name().to_string_lossy().to_string();
        let icon_path = entry.path().join("icon.png");
        if !icon_path.is_file() {
            continue;
        }
        let Ok(bytes) = std::fs::read(&icon_path) else { continue };

        let dir = icons_base.join("512x512/apps");
        let _ = std::fs::create_dir_all(&dir);
        let app_id = format!("dev.openrac.{game_id}");
        let prog_id = format!("openrac-{game_id}");

        icons_changed |= write_if_changed(&dir.join(format!("{app_id}.png")), &bytes);
        icons_changed |= write_if_changed(&dir.join(format!("{prog_id}.png")), &bytes);

        let exe_path = root.join(format!("port/build/release/openrac-{game_id}"));
        let title = match game_id.as_str() {
            "rac1-pal" => "Ratchet & Clank (PAL)",
            "rac1-ntsc" => "Ratchet & Clank (NTSC)",
            "rac2-ntsc" => "Ratchet & Clank: Going Commando (NTSC)",
            "rac3-ntsc" => "Ratchet & Clank: Up Your Arsenal (NTSC)",
            "rac4-ntsc" => "Ratchet: Deadlocked (NTSC)",
            _ => "Ratchet & Clank",
        };

        let make_entry = |wm_class: &str, hidden: bool| {
            format!(
                "[Desktop Entry]\nType=Application\nName={title}\nComment=Play {title} with OpenRAC\nExec=\"{}\"\nIcon={app_id}\nTerminal=false\nCategories=Game;\nStartupNotify=true\nStartupWMClass={wm_class}\n{}",
                exe_path.to_string_lossy(),
                if hidden { "NoDisplay=true\n" } else { "" }
            )
        };

        entries_changed |=
            write_if_changed(&app_dir.join(format!("{app_id}.desktop")), make_entry(&app_id, false).as_bytes());
        entries_changed |=
            write_if_changed(&app_dir.join(format!("{prog_id}.desktop")), make_entry(&prog_id, true).as_bytes());
    }

    if icons_changed {
        let _ = std::process::Command::new("gtk-update-icon-cache").arg("-f").arg("-t").arg(&icons_base).status();
    }
    if entries_changed || icons_changed {
        let _ = std::process::Command::new("update-desktop-database").arg(&app_dir).status();
        let _ = std::process::Command::new("kbuildsycoca6").status();
    }
}

/// AppImageKit's AppRun unconditionally sets PYTHONHOME=$APPDIR/usr/ and
/// PYTHONPATH=$APPDIR/usr/share/pyshared/:, which poisons any Python process
/// started from the AppImage (both our carried python in resources/openrac/python
/// and any system python). Clear them before anything runs.
#[cfg(target_os = "linux")]
fn clean_appimage_python_env() {
    if let Ok(appdir) = std::env::var("APPDIR") {
        if std::env::var("PYTHONHOME").is_ok_and(|h| h.starts_with(&appdir)) {
            std::env::remove_var("PYTHONHOME");
        }
        if std::env::var("PYTHONPATH").is_ok_and(|p| p.starts_with(&appdir)) {
            std::env::remove_var("PYTHONPATH");
        }
    }
}

/// WebKitGTK's DMA-BUF renderer draws a blank or flickering window under Wayland with NVIDIA's
/// own driver; the launcher turns it off there (and only there), before GTK starts. A value the
/// player set is left as it is.
#[cfg(target_os = "linux")]
fn tune_webkit_for_wayland() {
    let wayland = std::env::var_os("WAYLAND_DISPLAY").is_some();
    let nvidia = std::path::Path::new("/proc/driver/nvidia/version").exists();
    if wayland && nvidia && std::env::var_os("WEBKIT_DISABLE_DMABUF_RENDERER").is_none() {
        std::env::set_var("WEBKIT_DISABLE_DMABUF_RENDERER", "1");
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

// On Android and iOS the launcher is a library the app's activity starts through this entry point.
#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    #[cfg(target_os = "linux")]
    clean_appimage_python_env();
    #[cfg(target_os = "linux")]
    tune_webkit_for_wayland();
    #[cfg(target_os = "linux")]
    ensure_wayland_cursor_fallback();
    let builder = tauri::Builder::default().plugin(tauri_plugin_dialog::init()).plugin(tauri_plugin_opener::init());
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
        .setup(|app| {
            #[cfg(target_os = "linux")]
            ensure_linux_desktop_integration();

            #[cfg(desktop)]
            if let Some(window) = app.get_webview_window("main") {
                if let Some(icon) = app.default_window_icon() {
                    let _ = window.set_icon(icon.clone());
                }
            }

            let config_file = app.path().app_config_dir().ok().map(|dir| dir.join("launcher.json"));
            let mut config = config_file.as_deref().map(config::load).unwrap_or_default();

            // First run / startup: ensure the OpenRAC folder exists in the user's home directory.
            // On Linux: /home/<user>/OpenRAC
            // On macOS: /Users/<user>/OpenRAC
            // On Windows: C:\Users\<user>\OpenRAC
            let home_dir = app.path().home_dir().ok().or_else(detect::home_dir);
            let openrac_home = home_dir.as_ref().and_then(|h| detect::ensure_openrac_dir_in(h).ok());

            // OpenGOAL asks for an install folder at first start; until the
            // player picks one, games are set up in the user's OpenRAC folder (or launcher's data folder).
            let local_data = app.path().app_local_data_dir().ok();
            if let Some(openrac_dir) = &openrac_home {
                if config.install_dir.is_none() || config.install_dir.as_ref() == local_data.as_ref() {
                    if let Some(data) = &local_data {
                        let old_active = data.join("active");
                        let new_active = openrac_dir.join("active");
                        if old_active.exists() && !new_active.exists() {
                            let _ = std::fs::rename(&old_active, &new_active);
                        }
                    }
                    config.install_dir = Some(openrac_dir.clone());
                    if let Some(cfg_file) = &config_file {
                        let _ = config::save(cfg_file, &config);
                    }
                }
            } else if config.install_dir.is_none() {
                config.install_dir = local_data;
            }

            // The copy of OpenRAC the launcher carries: what a player plays from (detect::settle).
            // A copy whose Python does not run here is not for this machine (an Intel Mac has the
            // Apple silicon one): the launcher then works on a checkout, as it did.
            // On Android the copy is unpacked from the APK into the app's files (OpenracSetup.kt),
            // maybe still now: the page waits for it (tools_ready) before it reads anything there.
            let bundled = if Platform::current() == Platform::Android {
                android::bundled_root()
            } else {
                app.path().resource_dir().ok().and_then(|dir| detect::bundled_root(&dir)).filter(|dir| {
                    detect::bundled_python(dir).is_some_and(|python| detect::check_tool(Tool::Python, &python).ok)
                })
            };
            // Android's picker gives no paths: the disc images the player picks are copied into
            // one folder of the app's, which is then the games folder.
            if config.discs_dir.is_none() && Platform::current() == Platform::Android {
                config.discs_dir = android::discs_dir();
            }
            detect::settle(&mut config, bundled.as_deref());

            #[cfg(target_os = "linux")]
            if let Some(root) = &config.root {
                ensure_linux_game_desktop_integration(root);
            }

            let client_id = config
                .discord_client_id
                .clone()
                .unwrap_or_else(|| openrac_launcher_core::discord::DEFAULT_CLIENT_ID.to_string());
            let ipc = openrac_launcher_core::discord::DiscordIpc::new(client_id);
            let app_start = openrac_launcher_core::discord::now_sec();
            let discord = Arc::new(Mutex::new(DiscordState { ipc, app_start }));
            // Discord can take seconds to answer: connect without holding up the window.
            if config.discord_rpc {
                let discord = discord.clone();
                tauri::async_runtime::spawn_blocking(move || {
                    let mut discord = discord.lock().unwrap();
                    let _ = discord.ipc.set_status(&openrac_launcher_core::discord::DiscordStatus::Idle, app_start);
                });
            }

            if let Some(dir) = config_file.as_deref().and_then(Path::parent) {
                launcher_log::init(dir);
                launcher_log::line(&format!(
                    "the launcher {} started on {:?}",
                    app.package_info().version,
                    Platform::current()
                ));
            }
            app.manage(AppState { config: Mutex::new(config), config_file, jobs: Jobs::default(), discord, bundled });
            Ok(())
        })
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
            open_game_folder,
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
