//! The OpenRAC launcher's logic, without a window.
//!
//! Everything here reads the OpenRAC checkout the user points the launcher
//! at, or runs its tools; nothing is hard-coded about a game that its
//! `games/<game>/game.json` already says. The desktop app (`src-tauri/`)
//! exposes these functions as Tauri commands; this crate builds and tests on
//! any machine, without WebKit or a display.
//!
//! - [`catalog`]: the games and versions, from `games/*/game.json` and
//!   `progress/summary.json`.
//! - [`actions`]: what the launcher can run for each version, from
//!   `launcher/actions.json`, and how an action becomes a command line.
//! - [`status`]: what is in place for a version (disc, inputs, build output).
//! - [`config`]: the launcher's own settings file.
//! - [`detect`]: finding the checkout, Python, Godot and Docker.
//! - [`install`]: setting a game up from the player's disc, OpenGOAL's way.
//! - [`jobs`]: running an action as a child process and streaming its output.
//! - [`overrides`]: a game's overrides packs (replacement textures and models): list, order, on/off.
//! - [`library`]: all of the above, put together for the UI.

pub mod actions;
pub mod catalog;
pub mod config;
pub mod detect;
pub mod discord;
pub mod discs;
pub mod install;
pub mod jobs;
pub mod library;
pub mod overrides;
pub mod saves;
pub mod status;

use std::path::{Path, PathBuf};

/// Whether `dir` looks like an OpenRAC checkout: it has `games/*/game.json`
/// and `tools/openrac.py`.
pub fn is_openrac_root(dir: &Path) -> bool {
    dir.join("tools").join("openrac.py").is_file() && dir.join("games").is_dir()
}

/// `rel` (a path with `/` separators, relative to the checkout) under `root`.
pub fn under(root: &Path, rel: &str) -> PathBuf {
    rel.split('/')
        .filter(|part| !part.is_empty() && *part != ".")
        .fold(root.to_path_buf(), |path, part| path.join(part))
}

#[cfg(test)]
pub(crate) mod testing {
    use std::path::PathBuf;

    /// The OpenRAC checkout this crate is part of (launcher/core/../..).
    pub fn checkout() -> PathBuf {
        PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("..").join("..")
    }
}
