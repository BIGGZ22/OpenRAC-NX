//! For developers only: when the games folder holds more than one disc of a game (PAL and NTSC-U),
//! which of them the game's page plays. A player never sees this: without developer tools on, or
//! with one disc of a game, nothing here applies and the launcher picks the disc as it always has
//! (the one set up, else the first found; `playerVersion` in `src/lib/player.svelte.ts`).
//!
//! Each version keeps what is its own. The levels, the build, the mods, the memory card and the
//! logs already have a folder or a name per version; only what the set-up copies from the disc
//! (`iso_data/<game>`, and `decompiler_out/<game>` and `out/<game>` once there are any) is one
//! folder per game, which the port reads. Switching moves that folder aside, as
//! `<game>@<version>` beside it (`iso_data/rac1@pal`), and moves the chosen version's back, so
//! nothing is read from the disc again and nothing of the other version is deleted:
//!
//! ```text
//! <install dir>/active/rac1/data/
//!     iso_data/rac1/          the disc being played (what the port reads), NTSC-U here
//!     iso_data/rac1@pal/      PAL's set-up, kept for when PAL is chosen again
//! ```

use std::path::{Path, PathBuf};

use serde::Serialize;

use crate::catalog::Catalog;
use crate::config::Config;
use crate::discs::{self, FoundDisc};
use crate::install;

/// The folders of a game's set-up that hold one version at a time (OpenGOAL's layout).
const SET_UP: [&str; 3] = ["iso_data", "decompiler_out", "out"];

/// A game with more than one of its discs in the games folder, for a developer to pick from.
#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct DiscChoice {
    pub game: String,
    /// The versions found (`rac1/pal`, `rac1/ntsc`), in the catalogue's order.
    pub versions: Vec<String>,
    /// The one the game's page plays.
    pub chosen: String,
}

/// The version of `game` whose disc is set up now (its `buildinfo.json`).
fn set_up_version(config: &Config, game: &str) -> Option<String> {
    let dir = config.install_dir.as_deref()?;
    install::state(dir, game).extracted.map(|b| b.version)
}

/// The games a developer can switch discs for: developer tools on, and at least two versions of
/// the game among the discs found. The choice remembered in the settings when it is still found,
/// else the disc set up, else the first found: what the launcher plays without a choice.
pub fn choices(config: &Config, catalog: &Catalog, found: &[FoundDisc]) -> Vec<DiscChoice> {
    if !config.developer {
        return Vec::new();
    }
    let mut out = Vec::new();
    for game in &catalog.games {
        let versions: Vec<String> =
            game.versions.iter().filter(|v| found.iter().any(|f| f.version == v.key)).map(|v| v.key.clone()).collect();
        if versions.len() < 2 {
            continue;
        }
        let pick = |key: Option<&str>| key.filter(|k| versions.iter().any(|v| v == k)).map(str::to_string);
        let chosen = pick(config.disc_choice.get(&game.id).map(String::as_str))
            .or_else(|| pick(set_up_version(config, &game.id).as_deref()))
            .unwrap_or_else(|| versions[0].clone());
        out.push(DiscChoice { game: game.id.clone(), versions, chosen });
    }
    out
}

/// Plays `version`'s disc for its game from now on: refused unless that choice is offered
/// ([`choices`]). Puts that version's set-up in place (switch) and remembers the choice in
/// `config`, which the caller saves.
pub fn choose(config: &mut Config, catalog: &Catalog, found: &[FoundDisc], version: &str) -> Result<(), String> {
    let v = catalog.version(version).ok_or_else(|| format!("no version {version}"))?;
    let offered = choices(config, catalog, found);
    if !offered.iter().any(|c| c.game == v.game && c.versions.iter().any(|k| k == version)) {
        return Err(format!("{version} cannot be chosen: developer tools are off or its disc is not found"));
    }
    if let Some(install) = config.install_dir.as_deref() {
        switch(install, &v.game, version)?;
    }
    config.disc_choice.insert(v.game.clone(), version.to_string());
    Ok(())
}

/// Where `version`'s set-up of `game` waits while another is played: `<sub>/<game>@<name>`.
pub fn kept(install: &Path, sub: &str, game: &str, version: &str) -> PathBuf {
    let name = version.rsplit('/').next().unwrap_or(version);
    install::data_dir(install, game).join(sub).join(format!("{game}@{name}"))
}

/// Makes `version` the set-up of `game` that is played: the one in place now moves aside
/// ([`kept`]) and `version`'s, if it was set up before, comes back. Nothing is deleted or read
/// again; a version never set up is left to set up (the folder is then free).
pub fn switch(install: &Path, game: &str, version: &str) -> Result<(), String> {
    let data = install::data_dir(install, game);
    let current = install::state(install, game).extracted.map(|b| b.version);
    if current.as_deref() == Some(version) {
        return Ok(());
    }
    let active = |sub: &str| data.join(sub).join(game);
    if SET_UP.iter().any(|sub| active(sub).exists()) {
        let Some(current) = current else {
            return Err(format!("the set-up in {} is not complete: remove the set-up first", data.display()));
        };
        for sub in SET_UP {
            let to = kept(install, sub, game, &current);
            if active(sub).exists() && to.exists() {
                return Err(format!("{} is already there: move it away first", to.display()));
            }
        }
        for sub in SET_UP {
            if active(sub).exists() {
                let to = kept(install, sub, game, &current);
                std::fs::rename(active(sub), &to).map_err(|e| format!("cannot move {}: {e}", active(sub).display()))?;
            }
        }
    }
    for sub in SET_UP {
        let from = kept(install, sub, game, version);
        if from.exists() {
            std::fs::rename(&from, active(sub)).map_err(|e| format!("cannot move {}: {e}", from.display()))?;
        }
    }
    Ok(())
}

/// Before a developer sets `game` up from `image`: when another version's disc is set up, it is
/// kept aside ([`switch`]) instead of being replaced by the extractor. Off for players: their
/// set-up replaces the one before, as it always has.
pub fn before_set_up(config: &Config, catalog: &Catalog, game: &str, image: &Path) -> Result<(), String> {
    let Some(install) = config.install_dir.as_deref().filter(|_| config.developer) else { return Ok(()) };
    let Some(serial) = discs::serial_of(image) else { return Ok(()) };
    let Some(version) = catalog.games.iter().flat_map(|g| &g.versions).find(|v| v.serial == serial && v.game == game)
    else {
        return Ok(());
    };
    match install::state(install, game).extracted {
        Some(b) if b.version != version.key => switch(install, game, &version.key),
        _ => Ok(()),
    }
}

