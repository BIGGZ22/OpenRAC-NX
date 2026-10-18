//! What the Android launcher does that a desktop's does not, without Android itself: the disc
//! image the player picks, copied into the app's own storage, and the game's library a play
//! action names (docs/port/ANDROID.md; the launcher's src-tauri/src/android.rs calls these).
use crate::actions::Plan;

/// A disc image's name as it is kept: the picker's name without folders, `disc.iso` if it has none.
pub fn disc_file_name(name: &str) -> String {
    let base = name.rsplit(['/', '\\']).next().unwrap_or("").trim();
    if base.is_empty() || base == "." || base == ".." {
        "disc.iso".to_string()
    } else {
        base.to_string()
    }
}

/// Copies `from` into `dest` through a file beside it, reporting every 64 MB; an image already
/// there with the same size is kept as it is.
pub fn copy_disc(
    mut from: impl std::io::Read,
    total: u64,
    dest: &std::path::Path,
    progress: impl Fn(u64),
) -> std::io::Result<()> {
    use std::io::Write;
    if total > 0 && dest.metadata().is_ok_and(|m| m.len() == total) {
        progress(total);
        return Ok(());
    }
    if let Some(dir) = dest.parent() {
        std::fs::create_dir_all(dir)?;
    }
    let partial = dest.with_extension("partial");
    let result = (|| {
        let mut out = std::io::BufWriter::with_capacity(1 << 20, std::fs::File::create(&partial)?);
        let mut buffer = vec![0u8; 1 << 20];
        let (mut copied, mut reported) = (0u64, 0u64);
        loop {
            let n = from.read(&mut buffer)?;
            if n == 0 {
                break;
            }
            out.write_all(&buffer[..n])?;
            copied += n as u64;
            if copied - reported >= 64 << 20 {
                progress(copied);
                reported = copied;
            }
        }
        out.flush()?;
        drop(out);
        if total > 0 && copied != total {
            return Err(std::io::Error::other(format!("copied {copied} bytes of {total}")));
        }
        std::fs::rename(&partial, dest)?;
        progress(copied);
        Ok(())
    })();
    if result.is_err() {
        let _ = std::fs::remove_file(&partial);
    }
    result
}

/// The game's library for a play action's program: the carried copy links
/// port/build/release/openrac-<id> to libopenrac-<id>-cli.so; the activity loads libopenrac-<id>.so.
pub fn game_library(plan: &Plan) -> Result<String, String> {
    let name = plan.program.file_name().and_then(|n| n.to_str()).unwrap_or("");
    let name = name.strip_suffix(".exe").unwrap_or(name);
    if name.starts_with("openrac-") && name.len() > "openrac-".len() {
        Ok(name.to_string())
    } else {
        Err(format!("{} is not one of OpenRAC's games", plan.program.display()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    #[test]
    fn disc_names_keep_no_folders() {
        assert_eq!(disc_file_name("Ratchet & Clank (Europe).iso"), "Ratchet & Clank (Europe).iso");
        assert_eq!(disc_file_name("../../x.iso"), "x.iso");
        assert_eq!(disc_file_name("a\\b.iso"), "b.iso");
        assert_eq!(disc_file_name(".."), "disc.iso");
        assert_eq!(disc_file_name(""), "disc.iso");
    }

    #[test]
    fn a_copy_is_whole_or_not_there() {
        let dir = tempfile::tempdir().unwrap();
        let dest = dir.path().join("discs/game.iso");
        let data = vec![7u8; 3 << 20];
        let seen = std::sync::Mutex::new(Vec::new());
        copy_disc(&data[..], data.len() as u64, &dest, |n| seen.lock().unwrap().push(n)).unwrap();
        assert_eq!(std::fs::read(&dest).unwrap(), data);
        assert_eq!(seen.lock().unwrap().last(), Some(&(data.len() as u64)));
        // The same image again is not copied again.
        copy_disc(&[][..], data.len() as u64, &dest, |_| ()).unwrap();
        assert_eq!(std::fs::read(&dest).unwrap().len(), data.len());
        // A short read leaves nothing behind.
        let other = dir.path().join("discs/other.iso");
        assert!(copy_disc(&data[..10], 20, &other, |_| ()).is_err());
        assert!(!other.exists() && !other.with_extension("partial").exists());
    }

    #[test]
    fn the_game_library_is_the_programs_name() {
        let plan = |program: &str| Plan {
            title: "Play".into(),
            program: PathBuf::from(program),
            args: vec![],
            cwd: PathBuf::from("."),
            detached: false,
            env: vec![],
        };
        assert_eq!(game_library(&plan("/r/port/build/release/openrac-rac1-pal")).unwrap(), "openrac-rac1-pal");
        assert_eq!(game_library(&plan("/r/openrac-rac1-ntsc.exe")).unwrap(), "openrac-rac1-ntsc");
        assert!(game_library(&plan("/usr/bin/python3")).is_err());
    }
}
