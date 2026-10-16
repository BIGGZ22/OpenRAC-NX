//! A game's overrides packs: replacement textures, models and level edits the
//! native port loads on top of the extracted levels.
//!
//! They live in the game's set-up folder, `<install>/active/<game>/data/overrides`:
//! `packs/<id>/` (each one a copy made when it was added, with its
//! `openrac-mod.json`) and `overrides.json`, their order and which are on. The
//! format and everything that writes packs (adding, importing edits, stacking
//! them for the port) are the editor's `editor/overrides.py`; this module only
//! lists them and changes the order, the on/off state and removes one, which
//! need no Python and must be instant in the UI.

use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};
use serde_json::Value;

/// The pack manifest's name, at the root of each pack.
pub const MANIFEST: &str = "openrac-mod.json";
/// The list of packs, in order, with their state.
pub const STATE: &str = "overrides.json";

/// `<data>/overrides` for a game set up in `install_dir`.
pub fn dir(install_dir: &Path, game: &str) -> PathBuf {
    crate::install::data_dir(install_dir, game).join("overrides")
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
struct Entry {
    id: String,
    #[serde(default = "on")]
    enabled: bool,
}

fn on() -> bool {
    true
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct State {
    format: u32,
    packs: Vec<Entry>,
}

/// A pack as the Mods page shows it.
#[derive(Debug, Clone, Serialize, PartialEq, Eq)]
#[serde(rename_all = "camelCase")]
pub struct PackView {
    pub id: String,
    pub name: String,
    pub author: String,
    pub description: String,
    pub enabled: bool,
    /// How many levels it changes, and how many textures, models and level files in all.
    pub levels: usize,
    pub textures: usize,
    pub models: usize,
    pub level_files: usize,
}

fn read_state(dir: &Path) -> Result<State, String> {
    let path = dir.join(STATE);
    if !path.is_file() {
        return Ok(State { format: 1, packs: Vec::new() });
    }
    let text = std::fs::read_to_string(&path).map_err(|e| format!("{}: {e}", path.display()))?;
    let mut state: State =
        serde_json::from_str(&text).map_err(|e| format!("{} cannot be read ({e})", path.display()))?;
    if state.format != 1 {
        return Err(format!("{} is of a newer format ({}): update the launcher", path.display(), state.format));
    }
    // A pack whose folder went away drops out, as editor/overrides.py drops it.
    state.packs.retain(|p| valid_id(&p.id) && dir.join("packs").join(&p.id).join(MANIFEST).is_file());
    Ok(state)
}

fn write_state(dir: &Path, state: &State) -> Result<(), String> {
    let path = dir.join(STATE);
    let temporary = dir.join(format!("{STATE}.tmp"));
    let text = serde_json::to_string_pretty(state).map_err(|e| e.to_string())? + "\n";
    std::fs::write(&temporary, text).map_err(|e| format!("{}: {e}", temporary.display()))?;
    std::fs::rename(&temporary, &path).map_err(|e| format!("{}: {e}", path.display()))
}

/// An id `editor/overrides.py` makes: lowercase letters, digits and dashes. Anything else
/// (a path) is never used to name a folder.
fn valid_id(id: &str) -> bool {
    !id.is_empty() && id.len() <= 64 && id.chars().all(|c| c.is_ascii_lowercase() || c.is_ascii_digit() || c == '-')
}

fn view(dir: &Path, entry: &Entry) -> PackView {
    let manifest: Value = std::fs::read_to_string(dir.join("packs").join(&entry.id).join(MANIFEST))
        .ok()
        .and_then(|t| serde_json::from_str(&t).ok())
        .unwrap_or(Value::Null);
    let text = |key: &str| manifest.get(key).and_then(Value::as_str).unwrap_or_default().to_string();
    let (mut textures, mut models, mut level_files, mut levels) = (0, 0, 0, 0);
    if let Some(replaces) = manifest.get("replaces").and_then(Value::as_object) {
        levels = replaces.len();
        for level in replaces.values() {
            let count = |key: &str| level.get(key).and_then(Value::as_array).map_or(0, Vec::len);
            textures += count("textures");
            models += count("models");
            level_files += count("level");
        }
    }
    let name = text("name");
    PackView {
        name: if name.is_empty() { entry.id.clone() } else { name },
        author: text("author"),
        description: text("description"),
        id: entry.id.clone(),
        enabled: entry.enabled,
        levels,
        textures,
        models,
        level_files,
    }
}

/// The packs, in the order they apply (a later one wins over an earlier one).
pub fn list(dir: &Path) -> Result<Vec<PackView>, String> {
    let state = read_state(dir)?;
    Ok(state.packs.iter().map(|e| view(dir, e)).collect())
}

fn change(dir: &Path, id: &str, edit: impl FnOnce(&mut Vec<Entry>, usize)) -> Result<Vec<PackView>, String> {
    let mut state = read_state(dir)?;
    let at = state.packs.iter().position(|p| p.id == id).ok_or_else(|| format!("no pack {id}"))?;
    edit(&mut state.packs, at);
    write_state(dir, &state)?;
    list(dir)
}

/// Switches a pack on or off.
pub fn set_enabled(dir: &Path, id: &str, enabled: bool) -> Result<Vec<PackView>, String> {
    change(dir, id, |packs, at| packs[at].enabled = enabled)
}

/// Moves a pack one place earlier (`up`) or later (`down`) in the order.
pub fn move_pack(dir: &Path, id: &str, up: bool) -> Result<Vec<PackView>, String> {
    change(dir, id, |packs, at| {
        let to = if up { at.saturating_sub(1) } else { (at + 1).min(packs.len() - 1) };
        let entry = packs.remove(at);
        packs.insert(to, entry);
    })
}

/// Removes a pack: the copy that adding it made. What it was added from is never touched.
pub fn remove(dir: &Path, id: &str) -> Result<Vec<PackView>, String> {
    let views = change(dir, id, |packs, at| {
        packs.remove(at);
    })?;
    let folder = dir.join("packs").join(id);
    if valid_id(id) && folder.join(MANIFEST).is_file() {
        std::fs::remove_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
    }
    Ok(views)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn pack(dir: &Path, id: &str, name: &str) {
        let folder = dir.join("packs").join(id);
        std::fs::create_dir_all(folder.join("level_00/textures")).unwrap();
        std::fs::write(
            folder.join(MANIFEST),
            format!(
                r#"{{"format": 1, "game": "rac1", "version": "pal", "name": "{name}", "author": "me",
                    "replaces": {{"level_00": {{"textures": ["a", "b"], "models": ["moby_0"]}},
                                 "level_03": {{"level": ["gameplay_pal.wad"]}}}}}}"#
            ),
        )
        .unwrap();
    }

    #[test]
    fn lists_orders_switches_and_removes() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path();
        assert_eq!(list(dir).unwrap(), Vec::new(), "no folder yet: no packs");
        pack(dir, "hd", "HD textures");
        pack(dir, "green", "Green Ratchet");
        pack(dir, "unlisted", "Not in the list");
        std::fs::write(
            dir.join(STATE),
            r#"{"format": 1, "packs": [{"id": "hd", "enabled": true}, {"id": "green"}, {"id": "gone"}]}"#,
        )
        .unwrap();
        let packs = list(dir).unwrap();
        assert_eq!(packs.iter().map(|p| p.id.as_str()).collect::<Vec<_>>(), ["hd", "green"]);
        assert_eq!((packs[0].levels, packs[0].textures, packs[0].models, packs[0].level_files), (2, 2, 1, 1));
        assert_eq!((packs[1].name.as_str(), packs[1].enabled), ("Green Ratchet", true));

        let packs = move_pack(dir, "green", true).unwrap();
        assert_eq!(packs[0].id, "green");
        let packs = set_enabled(dir, "hd", false).unwrap();
        assert!(!packs[1].enabled);
        // What the Python side reads back.
        let state: Value = serde_json::from_str(&std::fs::read_to_string(dir.join(STATE)).unwrap()).unwrap();
        assert_eq!(state["packs"][1], serde_json::json!({"id": "hd", "enabled": false}));

        let packs = remove(dir, "green").unwrap();
        assert_eq!(packs.len(), 1);
        assert!(!dir.join("packs/green").exists());
        assert!(dir.join("packs/unlisted").exists(), "a folder not in the list is left alone");
        assert!(remove(dir, "../hd").is_err());
        assert!(set_enabled(dir, "nope", true).is_err());
    }
}
