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
    Ok(Plan {
        title: format!("Set up {game} from your disc"),
        program: python,
        args: vec![
            "tools/extractor.py".into(),
            image.display().to_string(),
            "--game".into(),
            game.into(),
            "--extract".into(),
            "--validate".into(),
            "--proj-path".into(),
            data_dir(install, game).display().to_string(),
        ],
        cwd: root.to_path_buf(),
        detached: false,
        env: Vec::new(),
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
    }
    Ok(())
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
}
