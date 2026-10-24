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
//! `OpenRAC` folder in the user's home, and the launcher's per-user data
//! folder where that folder cannot be made ([`settle_install_dir`]). Nothing
//! goes into the OpenRAC checkout.

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

/// The install folder at each start, before anything reads it (src-tauri's `setup`).
///
/// OpenGOAL asks for an install folder at first start; until the player picks one, games are set
/// up in the `OpenRAC` folder of `home` (`/home/<user>/OpenRAC`, `/Users/<user>/OpenRAC`,
/// `C:\Users\<user>\OpenRAC`), made here when it is missing. Launchers before that folder kept them
/// in `local_data`, the launcher's per-user data folder: a config that still names it, or names
/// none, is moved to the `OpenRAC` folder, its `active/` renamed there when the `OpenRAC` folder
/// has none yet, and the config saved to `config_file`. A folder the player chose is left as it is.
/// When the `OpenRAC` folder cannot be made (no home, a home that cannot be written), a config
/// naming no folder takes `local_data`, unsaved, as launchers did before.
///
/// Nothing here fails: a rename or a save that does not go through leaves the files where they are
/// and the config in memory pointing at the `OpenRAC` folder, as at every start since.
pub fn settle_install_dir(
    config: &mut Config,
    home: Option<&Path>,
    local_data: Option<&Path>,
    config_file: Option<&Path>,
) {
    let openrac_home = home.and_then(|home| crate::detect::ensure_openrac_dir_in(home).ok());
    let Some(openrac_dir) = openrac_home else {
        if config.install_dir.is_none() {
            config.install_dir = local_data.map(Path::to_path_buf);
        }
        return;
    };
    if config.install_dir.is_some() && config.install_dir.as_deref() != local_data {
        return;
    }
    if let Some(data) = local_data {
        let old_active = data.join("active");
        let new_active = openrac_dir.join("active");
        if old_active.exists() && !new_active.exists() {
            let _ = std::fs::rename(&old_active, &new_active);
        }
    }
    config.install_dir = Some(openrac_dir);
    if let Some(file) = config_file {
        let _ = crate::config::save(file, config);
    }
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

    /// A home, a launcher data folder and a config file path in one temporary folder.
    struct Start {
        _dir: tempfile::TempDir,
        home: PathBuf,
        data: PathBuf,
        file: PathBuf,
    }

    impl Start {
        fn new() -> Start {
            let dir = tempfile::tempdir().unwrap();
            let home = dir.path().join("home");
            let data = dir.path().join("data");
            std::fs::create_dir_all(&home).unwrap();
            std::fs::create_dir_all(&data).unwrap();
            let file = dir.path().join("config").join("launcher.json");
            Start { home, data, file, _dir: dir }
        }

        fn openrac(&self) -> PathBuf {
            self.home.join("OpenRAC")
        }

        fn settle(&self, config: &mut Config) {
            settle_install_dir(config, Some(&self.home), Some(&self.data), Some(&self.file));
        }
    }

    /// A game set up in `install`, as the extractor leaves it: its disc image under `iso_data`.
    fn set_up_game(install: &Path) {
        let marker = data_dir(install, "rac1").join("iso_data").join("rac1").join("disc.iso");
        std::fs::create_dir_all(marker.parent().unwrap()).unwrap();
        std::fs::write(&marker, b"image").unwrap();
    }

    /// Whether [`set_up_game`]'s game is set up in `install`.
    fn set_up_game_is_in(install: &Path) -> bool {
        data_dir(install, "rac1").join("iso_data").join("rac1").join("disc.iso").is_file()
    }

    #[test]
    fn a_fresh_start_makes_the_openrac_folder_the_install_folder() {
        let start = Start::new();
        let mut config = Config::default();
        start.settle(&mut config);
        assert!(start.openrac().is_dir());
        assert_eq!(config.install_dir, Some(start.openrac()));
        assert_eq!(crate::config::load(&start.file).install_dir, Some(start.openrac()), "saved");
    }

    #[test]
    fn the_data_folders_games_move_into_the_openrac_folder() {
        let start = Start::new();
        set_up_game(&start.data);
        let mut config = Config { install_dir: Some(start.data.clone()), ..Config::default() };
        start.settle(&mut config);
        assert!(!start.data.join("active").exists());
        assert!(set_up_game_is_in(&start.openrac()));
        assert_eq!(config.install_dir, Some(start.openrac()));
        assert_eq!(crate::config::load(&start.file).install_dir, Some(start.openrac()));
    }

    #[test]
    fn an_install_folder_already_moved_is_left_as_it_is() {
        let start = Start::new();
        std::fs::create_dir_all(start.openrac()).unwrap();
        set_up_game(&start.openrac());
        let mut config = Config { install_dir: Some(start.openrac()), ..Config::default() };
        start.settle(&mut config);
        assert_eq!(config.install_dir, Some(start.openrac()));
        assert!(set_up_game_is_in(&start.openrac()));
        assert!(!start.file.exists(), "nothing to save");
    }

    #[test]
    fn games_in_both_folders_stay_where_they_are() {
        let start = Start::new();
        set_up_game(&start.data);
        set_up_game(&start.openrac());
        let mut config = Config::default();
        start.settle(&mut config);
        assert!(set_up_game_is_in(&start.data), "the OpenRAC folder's active/ is never replaced");
        assert!(set_up_game_is_in(&start.openrac()));
        assert_eq!(config.install_dir, Some(start.openrac()));
        assert_eq!(crate::config::load(&start.file).install_dir, Some(start.openrac()));
    }

    #[test]
    fn a_folder_the_player_chose_is_kept() {
        let start = Start::new();
        set_up_game(&start.data);
        let chosen = start.home.join("Games");
        let mut config = Config { install_dir: Some(chosen.clone()), ..Config::default() };
        start.settle(&mut config);
        assert_eq!(config.install_dir, Some(chosen));
        assert!(set_up_game_is_in(&start.data));
        assert!(start.openrac().is_dir(), "the OpenRAC folder is made all the same");
        assert!(!start.file.exists());
    }

    #[test]
    fn without_an_openrac_folder_the_data_folder_is_the_install_folder() {
        let start = Start::new();
        // A home that is a file: the OpenRAC folder cannot be made in it, on any system.
        let home = start.home.join("not a folder");
        std::fs::write(&home, b"").unwrap();
        let mut config = Config::default();
        settle_install_dir(&mut config, Some(&home), Some(&start.data), Some(&start.file));
        assert_eq!(config.install_dir, Some(start.data.clone()));
        assert!(!start.file.exists(), "not saved");

        let mut config = Config::default();
        settle_install_dir(&mut config, None, Some(&start.data), Some(&start.file));
        assert_eq!(config.install_dir, Some(start.data.clone()));

        let chosen = start.home.join("Games");
        let mut config = Config { install_dir: Some(chosen.clone()), ..Config::default() };
        settle_install_dir(&mut config, None, Some(&start.data), Some(&start.file));
        assert_eq!(config.install_dir, Some(chosen));
        assert!(!start.file.exists());
    }

    #[cfg(unix)]
    #[test]
    fn a_rename_that_fails_still_points_at_the_openrac_folder() {
        use std::os::unix::fs::PermissionsExt;
        let start = Start::new();
        set_up_game(&start.data);
        let mode = |m| std::fs::Permissions::from_mode(m);
        std::fs::set_permissions(&start.data, mode(0o555)).unwrap();
        // Someone who writes anywhere (root) renames all the same: nothing to see then.
        let writable = std::fs::write(start.data.join("probe"), b"").is_ok();
        let mut config = Config::default();
        start.settle(&mut config);
        std::fs::set_permissions(&start.data, mode(0o755)).unwrap();
        if writable {
            return;
        }
        assert!(set_up_game_is_in(&start.data), "left where it was");
        assert!(!start.openrac().join("active").exists());
        assert_eq!(config.install_dir, Some(start.openrac()));
        assert_eq!(crate::config::load(&start.file).install_dir, Some(start.openrac()));
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
}
