//! What the launcher can run, from `launcher/actions.json`.
//!
//! The UI only ever names an action (`rac1/pal` + `build`); this module turns
//! that name into a program and its arguments, from the file in the user's
//! own checkout. Arguments are passed straight to the program, never through
//! a shell, and only the placeholders listed in [`PLACEHOLDERS`] are filled.
//! The file's format, and how to connect a game, are in docs/INTEGRATION.md.

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::catalog::{Catalog, Version};
use crate::config::Config;
use crate::status::{DiscState, VersionStatus};
use crate::under;

/// Where actions.json lives in a checkout.
pub const FILE: &str = "launcher/actions.json";

/// The only `{name}` placeholders an argument, `program` or `cwd` may hold.
pub const PLACEHOLDERS: &[&str] =
    &["root", "dir", "python", "godot", "docker", "disc", "boot", "artifact", "serial", "key", "data"];

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct ActionFile {
    /// The format's version; this launcher reads 1.
    pub format: u32,
    /// Actions for the checkout as a whole (identify discs, place inputs, progress).
    #[serde(default)]
    pub repository: Vec<Action>,
    /// Actions per version, by key (`rac1/pal`).
    #[serde(default)]
    pub versions: BTreeMap<String, Vec<Action>>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Action {
    /// Unique within its scope; what the UI sends back (`build`).
    pub id: String,
    /// The button's text (`Build`).
    pub label: String,
    /// One or two sentences under the button.
    #[serde(default)]
    pub description: String,
    pub kind: Kind,
    /// How far the action is wired up; see [`State`].
    #[serde(default)]
    pub state: State,
    /// The program: a placeholder for a configured tool (`{python}`,
    /// `{godot}`, `{docker}`) or a command found on PATH (`bash`, `make`).
    #[serde(default)]
    pub program: Option<String>,
    #[serde(default)]
    pub args: Vec<String>,
    /// The working folder, relative to the checkout; the version's own folder
    /// (`{dir}`) when left out for a version action, the checkout for a repository one.
    #[serde(default)]
    pub cwd: Option<String>,
    /// Where the action works; empty means everywhere.
    #[serde(default)]
    pub platforms: Vec<Platform>,
    /// What must be in place before it can run.
    #[serde(default)]
    pub requires: Vec<Requirement>,
    /// A file the action produces (a build) or uses (play a build),
    /// relative to the checkout. `{artifact}` in the arguments.
    #[serde(default)]
    pub artifact: Option<String>,
    /// Start it and let it run on its own (the game, Godot), instead of
    /// as a job in Tasks whose output the launcher shows.
    #[serde(default)]
    pub detached: bool,
    /// The document that explains the step, relative to the checkout.
    #[serde(default)]
    pub docs: Option<String>,
    /// For contributors connecting the action: what is left to do or check.
    /// Not shown to players.
    #[serde(default)]
    pub todo: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum Kind {
    /// Getting the inputs in place: discs, extracted files, toolchains.
    Setup,
    /// Building the game from the decompiled source.
    Build,
    /// Proving the build: the match checks, progress.
    Check,
    /// Playing: the native port.
    Play,
    /// The level editor.
    Edit,
}

/// How far an action is wired up. The launcher shows all three; it runs
/// `connected` and `unverified` ones, the latter with a warning.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum State {
    /// Run from the launcher end to end, on the platforms listed.
    Connected,
    /// Written from the game's docs, not yet run from the launcher.
    Unverified,
    /// Wanted, but no command yet: shown greyed out with its `todo`.
    #[default]
    Planned,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum Platform {
    Linux,
    Macos,
    Windows,
}

impl Platform {
    pub fn current() -> Platform {
        if cfg!(windows) {
            Platform::Windows
        } else if cfg!(target_os = "macos") {
            Platform::Macos
        } else {
            Platform::Linux
        }
    }

    fn name(self) -> &'static str {
        match self {
            Platform::Linux => "Linux",
            Platform::Macos => "macOS",
            Platform::Windows => "Windows",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum Requirement {
    /// The version's disc image is found (status.rs).
    Disc,
    /// Every input `openrac.py setup` places for the version is in place.
    Inputs,
    /// The shared toolchains/ folder holds a compiler.
    Toolchains,
    Python,
    Godot,
    Docker,
    /// `artifact` exists (a build to play).
    Artifact,
}

/// Where an action belongs: the checkout, or one version.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase", tag = "kind", content = "key")]
pub enum Scope {
    Repository,
    Version(String),
}

/// What the UI shows for an action: the action, and whether it can run now.
#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ActionView {
    pub id: String,
    pub label: String,
    pub description: String,
    pub kind: Kind,
    pub state: State,
    pub detached: bool,
    pub docs: Option<String>,
    pub todo: Option<String>,
    /// It runs on this platform at all; the UI tucks the others away.
    pub this_platform: bool,
    /// It can run now.
    pub runnable: bool,
    /// Why not, in words, one per missing thing.
    pub blockers: Vec<String>,
}

/// An action turned into a command line.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Plan {
    pub title: String,
    pub program: PathBuf,
    pub args: Vec<String>,
    pub cwd: PathBuf,
    pub detached: bool,
    /// Environment variables set for it besides the usual ones.
    pub env: Vec<(String, String)>,
}

/// Everything an action's placeholders and requirements are filled from.
pub struct Context<'a> {
    pub root: &'a Path,
    pub config: &'a Config,
    pub version: Option<&'a Version>,
    pub status: Option<&'a VersionStatus>,
    pub toolchains: bool,
    pub platform: Platform,
}

pub fn load(root: &Path) -> Result<ActionFile, String> {
    let path = under(root, FILE);
    let text = std::fs::read_to_string(&path).map_err(|e| format!("cannot read {}: {e}", path.display()))?;
    parse(&text).map_err(|e| format!("{}: {e}", path.display()))
}

pub fn parse(text: &str) -> Result<ActionFile, String> {
    let file: ActionFile = serde_json::from_str(text).map_err(|e| e.to_string())?;
    if file.format != 1 {
        return Err(format!("format {} is newer than this launcher reads (1): update the launcher", file.format));
    }
    Ok(file)
}

impl ActionFile {
    pub fn find(&self, scope: &Scope, id: &str) -> Option<&Action> {
        let list = match scope {
            Scope::Repository => &self.repository,
            Scope::Version(key) => self.versions.get(key)?,
        };
        list.iter().find(|a| a.id == id)
    }

    /// Every mistake in the file, checked against the checkout's catalogue:
    /// unknown versions, duplicate ids, unknown placeholders, runnable actions
    /// without a program. Run by the tests, so CI catches them.
    pub fn problems(&self, catalog: &Catalog) -> Vec<String> {
        let mut problems = Vec::new();
        let keys = catalog.version_keys();
        let mut scopes: Vec<(String, &Vec<Action>)> = vec![("repository".into(), &self.repository)];
        for (key, list) in &self.versions {
            if !keys.contains(key) {
                problems.push(format!("{key}: no such version in games/*/game.json"));
            }
            scopes.push((key.clone(), list));
        }
        for (scope, list) in scopes {
            let mut ids: Vec<&str> = Vec::new();
            for action in list {
                let at = format!("{scope}: {}", action.id);
                if ids.contains(&action.id.as_str()) {
                    problems.push(format!("{at}: duplicate id"));
                }
                ids.push(&action.id);
                if action.state != State::Planned && action.program.is_none() {
                    problems.push(format!("{at}: {:?} but no program", action.state));
                }
                if action.state == State::Planned && action.todo.is_none() {
                    problems.push(format!("{at}: planned, so it needs a todo saying what is missing"));
                }
                let texts = action.program.iter().chain(&action.args).chain(&action.cwd);
                for text in texts {
                    for name in placeholders(text) {
                        if !PLACEHOLDERS.contains(&name.as_str()) {
                            problems.push(format!("{at}: unknown placeholder {{{name}}} in {text:?}"));
                        }
                        if scope == "repository" && VERSION_ONLY.contains(&name.as_str()) {
                            problems.push(format!("{at}: {{{name}}} needs a version"));
                        }
                    }
                }
                if action.requires.contains(&Requirement::Artifact) && action.artifact.is_none() {
                    problems.push(format!("{at}: requires an artifact but names none"));
                }
            }
        }
        problems
    }
}

/// Placeholders that only mean something for a version.
const VERSION_ONLY: &[&str] = &["dir", "disc", "boot", "serial", "key", "data"];

/// The `{name}`s in `text`.
fn placeholders(text: &str) -> Vec<String> {
    let mut names = Vec::new();
    let mut rest = text;
    while let Some(start) = rest.find('{') {
        let Some(len) = rest[start + 1..].find('}') else { break };
        names.push(rest[start + 1..start + 1 + len].to_string());
        rest = &rest[start + 1 + len + 1..];
    }
    names
}

impl Action {
    /// Whether the action can run in `ctx`, and if not, why.
    pub fn view(&self, ctx: &Context) -> ActionView {
        let blockers = self.blockers(ctx);
        ActionView {
            id: self.id.clone(),
            label: self.label.clone(),
            description: self.description.clone(),
            kind: self.kind,
            state: self.state,
            detached: self.detached,
            docs: self.docs.clone(),
            todo: self.todo.clone(),
            this_platform: self.platforms.is_empty() || self.platforms.contains(&ctx.platform),
            runnable: blockers.is_empty(),
            blockers,
        }
    }

    fn blockers(&self, ctx: &Context) -> Vec<String> {
        let mut out = Vec::new();
        if self.state == State::Planned {
            out.push("not connected to the launcher yet".to_string());
            return out;
        }
        if !self.platforms.is_empty() && !self.platforms.contains(&ctx.platform) {
            let names: Vec<_> = self.platforms.iter().map(|p| p.name()).collect();
            out.push(format!("runs on {} only", names.join(" and ")));
        }
        for need in &self.requires {
            let missing =
                match need {
                    Requirement::Disc => (ctx.status.map(|s| s.disc.state) != Some(DiscState::Found))
                        .then_some("your disc image (baserom/)"),
                    Requirement::Inputs => (!ctx.status.is_some_and(|s| s.inputs_ready))
                        .then_some("the inputs Place inputs puts in the game"),
                    Requirement::Toolchains => (!ctx.toolchains).then_some("the compilers in toolchains/"),
                    Requirement::Python => ctx.config.python.is_none().then_some("Python (Settings)"),
                    Requirement::Godot => ctx.config.godot.is_none().then_some("Godot (Settings)"),
                    Requirement::Docker => {
                        (ctx.config.docker.is_none() && crate::detect::which("docker").is_none()).then_some("Docker")
                    }
                    Requirement::Artifact => {
                        if !self.artifact_path(ctx).is_ok_and(|p| p.exists()) {
                            let what = self.artifact.as_deref().unwrap_or("its artifact");
                            out.push(format!("needs {what}, which an earlier step makes"));
                        }
                        None
                    }
                };
            if let Some(what) = missing {
                out.push(format!("needs {what}"));
            }
        }
        out
    }

    /// The command line for this action, or why it cannot run.
    pub fn plan(&self, ctx: &Context) -> Result<Plan, String> {
        let blockers = self.blockers(ctx);
        if !blockers.is_empty() {
            return Err(format!("{}: {}", self.label, blockers.join("; ")));
        }
        let program = self.program.as_deref().ok_or_else(|| format!("{} has no program", self.label))?;
        let fill = |text: &str| fill(text, ctx, self.artifact_path(ctx).ok().as_deref());
        let program = PathBuf::from(fill(program)?);
        let args = self.args.iter().map(|a| fill(a)).collect::<Result<Vec<_>, _>>()?;
        let cwd = match (&self.cwd, ctx.version) {
            (Some(cwd), _) => abs(ctx.root, &fill(cwd)?),
            (None, Some(version)) => under(ctx.root, &version.dir),
            (None, None) => ctx.root.to_path_buf(),
        };
        let title = match ctx.version {
            Some(v) => format!("{} · {} ({})", self.label, v.title, v.region),
            None => self.label.clone(),
        };
        // Developer tools on: the native port offers its developer features (a level select in the
        // front end's Options).
        let mut env = Vec::new();
        if self.kind == Kind::Play && ctx.config.developer {
            env.push(("OPENRAC_DEVELOPER".to_string(), "1".to_string()));
        }
        Ok(Plan { title, program, args, cwd, detached: self.detached, env })
    }
}

impl Action {
    /// `artifact`, filled in and absolute.
    fn artifact_path(&self, ctx: &Context) -> Result<PathBuf, String> {
        let artifact = self.artifact.as_deref().ok_or_else(|| format!("{} names no artifact", self.label))?;
        let filled = fill(artifact, ctx, None)?;
        let full_path = abs(ctx.root, &filled);
        if !cfg!(windows) && filled.ends_with(".exe") {
            let without_exe = abs(ctx.root, &filled[..filled.len() - 4]);
            if without_exe.exists() || !full_path.exists() {
                return Ok(without_exe);
            }
        }
        Ok(full_path)
    }
}

/// A path that may be relative to the checkout, absolute.
fn abs(root: &Path, path: &str) -> PathBuf {
    let p = Path::new(path);
    if p.is_absolute() {
        p.to_path_buf()
    } else {
        under(root, path)
    }
}

/// `text` with its placeholders filled in from `ctx` (and `{artifact}` from `artifact`).
fn fill(text: &str, ctx: &Context, artifact: Option<&Path>) -> Result<String, String> {
    let mut out = text.to_string();
    for name in placeholders(text) {
        let value = match (name.as_str(), artifact) {
            ("artifact", Some(path)) => path.display().to_string(),
            ("artifact", None) => return Err("{artifact} needs the action's artifact".into()),
            _ => value(&name, ctx)?,
        };
        out = out.replace(&format!("{{{name}}}"), &value);
    }
    Ok(out)
}

fn value(name: &str, ctx: &Context) -> Result<String, String> {
    let path = |p: &Option<PathBuf>, what: &str| {
        p.as_ref().map(|p| p.display().to_string()).ok_or_else(|| format!("{what} is not set (Settings)"))
    };
    let version = || ctx.version.ok_or_else(|| format!("{{{name}}} needs a version"));
    Ok(match name {
        "root" => ctx.root.display().to_string(),
        "python" => path(&ctx.config.python, "Python")?,
        "godot" => path(&ctx.config.godot, "Godot")?,
        "docker" => match &ctx.config.docker {
            Some(p) => p.display().to_string(),
            None => "docker".to_string(),
        },
        "dir" => under(ctx.root, &version()?.dir).display().to_string(),
        "serial" => version()?.serial.clone(),
        "key" => version()?.key.clone(),
        "disc" => {
            let status = ctx.status.ok_or("no disc status")?;
            status.disc.path.as_ref().map(|p| p.display().to_string()).ok_or("the disc image was not found")?
        }
        "boot" => {
            let boot = ctx.status.and_then(|s| s.boot.as_ref()).ok_or("this version places no boot executable")?;
            under(ctx.root, &boot.path).display().to_string()
        }
        // The folder setting the game up from the disc wrote (install.rs), which the port reads.
        "data" => {
            let install = ctx.config.install_dir.as_ref().ok_or("the install folder is not set (Settings)")?;
            crate::install::data_dir(install, &version()?.game).display().to_string()
        }
        other => return Err(format!("unknown placeholder {{{other}}}")),
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    #[test]
    fn the_checkouts_actions_json_is_valid() {
        let root = checkout();
        let catalog = crate::catalog::load(&root).unwrap();
        let file = load(&root).expect("launcher/actions.json parses");
        assert_eq!(file.problems(&catalog), Vec::<String>::new());
        // Every version has an entry, even if all its actions are planned.
        for key in catalog.version_keys() {
            assert!(file.versions.contains_key(&key), "launcher/actions.json has no entry for {key}");
        }
        // Every document an action points to exists.
        let docs = file.repository.iter().chain(file.versions.values().flatten()).filter_map(|a| a.docs.as_ref());
        for doc in docs {
            let path = doc.split('#').next().unwrap();
            assert!(under(&root, path).exists(), "{doc} does not exist");
        }
    }

    #[test]
    fn rejects_mistakes() {
        let catalog = crate::catalog::load(&checkout()).unwrap();
        let file = parse(
            r#"{"format": 1,
                "repository": [{"id": "a", "label": "A", "kind": "setup", "state": "connected",
                                "program": "{python}", "args": ["{disc}"]}],
                "versions": {"rac9/pal": [],
                             "rac1/pal": [{"id": "b", "label": "B", "kind": "build", "state": "unverified"},
                                          {"id": "b", "label": "B", "kind": "build", "todo": "x",
                                           "args": ["{nope}"]}]}}"#,
        )
        .unwrap();
        let problems = file.problems(&catalog);
        let expect = [
            "repository: a: {disc} needs a version",
            "rac9/pal: no such version",
            "rac1/pal: b: Unverified but no program",
            "rac1/pal: b: duplicate id",
            "rac1/pal: b: unknown placeholder {nope}",
        ];
        for want in expect {
            assert!(problems.iter().any(|p| p.starts_with(want)), "missing {want:?} in {problems:?}");
        }
        assert!(parse(r#"{"format": 2}"#).is_err());
        assert!(parse(r#"{"format": 1, "extra": true}"#).is_err());
    }

    #[test]
    fn plans_fill_placeholders_and_report_blockers() {
        let root = checkout();
        let catalog = crate::catalog::load(&root).unwrap();
        let version = catalog.version("rac1/pal").unwrap();
        let status = crate::status::version_status(&root, version);
        let action: Action = serde_json::from_str(
            r#"{"id": "x", "label": "Extract", "kind": "edit", "state": "connected", "program": "{python}",
                "args": ["editor/extract.py", "{serial}", "{dir}/out"], "cwd": "{root}", "requires": ["python"]}"#,
        )
        .unwrap();

        let blocked = Config::default();
        let ready = Config { python: Some("/usr/bin/python3".into()), ..Config::default() };
        let ctx = |config| Context {
            root: &root,
            config,
            version: Some(version),
            status: Some(&status),
            toolchains: false,
            platform: Platform::Linux,
        };
        let view = action.view(&ctx(&blocked));
        assert!(!view.runnable && view.this_platform);
        assert_eq!(view.blockers, ["needs Python (Settings)"]);
        assert!(action.plan(&ctx(&blocked)).is_err());

        let plan = action.plan(&ctx(&ready)).unwrap();
        assert_eq!(plan.program, PathBuf::from("/usr/bin/python3"));
        assert_eq!(plan.args[1], "SCES_509.16");
        assert!(plan.args[2].ends_with("out") && plan.args[2].contains("rac1"), "{:?}", plan.args);
        assert_eq!(plan.cwd, root);
        assert!(plan.title.starts_with("Extract · Ratchet & Clank"), "{}", plan.title);

        let windows_only: Action = serde_json::from_str(
            r#"{"id": "w", "label": "W", "kind": "build", "state": "unverified", "program": "make",
                "platforms": ["windows"]}"#,
        )
        .unwrap();
        let view = windows_only.view(&ctx(&ready));
        assert!(!view.this_platform);
        assert_eq!(view.blockers, ["runs on Windows only"]);

        // {data}: the folder the game was set up in, which the port reads.
        let play: Action = serde_json::from_str(
            r#"{"id": "p", "label": "Play", "kind": "play", "state": "unverified", "program": "{python}",
                "args": ["--data", "{data}"]}"#,
        )
        .unwrap();
        assert!(play.plan(&ctx(&ready)).is_err(), "no install folder");
        let installed = Config { install_dir: Some("/games".into()), ..ready.clone() };
        let plan = play.plan(&ctx(&installed)).unwrap();
        let data = Path::new("/games").join("active").join("rac1").join("data");
        assert_eq!(plan.args[1], data.display().to_string());
    }
}
