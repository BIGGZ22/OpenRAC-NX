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
