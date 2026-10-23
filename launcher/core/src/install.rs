//! Setting a game up from the player's own disc, the way OpenGOAL's launcher
//! does: the player picks the image, the launcher runs the extractor
//! (`tools/extractor.py`, OpenGOAL's `extractor`) as a job, and the game's
//! state on disk says how far it got. Layout, as OpenGOAL's:
//!
//! ```text
//! <install dir>/active/<game>/data/
//!     iso_data/<game>/        the disc's files, the image, buildinfo.json   (extract)
//!     decompiler_out/<game>/  the assets in the port's formats              (decompile, not yet)
//!     out/<game>/             the built port                                (compile, not yet)
//! ```
//!
//! The install directory is chosen in Settings; by default it is the
//! launcher's per-user data folder. Nothing goes into the OpenRAC checkout.

use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::actions::Plan;
use crate::config::Config;

/// What `tools/extractor.py` writes to `iso_data/<game>/buildinfo.json` (one entry).
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct BuildInfo {
    pub serial: String,
    /// The version it is, `rac1/pal`.
    pub version: String,
    #[serde(rename(deserialize = "elf_sha1"))]
    pub elf_sha1: String,
    pub files: u64,
    pub image: String,
}

/// How far a game's set-up got.
#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct InstallState {
    /// `<install dir>/active/<game>/data`.
    pub dir: PathBuf,
    /// The disc was extracted and validated: which build it is.
    pub extracted: Option<BuildInfo>,
    /// The assets were turned into the port's formats.
    pub decompiled: bool,
    /// The port was built.
    pub compiled: bool,
}

/// `<install dir>/active/<game>/data`, OpenGOAL's project path for one game.
pub fn data_dir(install: &Path, game: &str) -> PathBuf {
    install.join("active").join(game).join("data")
}

/// Where an installed launcher keeps what a checkout keeps under `assets/` (tools/bundle_standalone.py,
/// MOD_FOLDERS): `assets/levels|packed|port/<game>/<version>[/...]` is `<data>/levels|packed|port/<version>[/...]`
/// of that game, in the per-user install folder, since an installed launcher's own folder (an AppImage,
/// the macOS app) is read-only. None for any other path, or one that climbs out with `..`.
pub fn player_folder(install: &Path, rel: &str) -> Option<PathBuf> {
    let parts: Vec<&str> = rel.trim_end_matches('/').split('/').collect();
    match parts.as_slice() {
        ["assets", kind @ ("levels" | "packed" | "port"), game, version, rest @ ..]
            if check_game(game).is_ok()
                && check_game(version).is_ok()
                && rest.iter().all(|p| !p.is_empty() && *p != "." && *p != "..") =>
        {
            let mut out = data_dir(install, game).join(kind).join(version);
            for p in rest {
                out.push(p);
            }
            Some(out)
        }
        _ => None,
    }
}

fn check_game(game: &str) -> Result<&str, String> {
    let ok = !game.is_empty() && game.chars().all(|c| c.is_ascii_alphanumeric() || c == '_' || c == '-');
    if ok {
        Ok(game)
    } else {
        Err(format!("invalid game id {game:?}"))
    }
}

pub fn state(install: &Path, game: &str) -> InstallState {
    let dir = data_dir(install, game);
    let extracted = std::fs::read_to_string(dir.join("iso_data").join(game).join("buildinfo.json"))
        .ok()
        .and_then(|text| serde_json::from_str::<Vec<BuildInfo>>(&text).ok())
        .and_then(|list| list.into_iter().next());
    let filled = |sub: &str| std::fs::read_dir(dir.join(sub).join(game)).is_ok_and(|mut d| d.next().is_some());
    InstallState { decompiled: filled("decompiler_out"), compiled: filled("out"), extracted, dir }
}

/// The disc image the extractor kept for `serial`, if this game was set up from that version.
pub fn disc_image(install: &Path, game: &str, serial: &str) -> Option<PathBuf> {
    let state = state(install, game);
    let image = state.dir.join("iso_data").join(game).join("disc.iso");
    (state.extracted?.serial == serial && image.is_file()).then_some(image)
}

/// The job that extracts and validates `image` for `game`: OpenGOAL's
/// `extractor <iso> --extract --validate --proj-path <data>`.
pub fn plan_extract(config: &Config, install: &Path, game: &str, image: &Path) -> Result<Plan, String> {
    let root = config.root()?;
    let python = config.python.clone().ok_or("Python is not set (Settings)")?;
    let game = check_game(game)?;
    if !image.is_file() {
        return Err(format!("{} is not a file", image.display()));
    }
    let mut args: Vec<String> = vec![
        "tools/extractor.py".into(),
        image.display().to_string(),
        "--game".into(),
        game.into(),
        "--extract".into(),
        "--validate".into(),
        "--proj-path".into(),
        data_dir(install, game).display().to_string(),
    ];
    // A phone's storage: of the disc's files only what the port reads beside the image (every
    // file is still checked), not gigabytes more to write.
    if crate::actions::Platform::current() == crate::actions::Platform::Android {
        args.push("--boot-only".into());
    }
    Ok(Plan {
        title: format!("Set up {game} from your disc"),
        program: python,
        args,
        cwd: root.to_path_buf(),
        detached: false,
        env: Vec::new(),
        output: None,
    })
}

/// Removes what set-up made for `game` (OpenGOAL's uninstall): its
/// `iso_data`, `decompiler_out` and `out` folders, and nothing else.
pub fn uninstall(install: &Path, game: &str) -> Result<(), String> {
    let dir = data_dir(install, check_game(game)?);
    for sub in ["iso_data", "decompiler_out", "out"] {
        let path = dir.join(sub).join(game);
        if path.exists() {
            std::fs::remove_dir_all(&path).map_err(|e| format!("cannot remove {}: {e}", path.display()))?;
        }
        // A developer's other disc's set-up waits beside it as <game>@<version> (disc_choice.rs).
        // It is what the set-up made as well, and goes with it.
        for other in kept_ones(&dir.join(sub), game) {
            std::fs::remove_dir_all(&other).map_err(|e| format!("cannot remove {}: {e}", other.display()))?;
        }
    }
    Ok(())
}

/// The folders of `game` that wait beside it under another name: `<game>@<version>`, the other
/// disc's set-up a developer keeps aside (disc_choice.rs).
fn kept_ones(dir: &Path, game: &str) -> Vec<PathBuf> {
    let prefix = format!("{game}@");
    std::fs::read_dir(dir)
        .map(|entries| {
            entries
                .flatten()
                .filter(|e| e.file_name().to_string_lossy().starts_with(&prefix))
                .map(|e| e.path())
                .filter(|p| p.is_dir())
                .collect()
        })
        .unwrap_or_default()
}

/// The install folder on a system that keeps it in one place of its own (Android: the folder
/// the Files app shows, OPENRAC_GAMES_DIR). A set-up made before in another folder (`earlier`:
/// the install folder the settings name, the launcher's data folder) is moved there when it can
/// be renamed, on one file system; one that cannot stays where it is, and that folder is the
/// install folder. Nothing is moved over a set-up already in `wanted`.
pub fn settle_install_dir(wanted: &Path, earlier: &[PathBuf]) -> PathBuf {
    let active = wanted.join("active");
    if active.exists() {
        return wanted.to_path_buf();
    }
    for old in earlier.iter().filter(|old| old.as_path() != wanted) {
        let from = old.join("active");
        if !from.is_dir() {
            continue;
        }
        let moved = std::fs::create_dir_all(wanted).and_then(|()| std::fs::rename(&from, &active));
        return if moved.is_ok() { wanted.to_path_buf() } else { old.clone() };
    }
    wanted.to_path_buf()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn extracted(install: &Path, game: &str, serial: &str) {
        let iso_data = data_dir(install, game).join("iso_data").join(game);
        std::fs::create_dir_all(&iso_data).unwrap();
        std::fs::write(iso_data.join("disc.iso"), b"image").unwrap();
        std::fs::write(
            iso_data.join("buildinfo.json"),
            format!(
                r#"[{{"serial": "{serial}", "version": "rac1/pal", "elf_sha1": "ab", "files": 3,
                     "contents": "cd", "image": "mine.iso"}}]"#
            ),
        )
        .unwrap();
    }

    #[test]
    fn reads_how_far_set_up_got_and_undoes_it() {
        let dir = tempfile::tempdir().unwrap();
        let install = dir.path();
        assert_eq!(state(install, "rac1").extracted, None);
        assert_eq!(disc_image(install, "rac1", "SCES_509.16"), None);

        extracted(install, "rac1", "SCES_509.16");
        let got = state(install, "rac1");
        assert_eq!(got.extracted.as_ref().unwrap().serial, "SCES_509.16");
        assert_eq!(got.extracted.as_ref().unwrap().elf_sha1, "ab");
        assert!(!got.decompiled && !got.compiled);
        assert!(disc_image(install, "rac1", "SCES_509.16").is_some());
        assert_eq!(disc_image(install, "rac1", "SCUS_971.99"), None);

        let keep = data_dir(install, "rac1").join("saves");
        std::fs::create_dir_all(&keep).unwrap();
        uninstall(install, "rac1").unwrap();
        assert_eq!(state(install, "rac1").extracted, None);
        assert!(keep.exists(), "uninstall removes only what set-up made");
        assert!(uninstall(install, "../rac1").is_err());
    }

    #[test]
    fn removes_the_other_discs_set_up_too() {
        let dir = tempfile::tempdir().unwrap();
        let install = dir.path();
        extracted(install, "rac1", "SCES_509.16");
        // The other disc's set-up waits beside it (disc_choice.rs).
        let kept = data_dir(install, "rac1").join("iso_data").join("rac1@pal");
        std::fs::create_dir_all(&kept).unwrap();
        std::fs::write(kept.join("disc.iso"), b"image").unwrap();

        uninstall(install, "rac1").unwrap();
        assert!(!kept.exists(), "the other disc's set-up goes with it");
        assert_eq!(state(install, "rac1").extracted, None);
    }

    #[test]
    fn plans_the_extractor() {
        let dir = tempfile::tempdir().unwrap();
        let image = dir.path().join("my disc.iso");
        std::fs::write(&image, b"x").unwrap();
        let config =
            Config { root: Some("/openrac".into()), python: Some("/usr/bin/python3".into()), ..Config::default() };
        let plan = plan_extract(&config, Path::new("/data"), "rac1", &image).unwrap();
        assert_eq!(plan.program, PathBuf::from("/usr/bin/python3"));
        assert_eq!(plan.args[0], "tools/extractor.py");
        assert_eq!(plan.args[1], image.display().to_string());
        assert_eq!(plan.args[2..7], ["--game", "rac1", "--extract", "--validate", "--proj-path"]);
        // Joined as the system joins paths: with backslashes on Windows.
        let data = Path::new("/data").join("active").join("rac1").join("data");
        assert_eq!(plan.args[7], data.display().to_string());
        assert_eq!(plan.cwd, PathBuf::from("/openrac"));
        assert!(plan_extract(&Config::default(), Path::new("/data"), "rac1", &image).is_err());
        assert!(plan_extract(&config, Path::new("/data"), "rac 1", &image).is_err());
    }

    #[test]
    fn an_installed_launchers_folders_are_in_the_data_folder() {
        let install = Path::new("/home/me/OpenRAC");
        let data = data_dir(install, "rac1");
        assert_eq!(player_folder(install, "assets/levels/rac1/pal"), Some(data.join("levels").join("pal")));
        assert_eq!(player_folder(install, "assets/packed/rac1/ntsc/"), Some(data.join("packed").join("ntsc")));
        assert_eq!(
            player_folder(install, "assets/port/rac1/pal/level_00"),
            Some(data.join("port").join("pal").join("level_00"))
        );
        assert_eq!(player_folder(install, "assets/levels/rac1"), None);
        assert_eq!(player_folder(install, "assets/levels/rac1/pal/../../x"), None);
        assert_eq!(player_folder(install, "assets/godot/rac1/pal"), None);
        assert_eq!(player_folder(install, "editor/README.md"), None);
    }

    #[test]
    fn an_earlier_set_up_moves_into_the_wanted_install_folder() {
        let tmp = std::env::temp_dir().join(format!("openrac-settle-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&tmp);
        let wanted = tmp.join("files").join("games");
        let old = tmp.join("data");
        // Nothing yet: the wanted folder, made on the first set-up.
        assert_eq!(settle_install_dir(&wanted, std::slice::from_ref(&old)), wanted);
        // A game set up in the launcher's data folder moves, with what it holds.
        std::fs::create_dir_all(old.join("active/rac1/data/iso_data")).unwrap();
        std::fs::write(old.join("active/rac1/data/iso_data/buildinfo.json"), "{}").unwrap();
        assert_eq!(settle_install_dir(&wanted, std::slice::from_ref(&old)), wanted);
        assert!(wanted.join("active/rac1/data/iso_data/buildinfo.json").is_file());
        assert!(!old.join("active").exists());
        // A set-up in the wanted folder is never replaced by an older one.
        std::fs::create_dir_all(old.join("active/rac2")).unwrap();
        assert_eq!(settle_install_dir(&wanted, &[old.clone(), wanted.clone()]), wanted);
        assert!(old.join("active/rac2").is_dir() && !wanted.join("active/rac2").exists());
        std::fs::remove_dir_all(&tmp).unwrap();
    }
}
