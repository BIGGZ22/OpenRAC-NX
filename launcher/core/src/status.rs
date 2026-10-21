//! What is in place for a version: the disc image, the inputs
//! `openrac.py setup` places, the shared toolchains folder.
//!
//! These are cheap checks (names, sizes, links), run every time the library
//! is shown. They never hash a 4 GB image: `openrac.py discs` does that, and
//! the launcher runs it as the repository's "Identify discs" action. See
//! docs/INTEGRATION.md ("Disc verification") for connecting its result here.

use std::path::{Path, PathBuf};

use serde::Serialize;

use crate::catalog::Version;
use crate::under;

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct VersionStatus {
    pub disc: DiscStatus,
    /// The boot executable `openrac.py setup` reads out of the image, if the version uses it.
    pub boot: Option<FileStatus>,
    pub inputs: Vec<InputStatus>,
    /// Every input is in place (true when the version has none).
    pub inputs_ready: bool,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct DiscStatus {
    pub state: DiscState,
    /// The image found, absolute.
    pub path: Option<PathBuf>,
    /// The name game.json gives the image (`SCES_509.16.iso`).
    pub expected: Option<String>,
    pub message: String,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub enum DiscState {
    /// An image of the right size is in baserom/ (or placed in the game);
    /// its checksums are `openrac.py discs`'s job.
    Found,
    /// An image has the expected name but not the expected size.
    Mismatch,
    Missing,
    /// game.json names no disc for this version.
    Unknown,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct FileStatus {
    pub path: String,
    pub present: bool,
    pub message: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct InputStatus {
    /// `disc`, `boot`, `toolchains` or `link`, as in game.json.
    pub from: String,
    /// Relative to the checkout.
    pub path: String,
    pub present: bool,
    pub message: String,
}

/// Whether the shared toolchains/ folder holds a compiler: a folder of its own, not a file
/// left there (a .gitkeep, an archive) and not the README.
pub fn toolchains_present(root: &Path) -> bool {
    std::fs::read_dir(root.join("toolchains"))
        .map(|entries| {
            entries.filter_map(|e| e.ok()).any(|e| {
                let name = e.file_name().to_string_lossy().to_string();
                !name.starts_with('.') && name != "README.md" && e.path().is_dir()
            })
        })
        .unwrap_or(false)
}

pub fn version_status(root: &Path, version: &Version) -> VersionStatus {
    let disc = disc_status(root, version);
    let boot = version.inputs.iter().find(|i| i.from == "boot").map(|input| {
        let path = under(root, &input.path);
        let want = version.boot.as_ref().and_then(|b| b.size);
        let size = std::fs::metadata(&path).ok().map(|m| m.len());
        let (present, message) = match (size, want) {
            (None, _) => (false, "not placed yet: run Place inputs".to_string()),
            (Some(got), Some(want)) if got != want => (false, format!("{got} bytes, game.json says {want}")),
            _ => (true, "placed".to_string()),
        };
        FileStatus { path: input.path.clone(), present, message }
    });
    let inputs: Vec<_> = version.inputs.iter().map(|input| input_status(root, input)).collect();
    let inputs_ready = inputs.iter().all(|i| i.present);
    VersionStatus { disc, boot, inputs, inputs_ready }
}

fn disc_status(root: &Path, version: &Version) -> DiscStatus {
    let Some(spec) = &version.disc else {
        return DiscStatus {
            state: DiscState::Unknown,
            path: None,
            expected: None,
            message: "game.json names no disc".into(),
        };
    };
    let expected = Some(spec.file.clone());
    let size_ok = |path: &Path| match (std::fs::metadata(path), spec.size) {
        (Ok(meta), Some(want)) => Some(meta.len() == want),
        (Ok(_), None) => Some(true),
        (Err(_), _) => None,
    };

    // The places a disc is expected first: where setup put it, then baserom/<name>.
    let mut named: Vec<PathBuf> =
        version.inputs.iter().filter(|i| i.from == "disc").map(|i| under(root, &i.path)).collect();
    named.push(root.join("baserom").join(&spec.file));
    let mut mismatch = None;
    for path in &named {
        match size_ok(path) {
            Some(true) => {
                return DiscStatus {
                    state: DiscState::Found,
                    path: Some(path.clone()),
                    expected,
                    message: "found (size matches; Identify discs checks the checksums)".into(),
                }
            }
            Some(false) => mismatch = mismatch.or(Some(path.clone())),
            None => {}
        }
    }
    // openrac.py recognises a disc whatever its name: so does this, by size.
    if let (Some(want), Ok(entries)) = (spec.size, std::fs::read_dir(root.join("baserom"))) {
        for entry in entries.filter_map(|e| e.ok()) {
            let path = entry.path();
            let iso = path.extension().is_some_and(|e| e.eq_ignore_ascii_case("iso"));
            if iso && std::fs::metadata(&path).is_ok_and(|m| m.len() == want) {
                return DiscStatus {
                    state: DiscState::Found,
                    message: format!(
                        "found as {} (size matches; Identify discs checks the checksums)",
                        entry.file_name().to_string_lossy()
                    ),
                    path: Some(path),
                    expected,
                };
            }
        }
    }
    match mismatch {
        Some(path) => DiscStatus {
            state: DiscState::Mismatch,
            message: format!("{} is not the expected size: another version or a bad dump?", path.display()),
            path: Some(path),
            expected,
        },
        None => DiscStatus {
            state: DiscState::Missing,
            path: None,
            message: format!("put your own image in baserom/ (as {})", spec.file),
            expected,
        },
    }
}

fn input_status(root: &Path, input: &crate::catalog::Input) -> InputStatus {
    let path = under(root, &input.path);
    let link = std::fs::symlink_metadata(&path).is_ok_and(|m| m.file_type().is_symlink());
    let (present, message) = if path.exists() {
        (true, if link { "linked" } else { "in place" }.to_string())
    } else if link {
        let what = if input.from == "toolchains" { "toolchains/ (see toolchains/README.md)" } else { "its target" };
        (false, format!("linked, but {what} is missing"))
    } else {
        (false, "not placed yet: run Place inputs".to_string())
    };
    InputStatus { from: input.from.clone(), path: input.path.clone(), present, message }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::catalog::{FileSpec, Input, Version};

    fn version(inputs: Vec<Input>) -> Version {
        Version {
            key: "rac9/pal".into(),
            game: "rac9".into(),
            name: "pal".into(),
            title: "Nine".into(),
            region: "PAL".into(),
            serial: "SCES_000.00".into(),
            dir: "games/rac9/pal".into(),
            disc: Some(FileSpec { file: "SCES_000.00.iso".into(), size: Some(8), sha1: None }),
            boot: Some(FileSpec { file: "SCES_000.00".into(), size: Some(3), sha1: None }),
            inputs,
            target: None,
            setup: None,
            build_host: None,
            readme: None,
            source: None,
            progress: None,
        }
    }

    #[test]
    fn the_toolchains_are_there_when_a_compiler_is() {
        let dir = tempfile::tempdir().unwrap();
        let root = dir.path();
        std::fs::create_dir_all(root.join("toolchains")).unwrap();
        // Only the README: no compiler.
        std::fs::write(root.join("toolchains").join("README.md"), b"compilers you supply").unwrap();
        assert!(!toolchains_present(root));
        // A file left there is not a compiler either.
        std::fs::write(root.join("toolchains").join("sn-prodg.tar.gz"), b"x").unwrap();
        assert!(!toolchains_present(root));
        // A folder of a compiler is.
        std::fs::create_dir_all(root.join("toolchains").join("sn-prodg-3.01")).unwrap();
        assert!(toolchains_present(root));
    }

    #[test]
    fn finds_a_disc_by_name_or_by_size() {
        let dir = tempfile::tempdir().unwrap();
        let root = dir.path();
        std::fs::create_dir_all(root.join("baserom")).unwrap();
        let v = version(vec![]);
        assert_eq!(version_status(root, &v).disc.state, DiscState::Missing);

        std::fs::write(root.join("baserom").join("SCES_000.00.iso"), b"short").unwrap();
        assert_eq!(version_status(root, &v).disc.state, DiscState::Mismatch);

        std::fs::write(root.join("baserom").join("my copy.ISO"), b"12345678").unwrap();
        let status = version_status(root, &v);
        assert_eq!(status.disc.state, DiscState::Found);
        assert!(status.disc.message.contains("my copy.ISO"), "{}", status.disc.message);
    }

    #[test]
    fn reports_each_input() {
        let dir = tempfile::tempdir().unwrap();
        let root = dir.path();
        let v = version(vec![
            Input { from: "boot".into(), path: "games/rac9/pal/baserom/SCES_000.00".into(), target: None },
            Input { from: "toolchains".into(), path: "games/rac9/pal/toolchain".into(), target: None },
        ]);
        let status = version_status(root, &v);
        assert!(!status.inputs_ready);
        assert!(!status.boot.as_ref().unwrap().present);

        std::fs::create_dir_all(root.join("games/rac9/pal/baserom")).unwrap();
        std::fs::write(root.join("games/rac9/pal/baserom/SCES_000.00"), b"abc").unwrap();
        std::fs::create_dir_all(root.join("toolchains")).unwrap();
        std::fs::create_dir_all(root.join("games/rac9/pal/toolchain")).unwrap();
        let status = version_status(root, &v);
        assert!(status.boot.as_ref().unwrap().present);
        assert!(status.inputs_ready, "{:?}", status.inputs);
        assert!(!toolchains_present(root));
        std::fs::write(root.join("toolchains").join("README.md"), b"").unwrap();
        assert!(!toolchains_present(root));
        std::fs::create_dir_all(root.join("toolchains").join("sn")).unwrap();
        assert!(toolchains_present(root));
    }
}
