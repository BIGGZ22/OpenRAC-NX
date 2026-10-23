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
pub const PLACEHOLDERS: &[&str] = &[
    "root", "dir", "python", "godot", "docker", "disc", "boot", "artifact", "serial", "key", "data", "level", "path",
    "option",
];

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
    /// Which of the game's parts it works on; the decompilation when left out.
    #[serde(default)]
    pub area: Area,
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
    /// A file the action leaves when it has done its work, when that is not its `artifact`
    /// (a check run with the game's own program leaves a stamp): what `done` looks for.
    #[serde(default)]
    pub makes: Option<String>,
    /// Files an earlier step makes and this one cannot run without, relative
    /// to the checkout: each is `{"path": ..., "from": the step's label}`.
    #[serde(default)]
    pub needs: Vec<Need>,
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
    /// The folder the action writes its results in (`assets/levels/rac1/pal`, `{data}/exports`),
    /// relative to the checkout: its job's result names it and the page can open it. The tool
    /// may name the exact folder it wrote on a line `openrac-output: <path>` (jobs.rs).
    #[serde(default)]
    pub output: Option<String>,
    /// The values `{option}` may take, one of which the UI chooses when it runs the action (what
    /// to export: `mobys`, `textures`...). An action with `{option}` lists them; anything else is
    /// refused.
    #[serde(default)]
    pub options: Vec<String>,
}

/// What an action works on. A game has two builds, and the launcher keeps them apart.
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum Area {
    /// The decompilation: the original program rebuilt and compared with the disc.
    #[default]
    Decomp,
    /// The PC port: the game built for this computer, and playing it.
    Port,
    /// The level editor.
    Editor,
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
    /// The Android launcher (docs/port/ANDROID.md): it plays from the copy it carries.
    Android,
}

impl Platform {
    pub fn current() -> Platform {
        if cfg!(windows) {
            Platform::Windows
        } else if cfg!(target_os = "macos") {
            Platform::Macos
        } else if cfg!(target_os = "android") {
            Platform::Android
        } else {
            Platform::Linux
        }
    }

    fn name(self) -> &'static str {
        match self {
            Platform::Linux => "Linux",
            Platform::Macos => "macOS",
            Platform::Windows => "Windows",
            Platform::Android => "Android",
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

/// A file an action needs and the step that makes it.
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Need {
    pub path: String,
    pub from: String,
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
    pub area: Area,
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
    /// Its `artifact` exists: for a step that makes one, it has been done.
    pub done: bool,
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
    /// The action's `output`, filled in and absolute.
    pub output: Option<PathBuf>,
}

/// Everything an action's placeholders and requirements are filled from.
pub struct Context<'a> {
    pub root: &'a Path,
    pub config: &'a Config,
    pub version: Option<&'a Version>,
    pub status: Option<&'a VersionStatus>,
    pub toolchains: bool,
    pub platform: Platform,
    /// The level an action is run for (`{level}`, two digits: the level editor's per-level
    /// projects). None while an action is only listed: `{level}` is then level 00, so that
    /// "is the project there" has an answer.
    pub level: Option<u32>,
    /// A file or folder the player chose for this run (`{path}`: a pack to add, an edited export
    /// to import), absolute. None while an action is only listed.
    pub path: Option<&'a str>,
    /// The choice `{option}` stands for, one of the action's `options`.
    pub option: Option<&'a str>,
}

/// What the UI gives a run besides the action's name: the level, a chosen path, an option.
#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct Inputs {
    pub level: Option<u32>,
    pub path: Option<String>,
    pub option: Option<String>,
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

    /// Whether action `id` of `scope` plays a game: a version's action of kind `play`. The
    /// launcher follows a play for Discord's presence and, on Android, starts it as an activity of
    /// its own instead of a program. A repository action never plays.
    pub fn is_play(&self, scope: &Scope, id: &str) -> bool {
        matches!(scope, Scope::Version(_)) && self.find(scope, id).is_some_and(|a| a.kind == Kind::Play)
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
                let needs = action.needs.iter().map(|n| &n.path);
                let texts =
                    action.program.iter().chain(&action.args).chain(&action.cwd).chain(needs).chain(&action.makes);
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
                let uses_option = action.args.iter().any(|a| placeholders(a).iter().any(|n| n == "option"));
                if uses_option && action.options.is_empty() {
                    problems.push(format!("{at}: uses {{option}} but lists no options"));
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
            area: self.area,
            state: self.state,
            detached: self.detached,
            docs: self.docs.clone(),
            todo: self.todo.clone(),
            this_platform: self.platforms.is_empty() || self.platforms.contains(&ctx.platform),
            runnable: blockers.is_empty(),
            blockers,
            done: match &self.makes {
                Some(made) => fill(made, ctx, None).is_ok_and(|p| abs(ctx.root, &p).exists()),
                None => self.artifact_path(ctx).is_ok_and(|p| p.exists()),
            },
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
                            // As written in actions.json, with its placeholders filled ({level}).
                            let named = self.artifact.as_deref().unwrap_or("its artifact");
                            let what = fill(named, ctx, None).unwrap_or_else(|_| named.to_string());
                            out.push(format!("needs {what}, which an earlier step makes"));
                        }
                        None
                    }
                };
            if let Some(what) = missing {
                out.push(format!("needs {what}"));
            }
        }
        for need in &self.needs {
            if !fill(&need.path, ctx, None).is_ok_and(|p| abs(ctx.root, &p).exists()) {
                out.push(format!("needs {}", need.from));
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
        if let Some(option) = ctx.option {
            if !self.options.iter().any(|o| o == option) {
                return Err(format!("{}: {option:?} is not one of {}", self.label, self.options.join(", ")));
            }
        }
        let fill = |text: &str| fill(text, ctx, self.artifact_path(ctx).ok().as_deref());
        let program = PathBuf::from(fill(program)?);
        let mut args = self.args.iter().map(|a| fill(a)).collect::<Result<Vec<_>, _>>()?;
        // Settings, "Purist mode": the game loads none of the native mods the Mods tab switched on
        // (docs/port/MODS.md). The packs of Play with mods are what that action is for, and stay.
        // Otherwise the game is told where the launcher keeps the list, in the game's set-up folder
        // in the per-user install folder, so that the port never guesses it: the same in a
        // checkout and in an installed launcher, whose own folder is read-only.
        if self.kind == Kind::Play {
            if ctx.config.purist {
                args.push("--no-mods".to_string());
            } else if let Ok(data) = fill("{data}") {
                args.push("--plugins".to_string());
                args.push(PathBuf::from(data).join("plugins").join(crate::plugins::LIST).display().to_string());
            }
        }
        let cwd = match (&self.cwd, ctx.version) {
            (Some(cwd), _) => abs(ctx.root, &fill(cwd)?),
            (None, Some(version)) => under(ctx.root, &version.dir),
            (None, None) => ctx.root.to_path_buf(),
        };
        // A folder that is not there cannot be started in (Windows: "the directory name is
        // invalid", error 267): the copy an installer carries has no folder for each version.
        let cwd = if cwd.is_dir() { cwd } else { ctx.root.to_path_buf() };
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
        // Settings, "Detailed game logs": the port's verbose log (port/common/log.h).
        if self.kind == Kind::Play && ctx.config.detailed_game_log {
            env.push(("OPENRAC_LOG".to_string(), "verbose".to_string()));
        }
        // Settings, "Renderer" (Windows, Linux and macOS, where it runs on the MoltenVK the game's
        // bundle carries): the port's Vulkan renderer, which draws with OpenGL instead where
        // Vulkan cannot start (port/platform/window.cpp). Android keeps its own choice
        // (src-tauri/src/android.rs).
        if self.kind == Kind::Play
            && ctx.config.renderer == "vulkan"
            && matches!(ctx.platform, Platform::Windows | Platform::Linux | Platform::Macos)
        {
            env.push(("OPENRAC_GL_API".to_string(), "vulkan".to_string()));
        }
        // The Mods tab's Enhanced graphics (enhanced.rs): with `enhanced_rendering = 1` in the
        // version's settings file the game draws with Vulkan, the one renderer that has it, and
        // takes the probe bakes made from the player's levels. Off, nothing is added. On Android
        // too: the game's activity is started with this environment (src-tauri/src/android.rs).
        if self.kind == Kind::Play {
            if let (Some(v), Ok(data)) = (ctx.version, fill("{data}")) {
                if crate::enhanced::supported(&v.game, &v.name) {
                    let file = crate::differences::settings_file(&crate::saves::data_home(), &v.game, &v.name);
                    let text = std::fs::read_to_string(file).unwrap_or_default();
                    if let Some((more_env, more_args)) =
                        crate::enhanced::play_additions(&text, Path::new(&data), &v.name)
                    {
                        for (key, value) in more_env {
                            if !env.iter().any(|(k, _)| *k == key) {
                                env.push((key, value));
                            }
                        }
                        args.extend(more_args);
                    }
                }
            }
        }
        let output = match &self.output {
            Some(output) => Some(abs(ctx.root, &fill(output)?)),
            None => None,
        };
        Ok(Plan { title, program, args, cwd, detached: self.detached, env, output })
    }
}

impl Action {
    /// The arguments with their placeholders filled in, and the artifact, whether or not the
    /// action can run now: what a page shows about an action's files before it runs.
    pub fn filled(&self, ctx: &Context) -> Result<(Vec<String>, Option<PathBuf>), String> {
        let artifact = self.artifact_path(ctx).ok();
        let args = self.args.iter().map(|a| fill(a, ctx, artifact.as_deref())).collect::<Result<Vec<_>, _>>()?;
        Ok((args, artifact))
    }

    /// `artifact`, filled in and absolute.
    fn artifact_path(&self, ctx: &Context) -> Result<PathBuf, String> {
        let artifact = self.artifact.as_deref().ok_or_else(|| format!("{} names no artifact", self.label))?;
        let filled = fill(artifact, ctx, None)?;
        let full_path = abs(ctx.root, &filled);
        if !cfg!(windows) && filled.ends_with(".exe") {
            // A program built here: without .exe, and on macOS in its application bundle.
            let without_exe = crate::program_path(abs(ctx.root, &filled[..filled.len() - 4]));
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
        "level" => format!("{:02}", ctx.level.unwrap_or(0)),
        // A chosen file or folder: absolute and there, so a relative one cannot reach into the
        // working folder.
        "path" => {
            let path = ctx.path.ok_or("choose a file or folder first")?;
            if !Path::new(path).is_absolute() || !Path::new(path).exists() {
                return Err(format!("{path} is not there"));
            }
            path.to_string()
        }
        "option" => ctx.option.ok_or("{option} needs a choice")?.to_string(),
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
    fn only_a_versions_play_action_plays() {
        let file = parse(
            r#"{"format": 1,
                "repository": [{"id": "play", "label": "P", "kind": "play"}],
                "versions": {"rac1/pal": [{"id": "play", "label": "Play", "kind": "play"},
                                          {"id": "build", "label": "Build", "kind": "build"}]}}"#,
        )
        .unwrap();
        let pal = Scope::Version("rac1/pal".into());
        assert!(file.is_play(&pal, "play"));
        assert!(!file.is_play(&pal, "build"));
        assert!(!file.is_play(&pal, "gone"));
        assert!(!file.is_play(&Scope::Version("rac9/pal".into()), "play"));
        assert!(!file.is_play(&Scope::Repository, "play"), "a repository action never plays");
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
        // Play reads the version's settings file (Enhanced graphics): an empty data folder, not
        // the settings of whoever runs the tests.
        let home = std::env::temp_dir().join(format!("openrac-plans-{}", std::process::id()));
        crate::saves::with_data_home(&home, plans_fill_placeholders_and_report_blockers_in_home);
    }

    fn plans_fill_placeholders_and_report_blockers_in_home() {
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
            level: None,
            path: None,
            option: None,
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
        assert!(!plan.env.iter().any(|(k, _)| k == "OPENRAC_LOG"));
        assert!(!plan.args.iter().any(|a| a == "--no-mods"), "mods load unless purist mode is on");
        // The list the launcher writes, named explicitly: <data>/plugins/plugins.json.
        let list = data.join("plugins").join("plugins.json").display().to_string();
        assert_eq!(plan.args[plan.args.len() - 2..], ["--plugins".to_string(), list], "{:?}", plan.args);
        // Settings, "Purist mode": no native mods.
        let purist = Config { purist: true, ..installed.clone() };
        let plan = play.plan(&ctx(&purist)).unwrap();
        assert_eq!(plan.args.last().map(String::as_str), Some("--no-mods"), "{:?}", plan.args);
        assert!(!plan.args.iter().any(|a| a == "--plugins"), "{:?}", plan.args);
        // Settings, "Detailed game logs": the game's verbose log.
        let detailed = Config { detailed_game_log: true, ..installed.clone() };
        let plan = play.plan(&ctx(&detailed)).unwrap();
        assert!(plan.env.contains(&("OPENRAC_LOG".to_string(), "verbose".to_string())), "{:?}", plan.env);
        // Settings, "Renderer": Vulkan on the desktops; OpenGL, the default, sets nothing.
        assert!(!plan.env.iter().any(|(k, _)| k == "OPENRAC_GL_API"));
        let vulkan = Config { renderer: "vulkan".into(), ..installed.clone() };
        let plan = play.plan(&ctx(&vulkan)).unwrap();
        assert!(plan.env.contains(&("OPENRAC_GL_API".to_string(), "vulkan".to_string())), "{:?}", plan.env);
        // macOS as well: the game's bundle carries MoltenVK.
        let on_mac = Context { platform: Platform::Macos, ..ctx(&vulkan) };
        let plan = play.plan(&on_mac).unwrap();
        assert!(plan.env.contains(&("OPENRAC_GL_API".to_string(), "vulkan".to_string())), "{:?}", plan.env);
    }

    /// `{path}` is a chosen file or folder that is there; `{option}` one of the action's options.
    #[test]
    fn path_and_option_are_checked() {
        let root = checkout();
        let catalog = crate::catalog::load(&root).unwrap();
        let version = catalog.version("rac1/pal").unwrap();
        let status = crate::status::version_status(&root, version);
        let config = Config { python: Some("/usr/bin/python3".into()), ..Config::default() };
        let action: Action = serde_json::from_str(
            r#"{"id": "x", "label": "Export", "kind": "edit", "state": "unverified", "program": "{python}",
                "args": ["{path}", "--what", "{option}"], "options": ["mobys", "textures"]}"#,
        )
        .unwrap();
        let here = root.canonicalize().unwrap().display().to_string();
        let ctx = |path, option| Context {
            root: &root,
            config: &config,
            version: Some(version),
            status: Some(&status),
            toolchains: false,
            platform: Platform::Linux,
            level: None,
            path,
            option,
        };
        let plan = action.plan(&ctx(Some(&here), Some("textures"))).unwrap();
        assert_eq!(plan.args, [here.as_str(), "--what", "textures"]);
        assert!(action.plan(&ctx(Some(&here), Some("--evil"))).is_err());
        assert!(action.plan(&ctx(None, Some("mobys"))).is_err());
        assert!(action.plan(&ctx(Some("relative/path"), Some("mobys"))).is_err());
        assert!(action.plan(&ctx(Some("/no/such/folder"), Some("mobys"))).is_err());
        let catalog = crate::catalog::load(&root).unwrap();
        let file = parse(
            r#"{"format": 1, "versions": {"rac1/pal": [{"id": "y", "label": "Y", "kind": "edit", "state": "unverified",
                "program": "{python}", "args": ["{option}"]}]}}"#,
        )
        .unwrap();
        assert!(file.problems(&catalog).iter().any(|p| p.contains("lists no options")));
    }

    /// An action starts in a folder that is there. 0.4.0 started the game in the version's own
    /// folder, which the copy an installer carries does not have: Windows refused it ("the
    /// directory name is invalid", error 267) and Play did nothing.
    #[test]
    fn an_action_never_starts_in_a_folder_that_is_missing() {
        let tmp = std::env::temp_dir().join(format!("openrac-cwd-{}", std::process::id()));
        std::fs::create_dir_all(&tmp).unwrap();
        let catalog = crate::catalog::load(&checkout()).unwrap();
        let version = catalog.version("rac1/pal").unwrap();
        let status = crate::status::version_status(&tmp, version);
        let config = Config { python: Some("/usr/bin/python3".into()), ..Config::default() };
        let ctx = Context {
            root: &tmp, // a root without games/rac1, as the carried copy is
            config: &config,
            version: Some(version),
            status: Some(&status),
            toolchains: false,
            platform: Platform::current(),
            level: None,
            path: None,
            option: None,
        };
        let play: Action = serde_json::from_str(
            r#"{"id": "p", "label": "Play", "kind": "play", "state": "unverified", "program": "{python}"}"#,
        )
        .unwrap();
        assert_eq!(play.plan(&ctx).unwrap().cwd, tmp);
        let elsewhere: Action = serde_json::from_str(
            r#"{"id": "q", "label": "Q", "kind": "play", "state": "unverified", "program": "{python}", "cwd": "no/such"}"#,
        )
        .unwrap();
        assert_eq!(elsewhere.plan(&ctx).unwrap().cwd, tmp);
        std::fs::remove_dir_all(&tmp).unwrap();
    }
}
