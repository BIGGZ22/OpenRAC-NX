// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

//! A game's differences from the original: what the native port does otherwise than the PS2 game,
//! from `port/game/<game>/differences.json`, the same file the port's build reads
//! (port/cmake/Differences.cmake; docs/port/SETTINGS.md section 8).
//!
//! Each entry is a `fix` (the player may switch it back to the console's behaviour: a setting
//! `fix_... = 1|0` in the port's settings file), a `guard` (keeps the game from crashing where the
//! console carried on; never switched off) or a `port` (a PC difference, for the record).
//!
//! The file is read from the checkout when there is one, else from the copy built into the
//! launcher. The fixes' values are in the port's own settings file,
//! `<data home>/settings/<game>-<version>.ini` (port/game/common/main.cpp, `default_settings`),
//! which the launcher edits a line at a time: everything else in it stays as the game wrote it.
//! The game reads the file when it starts, and writes it whole when a setting changes in its own
//! menus, so a change here while the game runs is overwritten by the next change made there.

use serde::{Deserialize, Serialize};
use std::path::{Path, PathBuf};

/// The lists built into the launcher, by game: what a launcher without a checkout shows.
const BUILT_IN: &[(&str, &str)] = &[("rac1", include_str!("../../../port/game/rac1/differences.json"))];

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum Kind {
    /// The port's fix of something the console does, which the player may switch back.
    Fix,
    /// Keeps the game from crashing where the console carried on unnoticed: shown, never switched.
    Guard,
    /// A difference that comes with being a PC program: for the record.
    Port,
}

/// A fix's setting in the port's settings file.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct FixSetting {
    pub key: String,
    /// On is the port's fix; off the console's behaviour.
    pub default: bool,
}

/// One entry of the file.
#[derive(Debug, Clone, Deserialize)]
pub struct Difference {
    pub id: String,
    pub kind: Kind,
    pub name: String,
    pub discs: Vec<String>,
    pub original: String,
    pub port: String,
    #[serde(default)]
    pub setting: Option<FixSetting>,
    #[serde(default)]
    pub refs: Vec<String>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct DifferenceFile {
    pub game: String,
    pub differences: Vec<Difference>,
}

/// An entry as a game's page shows it, with a fix's value now.
#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct DifferenceView {
    pub id: String,
    pub kind: Kind,
    pub name: String,
    pub original: String,
    pub port: String,
    pub refs: Vec<String>,
    pub setting: Option<FixSetting>,
    /// A fix's value in the settings file (its default when the file does not say); None otherwise.
    pub on: Option<bool>,
}

/// A version's differences: those that apply to its disc, in the file's order.
#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Differences {
    pub game: String,
    pub version: String,
    /// The port's settings file the fixes are kept in.
    pub settings_file: String,
    pub entries: Vec<DifferenceView>,
}

/// The longest name a fix may have (a row of the game's Options page; Differences.cmake's too).
pub const FIX_NAME_MAX: usize = 22;

fn is_word_list(text: &str, sep: char) -> bool {
    !text.is_empty()
        && text.split(sep).all(|w| !w.is_empty() && w.chars().all(|c| c.is_ascii_lowercase() || c.is_ascii_digit()))
}

/// Reads and checks a game's file: the same rules the port's configure applies.
pub fn parse(text: &str) -> Result<DifferenceFile, String> {
    let file: DifferenceFile = serde_json::from_str(text).map_err(|e| format!("differences.json: {e}"))?;
    let mut ids = std::collections::HashSet::new();
    let mut keys = std::collections::HashSet::new();
    for d in &file.differences {
        let at = |what: &str| format!("differences.json: {}: {what}", d.id);
        if !is_word_list(&d.id, '-') {
            return Err(at("the id is not lower-case words joined by hyphens"));
        }
        if !ids.insert(d.id.as_str()) {
            return Err(at("the id is there twice"));
        }
        if d.name.trim().is_empty() || d.original.trim().is_empty() || d.port.trim().is_empty() {
            return Err(at("name, original and port are all needed"));
        }
        if d.discs.is_empty() || d.discs.iter().any(|v| v != "pal" && v != "ntsc") {
            return Err(at("discs are pal, ntsc or both"));
        }
        match (d.kind, &d.setting) {
            (Kind::Fix, None) => return Err(at("a fix needs a setting")),
            (Kind::Fix, Some(s)) => {
                if !s.key.strip_prefix("fix_").is_some_and(|rest| is_word_list(rest, '_')) {
                    return Err(at("the setting's key is fix_ and lower-case words"));
                }
                if !keys.insert(s.key.as_str()) {
                    return Err(at("the setting's key is there twice"));
                }
                if d.name.chars().count() > FIX_NAME_MAX {
                    return Err(at(&format!("a fix's name is at most {FIX_NAME_MAX} letters")));
                }
            }
            (_, Some(_)) => return Err(at("only a fix has a setting")),
            (_, None) => {}
        }
    }
    Ok(file)
}

fn plain_name(name: &str) -> bool {
    !name.is_empty() && name.chars().all(|c| c.is_ascii_alphanumeric() || c == '_' || c == '-')
}

/// The text of a game's list: the checkout's (`root`), else the launcher's own copy; None for a
/// game without one.
pub fn source(root: Option<&Path>, game: &str) -> Option<String> {
    if !plain_name(game) {
        return None;
    }
    root.and_then(|root| {
        std::fs::read_to_string(crate::under(root, &format!("port/game/{game}/differences.json"))).ok()
    })
    .or_else(|| BUILT_IN.iter().find(|(id, _)| *id == game).map(|(_, text)| text.to_string()))
}

/// The port's settings file of a version: `<data home>/settings/<game>-<version>.ini`.
pub fn settings_file(data_home: &Path, game: &str, version: &str) -> PathBuf {
    data_home.join("settings").join(format!("{game}-{version}.ini"))
}

/// A `key = value` line's value as the port reads a switch (settings.cpp, parse_bool); None when
/// the key is not there or its value is not one the port knows (the port then uses the default).
pub fn read_switch(text: &str, key: &str) -> Option<bool> {
    let mut found = None;
    for line in text.lines() {
        let line = line.trim();
        if line.starts_with('#') {
            continue;
        }
        let Some((k, v)) = line.split_once('=') else { continue };
        if k.trim() != key {
            continue;
        }
        // The port reads every line in turn: the last one counts.
        found = match v.trim() {
            "1" | "true" | "on" | "yes" => Some(true),
            "0" | "false" | "off" | "no" => Some(false),
            _ => None,
        };
    }
    found
}

/// A `key = value` line's value as written (the last one counts), None when the key is not there.
pub fn read_value(text: &str, key: &str) -> Option<String> {
    let mut found = None;
    for line in text.lines() {
        let line = line.trim();
        if line.starts_with('#') {
            continue;
        }
        if let Some((k, v)) = line.split_once('=') {
            if k.trim() == key {
                found = Some(v.trim().to_string());
            }
        }
    }
    found
}

/// `text` with `key = 1|0`: the key's line changed where it is (every one, if there are several),
/// else added at the end. Every other line is kept as it was.
pub fn with_switch(text: &str, key: &str, on: bool) -> String {
    with_value(text, key, if on { "1" } else { "0" })
}

/// `text` with `key = value`, as with_switch.
pub fn with_value(text: &str, key: &str, value: &str) -> String {
    let line = format!("{key} = {value}");
    let mut out = String::new();
    let mut done = false;
    for l in text.lines() {
        let t = l.trim();
        if !t.starts_with('#') && t.split_once('=').is_some_and(|(k, _)| k.trim() == key) {
            out.push_str(&line);
            done = true;
        } else {
            out.push_str(l);
        }
        out.push('\n');
    }
    if !done {
        out.push_str(&line);
        out.push('\n');
    }
    out
}

/// What a version's page shows: the entries for its disc, with the fixes' values from `settings`.
pub fn view(root: Option<&Path>, data_home: &Path, game: &str, version: &str) -> Result<Differences, String> {
    if !plain_name(game) || !plain_name(version) {
        return Err(format!("{game}/{version} is not a game version"));
    }
    let file = settings_file(data_home, game, version);
    let entries = match source(root, game) {
        None => Vec::new(),
        Some(text) => {
            let list = parse(&text)?;
            if list.game != game {
                return Err(format!("port/game/{game}/differences.json is {}'s", list.game));
            }
            let settings = std::fs::read_to_string(&file).unwrap_or_default();
            list.differences
                .into_iter()
                .filter(|d| d.discs.iter().any(|v| v == version))
                .map(|d| {
                    let on = d.setting.as_ref().map(|s| read_switch(&settings, &s.key).unwrap_or(s.default));
                    DifferenceView {
                        id: d.id,
                        kind: d.kind,
                        name: d.name,
                        original: d.original,
                        port: d.port,
                        refs: d.refs,
                        setting: d.setting,
                        on,
                    }
                })
                .collect()
        }
    };
    Ok(Differences {
        game: game.to_string(),
        version: version.to_string(),
        settings_file: file.to_string_lossy().into_owned(),
        entries,
    })
}

/// Switches a fix on (the port's) or off (the console's behaviour) in the version's settings file,
/// and returns the version's differences as they are then.
pub fn set_fix(
    root: Option<&Path>,
    data_home: &Path,
    game: &str,
    version: &str,
    id: &str,
    on: bool,
) -> Result<Differences, String> {
    let now = view(root, data_home, game, version)?;
    let entry =
        now.entries.iter().find(|e| e.id == id).ok_or_else(|| format!("{id} is not one of {game}'s differences"))?;
    let key = match (&entry.kind, &entry.setting) {
        (Kind::Fix, Some(s)) => s.key.clone(),
        _ => return Err(format!("{} is not something that can be switched off", entry.name)),
    };
    let file = settings_file(data_home, game, version);
    let text = std::fs::read_to_string(&file).unwrap_or_default();
    write_settings(&file, &with_switch(&text, &key, on))?;
    view(root, data_home, game, version)
}

/// Writes the port's settings file whole: through a file beside it, then over the old one, as the
/// port writes it.
pub fn write_settings(file: &Path, changed: &str) -> Result<(), String> {
    if let Some(dir) = file.parent() {
        std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    }
    let mut written = file.to_path_buf().into_os_string();
    written.push(".new");
    let written = PathBuf::from(written);
    if let Err(e) = std::fs::write(&written, changed) {
        let _ = std::fs::remove_file(&written);
        return Err(format!("{}: {e}", written.display()));
    }
    std::fs::rename(&written, file).map_err(|e| {
        let _ = std::fs::remove_file(&written);
        format!("{}: {e}", file.display())
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    #[test]
    fn rac1s_list_parses_and_the_launcher_carries_the_same() {
        let path = checkout().join("port/game/rac1/differences.json");
        let text = std::fs::read_to_string(&path).unwrap();
        let list = parse(&text).unwrap();
        assert_eq!(list.game, "rac1");
        for kind in [Kind::Fix, Kind::Guard, Kind::Port] {
            assert!(list.differences.iter().any(|d| d.kind == kind), "{kind:?}");
        }
        assert_eq!(source(None, "rac1").unwrap(), text, "the built-in copy is the checkout's");
        let keys: Vec<&str> =
            list.differences.iter().filter_map(|d| d.setting.as_ref()).map(|s| s.key.as_str()).collect();
        assert!(keys.contains(&"fix_wide_moby_effects"), "{keys:?}");
        assert!(keys.contains(&"fix_wide_nanotech_orbs"), "{keys:?}");
        assert!(keys.contains(&"fix_wide_liquids"), "{keys:?}");
    }

    #[test]
    fn every_list_in_the_checkout_parses() {
        let games = checkout().join("port/game");
        for entry in std::fs::read_dir(&games).unwrap().flatten() {
            let file = entry.path().join("differences.json");
            if let Ok(text) = std::fs::read_to_string(&file) {
                let list = parse(&text).unwrap_or_else(|e| panic!("{}: {e}", file.display()));
                assert_eq!(list.game, entry.file_name().to_string_lossy(), "{}", file.display());
            }
        }
    }

    #[test]
    fn the_rules_are_the_ports() {
        let entry = |extra: &str| {
            format!(
                r#"{{"game": "rac1", "differences": [{{"id": "a-b", "name": "A", "discs": ["pal"],
                "original": "o", "port": "p", {extra}}}]}}"#
            )
        };
        assert!(parse(&entry(r#""kind": "fix", "setting": {"key": "fix_a_b", "default": true}"#)).is_ok());
        assert!(parse(&entry(r#""kind": "fix""#)).is_err(), "a fix without a setting");
        assert!(parse(&entry(r#""kind": "guard", "setting": {"key": "fix_a", "default": true}"#)).is_err());
        assert!(parse(&entry(r#""kind": "fix", "setting": {"key": "wide", "default": true}"#)).is_err());
        assert!(parse(&entry(r#""kind": "other""#)).is_err());
        assert!(parse(&entry(r#""kind": "port", "discs": ["jp"]"#)).is_err());
    }

    #[test]
    fn a_fix_reads_its_default_and_switches_in_the_settings_file() {
        let home = tempfile::tempdir().unwrap();
        let root = Some(checkout());
        let first = view(root.as_deref(), home.path(), "rac1", "pal").unwrap();
        assert!(first.settings_file.ends_with("rac1-pal.ini"), "{}", first.settings_file);
        let liquids = first.entries.iter().find(|e| e.id == "wide-liquids").unwrap();
        assert_eq!(liquids.on, Some(true), "the default, without a file");
        assert!(first.entries.iter().filter(|e| e.kind != Kind::Fix).all(|e| e.on.is_none()));
        // PAL-only entries are left out of the NTSC disc's list.
        let ntsc = view(root.as_deref(), home.path(), "rac1", "ntsc").unwrap();
        assert!(first.entries.iter().any(|e| e.id == "pal-60-hz"));
        assert!(!ntsc.entries.iter().any(|e| e.id == "pal-60-hz"));

        // The game's own file, as it writes it: switching keeps every other line.
        let file = settings_file(home.path(), "rac1", "pal");
        std::fs::create_dir_all(file.parent().unwrap()).unwrap();
        std::fs::write(&file, "# OpenRAC\naspect = 16:9\nfix_wide_liquids = 1\nvolume = 80\n").unwrap();
        let after = set_fix(root.as_deref(), home.path(), "rac1", "pal", "wide-liquids", false).unwrap();
        assert_eq!(after.entries.iter().find(|e| e.id == "wide-liquids").unwrap().on, Some(false));
        let text = std::fs::read_to_string(&file).unwrap();
        assert_eq!(text, "# OpenRAC\naspect = 16:9\nfix_wide_liquids = 0\nvolume = 80\n");
        // A key that is not there yet is added.
        set_fix(root.as_deref(), home.path(), "rac1", "pal", "wide-moby-effects", false).unwrap();
        assert!(std::fs::read_to_string(&file).unwrap().ends_with("fix_wide_moby_effects = 0\n"));
        // A guard is never switched.
        assert!(set_fix(root.as_deref(), home.path(), "rac1", "pal", "full-moby-pool-scene", false).is_err());
        assert!(view(root.as_deref(), home.path(), "../x", "pal").is_err());
    }

    #[test]
    fn switches_read_as_the_port_reads_them() {
        assert_eq!(read_switch("fix_a = on\n", "fix_a"), Some(true));
        assert_eq!(read_switch("fix_a=0", "fix_a"), Some(false));
        assert_eq!(read_switch("# fix_a = 0\n", "fix_a"), None);
        assert_eq!(read_switch("fix_a = maybe\n", "fix_a"), None);
        assert_eq!(read_switch("fix_ab = 0\n", "fix_a"), None);
        assert_eq!(read_switch("fix_a = 0\nfix_a = 1\n", "fix_a"), Some(true));
        assert_eq!(with_switch("", "fix_a", true), "fix_a = 1\n");
        assert_eq!(with_switch("# fix_a = 1\n", "fix_a", false), "# fix_a = 1\nfix_a = 0\n");
    }
}
