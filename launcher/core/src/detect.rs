//! Finding what the launcher needs on this machine: the OpenRAC checkout,
//! Python, Godot and Docker. Each finder returns candidates with
//! where they came from; the UI offers them, the user picks, and
//! [`check_tool`] says whether a pick works.

use std::ffi::OsString;
use std::path::{Path, PathBuf};
use std::process::{Command, Stdio};
use std::time::{Duration, Instant};

use serde::{Deserialize, Serialize};

use crate::is_openrac_root;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum Tool {
    Python,
    Godot,
    Docker,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Candidate {
    pub path: PathBuf,
    /// Where it was found: `PATH`, `OPENRAC_ROOT`, `next to the launcher`, ...
    pub source: String,
}

#[derive(Debug, Clone, Default, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Detected {
    pub roots: Vec<Candidate>,
    pub pythons: Vec<Candidate>,
    pub godots: Vec<Candidate>,
    pub dockers: Vec<Candidate>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Check {
    pub ok: bool,
    /// What the program says it is (`Python 3.12.4`), when it ran.
    pub version: Option<String>,
    pub message: String,
}

/// Everything at once, for the first-run screen.
pub fn detect_all(start: &[PathBuf], bundled: Option<&Path>) -> Detected {
    let mut roots = roots(start);
    let mut pythons = tool_candidates(Tool::Python);
    // What an installed launcher carries (bundled_root): a checkout that is found goes first, so
    // a contributor's launcher works on their tree; a player has only this one.
    if let Some(dir) = bundled {
        if !roots.iter().any(|c| c.path == dir) {
            roots.push(Candidate { path: dir.to_path_buf(), source: "built in".into() });
        }
        if let Some(python) = bundled_python(dir) {
            pythons.insert(0, Candidate { path: python, source: "built in".into() });
        }
    }
    Detected { roots, pythons, godots: tool_candidates(Tool::Godot), dockers: tool_candidates(Tool::Docker) }
}

/// The copy of OpenRAC an installed launcher carries in its resources (`openrac/`): the games'
/// manifests, the set-up and level tools, a Python to run them and the game built for this
/// system, put together by tools/bundle_standalone.py. With it a player needs only their disc
/// image. `None` when the launcher was built without one (a contributor's `tauri dev`).
pub fn bundled_root(resources: &Path) -> Option<PathBuf> {
    let dir = resources.join("openrac");
    (dir.join("STANDALONE").is_file() && is_openrac_root(&dir)).then_some(dir)
}

/// The Python a bundled copy carries (python-build-standalone's layout).
pub fn bundled_python(root: &Path) -> Option<PathBuf> {
    let rel = if cfg!(windows) { "python/python.exe" } else { "python/bin/python3" };
    let path = crate::under(root, rel);
    path.is_file().then_some(path)
}

/// OpenRAC checkouts: `OPENRAC_ROOT`, then each start folder and its parents
/// (the launcher runs from `launcher/` in a checkout during development),
/// then a few usual places in the home folder.
pub fn roots(start: &[PathBuf]) -> Vec<Candidate> {
    let mut found: Vec<Candidate> = Vec::new();
    let mut add = |path: PathBuf, source: &str| {
        let path = std::fs::canonicalize(&path).unwrap_or(path);
        if is_openrac_root(&path) && !found.iter().any(|c| c.path == path) {
            found.push(Candidate { path, source: source.to_string() });
        }
    };
    if let Some(dir) = std::env::var_os("OPENRAC_ROOT") {
        add(PathBuf::from(dir), "OPENRAC_ROOT");
    }
    for dir in start {
        for ancestor in dir.ancestors() {
            add(ancestor.to_path_buf(), "next to the launcher");
        }
    }
    if let Some(home) = home() {
        for rel in ["OpenRAC", "openrac", "Projects/OpenRAC", "src/OpenRAC", "Documents/OpenRAC"] {
            add(crate::under(&home, rel), "home folder");
        }
    }
    found
}

/// Whether `path` is an OpenRAC checkout, in words.
pub fn check_root(path: &Path) -> Check {
    if !path.is_dir() {
        return Check { ok: false, version: None, message: "not a folder".into() };
    }
    if !is_openrac_root(path) {
        return Check { ok: false, version: None, message: "not an OpenRAC checkout (no tools/openrac.py)".into() };
    }
    match crate::catalog::load(path) {
        Ok(catalog) => {
            let versions = catalog.version_keys().len();
            Check { ok: true, version: None, message: format!("{} games, {versions} versions", catalog.games.len()) }
        }
        Err(e) => Check { ok: false, version: None, message: e },
    }
}

/// The names each tool goes by on PATH, most specific first.
fn names(tool: Tool) -> &'static [&'static str] {
    match tool {
        Tool::Python if cfg!(windows) => &["python", "py", "python3"],
        Tool::Python => &["python3", "python"],
        Tool::Godot => &["godot", "godot4", "Godot", "org.godotengine.Godot"],
        Tool::Docker => &["docker", "podman"],
    }
}

/// Places installers put a tool, outside PATH.
fn usual_places(tool: Tool) -> Vec<PathBuf> {
    let mut places = Vec::new();
    let env = |name: &str| std::env::var_os(name).map(PathBuf::from);
    match tool {
        Tool::Godot => {
            if let Some(godot) = env("GODOT") {
                places.push(godot);
            }
            if cfg!(target_os = "macos") {
                places.push("/Applications/Godot.app/Contents/MacOS/Godot".into());
            } else if cfg!(windows) {
                for base in [env("ProgramFiles"), env("LOCALAPPDATA").map(|d| d.join("Programs"))].into_iter().flatten()
                {
                    places.push(base.join("Godot").join("Godot.exe"));
                }
            } else {
                for p in [
                    "/var/lib/flatpak/exports/bin/org.godotengine.Godot",
                    "/usr/bin/godot",
                    "/usr/bin/godot4",
                    "/usr/local/bin/godot",
                    "/usr/local/bin/godot4",
                ] {
                    places.push(PathBuf::from(p));
                }
                if let Some(home) = home() {
                    places.push(home.join(".local/share/flatpak/exports/bin/org.godotengine.Godot"));
                }
            }
        }
        Tool::Docker if cfg!(target_os = "macos") => places.push("/usr/local/bin/docker".into()),
        // A desktop app started from the Finder or a menu gets a short PATH:
        // on macOS it ends at the system's Python, which is too old for the
        // tools. Homebrew's and a local install's are looked at as well.
        Tool::Python if !cfg!(windows) => {
            places.push("/opt/homebrew/bin/python3".into());
            places.push("/usr/local/bin/python3".into());
        }
        _ => {}
    }
    places
}

/// The oldest Python 3 the tools run on: `editor/` and `tools/` use syntax from 3.10.
pub const PYTHON_MINOR: u32 = 10;

/// Why the Python that answered `--version` with `line` is too old, if it is.
pub fn python_too_old(line: &str) -> Option<String> {
    let version = line.trim().strip_prefix("Python 3.")?;
    let minor: u32 = version.split(|c: char| !c.is_ascii_digit()).next()?.parse().ok()?;
    (minor < PYTHON_MINOR)
        .then(|| format!("{} is too old: the tools need Python 3.{PYTHON_MINOR} or newer", line.trim()))
}

pub fn tool_candidates(tool: Tool) -> Vec<Candidate> {
    let mut found: Vec<Candidate> = Vec::new();
    for path in usual_places(tool) {
        if path.is_file() && !found.iter().any(|c| c.path == path) {
            found.push(Candidate { path, source: "usual place".into() });
        }
    }
    for name in names(tool) {
        if let Some(path) = which(name) {
            if !found.iter().any(|c| c.path == path) {
                found.push(Candidate { path, source: "PATH".into() });
            }
        }
    }
    // The first candidate is what Settings proposes, so a Python that works
    // goes before one that is too old. The order is otherwise kept.
    if tool == Tool::Python {
        let (good, bad): (Vec<_>, Vec<_>) = found.into_iter().partition(|c| check_tool(tool, &c.path).ok);
        found = good.into_iter().chain(bad).collect();
    }
    found
}

/// The flag that makes each tool print its version, and what a good answer contains.
fn version_probe(tool: Tool) -> (&'static [&'static str], &'static str) {
    match tool {
        Tool::Python => (&["--version"], "Python 3."),
        Tool::Godot => (&["--version"], "4."),
        Tool::Docker => (&["--version"], "version"),
    }
}

/// Whether `path` is a working `tool`. Runs it briefly (its version) when it can.
pub fn check_tool(tool: Tool, path: &Path) -> Check {
    if !path.is_file() {
        return Check { ok: false, version: None, message: "file not found".into() };
    }
    let (args, expect) = version_probe(tool);
    if args.is_empty() {
        return Check { ok: true, version: None, message: "found (not run)".into() };
    }
    let mut cmd = quiet(path);
    cmd.args(args);
    match run(cmd, Duration::from_secs(10)) {
        Ok(out) => {
            let line = out.lines().map(str::trim).find(|l| !l.is_empty()).unwrap_or("").to_string();
            if let Some(why) = (tool == Tool::Python).then(|| python_too_old(&line)).flatten() {
                Check { ok: false, message: why, version: Some(line) }
            } else if line.contains(expect) {
                Check { ok: true, message: line.clone(), version: Some(line) }
            } else {
                Check { ok: false, message: format!("unexpected answer: {line}"), version: Some(line) }
            }
        }
        Err(e) => Check { ok: false, version: None, message: e },
    }
}

/// `name` on PATH, with Windows' executable extensions.
pub fn which(name: &str) -> Option<PathBuf> {
    let path = std::env::var_os("PATH")?;
    let exts: Vec<OsString> = if cfg!(windows) {
        std::env::var("PATHEXT")
            .unwrap_or_else(|_| ".EXE;.CMD;.BAT".into())
            .split(';')
            .map(|e| OsString::from(e.to_ascii_lowercase()))
            .collect()
    } else {
        vec![OsString::new()]
    };
    for dir in std::env::split_paths(&path) {
        for ext in &exts {
            let mut file = OsString::from(name);
            file.push(ext);
            let candidate = dir.join(file);
            if candidate.is_file() {
                return Some(candidate);
            }
        }
    }
    None
}

fn home() -> Option<PathBuf> {
    std::env::var_os(if cfg!(windows) { "USERPROFILE" } else { "HOME" }).map(PathBuf::from)
}

#[cfg(windows)]
const CREATE_NO_WINDOW: u32 = 0x0800_0000;

/// A command for a console program that must not flash a console window on Windows.
pub fn quiet(program: &Path) -> Command {
    #[allow(unused_mut)]
    let mut cmd = Command::new(program);
    #[cfg(windows)]
    {
        use std::os::windows::process::CommandExt;
        cmd.creation_flags(CREATE_NO_WINDOW);
    }
    cmd
}

/// Runs a short probe; its output, or why it failed or did not finish in `timeout`.
pub fn run(mut cmd: Command, timeout: Duration) -> Result<String, String> {
    let program = cmd.get_program().to_string_lossy().into_owned();
    let mut child = cmd
        .stdin(Stdio::null())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| format!("cannot start {program}: {e}"))?;
    let start = Instant::now();
    loop {
        match child.try_wait() {
            Ok(Some(_)) => break,
            Ok(None) if start.elapsed() < timeout => std::thread::sleep(Duration::from_millis(25)),
            _ => {
                let _ = child.kill();
                return Err(format!("{program} did not finish within {} s", timeout.as_secs()));
            }
        }
    }
    let out = child.wait_with_output().map_err(|e| e.to_string())?;
    let mut text = String::from_utf8_lossy(&out.stdout).into_owned();
    text.push_str(&String::from_utf8_lossy(&out.stderr));
    if out.status.success() {
        Ok(text)
    } else {
        Err(text.trim().to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    /// A carried copy is one only with its marker; it goes after a checkout that is found, and
    /// its Python before the system's.
    #[test]
    fn a_carried_copy_is_the_last_root_and_its_python_the_first() {
        let tmp = std::env::temp_dir().join(format!("openrac-bundled-{}", std::process::id()));
        let dir = tmp.join("openrac");
        std::fs::create_dir_all(dir.join("tools")).unwrap();
        std::fs::create_dir_all(dir.join("games")).unwrap();
        std::fs::write(dir.join("tools").join("openrac.py"), "").unwrap();
        assert_eq!(bundled_root(&tmp), None, "no marker, no copy");
        std::fs::write(dir.join("STANDALONE"), "").unwrap();
        assert_eq!(bundled_root(&tmp), Some(dir.clone()));

        assert_eq!(bundled_python(&dir), None);
        let python = crate::under(&dir, if cfg!(windows) { "python/python.exe" } else { "python/bin/python3" });
        std::fs::create_dir_all(python.parent().unwrap()).unwrap();
        std::fs::write(&python, "").unwrap();

        let found = detect_all(&[checkout().join("launcher")], Some(&dir));
        assert_eq!(found.roots.last().map(|c| c.path.clone()), Some(dir.clone()));
        assert!(found.roots.len() >= 2, "the checkout comes first: {:?}", found.roots);
        assert_eq!(found.pythons.first().map(|c| c.path.clone()), Some(python));
        std::fs::remove_dir_all(&tmp).unwrap();
    }

    #[test]
    fn finds_the_checkout_above_the_launcher() {
        let start = checkout().join("launcher").join("core");
        let roots = roots(&[start]);
        let want = std::fs::canonicalize(checkout()).unwrap();
        assert!(roots.iter().any(|c| c.path == want), "{roots:?}");
        assert!(check_root(&want).ok);
        assert!(!check_root(&want.join("launcher")).ok);
    }

    #[test]
    fn checks_python_when_it_is_installed() {
        let Some(python) = tool_candidates(Tool::Python).into_iter().next() else { return };
        let check = check_tool(Tool::Python, &python.path);
        assert!(check.ok, "{check:?}");
        assert!(!check_tool(Tool::Python, Path::new("/no/such/python")).ok);
    }

    #[test]
    fn refuses_a_python_older_than_the_tools_need() {
        assert!(python_too_old("Python 3.9.6").is_some());
        assert!(python_too_old("Python 3.10.0").is_none());
        assert!(python_too_old("Python 3.14.0rc2").is_none());
        // Not a Python 3 at all: the caller's other check says so.
        assert!(python_too_old("Python 2.7.18").is_none());
    }
}
