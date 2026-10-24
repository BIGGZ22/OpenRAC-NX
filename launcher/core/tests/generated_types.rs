//! The TypeScript types of everything the Tauri commands send to or take from the page, generated
//! from the Rust types with ts-rs (docs: launcher/docs/ARCHITECTURE.md, "The page's types").
//!
//! The generated files are committed, in `launcher/src/lib/generated/`, because the page is
//! checked and built (`yarn verify`, `yarn build`) without a Rust toolchain. This test is the
//! check that they are fresh: it generates the types into a temporary folder and fails when
//! the committed files differ, extra or missing files included. After changing a type that is
//! sent to the page, regenerate with
//!
//! ```text
//! OPENRAC_UPDATE_TYPES=1 cargo test -p openrac-launcher-core --test generated_types
//! ```
//!
//! and commit the result. Never edit a generated file by hand.

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};

use openrac_launcher_core::{
    actions, config, detect, differences, disc_choice, discs, enhanced, jobs, library, overrides, plugins, remote,
    reports, save_edit, saves, update,
};
use ts_rs::{Config, TS};

/// Every type a command returns or takes, or an event carries; the types they contain are
/// generated with them. Add a type here when a command starts sending it to the page.
fn export_all(cfg: &Config) {
    macro_rules! roots {
        ($($ty:ty),* $(,)?) => { $( <$ty as TS>::export_all(cfg).unwrap(); )* };
    }
    roots!(
        actions::Scope,
        config::Config,
        detect::Tool,
        detect::Detected,
        detect::Check,
        differences::Differences,
        disc_choice::DiscChoice,
        discs::FoundDisc,
        enhanced::EnhancedView,
        jobs::Event,
        jobs::Started,
        library::Library,
        overrides::PackView,
        plugins::Installed,
        plugins::PluginView,
        plugins::Status,
        remote::Preview,
        remote::RemoteInstalled,
        remote::Sources,
        remote::Update,
        reports::ReportEntry,
        save_edit::SaveChange,
        save_edit::WriteResult,
        saves::GameSaveStatus,
        update::Channel,
        update::Release,
        update::UpdateSource,
    );
}

fn committed_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../src/lib/generated")
}

/// The `.ts` files under `dir` by file name, with line endings as git may have checked them out.
fn read_all(dir: &Path) -> BTreeMap<String, String> {
    let mut files = BTreeMap::new();
    let Ok(entries) = std::fs::read_dir(dir) else { return files };
    for entry in entries {
        let path = entry.unwrap().path();
        if path.extension().is_some_and(|e| e == "ts") {
            let text = std::fs::read_to_string(&path).unwrap().replace("\r\n", "\n");
            files.insert(path.file_name().unwrap().to_string_lossy().into_owned(), text);
        }
    }
    files
}

#[test]
fn generated_types_are_fresh() {
    // The page reads whole numbers as numbers: no id, size or time here goes past 2^53.
    let base = Config::new().with_large_int("number");
    let committed = committed_dir();
    if std::env::var_os("OPENRAC_UPDATE_TYPES").is_some() {
        let _ = std::fs::remove_dir_all(&committed);
        std::fs::create_dir_all(&committed).unwrap();
        export_all(&base.with_out_dir(&committed));
        return;
    }
    let temp = tempfile::tempdir().unwrap();
    export_all(&base.with_out_dir(temp.path()));
    let fresh = read_all(temp.path());
    let have = read_all(&committed);
    let stale: Vec<&String> = fresh.keys().filter(|name| have.get(*name) != fresh.get(*name)).collect();
    let extra: Vec<&String> = have.keys().filter(|name| !fresh.contains_key(*name)).collect();
    assert!(
        stale.is_empty() && extra.is_empty(),
        "launcher/src/lib/generated is out of date (changed or missing: {stale:?}; no longer generated: {extra:?}). \
         Regenerate: OPENRAC_UPDATE_TYPES=1 cargo test -p openrac-launcher-core --test generated_types"
    );
}
