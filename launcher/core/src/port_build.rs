//! Whether a built game is older than the checkout it would be built from now.
//!
//! tools/build_port.py leaves `openrac-build.json` in the build folder after each build that
//! succeeds: the OpenRAC version (launcher/src-tauri/tauri.conf.json) and the commit it built, and
//! the games it built. An update changes one of them (a new launcher carries a checkout of its own
//! version; `git pull` moves the commit), and the game built before it is then outdated: the
//! launcher has it built again from scratch (`build_port.py --clean`) before it is played.
//!
//! A new commit counts only when it changed something the game is built from (BUILT_FROM, asked
//! of git): a pull of docs or of another game's decompilation leaves the build current, so the
//! rebuild every player is made to do after an update is never one for nothing.

use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::sync::{Mutex, OnceLock};
use std::time::Duration;

use serde::Deserialize;

/// The build folder the launcher's build and play actions use (CMake preset `release`).
pub const BUILD_DIR: &str = "port/build/release";
/// The stamp tools/build_port.py writes there.
pub const STAMP: &str = "openrac-build.json";

#[derive(Debug, Default, Deserialize)]
struct Stamp {
    #[serde(default)]
    version: Option<String>,
    #[serde(default)]
    commit: Option<String>,
    #[serde(default)]
    games: Vec<String>,
}

/// The OpenRAC version of the checkout at `root`, as the port's build reads it.
pub fn checkout_version(root: &Path) -> Option<String> {
    let text = std::fs::read_to_string(root.join("launcher/src-tauri/tauri.conf.json")).ok()?;
    let json: serde_json::Value = serde_json::from_str(&text).ok()?;
    json.get("version")?.as_str().map(str::to_string)
}

/// The commit checked out at `root`, read from its `.git` (no git program needed): None when the
/// checkout is not a git one (an installer's copy) or its HEAD cannot be read.
pub fn checkout_commit(root: &Path) -> Option<String> {
    let dot_git = root.join(".git");
    // A worktree's (or submodule's) .git is a file naming its git folder.
    let git_dir = if dot_git.is_file() {
        let text = std::fs::read_to_string(&dot_git).ok()?;
        let named = PathBuf::from(text.trim().strip_prefix("gitdir:")?.trim());
        if named.is_absolute() {
            named
        } else {
            root.join(named)
        }
    } else {
        dot_git
    };
    let head = std::fs::read_to_string(git_dir.join("HEAD")).ok()?;
    let head = head.trim();
    let Some(reference) = head.strip_prefix("ref:").map(str::trim) else {
        return is_sha(head).then(|| head.to_string());
    };
    // Branches live in the common git folder; a worktree's names it.
    let common = match std::fs::read_to_string(git_dir.join("commondir")) {
        Ok(text) => {
            let named = PathBuf::from(text.trim());
            if named.is_absolute() {
                named
            } else {
                git_dir.join(named)
            }
        }
        Err(_) => git_dir.clone(),
    };
    for dir in [&git_dir, &common] {
        if let Ok(text) = std::fs::read_to_string(dir.join(reference)) {
            let sha = text.trim();
            if is_sha(sha) {
                return Some(sha.to_string());
            }
        }
    }
    let packed = std::fs::read_to_string(common.join("packed-refs")).ok()?;
    packed.lines().find_map(|line| {
        let (sha, name) = line.split_once(' ')?;
        (name.trim() == reference && is_sha(sha)).then(|| sha.to_string())
    })
}

fn is_sha(text: &str) -> bool {
    text.len() == 40 && text.bytes().all(|b| b.is_ascii_hexdigit())
}

/// What a game's build reads in the checkout (port/cmake/Games.cmake, tools/build_port.py), besides
/// its own `games/<game>/` and the version, which is compared on its own.
const BUILT_FROM: &[&str] = &[
    "port",
    "tools/build_port.py",
    "editor/moby_classes.tsv",
    "launcher/src-tauri/icons",
    "launcher/src-tauri/tauri.conf.json",
];

/// Whether commit `from` and commit `to` of the checkout at `root` differ in what game `game`
/// (`rac1-pal`) is built from. Changed when git cannot tell (no git, a commit it does not have):
/// an update is never missed for want of an answer. Each answer is kept, since the library asks on
/// every refresh.
fn inputs_changed(root: &Path, game: &str, from: &str, to: &str) -> bool {
    static SEEN: OnceLock<Mutex<HashMap<(PathBuf, String, String, String), bool>>> = OnceLock::new();
    let key = (root.to_path_buf(), game.to_string(), from.to_string(), to.to_string());
    if let Some(&changed) = SEEN.get_or_init(Default::default).lock().unwrap().get(&key) {
        return changed;
    }
    let family = game.split('-').next().unwrap_or(game);
    let mut cmd = crate::detect::quiet(Path::new("git"));
    cmd.arg("-C").arg(root).args(["diff", "--quiet", from, to, "--", &format!("games/{family}")]);
    cmd.args(BUILT_FROM);
    // `git diff --quiet` exits 0 with no difference; 1 (a difference), any other failure and a git
    // that does not start are all errors of run().
    let changed = crate::detect::run(cmd, Duration::from_secs(10)).is_err();
    SEEN.get_or_init(Default::default).lock().unwrap().insert(key, changed);
    changed
}

/// Game `game`'s build (`openrac-<game>`) is older than the checkout at `root`: no stamp (a build
/// from before stamps, or not by build_port.py), another version in it, a commit before a change to
/// what the game is built from, or the game not among those it built. Only asked of a game that is
/// built.
pub fn outdated(root: &Path, game: &str) -> bool {
    let stamp: Stamp = match std::fs::read_to_string(root.join(BUILD_DIR).join(STAMP)) {
        Ok(text) => serde_json::from_str(&text).unwrap_or_default(),
        Err(_) => return true,
    };
    if !stamp.games.iter().any(|g| g == game) {
        return true;
    }
    if stamp.version != checkout_version(root) {
        return true;
    }
    // A checkout whose commit cannot be read (an installer's copy) goes by the version alone.
    match (stamp.commit.as_deref(), checkout_commit(root)) {
        (Some(built), Some(now)) => built != now && inputs_changed(root, game, built, &now),
        _ => false,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const SHA_A: &str = "1111111111111111111111111111111111111111";
    const SHA_B: &str = "2222222222222222222222222222222222222222";

    fn checkout(version: &str, head: &str) -> tempfile::TempDir {
        let dir = tempfile::tempdir().unwrap();
        let root = dir.path();
        std::fs::create_dir_all(root.join("launcher/src-tauri")).unwrap();
        std::fs::write(root.join("launcher/src-tauri/tauri.conf.json"), format!(r#"{{"version": "{version}"}}"#))
            .unwrap();
        std::fs::create_dir_all(root.join(".git/refs/heads")).unwrap();
        std::fs::write(root.join(".git/HEAD"), "ref: refs/heads/master\n").unwrap();
        std::fs::write(root.join(".git/refs/heads/master"), format!("{head}\n")).unwrap();
        std::fs::create_dir_all(root.join(BUILD_DIR)).unwrap();
        dir
    }

    fn stamp(root: &Path, version: &str, commit: Option<&str>, games: &[&str]) {
        let json = serde_json::json!({"version": version, "commit": commit, "games": games});
        std::fs::write(root.join(BUILD_DIR).join(STAMP), json.to_string()).unwrap();
    }

    #[test]
    fn a_build_of_this_version_and_commit_is_current() {
        let dir = checkout("0.8.0", SHA_A);
        stamp(dir.path(), "0.8.0", Some(SHA_A), &["rac1-pal"]);
        assert!(!outdated(dir.path(), "rac1-pal"));
        // Another game, not built with it, is.
        assert!(outdated(dir.path(), "rac1-ntsc"));
    }

    #[test]
    fn an_update_makes_the_build_outdated() {
        let dir = checkout("0.8.1", SHA_A);
        stamp(dir.path(), "0.8.0", Some(SHA_A), &["rac1-pal"]);
        assert!(outdated(dir.path(), "rac1-pal"), "a new version");
        let dir = checkout("0.8.0", SHA_B);
        stamp(dir.path(), "0.8.0", Some(SHA_A), &["rac1-pal"]);
        assert!(outdated(dir.path(), "rac1-pal"), "a pulled commit");
    }

    #[test]
    fn no_stamp_is_outdated() {
        let dir = checkout("0.8.0", SHA_A);
        assert!(outdated(dir.path(), "rac1-pal"));
        std::fs::write(dir.path().join(BUILD_DIR).join(STAMP), "not json").unwrap();
        assert!(outdated(dir.path(), "rac1-pal"));
    }

    #[test]
    fn without_git_the_version_decides() {
        let dir = checkout("0.8.0", SHA_A);
        std::fs::remove_dir_all(dir.path().join(".git")).unwrap();
        stamp(dir.path(), "0.8.0", Some(SHA_B), &["rac1-pal"]);
        assert!(!outdated(dir.path(), "rac1-pal"));
    }

    /// A real repository: a commit to the docs or to another game keeps the build current, one to
    /// the port or to the game's own decompilation does not.
    #[test]
    fn only_a_change_to_what_the_game_is_built_from_counts() {
        let dir = tempfile::tempdir().unwrap();
        let root = dir.path();
        let git = |args: &[&str]| {
            let done = std::process::Command::new("git").arg("-C").arg(root).args(args).output();
            done.is_ok_and(|d| d.status.success())
        };
        if !git(&["init", "-q"]) {
            return; // no git here
        }
        let commit = |path: &str, text: &str| -> String {
            let file = root.join(path);
            std::fs::create_dir_all(file.parent().unwrap()).unwrap();
            std::fs::write(&file, text).unwrap();
            assert!(git(&["add", "-A"]));
            assert!(git(&["-c", "user.name=t", "-c", "user.email=t@t", "commit", "-q", "-m", path]));
            let head = std::process::Command::new("git").arg("-C").arg(root).args(["rev-parse", "HEAD"]).output();
            String::from_utf8(head.unwrap().stdout).unwrap().trim().to_string()
        };
        std::fs::create_dir_all(root.join(BUILD_DIR)).unwrap();
        std::fs::write(root.join(".gitignore"), "port/build/\n").unwrap();
        std::fs::create_dir_all(root.join("launcher/src-tauri")).unwrap();
        let built = commit("launcher/src-tauri/tauri.conf.json", r#"{"version": "0.8.0"}"#);
        stamp(root, "0.8.0", Some(&built), &["rac1-pal"]);
        assert!(!outdated(root, "rac1-pal"));
        commit("docs/notes.md", "notes");
        commit("games/rac2/src/a.c", "int a;");
        assert!(!outdated(root, "rac1-pal"), "docs and another game");
        commit("games/rac1/src/a.c", "int a;");
        assert!(outdated(root, "rac1-pal"), "the game's own decompilation");
        stamp(root, "0.8.0", Some(&commit("port/runtime/b.c", "int b;")), &["rac1-pal"]);
        assert!(!outdated(root, "rac1-pal"));
        commit("port/runtime/b.c", "int b = 1;");
        assert!(outdated(root, "rac1-pal"), "the port");
    }

    #[test]
    fn heads_from_packed_refs_worktrees_and_detached() {
        let dir = checkout("0.8.0", SHA_A);
        let root = dir.path();
        assert_eq!(checkout_commit(root).as_deref(), Some(SHA_A));
        // Packed: no loose ref.
        std::fs::remove_file(root.join(".git/refs/heads/master")).unwrap();
        std::fs::write(root.join(".git/packed-refs"), format!("# pack-refs\n{SHA_B} refs/heads/master\n")).unwrap();
        assert_eq!(checkout_commit(root).as_deref(), Some(SHA_B));
        // Detached.
        std::fs::write(root.join(".git/HEAD"), format!("{SHA_A}\n")).unwrap();
        assert_eq!(checkout_commit(root).as_deref(), Some(SHA_A));
        // A worktree: .git names its folder, whose commondir is the main one.
        let tree = tempfile::tempdir().unwrap();
        let wt_git = root.join(".git/worktrees/w");
        std::fs::create_dir_all(&wt_git).unwrap();
        std::fs::write(wt_git.join("HEAD"), "ref: refs/heads/master\n").unwrap();
        std::fs::write(wt_git.join("commondir"), "../..\n").unwrap();
        std::fs::write(tree.path().join(".git"), format!("gitdir: {}\n", wt_git.display())).unwrap();
        assert_eq!(checkout_commit(tree.path()).as_deref(), Some(SHA_B));
    }

    #[test]
    fn this_checkout_has_a_version_and_a_commit() {
        let root = crate::testing::checkout();
        assert!(checkout_version(&root).is_some());
        assert!(checkout_commit(&root).is_some());
    }
}
