// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

//! Enhanced graphics: the experimental probe lighting, modern shadows and ambient occlusion of the
//! native port (port/enhanced), as a built-in entry of the Mods tab. It is not a mod the player
//! installs: the game carries the code, off unless `enhanced_rendering = 1` is in the version's
//! settings file (docs/port/SETTINGS.md), and the entry writes that key and the others it shows
//! there, a line at a time, as the game fixes are (differences.rs).
//!
//! With it on, Play starts the game with the Vulkan renderer, the only one that has it, and with
//! `--probes <data>/probes/<version>`: the probe bakes made on this computer from the player's own
//! extracted levels by the `probe-bake` action (port/enhanced/tools/probes/bake_all.py --update).
//! Each baked level records what it was made from (`openrac-bake.json`); a level whose export or
//! baker changed since counts as not ready and is baked again. A level without a bake keeps the
//! original lighting.

use std::path::{Path, PathBuf};

use serde::Serialize;
use serde_json::Value;
use sha2::{Digest, Sha256};

use crate::differences::{read_switch, read_value, settings_file, with_value, write_settings};
use crate::plugins::{check_value, Choice, PluginView, SettingSpec, SettingView};

/// The entry's id on the Mods tab (`?mod=enhanced-graphics`).
pub const ID: &str = "enhanced-graphics";
/// The settings key that switches enhanced rendering on (port/game/common/settings.h).
pub const SWITCH: &str = "enhanced_rendering";
/// The action that bakes the probes (launcher/actions.json).
pub const BAKE_ACTION: &str = "probe-bake";
/// What each baked level records (bake_all.py, STAMP).
const STAMP: &str = "openrac-bake.json";

/// Whether a version has the entry: the bake planner reads RAC1's levels (PAL and NTSC-U).
pub fn supported(game: &str, version: &str) -> bool {
    game == "rac1" && matches!(version, "pal" | "ntsc")
}

/// Where a version's probe bakes are kept: `<data>/probes/<version>`.
pub fn probes_dir(data: &Path, version: &str) -> PathBuf {
    data.join("probes").join(version)
}

fn spec(key: &str, name: &str, kind: &str, default: Value, description: &str) -> SettingSpec {
    SettingSpec {
        key: key.into(),
        name: name.into(),
        kind: kind.into(),
        default,
        description: description.into(),
        min: None,
        max: None,
        step: None,
        choices: Vec::new(),
        max_length: None,
        binds: None,
        group: String::new(),
    }
}

fn range(mut s: SettingSpec, min: f64, max: f64, step: f64) -> SettingSpec {
    (s.min, s.max, s.step) = (Some(min), Some(max), Some(step));
    s
}

fn advanced(mut s: SettingSpec) -> SettingSpec {
    s.group = "advanced".into();
    s
}

/// The settings the entry's page shows, with the port's defaults and ranges (settings.cpp). The
/// diagnostic ones (probe spheres, receiver biases, GTAO directions and steps) stay in the game's
/// own Enhanced menu.
pub fn settings() -> Vec<SettingSpec> {
    let mut quality = spec("gtao_quality", "Occlusion quality", "choice", "performance".into(), "");
    quality.choices = vec![
        Choice { value: "performance".into(), name: "Performance".into() },
        Choice { value: "quality".into(), name: "Quality".into() },
    ];
    vec![
        spec("probe_lighting", "Probe lighting", "bool", true.into(), "Light that bounces off the world."),
        spec("enhanced_shadows", "Modern shadows", "bool", true.into(), "Sharp sun shadows, and Ratchet's own."),
        spec("gtao", "Ambient occlusion", "bool", false.into(), "Soft shade in corners and under objects."),
        quality,
        range(
            spec("probe_gi_multiplier", "Bounce light", "float", 1.0.into(), "How bright the bounced light is."),
            0.0,
            5.0,
            0.1,
        ),
        advanced(range(spec("gtao_strength", "Occlusion strength", "float", 1.0.into(), ""), 0.0, 3.0, 0.05)),
        advanced(range(spec("gtao_radius", "Occlusion radius", "float", 1.25.into(), ""), 0.05, 8.0, 0.05)),
        advanced(spec(
            "gtao_half_resolution",
            "Half-resolution occlusion",
            "bool",
            true.into(),
            "Faster; edges a little softer.",
        )),
    ]
}

/// A setting's value as the settings file holds it, or its default.
fn read_setting(text: &str, spec: &SettingSpec) -> Value {
    let raw = read_value(text, &spec.key);
    let value = match (spec.kind.as_str(), raw) {
        ("bool", _) => read_switch(text, &spec.key).map(Value::Bool),
        ("float", Some(v)) => v.parse::<f64>().ok().map(Value::from),
        ("choice", Some(v)) => Some(Value::String(v)),
        _ => None,
    };
    value.and_then(|v| check_value(spec, &v)).unwrap_or_else(|| spec.default.clone())
}

/// A value as the port reads it: 1|0, a plain number, a choice's name.
fn written(value: &Value) -> String {
    match value {
        Value::Bool(on) => (if *on { "1" } else { "0" }).to_string(),
        Value::Number(n) => {
            let x = n.as_f64().unwrap_or(0.0);
            let text = format!("{x:.3}");
            text.trim_end_matches('0').trim_end_matches('.').to_string()
        }
        Value::String(s) => s.clone(),
        other => other.to_string(),
    }
}

/// How far the version's probe bakes are: its exported levels, the ones whose bake is current,
/// and the ones whose last bake failed.
#[derive(Debug, Clone, Default, Serialize, PartialEq, Eq)]
#[serde(rename_all = "camelCase")]
pub struct BakeStatus {
    /// The bake can run: the levels are there and so is the baker; else `why` says what is missing.
    pub available: bool,
    pub why: String,
    pub levels: usize,
    pub ready: usize,
    pub failed: usize,
}

fn sha256(path: &Path) -> Option<String> {
    let bytes = std::fs::read(path).ok()?;
    Some(format!("{:x}", Sha256::digest(&bytes)))
}

/// The bake status of the levels in `levels` against the bakes in `probes`, made by `baker`.
pub fn bake_status(levels: &Path, probes: &Path, baker: &Path) -> BakeStatus {
    let mut out = BakeStatus::default();
    let Ok(entries) = std::fs::read_dir(levels) else {
        out.why = "the levels are not ready yet".into();
        return out;
    };
    let baker_hash = sha256(baker);
    for entry in entries.flatten() {
        let name = entry.file_name().to_string_lossy().into_owned();
        let numbered =
            name.strip_prefix("level_").is_some_and(|n| !n.is_empty() && n.bytes().all(|b| b.is_ascii_digit()));
        let manifest = entry.path().join("manifest.json");
        if !numbered || !manifest.is_file() {
            continue;
        }
        out.levels += 1;
        let baked = probes.join(&name);
        let stamp: Option<Value> =
            std::fs::read_to_string(baked.join(STAMP)).ok().and_then(|t| serde_json::from_str(&t).ok());
        let current = stamp.as_ref().is_some_and(|s| {
            s["manifest_sha256"].as_str() == sha256(&manifest).as_deref()
                && baker_hash.is_some()
                && s["baker_sha256"].as_str() == baker_hash.as_deref()
                && baked.join("sparse.orpg").is_file()
        });
        if current {
            out.ready += 1;
        } else if probes.join(format!(".{name}.failed")).is_dir() {
            out.failed += 1;
        }
    }
    if out.levels == 0 {
        out.why = "the levels are not ready yet".into();
    } else if baker_hash.is_none() {
        out.why = "this build of the game has no probe baker".into();
    } else {
        out.available = true;
    }
    out
}

/// The bake status of a version (`rac1/pal`) from its probe-bake action's folders and baker.
pub fn version_bake_status(config: &crate::config::Config, key: &str) -> BakeStatus {
    let value_after = |args: &[String], flag: &str| {
        args.iter().position(|a| a == flag).and_then(|i| args.get(i + 1)).map(PathBuf::from)
    };
    match crate::library::filled(config, key, BAKE_ACTION) {
        Ok((args, artifact)) => match (value_after(&args, "--levels-dir"), value_after(&args, "--output-dir")) {
            (Some(levels), Some(probes)) => {
                let levels = if levels.is_absolute() {
                    levels
                } else {
                    config.root().map(|r| r.join(&levels)).unwrap_or(levels)
                };
                bake_status(&levels, &probes, &artifact.unwrap_or_default())
            }
            _ => BakeStatus { why: "the probe bake names no folders".into(), ..BakeStatus::default() },
        },
        Err(e) => BakeStatus { why: e, ..BakeStatus::default() },
    }
}

/// The entry as the Mods tab shows it: a mod's row and page, with the bake's status beside it.
#[derive(Debug, Clone, Serialize, PartialEq)]
#[serde(rename_all = "camelCase")]
pub struct EnhancedView {
    #[serde(rename = "mod")]
    pub entry: PluginView,
    pub bake: BakeStatus,
}

/// The entry for `game`/`version`, its values from the version's settings file in `data_home`.
pub fn view(data_home: &Path, game: &str, version: &str, bake: BakeStatus) -> EnhancedView {
    let text = std::fs::read_to_string(settings_file(data_home, game, version)).unwrap_or_default();
    let enabled = read_switch(&text, SWITCH).unwrap_or(false);
    let settings = settings()
        .into_iter()
        .map(|spec| {
            let value = read_setting(&text, &spec);
            SettingView { spec, value, conflicts: Vec::new() }
        })
        .collect();
    EnhancedView {
        entry: PluginView {
            id: ID.into(),
            name: "Enhanced graphics".into(),
            version: "Experimental".into(),
            author: "OpenRAC".into(),
            description: "Bounced light, modern shadows and ambient occlusion.".into(),
            homepage: String::new(),
            license: "GPL-3.0-or-later".into(),
            enabled,
            state: if enabled { "ready" } else { "off" }.into(),
            reason: String::new(),
            blocked: String::new(),
            settings,
            presets: Vec::new(),
            hooks: Vec::new(),
            shared_hooks: Vec::new(),
            native: false,
            kind: "built-in".into(),
            path: String::new(),
        },
        bake,
    }
}

/// Switches enhanced rendering on or off for the version.
pub fn set_enabled(data_home: &Path, game: &str, version: &str, on: bool) -> Result<(), String> {
    let file = settings_file(data_home, game, version);
    let text = std::fs::read_to_string(&file).unwrap_or_default();
    write_settings(&file, &with_value(&text, SWITCH, if on { "1" } else { "0" }))
}

/// Sets one of the entry's settings, checked as the port would read it.
pub fn set_setting(data_home: &Path, game: &str, version: &str, key: &str, value: &Value) -> Result<(), String> {
    let all = settings();
    let spec = all.iter().find(|s| s.key == key).ok_or_else(|| format!("{key} is not an enhanced graphics setting"))?;
    let value = check_value(spec, value).ok_or_else(|| format!("{} cannot be {value}", spec.name))?;
    let file = settings_file(data_home, game, version);
    let text = std::fs::read_to_string(&file).unwrap_or_default();
    write_settings(&file, &with_value(&text, key, &written(&value)))
}

/// What Play adds: environment variables, then arguments.
pub type PlayAdditions = (Vec<(String, String)>, Vec<String>);

/// What Play adds when the version's settings switch enhanced rendering on: the Vulkan renderer
/// and the probe bakes' folder. `settings` is the version's settings file's text.
pub fn play_additions(settings: &str, data: &Path, version: &str) -> Option<PlayAdditions> {
    if !read_switch(settings, SWITCH).unwrap_or(false) {
        return None;
    }
    let env = vec![("OPENRAC_GL_API".to_string(), "vulkan".to_string())];
    let args = vec!["--probes".to_string(), probes_dir(data, version).display().to_string()];
    Some((env, args))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn temp(name: &str) -> PathBuf {
        let dir = std::env::temp_dir().join(format!("openrac-enhanced-{name}-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&dir);
        std::fs::create_dir_all(&dir).unwrap();
        dir
    }

    #[test]
    fn off_until_switched_on_and_settings_land_in_the_ports_file() {
        let home = temp("settings");
        let file = settings_file(&home, "rac1", "pal");
        std::fs::create_dir_all(file.parent().unwrap()).unwrap();
        std::fs::write(&file, "msaa = 4\ngtao = 1\n").unwrap();
        let v = view(&home, "rac1", "pal", BakeStatus::default());
        assert!(!v.entry.enabled);
        let value =
            |v: &EnhancedView, key: &str| v.entry.settings.iter().find(|s| s.spec.key == key).unwrap().value.clone();
        assert_eq!(value(&v, "gtao"), Value::Bool(true));
        assert_eq!(value(&v, "probe_lighting"), Value::Bool(true));
        assert_eq!(value(&v, "probe_gi_multiplier"), Value::from(1.0));

        set_enabled(&home, "rac1", "pal", true).unwrap();
        set_setting(&home, "rac1", "pal", "probe_gi_multiplier", &Value::from(2.5)).unwrap();
        set_setting(&home, "rac1", "pal", "gtao_quality", &Value::from("quality")).unwrap();
        set_setting(&home, "rac1", "pal", "enhanced_shadows", &Value::Bool(false)).unwrap();
        let text = std::fs::read_to_string(&file).unwrap();
        assert!(text.starts_with("msaa = 4\ngtao = 1\n"), "{text}");
        for line in
            ["enhanced_rendering = 1", "probe_gi_multiplier = 2.5", "gtao_quality = quality", "enhanced_shadows = 0"]
        {
            assert!(text.lines().any(|l| l == line), "{line} in {text}");
        }
        let v = view(&home, "rac1", "pal", BakeStatus::default());
        assert!(v.entry.enabled);
        assert_eq!(value(&v, "probe_gi_multiplier"), Value::from(2.5));
        // Out of the port's range, another type, or not one of the entry's: refused, file kept.
        assert!(set_setting(&home, "rac1", "pal", "probe_gi_multiplier", &Value::from(9.0)).is_err());
        assert!(set_setting(&home, "rac1", "pal", "gtao", &Value::from("yes")).is_err());
        assert!(set_setting(&home, "rac1", "pal", "probe_spheres", &Value::Bool(true)).is_err());
        assert_eq!(std::fs::read_to_string(&file).unwrap(), text);
        let _ = std::fs::remove_dir_all(&home);
    }

    #[test]
    fn play_takes_vulkan_and_the_bakes_only_when_on() {
        let data = Path::new("/data/rac1");
        assert_eq!(play_additions("", data, "pal"), None);
        assert_eq!(play_additions("enhanced_rendering = 0\n", data, "pal"), None);
        let (env, args) = play_additions("enhanced_rendering = 1\n", data, "pal").unwrap();
        assert_eq!(env, vec![("OPENRAC_GL_API".to_string(), "vulkan".to_string())]);
        assert_eq!(args, vec!["--probes".to_string(), Path::new("/data/rac1/probes/pal").display().to_string()]);
    }

    #[test]
    fn a_level_counts_as_ready_while_its_export_and_baker_are_the_bakes() {
        let root = temp("bake");
        let (levels, probes, baker) = (root.join("levels"), root.join("probes"), root.join("baker"));
        for name in ["level_00", "level_01", "level_99"] {
            std::fs::create_dir_all(levels.join(name)).unwrap();
            std::fs::write(levels.join(name).join("manifest.json"), name).unwrap();
        }
        std::fs::create_dir_all(levels.join("textures")).unwrap();
        let none = bake_status(&levels, &probes, &baker);
        assert_eq!((none.available, none.levels, none.ready), (false, 3, 0));
        std::fs::write(&baker, "baker").unwrap();
        let stamp = |name: &str, manifest: &str| {
            let dir = probes.join(name);
            std::fs::create_dir_all(&dir).unwrap();
            std::fs::write(dir.join("sparse.orpg"), "").unwrap();
            let text = serde_json::json!({
                "manifest_sha256": format!("{:x}", Sha256::digest(manifest.as_bytes())),
                "baker_sha256": format!("{:x}", Sha256::digest(b"baker")),
            });
            std::fs::write(dir.join(STAMP), text.to_string()).unwrap();
        };
        stamp("level_00", "level_00");
        stamp("level_01", "an older export");
        std::fs::create_dir_all(probes.join(".level_99.failed")).unwrap();
        let some = bake_status(&levels, &probes, &baker);
        assert_eq!((some.available, some.levels, some.ready, some.failed), (true, 3, 1, 1));
        // A new baker (a launcher update): every bake is made again.
        std::fs::write(&baker, "another baker").unwrap();
        assert_eq!(bake_status(&levels, &probes, &baker).ready, 0);
        let _ = std::fs::remove_dir_all(&root);
    }

    #[test]
    fn written_values_read_back_as_the_port_parses_them() {
        assert_eq!(written(&Value::from(1.0)), "1");
        assert_eq!(written(&Value::from(1.25)), "1.25");
        assert_eq!(written(&Value::from(0.05)), "0.05");
        assert_eq!(written(&Value::Bool(false)), "0");
    }
}
