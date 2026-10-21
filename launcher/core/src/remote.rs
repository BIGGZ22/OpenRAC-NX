//! Mods from a git repository's address (docs/port/MODS.md, section 10): what the address holds,
//! shown before anything is installed; a native mod's built archive for this computer from the
//! repository's release, checked against its `SHA256SUMS`; a script mod's or a data pack's files (the
//! release's joined archive, checked, else the repository's own files: MODS.md, section 11); where each mod came
//! from (`<data>/plugins/sources.json`), and newer releases to update to.
//!
//! GitHub is read through its HTTPS API and raw files, with `curl` (no git, no compiler): the
//! player has neither. Another git host is read with `git`, when it is installed, for data packs
//! only: a native mod comes built, from a release. Everything goes through [`Fetch`], which the
//! tests answer from memory, so no test needs the network.

use std::collections::BTreeMap;
use std::io::Read;
use std::path::Path;

use serde::{Deserialize, Serialize};
use serde_json::Value;
use sha2::{Digest, Sha256};

use crate::plugins::{self, Installed, Manifest, MANIFEST};

/// What remembers each installed mod's source, in the plugins folder.
pub const SOURCES: &str = "sources.json";
/// A repository's list of its mods' folders (optional).
pub const INDEX: &str = "openrac-mods.json";
/// A release's checksums, in `sha256sum`'s format.
pub const SUMS: &str = "SHA256SUMS";
/// The largest file fetched (a mod's archive, a repository's).
pub const MAX_BYTES: u64 = 256 * 1024 * 1024;

// ---- Fetching -------------------------------------------------------------------------------

/// Gets a URL's bytes. `api`: a GitHub API request (its JSON media type).
pub trait Fetch {
    fn get(&self, url: &str, api: bool) -> Result<Vec<u8>, String>;
}

/// The real one: `curl`, as the launcher's progress fetch runs it (on Linux, macOS and Windows 10+),
/// following redirects (a release's asset, a repository's archive), with a time limit and a size cap.
pub struct Curl;

impl Fetch for Curl {
    fn get(&self, url: &str, api: bool) -> Result<Vec<u8>, String> {
        if !url.starts_with("https://") {
            return Err(format!("{url} is not an https:// address"));
        }
        let mut cmd = crate::detect::quiet(Path::new("curl"));
        cmd.args(["-sS", "-f", "-L", "--proto", "=https", "--connect-timeout", "15", "--max-time", "600"])
            .args(["--max-filesize", &MAX_BYTES.to_string()])
            .args(["-H", concat!("User-Agent: openrac-launcher/", env!("CARGO_PKG_VERSION"))]);
        if api {
            cmd.args(["-H", "Accept: application/vnd.github+json", "-H", "X-GitHub-Api-Version: 2022-11-28"]);
        }
        let out = cmd
            .arg(url)
            .stdin(std::process::Stdio::null())
            .output()
            .map_err(|e| format!("curl cannot be started ({e}): the launcher fetches mods with it"))?;
        if out.status.success() {
            return Ok(out.stdout);
        }
        let why = String::from_utf8_lossy(&out.stderr).trim().to_string();
        Err(if why.contains("404") {
            format!("{url}: not found")
        } else if why.is_empty() {
            format!("{url} could not be fetched")
        } else {
            format!("{url} could not be fetched ({why})")
        })
    }
}

/// Where GitHub answers: its API and its raw files (the tests point both elsewhere).
#[derive(Debug, Clone)]
pub struct Hosts {
    pub api: String,
    pub raw: String,
}

impl Default for Hosts {
    fn default() -> Self {
        Hosts { api: "https://api.github.com".into(), raw: "https://raw.githubusercontent.com".into() }
    }
}

// ---- Addresses ------------------------------------------------------------------------------

/// What a repository GitHub does not show is, to someone without a login: private, or not there.
pub const NOT_FOUND: &str =
    "This repository is private or doesn't exist. OpenRAC can only install from public repositories.";

/// A repository's address, as the player pasted it.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase", tag = "host")]
pub enum Source {
    /// `https://github.com/<owner>/<repo>`, maybe at a branch or commit (`/tree/<ref>`) or a
    /// release (`/releases/tag/<tag>`).
    #[serde(rename = "github")]
    GitHub { owner: String, repo: String, reference: Option<String>, tag: Option<String> },
    /// Any other git host: the address git clones.
    #[serde(rename = "git")]
    Git { url: String },
}

impl Source {
    /// The repository's address without a version: what sources.json keeps.
    pub fn url(&self) -> String {
        match self {
            Source::GitHub { owner, repo, .. } => format!("https://github.com/{owner}/{repo}"),
            Source::Git { url } => url.clone(),
        }
    }

    /// "owner/repo", or the address.
    pub fn label(&self) -> String {
        match self {
            Source::GitHub { owner, repo, .. } => format!("{owner}/{repo}"),
            Source::Git { url } => url.clone(),
        }
    }
}

fn github_name(s: &str) -> bool {
    !s.is_empty() && s != "." && s != ".." && s.chars().all(|c| c.is_ascii_alphanumeric() || "-_.".contains(c))
}

/// Reads an address: GitHub's forms, or another host's https/ssh git address.
pub fn parse_url(text: &str) -> Result<Source, String> {
    let text = text.trim();
    if text.is_empty() {
        return Err("Paste the address of a repository.".into());
    }
    let bare = text.strip_prefix("https://").or_else(|| text.strip_prefix("http://")).unwrap_or(text);
    let bare = bare.strip_prefix("www.").unwrap_or(bare);
    // git@github.com:<owner>/<repo>(.git), the address GitHub's "Code" button gives for SSH.
    let bare = match bare.strip_prefix("git@github.com:").or_else(|| bare.strip_prefix("ssh://git@github.com/")) {
        Some(path) => format!("github.com/{path}"),
        None => bare.to_string(),
    };
    // <owner>/<repo>: GitHub's own shorthand.
    let bare = match bare.split('/').collect::<Vec<_>>().as_slice() {
        [owner, repo] if !bare.contains(':') && !owner.contains('.') && github_name(owner) && github_name(repo) => {
            format!("github.com/{bare}")
        }
        _ => bare,
    };
    if let Some(path) = bare.strip_prefix("github.com/") {
        let path = path.split(['?', '#']).next().unwrap_or_default().trim_end_matches('/');
        let parts: Vec<&str> = path.split('/').collect();
        if parts.len() < 2 {
            return Err(format!("{text} is not a repository's address (github.com/<owner>/<repository>)"));
        }
        let owner = parts[0].to_string();
        let repo = parts[1].strip_suffix(".git").unwrap_or(parts[1]).to_string();
        if !github_name(&owner) || !github_name(&repo) {
            return Err(format!("{text} is not a repository's address (github.com/<owner>/<repository>)"));
        }
        let rest = &parts[2..];
        let (reference, tag) = match rest {
            [] => (None, None),
            ["tree", r @ ..] if !r.is_empty() => (Some(r.join("/")), None),
            ["releases", "tag", t @ ..] if !t.is_empty() => (None, Some(t.join("/"))),
            ["releases"] | ["releases", "latest"] => (None, None),
            _ => return Err(format!("{text}: open the repository's main page and copy its address")),
        };
        return Ok(Source::GitHub { owner, repo, reference, tag });
    }
    let git = text.starts_with("https://") || text.starts_with("ssh://") || text.starts_with("git@");
    if git && !text.contains(char::is_whitespace) {
        return Ok(Source::Git { url: text.trim_end_matches('/').to_string() });
    }
    Err(format!("{text} is not a repository's address: paste one that starts with https://"))
}

// ---- What a repository holds ----------------------------------------------------------------

#[derive(Debug, Clone, Deserialize)]
struct Asset {
    name: String,
    browser_download_url: String,
}

#[derive(Debug, Clone, Deserialize)]
struct Release {
    tag_name: String,
    #[serde(default)]
    draft: bool,
    #[serde(default)]
    prerelease: bool,
    #[serde(default)]
    assets: Vec<Asset>,
}

impl Release {
    fn asset(&self, name: &str) -> Option<&Asset> {
        self.assets.iter().find(|a| a.name == name)
    }
}

/// One mod the repository holds, as the Mods tab shows it before it is installed.
#[derive(Debug, Clone, Serialize, PartialEq)]
#[serde(rename_all = "camelCase")]
pub struct RemoteMod {
    pub id: String,
    pub name: String,
    pub version: String,
    pub author: String,
    pub description: String,
    pub homepage: String,
    /// Its folder in the repository, "" for the top.
    pub path: String,
    /// "native" (a library the game loads: code), "script" (Luau the game runs sandboxed) or "pack"
    /// (textures, models, levels).
    pub kind: String,
    pub games: BTreeMap<String, Vec<String>>,
    pub api: Option<String>,
    /// Whether this computer's game can take it; if not, why.
    pub compatible: bool,
    pub why: Option<String>,
    /// The release asset that would be installed, and the release's tag.
    pub asset: Option<String>,
    pub tag: Option<String>,
    /// The version installed now, if it is.
    pub installed: Option<String>,
}

/// What an address holds.
#[derive(Debug, Clone, Serialize, PartialEq)]
#[serde(rename_all = "camelCase")]
pub struct Preview {
    /// The repository's address, without a version.
    pub url: String,
    /// "owner/repo", or the address.
    pub label: String,
    /// "github" or "git".
    pub host: String,
    /// The owner, who publishes it (GitHub).
    pub owner: String,
    /// What the manifests were read at: a release's tag, a branch or a commit.
    pub reference: String,
    /// The commit read, where known (a data pack with no release).
    pub commit: Option<String>,
    pub mods: Vec<RemoteMod>,
    /// Said about the whole repository ("no release yet").
    pub note: Option<String>,
}

fn json<T: for<'a> Deserialize<'a>>(bytes: &[u8], what: &str) -> Result<T, String> {
    serde_json::from_slice(bytes).map_err(|e| format!("{what} cannot be read ({e})"))
}

/// The manifests' paths in a repository's file list: the index's folders, the top's manifest, or
/// one in each top folder.
pub fn discover(paths: &[String], index: Option<&Value>) -> Result<Vec<String>, String> {
    if let Some(index) = index {
        let folders =
            index.get("mods").and_then(Value::as_array).ok_or_else(|| format!("{INDEX} has no \"mods\" list"))?;
        let mut out = Vec::new();
        for f in folders {
            let f = f.as_str().unwrap_or_default().trim_matches('/');
            if f.is_empty() || !plugins::safe_relative(f) {
                return Err(format!("{INDEX} names {f:?}, which is not a folder of the repository"));
            }
            let path = format!("{f}/{MANIFEST}");
            if !paths.contains(&path) {
                return Err(format!("{INDEX} names {f}, which has no {MANIFEST}"));
            }
            out.push(path);
        }
        return Ok(out);
    }
    if paths.iter().any(|p| p == MANIFEST) {
        return Ok(vec![MANIFEST.to_string()]);
    }
    let mut out: Vec<String> = paths
        .iter()
        .filter(|p| p.split('/').count() == 2 && p.ends_with(&format!("/{MANIFEST}")) && !p.starts_with('.'))
        .cloned()
        .collect();
    out.sort();
    if out.is_empty() {
        return Err(format!(
            "No OpenRAC mod in this repository: it has no {MANIFEST} at its top or in a folder at its top."
        ));
    }
    Ok(out)
}

/// The folder a manifest's path is in, "" for the top.
fn folder_of(path: &str) -> String {
    path.rsplit_once('/').map(|(f, _)| f.to_string()).unwrap_or_default()
}

/// A release's asset for one mod on one platform: `<id>-<version>-<platform>.zip`.
pub fn asset_name(id: &str, version: &str, platform: &str) -> String {
    format!("{id}-{version}-{platform}.zip")
}

/// `SHA256SUMS`: file name → hash (lower case). A `*` before the name (binary mode) is allowed.
pub fn parse_sums(text: &str) -> BTreeMap<String, String> {
    let mut out = BTreeMap::new();
    for line in text.lines() {
        let line = line.trim();
        let Some((hash, name)) = line.split_once(char::is_whitespace) else { continue };
        let name = name.trim_start().trim_start_matches('*').trim();
        if hash.len() == 64 && hash.chars().all(|c| c.is_ascii_hexdigit()) && !name.is_empty() {
            out.insert(name.to_string(), hash.to_ascii_lowercase());
        }
    }
    out
}

/// Whether `bytes` are the file `name` that `sums` lists: why not.
pub fn verify(bytes: &[u8], name: &str, sums: Option<&str>) -> Result<(), String> {
    let sums =
        sums.ok_or_else(|| format!("The release has no {SUMS}: {name} cannot be checked, so it is not installed."))?;
    let want = parse_sums(sums).remove(name).ok_or_else(|| {
        format!("The release's {SUMS} does not list {name}: it cannot be checked, so it is not installed.")
    })?;
    let got = format!("{:x}", Sha256::digest(bytes));
    if got != want {
        return Err(format!(
            "{name} is not the file the release lists (its SHA-256 differs): it was damaged or changed, and is not installed."
        ));
    }
    Ok(())
}

/// The manifest in a mod's .zip (at its top or in its one folder), read without unpacking it.
fn manifest_in_zip(bytes: &[u8]) -> Result<Value, String> {
    let mut archive =
        zip::ZipArchive::new(std::io::Cursor::new(bytes)).map_err(|e| format!("the download is not a .zip ({e})"))?;
    let mut best: Option<String> = None;
    for i in 0..archive.len() {
        let name = archive.by_index(i).map_err(|e| e.to_string())?.name().to_string();
        let depth = name.split('/').count();
        if name.ends_with(MANIFEST)
            && (name == MANIFEST || depth == 2)
            && best.as_ref().is_none_or(|b| b.len() > name.len())
        {
            best = Some(name);
        }
    }
    let name = best.ok_or_else(|| format!("the download has no {MANIFEST}: it is not an OpenRAC mod"))?;
    let mut text = String::new();
    archive.by_name(&name).map_err(|e| e.to_string())?.read_to_string(&mut text).map_err(|e| e.to_string())?;
    serde_json::from_str(&text).map_err(|e| format!("its {MANIFEST} cannot be read ({e})"))
}

/// A manifest's text as a mod to show: what it is and whether this computer's game can take it.
#[allow(clippy::too_many_arguments)]
fn describe(
    raw: &Value,
    path: &str,
    repo: &str,
    game: &str,
    version: &str,
    platform: Option<&str>,
    installed: &BTreeMap<String, String>,
) -> Result<RemoteMod, String> {
    let text = |k: &str| raw.get(k).and_then(Value::as_str).unwrap_or_default().to_string();
    let format = raw.get("format").and_then(Value::as_u64).unwrap_or(0);
    let folder = folder_of(path);
    if format == 1 {
        // A data pack (editor/MODDING.md): named after its folder, or the repository.
        let id = if folder.is_empty() { repo.to_ascii_lowercase() } else { folder.clone() };
        let pack_game = text("game");
        let (compatible, why) = if pack_game == game {
            (true, None)
        } else {
            (false, Some(format!("It is a pack for {pack_game}, not this game.")))
        };
        let mut games = BTreeMap::new();
        games.insert(pack_game, vec![text("version")]);
        return Ok(RemoteMod {
            installed: installed.get(&id).cloned(),
            id,
            name: text("name"),
            version: text("pack_version"),
            author: text("author"),
            description: text("description"),
            homepage: String::new(),
            path: folder,
            kind: "pack".into(),
            games,
            api: None,
            compatible,
            why,
            asset: None,
            tag: None,
        });
    }
    let m: Manifest = serde_json::from_value(raw.clone()).map_err(|e| format!("{path} cannot be read ({e})"))?;
    let check = m.validate().map_err(|e| format!("Its {MANIFEST} breaks a rule: {e}.")).and_then(|()| {
        if m.plugin.is_empty() && !m.is_script() {
            m.games
                .get(game)
                .filter(|d| d.iter().any(|d| d == version))
                .map(|_| ())
                .ok_or_else(|| format!("Not for this game's disc ({game}, {version})."))
        } else {
            m.runs_on(game, version, platform)
        }
    });
    Ok(RemoteMod {
        installed: installed.get(&m.id).cloned(),
        id: m.id.clone(),
        name: m.name.clone(),
        version: m.version.clone(),
        author: m.author.clone(),
        description: m.description.clone(),
        homepage: m.homepage.clone(),
        path: folder,
        // A repository's manifest is a mod's source: a native mod's names no library yet (its
        // release's archive adds it), so format 2 that is not a script is native. A data pack
        // is format 1 (handled above).
        kind: if m.is_script() { "script".into() } else { "native".into() },
        games: m.games.clone(),
        api: m.api.clone(),
        compatible: check.is_ok(),
        why: check.err(),
        asset: None,
        tag: None,
    })
}

/// The installed mods' versions, by id (native mods' manifests, and sources.json for packs).
fn installed_versions(dir: &Path) -> BTreeMap<String, String> {
    let mut out: BTreeMap<String, String> = read_sources(dir).mods.into_iter().map(|(id, s)| (id, s.version)).collect();
    if let Ok(entries) = std::fs::read_dir(dir) {
        for e in entries.flatten() {
            if let Ok(m) = plugins::read_manifest(&e.path()) {
                out.insert(m.id.clone(), m.version.clone());
            }
        }
    }
    out
}

/// A GitHub repository read through its API: the releases, the file list and the manifests at a
/// version, and per mod the release whose asset is for this computer.
struct GitHubRead {
    preview: Preview,
    releases: Vec<Release>,
}

fn read_github(
    fetch: &dyn Fetch,
    hosts: &Hosts,
    owner: &str,
    repo: &str,
    reference: Option<&str>,
    tag: Option<&str>,
    ctx: (&str, &str, Option<&str>, &Path),
) -> Result<GitHubRead, String> {
    let (game, version, platform, dir) = ctx;
    let base = format!("{}/repos/{owner}/{repo}", hosts.api);
    let releases: Vec<Release> = json(
        &fetch.get(&format!("{base}/releases?per_page=100"), true).map_err(|e| {
            // GitHub answers a private repository and one that is not there alike (404) without a
            // login, so the message cannot tell them apart.
            if e.ends_with("not found") {
                NOT_FOUND.to_string()
            } else {
                e
            }
        })?,
        "the repository's releases",
    )?;
    let eligible: Vec<Release> = match tag {
        Some(t) => {
            let r = releases
                .iter()
                .find(|r| r.tag_name == t)
                .ok_or_else(|| format!("github.com/{owner}/{repo} has no release {t}"))?;
            vec![r.clone()]
        }
        None => releases.iter().filter(|r| !r.draft && !r.prerelease).cloned().collect(),
    };
    // What the manifests are read at: the release named, a branch or commit named, the newest
    // release, or the default branch.
    // A link to a folder, /tree/<branch>/<folder>: the branch is the shortest start of the path the
    // repository knows (a branch's name may hold slashes), the rest the folder whose mods are shown.
    let (at, commit, subdir) = match (tag, reference, eligible.first()) {
        (Some(t), _, _) => (t.to_string(), commit_of(fetch, &base, t)?, None),
        (None, Some(r), _) => {
            let parts: Vec<&str> = r.split('/').collect();
            let mut found = None;
            for n in 1..=parts.len() {
                let branch = parts[..n].join("/");
                match commit_of(fetch, &base, &branch) {
                    Ok(c) => {
                        let rest = parts[n..].join("/");
                        found = Some((branch, c, (!rest.is_empty()).then_some(rest)));
                        break;
                    }
                    Err(e) if n == parts.len() || !e.ends_with("not found") => return Err(e),
                    Err(_) => {}
                }
            }
            found.ok_or_else(|| format!("github.com/{owner}/{repo} has no branch {r}"))?
        }
        (None, None, Some(r)) => (r.tag_name.clone(), commit_of(fetch, &base, &r.tag_name)?, None),
        (None, None, None) => ("HEAD".to_string(), commit_of(fetch, &base, "HEAD")?, None),
    };
    let tree_at = commit.clone().unwrap_or_else(|| at.clone());
    let tree: Value =
        json(&fetch.get(&format!("{base}/git/trees/{tree_at}?recursive=1"), true)?, "the repository's files")?;
    let paths: Vec<String> = tree
        .get("tree")
        .and_then(Value::as_array)
        .map(|t| {
            t.iter()
                .filter(|e| e.get("type").and_then(Value::as_str) == Some("blob"))
                .filter_map(|e| e.get("path").and_then(Value::as_str).map(str::to_string))
                .collect()
        })
        .unwrap_or_default();
    let raw_at = |path: &str| format!("{}/{owner}/{repo}/{tree_at}/{path}", hosts.raw);
    let index = if paths.iter().any(|p| p == INDEX) {
        Some(json::<Value>(&fetch.get(&raw_at(INDEX), false)?, INDEX)?)
    } else {
        None
    };
    let installed = installed_versions(dir);
    let mut mods = Vec::new();
    // A folder named in the link: only the mods in it.
    let found = discover(&paths, index.as_ref())?;
    let shown: Vec<String> = match &subdir {
        None => found.clone(),
        Some(d) => found.iter().filter(|p| p.starts_with(&format!("{d}/"))).cloned().collect(),
    };
    if !found.is_empty() && shown.is_empty() {
        return Err(format!("github.com/{owner}/{repo} has no mod in {}", subdir.unwrap_or_default()));
    }
    for path in shown {
        let raw: Value = json(&fetch.get(&raw_at(&path), false)?, &path)?;
        let mut m = describe(&raw, &path, repo, game, version, platform, &installed)?;
        if m.kind == "native" {
            match platform {
                None if m.compatible => {
                    m.compatible = false;
                    m.why = Some("Native mods are not loaded on this system.".to_string());
                }
                None => {}
                Some(p) => {
                    let name = asset_name(&m.id, &m.version, p);
                    match eligible.iter().find(|r| r.asset(&name).is_some()) {
                        Some(r) => {
                            m.asset = Some(name);
                            m.tag = Some(r.tag_name.clone());
                            if r.asset(SUMS).is_none() && m.compatible {
                                m.compatible = false;
                                m.why = Some(format!("Its release has no {SUMS}, so its download cannot be checked."));
                            }
                        }
                        None if m.compatible => {
                            m.compatible = false;
                            m.why = Some(if eligible.is_empty() {
                                "No release yet: a native mod is installed built, from a release.".to_string()
                            } else {
                                format!("No release has it built for this computer ({name}).")
                            });
                        }
                        None => {}
                    }
                }
            }
        } else {
            // A script mod or a data pack: its joined archive from a release when there is one
            // (checked against SHA256SUMS), else the repository's own files.
            let name = format!("{}-{}.zip", m.id, m.version);
            if let Some(r) = eligible.iter().find(|r| r.asset(&name).is_some() && r.asset(SUMS).is_some()) {
                m.asset = Some(name);
                m.tag = Some(r.tag_name.clone());
            }
        }
        mods.push(m);
    }
    let note = releases.is_empty().then(|| "This repository has no release yet.".to_string());
    Ok(GitHubRead {
        preview: Preview {
            url: format!("https://github.com/{owner}/{repo}"),
            label: format!("{owner}/{repo}"),
            host: "github".into(),
            owner: owner.into(),
            reference: at,
            commit,
            mods,
            note,
        },
        releases,
    })
}

/// The commit `at` names (a tag, a branch, HEAD), or none when the answer has no sha.
fn commit_of(fetch: &dyn Fetch, base: &str, at: &str) -> Result<Option<String>, String> {
    Ok(json::<Value>(&fetch.get(&format!("{base}/commits/{at}"), true)?, "the repository's commit")?
        .get("sha")
        .and_then(Value::as_str)
        .map(str::to_string))
}

// ---- Other git hosts --------------------------------------------------------------------------

fn git_found() -> bool {
    crate::detect::quiet(Path::new("git"))
        .arg("--version")
        .stdout(std::process::Stdio::null())
        .stderr(std::process::Stdio::null())
        .status()
        .is_ok_and(|s| s.success())
}

fn git(args: &[&str], cwd: Option<&Path>) -> Result<String, String> {
    let mut cmd = crate::detect::quiet(Path::new("git"));
    cmd.args(args).env("GIT_TERMINAL_PROMPT", "0").stdin(std::process::Stdio::null());
    if let Some(cwd) = cwd {
        cmd.current_dir(cwd);
    }
    let out = cmd.output().map_err(|e| format!("git cannot be started ({e})"))?;
    if !out.status.success() {
        return Err(format!("git {}: {}", args.first().unwrap_or(&""), String::from_utf8_lossy(&out.stderr).trim()));
    }
    Ok(String::from_utf8_lossy(&out.stdout).trim().to_string())
}

/// A shallow clone of another host's repository into a folder of `dir`'s: its files and commit.
fn clone_git(url: &str, dir: &Path) -> Result<(plugins::Scratch, String), String> {
    if !git_found() {
        return Err(format!(
            "{url} is not on GitHub, and reading another host needs git, which is not installed. Install git, or ask for the mod's .zip."
        ));
    }
    std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    let scratch = plugins::Scratch::new(dir, "clone")?;
    let into = scratch.0.join("repo");
    git(&["clone", "--depth", "1", "--quiet", url, &into.to_string_lossy()], None)?;
    let commit = git(&["rev-parse", "HEAD"], Some(&into))?;
    Ok((scratch, commit))
}

fn files_under(root: &Path) -> Vec<String> {
    fn walk(at: &Path, root: &Path, out: &mut Vec<String>) {
        for e in std::fs::read_dir(at).into_iter().flatten().flatten() {
            let p = e.path();
            if e.file_name() == ".git" {
                continue;
            }
            if p.is_dir() {
                walk(&p, root, out);
            } else if let Ok(rel) = p.strip_prefix(root) {
                out.push(rel.to_string_lossy().replace('\\', "/"));
            }
        }
    }
    let mut out = Vec::new();
    walk(root, root, &mut out);
    out
}

fn read_git(url: &str, ctx: (&str, &str, Option<&str>, &Path)) -> Result<(Preview, plugins::Scratch), String> {
    let (game, version, platform, dir) = ctx;
    let (scratch, commit) = clone_git(url, dir)?;
    let root = scratch.0.join("repo");
    let paths = files_under(&root);
    let index = if root.join(INDEX).is_file() {
        Some(json::<Value>(&std::fs::read(root.join(INDEX)).map_err(|e| e.to_string())?, INDEX)?)
    } else {
        None
    };
    let name = url.trim_end_matches(".git").rsplit(['/', ':']).next().unwrap_or("mod").to_string();
    let installed = installed_versions(dir);
    let mut mods = Vec::new();
    for path in discover(&paths, index.as_ref())? {
        let raw: Value = json(&std::fs::read(root.join(&path)).map_err(|e| e.to_string())?, &path)?;
        let mut m = describe(&raw, &path, &name, game, version, platform, &installed)?;
        if m.kind == "native" && m.compatible {
            m.compatible = false;
            m.why =
                Some("A native mod is installed built, from a release, which only GitHub's are read for yet.".into());
        }
        mods.push(m);
    }
    Ok((
        Preview {
            url: url.into(),
            label: url.into(),
            host: "git".into(),
            owner: String::new(),
            reference: "HEAD".into(),
            commit: Some(commit),
            mods,
            note: None,
        },
        scratch,
    ))
}

// ---- sources.json ---------------------------------------------------------------------------

/// Where one installed mod came from.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Default)]
#[serde(rename_all = "camelCase")]
pub struct ModSource {
    pub url: String,
    /// Its folder in the repository, "" for the top.
    #[serde(default)]
    pub path: String,
    pub version: String,
    #[serde(default)]
    pub tag: Option<String>,
    #[serde(default)]
    pub commit: Option<String>,
    /// "native", "script" or "pack".
    #[serde(default)]
    pub kind: String,
    /// Kept at this version: no update is offered.
    #[serde(default)]
    pub pin: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct Sources {
    pub format: u32,
    #[serde(default)]
    pub mods: BTreeMap<String, ModSource>,
}

impl Default for Sources {
    fn default() -> Self {
        Sources { format: 1, mods: BTreeMap::new() }
    }
}

pub fn read_sources(dir: &Path) -> Sources {
    std::fs::read(dir.join(SOURCES))
        .ok()
        .and_then(|b| serde_json::from_slice::<Sources>(&b).ok())
        .filter(|s| s.format == 1)
        .unwrap_or_default()
}

pub fn write_sources(dir: &Path, sources: &Sources) -> Result<(), String> {
    std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    let path = dir.join(SOURCES);
    let tmp = dir.join(format!("{SOURCES}.tmp"));
    let text = serde_json::to_string_pretty(sources).map_err(|e| e.to_string())? + "\n";
    std::fs::write(&tmp, text).map_err(|e| format!("{}: {e}", tmp.display()))?;
    std::fs::rename(&tmp, &path).map_err(|e| format!("{}: {e}", path.display()))
}

/// Forgets a removed mod's source.
pub fn forget(dir: &Path, id: &str) -> Result<(), String> {
    let mut s = read_sources(dir);
    if s.mods.remove(id).is_some() {
        write_sources(dir, &s)?;
    }
    Ok(())
}

/// Keeps a mod at its version (or lets it update again).
pub fn set_pin(dir: &Path, id: &str, pin: bool) -> Result<Sources, String> {
    let mut s = read_sources(dir);
    let entry = s.mods.get_mut(id).ok_or_else(|| format!("{id} was not installed from a repository"))?;
    entry.pin = pin;
    write_sources(dir, &s)?;
    Ok(s)
}

// ---- Preview, install, update ---------------------------------------------------------------

/// Where a game's install comes from: the plugins folder, the game and disc, this computer's
/// platform (None: no native mods here), the fetcher and GitHub's addresses.
pub struct Context<'a> {
    pub dir: &'a Path,
    pub game: &'a str,
    pub version: &'a str,
    pub platform: Option<&'a str>,
    pub fetch: &'a dyn Fetch,
    pub hosts: Hosts,
}

impl Context<'_> {
    fn parts(&self) -> (&str, &str, Option<&str>, &Path) {
        (self.game, self.version, self.platform, self.dir)
    }
}

/// What an address holds, for the Mods tab to show before anything is installed.
pub fn preview(ctx: &Context, url: &str) -> Result<Preview, String> {
    match parse_url(url)? {
        Source::GitHub { owner, repo, reference, tag } => {
            Ok(read_github(ctx.fetch, &ctx.hosts, &owner, &repo, reference.as_deref(), tag.as_deref(), ctx.parts())?
                .preview)
        }
        Source::Git { url } => Ok(read_git(&url, ctx.parts())?.0),
    }
}

/// What installing from an address came to.
#[derive(Debug, Clone, Serialize, PartialEq)]
#[serde(rename_all = "camelCase", tag = "kind")]
pub enum RemoteInstalled {
    /// A native or script mod, now in the store.
    Plugin { id: String, name: String, version: String, replaced: bool, enabled: bool },
    /// A data pack, its files in `path`: the overrides' to add (actions.json `overrides-add`).
    Pack { id: String, version: String, path: String },
}

/// Installs mod `id` of the repository at `url`: a native mod's built archive for this computer,
/// checked against the release's SHA256SUMS; a script mod into the store and a data pack's files for
/// the overrides, each from its release's joined archive (checked) or the repository's files. Remembers the
/// source, keeping its pin.
pub fn install(ctx: &Context, url: &str, id: &str) -> Result<RemoteInstalled, String> {
    let source = parse_url(url)?;
    std::fs::create_dir_all(ctx.dir).map_err(|e| format!("{}: {e}", ctx.dir.display()))?;
    let (preview, releases, clone) = match &source {
        Source::GitHub { owner, repo, reference, tag } => {
            let r = read_github(ctx.fetch, &ctx.hosts, owner, repo, reference.as_deref(), tag.as_deref(), ctx.parts())?;
            (r.preview, r.releases, None)
        }
        Source::Git { url } => {
            let (p, scratch) = read_git(url, ctx.parts())?;
            (p, Vec::new(), Some(scratch))
        }
    };
    let m =
        preview.mods.iter().find(|m| m.id == id).ok_or_else(|| format!("{} holds no mod {id}", preview.label))?.clone();
    if !m.compatible {
        return Err(m.why.clone().unwrap_or_else(|| format!("{} cannot be installed here", m.name)));
    }
    let release = |tag: &Option<String>| releases.iter().find(|r| Some(&r.tag_name) == tag.as_ref()).cloned();
    let sums_of = |r: &Release| -> Result<Option<String>, String> {
        match r.asset(SUMS) {
            Some(a) => Ok(Some(String::from_utf8_lossy(&ctx.fetch.get(&a.browser_download_url, false)?).into_owned())),
            None => Ok(None),
        }
    };
    let result = if m.kind == "native" {
        let r = release(&m.tag).ok_or("the release went away while it was read")?;
        let name = m.asset.clone().ok_or("no asset")?;
        let bytes = ctx.fetch.get(&r.asset(&name).ok_or("no asset")?.browser_download_url, false)?;
        verify(&bytes, &name, sums_of(&r)?.as_deref())?;
        let inside = manifest_in_zip(&bytes)?;
        let (got_id, got_version) = (
            inside.get("id").and_then(Value::as_str).unwrap_or_default(),
            inside.get("version").and_then(Value::as_str).unwrap_or_default(),
        );
        if got_id != m.id || got_version != m.version {
            return Err(format!(
                "{name} holds {got_id} {got_version}, not the {} {} the repository describes: it is not installed.",
                m.id, m.version
            ));
        }
        let scratch = plugins::Scratch::new(ctx.dir, "download")?;
        let file = scratch.0.join(&name);
        std::fs::write(&file, &bytes).map_err(|e| format!("{}: {e}", file.display()))?;
        match plugins::install(ctx.dir, ctx.game, ctx.version, &file)? {
            Installed::Plugin { id, name, version, replaced, enabled } => {
                RemoteInstalled::Plugin { id, name, version, replaced, enabled }
            }
            Installed::Pack => return Err(format!("{name} is a data pack, not the native mod described")),
        }
    } else {
        // A script mod goes to the store; a pack's files wait in .downloads/<id>-<version>/ for
        // overrides-add.
        let target = ctx.dir.join(".downloads").join(format!("{}-{}", m.id, m.version.replace('/', "_")));
        let _ = std::fs::remove_dir_all(&target);
        let staging = plugins::Scratch::new(ctx.dir, "pack")?;
        let folder = if let (Some(name), Some(r)) = (m.asset.clone(), release(&m.tag)) {
            let bytes = ctx.fetch.get(&r.asset(&name).ok_or("no asset")?.browser_download_url, false)?;
            verify(&bytes, &name, sums_of(&r)?.as_deref())?;
            let zip = staging.0.join(&name);
            std::fs::write(&zip, &bytes).map_err(|e| e.to_string())?;
            let out = staging.0.join("unpacked");
            plugins::unzip(&zip, &out)?;
            // package_mod.py's archives hold one top folder, <id>-<version>/.
            plugins::manifest_root(&out)?
        } else if let Some(scratch) = &clone {
            scratch.0.join("repo").join(&m.path)
        } else {
            // The repository's archive at the commit shown: one top folder, then the mod's.
            let Source::GitHub { owner, repo, .. } = &source else { unreachable!() };
            let at = preview.commit.clone().unwrap_or_else(|| preview.reference.clone());
            let bytes = ctx.fetch.get(&format!("{}/repos/{owner}/{repo}/zipball/{at}", ctx.hosts.api), false)?;
            let zip = staging.0.join("repo.zip");
            std::fs::write(&zip, &bytes).map_err(|e| e.to_string())?;
            let out = staging.0.join("repo");
            plugins::unzip(&zip, &out)?;
            let top = std::fs::read_dir(&out)
                .map_err(|e| e.to_string())?
                .flatten()
                .find(|e| e.path().is_dir())
                .ok_or("the repository's archive is empty")?
                .path();
            top.join(&m.path)
        };
        if !folder.join(MANIFEST).is_file() {
            return Err(format!("the download of {} has no {MANIFEST} where the repository has it", m.name));
        }
        if m.kind == "script" {
            let inside: Value = json(&std::fs::read(folder.join(MANIFEST)).map_err(|e| e.to_string())?, MANIFEST)?;
            let (got_id, got_version) = (
                inside.get("id").and_then(Value::as_str).unwrap_or_default(),
                inside.get("version").and_then(Value::as_str).unwrap_or_default(),
            );
            if got_id != m.id || got_version != m.version {
                return Err(format!(
                    "The download holds {got_id} {got_version}, not the {} {} the repository describes: it is not installed.",
                    m.id, m.version
                ));
            }
            match plugins::install(ctx.dir, ctx.game, ctx.version, &folder)? {
                Installed::Plugin { id, name, version, replaced, enabled } => {
                    RemoteInstalled::Plugin { id, name, version, replaced, enabled }
                }
                Installed::Pack => return Err(format!("{} is a data pack, not the script mod described", m.name)),
            }
        } else {
            std::fs::create_dir_all(target.parent().unwrap_or(ctx.dir)).map_err(|e| e.to_string())?;
            plugins::copy_tree(&folder, &target)?;
            RemoteInstalled::Pack {
                id: m.id.clone(),
                version: m.version.clone(),
                path: target.to_string_lossy().into_owned(),
            }
        }
    };
    let mut sources = read_sources(ctx.dir);
    let pin = sources.mods.get(&m.id).is_some_and(|s| s.pin);
    sources.mods.insert(
        m.id.clone(),
        ModSource {
            url: source.url(),
            path: m.path.clone(),
            version: m.version.clone(),
            tag: m.tag.clone(),
            commit: preview.commit.clone(),
            kind: m.kind.clone(),
            pin,
        },
    );
    write_sources(ctx.dir, &sources)?;
    Ok(result)
}

/// A newer version of an installed mod.
#[derive(Debug, Clone, Serialize, PartialEq)]
#[serde(rename_all = "camelCase")]
pub struct Update {
    pub id: String,
    pub installed: String,
    pub available: String,
    /// The release it is in, or the commit (a data pack with no release).
    pub tag: Option<String>,
    pub commit: Option<String>,
}

/// What each source not pinned offers that is newer: a release with a newer version built for
/// this computer, or, for a script mod or a data pack with no release, a newer commit. A source that cannot be read
/// now is left out (`errors` says which).
pub fn check_updates(ctx: &Context) -> (Vec<Update>, Vec<String>) {
    let mut updates = Vec::new();
    let mut errors = Vec::new();
    for (id, s) in read_sources(ctx.dir).mods {
        if s.pin {
            continue;
        }
        match preview(ctx, &s.url) {
            Err(e) => errors.push(format!("{id}: {e}")),
            Ok(p) => {
                let Some(m) = p.mods.iter().find(|m| m.id == id && m.compatible) else { continue };
                let newer = match (plugins::parse_version(&m.version), plugins::parse_version(&s.version)) {
                    (Some(a), Some(b)) => plugins::compare_versions(a, b).is_gt(),
                    _ => false,
                };
                let moved = m.tag.is_none() && m.kind != "native" && p.commit.is_some() && p.commit != s.commit;
                if newer || (moved && m.version == s.version) {
                    updates.push(Update {
                        id,
                        installed: s.version.clone(),
                        available: m.version.clone(),
                        tag: m.tag.clone(),
                        commit: if moved { p.commit.clone() } else { None },
                    });
                }
            }
        }
    }
    (updates, errors)
}

/// Installs the newest version of mod `id` from where it came from (its place, on/off and settings
/// kept). A pinned mod is refused.
pub fn update(ctx: &Context, id: &str) -> Result<RemoteInstalled, String> {
    let s = read_sources(ctx.dir).mods.remove(id).ok_or_else(|| format!("{id} was not installed from a repository"))?;
    if s.pin {
        return Err(format!("{id} is kept at {}: let it update first", s.version));
    }
    install(ctx, &s.url, id)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::cell::RefCell;
    use std::collections::HashMap;
    use std::io::Write;
    use std::path::PathBuf;

    /// GitHub, answered from memory; every request is recorded.
    #[derive(Default)]
    struct Fixture {
        files: HashMap<String, Vec<u8>>,
        asked: RefCell<Vec<String>>,
    }

    impl Fixture {
        fn put(&mut self, url: &str, bytes: impl Into<Vec<u8>>) {
            self.files.insert(url.to_string(), bytes.into());
        }
    }

    impl Fetch for Fixture {
        fn get(&self, url: &str, _api: bool) -> Result<Vec<u8>, String> {
            self.asked.borrow_mut().push(url.to_string());
            self.files.get(url).cloned().ok_or_else(|| format!("{url}: not found"))
        }
    }

    const API: &str = "https://api.test";
    const RAW: &str = "https://raw.test";
    const DL: &str = "https://dl.test";
    const PLATFORM: &str = "linux-x86_64";

    fn hosts() -> Hosts {
        Hosts { api: API.into(), raw: RAW.into() }
    }

    fn manifest(id: &str, version: &str) -> String {
        format!(
            r#"{{"format": 2, "id": "{id}", "name": "Mod {id}", "version": "{version}", "author": "Someone",
                "api": "1.0", "games": {{"rac1": ["pal", "ntsc"]}},
                "plugin": {{"{PLATFORM}": "lib/{PLATFORM}/m.so", "windows-x86_64": "lib/windows-x86_64/m.dll"}}}}"#
        )
    }

    fn mod_zip(id: &str, version: &str) -> Vec<u8> {
        let mut w = zip::ZipWriter::new(std::io::Cursor::new(Vec::new()));
        let o = zip::write::SimpleFileOptions::default();
        w.start_file(format!("{id}/openrac-mod.json"), o).unwrap();
        // As package_mod.py writes it: the manifest names this platform's library only.
        let one = manifest(id, version).replace(r#", "windows-x86_64": "lib/windows-x86_64/m.dll""#, "");
        w.write_all(one.as_bytes()).unwrap();
        w.start_file(format!("{id}/lib/{PLATFORM}/m.so"), o).unwrap();
        w.write_all(b"library").unwrap();
        w.finish().unwrap().into_inner()
    }

    fn sha(bytes: &[u8]) -> String {
        format!("{:x}", Sha256::digest(bytes))
    }

    /// A repository o/r with the given files at commit c1, releases as given.
    fn repo(fx: &mut Fixture, files: &[(&str, String)], releases: Value, commit: &str) {
        fx.put(&format!("{API}/repos/o/r/releases?per_page=100"), releases.to_string());
        let tree: Vec<Value> = files.iter().map(|(p, _)| serde_json::json!({"path": p, "type": "blob"})).collect();
        for at in ["HEAD", "v1.0.0", "v1.1.0", "v2.0.0", "main"] {
            fx.put(&format!("{API}/repos/o/r/commits/{at}"), serde_json::json!({"sha": commit}).to_string());
        }
        fx.put(
            &format!("{API}/repos/o/r/git/trees/{commit}?recursive=1"),
            serde_json::json!({"tree": tree}).to_string(),
        );
        for (p, text) in files {
            fx.put(&format!("{RAW}/o/r/{commit}/{p}"), text.clone());
        }
    }

    /// A release with a mod's asset for this platform and, unless `sums` is None, SHA256SUMS.
    fn release(fx: &mut Fixture, tag: &str, id: &str, version: &str, sums: Option<String>, pre: bool) -> Value {
        let name = asset_name(id, version, PLATFORM);
        let bytes = mod_zip(id, version);
        fx.put(&format!("{DL}/{tag}/{name}"), bytes.clone());
        let mut assets = vec![serde_json::json!({"name": name, "browser_download_url": format!("{DL}/{tag}/{name}")})];
        if let Some(sums) = sums {
            let text = if sums.is_empty() { format!("{}  {name}\n", sha(&bytes)) } else { sums };
            fx.put(&format!("{DL}/{tag}/SHA256SUMS"), text);
            assets.push(
                serde_json::json!({"name": "SHA256SUMS", "browser_download_url": format!("{DL}/{tag}/SHA256SUMS")}),
            );
        }
        serde_json::json!({"tag_name": tag, "draft": false, "prerelease": pre, "assets": assets})
    }

    fn ctx<'a>(dir: &'a Path, fetch: &'a dyn Fetch) -> Context<'a> {
        Context { dir, game: "rac1", version: "pal", platform: Some(PLATFORM), fetch, hosts: hosts() }
    }

    #[test]
    fn addresses() {
        let gh = |o: &str, r: &str, reference: Option<&str>, tag: Option<&str>| Source::GitHub {
            owner: o.into(),
            repo: r.into(),
            reference: reference.map(Into::into),
            tag: tag.map(Into::into),
        };
        assert_eq!(parse_url("https://github.com/OpenRAC/rac1-mods").unwrap(), gh("OpenRAC", "rac1-mods", None, None));
        assert_eq!(parse_url("github.com/OpenRAC/rac1-mods.git/").unwrap(), gh("OpenRAC", "rac1-mods", None, None));
        assert_eq!(parse_url("https://www.github.com/a/b/tree/dev/x").unwrap(), gh("a", "b", Some("dev/x"), None));
        assert_eq!(
            parse_url("https://github.com/a/b/releases/tag/v1.2.0").unwrap(),
            gh("a", "b", None, Some("v1.2.0"))
        );
        assert_eq!(parse_url("https://github.com/a/b/releases/latest").unwrap(), gh("a", "b", None, None));
        assert!(parse_url("https://github.com/a").is_err());
        assert!(parse_url("https://github.com/a/b/issues/3").is_err());
        assert!(parse_url("https://github.com/a/../b").is_err());
        assert_eq!(
            parse_url("https://gitlab.com/x/y.git").unwrap(),
            Source::Git { url: "https://gitlab.com/x/y.git".into() }
        );
        assert_eq!(
            parse_url("git@codeberg.org:x/y.git").unwrap(),
            Source::Git { url: "git@codeberg.org:x/y.git".into() }
        );
        assert!(parse_url("not a url").is_err());
        assert!(parse_url("").is_err());
    }

    #[test]
    fn every_common_link_form() {
        let plain = Source::GitHub { owner: "OpenRAC".into(), repo: "rac1-mods".into(), reference: None, tag: None };
        for text in [
            "https://github.com/OpenRAC/rac1-mods",
            "http://github.com/OpenRAC/rac1-mods",
            "github.com/OpenRAC/rac1-mods",
            "  https://github.com/OpenRAC/rac1-mods/  ",
            "https://github.com/OpenRAC/rac1-mods.git",
            "https://github.com/OpenRAC/rac1-mods?tab=readme",
            "git@github.com:OpenRAC/rac1-mods.git",
            "git@github.com:OpenRAC/rac1-mods",
            "ssh://git@github.com/OpenRAC/rac1-mods.git",
            "OpenRAC/rac1-mods",
        ] {
            assert_eq!(parse_url(text).unwrap(), plain, "{text}");
        }
        assert_eq!(
            parse_url("https://github.com/OpenRAC/rac1-mods/tree/master/lock-strafe").unwrap(),
            Source::GitHub {
                owner: "OpenRAC".into(),
                repo: "rac1-mods".into(),
                reference: Some("master/lock-strafe".into()),
                tag: None
            }
        );
        assert_eq!(
            parse_url("https://github.com/OpenRAC/rac1-mods/releases/tag/v1.0.0").unwrap(),
            Source::GitHub {
                owner: "OpenRAC".into(),
                repo: "rac1-mods".into(),
                reference: None,
                tag: Some("v1.0.0".into())
            }
        );
        // Not a shorthand: a host's name, or a path deeper than owner/repo.
        assert!(matches!(parse_url("gitlab.com/x").unwrap_err(), e if e.contains("not a repository")));
        assert!(parse_url("a/b/c").is_err());
    }

    #[test]
    fn a_link_to_a_folder_shows_its_mods_only() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let files = [
            ("lock-strafe/openrac-mod.json", manifest("lock-strafe", "1.0.0")),
            ("button-swap/openrac-mod.json", script_manifest("button-swap", "1.0.0")),
        ];
        repo(&mut fx, &files, serde_json::json!([]), "c1");
        // The branch is the first part of the path the repository knows; the rest is the folder.
        let p = preview(&ctx(dir.path(), &fx), "https://github.com/o/r/tree/main/button-swap").unwrap();
        assert_eq!(p.reference, "main");
        assert_eq!(p.mods.iter().map(|m| m.id.as_str()).collect::<Vec<_>>(), ["button-swap"]);
        // A branch whose name holds a slash: the longer start is tried when the shorter is unknown.
        fx.put(&format!("{API}/repos/o/r/commits/feature/x"), serde_json::json!({"sha": "c1"}).to_string());
        let p = preview(&ctx(dir.path(), &fx), "https://github.com/o/r/tree/feature/x/lock-strafe").unwrap();
        assert_eq!(p.reference, "feature/x");
        assert_eq!(p.mods.iter().map(|m| m.id.as_str()).collect::<Vec<_>>(), ["lock-strafe"]);
        let err = preview(&ctx(dir.path(), &fx), "https://github.com/o/r/tree/main/nothing-here").unwrap_err();
        assert!(err.contains("no mod in nothing-here"), "{err}");
    }

    #[test]
    fn a_repository_github_does_not_show_is_said_plainly() {
        let dir = tempfile::tempdir().unwrap();
        let fx = Fixture::default();
        let err = preview(&ctx(dir.path(), &fx), "https://github.com/someone/private-thing").unwrap_err();
        assert_eq!(err, NOT_FOUND);
    }

    #[test]
    fn discovery() {
        let p = |v: &[&str]| v.iter().map(|s| s.to_string()).collect::<Vec<_>>();
        assert_eq!(discover(&p(&["openrac-mod.json", "a/openrac-mod.json"]), None).unwrap(), ["openrac-mod.json"]);
        assert_eq!(
            discover(&p(&["b/openrac-mod.json", "a/openrac-mod.json", "a/x/openrac-mod.json", "README.md"]), None)
                .unwrap(),
            ["a/openrac-mod.json", "b/openrac-mod.json"]
        );
        let index = serde_json::json!({"format": 1, "mods": ["b"]});
        assert_eq!(
            discover(&p(&["b/openrac-mod.json", "a/openrac-mod.json"]), Some(&index)).unwrap(),
            ["b/openrac-mod.json"]
        );
        assert!(discover(&p(&["a/openrac-mod.json"]), Some(&index)).unwrap_err().contains("names b"));
        assert!(discover(&p(&["README.md"]), None).unwrap_err().contains("No OpenRAC mod"));
        let bad = serde_json::json!({"mods": ["../x"]});
        assert!(discover(&p(&["../x/openrac-mod.json"]), Some(&bad)).is_err());
    }

    #[test]
    fn a_manifest_in_its_sources_is_a_native_mod() {
        // As a repository holds it: no `plugin` yet (the release's archive adds it).
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let rel = release(&mut fx, "v1.0.0", "lock-strafe", "1.0.0", Some(String::new()), false);
        let source = r#"{"format": 2, "id": "lock-strafe", "name": "Lock-strafe", "version": "1.0.0",
            "author": "A", "api": "1.1", "games": {"rac1": ["pal", "ntsc"]}}"#;
        repo(&mut fx, &[("lock-strafe/openrac-mod.json", source.to_string())], serde_json::json!([rel]), "c1");
        let m = &preview(&ctx(dir.path(), &fx), "https://github.com/o/r").unwrap().mods[0];
        assert_eq!(m.kind, "native");
        assert!(m.compatible, "{:?}", m.why);
        assert_eq!(m.asset.as_deref(), Some("lock-strafe-1.0.0-linux-x86_64.zip"));
        // No platform for native mods (Android): refused, with the reason.
        let none = Context { platform: None, ..ctx(dir.path(), &fx) };
        let m = &preview(&none, "https://github.com/o/r").unwrap().mods[0];
        assert!(!m.compatible && m.why.as_deref().unwrap().contains("not loaded"), "{:?}", m.why);
    }

    #[test]
    fn asset_names_per_platform() {
        assert_eq!(asset_name("lock-strafe", "1.0.0", "macos-arm64"), "lock-strafe-1.0.0-macos-arm64.zip");
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let rel = release(&mut fx, "v1.0.0", "lock-strafe", "1.0.0", Some(String::new()), false);
        repo(
            &mut fx,
            &[("lock-strafe/openrac-mod.json", manifest("lock-strafe", "1.0.0"))],
            serde_json::json!([rel]),
            "c1",
        );
        let p = preview(&ctx(dir.path(), &fx), "https://github.com/o/r").unwrap();
        let m = &p.mods[0];
        assert!(m.compatible, "{:?}", m.why);
        assert_eq!(m.asset.as_deref(), Some("lock-strafe-1.0.0-linux-x86_64.zip"));
        assert_eq!(m.tag.as_deref(), Some("v1.0.0"));
        assert_eq!((p.label.as_str(), p.owner.as_str(), m.kind.as_str()), ("o/r", "o", "native"));
        // On Windows the manifest has a library but the release has no asset for it.
        let win = Context { platform: Some("windows-x86_64"), ..ctx(dir.path(), &fx) };
        let m = &preview(&win, "https://github.com/o/r").unwrap().mods[0];
        assert!(!m.compatible);
        assert!(m.why.as_deref().unwrap().contains("lock-strafe-1.0.0-windows-x86_64.zip"), "{:?}", m.why);
        // macOS: the manifest names no library for it.
        let mac = Context { platform: Some("macos-arm64"), ..ctx(dir.path(), &fx) };
        assert!(preview(&mac, "https://github.com/o/r").unwrap().mods[0].why.as_deref().unwrap().contains("Not built"));
        // No native mods at all (Android).
        let none = Context { platform: None, ..ctx(dir.path(), &fx) };
        assert!(!preview(&none, "https://github.com/o/r").unwrap().mods[0].compatible);
    }

    #[test]
    fn no_release_yet() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        repo(&mut fx, &[("openrac-mod.json", manifest("m", "0.1.0"))], serde_json::json!([]), "c1");
        let p = preview(&ctx(dir.path(), &fx), "https://github.com/o/r").unwrap();
        assert_eq!(p.note.as_deref(), Some("This repository has no release yet."));
        assert_eq!(p.reference, "HEAD");
        assert!(p.mods[0].why.as_deref().unwrap().starts_with("No release yet"));
        assert!(install(&ctx(dir.path(), &fx), "https://github.com/o/r", "m").is_err());
    }

    #[test]
    fn installs_and_remembers_the_source() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let rel = release(&mut fx, "v1.0.0", "lock-strafe", "1.0.0", Some(String::new()), false);
        repo(
            &mut fx,
            &[("lock-strafe/openrac-mod.json", manifest("lock-strafe", "1.0.0"))],
            serde_json::json!([rel]),
            "c1",
        );
        let c = ctx(dir.path(), &fx);
        let got = install(&c, "https://github.com/o/r/", "lock-strafe").unwrap();
        assert!(
            matches!(got, RemoteInstalled::Plugin { ref id, ref version, .. } if id == "lock-strafe" && version == "1.0.0")
        );
        assert!(dir.path().join("lock-strafe/openrac-mod.json").is_file());
        let s = read_sources(dir.path());
        let entry = &s.mods["lock-strafe"];
        assert_eq!(
            (entry.url.as_str(), entry.path.as_str(), entry.version.as_str(), entry.tag.as_deref(), entry.pin),
            ("https://github.com/o/r", "lock-strafe", "1.0.0", Some("v1.0.0"), false)
        );
        // The preview now says it is installed.
        assert_eq!(preview(&c, "https://github.com/o/r").unwrap().mods[0].installed.as_deref(), Some("1.0.0"));
        // sources.json round trip, pin, forget.
        set_pin(dir.path(), "lock-strafe", true).unwrap();
        assert!(read_sources(dir.path()).mods["lock-strafe"].pin);
        forget(dir.path(), "lock-strafe").unwrap();
        assert!(read_sources(dir.path()).mods.is_empty());
        assert!(set_pin(dir.path(), "lock-strafe", true).is_err());
    }

    #[test]
    fn checksums_refuse() {
        let id = "m";
        for (sums, want) in [
            (None, "has no SHA256SUMS"),
            (Some(format!("{}  other.zip\n", "0".repeat(64))), "does not list"),
            (Some(format!("{} *{}\n", "0".repeat(64), asset_name(id, "1.0.0", PLATFORM))), "SHA-256 differs"),
        ] {
            let dir = tempfile::tempdir().unwrap();
            let mut fx = Fixture::default();
            let missing = sums.is_none();
            let rel = release(&mut fx, "v1.0.0", id, "1.0.0", sums, false);
            repo(&mut fx, &[("openrac-mod.json", manifest(id, "1.0.0"))], serde_json::json!([rel]), "c1");
            let c = ctx(dir.path(), &fx);
            let err = install(&c, "https://github.com/o/r", id).unwrap_err();
            if missing {
                // Shown as not installable before anything is downloaded.
                assert!(err.contains("no SHA256SUMS"), "{err}");
            } else {
                assert!(err.contains(want), "{err}");
            }
            assert!(!dir.path().join(id).exists());
        }
        // The parser: binary marker, upper case, junk lines.
        let s = parse_sums(&format!("{}  a.zip\n{} *b.zip\nnonsense\n", "A".repeat(64), "b".repeat(64)));
        assert_eq!(s["a.zip"], "a".repeat(64));
        assert_eq!(s["b.zip"], "b".repeat(64));
        assert_eq!(s.len(), 2);
    }

    #[test]
    fn a_download_that_is_another_mod_is_refused() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let name = asset_name("m", "1.0.0", PLATFORM);
        let bytes = mod_zip("other", "9.9.9");
        fx.put(&format!("{DL}/v1.0.0/{name}"), bytes.clone());
        fx.put(&format!("{DL}/v1.0.0/SHA256SUMS"), format!("{}  {name}\n", sha(&bytes)));
        let rel = serde_json::json!({"tag_name": "v1.0.0", "assets": [
            {"name": name, "browser_download_url": format!("{DL}/v1.0.0/{name}")},
            {"name": "SHA256SUMS", "browser_download_url": format!("{DL}/v1.0.0/SHA256SUMS")}]});
        repo(&mut fx, &[("openrac-mod.json", manifest("m", "1.0.0"))], serde_json::json!([rel]), "c1");
        let err = install(&ctx(dir.path(), &fx), "https://github.com/o/r", "m").unwrap_err();
        assert!(err.contains("holds other 9.9.9"), "{err}");
        assert!(!dir.path().join("m").exists() && !dir.path().join("other").exists());
    }

    #[test]
    fn a_data_pack_from_the_repository_archive() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let pack = r#"{"format": 1, "game": "rac1", "version": "pal", "name": "Shiny bolts", "author": "A"}"#;
        repo(
            &mut fx,
            &[("shiny/openrac-mod.json", pack.to_string()), ("shiny/level_00/textures/x.png", "png".into())],
            serde_json::json!([]),
            "c1",
        );
        let mut w = zip::ZipWriter::new(std::io::Cursor::new(Vec::new()));
        let o = zip::write::SimpleFileOptions::default();
        w.start_file("o-r-c1/shiny/openrac-mod.json", o).unwrap();
        w.write_all(pack.as_bytes()).unwrap();
        w.start_file("o-r-c1/shiny/level_00/textures/x.png", o).unwrap();
        w.write_all(b"png").unwrap();
        w.start_file("o-r-c1/README.md", o).unwrap();
        fx.put(&format!("{API}/repos/o/r/zipball/c1"), w.finish().unwrap().into_inner());
        let c = ctx(dir.path(), &fx);
        let p = preview(&c, "https://github.com/o/r").unwrap();
        assert_eq!((p.mods[0].id.as_str(), p.mods[0].kind.as_str(), p.mods[0].compatible), ("shiny", "pack", true));
        let RemoteInstalled::Pack { path, .. } = install(&c, "https://github.com/o/r", "shiny").unwrap() else {
            panic!("not a pack")
        };
        let path = PathBuf::from(path);
        assert!(path.join("openrac-mod.json").is_file() && path.join("level_00/textures/x.png").is_file());
        assert!(!path.join("README.md").exists(), "only the mod's folder");
        assert_eq!(read_sources(dir.path()).mods["shiny"].commit.as_deref(), Some("c1"));
        // A newer commit on the branch is an update for a pack with no release.
        let mut fx2 = Fixture::default();
        repo(&mut fx2, &[("shiny/openrac-mod.json", pack.to_string())], serde_json::json!([]), "c2");
        let (updates, errors) = check_updates(&ctx(dir.path(), &fx2));
        assert!(errors.is_empty(), "{errors:?}");
        assert_eq!(updates.len(), 1);
        assert_eq!(updates[0].commit.as_deref(), Some("c2"));
        // A pack for another game is not offered.
        let mut fx3 = Fixture::default();
        repo(&mut fx3, &[("openrac-mod.json", pack.replace("rac1", "rac2"))], serde_json::json!([]), "c1");
        assert!(!preview(&ctx(dir.path(), &fx3), "https://github.com/o/r").unwrap().mods[0].compatible);
    }

    fn script_manifest(id: &str, version: &str) -> String {
        format!(
            r#"{{"format": 2, "id": "{id}", "name": "Mod {id}", "version": "{version}", "author": "Someone",
                "type": "script", "scripts": ["main.luau"], "api": "1.1", "games": {{"rac1": ["pal", "ntsc"]}}}}"#
        )
    }

    /// The repository's archive at `commit`, holding a script mod in `folder`.
    fn script_zipball(fx: &mut Fixture, commit: &str, folder: &str, id: &str, version: &str) {
        let mut w = zip::ZipWriter::new(std::io::Cursor::new(Vec::new()));
        let o = zip::write::SimpleFileOptions::default();
        w.start_file(format!("o-r-{commit}/{folder}/openrac-mod.json"), o).unwrap();
        w.write_all(script_manifest(id, version).as_bytes()).unwrap();
        w.start_file(format!("o-r-{commit}/{folder}/main.luau"), o).unwrap();
        w.write_all(b"openrac.log('hi')\n").unwrap();
        w.start_file(format!("o-r-{commit}/README.md"), o).unwrap();
        fx.put(&format!("{API}/repos/o/r/zipball/{commit}"), w.finish().unwrap().into_inner());
    }

    #[test]
    fn a_script_mod_from_the_repository_itself() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        // No release: the repository's own files are the mod.
        repo(
            &mut fx,
            &[("swap/openrac-mod.json", script_manifest("swap", "1.0.0")), ("swap/main.luau", "-- x".into())],
            serde_json::json!([]),
            "c1",
        );
        script_zipball(&mut fx, "c1", "swap", "swap", "1.0.0");
        // On a system with no native mods (Android: no platform) it is offered all the same.
        let android = Context { platform: None, ..ctx(dir.path(), &fx) };
        let p = preview(&android, "https://github.com/o/r").unwrap();
        let m = &p.mods[0];
        assert_eq!((m.id.as_str(), m.kind.as_str(), m.compatible, m.asset.as_deref()), ("swap", "script", true, None));
        let got = install(&android, "https://github.com/o/r", "swap").unwrap();
        assert!(matches!(got, RemoteInstalled::Plugin { ref id, enabled: true, .. } if id == "swap"));
        assert!(dir.path().join("swap/main.luau").is_file());
        assert!(!dir.path().join("swap/README.md").exists(), "only the mod's folder");
        let s = &read_sources(dir.path()).mods["swap"];
        assert_eq!((s.kind.as_str(), s.commit.as_deref(), s.tag.as_deref()), ("script", Some("c1"), None));
        // A newer commit on the branch is an update, and installs as one.
        let mut fx2 = Fixture::default();
        repo(&mut fx2, &[("swap/openrac-mod.json", script_manifest("swap", "1.0.0"))], serde_json::json!([]), "c2");
        script_zipball(&mut fx2, "c2", "swap", "swap", "1.0.0");
        let c2 = ctx(dir.path(), &fx2);
        let (u, errors) = check_updates(&c2);
        assert!(errors.is_empty(), "{errors:?}");
        assert_eq!((u.len(), u[0].commit.as_deref()), (1, Some("c2")));
        assert!(matches!(update(&c2, "swap").unwrap(), RemoteInstalled::Plugin { replaced: true, .. }));
        assert_eq!(read_sources(dir.path()).mods["swap"].commit.as_deref(), Some("c2"));
        assert!(check_updates(&c2).0.is_empty());
        // A script mod for an interface this launcher's port does not have is not offered.
        let mut fx3 = Fixture::default();
        repo(
            &mut fx3,
            &[("openrac-mod.json", script_manifest("swap", "1.0.0").replace("\"1.1\"", "\"1.9\""))],
            serde_json::json!([]),
            "c1",
        );
        let p = preview(&ctx(dir.path(), &fx3), "https://github.com/o/r").unwrap();
        assert!(!p.mods[0].compatible && p.mods[0].why.as_deref().unwrap().contains("interface 1.9"));
    }

    #[test]
    fn a_script_mod_from_its_release() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        // The joined archive package_mod.py writes for a script mod, listed in SHA256SUMS.
        let mut w = zip::ZipWriter::new(std::io::Cursor::new(Vec::new()));
        let o = zip::write::SimpleFileOptions::default();
        w.start_file("swap-1.0.0/openrac-mod.json", o).unwrap();
        w.write_all(script_manifest("swap", "1.0.0").as_bytes()).unwrap();
        w.start_file("swap-1.0.0/main.luau", o).unwrap();
        w.write_all(b"-- released").unwrap();
        let bytes = w.finish().unwrap().into_inner();
        let name = "swap-1.0.0.zip";
        fx.put(&format!("{DL}/v1.0.0/{name}"), bytes.clone());
        let release = serde_json::json!({"tag_name": "v1.0.0", "draft": false, "prerelease": false, "assets": [
            {"name": name, "browser_download_url": format!("{DL}/v1.0.0/{name}")},
            {"name": "SHA256SUMS", "browser_download_url": format!("{DL}/v1.0.0/SHA256SUMS")}]});
        fx.put(&format!("{DL}/v1.0.0/SHA256SUMS"), format!("{}  {name}\n", sha(&bytes)));
        repo(&mut fx, &[("openrac-mod.json", script_manifest("swap", "1.0.0"))], serde_json::json!([release]), "c1");
        let c = ctx(dir.path(), &fx);
        let p = preview(&c, "https://github.com/o/r").unwrap();
        assert_eq!((p.mods[0].asset.as_deref(), p.mods[0].tag.as_deref()), (Some(name), Some("v1.0.0")));
        assert!(matches!(install(&c, "https://github.com/o/r", "swap").unwrap(), RemoteInstalled::Plugin { .. }));
        assert_eq!(std::fs::read_to_string(dir.path().join("swap/main.luau")).unwrap(), "-- released");
        assert_eq!(read_sources(dir.path()).mods["swap"].tag.as_deref(), Some("v1.0.0"));
        // A hash that differs is refused.
        fx.put(&format!("{DL}/v1.0.0/SHA256SUMS"), format!("{}  {name}\n", "0".repeat(64)));
        let refused = install(&ctx(dir.path(), &fx), "https://github.com/o/r", "swap").unwrap_err();
        assert!(refused.contains("SHA-256") || refused.contains("differs"), "{refused}");
    }

    #[test]
    fn updates() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let r1 = release(&mut fx, "v1.0.0", "m", "1.0.0", Some(String::new()), false);
        repo(&mut fx, &[("openrac-mod.json", manifest("m", "1.0.0"))], serde_json::json!([r1.clone()]), "c1");
        install(&ctx(dir.path(), &fx), "https://github.com/o/r", "m").unwrap();
        assert!(check_updates(&ctx(dir.path(), &fx)).0.is_empty(), "nothing newer yet");

        // 1.1.0 released; 2.0.0 only as a prerelease, which is not offered.
        let mut fx = Fixture::default();
        let r1 = release(&mut fx, "v1.0.0", "m", "1.0.0", Some(String::new()), false);
        let r2 = release(&mut fx, "v1.1.0", "m", "1.1.0", Some(String::new()), false);
        let pre = release(&mut fx, "v2.0.0", "m", "2.0.0", Some(String::new()), true);
        repo(&mut fx, &[("openrac-mod.json", manifest("m", "1.1.0"))], serde_json::json!([pre, r2, r1]), "c2");
        let c = ctx(dir.path(), &fx);
        let (u, errors) = check_updates(&c);
        assert!(errors.is_empty(), "{errors:?}");
        assert_eq!(
            u,
            [Update {
                id: "m".into(),
                installed: "1.0.0".into(),
                available: "1.1.0".into(),
                tag: Some("v1.1.0".into()),
                commit: None
            }]
        );
        // Pinned: nothing offered, and update refused.
        set_pin(dir.path(), "m", true).unwrap();
        assert!(check_updates(&c).0.is_empty());
        assert!(update(&c, "m").is_err());
        set_pin(dir.path(), "m", false).unwrap();
        let got = update(&c, "m").unwrap();
        assert!(matches!(got, RemoteInstalled::Plugin { ref version, replaced: true, .. } if version == "1.1.0"));
        assert_eq!(read_sources(dir.path()).mods["m"].version, "1.1.0");
        assert!(check_updates(&c).0.is_empty());
        // An older release is not an update.
        let mut fx = Fixture::default();
        let r1 = release(&mut fx, "v1.0.0", "m", "1.0.0", Some(String::new()), false);
        repo(&mut fx, &[("openrac-mod.json", manifest("m", "1.0.0"))], serde_json::json!([r1]), "c1");
        assert!(check_updates(&ctx(dir.path(), &fx)).0.is_empty());
        // A source that cannot be read is said, not fatal.
        let empty = Fixture::default();
        let (u, errors) = check_updates(&ctx(dir.path(), &empty));
        assert!(u.is_empty() && errors.len() == 1);
    }

    #[test]
    fn a_named_release() {
        let dir = tempfile::tempdir().unwrap();
        let mut fx = Fixture::default();
        let r1 = release(&mut fx, "v1.0.0", "m", "1.0.0", Some(String::new()), false);
        repo(&mut fx, &[("openrac-mod.json", manifest("m", "1.0.0"))], serde_json::json!([r1]), "c1");
        let p = preview(&ctx(dir.path(), &fx), "https://github.com/o/r/releases/tag/v1.0.0").unwrap();
        assert_eq!(p.reference, "v1.0.0");
        assert!(preview(&ctx(dir.path(), &fx), "https://github.com/o/r/releases/tag/v9")
            .unwrap_err()
            .contains("no release v9"));
    }

    /// Another host, through git: a local repository stands for it. Skipped without git.
    #[test]
    fn another_host_through_git() {
        if !git_found() {
            eprintln!("git is not installed: skipped");
            return;
        }
        let dir = tempfile::tempdir().unwrap();
        let src = dir.path().join("src");
        std::fs::create_dir_all(src.join("shiny/level_00")).unwrap();
        let pack = r#"{"format": 1, "game": "rac1", "version": "pal", "name": "Shiny", "author": "A"}"#;
        std::fs::write(src.join("shiny/openrac-mod.json"), pack).unwrap();
        std::fs::write(src.join("shiny/level_00/x.png"), "png").unwrap();
        std::fs::create_dir_all(src.join("native")).unwrap();
        std::fs::write(src.join("native/openrac-mod.json"), manifest("native", "1.0.0")).unwrap();
        let run = |args: &[&str]| git(args, Some(&src)).unwrap();
        run(&["init", "-q"]);
        run(&["-c", "user.name=t", "-c", "user.email=t@t", "add", "."]);
        run(&["-c", "user.name=t", "-c", "user.email=t@t", "commit", "-q", "-m", "x"]);
        // A file:// address stands for another host's https one.
        let url = format!("file://{}", src.display());
        let plugins = dir.path().join("plugins");
        let fx = Fixture::default();
        let c = ctx(&plugins, &fx);
        let (p, _scratch) = read_git(&url, c.parts()).unwrap();
        let shiny = p.mods.iter().find(|m| m.id == "shiny").unwrap();
        assert!(shiny.compatible);
        let native = p.mods.iter().find(|m| m.id == "native").unwrap();
        assert!(!native.compatible && native.why.as_deref().unwrap().contains("release"));
        assert!(fx.asked.borrow().is_empty(), "nothing fetched over https");
    }

    /// The real GitHub, once by hand: `cargo test -p openrac-launcher-core live_rac1_mods -- --ignored`.
    #[test]
    #[ignore = "network"]
    fn live_rac1_mods() {
        let dir = tempfile::tempdir().unwrap();
        let c = Context {
            dir: dir.path(),
            game: "rac1",
            version: "pal",
            platform: plugins::platform_key(),
            fetch: &Curl,
            hosts: Hosts::default(),
        };
        // A public repository with releases (the SDK's) and no mod at its top: the API's shapes
        // (releases, commits, trees) read, and discovery says there is none.
        let e = preview(&c, "https://github.com/OpenRAC/OpenRAC").unwrap_err();
        eprintln!("OpenRAC/OpenRAC: {e}");
        assert!(e.starts_with("No OpenRAC mod"), "{e}");
        // rac1-mods, once it is public, holds the lock-strafe.
        match preview(&c, "https://github.com/OpenRAC/rac1-mods") {
            Ok(p) => assert!(p.mods.iter().any(|m| m.id == "lock-strafe"), "{p:#?}"),
            Err(e) => eprintln!("OpenRAC/rac1-mods: {e}"),
        }
    }
}
