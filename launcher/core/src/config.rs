//! The launcher's own settings: where the OpenRAC checkout and the programs
//! it runs are. Stored as `launcher.json` in the per-user config folder (the
//! app decides where; see `src-tauri/src/lib.rs`), never in the checkout.

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize, ts_rs::TS)]
#[serde(rename_all = "camelCase", default)]
pub struct Config {
    /// The OpenRAC checkout (holds games/, tools/ and baserom/).
    pub root: Option<PathBuf>,
    /// Python 3.10+, for tools/openrac.py, the editor and the games' Python tools.
    pub python: Option<PathBuf>,
    /// Godot 4 (4.7+), for the level editor.
    pub godot: Option<PathBuf>,
    /// Docker, which rac1/pal's and rac4's builds run in.
    pub docker: Option<PathBuf>,
    /// Show what contributors use: builds, checks, toolchains, progress.
    /// Off, the launcher shows a player's three steps: set up from your disc, play, edit levels.
    pub developer: bool,
    /// The save editor shows every field of a save, raw bytes included (for debugging). Off, the
    /// player's safe set only.
    pub save_editor_advanced: bool,
    /// Where games are set up from the player's discs (OpenGOAL's install
    /// directory); unset: the launcher's per-user data folder.
    pub install_dir: Option<PathBuf>,
    /// The player's folder of PlayStation 2 disc images, looked through for the games OpenRAC
    /// knows (discs.rs): with it nobody picks an image by hand.
    #[serde(default)]
    pub discs_dir: Option<PathBuf>,
    /// The first-run setup was finished (or skipped).
    pub setup_complete: bool,
    /// The version the launcher showed last (`rac1/pal`).
    pub last_version: Option<String>,
    /// Discord Rich Presence enabled (default true).
    pub discord_rpc: bool,
    /// Look for a new launcher when it starts (default true). Off, it looks only when asked
    /// (Settings, "Check now"); an update is never installed without the player's click.
    pub check_updates: bool,
    /// Which builds the launcher updates to (update::Channel): the releases, or the newest
    /// development build. Unset: the one it was built from (a release's build, the releases).
    pub update_channel: Option<crate::update::Channel>,
    /// A fork's launcher only: the branch whose development builds the development channel follows
    /// (update::UpdateSource::chosen). Unset: the one it was built from, or the default branch.
    pub update_branch: Option<String>,
    /// Custom Discord application ID (optional).
    pub discord_client_id: Option<String>,
    /// The page layout: "compact" (default) or "classic", the original design (src/layouts/).
    #[ts(type = "\"compact\" | \"classic\"")]
    pub layout: String,
    /// The launcher's theme (src/lib/palettes.json): "openrac" (default), "gruvbox", "oled", "nord"
    /// or "mocha". The accent is not a theme's: on Android it is the picked game's.
    pub theme: String,
    /// The game writes a detailed log (OPENRAC_LOG=verbose): level loads, controller changes,
    /// what the port is missing. Off by default, so a log stays small.
    pub detailed_game_log: bool,
    /// What the game draws with on Windows, Linux and macOS: "opengl" (OpenGL 4.1, the default)
    /// or "vulkan", the port's experimental Vulkan renderer (OPENRAC_GL_API=vulkan; on macOS the
    /// MoltenVK its bundle carries). Where Vulkan cannot start, the game draws with OpenGL
    /// (port/platform/window.cpp). Not used on Android, which keeps its own choice
    /// (src-tauri/src/android.rs).
    #[ts(type = "\"opengl\" | \"vulkan\"")]
    pub renderer: String,
    /// Purist mode: the game loads no native mods (`--no-mods`, docs/port/MODS.md), whatever the
    /// Mods tab has switched on. Off by default.
    pub purist: bool,
    /// For each game, the time (seconds since 1970) of the newest crash report the player has
    /// dismissed: crashes up to then are seen, and no notice, button or mark is shown for them.
    pub crashes_seen: BTreeMap<String, u64>,
    /// For developers with more than one disc of a game in the games folder: the version each
    /// game's page plays (`rac1` -> `rac1/ntsc`). Used only with developer tools on
    /// (disc_choice.rs); without one, the disc set up, as for a player.
    pub disc_choice: BTreeMap<String, String>,
    /// Where the launcher keeps its own copy of the figures it fetched from bytematch.dev: in
    /// the per-user folder beside its settings, which the launcher can always write (a
    /// checkout it does not own, as an installed launcher carries one, is read-only).
    /// Not saved with the settings: the app sets it at every start.
    #[serde(skip)]
    pub progress_cache: Option<PathBuf>,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            root: None,
            python: None,
            godot: None,
            docker: None,
            install_dir: None,
            discs_dir: None,
            developer: false,
            save_editor_advanced: false,
            setup_complete: false,
            last_version: None,
            discord_rpc: true,
            check_updates: true,
            update_channel: None,
            update_branch: None,
            discord_client_id: None,
            layout: "compact".into(),
            theme: "openrac".into(),
            detailed_game_log: false,
            renderer: "opengl".into(),
            purist: false,
            crashes_seen: BTreeMap::new(),
            disc_choice: BTreeMap::new(),
            progress_cache: None,
        }
    }
}

impl Config {
    pub fn root(&self) -> Result<&Path, String> {
        self.root.as_deref().ok_or_else(|| "the OpenRAC folder is not set (Settings)".to_string())
    }
}

/// Reads `path`; a missing or unreadable file gives the defaults, so a
/// broken settings file never stops the launcher from starting.
pub fn load(path: &Path) -> Config {
    std::fs::read_to_string(path).ok().and_then(|text| serde_json::from_str(&text).ok()).unwrap_or_default()
}

/// Writes `config` to `path` through a temporary file, so a crash never
/// leaves half a file behind.
pub fn save(path: &Path, config: &Config) -> Result<(), String> {
    if let Some(dir) = path.parent() {
        std::fs::create_dir_all(dir).map_err(|e| format!("cannot create {}: {e}", dir.display()))?;
    }
    let text = serde_json::to_string_pretty(config).map_err(|e| e.to_string())?;
    let tmp = path.with_extension("json.tmp");
    if let Err(e) = std::fs::write(&tmp, text) {
        let _ = std::fs::remove_file(&tmp);
        return Err(format!("cannot write {}: {e}", tmp.display()));
    }
    if let Err(e) = std::fs::rename(&tmp, path) {
        let _ = std::fs::remove_file(&tmp);
        return Err(format!("cannot write {}: {e}", path.display()));
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn round_trips_and_tolerates_a_broken_file() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("sub").join("launcher.json");
        assert_eq!(load(&path), Config::default());
        let config = Config {
            root: Some("/somewhere/OpenRAC".into()),
            setup_complete: true,
            last_version: Some("rac1/pal".into()),
            ..Config::default()
        };
        save(&path, &config).unwrap();
        assert_eq!(load(&path), config);
        let text = std::fs::read_to_string(&path).unwrap();
        assert!(text.contains("\"setupComplete\": true"), "{text}");
        std::fs::write(&path, "{ not json").unwrap();
        assert_eq!(load(&path), Config::default());
    }

    /// A settings file from before the option looks for updates, and turning it off is kept.
    #[test]
    fn checks_for_updates_unless_turned_off() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("launcher.json");
        std::fs::write(&path, r#"{"setupComplete": true}"#).unwrap();
        assert!(load(&path).check_updates);
        save(&path, &Config { check_updates: false, ..Config::default() }).unwrap();
        assert!(!load(&path).check_updates);
        assert!(std::fs::read_to_string(&path).unwrap().contains("\"checkUpdates\": false"));
    }

    /// A settings file from before the renderer choice draws with OpenGL; Vulkan, chosen, is kept.
    #[test]
    fn renderer_defaults_to_opengl() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("launcher.json");
        std::fs::write(&path, r#"{"setupComplete": true}"#).unwrap();
        assert_eq!(load(&path).renderer, "opengl");
        save(&path, &Config { renderer: "vulkan".into(), ..Config::default() }).unwrap();
        assert_eq!(load(&path).renderer, "vulkan");
    }
}
