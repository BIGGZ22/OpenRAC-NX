// The launcher's Android side (docs/port/ANDROID.md): what only Android's own APIs do, in Kotlin
// (gen/android/app/src/main/java/dev/openrac/launcher/OpenracPlugin.kt), reached through a plugin
// of the launcher's own. Everything else runs on Android as on a desktop: the tools are the
// carried copy's (tools/bundle_standalone.py --android), run by the Python the APK carries.
//
// The commands exist on every system so that the UI has one list; off Android they say so.
use std::path::PathBuf;

use openrac_launcher_core::actions::Plan;
use openrac_launcher_core::android::game_library;
#[cfg(target_os = "android")]
use openrac_launcher_core::android::{copy_disc, disc_file_name};
#[cfg(target_os = "android")]
use openrac_launcher_core::chd;
use openrac_launcher_core::jobs::Started;
use openrac_launcher_core::update;
#[cfg(target_os = "android")]
use serde::Deserialize;
use serde::Serialize;
use tauri::{AppHandle, Runtime};

#[cfg(target_os = "android")]
use tauri::{Emitter, Manager};

#[cfg(not(target_os = "android"))]
const ANDROID_ONLY: &str = "only the Android launcher does this";

/// How far a disc image's copy into the app's own storage, or a CHD's unpacking, is (the
/// `disc-copy` event).
#[derive(Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct DiscCopy {
    pub name: String,
    pub copied: u64,
    pub total: u64,
}

#[cfg(target_os = "android")]
pub struct Openrac<R: Runtime>(tauri::plugin::PluginHandle<R>);

/// The plugin that carries the Kotlin side; registered on Android only.
#[cfg(target_os = "android")]
pub fn init<R: Runtime>() -> tauri::plugin::TauriPlugin<R> {
    tauri::plugin::Builder::new("openrac")
        .setup(|app, api| {
            let handle = api.register_android_plugin("dev.openrac.launcher", "OpenracPlugin")?;
            app.manage(Openrac(handle));
            Ok(())
        })
        .build()
}

#[cfg(target_os = "android")]
async fn kotlin<R: Runtime, T: serde::de::DeserializeOwned>(
    app: &AppHandle<R>,
    command: &str,
    payload: impl Serialize,
) -> Result<T, String> {
    let plugin = app.try_state::<Openrac<R>>().ok_or("the launcher's Android plugin is not loaded")?;
    plugin.0.run_mobile_plugin_async(command, payload).await.map_err(|e| e.to_string())
}

/// Where the carried copy is (files/openrac, OpenracSetup.kt) and the folder disc images are
/// copied into: what the environment names.
pub fn bundled_root() -> Option<PathBuf> {
    std::env::var_os("OPENRAC_BUNDLED_ROOT").map(PathBuf::from)
}

pub fn discs_dir() -> Option<PathBuf> {
    std::env::var_os("OPENRAC_DISCS_DIR").map(PathBuf::from)
}

/// The player picks their disc image with the system's picker; it is copied into the app's own
/// storage (the extractor links it into place, and Android gives no path to the original), with
/// `disc-copy` events as it goes; a CHD is unpacked into a plain image instead (core/src/chd.rs).
/// The copy's path, or None when the player cancelled.
#[tauri::command]
pub async fn pick_disc<R: Runtime>(app: AppHandle<R>) -> Result<Option<PathBuf>, String> {
    #[cfg(target_os = "android")]
    {
        #[derive(Deserialize)]
        struct Picked {
            fd: Option<i32>,
            name: Option<String>,
            size: Option<i64>,
            free: Option<i64>,
        }
        let picked: Picked = kotlin(&app, "pickDisc", ()).await?;
        let (Some(fd), Some(name)) = (picked.fd, picked.name) else { return Ok(None) };
        // The descriptor Kotlin detached is this process's to close: File owns it from here.
        let file = unsafe { <std::fs::File as std::os::fd::FromRawFd>::from_raw_fd(fd) };
        let dir = discs_dir().ok_or("OPENRAC_DISCS_DIR is not set")?;
        let name = disc_file_name(&name);
        let (packed, plain) = (chd::is_chd(std::path::Path::new(&name)), dir.join(chd::iso_name(&name)));
        let dest = if packed { plain } else { dir.join(&name) };
        let emitter = app.clone();
        let emit = move |copied, total| {
            let _ = emitter.emit("disc-copy", DiscCopy { name: name.clone(), copied, total });
        };
        if packed {
            let unpacked = dest.clone();
            let free = picked.free.unwrap_or(-1);
            tauri::async_runtime::spawn_blocking(move || {
                let mut image = chd::ChdImage::new(file)?;
                // As the Kotlin side checks a plain image: room for it and the levels written next.
                let need = image.len() + (256 << 20);
                if free >= 0 && need > free as u64 {
                    return Err(format!("it needs {} MB and the device has {} MB free", need >> 20, free >> 20));
                }
                image.save_iso(&unpacked, emit)
            })
            .await
            .map_err(|e| e.to_string())?
            .map_err(|e| format!("cannot unpack the CHD image: {e}"))?;
            return Ok(Some(dest));
        }
        let total = picked.size.unwrap_or(-1).max(0) as u64;
        let copied = dest.clone();
        tauri::async_runtime::spawn_blocking(move || copy_disc(file, total, &copied, |done| emit(done, total)))
            .await
            .map_err(|e| e.to_string())?
            .map_err(|e| format!("cannot copy the disc image into the app's storage: {e}"))?;
        Ok(Some(dest))
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = app;
        Err(ANDROID_ONLY.into())
    }
}

/// The player's GPU driver packages, the one chosen and what the device is (OpenracPlugin.kt).
#[tauri::command]
pub async fn gpu_drivers<R: Runtime>(app: AppHandle<R>) -> Result<serde_json::Value, String> {
    #[cfg(target_os = "android")]
    return kotlin(&app, "gpuDrivers", ()).await;
    #[cfg(not(target_os = "android"))]
    {
        let _ = app;
        Err(ANDROID_ONLY.into())
    }
}

/// The player picks a driver package's zip; it is checked and unpacked. {driver} or {} (cancelled).
#[tauri::command]
pub async fn gpu_driver_import<R: Runtime>(app: AppHandle<R>) -> Result<serde_json::Value, String> {
    #[cfg(target_os = "android")]
    return kotlin(&app, "importDriver", ()).await;
    #[cfg(not(target_os = "android"))]
    {
        let _ = app;
        Err(ANDROID_ONLY.into())
    }
}

/// The package the game loads from now on; None: the system's driver.
#[tauri::command]
pub async fn gpu_driver_select<R: Runtime>(app: AppHandle<R>, id: Option<String>) -> Result<(), String> {
    #[cfg(target_os = "android")]
    {
        let _: serde_json::Value = kotlin(&app, "selectDriver", serde_json::json!({ "id": id })).await?;
        Ok(())
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = (app, id);
        Err(ANDROID_ONLY.into())
    }
}

#[tauri::command]
pub async fn gpu_driver_remove<R: Runtime>(app: AppHandle<R>, id: String) -> Result<(), String> {
    #[cfg(target_os = "android")]
    {
        let _: serde_json::Value = kotlin(&app, "removeDriver", serde_json::json!({ "id": id })).await?;
        Ok(())
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = (app, id);
        Err(ANDROID_ONLY.into())
    }
}

/// What the set-up's notification shows while the launcher is in the background, or None when the
/// set-up is over (OpenracPlugin.backgroundStatus, SetupService.kt): it keeps the process running.
#[tauri::command]
pub async fn background_status<R: Runtime>(
    app: AppHandle<R>,
    title: Option<String>,
    text: Option<String>,
    percent: Option<u8>,
) -> Result<(), String> {
    #[cfg(target_os = "android")]
    {
        let payload = serde_json::json!({ "title": title, "text": text.unwrap_or_default(), "percent": percent });
        let _: serde_json::Value = kotlin(&app, "backgroundStatus", payload).await?;
        Ok(())
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = (app, title, text, percent);
        Err(ANDROID_ONLY.into())
    }
}

/// The launcher's updater settings in tauri.conf.json (`plugins.updater`): the manifest's address
/// and the public key its files are signed with. Tauri's updater reads them on a desktop.
#[cfg(target_os = "android")]
fn updater_config<R: Runtime>(app: &AppHandle<R>) -> Result<(String, String), String> {
    let config = app.config().plugins.0.get("updater").ok_or("tauri.conf.json has no updater settings")?;
    let endpoint = config["endpoints"][0].as_str().ok_or("tauri.conf.json names no update manifest")?;
    let key = config["pubkey"].as_str().ok_or("tauri.conf.json has no update key")?;
    Ok((endpoint.to_string(), key.to_string()))
}

#[cfg(target_os = "android")]
fn updates_dir<R: Runtime>(app: &AppHandle<R>) -> Result<PathBuf, String> {
    Ok(app.path().app_cache_dir().map_err(|e| e.to_string())?.join("updates"))
}

/// A newer launcher on the releases (core/src/update.rs), or None. Android only: a desktop's
/// launcher is updated by Tauri's updater.
#[tauri::command]
pub async fn update_check<R: Runtime>(app: AppHandle<R>) -> Result<Option<update::Release>, String> {
    #[cfg(target_os = "android")]
    {
        let (endpoint, _) = updater_config(&app)?;
        let manifest = updates_dir(&app)?.join("latest.json");
        let _: serde_json::Value =
            kotlin(&app, "download", serde_json::json!({ "url": endpoint, "path": manifest })).await?;
        let text = std::fs::read_to_string(&manifest).map_err(|e| e.to_string())?;
        let _ = std::fs::remove_file(&manifest);
        update::release_for(&text, update::ANDROID, &crate::launcher_version(&app))
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = app;
        Err(ANDROID_ONLY.into())
    }
}

/// How far the update's download is (the `update-progress` event); total 0 while it is not known.
#[cfg(target_os = "android")]
#[derive(Clone, Serialize)]
pub struct UpdateProgress {
    pub downloaded: u64,
    pub total: u64,
}

/// Downloads the release's APK, checks its signature against the launcher's update key and hands
/// it to Android's installer, which asks the player to confirm.
#[tauri::command]
pub async fn update_install<R: Runtime>(app: AppHandle<R>, url: String, signature: String) -> Result<(), String> {
    #[cfg(target_os = "android")]
    {
        use std::sync::atomic::{AtomicBool, Ordering};
        use std::sync::Arc;

        let (_, key) = updater_config(&app)?;
        let apk = updates_dir(&app)?.join("OpenRAC-Launcher.apk");
        let _ = std::fs::remove_file(&apk);
        // The download runs in Kotlin; its progress is read from there four times a second.
        let done = Arc::new(AtomicBool::new(false));
        let watcher = {
            let (app, done) = (app.clone(), done.clone());
            std::thread::spawn(move || {
                let Some(plugin) = app.try_state::<Openrac<R>>() else { return };
                while !done.load(Ordering::Relaxed) {
                    std::thread::sleep(std::time::Duration::from_millis(250));
                    if let Ok(p) = plugin.0.run_mobile_plugin::<serde_json::Value>("downloadProgress", ()) {
                        let downloaded = p["downloaded"].as_u64().unwrap_or(0);
                        let total = p["total"].as_i64().unwrap_or(-1).max(0) as u64;
                        let _ = app.emit("update-progress", UpdateProgress { downloaded, total });
                    }
                }
            })
        };
        let downloaded: Result<serde_json::Value, String> =
            kotlin(&app, "download", serde_json::json!({ "url": url, "path": apk })).await;
        done.store(true, Ordering::Relaxed);
        let _ = watcher.join();
        downloaded?;
        let checked = apk.clone();
        let verified = tauri::async_runtime::spawn_blocking(move || update::verify(&checked, &signature, &key))
            .await
            .map_err(|e| e.to_string())?;
        if let Err(e) = verified {
            let _ = std::fs::remove_file(&apk);
            return Err(e);
        }
        let _: serde_json::Value = kotlin(&app, "installApk", serde_json::json!({ "path": apk })).await?;
        Ok(())
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = (app, url, signature);
        Err(ANDROID_ONLY.into())
    }
}

/// Whether the game's activity is running (in its own task, beside the launcher's) and which
/// game's library it is, when the launcher knows.
#[derive(Clone, Debug, Default, Serialize)]
#[cfg_attr(target_os = "android", derive(Deserialize))]
#[serde(rename_all = "camelCase")]
pub struct GameStatus {
    pub running: bool,
    pub library: Option<String>,
}

#[tauri::command]
pub async fn game_status<R: Runtime>(app: AppHandle<R>) -> Result<GameStatus, String> {
    #[cfg(target_os = "android")]
    {
        kotlin(&app, "gameStatus", ()).await
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = app;
        Ok(GameStatus::default())
    }
}

/// Brings the running game back in front of the launcher, or (`stop`) ends it.
#[tauri::command]
pub async fn game_window<R: Runtime>(app: AppHandle<R>, stop: bool) -> Result<(), String> {
    #[cfg(target_os = "android")]
    {
        let _: serde_json::Value = kotlin(&app, if stop { "stopGame" } else { "returnToGame" }, ()).await?;
        Ok(())
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = (app, stop);
        Err(ANDROID_ONLY.into())
    }
}

/// Plays: starts the game's activity with the command line a desktop would run. A detached
/// launch, as the game's window is its own activity.
pub fn launch<R: Runtime>(app: &AppHandle<R>, plan: &Plan) -> Result<Started, String> {
    let library = game_library(plan)?;
    #[cfg(target_os = "android")]
    {
        let plugin = app.try_state::<Openrac<R>>().ok_or("the launcher's Android plugin is not loaded")?;
        plugin
            .0
            .run_mobile_plugin::<serde_json::Value>(
                "launchGame",
                serde_json::json!({ "library": library, "args": plan.args }),
            )
            .map_err(|e| e.to_string())?;
        let command = std::iter::once(library).chain(plan.args.iter().cloned()).collect::<Vec<_>>().join(" ");
        Ok(Started { id: 0, title: plan.title.clone(), command, detached: true })
    }
    #[cfg(not(target_os = "android"))]
    {
        let _ = (app, library);
        Err(ANDROID_ONLY.into())
    }
}
