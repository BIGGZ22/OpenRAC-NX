//! The player's folder of PlayStation 2 disc images: which of them are games
//! OpenRAC knows. An image is told by the name of its boot program (the
//! `BOOT2` line of its `SYSTEM.CNF`, the disc's serial), read from the image
//! itself, so its file name does not matter. Nothing is changed and nothing
//! leaves the machine; the set-up checks the boot program's bytes afterwards
//! (tools/extractor.py --validate).

use std::fs::File;
use std::io::{Read, Seek, SeekFrom};
use std::path::{Path, PathBuf};

use serde::Serialize;

use crate::catalog::Catalog;

const SECTOR: u64 = 2048;
/// Folders are looked into this deep, and no more images than this are opened.
const DEPTH: usize = 2;
const MOST: usize = 2000;

/// A disc image of a game OpenRAC knows.
#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct FoundDisc {
    pub path: PathBuf,
    /// As the catalogue writes it: `SCES_509.16`.
    pub serial: String,
    pub game: String,
    /// `rac1/pal`.
    pub version: String,
    pub title: String,
    pub region: String,
}

/// The images under `dir` that are a version in `catalog`, one for each version (the first found).
pub fn scan(dir: &Path, catalog: &Catalog) -> Vec<FoundDisc> {
    let mut images = Vec::new();
    collect(dir, DEPTH, &mut images);
    images.sort();
    let mut found: Vec<FoundDisc> = Vec::new();
    for path in images {
        let Some(serial) = serial_of(&path) else { continue };
        for game in &catalog.games {
            for version in &game.versions {
                if version.serial == serial && !found.iter().any(|f| f.version == version.key) {
                    found.push(FoundDisc {
                        path: path.clone(),
                        serial: serial.clone(),
                        game: version.game.clone(),
                        version: version.key.clone(),
                        title: version.title.clone(),
                        region: version.region.clone(),
                    });
                }
            }
        }
    }
    found
}

fn collect(dir: &Path, depth: usize, out: &mut Vec<PathBuf>) {
    let Ok(entries) = std::fs::read_dir(dir) else { return };
    for entry in entries.filter_map(Result::ok) {
        if out.len() >= MOST {
            return;
        }
        let path = entry.path();
        let hidden = path.file_name().is_some_and(|n| n.to_string_lossy().starts_with('.'));
        if hidden {
            continue;
        }
        if path.is_dir() {
            if depth > 0 {
                collect(&path, depth - 1, out);
            }
        } else if path.extension().is_some_and(|e| e.eq_ignore_ascii_case("iso")) {
            out.push(path);
        }
    }
}

/// The serial of a PlayStation 2 image: the boot program `SYSTEM.CNF` names, as `SCES_509.16`.
/// `None` when the file is not an ISO 9660 image with one.
pub fn serial_of(image: &Path) -> Option<String> {
    let mut file = File::open(image).ok()?;
    // The primary volume descriptor, sector 16: "\x01CD001", the root directory's record at 156.
    let mut pvd = [0u8; SECTOR as usize];
    read_at(&mut file, 16 * SECTOR, &mut pvd)?;
    if &pvd[..6] != b"\x01CD001" {
        return None;
    }
    let root = &pvd[156..190];
    let extent = u64::from(u32::from_le_bytes(root[2..6].try_into().ok()?));
    let size = (u32::from_le_bytes(root[10..14].try_into().ok()?) as usize).min(64 * SECTOR as usize);
    let mut dir = vec![0u8; size];
    read_at(&mut file, extent * SECTOR, &mut dir)?;

    let mut at = 0;
    while at + 33 < dir.len() {
        let len = dir[at] as usize;
        if len == 0 {
            at = (at / SECTOR as usize + 1) * SECTOR as usize; // records do not cross sectors
            continue;
        }
        if at + len > dir.len() {
            break;
        }
        let record = &dir[at..at + len];
        let name_len = record[32] as usize;
        let name = record.get(33..33 + name_len)?;
        if name.eq_ignore_ascii_case(b"SYSTEM.CNF;1") || name.eq_ignore_ascii_case(b"SYSTEM.CNF") {
            let extent = u64::from(u32::from_le_bytes(record[2..6].try_into().ok()?));
            let size = (u32::from_le_bytes(record[10..14].try_into().ok()?) as usize).min(4096);
            let mut text = vec![0u8; size];
            read_at(&mut file, extent * SECTOR, &mut text)?;
            return boot_serial(&String::from_utf8_lossy(&text));
        }
        at += len;
    }
    None
}

fn read_at(file: &mut File, offset: u64, buffer: &mut [u8]) -> Option<()> {
    file.seek(SeekFrom::Start(offset)).ok()?;
    file.read_exact(buffer).ok()
}

/// `BOOT2 = cdrom0:\SCES_509.16;1` gives `SCES_509.16`.
fn boot_serial(system_cnf: &str) -> Option<String> {
    let line = system_cnf.lines().find(|l| l.trim_start().to_ascii_uppercase().starts_with("BOOT2"))?;
    let value = line.split_once('=')?.1.trim();
    let name = value.rsplit(['\\', ':', '/']).next()?;
    let name = name.split(';').next()?.trim();
    (!name.is_empty()).then(|| name.to_ascii_uppercase())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testing::checkout;

    /// A small ISO 9660 image: a volume descriptor, a root directory with SYSTEM.CNF, its text.
    fn image(path: &Path, system_cnf: &str) {
        let mut data = vec![0u8; 20 * SECTOR as usize];
        let pvd = 16 * SECTOR as usize;
        data[pvd..pvd + 6].copy_from_slice(b"\x01CD001");
        let root = pvd + 156;
        data[root] = 34;
        data[root + 2..root + 6].copy_from_slice(&18u32.to_le_bytes());
        data[root + 10..root + 14].copy_from_slice(&(SECTOR as u32).to_le_bytes());
        let dir = 18 * SECTOR as usize;
        let name = b"SYSTEM.CNF;1";
        data[dir] = (33 + name.len()) as u8;
        data[dir + 2..dir + 6].copy_from_slice(&19u32.to_le_bytes());
        data[dir + 10..dir + 14].copy_from_slice(&(system_cnf.len() as u32).to_le_bytes());
        data[dir + 32] = name.len() as u8;
        data[dir + 33..dir + 33 + name.len()].copy_from_slice(name);
        let text = 19 * SECTOR as usize;
        data[text..text + system_cnf.len()].copy_from_slice(system_cnf.as_bytes());
        std::fs::write(path, data).unwrap();
    }

    #[test]
    fn reads_the_serial_the_disc_boots() {
        assert_eq!(boot_serial("BOOT2 = cdrom0:\\SCES_509.16;1\r\nVER = 2.00\r\n"), Some("SCES_509.16".into()));
        assert_eq!(boot_serial("VER = 1.00\nBOOT2=cdrom0:\\scus_971.99;1"), Some("SCUS_971.99".into()));
        assert_eq!(boot_serial("BOOT = cdrom:\\PSX.EXE;1"), None);
    }

    /// A real image, where this checkout has one (baserom/ is the contributor's own, never in git).
    #[test]
    fn reads_a_real_image() {
        let image = checkout().join("baserom").join("SCES_509.16.iso");
        if image.is_file() {
            assert_eq!(serial_of(&image), Some("SCES_509.16".into()));
        }
    }

    #[test]
    fn finds_the_games_it_knows_whatever_their_files_are_called() {
        let tmp = std::env::temp_dir().join(format!("openrac-discs-{}", std::process::id()));
        std::fs::create_dir_all(tmp.join("PS2").join("Ratchet")).unwrap();
        image(&tmp.join("PS2").join("Ratchet").join("my backup (1).iso"), "BOOT2 = cdrom0:\\SCES_509.16;1\r\n");
        image(&tmp.join("another game.ISO"), "BOOT2 = cdrom0:\\SLES_123.45;1\r\n");
        std::fs::write(tmp.join("notes.iso"), b"not an image").unwrap();
        std::fs::write(tmp.join("readme.txt"), b"").unwrap();

        let catalog = crate::catalog::load(&checkout()).unwrap();
        let found = scan(&tmp, &catalog);
        assert_eq!(found.len(), 1, "{found:?}");
        assert_eq!((found[0].version.as_str(), found[0].serial.as_str()), ("rac1/pal", "SCES_509.16"));
        assert!(found[0].path.ends_with("my backup (1).iso"));
        assert_eq!(serial_of(&tmp.join("notes.iso")), None);
        std::fs::remove_dir_all(&tmp).unwrap();
    }
}
