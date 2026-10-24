//! A game's gameplay mods (docs/port/MODS.md): native ones, shared libraries the port loads at
//! start, and script mods, Luau files the port runs in a sandbox (section 11).
//!
//! They live in the game's set-up folder, `<install>/active/<game>/data/plugins`: `<id>/` (each
//! one as installed, with its `openrac-mod.json`, format 2), `plugins.json` (the order, which are
//! on and their settings: written here, read by the port) and `status.json` (what the port did with
//! each at its last start: read here). A mod lists the discs it runs on, and one folder serves both
//! of a game's discs, as the overrides packs do ([`crate::overrides`]).
//!
//! This module checks a mod by the rules of MODS.md, section 3, the port's loader checks it by the
//! same ones (port/game/common/mod_manifest.cpp), and the Mods tab shows the result before Play:
//! the port's `status.json` adds what only loading tells.

use std::collections::{BTreeMap, BTreeSet};
use std::io::Read;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};
use serde_json::{Map, Value};

/// The mod interface this launcher's port has: `OPENRAC_MOD_API_MAJOR` and `_MINOR` of
/// `sdk/include/openrac/openrac_mod.h`. Change them with it.
pub const API_MAJOR: u32 = 1;
pub const API_MINOR: u32 = 3;
/// The interface a script mod needs at least: 1.1, the first with scripts (MODS.md, section 11).
pub const SCRIPT_API_MINOR: u32 = 1;

/// The names a keybind setting's binds take (interface 1.2): `sdk/include/openrac/keys.h`'s table,
/// keys by position as the web's `KeyboardEvent.code` names them, then the mouse's buttons and the
/// wheel's directions. Change it with keys.h (a test checks they agree).
#[rustfmt::skip]
pub const BIND_NAMES: &[&str] = &[
    "KeyA", "KeyB", "KeyC", "KeyD", "KeyE", "KeyF", "KeyG", "KeyH",
    "KeyI", "KeyJ", "KeyK", "KeyL", "KeyM", "KeyN", "KeyO", "KeyP",
    "KeyQ", "KeyR", "KeyS", "KeyT", "KeyU", "KeyV", "KeyW", "KeyX",
    "KeyY", "KeyZ", "Digit1", "Digit2", "Digit3", "Digit4", "Digit5", "Digit6",
    "Digit7", "Digit8", "Digit9", "Digit0", "Enter", "Escape", "Backspace", "Tab",
    "Space", "Minus", "Equal", "BracketLeft", "BracketRight", "Backslash", "IntlHash", "Semicolon",
    "Quote", "Backquote", "Comma", "Period", "Slash", "CapsLock", "F1", "F2",
    "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
    "F11", "F12", "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp",
    "Delete", "End", "PageDown", "ArrowRight", "ArrowLeft", "ArrowDown", "ArrowUp", "NumLock",
    "NumpadDivide", "NumpadMultiply", "NumpadSubtract", "NumpadAdd", "NumpadEnter", "Numpad1", "Numpad2", "Numpad3",
    "Numpad4", "Numpad5", "Numpad6", "Numpad7", "Numpad8", "Numpad9", "Numpad0", "NumpadDecimal",
    "IntlBackslash", "ContextMenu", "NumpadEqual", "ControlLeft", "ShiftLeft", "AltLeft", "MetaLeft", "ControlRight",
    "ShiftRight", "AltRight", "MetaRight", "MouseLeft", "MouseRight", "MouseMiddle", "MouseBack", "MouseForward",
    "WheelUp", "WheelDown", "WheelLeft", "WheelRight",
];

/// A keybind's value as the setting keeps it: its names comma-joined ("KeyW,ArrowUp"), or None
/// when one is no name of [`BIND_NAMES`], one is there twice, or there are more than `most`.
pub fn parse_binds(text: &str, most: usize) -> Option<String> {
    let names: Vec<&str> = text.split([',', ' ']).filter(|n| !n.is_empty()).collect();
    if names.len() > most || names.iter().any(|n| !BIND_NAMES.contains(n)) {
        return None;
    }
    if names.iter().enumerate().any(|(i, n)| names[..i].contains(n)) {
        return None;
    }
    Some(names.join(","))
}

/// A mod's manifest, at the root of its folder (the overrides packs' name; they are format 1).
pub const MANIFEST: &str = "openrac-mod.json";
/// The launcher's list: order, on or off, settings.
pub const LIST: &str = "plugins.json";
/// The port's answer at its last start.
pub const STATUS: &str = "status.json";

/// `<data>/plugins` for a game set up in `install_dir`.
pub fn dir(install_dir: &Path, game: &str) -> PathBuf {
    crate::install::data_dir(install_dir, game).join("plugins")
}

/// The manifest's name for this system (`macos-arm64`, `linux-x86_64`, `windows-x86_64`, ...), or
/// None where no native mod runs (Android).
pub fn platform_key() -> Option<&'static str> {
    platform_key_for(std::env::consts::OS, std::env::consts::ARCH)
}

fn platform_key_for(os: &str, arch: &str) -> Option<&'static str> {
    Some(match (os, arch) {
        ("macos", "aarch64") => "macos-arm64",
        ("macos", "x86_64") => "macos-x86_64",
        ("linux", "x86_64") => "linux-x86_64",
        ("linux", "aarch64") => "linux-aarch64",
        ("windows", "x86_64") => "windows-x86_64",
        ("windows", "aarch64") => "windows-arm64",
        _ => return None,
    })
}

const PLATFORMS: [&str; 6] =
    ["macos-arm64", "macos-x86_64", "linux-x86_64", "linux-aarch64", "windows-x86_64", "windows-arm64"];

// ---- The manifest -------------------------------------------------------------------------------

/// One of a choice setting's values.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, ts_rs::TS)]
#[ts(rename = "SettingChoice")]
pub struct Choice {
    pub value: String,
    pub name: String,
}

/// A setting a mod declares (MODS.md, section 3).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct SettingSpec {
    pub key: String,
    pub name: String,
    #[serde(rename = "type")]
    #[ts(type = "\"bool\" | \"int\" | \"float\" | \"choice\" | \"string\" | \"keybind\"")]
    pub kind: String,
    #[serde(default)]
    #[ts(type = "unknown")]
    pub default: Value,
    #[serde(default, skip_serializing_if = "String::is_empty")]
    #[ts(as = "Option<String>", optional)]
    pub description: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    #[ts(optional)]
    pub min: Option<f64>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    #[ts(optional)]
    pub max: Option<f64>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    #[ts(optional)]
    pub step: Option<f64>,
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    #[ts(as = "Option<Vec<Choice>>", optional)]
    pub choices: Vec<Choice>,
    #[serde(default, alias = "max_length", skip_serializing_if = "Option::is_none")]
    #[ts(optional)]
    pub max_length: Option<usize>,
    /// A keybind's most binds: 1 or 2 (2 when absent).
    #[serde(default, skip_serializing_if = "Option::is_none")]
    #[ts(optional)]
    pub binds: Option<u32>,
    /// A keybind's group: two keybinds of one group on the same input conflict; of two groups
    /// ("play", "menu") they may share it.
    #[serde(default, skip_serializing_if = "String::is_empty")]
    #[ts(as = "Option<String>", optional)]
    pub group: String,
}

impl SettingSpec {
    /// A keybind's most binds.
    pub fn most_binds(&self) -> usize {
        self.binds.unwrap_or(2) as usize
    }
}

/// A set of setting values offered as one button (the manifest's `presets`), of any type: sliders,
/// choices, switches, text and keybinds. The settings it does not name, among the mod's keybinds
/// and those another of its presets names, go back to their defaults.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, ts_rs::TS)]
#[ts(rename = "PluginPreset")]
pub struct Preset {
    pub id: String,
    pub name: String,
    #[serde(default)]
    #[ts(type = "Record<string, unknown>")]
    pub values: Map<String, Value>,
}

#[derive(Debug, Clone, Deserialize, PartialEq, Default)]
pub struct Requirement {
    pub id: String,
    #[serde(default)]
    pub version: Option<String>,
}

#[derive(Debug, Clone, Deserialize, PartialEq, Default)]
pub struct Conflict {
    pub id: String,
    #[serde(default)]
    pub reason: String,
}

#[derive(Debug, Clone, Deserialize, PartialEq, Default)]
pub struct LoadHints {
    #[serde(default)]
    pub after: Vec<String>,
    #[serde(default)]
    pub before: Vec<String>,
}

/// `openrac-mod.json`, format 2.
#[derive(Debug, Clone, Deserialize, PartialEq, Default)]
pub struct Manifest {
    pub format: u32,
    #[serde(default)]
    pub id: String,
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub author: String,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub license: String,
    #[serde(default)]
    pub homepage: String,
    #[serde(default)]
    pub api: Option<String>,
    #[serde(default)]
    pub games: BTreeMap<String, Vec<String>>,
    #[serde(default)]
    pub plugin: BTreeMap<String, String>,
    /// `native` (the default, also when absent) or `script`.
    #[serde(default, rename = "type")]
    pub kind: String,
    /// A script mod's entry files, run in this order, relative to its folder.
    #[serde(default)]
    pub scripts: Vec<String>,
    #[serde(default)]
    pub hooks: Vec<String>,
    #[serde(default)]
    pub settings: Vec<SettingSpec>,
    /// Presets of its settings (interface 1.2).
    #[serde(default)]
    pub presets: Vec<Preset>,
    #[serde(default)]
    pub requires: Vec<Requirement>,
    #[serde(default)]
    pub conflicts: Vec<Conflict>,
    #[serde(default)]
    pub load: LoadHints,
}

/// An id: lower-case letters and digits in words joined by hyphens, at most 64 characters.
pub fn valid_id(id: &str) -> bool {
    !id.is_empty()
        && id.len() <= 64
        && id.split('-').all(|w| !w.is_empty() && w.chars().all(|c| c.is_ascii_lowercase() || c.is_ascii_digit()))
}

fn valid_key(key: &str) -> bool {
    !key.is_empty() && key.chars().all(|c| c.is_ascii_lowercase() || c.is_ascii_digit() || c == '_')
}

/// `MAJOR.MINOR.PATCH` with an optional `-suffix`: the numbers, and whether there is a suffix.
pub fn parse_version(text: &str) -> Option<(u64, u64, u64, bool)> {
    let (core, pre) = match text.split_once('-') {
        Some((core, suffix)) if !suffix.is_empty() => (core, true),
        Some(_) => return None,
        None => (text, false),
    };
    let parts: Vec<&str> = core.split('.').collect();
    if parts.len() != 3 {
        return None;
    }
    let number = |p: &str| (!p.is_empty() && p.chars().all(|c| c.is_ascii_digit())).then(|| p.parse().ok()).flatten();
    Some((number(parts[0])?, number(parts[1])?, number(parts[2])?, pre))
}

/// `a` against `b`: a version with a suffix comes before the same numbers without one.
pub(crate) fn compare_versions(a: (u64, u64, u64, bool), b: (u64, u64, u64, bool)) -> std::cmp::Ordering {
    (a.0, a.1, a.2, !a.3).cmp(&(b.0, b.1, b.2, !b.3))
}

/// `MAJOR.MINOR`.
pub fn parse_api(text: &str) -> Option<(u32, u32)> {
    let (major, minor) = text.split_once('.')?;
    let number = |p: &str| (!p.is_empty() && p.chars().all(|c| c.is_ascii_digit())).then(|| p.parse().ok()).flatten();
    Some((number(major)?, number(minor)?))
}

/// Whether `version` meets `wanted` (`>=X.Y.Z`, `=X.Y.Z`, or none). Err: `wanted` is not one.
fn meets(version: &str, wanted: Option<&str>) -> Result<bool, String> {
    let Some(wanted) = wanted.map(str::trim).filter(|w| !w.is_empty()) else { return Ok(true) };
    let (exact, number) = if let Some(rest) = wanted.strip_prefix(">=") {
        (false, rest)
    } else if let Some(rest) = wanted.strip_prefix('=') {
        (true, rest)
    } else {
        return Err(format!("the version wanted, {wanted:?}, is not >=X.Y.Z or =X.Y.Z"));
    };
    let want = parse_version(number.trim()).ok_or_else(|| format!("{wanted:?} is not a version"))?;
    let Some(have) = parse_version(version) else { return Ok(false) };
    let order = compare_versions(have, want);
    Ok(if exact { order.is_eq() } else { order.is_ge() })
}

impl Manifest {
    /// Whether it is a script mod (Luau, sandboxed: MODS.md, section 11).
    pub fn is_script(&self) -> bool {
        self.kind == "script"
    }

    /// What it is, as the Mods tab tells it: `script`, `native` (a library), or `data`.
    pub fn kind_name(&self) -> &'static str {
        if self.is_script() {
            "script"
        } else if self.plugin.is_empty() {
            "data"
        } else {
            "native"
        }
    }

    /// The rules of MODS.md, section 3, that need nothing but the file: the first broken one.
    pub fn validate(&self) -> Result<(), String> {
        if self.format != 2 {
            return Err(format!("format {} is not a native mod's (2)", self.format));
        }
        if !valid_id(&self.id) {
            return Err(format!("its id {:?} is not lower-case words joined by hyphens", self.id));
        }
        if self.name.trim().is_empty() {
            return Err("it has no name".into());
        }
        if parse_version(&self.version).is_none() {
            return Err(format!("its version {:?} is not MAJOR.MINOR.PATCH", self.version));
        }
        if !self.homepage.is_empty() && !self.homepage.starts_with("https://") {
            return Err("its homepage is not an https:// address".into());
        }
        if self.games.is_empty() || self.games.values().any(Vec::is_empty) {
            return Err("it names no game, or a game with no disc".into());
        }
        match self.kind.as_str() {
            "" | "native" | "script" => {}
            other => return Err(format!("its type {other:?} is not native or script")),
        }
        if self.is_script() {
            if !self.plugin.is_empty() {
                return Err("a script mod carries no library (\"plugin\")".into());
            }
            if self.scripts.is_empty() {
                return Err("a script mod names no scripts".into());
            }
            for path in &self.scripts {
                if !safe_relative(path) || !path.ends_with(".luau") {
                    return Err(format!("its script {path:?} is not a .luau file inside the mod"));
                }
            }
            let api = self.api.as_deref().ok_or("a script mod has no \"api\"")?;
            let (major, minor) = parse_api(api).ok_or_else(|| format!("its api {api:?} is not MAJOR.MINOR"))?;
            if major == API_MAJOR && minor < SCRIPT_API_MINOR {
                return Err(format!("its api {api} is older than the first with scripts (1.{SCRIPT_API_MINOR})"));
            }
        } else if !self.scripts.is_empty() {
            return Err("only a script mod (\"type\": \"script\") names scripts".into());
        }
        if !self.plugin.is_empty() {
            let api = self.api.as_deref().ok_or("it has a library but no \"api\"")?;
            parse_api(api).ok_or_else(|| format!("its api {api:?} is not MAJOR.MINOR"))?;
        }
        for (platform, path) in &self.plugin {
            if !PLATFORMS.contains(&platform.as_str()) {
                return Err(format!("{platform:?} is not a platform OpenRAC knows"));
            }
            if !safe_relative(path) {
                return Err(format!("its library for {platform} is not a path inside the mod"));
            }
        }
        let mut keys = BTreeSet::new();
        for s in &self.settings {
            if !valid_key(&s.key) || !keys.insert(s.key.as_str()) {
                return Err(format!("setting {:?}: the key is not [a-z0-9_]+, or is there twice", s.key));
            }
            match s.kind.as_str() {
                "bool" | "int" | "float" | "string" | "keybind" => {}
                "choice" if s.choices.len() >= 2 => {}
                "choice" => return Err(format!("setting {:?}: a choice needs two or more choices", s.key)),
                other => return Err(format!("setting {:?}: no type {other:?}", s.key)),
            }
            if s.binds.is_some() && (s.kind != "keybind" || !matches!(s.binds, Some(1 | 2))) {
                return Err(format!("setting {:?}: \"binds\" is 1 or 2, for a keybind", s.key));
            }
            if check_value(s, &s.default).is_none() {
                return Err(format!("setting {:?}: its default is not one of its values", s.key));
            }
        }
        let mut presets = BTreeSet::new();
        for p in &self.presets {
            if !valid_key(&p.id) || p.name.trim().is_empty() || !presets.insert(p.id.as_str()) {
                return Err(format!("preset {:?}: its id is not [a-z0-9_]+, is there twice, or it has no name", p.id));
            }
            for (key, value) in &p.values {
                let Some(spec) = self.settings.iter().find(|s| s.key == *key) else {
                    return Err(format!("preset {:?}: {key} is not a setting of the mod", p.id));
                };
                if check_value(spec, value).is_none() {
                    return Err(format!("preset {:?}: {value} is not a value of {key}", p.id));
                }
            }
        }
        for r in &self.requires {
            meets("0.0.0", r.version.as_deref()).map_err(|e| format!("requires {}: {e}", r.id))?;
        }
        Ok(())
    }

    /// Whether it runs on `game`'s disc `version`, and a library for this system: why not.
    pub fn runs_on(&self, game: &str, version: &str, platform: Option<&str>) -> Result<(), String> {
        let discs = self.games.get(game).ok_or_else(|| format!("It is not for {game}."))?;
        if !discs.iter().any(|d| d == version) {
            return Err(format!("Not for this disc ({version}): it runs on {}.", discs.join(", ")));
        }
        if self.plugin.is_empty() && !self.is_script() {
            return Ok(());
        }
        let (major, minor) = parse_api(self.api.as_deref().unwrap_or_default()).unwrap_or_default();
        if major != API_MAJOR || minor > API_MINOR {
            return Err(format!(
                "Needs OpenRAC with mod interface {major}.{minor}; this one has {API_MAJOR}.{API_MINOR}."
            ));
        }
        // A script runs wherever the port does, Android included.
        if self.is_script() {
            return Ok(());
        }
        match platform {
            Some(p) if self.plugin.contains_key(p) => Ok(()),
            Some(p) => Err(format!("Not built for this computer ({p}).")),
            None => Err("Native mods do not run on this system.".into()),
        }
    }
}

/// A path inside a folder: relative, no `..`, no drive.
pub(crate) fn safe_relative(path: &str) -> bool {
    let p = Path::new(path);
    !path.is_empty() && !path.contains('\\') && p.components().all(|c| matches!(c, std::path::Component::Normal(_)))
}

/// `value` as a setting takes it, or None when it is not one of its values.
pub fn check_value(spec: &SettingSpec, value: &Value) -> Option<Value> {
    let in_range = |x: f64| spec.min.is_none_or(|m| x >= m) && spec.max.is_none_or(|m| x <= m);
    match spec.kind.as_str() {
        "bool" => value.as_bool().map(Value::Bool),
        "int" => value.as_i64().filter(|&x| in_range(x as f64)).map(Value::from),
        "float" => value.as_f64().filter(|&x| x.is_finite() && in_range(x)).map(Value::from),
        "choice" => {
            value.as_str().filter(|v| spec.choices.iter().any(|c| c.value == *v)).map(|v| Value::String(v.into()))
        }
        "string" => value
            .as_str()
            .filter(|v| spec.max_length.is_none_or(|n| v.chars().count() <= n))
            .map(|v| Value::String(v.into())),
        "keybind" => value.as_str().and_then(|v| parse_binds(v, spec.most_binds())).map(Value::String),
        _ => None,
    }
}

/// Reads a manifest: the parsed file, or why it cannot be read.
pub fn read_manifest(folder: &Path) -> Result<Manifest, String> {
    let path = folder.join(MANIFEST);
    let text = std::fs::read_to_string(&path).map_err(|e| format!("{MANIFEST} cannot be read ({e})"))?;
    serde_json::from_str(&text).map_err(|e| format!("{MANIFEST} cannot be read ({e})"))
}

// ---- The list and the answer ------------------------------------------------------------------

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
struct Entry {
    id: String,
    #[serde(default)]
    enabled: bool,
    #[serde(default)]
    settings: Map<String, Value>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct List {
    format: u32,
    mods: Vec<Entry>,
}

/// The installed mods' folders, by id (sorted).
fn installed(dir: &Path) -> Vec<String> {
    let mut ids: Vec<String> = std::fs::read_dir(dir)
        .map(|entries| {
            entries
                .filter_map(Result::ok)
                .filter(|e| e.path().is_dir())
                .filter_map(|e| e.file_name().to_str().map(str::to_string))
                .filter(|id| valid_id(id) && dir.join(id).join(MANIFEST).is_file())
                .collect()
        })
        .unwrap_or_default();
    ids.sort();
    ids
}

/// The list as the port reads it: listed mods whose folder is there, in order, then the folders
/// not listed, off.
fn read_list(dir: &Path) -> Result<Vec<Entry>, String> {
    let path = dir.join(LIST);
    let mut mods = if path.is_file() {
        let text = std::fs::read_to_string(&path).map_err(|e| format!("{}: {e}", path.display()))?;
        let list: List = serde_json::from_str(&text).map_err(|e| format!("{} cannot be read ({e})", path.display()))?;
        if list.format != 1 {
            return Err(format!("{} is of a newer format ({}): update the launcher", path.display(), list.format));
        }
        list.mods
    } else {
        Vec::new()
    };
    let there = installed(dir);
    let mut seen = BTreeSet::new();
    mods.retain(|e| there.contains(&e.id) && seen.insert(e.id.clone()));
    for id in there {
        if !seen.contains(&id) {
            mods.push(Entry { id, enabled: false, settings: Map::new() });
        }
    }
    Ok(mods)
}

fn write_list(dir: &Path, mods: &[Entry]) -> Result<(), String> {
    std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    let path = dir.join(LIST);
    let temporary = dir.join(format!("{LIST}.tmp"));
    let list = List { format: 1, mods: mods.to_vec() };
    let text = serde_json::to_string_pretty(&list).map_err(|e| e.to_string())? + "\n";
    if let Err(e) = std::fs::write(&temporary, text) {
        let _ = std::fs::remove_file(&temporary);
        return Err(format!("{}: {e}", temporary.display()));
    }
    if let Err(e) = std::fs::rename(&temporary, &path) {
        let _ = std::fs::remove_file(&temporary);
        return Err(format!("{}: {e}", path.display()));
    }
    Ok(())
}

/// One mod in the port's `status.json`.
#[derive(Debug, Clone, Deserialize, Serialize, PartialEq, Default, ts_rs::TS)]
#[ts(rename = "PluginStatusEntry")]
#[serde(rename_all = "camelCase")]
pub struct StatusEntry {
    pub id: String,
    #[serde(default)]
    pub version: String,
    pub state: String,
    #[serde(default)]
    pub reason: String,
    #[serde(default)]
    pub hooks: Vec<String>,
}

/// The port's `status.json`: what it did with each mod at its last start.
#[derive(Debug, Clone, Deserialize, Serialize, PartialEq, Default, ts_rs::TS)]
#[ts(rename = "PluginStatus")]
#[serde(rename_all = "camelCase")]
pub struct Status {
    #[serde(default)]
    pub time: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub purist: bool,
    #[serde(default)]
    pub mods: Vec<StatusEntry>,
    #[serde(default, alias = "hook_points")]
    pub hook_points: Vec<String>,
}

/// The last start's answer, when the port wrote one.
pub fn read_status(dir: &Path) -> Option<Status> {
    serde_json::from_str(&std::fs::read_to_string(dir.join(STATUS)).ok()?).ok()
}

// ---- What the Mods tab shows --------------------------------------------------------------------

/// A setting with its value now.
#[derive(Debug, Clone, Serialize, PartialEq, ts_rs::TS)]
#[ts(rename = "PluginSetting")]
#[serde(rename_all = "camelCase")]
pub struct SettingView {
    #[serde(flatten)]
    pub spec: SettingSpec,
    #[ts(type = "unknown")]
    pub value: Value,
    /// A keybind's: the names of the mod's other keybinds of its group bound to one of its inputs.
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    #[ts(as = "Option<Vec<String>>", optional)]
    pub conflicts: Vec<String>,
}

/// Fills each keybind's `conflicts`: the other keybinds of its group that share one of its binds.
pub fn mark_conflicts(settings: &mut [SettingView]) {
    let binds: Vec<Vec<String>> = settings
        .iter()
        .map(|s| match (s.spec.kind.as_str(), s.value.as_str()) {
            ("keybind", Some(v)) => v.split(',').filter(|n| !n.is_empty()).map(String::from).collect(),
            _ => Vec::new(),
        })
        .collect();
    for i in 0..settings.len() {
        let with: Vec<String> = (0..settings.len())
            .filter(|&j| {
                j != i
                    && settings[j].spec.group == settings[i].spec.group
                    && binds[i].iter().any(|b| binds[j].contains(b))
            })
            .map(|j| settings[j].spec.name.clone())
            .collect();
        settings[i].conflicts = with;
    }
}

/// A hook point this mod shares with other mods that are on.
#[derive(Debug, Clone, Serialize, PartialEq, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct SharedHook {
    pub hook: String,
    pub with: Vec<String>,
}

/// A mod as the Mods tab shows it.
#[derive(Debug, Clone, Serialize, PartialEq, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct PluginView {
    pub id: String,
    pub name: String,
    pub version: String,
    pub author: String,
    pub description: String,
    pub homepage: String,
    pub license: String,
    pub enabled: bool,
    /// `ready` (on, and nothing against it), `off`, `refused` (it cannot load: `reason`), or
    /// `failed` (the port did not load it at its last start: `reason`).
    #[ts(type = "\"ready\" | \"off\" | \"refused\" | \"failed\"")]
    pub state: String,
    pub reason: String,
    /// Off, and could not be switched on as things are: why (also for a mod that is off).
    pub blocked: String,
    pub settings: Vec<SettingView>,
    /// Presets of its settings the mod offers (its manifest's).
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    #[ts(as = "Option<Vec<Preset>>", optional)]
    pub presets: Vec<Preset>,
    pub hooks: Vec<String>,
    pub shared_hooks: Vec<SharedHook>,
    /// Whether it carries code (a library) or only data.
    pub native: bool,
    /// `native` (runs code on this computer), `script` (runs scripts, sandboxed) or `data`.
    #[ts(type = "\"native\" | \"script\" | \"data\" | \"built-in\"")]
    pub kind: String,
    /// The installed folder (developer tools show it).
    pub path: String,
}

/// The mods of a game, in load order, for its disc `version` ("pal").
pub fn list(dir: &Path, game: &str, version: &str) -> Result<Vec<PluginView>, String> {
    let mods = read_list(dir)?;
    let manifests: Vec<Result<Manifest, String>> =
        mods.iter().map(|e| read_manifest(&dir.join(&e.id)).and_then(|m| m.validate().map(|()| m))).collect();
    let status = read_status(dir);
    let platform = platform_key();
    let mut out = Vec::new();
    for (i, entry) in mods.iter().enumerate() {
        let folder = dir.join(&entry.id);
        let manifest = manifests[i].as_ref();
        let m = manifest.cloned().unwrap_or_default();
        // What stops it, on or off: the file, the disc, the system, then the other mods.
        let mut why = match manifest {
            Err(e) => Some(format!("Cannot be read: {e}")),
            Ok(m) if m.id != entry.id => Some(format!("Its manifest says it is {:?}.", m.id)),
            Ok(m) => m.runs_on(game, version, platform).err(),
        };
        if why.is_none() {
            why = against_others(&m, i, &mods, &manifests, false);
        }
        let blocked = if entry.enabled {
            String::new()
        } else {
            why.clone().or_else(|| against_others(&m, i, &mods, &manifests, true)).unwrap_or_default()
        };
        let (mut state, mut reason) = match (&why, entry.enabled) {
            (Some(why), true) => ("refused", why.clone()),
            (_, false) => ("off", String::new()),
            (None, true) => ("ready", String::new()),
        };
        // The port's answer, when it is about this version of the mod and it was not loaded.
        if state == "ready" {
            if let Some(s) = status.as_ref().and_then(|s| s.mods.iter().find(|s| s.id == entry.id)) {
                if (s.version.is_empty() || s.version == m.version) && s.state != "loaded" {
                    state = "failed";
                    reason = format!("Not loaded at the last start: {}", s.reason);
                }
            }
        }
        let mut settings: Vec<SettingView> = m
            .settings
            .iter()
            .map(|spec| SettingView {
                spec: spec.clone(),
                value: entry.settings.get(&spec.key).and_then(|v| check_value(spec, v)).unwrap_or(spec.default.clone()),
                conflicts: Vec::new(),
            })
            .collect();
        mark_conflicts(&mut settings);
        // By permanent id (hook_ids.rs): one mod may name a hook point by its name, another by its id.
        let shared_hooks = m
            .hooks
            .iter()
            .filter_map(|hook| {
                let hid = crate::hook_ids::id(game, hook);
                let with: Vec<String> = mods
                    .iter()
                    .zip(&manifests)
                    .filter(|(e, other)| {
                        e.id != entry.id
                            && e.enabled
                            && other.as_ref().is_ok_and(|o| o.hooks.iter().any(|h| crate::hook_ids::id(game, h) == hid))
                    })
                    .map(|(e, other)| other.as_ref().map_or(e.id.clone(), |o| o.name.clone()))
                    .collect();
                (!with.is_empty()).then(|| SharedHook { hook: hook.clone(), with })
            })
            .collect();
        out.push(PluginView {
            id: entry.id.clone(),
            name: if m.name.is_empty() { entry.id.clone() } else { m.name.clone() },
            version: m.version.clone(),
            author: m.author.clone(),
            description: m.description.clone(),
            homepage: m.homepage.clone(),
            license: m.license.clone(),
            enabled: entry.enabled,
            state: state.into(),
            reason,
            blocked,
            settings,
            presets: m.presets.clone(),
            hooks: m.hooks.clone(),
            shared_hooks,
            native: !m.plugin.is_empty(),
            kind: m.kind_name().into(),
            path: folder.display().to_string(),
        });
    }
    Ok(out)
}

/// What the other mods say against mod `at` of the list: a dependency off, missing, too old or
/// later in the list, or a conflict with a mod that is on. `as_if_on`: for a mod that is off,
/// whether it could be switched on (a dependency's place is then not held against it: switching
/// on moves it after).
fn against_others(
    m: &Manifest,
    at: usize,
    mods: &[Entry],
    manifests: &[Result<Manifest, String>],
    as_if_on: bool,
) -> Option<String> {
    let find = |id: &str| mods.iter().position(|e| e.id == id);
    for r in &m.requires {
        let Some(j) = find(&r.id) else {
            return Some(format!("Needs the mod {}, which is not installed.", r.id));
        };
        let Ok(other) = &manifests[j] else {
            return Some(format!("Needs the mod {}, which cannot be read.", r.id));
        };
        if !meets(&other.version, r.version.as_deref()).unwrap_or(false) {
            return Some(format!(
                "Needs {} {}; {} is installed.",
                other.name,
                r.version.as_deref().unwrap_or(""),
                other.version
            ));
        }
        if !mods[j].enabled {
            return Some(format!("Needs {}: switch it on.", other.name));
        }
        if j > at && !as_if_on {
            return Some(format!("Needs {} to load first: move it above.", other.name));
        }
    }
    for (j, (e, other)) in mods.iter().zip(manifests).enumerate() {
        if j == at || !e.enabled {
            continue;
        }
        let Ok(other) = other else { continue };
        let mine = m.conflicts.iter().find(|c| c.id == e.id);
        let theirs = other.conflicts.iter().find(|c| c.id == m.id);
        if let Some(c) = mine.or(theirs) {
            let why = if c.reason.is_empty() { String::new() } else { format!(": {}", c.reason) };
            return Some(format!("Conflicts with {}{why}.", other.name));
        }
    }
    None
}

// ---- Changes --------------------------------------------------------------------------------

fn position(mods: &[Entry], id: &str) -> Result<usize, String> {
    mods.iter().position(|e| e.id == id).ok_or_else(|| format!("no mod {id}"))
}

/// Switches a mod on or off. On: refused, with the reason, when it cannot run here or another mod
/// stands against it; otherwise moved where its dependencies and its own and the others' load hints
/// want it.
pub fn set_enabled(dir: &Path, game: &str, version: &str, id: &str, on: bool) -> Result<Vec<PluginView>, String> {
    let mut mods = read_list(dir)?;
    let at = position(&mods, id)?;
    if on {
        let manifests: Vec<Result<Manifest, String>> =
            mods.iter().map(|e| read_manifest(&dir.join(&e.id)).and_then(|m| m.validate().map(|()| m))).collect();
        let m = manifests[at].clone().map_err(|e| format!("{id} cannot be read: {e}"))?;
        m.runs_on(game, version, platform_key())?;
        if let Some(why) = against_others(&m, at, &mods, &manifests, true) {
            return Err(why);
        }
        mods[at].enabled = true;
        place(&mut mods, at, &manifests);
    } else {
        mods[at].enabled = false;
    }
    write_list(dir, &mods)?;
    list(dir, game, version)
}

/// Moves mod `at`, just switched on, after what it must load after and before what it must load
/// before, as far as both can be met: its dependencies and `load.after`, the mods whose
/// `load.before` names it, then its `load.before` and the mods whose `load.after` or `requires`
/// name it.
fn place(mods: &mut Vec<Entry>, at: usize, manifests: &[Result<Manifest, String>]) {
    let Ok(m) = &manifests[at] else { return };
    let mut after: BTreeSet<&str> = m.requires.iter().map(|r| r.id.as_str()).collect();
    after.extend(m.load.after.iter().map(String::as_str));
    let mut before: BTreeSet<&str> = m.load.before.iter().map(String::as_str).collect();
    for (e, other) in mods.iter().zip(manifests) {
        let Ok(other) = other else { continue };
        if other.load.before.contains(&m.id) {
            after.insert(e.id.as_str());
        }
        if other.load.after.contains(&m.id) || other.requires.iter().any(|r| r.id == m.id) {
            before.insert(e.id.as_str());
        }
    }
    let low = mods.iter().enumerate().filter(|(_, e)| after.contains(e.id.as_str())).map(|(i, _)| i + 1).max();
    let high = mods.iter().enumerate().filter(|(_, e)| before.contains(e.id.as_str())).map(|(i, _)| i).min();
    let fits = low.is_none_or(|l| at >= l) && high.is_none_or(|h| at < h);
    if fits {
        return;
    }
    let entry = mods.remove(at);
    // Indices after `at` move down by one now that it is out.
    let shift = |i: usize| if i > at { i - 1 } else { i };
    let to = match (low.map(shift), high.map(shift)) {
        (Some(l), _) => l,
        (None, Some(h)) => h,
        (None, None) => at,
    };
    mods.insert(to.min(mods.len()), entry);
}

/// Moves a mod one place earlier (`up`) or later in the load order.
pub fn move_mod(dir: &Path, game: &str, version: &str, id: &str, up: bool) -> Result<Vec<PluginView>, String> {
    let mut mods = read_list(dir)?;
    let at = position(&mods, id)?;
    let to = if up { at.saturating_sub(1) } else { (at + 1).min(mods.len() - 1) };
    let entry = mods.remove(at);
    mods.insert(to, entry);
    write_list(dir, &mods)?;
    list(dir, game, version)
}

/// Sets one of a mod's settings, after checking it against the mod's manifest.
pub fn set_setting(
    dir: &Path,
    game: &str,
    version: &str,
    id: &str,
    key: &str,
    value: Value,
) -> Result<Vec<PluginView>, String> {
    let mut mods = read_list(dir)?;
    let at = position(&mods, id)?;
    let m = read_manifest(&dir.join(id))?;
    let spec = m.settings.iter().find(|s| s.key == key).ok_or_else(|| format!("{id} has no setting {key}"))?;
    let value = check_value(spec, &value).ok_or_else(|| format!("{value} is not a value of {}", spec.name))?;
    mods[at].settings.insert(key.to_string(), value);
    write_list(dir, &mods)?;
    list(dir, game, version)
}

/// Removes a mod: its folder in the store and its line in the list.
pub fn remove(dir: &Path, game: &str, version: &str, id: &str) -> Result<Vec<PluginView>, String> {
    if !valid_id(id) {
        return Err(format!("no mod {id}"));
    }
    let mut mods = read_list(dir)?;
    let at = position(&mods, id)?;
    mods.remove(at);
    let folder = dir.join(id);
    if folder.join(MANIFEST).is_file() {
        std::fs::remove_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
    }
    write_list(dir, &mods)?;
    list(dir, game, version)
}

// ---- Installing ---------------------------------------------------------------------------

/// What adding a file or folder came to.
#[derive(Debug, Clone, Serialize, PartialEq, ts_rs::TS)]
#[ts(rename = "PluginInstalled")]
#[serde(rename_all = "camelCase", tag = "kind")]
pub enum Installed {
    /// A native mod, now in the store (`replaced`: an older install of it was there).
    Plugin { id: String, name: String, version: String, replaced: bool, enabled: bool },
    /// A data pack (format 1): the overrides' to add (editor/overrides.py, actions.json `overrides-add`).
    Pack,
}

/// A folder of our own beside the store, which a rename moves into place (the same file system).
pub(crate) struct Scratch(pub(crate) PathBuf);

impl Scratch {
    pub(crate) fn new(dir: &Path, what: &str) -> Result<Scratch, String> {
        let stamp = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).map_or(0, |d| d.as_nanos());
        let path = dir.join(format!(".{what}-{}-{stamp}", std::process::id()));
        std::fs::create_dir_all(&path).map_err(|e| format!("{}: {e}", path.display()))?;
        Ok(Scratch(path))
    }
}

impl Drop for Scratch {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

/// Where the manifest is in an unpacked mod: its root, or its one folder (a zip of a folder).
/// Hidden entries and macOS's `__MACOSX` do not count.
pub(crate) fn manifest_root(folder: &Path) -> Result<PathBuf, String> {
    if folder.join(MANIFEST).is_file() {
        return Ok(folder.to_path_buf());
    }
    let entries: Vec<PathBuf> = std::fs::read_dir(folder)
        .map_err(|e| format!("{}: {e}", folder.display()))?
        .filter_map(Result::ok)
        .filter(|e| {
            let name = e.file_name();
            let name = name.to_string_lossy();
            !name.starts_with('.') && name != "__MACOSX"
        })
        .map(|e| e.path())
        .collect();
    match entries.as_slice() {
        [one] if one.is_dir() && one.join(MANIFEST).is_file() => Ok(one.clone()),
        _ => Err(format!("there is no {MANIFEST} at its top: it is not an OpenRAC mod")),
    }
}

/// The mods a folder the player chose holds, each one to install: the folder itself when it is one
/// mod (its manifest at its top or in its one folder), else every folder at its top that is one.
/// A checkout of a mods repository (rac1-mods) is taken from its build, `build/package/<id>`,
/// first: there a native mod has its library, which its source folder does not. A .zip is
/// returned as it is (install says what it holds).
pub fn mod_folders(source: &Path) -> Result<Vec<PathBuf>, String> {
    if !source.is_dir() || manifest_root(source).is_ok() {
        return Ok(vec![source.to_path_buf()]);
    }
    let mods_in = |folder: &Path| -> Vec<PathBuf> {
        let mut found: Vec<PathBuf> =
            std::fs::read_dir(folder).map(|d| d.filter_map(Result::ok).map(|e| e.path()).collect()).unwrap_or_default();
        found.retain(|p| p.is_dir() && p.join(MANIFEST).is_file());
        found.sort();
        found
    };
    let built = mods_in(&source.join("build").join("package"));
    if !built.is_empty() {
        return Ok(built);
    }
    let found = mods_in(source);
    if found.is_empty() {
        return Err(format!("there is no {MANIFEST} at its top or in a folder at its top: it holds no OpenRAC mod"));
    }
    Ok(found)
}

/// Copies a folder, refusing links (a mod is files and folders only).
pub(crate) fn copy_tree(from: &Path, to: &Path) -> Result<(), String> {
    std::fs::create_dir_all(to).map_err(|e| format!("{}: {e}", to.display()))?;
    for entry in std::fs::read_dir(from).map_err(|e| format!("{}: {e}", from.display()))? {
        let entry = entry.map_err(|e| e.to_string())?;
        let kind = entry.file_type().map_err(|e| e.to_string())?;
        let target = to.join(entry.file_name());
        // A repository's own metadata is not the mod's (a script mod taken from a clone).
        if entry.file_name() == ".git" {
            continue;
        }
        if kind.is_symlink() {
            return Err(format!("{} is a link: a mod holds files and folders only", entry.path().display()));
        } else if kind.is_dir() {
            copy_tree(&entry.path(), &target)?;
        } else {
            std::fs::copy(entry.path(), &target).map_err(|e| format!("{}: {e}", entry.path().display()))?;
        }
    }
    Ok(())
}

/// Unpacks a .zip into `into`, refusing a path out of it (`..`, an absolute path) and links.
pub(crate) fn unzip(zip_path: &Path, into: &Path) -> Result<(), String> {
    let file = std::fs::File::open(zip_path).map_err(|e| format!("{}: {e}", zip_path.display()))?;
    let mut archive = zip::ZipArchive::new(file).map_err(|e| format!("{} is not a .zip ({e})", zip_path.display()))?;
    for i in 0..archive.len() {
        let mut entry = archive.by_index(i).map_err(|e| e.to_string())?;
        let name = entry.name().to_string();
        let Some(relative) = entry.enclosed_name().filter(|p| safe_relative(&p.to_string_lossy().replace('\\', "/")))
        else {
            return Err(format!("the .zip names a file outside its folder ({name})"));
        };
        // A name the archive's own reading would make safe is refused all the same: a mod made
        // that way is not one to install.
        if name.starts_with(['/', '\\']) || name.contains(':') || name.split(['/', '\\']).any(|p| p == "..") {
            return Err(format!("the .zip names a file outside its folder ({name})"));
        }
        if entry.is_symlink() {
            return Err(format!("the .zip holds a link ({name}): a mod holds files and folders only"));
        }
        let target = into.join(relative);
        if entry.is_dir() {
            std::fs::create_dir_all(&target).map_err(|e| format!("{}: {e}", target.display()))?;
            continue;
        }
        if let Some(parent) = target.parent() {
            std::fs::create_dir_all(parent).map_err(|e| format!("{}: {e}", parent.display()))?;
        }
        let mut bytes = Vec::new();
        entry.read_to_end(&mut bytes).map_err(|e| format!("{name}: {e}"))?;
        std::fs::write(&target, bytes).map_err(|e| format!("{}: {e}", target.display()))?;
    }
    Ok(())
}

/// Takes macOS's download quarantine off what was installed: the system refuses to open a library
/// that carries it. Elsewhere nothing.
fn unquarantine(folder: &Path) {
    if cfg!(target_os = "macos") {
        let _ = std::process::Command::new("/usr/bin/xattr")
            .arg("-dr")
            .arg("com.apple.quarantine")
            .arg(folder)
            .stdout(std::process::Stdio::null())
            .stderr(std::process::Stdio::null())
            .status();
    }
}

/// Adds a mod from a .zip or a folder to `game`'s store `dir`. A native mod (format 2) is copied
/// to `<id>/`, replacing an older install of it (its place in the list and its settings are kept)
/// and switched on when it can run on `version`; a data pack (format 1) is left for the overrides
/// ([`Installed::Pack`]). A mod for another game is refused.
pub fn install(dir: &Path, game: &str, version: &str, source: &Path) -> Result<Installed, String> {
    std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    let unpacked = Scratch::new(dir, "unpack")?;
    let root = if source.is_dir() {
        manifest_root(source)?
    } else if source.extension().is_some_and(|e| e.eq_ignore_ascii_case("zip")) {
        unzip(source, &unpacked.0)?;
        manifest_root(&unpacked.0)?
    } else {
        return Err(format!("{} is not a .zip or a folder", source.display()));
    };
    let raw: Value = serde_json::from_str(
        &std::fs::read_to_string(root.join(MANIFEST)).map_err(|e| format!("{MANIFEST} cannot be read ({e})"))?,
    )
    .map_err(|e| format!("{MANIFEST} cannot be read ({e})"))?;
    if raw.get("format").and_then(Value::as_u64) == Some(1) {
        return Ok(Installed::Pack);
    }
    let m = read_manifest(&root)?;
    m.validate()?;
    if !m.games.contains_key(game) {
        let games: Vec<&str> = m.games.keys().map(String::as_str).collect();
        return Err(format!("{} is a mod for {}, not this game", m.name, games.join(", ")));
    }
    for path in m.plugin.values() {
        if !root.join(path).is_file() {
            return Err(format!("{} names a library it does not hold ({path})", m.name));
        }
    }
    for path in &m.scripts {
        if !root.join(path).is_file() {
            return Err(format!("{} names a script it does not hold ({path})", m.name));
        }
    }
    // The copy is made beside the store, then put in place with renames.
    let staged = Scratch::new(dir, "install")?;
    let copy = staged.0.join(&m.id);
    copy_tree(&root, &copy)?;
    unquarantine(&copy);
    let target = dir.join(&m.id);
    let replaced = target.exists();
    let old = staged.0.join(".old");
    if replaced {
        std::fs::rename(&target, &old).map_err(|e| format!("{}: {e}", target.display()))?;
    }
    if let Err(e) = std::fs::rename(&copy, &target) {
        if replaced {
            let _ = std::fs::rename(&old, &target);
        }
        return Err(format!("{}: {e}", target.display()));
    }
    let mut mods = read_list(dir)?;
    let at = position(&mods, &m.id)?;
    let mut enabled = mods[at].enabled;
    if !replaced {
        // New: on when it can run here and nothing stands against it.
        let manifests: Vec<Result<Manifest, String>> =
            mods.iter().map(|e| read_manifest(&dir.join(&e.id)).and_then(|m| m.validate().map(|()| m))).collect();
        if m.runs_on(game, version, platform_key()).is_ok() && against_others(&m, at, &mods, &manifests, true).is_none()
        {
            mods[at].enabled = true;
            place(&mut mods, at, &manifests);
            enabled = true;
        }
    }
    // Settings the new version no longer has, or whose values it no longer takes, go.
    if let Some(entry) = mods.iter_mut().find(|e| e.id == m.id) {
        entry.settings.retain(|key, value| m.settings.iter().any(|s| s.key == *key && check_value(s, value).is_some()));
    }
    write_list(dir, &mods)?;
    Ok(Installed::Plugin { id: m.id.clone(), name: m.name.clone(), version: m.version.clone(), replaced, enabled })
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;
    use std::io::Write;

    /// This system's platform, or one to stand for it where no native mod runs.
    fn here() -> &'static str {
        platform_key().unwrap_or("linux-x86_64")
    }

    fn manifest(id: &str, extra: Value) -> Value {
        let mut m = json!({
            "format": 2, "id": id, "name": format!("Mod {id}"), "version": "1.0.0", "author": "me",
            "api": "1.0", "games": {"rac1": ["pal", "ntsc"]},
            "plugin": {(here()): format!("lib/{}/lib{id}.so", here())},
        });
        for (k, v) in extra.as_object().unwrap() {
            m[k] = v.clone();
        }
        m
    }

    /// A mod's folder made at `folder`, with its library.
    fn write_mod(folder: &Path, m: &Value) {
        std::fs::create_dir_all(folder).unwrap();
        std::fs::write(folder.join(MANIFEST), serde_json::to_string_pretty(m).unwrap()).unwrap();
        for path in m["plugin"].as_object().into_iter().flat_map(|p| p.values()) {
            let lib = folder.join(path.as_str().unwrap());
            std::fs::create_dir_all(lib.parent().unwrap()).unwrap();
            std::fs::write(lib, b"\x7fELF").unwrap();
        }
    }

    fn installed_mod(dir: &Path, id: &str, extra: Value) {
        write_mod(&dir.join(id), &manifest(id, extra));
    }

    fn ids(views: &[PluginView]) -> Vec<&str> {
        views.iter().map(|v| v.id.as_str()).collect()
    }

    fn view<'a>(views: &'a [PluginView], id: &str) -> &'a PluginView {
        views.iter().find(|v| v.id == id).unwrap()
    }

    #[test]
    fn platform_keys() {
        assert_eq!(platform_key_for("macos", "aarch64"), Some("macos-arm64"));
        assert_eq!(platform_key_for("windows", "x86_64"), Some("windows-x86_64"));
        assert_eq!(platform_key_for("linux", "aarch64"), Some("linux-aarch64"));
        assert_eq!(platform_key_for("android", "aarch64"), None);
        assert!(platform_key().is_none() || PLATFORMS.contains(&platform_key().unwrap()));
    }

    #[test]
    fn versions_and_requirements() {
        assert_eq!(parse_version("1.2.3"), Some((1, 2, 3, false)));
        assert_eq!(parse_version("1.2.3-beta"), Some((1, 2, 3, true)));
        for bad in ["1.2", "1.2.3.4", "v1.2.3", "1.2.3-", "1..3", "1.2.x"] {
            assert_eq!(parse_version(bad), None, "{bad}");
        }
        assert_eq!(parse_api("1.0"), Some((1, 0)));
        assert_eq!(parse_api("1"), None);
        assert!(meets("1.2.0", Some(">=1.2.0")).unwrap());
        assert!(meets("1.10.0", Some(">=1.9.0")).unwrap());
        assert!(!meets("1.2.0-rc", Some(">=1.2.0")).unwrap(), "a pre-release comes before its release");
        assert!(!meets("1.1.9", Some(">=1.2.0")).unwrap());
        assert!(meets("2.0.0", Some("=2.0.0")).unwrap());
        assert!(!meets("2.0.1", Some("=2.0.0")).unwrap());
        assert!(meets("0.1.0", None).unwrap());
        assert!(meets("1.0.0", Some("^1.0")).is_err());
    }

    #[test]
    fn manifests_are_checked() {
        let good: Manifest = serde_json::from_value(manifest(
            "lock-strafe",
            json!({"settings": [
                {"key": "mode", "name": "Mode", "type": "choice", "default": "lock",
                 "choices": [{"value": "lock", "name": "Lock"}, {"value": "hold", "name": "Hold"}]},
                {"key": "speed", "name": "Speed", "type": "float", "default": 1.0, "min": 0.5, "max": 2.0},
            ]}),
        ))
        .unwrap();
        good.validate().unwrap();
        let broken = |extra: Value| -> String {
            let m: Manifest = serde_json::from_value(manifest("x", extra)).unwrap();
            m.validate().unwrap_err()
        };
        assert!(broken(json!({"format": 1})).contains("format"));
        assert!(broken(json!({"id": "Lock_Strafe"})).contains("id"));
        assert!(broken(json!({"id": "a--b"})).contains("id"));
        assert!(broken(json!({"version": "1.0"})).contains("version"));
        assert!(broken(json!({"api": "one"})).contains("api"));
        assert!(broken(json!({"games": {}})).contains("game"));
        assert!(broken(json!({"games": {"rac1": []}})).contains("game"));
        assert!(broken(json!({"plugin": {"amiga-68k": "x.so"}})).contains("platform"));
        assert!(broken(json!({"plugin": {"linux-x86_64": "../x.so"}})).contains("inside"));
        assert!(broken(json!({"plugin": {"linux-x86_64": "/x.so"}})).contains("inside"));
        assert!(broken(json!({"homepage": "http://example.org"})).contains("https"));
        assert!(
            broken(json!({"settings": [{"key": "a", "name": "A", "type": "colour", "default": 1}]})).contains("type")
        );
        assert!(broken(json!({"settings": [{"key": "a", "name": "A", "type": "int", "default": 9, "max": 5}]}))
            .contains("default"));
        assert!(broken(json!({"settings": [
            {"key": "a", "name": "A", "type": "bool", "default": true},
            {"key": "a", "name": "B", "type": "bool", "default": false}]}))
        .contains("twice"));
        assert!(broken(json!({"settings": [{"key": "a", "name": "A", "type": "choice", "default": "x",
                                            "choices": [{"value": "x", "name": "X"}]}]}))
        .contains("two or more"));
        assert!(broken(json!({"requires": [{"id": "b", "version": "~1"}]})).contains("requires b"));
    }

    #[test]
    fn where_a_mod_runs() {
        let m: Manifest = serde_json::from_value(manifest("a", json!({"games": {"rac1": ["pal"]}}))).unwrap();
        assert!(m.runs_on("rac1", "pal", Some(here())).is_ok());
        assert!(m.runs_on("rac1", "ntsc", Some(here())).unwrap_err().contains("Not for this disc"));
        assert!(m.runs_on("rac2", "ntsc", Some(here())).unwrap_err().contains("not for rac2"));
        assert!(m.runs_on("rac1", "pal", Some("windows-arm64")).unwrap_err().contains("Not built"));
        assert!(m.runs_on("rac1", "pal", None).unwrap_err().contains("do not run"));
        let newer: Manifest = serde_json::from_value(manifest("a", json!({"api": "1.9"}))).unwrap();
        assert!(newer.runs_on("rac1", "pal", Some(here())).unwrap_err().contains("interface 1.9"));
        let other: Manifest = serde_json::from_value(manifest("a", json!({"api": "2.0"}))).unwrap();
        assert!(other.runs_on("rac1", "pal", Some(here())).is_err());
        let data: Manifest = serde_json::from_value(manifest("a", json!({"plugin": {}, "api": null}))).unwrap();
        data.validate().unwrap();
        assert!(data.runs_on("rac1", "pal", None).is_ok(), "a data-only mod needs no library");
    }

    fn script(id: &str, extra: Value) -> Value {
        let mut m = manifest(id, json!({"type": "script", "scripts": ["main.luau"], "plugin": {}, "api": "1.1"}));
        for (k, v) in extra.as_object().unwrap() {
            m[k] = v.clone();
        }
        m
    }

    #[test]
    fn script_mods_are_checked() {
        let good: Manifest = serde_json::from_value(script("swap", json!({}))).unwrap();
        good.validate().unwrap();
        assert!(good.is_script());
        assert_eq!(good.kind_name(), "script");
        let broken = |extra: Value| -> String {
            let m: Manifest = serde_json::from_value(script("x", extra)).unwrap();
            m.validate().unwrap_err()
        };
        assert!(broken(json!({"plugin": {"linux-x86_64": "x.so"}})).contains("no library"));
        assert!(broken(json!({"scripts": []})).contains("names no scripts"));
        assert!(broken(json!({"scripts": ["../main.luau"]})).contains("inside the mod"));
        assert!(broken(json!({"scripts": ["/main.luau"]})).contains("inside the mod"));
        assert!(broken(json!({"scripts": ["main.lua"]})).contains(".luau"));
        assert!(broken(json!({"api": "1.0"})).contains("first with scripts"));
        assert!(broken(json!({"api": null})).contains("no \"api\""));
        assert!(broken(json!({"type": "dll"})).contains("not native or script"));
        // Only a script mod names scripts; "native" is the default and may be written.
        let native = |extra: Value| -> Result<(), String> {
            serde_json::from_value::<Manifest>(manifest("n", extra)).unwrap().validate()
        };
        assert!(native(json!({"scripts": ["main.luau"]})).unwrap_err().contains("only a script mod"));
        native(json!({"type": "native"})).unwrap();
        // A native mod built for interface 1.0 still loads on 1.1.
        let old: Manifest = serde_json::from_value(manifest("n", json!({"api": "1.0"}))).unwrap();
        assert!(old.runs_on("rac1", "pal", Some(here())).is_ok());
    }

    #[test]
    fn a_script_mod_runs_on_every_system() {
        let m: Manifest = serde_json::from_value(script("swap", json!({"games": {"rac1": ["pal"]}}))).unwrap();
        // Android has no platform key for native mods; a script runs there all the same.
        assert!(m.runs_on("rac1", "pal", None).is_ok());
        assert!(m.runs_on("rac1", "pal", Some("windows-arm64")).is_ok());
        assert!(m.runs_on("rac1", "ntsc", None).unwrap_err().contains("Not for this disc"));
        let newer: Manifest = serde_json::from_value(script("swap", json!({"api": "1.7"}))).unwrap();
        assert!(newer.runs_on("rac1", "pal", None).unwrap_err().contains("interface 1.7"));
    }

    #[test]
    fn install_a_script_mod() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("plugins");
        // From a folder: it must hold the scripts it names.
        let src = tmp.path().join("button-swap");
        std::fs::create_dir_all(&src).unwrap();
        std::fs::write(src.join(MANIFEST), serde_json::to_vec(&script("button-swap", json!({}))).unwrap()).unwrap();
        assert!(install(&dir, "rac1", "pal", &src).unwrap_err().contains("does not hold (main.luau)"));
        std::fs::write(src.join("main.luau"), "openrac.log(\"hi\")\n").unwrap();
        std::fs::create_dir_all(src.join(".git")).unwrap();
        std::fs::write(src.join(".git").join("HEAD"), "ref: refs/heads/master\n").unwrap();
        match install(&dir, "rac1", "pal", &src).unwrap() {
            Installed::Plugin { id, enabled, .. } => assert_eq!((id.as_str(), enabled), ("button-swap", true)),
            other => panic!("{other:?}"),
        }
        assert!(dir.join("button-swap").join("main.luau").is_file());
        assert!(!dir.join("button-swap").join(".git").exists(), "a repository's metadata is not copied");
        let views = list(&dir, "rac1", "pal").unwrap();
        let v = view(&views, "button-swap");
        assert_eq!((v.kind.as_str(), v.native, v.state.as_str()), ("script", false, "ready"));
        // From a zip, as package_mod.py writes it (one top folder).
        let zip = tmp.path().join("other.zip");
        let m = serde_json::to_vec(&script("other", json!({}))).unwrap();
        zip_of(&zip, &[("other-1.0.0/openrac-mod.json", &m), ("other-1.0.0/main.luau", b"-- empty")]);
        assert!(matches!(install(&dir, "rac1", "pal", &zip).unwrap(), Installed::Plugin { .. }));
        let views = list(&dir, "rac1", "pal").unwrap();
        assert_eq!(view(&views, "other").kind, "script");
        // A native mod's view says so; a data-only one says data.
        installed_mod(&dir, "native", json!({}));
        installed_mod(&dir, "data", json!({"plugin": {}, "api": null}));
        let views = list(&dir, "rac1", "pal").unwrap();
        assert_eq!(view(&views, "native").kind, "native");
        assert_eq!(view(&views, "data").kind, "data");
    }

    #[test]
    fn settings_values() {
        let spec = |v: Value| -> SettingSpec { serde_json::from_value(v).unwrap() };
        let int = spec(json!({"key": "n", "name": "N", "type": "int", "default": 1, "min": 0, "max": 10}));
        assert_eq!(check_value(&int, &json!(5)), Some(json!(5)));
        assert_eq!(check_value(&int, &json!(11)), None);
        assert_eq!(check_value(&int, &json!(1.5)), None);
        assert_eq!(check_value(&int, &json!("5")), None);
        let float = spec(json!({"key": "f", "name": "F", "type": "float", "default": 1.0, "min": 0.5}));
        assert_eq!(check_value(&float, &json!(2)), Some(json!(2.0)));
        assert_eq!(check_value(&float, &json!(0.25)), None);
        let text = spec(json!({"key": "s", "name": "S", "type": "string", "default": "", "max_length": 3}));
        assert_eq!(check_value(&text, &json!("abc")), Some(json!("abc")));
        assert_eq!(check_value(&text, &json!("abcd")), None);
        let flag = spec(json!({"key": "b", "name": "B", "type": "bool", "default": false}));
        assert_eq!(check_value(&flag, &json!(true)), Some(json!(true)));
        assert_eq!(check_value(&flag, &json!(1)), None);
    }

    #[test]
    fn keybind_values() {
        let spec = |v: Value| -> SettingSpec { serde_json::from_value(v).unwrap() };
        let two = spec(json!({"key": "jump", "name": "Jump", "type": "keybind", "default": "Space"}));
        assert_eq!(check_value(&two, &json!("KeyW, ArrowUp")), Some(json!("KeyW,ArrowUp")));
        assert_eq!(check_value(&two, &json!("")), Some(json!("")));
        assert_eq!(check_value(&two, &json!("MouseRight,WheelDown")), Some(json!("MouseRight,WheelDown")));
        assert_eq!(check_value(&two, &json!("KeyW,KeyW")), None);
        assert_eq!(check_value(&two, &json!("KeyW,KeyA,KeyS")), None);
        assert_eq!(check_value(&two, &json!("W")), None);
        assert_eq!(check_value(&two, &json!(4)), None);
        let one = spec(json!({"key": "pause", "name": "Pause", "type": "keybind", "default": "Escape", "binds": 1}));
        assert_eq!(check_value(&one, &json!("KeyP")), Some(json!("KeyP")));
        assert_eq!(check_value(&one, &json!("KeyP,KeyO")), None);
    }

    /// The test vectors the SDK's and the port's checks load too (sdk/tests/manifests/README.md).
    const SHARED_CASES: &str = include_str!("../../../sdk/tests/manifests/cases.json");

    /// The launcher's verdict a shared case expects: its own where the three checks disagree.
    fn expected(case: &Value) -> bool {
        case["valid"].as_bool().unwrap_or_else(|| case["valid"]["launcher"].as_bool().unwrap())
    }

    #[test]
    fn shared_ids() {
        let cases: Value = serde_json::from_str(SHARED_CASES).unwrap();
        let wrong: Vec<(&str, bool)> = cases["ids"]
            .as_array()
            .unwrap()
            .iter()
            .filter(|c| valid_id(c["id"].as_str().unwrap()) != expected(c))
            .map(|c| (c["id"].as_str().unwrap(), expected(c)))
            .collect();
        assert!(wrong.is_empty(), "ids whose verdict is not the expected one (id, expected): {wrong:?}");
    }

    #[test]
    fn shared_manifests() {
        let cases: Value = serde_json::from_str(SHARED_CASES).unwrap();
        // A file serde cannot read as a Manifest is refused before validate, as read_manifest refuses it.
        let accepted = |m: &Value| serde_json::from_value::<Manifest>(m.clone()).is_ok_and(|m| m.validate().is_ok());
        let wrong: Vec<(&str, bool)> = cases["manifests"]
            .as_array()
            .unwrap()
            .iter()
            .filter(|c| accepted(&c["manifest"]) != expected(c))
            .map(|c| (c["name"].as_str().unwrap(), expected(c)))
            .collect();
        assert!(wrong.is_empty(), "manifests whose verdict is not the expected one (name, expected): {wrong:?}");
    }

    #[test]
    fn bind_names_are_keys_h() {
        let header =
            std::fs::read_to_string(Path::new(env!("CARGO_MANIFEST_DIR")).join("../../sdk/include/openrac/keys.h"))
                .unwrap();
        let names: Vec<&str> = header
            .split("{\"")
            .skip(1)
            .filter_map(|rest| rest.split_once("\", ").map(|(name, _)| name))
            .filter(|n| n.chars().all(|c| c.is_ascii_alphanumeric()))
            .collect();
        assert_eq!(names, BIND_NAMES);
    }

    #[test]
    fn keybind_manifests_presets_and_conflicts() {
        let with = |settings: Value, presets: Value| -> Result<(), String> {
            let m: Manifest =
                serde_json::from_value(manifest("kbm", json!({"settings": settings, "presets": presets}))).unwrap();
            m.validate()
        };
        let settings = json!([
            {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space", "group": "play"},
            {"key": "fire", "name": "Fire", "type": "keybind", "default": "MouseLeft", "group": "play"},
            {"key": "back", "name": "Back", "type": "keybind", "default": "Space", "group": "menu", "binds": 1},
        ]);
        let arrows = json!([{"id": "arrows", "name": "Arrows", "values": {"jump": "Numpad0"}}]);
        assert_eq!(with(settings.clone(), arrows), Ok(()));
        assert!(with(settings.clone(), json!([{"id": "x", "name": "X", "values": {"nothing": "KeyA"}}])).is_err());
        assert!(with(settings.clone(), json!([{"id": "x", "name": "X", "values": {"jump": "Nope"}}])).is_err());
        assert!(with(settings.clone(), json!([{"id": "x", "name": "X", "values": {"back": "KeyA,KeyB"}}])).is_err());
        assert!(with(settings.clone(), json!([{"id": "x", "name": "X"}, {"id": "x", "name": "Y"}])).is_err());
        assert!(with(json!([{"key": "j", "name": "J", "type": "keybind", "default": "Spacebar"}]), json!([])).is_err());
        assert!(with(json!([{"key": "j", "name": "J", "type": "keybind", "default": "Space", "binds": 3}]), json!([]))
            .is_err());
        assert!(with(json!([{"key": "n", "name": "N", "type": "int", "default": 1, "binds": 1}]), json!([])).is_err());

        // A preset fills any type of setting: the sliders, a choice, a switch, text.
        let mixed = json!([
            {"key": "damage", "name": "Damage", "type": "float", "default": 1.0, "min": 0.25, "max": 4.0},
            {"key": "count", "name": "Count", "type": "int", "default": 3, "min": 1, "max": 30},
            {"key": "mode", "name": "Mode", "type": "choice", "default": "a",
             "choices": [{"value": "a", "name": "A"}, {"value": "b", "name": "B"}]},
            {"key": "chaos", "name": "Chaos", "type": "bool", "default": false},
            {"key": "seed", "name": "Seed", "type": "string", "default": "", "max_length": 4},
        ]);
        let hard = json!([{"id": "hard", "name": "Hard",
                           "values": {"damage": 2.5, "count": 30, "mode": "b", "chaos": true, "seed": "abcd"}}]);
        assert_eq!(with(mixed.clone(), hard), Ok(()));
        for bad in [
            json!({"damage": 5.0}),
            json!({"damage": "2"}),
            json!({"count": 1.5}),
            json!({"count": 0}),
            json!({"mode": "c"}),
            json!({"chaos": 1}),
            json!({"seed": "abcde"}),
        ] {
            let err = with(mixed.clone(), json!([{"id": "x", "name": "X", "values": bad}])).unwrap_err();
            assert!(err.contains("is not a value of"), "{err}");
        }

        // Jump and Fire share KeyQ (one group): both say so; Back has Space with Jump, in another group.
        let specs: Vec<SettingSpec> = serde_json::from_value(settings).unwrap();
        let mut views: Vec<SettingView> = specs
            .into_iter()
            .zip(["Space,KeyQ", "KeyQ", "Space"])
            .map(|(spec, v)| SettingView { spec, value: json!(v), conflicts: Vec::new() })
            .collect();
        mark_conflicts(&mut views);
        assert_eq!(views[0].conflicts, vec!["Fire".to_string()]);
        assert_eq!(views[1].conflicts, vec!["Jump".to_string()]);
        assert!(views[2].conflicts.is_empty());
    }

    #[test]
    fn the_list_its_order_and_the_folders_not_in_it() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path();
        assert!(list(dir, "rac1", "pal").unwrap().is_empty(), "no folder: no mods");
        installed_mod(dir, "b", json!({}));
        installed_mod(dir, "a", json!({}));
        installed_mod(dir, "c", json!({}));
        std::fs::create_dir_all(dir.join("not-a-mod")).unwrap();
        std::fs::write(
            dir.join(LIST),
            r#"{"format": 1, "mods": [{"id": "c", "enabled": true}, {"id": "gone", "enabled": true},
                                     {"id": "b", "enabled": true}, {"id": "c"}]}"#,
        )
        .unwrap();
        let views = list(dir, "rac1", "pal").unwrap();
        assert_eq!(ids(&views), ["c", "b", "a"], "listed first, in order; the unlisted at the end");
        assert_eq!(views[0].state, "ready");
        assert_eq!(views[2].state, "off");
        assert!(views[0].native);

        let views = move_mod(dir, "rac1", "pal", "a", true).unwrap();
        assert_eq!(ids(&views), ["c", "a", "b"]);
        let views = move_mod(dir, "rac1", "pal", "c", true).unwrap();
        assert_eq!(ids(&views), ["c", "a", "b"], "the first stays first");
        let written: Value = serde_json::from_str(&std::fs::read_to_string(dir.join(LIST)).unwrap()).unwrap();
        assert_eq!(written["format"], 1);
        assert_eq!(written["mods"][1], json!({"id": "a", "enabled": false, "settings": {}}));

        std::fs::write(dir.join(LIST), r#"{"format": 2, "mods": []}"#).unwrap();
        assert!(list(dir, "rac1", "pal").unwrap_err().contains("newer format"));
    }

    #[test]
    fn states_from_the_disc_the_system_and_the_last_start() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path();
        installed_mod(dir, "pal-only", json!({"games": {"rac1": ["pal"]}}));
        installed_mod(dir, "fine", json!({"version": "1.1.0"}));
        installed_mod(dir, "newer-api", json!({"api": "1.5"}));
        std::fs::create_dir_all(dir.join("broken")).unwrap();
        std::fs::write(dir.join("broken").join(MANIFEST), "{ nope").unwrap();
        std::fs::write(
            dir.join(LIST),
            r#"{"format": 1, "mods": [{"id": "pal-only", "enabled": true}, {"id": "fine", "enabled": true},
                                     {"id": "newer-api", "enabled": true}, {"id": "broken", "enabled": true}]}"#,
        )
        .unwrap();
        let views = list(dir, "rac1", "ntsc").unwrap();
        assert_eq!(view(&views, "pal-only").state, "refused");
        assert!(view(&views, "pal-only").reason.contains("Not for this disc"));
        assert_eq!(view(&views, "fine").state, "ready");
        assert!(view(&views, "newer-api").reason.contains("interface 1.5"));
        assert!(view(&views, "broken").reason.contains("Cannot be read"));
        assert_eq!(view(&views, "broken").name, "broken");

        // The port did not load "fine" at its last start: said, while it is the version it tried.
        std::fs::write(
            dir.join(STATUS),
            r#"{"format": 1, "time": "t", "mods": [{"id": "fine", "version": "1.1.0", "state": "failed",
                 "reason": "openrac_mod_load returned 3"}], "hook_points": ["Pad_ProcessReport"]}"#,
        )
        .unwrap();
        let views = list(dir, "rac1", "pal").unwrap();
        assert_eq!(view(&views, "fine").state, "failed");
        assert!(view(&views, "fine").reason.ends_with("openrac_mod_load returned 3"));
        assert_eq!(read_status(dir).unwrap().hook_points, ["Pad_ProcessReport"]);
        installed_mod(dir, "fine", json!({"version": "1.2.0"}));
        assert_eq!(
            view(&list(dir, "rac1", "pal").unwrap(), "fine").state,
            "ready",
            "a new version: the old answer goes"
        );
    }

    #[test]
    fn requires_conflicts_and_shared_hooks() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path();
        installed_mod(dir, "base", json!({"version": "1.2.0", "hooks": ["Pad_ProcessReport"]}));
        installed_mod(
            dir,
            "user",
            json!({"requires": [{"id": "base", "version": ">=1.1.0"}],
                                          "hooks": ["Pad_ProcessReport", "Hero_TurnToHeading"]}),
        );
        installed_mod(dir, "picky", json!({"requires": [{"id": "base", "version": ">=2.0.0"}]}));
        installed_mod(dir, "lonely", json!({"requires": [{"id": "absent"}]}));
        installed_mod(dir, "rival", json!({"conflicts": [{"id": "user", "reason": "both take L2 and R2"}]}));

        let err = set_enabled(dir, "rac1", "pal", "user", true).unwrap_err();
        assert!(err.contains("switch it on"), "{err}");
        assert!(set_enabled(dir, "rac1", "pal", "lonely", true).unwrap_err().contains("not installed"));
        assert!(set_enabled(dir, "rac1", "pal", "picky", true).unwrap_err().contains(">=2.0.0"));

        set_enabled(dir, "rac1", "pal", "base", true).unwrap();
        let views = set_enabled(dir, "rac1", "pal", "user", true).unwrap();
        assert!(ids(&views).iter().position(|&i| i == "base") < ids(&views).iter().position(|&i| i == "user"));
        assert_eq!(view(&views, "user").state, "ready");
        assert_eq!(
            view(&views, "user").shared_hooks,
            [SharedHook { hook: "Pad_ProcessReport".into(), with: vec!["Mod base".into()] }]
        );
        assert!(view(&views, "picky").blocked.contains(">=2.0.0"), "an off mod says why it cannot go on");

        // Either side's list makes a conflict.
        let err = set_enabled(dir, "rac1", "pal", "rival", true).unwrap_err();
        assert!(err.contains("Conflicts with Mod user: both take L2 and R2"), "{err}");

        // Moved above what it needs by hand: it says so.
        let mut views = list(dir, "rac1", "pal").unwrap();
        while ids(&views)[0] != "user" {
            views = move_mod(dir, "rac1", "pal", "user", true).unwrap();
        }
        assert_eq!(ids(&views)[0], "user");
        assert_eq!(view(&views, "user").state, "refused");
        assert!(view(&views, "user").reason.contains("move it above"));

        // Its dependency switched off: refused, with the reason.
        let views = set_enabled(dir, "rac1", "pal", "base", false).unwrap();
        assert!(view(&views, "user").reason.contains("switch it on"));
    }

    #[test]
    fn load_hints_place_a_mod_switched_on() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path();
        installed_mod(dir, "a", json!({}));
        installed_mod(dir, "b", json!({"load": {"after": ["c"]}}));
        installed_mod(dir, "c", json!({}));
        installed_mod(dir, "d", json!({"load": {"before": ["a"]}}));
        installed_mod(dir, "e", json!({}));
        installed_mod(dir, "z", json!({"load": {"before": ["e"]}}));
        set_enabled(dir, "rac1", "pal", "c", true).unwrap();
        // b wants to be after c: it was before it, it moves to just after.
        let views = set_enabled(dir, "rac1", "pal", "b", true).unwrap();
        assert_eq!(ids(&views), ["a", "c", "b", "d", "e", "z"]);
        // d wants to be before a.
        let views = set_enabled(dir, "rac1", "pal", "d", true).unwrap();
        assert_eq!(ids(&views), ["d", "a", "c", "b", "e", "z"]);
        // e, switched on, is placed after z, which asks to be before it.
        let views = set_enabled(dir, "rac1", "pal", "e", true).unwrap();
        assert_eq!(ids(&views), ["d", "a", "c", "b", "z", "e"]);
    }

    #[test]
    fn settings_are_checked_and_defaults_filled() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path();
        installed_mod(
            dir,
            "lock-strafe",
            json!({"settings": [
                {"key": "mode", "name": "Mode", "type": "choice", "default": "lock",
                 "choices": [{"value": "lock", "name": "Lock with weapons"}, {"value": "hold", "name": "Hold"}]},
                {"key": "trace", "name": "Trace", "type": "bool", "default": false}]}),
        );
        std::fs::write(
            dir.join(LIST),
            r#"{"format": 1, "mods": [{"id": "lock-strafe", "enabled": true, "settings": {"mode": "spin"}}]}"#,
        )
        .unwrap();
        let views = list(dir, "rac1", "pal").unwrap();
        assert_eq!(views[0].settings[0].value, json!("lock"), "a value it does not take is the default");
        assert_eq!(views[0].settings[1].value, json!(false));
        let views = set_setting(dir, "rac1", "pal", "lock-strafe", "mode", json!("hold")).unwrap();
        assert_eq!(views[0].settings[0].value, json!("hold"));
        assert!(set_setting(dir, "rac1", "pal", "lock-strafe", "mode", json!("spin")).is_err());
        assert!(set_setting(dir, "rac1", "pal", "lock-strafe", "trace", json!("yes")).is_err());
        assert!(set_setting(dir, "rac1", "pal", "lock-strafe", "nope", json!(1)).is_err());
        let written: Value = serde_json::from_str(&std::fs::read_to_string(dir.join(LIST)).unwrap()).unwrap();
        assert_eq!(written["mods"][0]["settings"], json!({"mode": "hold"}));
        // The UI's view of a setting: the schema's fields and the value.
        let shown = serde_json::to_value(&views[0].settings[0]).unwrap();
        assert_eq!(shown["type"], "choice");
        assert_eq!(shown["value"], "hold");
    }

    fn zip_of(path: &Path, files: &[(&str, &[u8])]) {
        let file = std::fs::File::create(path).unwrap();
        let mut zip = zip::ZipWriter::new(file);
        let options = zip::write::SimpleFileOptions::default().compression_method(zip::CompressionMethod::Deflated);
        for (name, bytes) in files {
            zip.start_file(*name, options).unwrap();
            zip.write_all(bytes).unwrap();
        }
        zip.finish().unwrap();
    }

    #[test]
    fn install_from_a_folder_and_replace_keeping_settings() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("plugins");
        let source = tmp.path().join("download/lock-strafe-1.0.0");
        let settings = json!([{"key": "mode", "name": "Mode", "type": "choice", "default": "lock",
                               "choices": [{"value": "lock", "name": "Lock"}, {"value": "hold", "name": "Hold"}]}]);
        write_mod(&source, &manifest("lock-strafe", json!({"settings": settings})));
        // The folder holding the mod's folder: its one folder counts.
        let got = install(&dir, "rac1", "pal", source.parent().unwrap()).unwrap();
        assert_eq!(
            got,
            Installed::Plugin {
                id: "lock-strafe".into(),
                name: "Mod lock-strafe".into(),
                version: "1.0.0".into(),
                replaced: false,
                enabled: true
            }
        );
        assert!(dir.join("lock-strafe").join(MANIFEST).is_file());
        assert!(source.join(MANIFEST).is_file(), "what it was added from is kept");
        set_setting(&dir, "rac1", "pal", "lock-strafe", "mode", json!("hold")).unwrap();

        // A new version: replaced, the settings it still takes kept, the place and state kept.
        write_mod(&source, &manifest("lock-strafe", json!({"version": "1.1.0", "settings": settings})));
        std::fs::write(source.join("README.md"), "new").unwrap();
        let got = install(&dir, "rac1", "pal", &source).unwrap();
        assert!(matches!(got, Installed::Plugin { replaced: true, enabled: true, .. }));
        let views = list(&dir, "rac1", "pal").unwrap();
        assert_eq!(views[0].version, "1.1.0");
        assert_eq!(views[0].settings[0].value, json!("hold"));
        assert!(dir.join("lock-strafe/README.md").is_file());
        let leftovers: Vec<_> = std::fs::read_dir(&dir)
            .unwrap()
            .filter_map(Result::ok)
            .filter(|e| e.file_name().to_string_lossy().starts_with('.'))
            .collect();
        assert!(leftovers.is_empty(), "no scratch folders left: {leftovers:?}");

        // Removing deletes the store's copy only.
        let views = remove(&dir, "rac1", "pal", "lock-strafe").unwrap();
        assert!(views.is_empty());
        assert!(!dir.join("lock-strafe").exists());
        assert!(source.is_dir());
        assert!(remove(&dir, "rac1", "pal", "../download").is_err());
        assert!(remove(&dir, "rac1", "pal", "lock-strafe").is_err());
    }

    #[test]
    fn a_folder_of_mods() {
        let root = tempfile::tempdir().unwrap();
        let r = root.path();
        let one = |dir: &Path| {
            std::fs::create_dir_all(dir).unwrap();
            std::fs::write(dir.join(MANIFEST), "{}").unwrap();
        };
        // One mod: the folder itself.
        one(&r.join("solo"));
        assert_eq!(mod_folders(&r.join("solo")).unwrap(), [r.join("solo")]);
        // A repository's checkout: its mods' folders, from its build when there is one.
        one(&r.join("repo/b"));
        one(&r.join("repo/a"));
        std::fs::create_dir_all(r.join("repo/tools")).unwrap();
        assert_eq!(mod_folders(&r.join("repo")).unwrap(), [r.join("repo/a"), r.join("repo/b")]);
        one(&r.join("repo/build/package/a"));
        assert_eq!(mod_folders(&r.join("repo")).unwrap(), [r.join("repo/build/package/a")]);
        // No mod anywhere: said.
        std::fs::create_dir_all(r.join("empty/x")).unwrap();
        assert!(mod_folders(&r.join("empty")).unwrap_err().contains("holds no OpenRAC mod"));
    }

    #[test]
    fn install_from_a_zip() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("plugins");
        let m = serde_json::to_vec(&manifest("hello", json!({}))).unwrap();
        let lib = format!("hello-1.0.0/lib/{}/libhello.so", here());
        let zip = tmp.path().join("hello.zip");
        zip_of(&zip, &[("hello-1.0.0/openrac-mod.json", &m), (&lib, b"lib"), ("__MACOSX/._x", b"")]);
        assert!(matches!(install(&dir, "rac1", "pal", &zip).unwrap(), Installed::Plugin { .. }));
        assert!(dir.join("hello").join("lib").join(here()).join("libhello.so").is_file());

        // A data pack is the overrides'.
        let pack = tmp.path().join("pack.zip");
        zip_of(&pack, &[("openrac-mod.json", br#"{"format": 1, "game": "rac1", "replaces": {}}"#)]);
        assert_eq!(install(&dir, "rac1", "pal", &pack).unwrap(), Installed::Pack);

        // Another game's mod.
        let other = tmp.path().join("other.zip");
        let m = serde_json::to_vec(&manifest("other", json!({"games": {"rac2": ["ntsc"]}}))).unwrap();
        let lib = format!("lib/{}/libother.so", here());
        zip_of(&other, &[("openrac-mod.json", &m), (&lib, b"lib")]);
        assert!(install(&dir, "rac1", "pal", &other).unwrap_err().contains("for rac2"));

        // A library it names but does not hold.
        let hollow = tmp.path().join("hollow.zip");
        let m = serde_json::to_vec(&manifest("hollow", json!({}))).unwrap();
        zip_of(&hollow, &[("openrac-mod.json", &m)]);
        assert!(install(&dir, "rac1", "pal", &hollow).unwrap_err().contains("does not hold"));

        // Not a mod at all.
        let empty = tmp.path().join("empty.zip");
        zip_of(&empty, &[("a/readme.txt", b"hi"), ("b/readme.txt", b"hi")]);
        assert!(install(&dir, "rac1", "pal", &empty).unwrap_err().contains("not an OpenRAC mod"));
        assert!(install(&dir, "rac1", "pal", &tmp.path().join("file.txt")).is_err());
    }

    #[test]
    fn install_refuses_paths_out_of_the_zip() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("plugins");
        let m = serde_json::to_vec(&manifest("evil", json!({}))).unwrap();
        let lib = format!("lib/{}/libevil.so", here());
        let zip = tmp.path().join("evil.zip");
        zip_of(&zip, &[("openrac-mod.json", &m), (&lib, b"lib"), ("../../escaped.txt", b"x")]);
        let err = install(&dir, "rac1", "pal", &zip).unwrap_err();
        assert!(err.contains("outside"), "{err}");
        assert!(!tmp.path().join("escaped.txt").exists());
        assert!(!dir.join("evil").exists());

        let abs = tmp.path().join("abs.zip");
        zip_of(&abs, &[("openrac-mod.json", &m), ("/etc/evil", b"x")]);
        assert!(install(&dir, "rac1", "pal", &abs).unwrap_err().contains("outside"));
    }

    #[cfg(unix)]
    #[test]
    fn install_refuses_links() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("plugins");
        let source = tmp.path().join("linked");
        write_mod(&source, &manifest("linked", json!({})));
        std::os::unix::fs::symlink("/etc/hosts", source.join("hosts")).unwrap();
        assert!(install(&dir, "rac1", "pal", &source).unwrap_err().contains("link"));
        assert!(!dir.join("linked").exists());

        let zip = tmp.path().join("link.zip");
        let file = std::fs::File::create(&zip).unwrap();
        let mut writer = zip::ZipWriter::new(file);
        let options = zip::write::SimpleFileOptions::default();
        writer.start_file("openrac-mod.json", options).unwrap();
        writer.write_all(&serde_json::to_vec(&manifest("link", json!({"plugin": {}, "api": null}))).unwrap()).unwrap();
        writer.add_symlink("evil", "/etc/hosts", options).unwrap();
        writer.finish().unwrap();
        assert!(install(&dir, "rac1", "pal", &zip).unwrap_err().contains("link"));
    }
}
