//! The catalogue, each version's status and its actions, put together for
//! the UI in one call; and the way back, from an action's name to its plan.

use std::path::PathBuf;

use serde::Serialize;

use crate::actions::{self, ActionFile, ActionView, Context, Plan, Platform, Scope};
use crate::catalog::{self, Version};
use crate::config::Config;
use crate::install::{self, InstallState};
use crate::status::{self, DiscState, VersionStatus};

#[derive(Debug, Clone, Serialize)]
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

#[derive(Debug, Clone, Serialize)]
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

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct VersionView {
    #[serde(flatten)]
    pub version: Version,
    pub status: VersionStatus,
    pub actions: Vec<ActionView>,
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
    let catalog = catalog::load(&root)?;
    let (file, actions_error) = match actions::load(&root) {
        Ok(file) => (Some(file), None),
        Err(e) => (None, Some(e)),
    };
    let toolchains = status::toolchains_present(&root);
    let platform = Platform::current();
    let views = |list: Option<&Vec<actions::Action>>, ctx: &Context| -> Vec<ActionView> {
        list.map(|l| l.iter().map(|a| a.view(ctx)).collect()).unwrap_or_default()
    };

    let repo_ctx = Context { root: &root, config, version: None, status: None, toolchains, platform, level: None };
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
                    };
                    let actions = views(file.as_ref().and_then(|f| f.versions.get(&version.key)), &ctx);
                    VersionView { version, status, actions }
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
pub fn plan(config: &Config, scope: &Scope, id: &str, level: Option<u32>) -> Result<Plan, String> {
    let root = config.root()?;
    let file: ActionFile = actions::load(root)?;
    let action = file.find(scope, id).ok_or_else(|| format!("no action {id:?} for {scope:?}"))?;
    let toolchains = status::toolchains_present(root);
    let platform = Platform::current();
    match scope {
        Scope::Repository => {
            action.plan(&Context { root, config, version: None, status: None, toolchains, platform, level })
        }
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
            })
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    #[test]
    fn the_checkout_makes_a_library() {
        let config = Config { root: Some(checkout()), ..Config::default() };
        let library = library(&config).unwrap();
        assert_eq!(library.actions_error, None);
        assert_eq!(library.games.len(), 4);
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
        let err = plan(&config, &Scope::Repository, "discs", None).unwrap_err();
        assert!(err.contains("Python"), "{err}");
        let python = crate::detect::which("python3").unwrap_or_else(|| "/usr/bin/python3".into());
        let config = Config { python: Some(python.clone()), ..config };
        let plan = plan(&config, &Scope::Repository, "discs", None).unwrap();
        assert_eq!(plan.program, python);
        assert_eq!(plan.args, ["tools/openrac.py", "discs"]);
        assert!(super::plan(&config, &Scope::Repository, "nope", None).is_err());
        assert!(super::plan(&config, &Scope::Version("rac9/pal".into()), "build", None).is_err());
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
        let err = plan(&config, &scope, "editor-preview", None).unwrap_err();
        assert!(err.contains("assets/levels/rac1/level_00/project.godot"), "{err}");

        // Each level is a project of its own: the one asked for is the one opened.
        let project = dir.path().join("assets/levels/rac1/level_07");
        std::fs::create_dir_all(&project).unwrap();
        std::fs::write(project.join("project.godot"), "").unwrap();
        assert!(plan(&config, &scope, "editor-preview", Some(3)).is_err(), "level 03 was not extracted");
        let plan = plan(&config, &scope, "editor-preview", Some(7)).unwrap();
        assert_eq!(plan.program, PathBuf::from("/usr/bin/godot"));
        assert!(plan.detached);
        assert_eq!(plan.args, ["--path", "assets/levels/rac1/level_07", "res://levels/level_07/level_07.tscn"]);
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
                        crate::under(&root, &artifact[..artifact.len() - 4]).exists()
                    } else {
                        false
                    }
                });
            assert!(!play.runnable || built, "{key}: play runs before the port is built");
        }
    }
}
