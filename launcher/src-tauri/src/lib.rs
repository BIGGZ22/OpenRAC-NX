// OpenRAC Launcher: the desktop app. Every file and process operation lives
// in openrac-launcher-core (../core); this file only exposes it to the UI
// (../src) as Tauri commands and forwards job output as events. The list of
// commands and events is docs/ARCHITECTURE.md's; keep them in step with
// src/lib/api.ts and src/lib/mock.ts.
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

use openrac_launcher_core::actions::{Platform, Scope};
use openrac_launcher_core::config::{self, Config};
use openrac_launcher_core::detect::{self, Check, Detected, Tool};
use openrac_launcher_core::install;
use openrac_launcher_core::jobs::{Event, Jobs, Started};
use openrac_launcher_core::library::{self, Library};
use serde::Serialize;
use tauri::{AppHandle, Emitter, Manager, State};
use tauri_plugin_opener::OpenerExt;

struct AppState {
    config: Mutex<Config>,
    config_file: Option<PathBuf>,
    jobs: Jobs,
    discord: Arc<Mutex<DiscordState>>,
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
}

#[tauri::command]
fn app_info(app: AppHandle, state: State<'_, AppState>) -> AppInfo {
    AppInfo {
        version: app.package_info().version.to_string(),
        platform: Platform::current(),
        config_file: state.config_file.clone(),
    }
}

#[tauri::command]
fn get_config(state: State<'_, AppState>) -> Config {
    state.config.lock().unwrap().clone()
}

#[tauri::command]
fn save_config(state: State<'_, AppState>, config: Config) -> Result<Config, String> {
    let file = state.config_file.as_deref().ok_or("no per-user config folder")?;
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
async fn detect() -> Detected {
    tauri::async_runtime::spawn_blocking(|| detect::detect_all(&start_folders())).await.unwrap_or_default()
}

#[tauri::command]
async fn check_root(path: PathBuf) -> Check {
    detect::check_root(&path)
}

#[tauri::command]
async fn check_tool(tool: Tool, path: PathBuf) -> Result<Check, String> {
    tauri::async_runtime::spawn_blocking(move || detect::check_tool(tool, &path)).await.map_err(|e| e.to_string())
}

#[tauri::command]
async fn library(state: State<'_, AppState>) -> Result<Library, String> {
    let config = state.config.lock().unwrap().clone();
    tauri::async_runtime::spawn_blocking(move || library::library(&config)).await.map_err(|e| e.to_string())?
}

/// Runs action `id` of `scope`: a job whose output streams as `job-output`
/// and `job-exit` events, or a detached launch (id 0).
#[tauri::command]
fn run_action(app: AppHandle, state: State<'_, AppState>, scope: Scope, id: String) -> Result<Started, String> {
    let config = state.config.lock().unwrap().clone();
    let plan = library::plan(&config, &scope, &id)?;
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
    let app_handle = app.clone();
    state.jobs.start(&plan, config.root.as_deref(), move |event| {
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
            Event::Exit { .. } => {
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
    })
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
    let target = std::fs::canonicalize(openrac_launcher_core::under(&root, rel)).map_err(|e| format!("{path}: {e}"))?;
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
    state.jobs.start(&plan, config.root.as_deref(), move |event| {
        let _ = match &event {
            Event::Output { .. } => app.emit("job-output", &event),
            Event::Exit { .. } => app.emit("job-exit", &event),
        };
    })
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

    // Attempt to fetch latest numbers from openrac.dev using curl (available across all Linux distros)
    let output = std::process::Command::new("curl")
        .arg("-s")
        .arg("--connect-timeout")
        .arg("4")
        .arg("https://openrac.dev/progress.json")
        .output();

    if let Ok(out) = output {
        if out.status.success() {
            if let Ok(json_str) = String::from_utf8(out.stdout) {
                let _ = openrac_launcher_core::catalog::update_progress_from_openrac_dev(&root, &json_str);
            }
        }
    }

    tauri::async_runtime::spawn_blocking(move || library::library(&config)).await.map_err(|e| e.to_string())?
}

#[tauri::command]
async fn apply_progress_json(state: State<'_, AppState>, json: String) -> Result<Library, String> {
    let config = state.config.lock().unwrap().clone();
    let root = config.root.clone().ok_or("the OpenRAC folder is not set")?;
    openrac_launcher_core::catalog::update_progress_from_openrac_dev(&root, &json)?;
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

#[tauri::command]
fn restore_backup(serial: String, backup_name: String) -> Result<(), String> {
    openrac_launcher_core::saves::restore_backup(&serial, &backup_name)
}

#[tauri::command]
fn open_saves_folder(app: AppHandle, serial: String) -> Result<(), String> {
    let dir = openrac_launcher_core::saves::get_memcard_dir(&serial);
    if !dir.exists() {
        let _ = std::fs::create_dir_all(&dir);
    }
    app.opener().open_path(dir.to_string_lossy(), None::<&str>).map_err(|e| e.to_string())
}

#[cfg(target_os = "linux")]
fn ensure_linux_desktop_integration() {
    let icon_bytes_512 = include_bytes!("../icons/icon.png");
    let icon_bytes_256 = include_bytes!("../icons/128x128@2x.png");
    let icon_bytes_128 = include_bytes!("../icons/128x128.png");
    let icon_bytes_64 = include_bytes!("../icons/64x64.png");
    let icon_bytes_32 = include_bytes!("../icons/32x32.png");

    if let Ok(home) = std::env::var("HOME") {
        let home_path = PathBuf::from(home);
        let icons_base = home_path.join(".local/share/icons/hicolor");
        let app_dir = home_path.join(".local/share/applications");
        let _ = std::fs::create_dir_all(&app_dir);

        let sizes: &[(&str, &[u8])] = &[
            ("32x32", icon_bytes_32),
            ("64x64", icon_bytes_64),
            ("128x128", icon_bytes_128),
            ("256x256", icon_bytes_256),
            ("512x512", icon_bytes_512),
        ];

        for (sz, bytes) in sizes {
            let dir = icons_base.join(sz).join("apps");
            let _ = std::fs::create_dir_all(&dir);
            let _ = std::fs::write(dir.join("dev.openrac.launcher.png"), bytes);
            let _ = std::fs::write(dir.join("openrac-launcher.png"), bytes);
        }

        let main_icon = icons_base.join("128x128/apps/dev.openrac.launcher.png");
        let icon_target = if main_icon.exists() {
            main_icon.to_string_lossy().to_string()
        } else {
            "dev.openrac.launcher".to_string()
        };

        let exe_path = std::env::current_exe().unwrap_or_else(|_| PathBuf::from("openrac-launcher"));
        let exe_str = exe_path.to_string_lossy();

        let desktop_content1 = format!(
            "[Desktop Entry]\nType=Application\nName=OpenRAC Launcher\nComment=OpenRAC Launcher\nExec={}\nIcon={}\nTerminal=false\nCategories=Game;Development;\nStartupWMClass=openrac-launcher\n",
            exe_str, icon_target
        );
        let desktop_content2 = format!(
            "[Desktop Entry]\nType=Application\nName=OpenRAC Launcher\nComment=OpenRAC Launcher\nExec={}\nIcon={}\nTerminal=false\nCategories=Game;Development;\nStartupWMClass=dev.openrac.launcher\n",
            exe_str, icon_target
        );

        let _ = std::fs::write(app_dir.join("openrac-launcher.desktop"), desktop_content1);
        let _ = std::fs::write(app_dir.join("dev.openrac.launcher.desktop"), desktop_content2);

        // Notify desktop environment and refresh icon caches silently
        let _ = std::process::Command::new("gtk-update-icon-cache").arg("-f").arg("-t").arg(&icons_base).status();
        let _ = std::process::Command::new("update-desktop-database").arg(&app_dir).status();
        let _ = std::process::Command::new("kbuildsycoca6").arg("--noincremental").status();

        // If Flatpak Discord is running, ensure standard XDG_RUNTIME_DIR socket symlink exists
        if let Ok(runtime_dir) = std::env::var("XDG_RUNTIME_DIR") {
            let standard_sock = PathBuf::from(&runtime_dir).join("discord-ipc-0");
            let flatpak_sock = PathBuf::from(&runtime_dir).join("app/com.discordapp.Discord/discord-ipc-0");
            if flatpak_sock.exists() && !standard_sock.exists() {
                let _ = std::os::unix::fs::symlink(&flatpak_sock, &standard_sock);
            }
        }
    }
}

pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_opener::init())
        .setup(|app| {
            #[cfg(target_os = "linux")]
            ensure_linux_desktop_integration();

            if let Some(window) = app.get_webview_window("main") {
                if let Some(icon) = app.default_window_icon() {
                    let _ = window.set_icon(icon.clone());
                }
            }

            let config_file = app.path().app_config_dir().ok().map(|dir| dir.join("launcher.json"));
            let mut config = config_file.as_deref().map(config::load).unwrap_or_default();
            // OpenGOAL asks for an install folder at first start; until the
            // player picks one, games are set up in the launcher's own data folder.
            if config.install_dir.is_none() {
                config.install_dir = app.path().app_local_data_dir().ok();
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

            app.manage(AppState { config: Mutex::new(config), config_file, jobs: Jobs::default(), discord });
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            app_info,
            get_config,
            save_config,
            detect,
            check_root,
            check_tool,
            library,
            run_action,
            cancel_job,
            open_path,
            open_url,
            install_game,
            uninstall_game,
            sync_progress_from_web,
            apply_progress_json,
            set_discord_status,
            inspect_saves,
            backup_saves,
            restore_backup,
            open_saves_folder,
        ])
        .run(tauri::generate_context!())
        .expect("the launcher failed to start");
}
