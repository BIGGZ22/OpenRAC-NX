//! The catalogue, each version's status and its actions, put together for
//! the UI in one call; and the way back, from an action's name to its plan.

use std::path::PathBuf;

use serde::Serialize;

use crate::actions::{self, ActionFile, ActionView, Context, Inputs, Plan, Platform, Scope};
use crate::catalog::{self, Version};
use crate::config::Config;
use crate::install::{self, InstallState};
use crate::port_build;
use crate::status::{self, DiscState, VersionStatus};

#[derive(Debug, Clone, Serialize, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct Library {
    pub root: PathBuf,
    pub platform: Platform,
    /// toolchains/ holds a compiler.
    pub toolchains: bool,
    pub repository: Vec<ActionView>,
    pub games: Vec<GameView>,
    /// launcher/actions.json could not be read: why. The games still show,
    /// without actions.
    pub actions_error: Option<String>,
}

#[derive(Debug, Clone, Serialize, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct GameView {
    pub id: String,
    pub title: String,
    pub year: Option<u32>,
    /// How far the game is set up from the player's disc; None while no
    /// install directory is known.
    pub install: Option<InstallState>,
    pub versions: Vec<VersionView>,
}

#[derive(Debug, Clone, Serialize, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct VersionView {
    #[serde(flatten)]
    pub version: Version,
    pub status: VersionStatus,
    pub actions: Vec<ActionView>,
    /// The game is built, but from an older OpenRAC than the checkout holds now (an update): it is
    /// built again from scratch before it is played (port_build.rs).
    pub port_outdated: bool,
}

/// A version's status, with the disc image a set-up from the player's disc
/// kept counting as its disc when `baserom/` has none.
fn version_status(root: &std::path::Path, config: &Config, version: &Version) -> VersionStatus {
    let mut status = status::version_status(root, version);
    if status.disc.state != DiscState::Found {
        let installed =
            config.install_dir.as_deref().and_then(|dir| install::disc_image(dir, &version.game, &version.serial));
        if let Some(image) = installed {
            status.disc.state = DiscState::Found;
            status.disc.message = "set up from your disc".into();
            status.disc.path = Some(image);
        }
    }
    status
}

pub fn library(config: &Config) -> Result<Library, String> {
    let root = config.root()?.to_path_buf();
    let catalog = catalog::load_with(&root, config.progress_cache.as_deref())?;
    let (file, actions_error) = match actions::load(&root) {
        Ok(file) => (Some(file), None),
        Err(e) => (None, Some(e)),
    };
    let toolchains = status::toolchains_present(&root);
    let platform = Platform::current();
    let views = |list: Option<&Vec<actions::Action>>, ctx: &Context| -> Vec<ActionView> {
        list.map(|l| l.iter().map(|a| a.view(ctx)).collect()).unwrap_or_default()
    };

    let repo_ctx = Context {
        root: &root,
        config,
        version: None,
        status: None,
        toolchains,
        platform,
        level: None,
        path: None,
        option: None,
    };
    let repository = views(file.as_ref().map(|f| &f.repository), &repo_ctx);
    let games = catalog
        .games
        .into_iter()
        .map(|game| GameView {
            install: config.install_dir.as_deref().map(|dir| install::state(dir, &game.id)),
            versions: game
                .versions
                .into_iter()
                .map(|version| {
                    let status = version_status(&root, config, &version);
                    let ctx = Context {
                        root: &root,
                        config,
                        version: Some(&version),
                        status: Some(&status),
                        toolchains,
                        platform,
                        level: None,
                        path: None,
                        option: None,
                    };
                    let actions = views(file.as_ref().and_then(|f| f.versions.get(&version.key)), &ctx);
                    let built = actions.iter().any(|a| a.id == "play" && a.this_platform && a.done);
                    let port_outdated = built && port_build::outdated(&root, &version.key.replace('/', "-"));
                    VersionView { version, status, actions, port_outdated }
                })
                .collect(),
            id: game.id,
            title: game.title,
            year: game.year,
        })
        .collect();
    Ok(Library { root, platform, toolchains, repository, games, actions_error })
}

/// The command line for action `id` in `scope`, checked against the
/// checkout as it is now (not as the UI last saw it).
pub fn plan(config: &Config, scope: &Scope, id: &str, inputs: &Inputs) -> Result<Plan, String> {
    let (level, path, option) = (inputs.level, inputs.path.as_deref(), inputs.option.as_deref());
    let root = config.root()?;
    let file: ActionFile = actions::load(root)?;
    let action = file.find(scope, id).ok_or_else(|| format!("no action {id:?} for {scope:?}"))?;
    let toolchains = status::toolchains_present(root);
    let platform = Platform::current();
    match scope {
        Scope::Repository => action.plan(&Context {
            root,
            config,
            version: None,
            status: None,
            toolchains,
            platform,
            level,
            path,
            option,
        }),
        Scope::Version(key) => {
            let catalog = catalog::load(root)?;
            let version = catalog.version(key).ok_or_else(|| format!("no version {key}"))?;
            let status = version_status(root, config, version);
            action.plan(&Context {
                root,
                config,
                version: Some(version),
                status: Some(&status),
                toolchains,
                platform,
                level,
                path,
                option,
            })
        }
    }
}

/// Whether action `id` of `scope` plays a game (ActionFile::is_play), from the checkout's
/// actions.json as it is now. False without a checkout or a file that reads; a repository action
/// is answered without reading it.
pub fn is_play(config: &Config, scope: &Scope, id: &str) -> bool {
    let (Scope::Version(_), Some(root)) = (scope, &config.root) else { return false };
    actions::load(root).is_ok_and(|file| file.is_play(scope, id))
}

/// A version's action's arguments and artifact, filled in, whether or not it can run now
/// (Action::filled).
pub fn filled(config: &Config, key: &str, id: &str) -> Result<(Vec<String>, Option<std::path::PathBuf>), String> {
    let root = config.root()?;
    let file: ActionFile = actions::load(root)?;
    let scope = Scope::Version(key.to_string());
    let action = file.find(&scope, id).ok_or_else(|| format!("no action {id:?} for {key}"))?;
    let catalog = catalog::load(root)?;
    let version = catalog.version(key).ok_or_else(|| format!("no version {key}"))?;
    let status = version_status(root, config, version);
    action.filled(&Context {
        root,
        config,
        version: Some(version),
        status: Some(&status),
        toolchains: status::toolchains_present(root),
        platform: Platform::current(),
        level: None,
        path: None,
        option: None,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    /// The versions launcher/actions.json says the level editor reads (an editor-extract that is
    /// not planned): rac1's two, and the later games' in early access.
    fn editable_versions() -> Vec<String> {
        let file = actions::load(&checkout()).unwrap();
        let mut keys: Vec<String> = file
            .versions
            .iter()
            .filter(|(_, list)| list.iter().any(|a| a.id == "editor-extract" && a.state != actions::State::Planned))
            .map(|(key, _)| key.clone())
            .collect();
        keys.sort();
        keys
    }

    /// The Mods page offers "Make your own" for a version whose actions have the editor's
    /// (Mods.svelte, editable): each version the editor reads has them, on this platform.
    fn assert_the_editor_is_offered(library: &Library, editable: &[String]) {
        for key in editable {
            let version = library.games.iter().flat_map(|g| &g.versions).find(|v| &v.version.key == key);
            let version = version.unwrap_or_else(|| panic!("{key} is not in the library"));
            for id in ["editor-extract", "editor-open"] {
                let action = version.actions.iter().find(|a| a.id == id && a.this_platform);
                let action = action.unwrap_or_else(|| panic!("{key} has no {id}"));
                assert_ne!(action.state, actions::State::Planned, "{key} {id}");
            }
        }
    }

    #[test]
    fn every_game_the_editor_reads_offers_it() {
        let editable = editable_versions();
        for key in ["rac1/pal", "rac1/ntsc", "rac2/ntsc", "rac3/ntsc", "rac4/ntsc"] {
            assert!(editable.iter().any(|k| k == key), "{key} is not editable: {editable:?}");
        }
        let config = Config { root: Some(checkout()), ..Config::default() };
        assert_the_editor_is_offered(&library(&config).unwrap(), &editable);
    }

    /// `tauri dev` with a copy left in its resources by an installer built there (0fdd5b1e's, from
    /// before the mods came into the copy): the launcher worked on that copy, whose actions had no
    /// level editor, and the Mods page showed none for any game. A contributor's launcher leaves
    /// it for the checkout it runs from (detect::leave_copy).
    #[test]
    fn a_contributors_launcher_leaves_an_old_copy_for_its_checkout() {
        let tmp = tempfile::tempdir().unwrap();
        let copy = tmp.path().join("openrac");
        for dir in ["tools", "games/rac1", "launcher", "python/bin"] {
            std::fs::create_dir_all(copy.join(dir)).unwrap();
        }
        std::fs::write(copy.join("STANDALONE"), "commit: 0fdd5b1e\n").unwrap();
        std::fs::write(copy.join("tools/openrac.py"), "").unwrap();
        std::fs::write(copy.join("python/bin/python3"), "").unwrap();
        std::fs::copy(checkout().join("games/rac1/game.json"), copy.join("games/rac1/game.json")).unwrap();
        std::fs::write(copy.join("launcher/actions.json"), r#"{"format": 1, "versions": {"rac1/pal": []}}"#).unwrap();
        let copy = crate::detect::bundled_root(tmp.path()).expect("a carried copy");

        let mut config = Config {
            root: Some(copy.clone()),
            python: Some(copy.join("python/bin/python3")),
            developer: true,
            ..Config::default()
        };
        let stale = library(&config).unwrap();
        let pal = stale.games.iter().flat_map(|g| &g.versions).find(|v| v.version.key == "rac1/pal").unwrap();
        assert!(!pal.actions.iter().any(|a| a.id == "editor-extract"), "the old copy reproduces the bug");

        crate::detect::leave_copy(&mut config, &copy, &[checkout().join("launcher")]);
        let root = config.root.clone().expect("the checkout");
        assert_eq!(std::fs::canonicalize(&root).unwrap(), std::fs::canonicalize(checkout()).unwrap());
        assert!(!config.python.as_deref().is_some_and(|p| p.starts_with(&copy)), "{:?}", config.python);
        assert_the_editor_is_offered(&library(&config).unwrap(), &editable_versions());
    }

    /// An installed launcher's copy (tools/bundle_standalone.py, as an installer makes it) offers
    /// the level editor for the same games as a checkout, working in the data folder.
    #[test]
    fn an_installed_launchers_copy_offers_the_editor() {
        let tmp = tempfile::tempdir().unwrap();
        let out = tmp.path().join("openrac");
        let python = if cfg!(windows) { "python" } else { "python3" };
        let Ok(status) = std::process::Command::new(python)
            .arg(checkout().join("tools/bundle_standalone.py"))
            .arg(&out)
            .stdout(std::process::Stdio::null())
            .status()
        else {
            return; // no Python here to put a copy together
        };
        assert!(status.success(), "bundle_standalone.py ended with {status}");
        let install = tmp.path().join("install");
        let config = Config { root: Some(out), install_dir: Some(install), ..Config::default() };
        let library = library(&config).unwrap();
        assert_eq!(library.actions_error, None);
        assert_the_editor_is_offered(&library, &editable_versions());
    }

    #[test]
    fn the_checkouts_play_actions_play() {
        let config = Config { root: Some(checkout()), ..Config::default() };
        let pal = Scope::Version("rac1/pal".into());
        assert!(is_play(&config, &pal, "play"));
        assert!(!is_play(&config, &pal, "build"));
        assert!(!is_play(&config, &Scope::Repository, "discs"));
        assert!(!is_play(&Config::default(), &pal, "play"), "no checkout");
        let elsewhere = tempfile::tempdir().unwrap();
        let unreadable = Config { root: Some(elsewhere.path().to_path_buf()), ..Config::default() };
        assert!(!is_play(&unreadable, &pal, "play"), "no actions.json");
    }

    #[test]
    fn the_checkout_makes_a_library() {
        let config = Config { root: Some(checkout()), ..Config::default() };
        let library = library(&config).unwrap();
        assert_eq!(library.actions_error, None);
        assert_eq!(library.games.len(), 6);
        assert!(library.repository.iter().any(|a| a.id == "discs"));
        let json = serde_json::to_value(&library).unwrap();
        // The version's own fields sit next to status and actions.
        let pal = &json["games"][0]["versions"][0];
        assert_eq!(pal["key"], "rac1/pal");
        assert!(pal["status"]["disc"]["state"].is_string());
        assert!(pal["actions"].is_array());
    }

    #[test]
    fn plans_are_refused_until_their_needs_are_met() {
        let config = Config { root: Some(checkout()), ..Config::default() };
        let err = plan(&config, &Scope::Repository, "discs", &Inputs::default()).unwrap_err();
        assert!(err.contains("Python"), "{err}");
        let python = crate::detect::which("python3").unwrap_or_else(|| "/usr/bin/python3".into());
        let config = Config { python: Some(python.clone()), ..config };
        let plan = plan(&config, &Scope::Repository, "discs", &Inputs::default()).unwrap();
        assert_eq!(plan.program, python);
        assert_eq!(plan.args, ["tools/openrac.py", "discs"]);
        assert!(super::plan(&config, &Scope::Repository, "nope", &Inputs::default()).is_err());
        assert!(super::plan(&config, &Scope::Version("rac9/pal".into()), "build", &Inputs::default()).is_err());
    }

    /// A copy of the checkout's game manifests and actions in a temporary
    /// folder, so a test can put files in place without touching the checkout.
    fn scratch_checkout() -> tempfile::TempDir {
        let dir = tempfile::tempdir().unwrap();
        let root = checkout();
        for entry in std::fs::read_dir(root.join("games")).unwrap() {
            let manifest = entry.unwrap().path().join("game.json");
            if manifest.is_file() {
                let game = manifest.parent().unwrap().file_name().unwrap().to_owned();
                std::fs::create_dir_all(dir.path().join("games").join(&game)).unwrap();
                std::fs::copy(&manifest, dir.path().join("games").join(&game).join("game.json")).unwrap();
            }
        }
        std::fs::create_dir_all(dir.path().join("launcher")).unwrap();
        std::fs::copy(root.join(actions::FILE), dir.path().join(actions::FILE)).unwrap();
        dir
    }

    #[test]
    fn previews_a_level_once_the_godot_project_exists() {
        let dir = scratch_checkout();
        let config =
            Config { root: Some(dir.path().to_path_buf()), godot: Some("/usr/bin/godot".into()), ..Config::default() };
        let scope = Scope::Version("rac1/pal".into());
        // Listed without a level, an action stands for level 00.
        let err = plan(&config, &scope, "editor-preview", &Inputs::default()).unwrap_err();
        assert!(err.contains("assets/levels/rac1/pal/level_00/project.godot"), "{err}");

        // Each level is a project of its own: the one asked for is the one opened.
        let project = dir.path().join("assets/levels/rac1/pal/level_07");
        std::fs::create_dir_all(&project).unwrap();
        std::fs::write(project.join("project.godot"), "").unwrap();
        assert!(
            plan(&config, &scope, "editor-preview", &Inputs { level: Some(3), ..Inputs::default() }).is_err(),
            "level 03 was not extracted"
        );
        let plan = plan(&config, &scope, "editor-preview", &Inputs { level: Some(7), ..Inputs::default() }).unwrap();
        assert_eq!(plan.program, PathBuf::from("/usr/bin/godot"));
        assert!(plan.detached);
        assert_eq!(plan.args, ["--path", "assets/levels/rac1/pal/level_07", "res://levels/level_07/level_07.tscn"]);
    }

    /// The port is one program for Windows, Linux and macOS: building it and playing it are
    /// offered on all three. (Play was once listed for Windows and Linux only, which hid it on
    /// a Mac where the port had been built and run.)
    #[test]
    fn building_and_playing_the_port_are_not_tied_to_a_platform() {
        let file = actions::load(&checkout()).unwrap();
        let scope = Scope::Version("rac1/pal".into());
        for id in ["port-build", "play", "play-edited"] {
            let action = file.find(&scope, id).unwrap_or_else(|| panic!("rac1/pal has no {id}"));
            assert!(action.platforms.is_empty(), "{id} is limited to {:?}", action.platforms);
        }
    }

    /// Android plays from the game its APK carries and gets the levels ready with the Python it
    /// carries: those steps name it, building does not. (They once named no platform, which hid
    /// them there, so a game set up from its disc could never be played.)
    #[test]
    fn android_gets_the_levels_ready_but_builds_nothing() {
        let file = actions::load(&checkout()).unwrap();
        for key in ["rac1/pal", "rac1/ntsc"] {
            let scope = Scope::Version(key.into());
            for id in ["port-levels", "port-levels-check", "probe-bake"] {
                let action = file.find(&scope, id).unwrap_or_else(|| panic!("{key} has no {id}"));
                assert!(action.platforms.contains(&Platform::Android), "{key} {id} does not run on Android");
            }
            assert!(file.find(&scope, "port-build").unwrap().platforms.is_empty());
        }
    }

    /// Play draws from the levels `port-levels` writes, so it names them and waits for them.
    /// (It once started without: menus with no world behind them.)
    #[test]
    fn play_waits_for_the_levels_it_draws() {
        let file = actions::load(&checkout()).unwrap();
        let scope = Scope::Version("rac1/pal".into());
        let made = file.find(&scope, "port-levels").and_then(|a| a.artifact.clone()).expect("port-levels makes a file");
        // Play waits for the check, which waits for the export: levels that are there but cannot
        // be drawn (an export cut short, a file gone) were a black world behind the HUD.
        let check = file.find(&scope, "port-levels-check").expect("port-levels-check");
        let stamp = check.makes.clone().expect("the check leaves a stamp");
        assert!(check.needs.iter().any(|n| n.path == made), "the check does not wait for {made}");
        assert!(check.args.iter().any(|a| a == "--check-levels"));
        for id in ["play", "play-edited"] {
            let action = file.find(&scope, id).unwrap();
            assert!(action.needs.iter().any(|n| n.path == stamp), "{id} does not wait for {stamp}");
            let at = action.args.iter().position(|a| a == "--levels").unwrap_or_else(|| panic!("{id} has no --levels"));
            let dir = action.args[at + 1].trim_start_matches("{root}/");
            assert!(made.starts_with(dir) && stamp.starts_with(dir), "{id} draws from {dir}");
        }
    }

    /// NTSC-U's levels are read from its own disc into a folder of their own, checked by its own
    /// program, never over PAL's: the two versions share the game's data folder.
    #[test]
    fn ntsc_levels_stay_apart_from_pals() {
        let file = actions::load(&checkout()).unwrap();
        let (pal, ntsc) = (Scope::Version("rac1/pal".into()), Scope::Version("rac1/ntsc".into()));
        let made = |scope: &Scope| file.find(scope, "port-levels").and_then(|a| a.artifact.clone()).unwrap();
        let (pal_made, ntsc_made) = (made(&pal), made(&ntsc));
        let folder = |path: &str| path.rsplitn(3, '/').nth(2).unwrap().to_string();
        assert_ne!(folder(&pal_made), folder(&ntsc_made));
        assert!(!ntsc_made.starts_with(&format!("{}/", folder(&pal_made))), "{ntsc_made} is inside PAL's levels");
        let check = file.find(&ntsc, "port-levels-check").expect("rac1/ntsc port-levels-check");
        assert!(check.needs.iter().any(|n| n.path == ntsc_made));
        assert!(check.makes.as_deref().is_some_and(|m| m.starts_with(&folder(&ntsc_made))), "{:?}", check.makes);
        let at = check.args.iter().position(|a| a == "--levels").expect("the check names its levels");
        assert_eq!(check.args[at + 1].trim_start_matches("{root}/"), folder(&ntsc_made));
        assert!(check.artifact.as_deref().is_some_and(|a| a.contains("openrac-rac1-ntsc")));
        let levels = file.find(&ntsc, "port-levels").unwrap();
        assert!(levels.args.iter().any(|a| a == &folder(&ntsc_made)));
    }

    /// The copy an installer carries (tools/bundle_standalone.py), when one was put together in
    /// build/standalone: its actions are valid, need no Python to play, and write nothing in it.
    #[test]
    fn a_carried_copy_offers_the_players_actions() {
        let root = checkout().join("build").join("standalone");
        if !root.join("STANDALONE").is_file() {
            return; // not put together here
        }
        let catalog = crate::catalog::load(&root).unwrap();
        let file = actions::load(&root).unwrap();
        assert_eq!(file.problems(&catalog), Vec::<String>::new());
        let scope = Scope::Version("rac1/pal".into());
        let levels = file.find(&scope, "port-levels").expect("port-levels");
        assert!(levels.args.iter().any(|a| a == "{data}/port/pal"), "{:?}", levels.args);
        let play = file.find(&scope, "play").expect("play");
        assert!(play.needs.iter().all(|n| n.path.starts_with("{data}/")), "{:?}", play.needs);
        let check = file.find(&scope, "port-levels-check").expect("port-levels-check");
        assert!(check.makes.as_deref().is_some_and(|m| m.starts_with("{data}/")), "{:?}", check.makes);
        assert!(file.find(&scope, "port-build").is_none() && file.repository.is_empty());

        // With a game set up (OPENRAC_TEST_INSTALL: an install folder whose active/rac1/data has
        // the disc and the levels), Play is planned as the launcher plans it and started as it
        // starts it, for a few frames: the program, its arguments and the folder it starts in
        // are all the copy's own. 0.4.0 planned a folder the copy does not have.
        let Some(install) = std::env::var_os("OPENRAC_TEST_INSTALL") else { return };
        let config = Config {
            root: Some(root.clone()),
            install_dir: Some(install.into()),
            python: crate::detect::bundled_python(&root),
            ..Config::default()
        };
        let plan = plan(&config, &scope, "play", &Inputs::default()).unwrap();
        assert!(plan.cwd.is_dir(), "play would start in {}", plan.cwd.display());
        assert!(plan.program.is_file(), "{}", plan.program.display());
        let status = std::process::Command::new(&plan.program)
            .args(&plan.args)
            .args(["--frames", "120", "--skip-movies"])
            .current_dir(&plan.cwd)
            .status()
            .unwrap();
        assert!(status.success(), "the game ended with {status}");
    }

    #[test]
    fn play_runs_only_once_the_port_is_built() {
        let root = checkout();
        let config = Config { root: Some(root.clone()), ..Config::default() };
        let library = library(&config).unwrap();
        let file = actions::load(&root).unwrap();
        for version in library.games.iter().flat_map(|g| &g.versions) {
            let key = &version.version.key;
            let play = version.actions.iter().find(|a| a.id == "play");
            let play = play.unwrap_or_else(|| panic!("{key} has no play action"));
            let built = file
                .find(&Scope::Version(key.clone()), "play")
                .and_then(|a| a.artifact.as_deref())
                .is_some_and(|artifact| {
                    let path = crate::under(&root, artifact);
                    if path.exists() {
                        true
                    } else if !cfg!(windows) && artifact.ends_with(".exe") {
                        crate::program_path(crate::under(&root, &artifact[..artifact.len() - 4])).exists()
                    } else {
                        false
                    }
                });
            assert!(!play.runnable || built, "{key}: play runs before the port is built");
        }
    }
}
