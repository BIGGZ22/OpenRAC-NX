//! The games and their versions, read from the checkout: every
//! `games/<game>/game.json` (the same manifests `tools/openrac.py` reads) and
//! the consolidated `progress/summary.json` that `openrac.py progress` writes.
//!
//! Only the fields the launcher shows are parsed; everything else in
//! game.json is ignored, so the manifests can grow without launcher changes.

use std::collections::BTreeMap;
use std::path::Path;

use serde::{Deserialize, Serialize};

/// Every game in the checkout, in game order (rac1, rac2, ...).
#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Catalog {
    pub games: Vec<Game>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Game {
    /// `rac1`, `rac2`, ...: the directory under `games/`.
    pub id: String,
    pub title: String,
    pub year: Option<u32>,
    /// Names other regions released the game under (`"pal": "Ratchet: Gladiator"`).
    pub titles: BTreeMap<String, String>,
    /// The disc's video standard of each version whose game.json names one
    /// (`versions.<v>.disc.video`, `"pal": "PAL"`), which the game's name carries on the desktop
    /// (`desktop::game_title`). Kept beside `titles`, keyed the same way; not sent to the UI.
    #[serde(skip)]
    pub videos: BTreeMap<String, String>,
    pub versions: Vec<Version>,
}

#[derive(Debug, Clone, Serialize, ts_rs::TS)]
#[ts(rename = "CatalogVersion")]
#[serde(rename_all = "camelCase")]
pub struct Version {
    /// `rac1/pal`: the key used everywhere (progress, actions, commit scopes).
    pub key: String,
    pub game: String,
    /// `pal`, `ntsc`: the directory under `games/<game>/`.
    pub name: String,
    /// The game's title in this region.
    pub title: String,
    pub region: String,
    pub serial: String,
    /// The project's directory, relative to the checkout (`games/rac1`).
    pub dir: String,
    pub disc: Option<FileSpec>,
    pub boot: Option<FileSpec>,
    /// What `openrac.py setup` places for this version.
    pub inputs: Vec<Input>,
    /// What the version's build reproduces, in one sentence.
    pub target: Option<String>,
    /// The manifest's own next steps after `openrac.py setup`, as prose.
    pub setup: Option<String>,
    /// Which machines the build runs on, as prose.
    pub build_host: Option<String>,
    /// The version's README, relative to the checkout.
    pub readme: Option<String>,
    pub source: Option<Source>,
    pub progress: Option<Progress>,
}

/// A file game.json describes by name and checksums (the disc image, the boot executable).
#[derive(Debug, Clone, Serialize, Deserialize, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct FileSpec {
    pub file: String,
    #[serde(default)]
    pub size: Option<u64>,
    #[serde(default)]
    pub sha1: Option<String>,
}

/// One input `openrac.py setup` places: `from` is `disc`, `boot`,
/// `toolchains` or `link` (then `target` names what it links to).
#[derive(Debug, Clone, Serialize, Deserialize, ts_rs::TS)]
#[ts(rename = "VersionInput")]
#[serde(rename_all = "camelCase")]
pub struct Input {
    pub from: String,
    pub path: String,
    #[serde(default)]
    pub target: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize, ts_rs::TS)]
#[ts(rename = "VersionSource")]
#[serde(rename_all = "camelCase")]
pub struct Source {
    pub name: String,
    pub repo: String,
    #[serde(default)]
    pub license: Option<String>,
    #[serde(default)]
    pub commit: Option<String>,
    #[serde(default)]
    pub date: Option<String>,
}

/// One row of progress/summary.json (snake_case on disk, camelCase to the UI).
#[derive(Debug, Clone, Serialize, Deserialize, ts_rs::TS)]
#[ts(rename_all = "camelCase")]
#[serde(rename_all(serialize = "camelCase", deserialize = "snake_case"))]
pub struct Progress {
    pub matched_code: u64,
    pub total_code: u64,
    pub matched_functions: Option<u64>,
    pub total_functions: Option<u64>,
    pub fuzzy_percent: Option<f64>,
    pub percent: f64,
    pub date: Option<String>,
    /// What the project counts, from game.json's progress note.
    #[serde(default)]
    pub note: Option<String>,
    /// Functional C: functions that do what retail's do without matching its bytes (ByteMatch's
    /// figure from the project's functional list). Never part of the matched numbers.
    #[serde(default)]
    pub functional: Option<FunctionalProgress>,
}

/// How much of the code has functional C (not matched; ByteMatch shows it under the Code bar).
#[derive(Debug, Clone, Serialize, Deserialize, ts_rs::TS)]
#[ts(rename_all = "camelCase")]
#[serde(rename_all(serialize = "camelCase", deserialize = "snake_case"))]
pub struct FunctionalProgress {
    pub functions: u64,
    pub code: u64,
    pub percent: f64,
}

// --- game.json as it is on disk -------------------------------------------------

#[derive(Deserialize)]
struct RawGame {
    id: String,
    title: String,
    #[serde(default)]
    year: Option<u32>,
    #[serde(default)]
    titles: BTreeMap<String, String>,
    /// In the manifest's order (serde_json's preserve_order), so a game's
    /// first version stays first: rac1's PAL, the project OpenRAC began with.
    #[serde(default)]
    versions: serde_json::Map<String, serde_json::Value>,
}

#[derive(Deserialize)]
struct RawVersion {
    /// Where the version's tree lives, when not games/<game>/<version> (rac1's PAL tree is the
    /// merged games/rac1 itself).
    #[serde(default)]
    dir: Option<String>,
    #[serde(default)]
    region: String,
    #[serde(default)]
    serial: String,
    #[serde(default)]
    disc: Option<FileSpec>,
    #[serde(default)]
    boot: Option<FileSpec>,
    #[serde(default)]
    inputs: Vec<Input>,
    #[serde(default)]
    target: Option<String>,
    #[serde(default)]
    setup: Option<String>,
    #[serde(default)]
    build: Option<RawBuild>,
    #[serde(default)]
    source: Option<Source>,
}

#[derive(Deserialize)]
struct RawBuild {
    #[serde(default)]
    host: Option<String>,
    #[serde(default)]
    readme: Option<String>,
}

#[derive(Deserialize, Debug)]
struct RawSummaryRow {
    /// None for a version with no report yet: one such row must not make the whole file unread.
    #[serde(default)]
    progress: Option<Progress>,
    #[serde(default)]
    note: Option<String>,
}

/// Reads the catalogue from the checkout at `root`. A game.json that does
/// not parse is an error (CI keeps them valid); a missing or unreadable
/// progress/summary.json only leaves the progress out.
pub fn load(root: &Path) -> Result<Catalog, String> {
    load_with(root, None)
}

/// The catalogue from the checkout at `root`, with the launcher's own copy of the figures
/// from bytematch.dev in `cache` (the file the launcher keeps them in, so that it can show
/// them without a network; none when it keeps no copy).
pub fn load_with(root: &Path, cache: Option<&Path>) -> Result<Catalog, String> {
    let games_dir = root.join("games");
    let mut manifests: Vec<_> = std::fs::read_dir(&games_dir)
        .map_err(|e| format!("cannot read {}: {e}", games_dir.display()))?
        .filter_map(|entry| entry.ok())
        .map(|entry| entry.path().join("game.json"))
        .filter(|path| path.is_file())
        .collect();
    manifests.sort();

    let summary = read_summary(root, cache);
    let mut games = Vec::new();
    for path in manifests {
        let text = std::fs::read_to_string(&path).map_err(|e| format!("cannot read {}: {e}", path.display()))?;
        let raw: RawGame = serde_json::from_str(&text).map_err(|e| format!("{}: {e}", path.display()))?;
        games.push(game(raw, &summary).map_err(|e| format!("{}: {e}", path.display()))?);
    }
    Ok(Catalog { games })
}

/// The rows of a progress file: the ones that can be read. A row that is malformed (a figure
/// of the wrong kind) is left out, not the whole file: one bad row must not take every game's
/// figures away.
fn read_rows(path: &Path) -> BTreeMap<String, RawSummaryRow> {
    let Some(text) = std::fs::read_to_string(path).ok() else { return BTreeMap::new() };
    let Some(json) = serde_json::from_str::<serde_json::Value>(&text).ok() else { return BTreeMap::new() };
    let Some(rows) = json.as_object() else { return BTreeMap::new() };
    rows.iter()
        .filter_map(|(key, value)| {
            serde_json::from_value::<RawSummaryRow>(value.clone()).ok().map(|row| (key.clone(), row))
        })
        .collect()
}

/// The checkout's own figures, with bytematch.dev's in their place wherever the site has a
/// version: the site is what every project's CI last reported, so it is the current figure.
fn read_summary(root: &Path, cache: Option<&Path>) -> BTreeMap<String, RawSummaryRow> {
    let mut rows = read_rows(&root.join("progress").join("summary.json"));
    let Some(cache) = cache else { return rows };
    for (key, site) in read_rows(cache) {
        match rows.get_mut(&key) {
            Some(own) => {
                if site.progress.is_some() {
                    own.progress = site.progress;
                }
            }
            None => {
                rows.insert(key, site);
            }
        }
    }
    rows
}

fn game(raw: RawGame, summary: &BTreeMap<String, RawSummaryRow>) -> Result<Game, String> {
    let mut versions = Vec::new();
    let mut videos = BTreeMap::new();
    for (name, value) in raw.versions {
        if let Some(video) = value.pointer("/disc/video").and_then(serde_json::Value::as_str) {
            videos.insert(name.clone(), video.to_string());
        }
        let v: RawVersion = serde_json::from_value(value).map_err(|e| format!("version {name}: {e}"))?;
        versions.push({
            let key = format!("{}/{}", raw.id, name);
            let progress = summary
                .get(&key)
                .and_then(|row| row.progress.clone().map(|p| Progress { note: row.note.clone(), ..p }));
            Version {
                title: raw.titles.get(&name).cloned().unwrap_or_else(|| raw.title.clone()),
                dir: v.dir.clone().unwrap_or_else(|| format!("games/{}/{}", raw.id, name)),
                game: raw.id.clone(),
                region: v.region,
                serial: v.serial,
                disc: v.disc,
                boot: v.boot,
                inputs: v.inputs,
                target: v.target,
                setup: v.setup,
                build_host: v.build.as_ref().and_then(|b| b.host.clone()),
                readme: v.build.and_then(|b| b.readme),
                source: v.source,
                progress,
                key,
                name,
            }
        });
    }
    Ok(Game { id: raw.id, title: raw.title, year: raw.year, titles: raw.titles, videos, versions })
}

/// One project of bytematch.dev's /api/v1/projects: where it is, the version its figures are for,
/// when, and its measures (objdiff's names).
#[derive(Deserialize)]
struct ByteMatchProject {
    owner: String,
    repo: String,
    #[serde(default)]
    version: Option<String>,
    #[serde(default)]
    timestamp: Option<String>,
    #[serde(default)]
    measures: Option<ByteMatchMeasures>,
}

#[derive(Deserialize)]
struct ByteMatchMeasures {
    #[serde(default)]
    matched_code: u64,
    #[serde(default)]
    total_code: u64,
    #[serde(default)]
    matched_functions: Option<u64>,
    #[serde(default)]
    total_functions: Option<u64>,
    #[serde(default)]
    fuzzy_match_percent: Option<f64>,
    #[serde(default)]
    matched_code_percent: f64,
}

/// The functional summary in a project's own JSON (bytematch.dev/{owner}/{repo}.json).
#[derive(Deserialize)]
struct ByteMatchProjectJson {
    #[serde(default)]
    functional: Option<ByteMatchFunctional>,
}

#[derive(Deserialize)]
struct ByteMatchFunctional {
    #[serde(default)]
    functions: u64,
    #[serde(default)]
    code: u64,
    #[serde(default)]
    code_percent: f64,
}

/// Where the website publishes every project's progress (ByteMatch, bytematch.dev).
pub const BYTEMATCH_PROJECTS: &str = "https://bytematch.dev/api/v1/projects?per_page=500";

/// A project's own JSON on bytematch.dev, which also carries its functional C summary.
pub fn bytematch_project_url(slug: &str) -> String {
    format!("https://bytematch.dev/{slug}.json")
}

/// The game versions of the checkout and the bytematch.dev project each takes from PROJECTS_JSON
/// (the one game.json names, else the one project whose version is the disc's serial):
/// [(version key, "owner/repo")].
pub fn bytematch_projects_for(root: &Path, projects_json: &str) -> Result<Vec<(String, String)>, String> {
    let projects: Vec<ByteMatchProject> =
        serde_json::from_str(projects_json).map_err(|e| format!("invalid project list from bytematch.dev: {e}"))?;
    let mut out = Vec::new();
    let games_dir = root.join("games");
    let manifests = std::fs::read_dir(&games_dir).map_err(|e| format!("cannot read {}: {e}", games_dir.display()))?;
    for entry in manifests.filter_map(|e| e.ok()) {
        let Ok(text) = std::fs::read_to_string(entry.path().join("game.json")) else { continue };
        let Ok(raw) = serde_json::from_str::<serde_json::Value>(&text) else { continue };
        let (Some(id), Some(versions)) = (raw["id"].as_str(), raw["versions"].as_object()) else { continue };
        for (name, version) in versions {
            let named = version["bytematch"].as_str();
            let serial = version["serial"].as_str().unwrap_or_default();
            let mut found = projects.iter().filter(|p| match named {
                Some(slug) => format!("{}/{}", p.owner, p.repo).eq_ignore_ascii_case(slug),
                None => !serial.is_empty() && p.version.as_deref() == Some(serial),
            });
            if let (Some(project), None) = (found.next(), found.next()) {
                out.push((format!("{id}/{name}"), format!("{}/{}", project.owner, project.repo)));
            }
        }
    }
    Ok(out)
}

/// Keeps the latest figures from bytematch.dev's project list, with the functional C summary of
/// each project's own JSON in PROJECT_JSONS ("owner/repo" -> its JSON), in `cache` (the file
/// the launcher keeps them in, for the next start without a network), and returns the reloaded
/// Catalog. A version takes the project bytematch_projects_for gives it. The checkout's
/// progress/summary.json is read, never written.
pub fn update_progress_from_bytematch(
    root: &Path,
    cache: &Path,
    projects_json: &str,
    project_jsons: &BTreeMap<String, String>,
) -> Result<Catalog, String> {
    let projects: Vec<ByteMatchProject> =
        serde_json::from_str(projects_json).map_err(|e| format!("invalid project list from bytematch.dev: {e}"))?;
    let mut rows = serde_json::Map::new();
    for (key, slug) in bytematch_projects_for(root, projects_json)? {
        let Some(project) = projects.iter().find(|p| format!("{}/{}", p.owner, p.repo) == slug) else { continue };
        let Some(m) = &project.measures else { continue };
        let functional = project_jsons
            .get(&slug)
            .and_then(|text| serde_json::from_str::<ByteMatchProjectJson>(text).ok())
            .and_then(|p| p.functional)
            .map(|f| serde_json::json!({"functions": f.functions, "code": f.code, "percent": f.code_percent}));
        rows.insert(
            key,
            serde_json::json!({
                "progress": {
                    "matched_code": m.matched_code,
                    "total_code": m.total_code,
                    "matched_functions": m.matched_functions,
                    "total_functions": m.total_functions,
                    "fuzzy_percent": m.fuzzy_match_percent,
                    "percent": m.matched_code_percent,
                    "date": project.timestamp.as_deref().map(|t| t.get(..10).unwrap_or(t)),
                    "functional": functional,
                },
                "source": format!("bytematch.dev/{slug}"),
            }),
        );
    }
    if let Some(dir) = cache.parent() {
        let _ = std::fs::create_dir_all(dir);
    }
    let formatted = serde_json::to_string_pretty(&rows).map_err(|e| e.to_string())?;
    std::fs::write(cache, formatted).map_err(|e| format!("cannot write {}: {e}", cache.display()))?;
    load_with(root, Some(cache))
}

impl Catalog {
    pub fn version(&self, key: &str) -> Option<&Version> {
        self.games.iter().flat_map(|g| &g.versions).find(|v| v.key == key)
    }

    pub fn version_keys(&self) -> Vec<String> {
        self.games.iter().flat_map(|g| &g.versions).map(|v| v.key.clone()).collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    #[test]
    fn reads_every_game_in_the_checkout() {
        let catalog = load(&checkout()).expect("the checkout's game.json files parse");
        let ids: Vec<_> = catalog.games.iter().map(|g| g.id.as_str()).collect();
        assert_eq!(ids, ["rac1", "rac2", "rac3", "rac4", "rac5", "rac6"]);
        // In game.json's order: rac1's PAL first.
        assert_eq!(
            catalog.version_keys(),
            ["rac1/pal", "rac1/ntsc", "rac2/ntsc", "rac3/ntsc", "rac4/ntsc", "rac5/ntsc", "rac6/ntsc"]
        );
        let pal = catalog.version("rac1/pal").unwrap();
        assert_eq!(pal.serial, "SCES_509.16");
        assert_eq!(pal.dir, "games/rac1");
        assert!(pal.disc.as_ref().is_some_and(|d| d.size.is_some() && d.sha1.is_some()));
    }

    #[test]
    fn regional_titles_and_progress_are_attached() {
        let dir = tempfile::tempdir().unwrap();
        let game = dir.path().join("games").join("rac9");
        std::fs::create_dir_all(&game).unwrap();
        std::fs::write(
            game.join("game.json"),
            r#"{"id": "rac9", "title": "Nine", "year": 2009, "titles": {"pal": "Nine (EU)"},
                "versions": {"pal": {"region": "PAL", "serial": "SCES_000.00", "extra": [1, 2]},
                             "ntsc": {"region": "NTSC-U", "serial": "SCUS_000.00"}}}"#,
        )
        .unwrap();
        std::fs::create_dir_all(dir.path().join("progress")).unwrap();
        std::fs::write(
            dir.path().join("progress").join("summary.json"),
            r#"{"rac9/pal": {"title": "Nine", "progress": {"matched_code": 5, "total_code": 10,
                "matched_functions": null, "total_functions": null, "fuzzy_percent": null,
                "date": "2026-10-08", "percent": 50.0}, "note": "all of it"},
                "rac9/ntsc": {"title": "Nine", "progress": null, "note": "no report yet"}}"#,
        )
        .unwrap();
        let catalog = load(dir.path()).unwrap();
        let pal = catalog.version("rac9/pal").unwrap();
        assert_eq!(pal.title, "Nine (EU)");
        assert_eq!(pal.progress.as_ref().unwrap().percent, 50.0);
        assert_eq!(pal.progress.as_ref().unwrap().note.as_deref(), Some("all of it"));
        let ntsc = catalog.version("rac9/ntsc").unwrap();
        assert_eq!(ntsc.title, "Nine");
        assert!(ntsc.progress.is_none());
    }

    #[test]
    fn a_malformed_row_leaves_the_others_readable() {
        let dir = tempfile::tempdir().unwrap();
        let summary = dir.path().join("summary.json");
        // One row with a figure of the wrong kind, and one that is right.
        std::fs::write(
            &summary,
            r#"{"rac1/pal": {"progress": {"matched_code": 5, "total_code": 10, "percent": 50.0}},
                "rac1/ntsc": {"progress": {"percent": "86%"}}}"#,
        )
        .unwrap();
        let rows = read_rows(&summary);
        assert_eq!(rows.len(), 1, "the row that can be read is kept: {rows:?}");
        assert_eq!(rows["rac1/pal"].progress.as_ref().unwrap().percent, 50.0);
    }

    #[test]
    fn updates_progress_from_bytematch_and_caches_offline() {
        let dir = tempfile::tempdir().unwrap();
        let game = dir.path().join("games").join("rac1");
        std::fs::create_dir_all(&game).unwrap();
        std::fs::write(
            game.join("game.json"),
            r#"{"id": "rac1", "title": "Ratchet & Clank", "versions": {
                "pal": {"region": "PAL", "serial": "SCES_509.16"},
                "ntsc": {"region": "NTSC-U", "serial": "SCUS_971.99", "bytematch": "lombyte-project/Lombyte"}}}"#,
        )
        .unwrap();
        std::fs::create_dir_all(dir.path().join("progress")).unwrap();
        std::fs::write(
            dir.path().join("progress").join("summary.json"),
            r#"{"rac1/pal": {"progress": {"matched_code": 99, "total_code": 100, "percent": 99.0}}}"#,
        )
        .unwrap();

        let site_json = r#"[
            {"owner": "OpenRAC", "repo": "rac1-decomp", "version": "SCES_509.16", "timestamp": "2026-10-16T01:00:00.000Z",
             "measures": {"matched_code": 3199668, "total_code": 3712808, "matched_functions": 4595,
                          "total_functions": 5109, "fuzzy_match_percent": 88.9, "matched_code_percent": 86.18}},
            {"owner": "lombyte-project", "repo": "Lombyte", "version": "SCUS_971.99",
             "measures": {"matched_code": 3300000, "total_code": 3677000, "matched_code_percent": 89.75}},
            {"owner": "someone", "repo": "fork", "version": "SCUS_971.99",
             "measures": {"matched_code": 1, "total_code": 2, "matched_code_percent": 50.0}}
        ]"#;

        let mut jsons = BTreeMap::new();
        jsons.insert(
            "OpenRAC/rac1-decomp".to_string(),
            r#"{"functional": {"code": 370968, "code_percent": 9.99, "functions": 389}}"#.to_string(),
        );
        // The launcher's own folder, beside its settings: where it can always write, unlike
        // a checkout it does not own (the copy an installed launcher carries).
        let mine = tempfile::tempdir().unwrap();
        let cache = mine.path().join("progress.json");
        let catalog = update_progress_from_bytematch(dir.path(), &cache, site_json, &jsons).unwrap();
        // By serial, and the site's figure wins over the checkout's.
        let pal = catalog.version("rac1/pal").unwrap().progress.as_ref().unwrap();
        assert_eq!(pal.percent, 86.18);
        assert_eq!(pal.matched_functions, Some(4595));
        assert_eq!(pal.date.as_deref(), Some("2026-10-16"));
        let functional = pal.functional.as_ref().unwrap();
        assert_eq!((functional.functions, functional.code, functional.percent), (389, 370968, 9.99));
        // By the project game.json names, though another project has the same serial.
        assert_eq!(catalog.version("rac1/ntsc").unwrap().progress.as_ref().unwrap().percent, 89.75);

        // The checkout's own file is not changed, and nothing is written in the checkout;
        // offline, the kept copy is read again.
        assert!(std::fs::read_to_string(dir.path().join("progress").join("summary.json")).unwrap().contains("99.0"));
        assert!(!dir.path().join("build").exists(), "the copy is not written in the checkout");
        let offline = load_with(dir.path(), Some(&cache)).unwrap();
        assert_eq!(offline.version("rac1/pal").unwrap().progress.as_ref().unwrap().percent, 86.18);
    }
}
