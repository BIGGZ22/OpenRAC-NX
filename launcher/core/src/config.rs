//! The launcher's own settings: where the OpenRAC checkout and the programs
//! it runs are. Stored as `launcher.json` in the per-user config folder (the
//! app decides where; see `src-tauri/src/lib.rs`), never in the checkout.

use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
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
    /// Where games are set up from the player's discs (OpenGOAL's install
    /// directory); unset: the launcher's per-user data folder.
    pub install_dir: Option<PathBuf>,
    /// The first-run setup was finished (or skipped).
    pub setup_complete: bool,
    /// The version the launcher showed last (`rac1/pal`).
    pub last_version: Option<String>,
    /// Discord Rich Presence enabled (default true).
    pub discord_rpc: bool,
    /// Custom Discord application ID (optional).
    pub discord_client_id: Option<String>,
    /// The page layout: "compact" (default) or "classic", the original design (src/layouts/).
    pub layout: String,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            root: None,
            python: None,
            godot: None,
            docker: None,
            install_dir: None,
            developer: false,
            setup_complete: false,
            last_version: None,
            discord_rpc: true,
            discord_client_id: None,
            layout: "compact".into(),
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
    std::fs::write(&tmp, text).map_err(|e| format!("cannot write {}: {e}", tmp.display()))?;
    std::fs::rename(&tmp, path).map_err(|e| format!("cannot write {}: {e}", path.display()))
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
}
