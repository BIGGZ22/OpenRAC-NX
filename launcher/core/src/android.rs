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

/// The name a picked plain image is kept under: its own, with `.iso` added when the picker gave
/// it without (the games folder and the extractor know an image by that extension).
pub fn iso_file_name(name: &str) -> String {
    let iso =
        name.len().checked_sub(4).is_some_and(|n| name.is_char_boundary(n) && name[n..].eq_ignore_ascii_case(".iso"));
    if iso {
        name.to_string()
    } else {
        format!("{name}.iso")
    }
}

/// Copies `from` into `dest` through a file beside it, reporting every 64 MB; an image already
/// there with the same size, and the same first and last MB, is kept as it is (another dump of the
/// same name and size is copied). File to file, so that the kernel copies (copy_file_range or
/// sendfile, which std::io::copy uses on Linux): no bytes through the app.
pub fn copy_disc(
    from: std::fs::File,
    total: u64,
    dest: &std::path::Path,
    progress: impl Fn(u64),
) -> std::io::Result<()> {
    use std::io::{Read, Seek, SeekFrom};
    if total > 0 && dest.metadata().is_ok_and(|m| m.len() == total) && (&from).seek(SeekFrom::Start(0)).is_ok() {
        let same = same_ends(&from, dest, total).unwrap_or(false);
        (&from).seek(SeekFrom::Start(0))?;
        if same {
            progress(total);
            return Ok(());
        }
    }
    if let Some(dir) = dest.parent() {
        std::fs::create_dir_all(dir)?;
    }
    let partial = dest.with_extension("partial");
    let result = (|| {
        let mut out = std::fs::File::create(&partial)?;
        let mut copied = 0u64;
        loop {
            let n = std::io::copy(&mut (&from).take(64 << 20), &mut out)?;
            if n == 0 {
                break;
            }
            copied += n;
            progress(copied);
        }
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

/// Unpacks a picked CHD (`from`, `size` bytes, `free` bytes left on the device, -1 when unknown) into
/// the plain image `dest`, reporting (written, total). A CHD is read out of order, which a picker's
/// file allows only when it is a file on the device: one that is not (a pipe from a cloud or a
/// file manager's provider) is copied beside `dest` first, unpacked from there, and removed.
pub fn unpack_chd(
    from: std::fs::File,
    size: u64,
    free: i64,
    dest: &std::path::Path,
    progress: impl Fn(u64, u64),
) -> Result<(), String> {
    use std::io::{Seek, SeekFrom};
    let seekable = from.metadata().is_ok_and(|m| m.is_file()) && (&from).seek(SeekFrom::Start(0)).is_ok();
    let copy = dest.with_extension("chd");
    let image = if seekable {
        from
    } else {
        // Room for the copy too, which goes once the image is unpacked.
        if free >= 0 && size + (256 << 20) > free as u64 {
            return Err(format!(
                "it needs {} MB and the device has {} MB free",
                (size + (256 << 20)) >> 20,
                free >> 20
            ));
        }
        copy_disc(from, size, &copy, |done| progress(done, size)).map_err(|e| format!("cannot copy it: {e}"))?;
        std::fs::File::open(&copy).map_err(|e| format!("{}: {e}", copy.display()))?
    };
    let free = if seekable || free < 0 { free } else { free - size as i64 };
    let result = (|| {
        let mut image = crate::chd::ChdImage::new(image)?;
        // As the Kotlin side checks a plain image: room for it and the levels written next.
        let need = image.len() + (256 << 20);
        if free >= 0 && need > free as u64 {
            return Err(format!("it needs {} MB and the device has {} MB free", need >> 20, free >> 20));
        }
        image.save_iso(dest, progress)
    })();
    if !seekable {
        let _ = std::fs::remove_file(&copy);
    }
    result
}

/// Whether two images of `total` bytes begin and end with the same MB.
fn same_ends(from: &std::fs::File, dest: &std::path::Path, total: u64) -> std::io::Result<bool> {
    use std::io::{Read, Seek, SeekFrom};
    const SAMPLE: u64 = 1 << 20;
    let mut kept = std::fs::File::open(dest)?;
    let mut from = from;
    let ends = [0, total.saturating_sub(SAMPLE)];
    for at in ends {
        let len = SAMPLE.min(total) as usize;
        let (mut a, mut b) = (vec![0; len], vec![0; len]);
        from.seek(SeekFrom::Start(at))?;
        from.read_exact(&mut a)?;
        kept.seek(SeekFrom::Start(at))?;
        kept.read_exact(&mut b)?;
        if a != b {
            return Ok(false);
        }
    }
    Ok(true)
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
        let data: Vec<u8> = (0..(67u32 << 20)).map(|i| (i % 251) as u8).collect();
        let picked = dir.path().join("picked.iso");
        std::fs::write(&picked, &data).unwrap();
        let open = || std::fs::File::open(&picked).unwrap();
        let seen = std::sync::Mutex::new(Vec::new());
        copy_disc(open(), data.len() as u64, &dest, |n| seen.lock().unwrap().push(n)).unwrap();
        assert!(std::fs::read(&dest).unwrap() == data);
        assert_eq!(seen.lock().unwrap().first(), Some(&(64u64 << 20)));
        assert_eq!(seen.lock().unwrap().last(), Some(&(data.len() as u64)));
        // The same image again is not copied again: one report, of the whole.
        let seen = std::sync::Mutex::new(Vec::new());
        copy_disc(open(), data.len() as u64, &dest, |n| seen.lock().unwrap().push(n)).unwrap();
        assert_eq!(*seen.lock().unwrap(), vec![data.len() as u64]);
        // Another image of the same name and size is.
        let mut changed = data.clone();
        *changed.last_mut().unwrap() ^= 1;
        std::fs::write(&picked, &changed).unwrap();
        copy_disc(open(), data.len() as u64, &dest, |_| ()).unwrap();
        assert!(std::fs::read(&dest).unwrap() == changed);
        // A short read leaves nothing behind.
        let other = dir.path().join("discs/other.iso");
        assert!(copy_disc(open(), data.len() as u64 + 1, &other, |_| ()).is_err());
        assert!(!other.exists() && !other.with_extension("partial").exists());
    }

    /// A CHD the picker gives as a file on the device is unpacked from it; one it streams (a pipe)
    /// is copied beside the image first, and that copy goes once the image is there.
    #[cfg(unix)]
    #[test]
    fn a_picked_chd_is_unpacked_whether_or_not_it_can_be_seeked() {
        use std::io::Write;
        let dir = tempfile::tempdir().unwrap();
        let plain: Vec<u8> = (0..5 * 4096 + 2048).map(|i| (i * 7 % 251) as u8).collect();
        let packed = crate::chd::tests::chd_of(&plain, 4096);
        let picked = dir.path().join("picked");
        std::fs::write(&picked, &packed).unwrap();
        let dest = dir.path().join("discs/game.iso");
        unpack_chd(std::fs::File::open(&picked).unwrap(), packed.len() as u64, -1, &dest, |_, _| ()).unwrap();
        assert_eq!(std::fs::read(&dest).unwrap(), plain);

        let (reader, mut writer) = std::io::pipe().unwrap();
        let feed = std::thread::spawn(move || writer.write_all(&packed));
        let piped = std::fs::File::from(std::os::fd::OwnedFd::from(reader));
        let other = dir.path().join("discs/other.iso");
        let size = std::fs::metadata(&picked).unwrap().len();
        unpack_chd(piped, size, -1, &other, |_, _| ()).unwrap();
        feed.join().unwrap().unwrap();
        assert_eq!(std::fs::read(&other).unwrap(), plain);
        assert!(!other.with_extension("chd").exists());
        // Too little room for the copy and the image: refused before anything is written.
        let (reader, _writer) = std::io::pipe().unwrap();
        let piped = std::fs::File::from(std::os::fd::OwnedFd::from(reader));
        let third = dir.path().join("discs/third.iso");
        assert!(unpack_chd(piped, 1 << 30, 1 << 20, &third, |_, _| ()).is_err());
        assert!(!third.exists() && !third.with_extension("chd").exists());
    }

    #[test]
    fn a_plain_image_is_kept_as_an_iso() {
        assert_eq!(iso_file_name("Ratchet & Clank.ISO"), "Ratchet & Clank.ISO");
        assert_eq!(iso_file_name("Ratchet & Clank"), "Ratchet & Clank.iso");
        assert_eq!(iso_file_name("disc.bin"), "disc.bin.iso");
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
            output: None,
        };
        assert_eq!(game_library(&plan("/r/port/build/release/openrac-rac1-pal")).unwrap(), "openrac-rac1-pal");
        assert_eq!(game_library(&plan("/r/openrac-rac1-ntsc.exe")).unwrap(), "openrac-rac1-ntsc");
        assert!(game_library(&plan("/usr/bin/python3")).is_err());
    }
}
