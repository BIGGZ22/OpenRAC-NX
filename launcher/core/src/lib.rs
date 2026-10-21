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
//! - [`disc_choice`]: for developers with two discs of a game, which one is played.
//! - [`differences`]: what the port does otherwise than the PS2 game, and the fixes a player switches back.
//! - [`jobs`]: running an action as a child process and streaming its output.
//! - [`overrides`]: a game's overrides packs (replacement textures and models): list, order, on/off.
//! - [`plugins`]: a game's native mods (docs/port/MODS.md): install, list, order, on/off, settings.
//! - [`library`]: all of the above, put together for the UI.
//! - [`reports`]: a game's session logs and crash reports, the text and the .zip players send.
//! - [`launcher_log`]: the launcher's own log, which goes in that .zip.
//! - [`update`]: the launcher's own updates on Android: the manifest and the signature check.

pub mod actions;
pub mod android;
pub mod catalog;
pub mod chd;
pub mod config;
pub mod detect;
pub mod differences;
pub mod disc_choice;
pub mod discord;
pub mod discs;
pub mod install;
pub mod jobs;
pub mod launcher_log;
pub mod library;
pub mod overrides;
pub mod plugins;
pub mod port_build;
pub mod reports;
pub mod save_edit;
pub mod saves;
pub mod status;
pub mod update;

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

/// The program a build made at `path` (absolute, without `.exe` off Windows). On macOS a windowed
/// program is an application bundle named by its title, in a folder named after the program:
/// `<folder>/apps/<name>/<title>.app` (port/cmake/MacApp.cmake). The program is the bundle's
/// `CFBundleExecutable` in `Contents/MacOS`, and started from there the Dock, Cmd-Tab and Activity
/// Monitor show the game's name and icon. Elsewhere, or with no bundle, `path` itself.
pub fn program_path(path: PathBuf) -> PathBuf {
    if !cfg!(target_os = "macos") {
        return path;
    }
    let (Some(folder), Some(name)) = (path.parent(), path.file_name()) else { return path };
    let Ok(entries) = std::fs::read_dir(folder.join("apps").join(name)) else { return path };
    let mut bundles: Vec<PathBuf> = entries
        .filter_map(|entry| entry.ok().map(|e| e.path()))
        .filter(|p| p.extension().is_some_and(|ext| ext == "app") && p.is_dir())
        .collect();
    bundles.sort();
    bundles
        .iter()
        .find_map(|bundle| bundle_executable(bundle).map(|exe| bundle.join("Contents").join("MacOS").join(exe)))
        .unwrap_or(path)
}

/// The `CFBundleExecutable` of the application bundle `bundle`, from its `Contents/Info.plist`
/// (the XML form the port's build writes), when it is there.
fn bundle_executable(bundle: &Path) -> Option<String> {
    let plist = std::fs::read_to_string(bundle.join("Contents").join("Info.plist")).ok()?;
    let after = &plist[plist.find("<key>CFBundleExecutable</key>")?..];
    let start = after.find("<string>")? + "<string>".len();
    let end = start + after[start..].find("</string>")?;
    let name = after[start..end]
        .replace("&lt;", "<")
        .replace("&gt;", ">")
        .replace("&quot;", "\"")
        .replace("&apos;", "'")
        .replace("&amp;", "&");
    let program = bundle.join("Contents").join("MacOS").join(&name);
    (!name.is_empty() && !name.contains('/') && program.is_file()).then_some(name)
}

#[cfg(test)]
mod program_tests {
    use super::*;

    #[test]
    fn a_bundle_beside_the_build_is_the_program_on_macos() {
        let dir = tempfile::tempdir().unwrap();
        let plain = dir.path().join("openrac-rac1-pal");
        let bundle = dir.path().join("apps/openrac-rac1-pal/Ratchet & Clank.app");
        let macos = bundle.join("Contents").join("MacOS");
        std::fs::create_dir_all(&macos).unwrap();
        std::fs::write(macos.join("Ratchet & Clank"), b"").unwrap();
        std::fs::write(
            bundle.join("Contents/Info.plist"),
            "<plist><dict><key>CFBundleName</key><string>x</string>\n\
             <key>CFBundleExecutable</key>\n\t<string>Ratchet &amp; Clank</string></dict></plist>",
        )
        .unwrap();
        let expected = if cfg!(target_os = "macos") { macos.join("Ratchet & Clank") } else { plain.clone() };
        assert_eq!(program_path(plain), expected);
    }

    #[test]
    fn without_a_bundle_the_path_is_the_program() {
        let dir = tempfile::tempdir().unwrap();
        let plain = dir.path().join("openrac-rac1-pal");
        assert_eq!(program_path(plain.clone()), plain);
        // A bundle naming a program it does not hold is not used.
        let bundle = dir.path().join("apps/openrac-rac1-pal/Ratchet & Clank.app");
        std::fs::create_dir_all(bundle.join("Contents")).unwrap();
        std::fs::write(bundle.join("Contents/Info.plist"), "<key>CFBundleExecutable</key><string>gone</string>")
            .unwrap();
        assert_eq!(program_path(plain.clone()), plain);
    }
}

#[cfg(test)]
pub(crate) mod testing {
    use std::path::PathBuf;

    /// The OpenRAC checkout this crate is part of (launcher/core/../..).
    pub fn checkout() -> PathBuf {
        PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("..").join("..")
    }
}
