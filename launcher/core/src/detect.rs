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
    let dir = plain(&resources.join("openrac"));
    (dir.join("STANDALONE").is_file() && is_openrac_root(&dir)).then_some(dir)
}

/// Which folder and Python the launcher works with, given the copy it carries (`bundled`).
///
/// A player always plays from that copy: nothing to install, nothing to build. A checkout is a
/// contributor's, used only with developer tools on; a player who once cloned OpenRAC (the way
/// in before the installers carried the game) is not sent back to it, where playing needs Python,
/// compilers and a build. With developer tools on, the folder and Python chosen in Settings stand;
/// one that is gone gives way to the carried copy.
pub fn settle(config: &mut crate::config::Config, bundled: Option<&Path>) {
    let Some(dir) = bundled else { return };
    if !config.developer
        || !config.root.as_deref().is_some_and(is_openrac_root)
        || on_the_copy(config.root.as_deref(), dir)
    {
        // The copy as this launcher names it: one an older launcher saved as `\\?\C:\...` is the
        // same folder.
        config.root = Some(dir.to_path_buf());
    }
    let on_copy = on_the_copy(config.root.as_deref(), dir);
    if on_copy && (!config.developer || !config.python.as_deref().is_some_and(Path::is_file)) {
        config.python = bundled_python(dir);
    }
}

/// Whether `root` is the carried copy `bundled`: the same folder whether or not either is written
/// with Windows' `\\?\` prefix. 0.7 saved the copy as `\\?\C:\...`, 0.8 names it `C:\...`: with
/// developer tools on (which keep the saved folder) the launcher took its own copy for a checkout,
/// and the Mods page looked for a checkout's assets/ in it ("The system cannot find the path
/// specified").
pub fn on_the_copy(root: Option<&Path>, bundled: &Path) -> bool {
    root.is_some_and(|root| plain(root) == plain(bundled))
}

/// `path` without Windows' `\\?\` prefix, when it is a plain drive path (`\\?\C:\Users\...`): the
/// prefix canonicalize and Tauri's resource folder add turns off `/` as a separator and `..`, and
/// a program started in such a folder (cmd, a tool's own subprocess) can refuse it.
pub fn plain(path: &Path) -> PathBuf {
    let text = path.to_string_lossy();
    match text.strip_prefix(r"\\?\") {
        Some(rest) if rest.as_bytes().get(1) == Some(&b':') && rest.as_bytes()[0].is_ascii_alphabetic() => {
            PathBuf::from(rest)
        }
        _ => path.to_path_buf(),
    }
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
        let path = std::fs::canonicalize(&path).map(|p| plain(&p)).unwrap_or(path);
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
                // Godot for Windows is a zip with no installer: the player unpacks it wherever
                // they like (Downloads, the desktop, Program Files), or winget, Scoop, Steam or
                // itch put it in folders of their own. Each is looked through for its programs.
                let local = env("LOCALAPPDATA");
                if let Some(local) = &local {
                    places.push(local.join("Microsoft").join("WinGet").join("Links").join("godot.exe"));
                }
                let mut bases: Vec<PathBuf> = Vec::new();
                if let Some(home) = home() {
                    bases.extend(["", "Downloads", "Desktop", "Documents", "Games", "Tools"].map(|d| home.join(d)));
                    bases.push(home.join("scoop").join("apps"));
                }
                bases.extend(
                    [
                        env("ProgramFiles"),
                        env("ProgramFiles(x86)"),
                        env("SystemDrive").map(|d| PathBuf::from(format!(r"{}\", d.display()))),
                    ]
                    .into_iter()
                    .flatten(),
                );
                if let Some(local) = &local {
                    bases.push(local.join("Programs"));
                    bases.push(local.clone());
                    bases.push(local.join("Microsoft").join("WinGet").join("Packages"));
                }
                bases.extend(env("APPDATA").map(|d| d.join("itch").join("apps")));
                bases.extend(env("ProgramData").map(|d| d.join("chocolatey").join("lib")));
                bases.extend(steam_libraries().into_iter().map(|l| l.join("steamapps").join("common")));
                for base in bases {
                    places.extend(godot_programs_in(&base, false, 3));
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

/// Godot's Windows programs in `dir`: `Godot_v4.7.2-stable_win64.exe`, Steam's
/// `godot.windows.opt.tools.64.exe` and the like, the window program before the `_console` one
/// beside it. Only folders named for Godot are gone into (`Godot_v4.7.2-stable_win64.exe\` is
/// what Explorer's Extract All makes of the zip, `GodotEngine.GodotEngine_...` winget's,
/// `Godot Engine` Steam's), and anything below one, `depth` levels down.
fn godot_programs_in(dir: &Path, inside: bool, depth: u32) -> Vec<PathBuf> {
    let Ok(entries) = std::fs::read_dir(dir) else { return Vec::new() };
    let mut programs = Vec::new();
    let mut folders = Vec::new();
    for entry in entries.filter_map(Result::ok) {
        let name = entry.file_name().to_string_lossy().to_ascii_lowercase();
        let godot = name.starts_with("godot");
        let Ok(kind) = entry.file_type() else { continue };
        if kind.is_dir() {
            if depth > 0 && (godot || inside) {
                folders.push(entry.path());
            }
        } else if godot && name.ends_with(".exe") {
            programs.push((name.contains("_console"), entry.path()));
        }
    }
    programs.sort();
    let mut found: Vec<PathBuf> = programs.into_iter().map(|(_, p)| p).collect();
    folders.sort();
    for folder in folders {
        found.extend(godot_programs_in(&folder, true, depth - 1));
    }
    found
}

/// Steam's library folders: its own and those `libraryfolders.vdf` names.
#[cfg(windows)]
fn steam_libraries() -> Vec<PathBuf> {
    let Some(steam) = std::env::var_os("ProgramFiles(x86)").map(|d| PathBuf::from(d).join("Steam")) else {
        return Vec::new();
    };
    let mut libraries = vec![steam.clone()];
    let vdf = std::fs::read_to_string(steam.join("steamapps").join("libraryfolders.vdf")).unwrap_or_default();
    for line in vdf.lines() {
        // 		"path"		"D:\\SteamLibrary"
        let mut quoted = line.split('"').skip(1).step_by(2);
        if let (Some("path"), Some(path)) = (quoted.next(), quoted.next()) {
            let path = PathBuf::from(path.replace("\\\\", "\\"));
            if !libraries.contains(&path) {
                libraries.push(path);
            }
        }
    }
    libraries
}

#[cfg(not(windows))]
fn steam_libraries() -> Vec<PathBuf> {
    Vec::new()
}

/// Why the Godot that answered `--version` with `line` (`4.7.2.stable.official.ed1daf0bf`) is
/// not one the level editor opens in, if it is not: it needs Godot 4.
pub fn godot_too_old(line: &str) -> Option<String> {
    let major: u32 = line.trim().split('.').next()?.parse().ok()?;
    (major < 4).then(|| format!("Godot {} is too old: the level editor needs Godot 4", line.trim()))
}

/// The version numbers at the start of what a tool printed, to put the newest first.
fn version_numbers(line: &str) -> Vec<u32> {
    let line = line.trim().trim_start_matches(|c: char| !c.is_ascii_digit());
    line.split('.').map_while(|part| part.parse().ok()).collect()
}

/// How long a version probe may take. Windows Defender reads a program through the first time it
/// runs: Godot's 180 MB took 13 s to answer on a small machine, and then a tenth of a second.
fn probe_timeout(tool: Tool) -> Duration {
    match tool {
        Tool::Godot => Duration::from_secs(90),
        Tool::Python | Tool::Docker => Duration::from_secs(30),
    }
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
    // goes before one that is too old. The order is otherwise kept. Of the
    // Godots that work, the newest goes first.
    if matches!(tool, Tool::Python | Tool::Godot) {
        let checked = found.into_iter().map(|c| {
            let check = check_tool(tool, &c.path);
            (c, check)
        });
        let (mut good, mut bad): (Vec<_>, Vec<_>) = checked.partition(|(_, k)| k.ok);
        // A fresh Windows has `python.exe` in WindowsApps that only says "Python was not found"
        // and points at the Store: that one is no Python to offer. A Store Python that is
        // installed answers from the same folder, and stays.
        bad.retain(|(c, _)| !c.path.components().any(|p| p.as_os_str().eq_ignore_ascii_case("WindowsApps")));
        if tool == Tool::Godot {
            good.sort_by_key(|(_, k)| std::cmp::Reverse(version_numbers(k.version.as_deref().unwrap_or(""))));
        }
        found = good.into_iter().chain(bad).map(|(c, _)| c).collect();
    }
    found
}

/// The flag that makes each tool print its version, and what a good answer contains.
fn version_probe(tool: Tool) -> (&'static [&'static str], &'static str) {
    match tool {
        Tool::Python => (&["--version"], "Python 3."),
        // `4.7.2.stable.official.ed1daf0bf` (or `.beta1.`, `.dev.`); godot_too_old checks the number.
        Tool::Godot => (&["--version"], "."),
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
    if tool == Tool::Python {
        if let Ok(appdir) = std::env::var("APPDIR") {
            if std::env::var("PYTHONHOME").is_ok_and(|h| h.starts_with(&appdir)) {
                cmd.env_remove("PYTHONHOME");
            }
            if std::env::var("PYTHONPATH").is_ok_and(|p| p.starts_with(&appdir)) {
                cmd.env_remove("PYTHONPATH");
            }
        }
    }
    match run(cmd, probe_timeout(tool)) {
        Ok(out) => {
            let line = out.lines().map(str::trim).find(|l| !l.is_empty()).unwrap_or("").to_string();
            let too_old = match tool {
                Tool::Python => python_too_old(&line),
                Tool::Godot => godot_too_old(&line),
                Tool::Docker => None,
            };
            let numbered = tool != Tool::Godot || line.starts_with(|c: char| c.is_ascii_digit());
            if let Some(why) = too_old {
                Check { ok: false, message: why, version: Some(line) }
            } else if numbered && line.contains(expect) {
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

/// The user's home directory.
pub fn home_dir() -> Option<PathBuf> {
    #[cfg(windows)]
    {
        std::env::var_os("USERPROFILE").map(PathBuf::from).or_else(|| {
            let drive = std::env::var_os("HOMEDRIVE")?;
            let path = std::env::var_os("HOMEPATH")?;
            Some(PathBuf::from(drive).join(path))
        })
    }
    #[cfg(not(windows))]
    {
        std::env::var_os("HOME").map(PathBuf::from)
    }
}

/// The default OpenRAC folder in the user's home directory:
/// - Linux: `/home/<user>/OpenRAC`
/// - macOS: `/Users/<user>/OpenRAC`
/// - Windows: `C:\Users\<user>\OpenRAC` (via `%USERPROFILE%`)
pub fn default_openrac_dir() -> Option<PathBuf> {
    home_dir().map(|h| h.join("OpenRAC"))
}

/// Ensures the default OpenRAC folder exists in a given home directory.
/// Checks if it already exists; if not, creates it.
pub fn ensure_openrac_dir_in(home: &Path) -> Result<PathBuf, String> {
    let dir = home.join("OpenRAC");
    if !dir.exists() {
        std::fs::create_dir_all(&dir).map_err(|e| format!("cannot create {}: {e}", dir.display()))?;
    }
    Ok(dir)
}

/// Ensures the default OpenRAC folder exists in the user's home directory.
pub fn ensure_default_openrac_dir() -> Result<PathBuf, String> {
    let home = home_dir().ok_or_else(|| "cannot determine home directory".to_string())?;
    ensure_openrac_dir_in(&home)
}

fn home() -> Option<PathBuf> {
    home_dir()
}

#[cfg(windows)]
const CREATE_NO_WINDOW: u32 = 0x0800_0000;

/// A command for a console program that must not flash a console window on Windows, and that
/// gets the system's environment rather than an AppImage's ([`crate::appimage::sanitize`]): the
/// probes of Python and Godot, curl, git.
pub fn quiet(program: &Path) -> Command {
    #[allow(unused_mut)]
    let mut cmd = Command::new(program);
    #[cfg(windows)]
    {
        use std::os::windows::process::CommandExt;
        cmd.creation_flags(CREATE_NO_WINDOW);
    }
    crate::appimage::sanitize(&mut cmd);
    cmd
}

/// `program` as a Linux desktop entry's `Exec` runs it: in double quotes, a backslash before `"`,
/// `` ` ``, `$` and a backslash (each then doubled, as an entry's strings take it) and `%` as `%%`,
/// the Desktop Entry Specification's rules, so a path with spaces or a `%` in it works.
pub fn desktop_exec(program: &Path) -> String {
    let mut out = String::from("\"");
    for c in program.to_string_lossy().chars() {
        match c {
            '"' | '`' | '$' | '\\' => {
                out.push_str("\\\\");
                out.push(c);
            }
            '%' => out.push_str("%%"),
            _ => out.push(c),
        }
    }
    out.push('"');
    out
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
    // The output is read while the program runs, not after: Godot on Windows flushes its
    // output pipe as it exits, which waits for the pipe to be read, so it never ended.
    let drain = |stream: Option<Box<dyn std::io::Read + Send>>| {
        std::thread::spawn(move || {
            let mut bytes = Vec::new();
            if let Some(mut stream) = stream {
                let _ = stream.read_to_end(&mut bytes);
            }
            bytes
        })
    };
    let stdout = drain(child.stdout.take().map(|s| Box::new(s) as _));
    let stderr = drain(child.stderr.take().map(|s| Box::new(s) as _));
    let start = Instant::now();
    let status = loop {
        match child.try_wait() {
            Ok(Some(status)) => break status,
            Ok(None) if start.elapsed() < timeout => std::thread::sleep(Duration::from_millis(25)),
            _ => {
                let _ = child.kill();
                // The killed process is reaped: one that is not waited for stays a zombie
                // until the launcher exits, and a probe that times out is not rare.
                let _ = child.wait();
                return Err(format!("{program} did not finish within {} s", timeout.as_secs()));
            }
        }
    };
    let mut text = String::from_utf8_lossy(&stdout.join().unwrap_or_default()).into_owned();
    text.push_str(&String::from_utf8_lossy(&stderr.join().unwrap_or_default()));
    if status.success() {
        Ok(text)
    } else {
        Err(text.trim().to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    #[test]
    fn a_desktop_entry_starts_a_path_with_spaces_and_signs() {
        assert_eq!(
            desktop_exec(Path::new("/home/p/My Games/OpenRAC.AppImage")),
            r#""/home/p/My Games/OpenRAC.AppImage""#
        );
        assert_eq!(desktop_exec(Path::new("/a/100%/$x\"y")), r#""/a/100%%/\\$x\\"y""#);
    }

    /// Godot for Windows as players have it: Extract All's folder named for the zip in
    /// Downloads, winget's package folder, Steam's, a renamed copy loose in a folder. The window
    /// program goes before its `_console` twin; nothing outside a Godot folder is gone into.
    #[test]
    fn finds_godot_where_windows_players_unpack_it() {
        let tmp = tempfile::tempdir().unwrap();
        let base = tmp.path();
        let file = |rel: &str| {
            let path = base.join(rel);
            std::fs::create_dir_all(path.parent().unwrap()).unwrap();
            std::fs::write(&path, "").unwrap();
            path
        };
        let unzipped = file("Godot_v4.7.2-stable_win64.exe/Godot_v4.7.2-stable_win64.exe");
        let console = file("Godot_v4.7.2-stable_win64.exe/Godot_v4.7.2-stable_win64_console.exe");
        let winget =
            file("GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe/Godot_v4.6.3-stable_win64.exe");
        let steam = file("Godot Engine/godot.windows.opt.tools.64.exe");
        let loose = file("godot.exe");
        file("Some Game/godot.exe"); // another program's folder: not gone into
        file("Godot Engine/readme.txt");
        let found = godot_programs_in(base, false, 3);
        assert_eq!(found, vec![loose, steam, winget, unzipped, console]);
        assert_eq!(godot_programs_in(&base.join("missing"), false, 3), Vec::<PathBuf>::new());
    }

    #[test]
    fn the_copy_is_the_copy_with_or_without_the_prefix() {
        let copy = Path::new(r"C:\Users\me\AppData\Local\OpenRAC Launcher\openrac");
        assert!(on_the_copy(Some(Path::new(r"\\?\C:\Users\me\AppData\Local\OpenRAC Launcher\openrac")), copy));
        assert!(on_the_copy(Some(copy), copy));
        assert!(!on_the_copy(Some(Path::new(r"C:\Users\me\OpenRAC")), copy));
        assert!(!on_the_copy(None, copy));
    }

    #[test]
    fn drops_the_verbatim_prefix_of_a_drive_path_only() {
        assert_eq!(plain(Path::new(r"\\?\C:\Users\me\openrac")), PathBuf::from(r"C:\Users\me\openrac"));
        assert_eq!(plain(Path::new(r"\\?\UNC\server\share")), PathBuf::from(r"\\?\UNC\server\share"));
        assert_eq!(plain(Path::new("/home/me/OpenRAC")), PathBuf::from("/home/me/OpenRAC"));
    }

    #[test]
    fn takes_godot_4_or_newer_and_orders_by_version() {
        assert_eq!(godot_too_old("4.7.2.stable.official.ed1daf0bf"), None);
        assert_eq!(godot_too_old("4.8.beta1.official.0123abcd"), None);
        assert_eq!(godot_too_old("5.0.stable.official"), None);
        assert!(godot_too_old("3.6.3.stable.official.ab12").is_some_and(|w| w.contains("needs Godot 4")));
        // "4." in 3.4 is not Godot 4.
        assert!(godot_too_old("3.4.stable.official").is_some());
        assert!(version_numbers("4.7.2.stable") > version_numbers("4.7.stable"));
        assert!(version_numbers("4.10.stable") > version_numbers("4.9.1.stable"));
        assert_eq!(version_numbers("Python 3.12.4"), vec![3, 12, 4]);
    }

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

    /// A player plays from the carried copy even with a checkout set from before; a contributor's
    /// choice stands with developer tools on, and a folder that is gone gives way.
    #[test]
    fn a_player_plays_from_the_carried_copy() {
        let tmp = std::env::temp_dir().join(format!("openrac-settle-{}", std::process::id()));
        let copy = tmp.join("openrac");
        std::fs::create_dir_all(copy.join("tools")).unwrap();
        std::fs::create_dir_all(copy.join("games")).unwrap();
        std::fs::write(copy.join("tools").join("openrac.py"), "").unwrap();
        let python = crate::under(&copy, if cfg!(windows) { "python/python.exe" } else { "python/bin/python3" });
        std::fs::create_dir_all(python.parent().unwrap()).unwrap();
        std::fs::write(&python, "").unwrap();
        let checkout = checkout();

        let mut player = crate::config::Config {
            root: Some(checkout.clone()),
            python: Some("/usr/bin/python3".into()),
            ..crate::config::Config::default()
        };
        settle(&mut player, Some(&copy));
        assert_eq!((player.root.as_deref(), player.python.as_deref()), (Some(copy.as_path()), Some(python.as_path())));

        let mut contributor = crate::config::Config {
            root: Some(checkout.clone()),
            python: Some("/usr/bin/python3".into()),
            developer: true,
            ..crate::config::Config::default()
        };
        settle(&mut contributor, Some(&copy));
        assert_eq!(contributor.root.as_deref(), Some(checkout.as_path()));
        assert_eq!(contributor.python.as_deref(), Some(Path::new("/usr/bin/python3")));

        let mut gone = crate::config::Config { root: Some(tmp.join("nothing")), developer: true, ..Default::default() };
        settle(&mut gone, Some(&copy));
        assert_eq!((gone.root.as_deref(), gone.python.as_deref()), (Some(copy.as_path()), Some(python.as_path())));

        let mut no_copy = player.clone();
        no_copy.root = Some(checkout.clone());
        settle(&mut no_copy, None);
        assert_eq!(no_copy.root.as_deref(), Some(checkout.as_path()), "without a copy nothing changes");
        std::fs::remove_dir_all(&tmp).unwrap();
    }

    #[test]
    fn finds_the_checkout_above_the_launcher() {
        let start = checkout().join("launcher").join("core");
        let roots = roots(&[start]);
        let want = plain(&std::fs::canonicalize(checkout()).unwrap());
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
    fn checks_godot_when_it_is_installed() {
        let Some(godot) = tool_candidates(Tool::Godot).into_iter().next() else { return };
        let check = check_tool(Tool::Godot, &godot.path);
        assert!(check.ok, "{godot:?}: {check:?}");
    }

    #[test]
    fn refuses_a_python_older_than_the_tools_need() {
        assert!(python_too_old("Python 3.9.6").is_some());
        assert!(python_too_old("Python 3.10.0").is_none());
        assert!(python_too_old("Python 3.14.0rc2").is_none());
        // Not a Python 3 at all: the caller's other check says so.
        assert!(python_too_old("Python 2.7.18").is_none());
    }

    #[test]
    fn creates_openrac_folder_in_home_if_missing_and_preserves_existing() {
        let temp = tempfile::tempdir().unwrap();
        let home = temp.path();
        let expected = home.join("OpenRAC");
        assert!(!expected.exists());

        // First call creates the folder
        let created = ensure_openrac_dir_in(home).unwrap();
        assert_eq!(created, expected);
        assert!(expected.is_dir());

        // Put a file inside to ensure it is not wiped/overwritten
        std::fs::write(expected.join("marker.txt"), "hello").unwrap();

        // Second call detects it exists and leaves it intact
        let existing = ensure_openrac_dir_in(home).unwrap();
        assert_eq!(existing, expected);
        assert!(expected.join("marker.txt").is_file());
    }
}
