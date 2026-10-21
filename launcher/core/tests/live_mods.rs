//! Installing mods from a link against the real GitHub, end to end, as the Mods tab does it: the
//! public OpenRAC/rac1-mods, its lock-strafe release and its script mod. Needs the network, so it is
//! run by hand: `cargo test -p openrac-launcher-core --test live_mods -- --ignored --nocapture`.
//!
//! The mods go where an installed launcher puts them, `<install>/active/rac1/data/plugins` under a
//! temporary install folder; nothing of a checkout is read.

use std::path::Path;

use openrac_launcher_core::plugins;
use openrac_launcher_core::remote::{self, Context, Curl, Hosts};

const REPO: &str = "https://github.com/OpenRAC/rac1-mods";

fn context(dir: &Path) -> Context<'_> {
    Context {
        dir,
        game: "rac1",
        version: "pal",
        platform: plugins::platform_key(),
        fetch: &Curl,
        hosts: Hosts::default(),
    }
}

fn state(dir: &Path, id: &str) -> Option<(bool, String, String)> {
    let list = plugins::list(dir, "rac1", "pal").unwrap();
    let m = list.into_iter().find(|m| m.id == id)?;
    let v = serde_json::to_value(&m).unwrap();
    Some((
        m.enabled,
        v["state"].as_str().unwrap_or_default().to_string(),
        v["kind"].as_str().unwrap_or_default().to_string(),
    ))
}

#[test]
#[ignore = "network"]
fn install_from_link_end_to_end() {
    let install = tempfile::tempdir().unwrap();
    let dir = plugins::dir(&install.path().join("OpenRAC"), "rac1");
    let ctx = context(&dir);
    let platform = plugins::platform_key().expect("a desktop platform");

    // Paste the link: the mods, with what each is for and whether this computer can take it.
    let preview = remote::preview(&ctx, REPO).expect("the preview");
    for m in &preview.mods {
        eprintln!(
            "preview: {} {} ({}) compatible {} asset {:?} tag {:?} why {:?}",
            m.id, m.version, m.kind, m.compatible, m.asset, m.tag, m.why
        );
    }
    let lock = preview.mods.iter().find(|m| m.id == "lock-strafe").expect("lock-strafe listed");
    assert!(lock.compatible, "{lock:?}");
    assert_eq!(lock.asset.as_deref(), Some(remote::asset_name("lock-strafe", &lock.version, platform).as_str()));
    let swap = preview.mods.iter().find(|m| m.id == "button-swap").expect("button-swap listed");
    assert!(swap.compatible, "{swap:?}");

    // The native mod: this platform's archive from the release, checked against SHA256SUMS.
    let got = remote::install(&ctx, REPO, "lock-strafe").expect("lock-strafe installs");
    eprintln!("installed: {got:?}");
    let lib = dir.join("lock-strafe").join("lib").join(platform);
    assert!(lib.is_dir() && std::fs::read_dir(&lib).unwrap().count() == 1, "{}", lib.display());
    assert_eq!(state(&dir, "lock-strafe").map(|s| s.1), Some("ready".to_string()));

    // The item randomizer, released apart (item-randomizer-v1.0.0): its own archive for this
    // platform, checked against its release's SHA256SUMS.
    let rando = preview.mods.iter().find(|m| m.id == "item-randomizer").expect("item-randomizer listed");
    assert!(rando.compatible, "{rando:?}");
    assert_eq!(rando.tag.as_deref(), Some("item-randomizer-v1.0.0"));
    remote::install(&ctx, REPO, "item-randomizer").expect("item-randomizer installs");
    assert_eq!(state(&dir, "item-randomizer").map(|s| s.1), Some("ready".to_string()));

    // The script mod: no release of its own, so the repository's files at the release's tag.
    let got = remote::install(&ctx, REPO, "button-swap").expect("button-swap installs");
    eprintln!("installed: {got:?}");
    assert!(dir.join("button-swap").join("main.luau").is_file());
    let (_, st, kind) = state(&dir, "button-swap").unwrap();
    assert_eq!((st.as_str(), kind.as_str()), ("ready", "script"));

    // Switched off and on again; the list the port reads follows.
    plugins::set_enabled(&dir, "rac1", "pal", "lock-strafe", false).unwrap();
    assert_eq!(state(&dir, "lock-strafe").map(|s| s.0), Some(false));
    plugins::set_enabled(&dir, "rac1", "pal", "lock-strafe", true).unwrap();
    let list = std::fs::read_to_string(dir.join("plugins.json")).unwrap();
    eprintln!("plugins.json: {list}");
    assert!(list.contains("\"lock-strafe\"") && list.contains("\"button-swap\""));

    // Nothing newer than what was just installed.
    let (updates, errors) = remote::check_updates(&ctx);
    eprintln!("updates: {updates:?} errors: {errors:?}");
    assert!(errors.is_empty(), "{errors:?}");
    assert!(!updates.iter().any(|u| u.id == "lock-strafe"), "{updates:?}");
    // An older lock-strafe on record is offered the release; pinned, it is not.
    let mut sources = remote::read_sources(&dir);
    sources.mods.get_mut("lock-strafe").unwrap().version = "0.9.0".into();
    remote::write_sources(&dir, &sources).unwrap();
    let (updates, _) = remote::check_updates(&ctx);
    assert!(updates.iter().any(|u| u.id == "lock-strafe" && u.available == lock.version), "{updates:?}");
    remote::set_pin(&dir, "lock-strafe", true).unwrap();
    let (updates, _) = remote::check_updates(&ctx);
    assert!(!updates.iter().any(|u| u.id == "lock-strafe"), "{updates:?}");
    remote::set_pin(&dir, "lock-strafe", false).unwrap();
    let again = remote::update(&ctx, "lock-strafe").expect("the update installs");
    eprintln!("updated: {again:?}");

    // Removed: the folders, the list and the sources forget them.
    for id in ["lock-strafe", "button-swap", "item-randomizer"] {
        plugins::remove(&dir, "rac1", "pal", id).unwrap();
        remote::forget(&dir, id).unwrap();
        assert!(!dir.join(id).exists());
    }
    assert!(plugins::list(&dir, "rac1", "pal").unwrap().is_empty());
    assert!(remote::read_sources(&dir).mods.is_empty());
}
