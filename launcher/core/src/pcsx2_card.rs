// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

//! Saves imported from a PCSX2 memory card.
//!
//! PCSX2 keeps a card either as one file (`Mcd001.ps2`: the card's pages as the console's card
//! holds them, 512 bytes each, with or without a 16-byte ECC spare after each) or as a folder
//! card (a folder per save, beside a `_pcsx2_superblock` file). The port's card is a folder per
//! save too (`saves.rs`), so an import copies the game's save folders (`BESCES-50916RATCHET` for
//! `SCES_509.16`) into it, after a backup of what was there.
//!
//! The card file's layout (the PS2 memory card file system): a superblock at the start; data
//! clusters from `alloc_offset` on, chained through a FAT that the superblock's indirect list
//! reaches; directories of 512-byte entries, the first (".") giving the entry count.

use serde::{Deserialize, Serialize};
use std::fs;
use std::path::{Path, PathBuf};

use crate::saves::{self, SaveBackupInfo};

const MAGIC: &[u8] = b"Sony PS2 Memory Card Format ";
const ENTRY_SIZE: usize = 512;
const MODE_EXISTS: u16 = 0x8000;
const MODE_DIR: u16 = 0x0020;
const MODE_FILE: u16 = 0x0010;
const FAT_END: u32 = 0xFFFF_FFFF;
const FAT_USED: u32 = 0x8000_0000;

/// One save folder read off a card: its name and its files.
#[derive(Debug, Clone, PartialEq)]
pub struct CardFolder {
    pub name: String,
    pub files: Vec<(String, Vec<u8>)>,
}

/// What an import brought in, for the page to say.
#[derive(Debug, Clone, Serialize, Deserialize, ts_rs::TS)]
#[serde(rename_all = "camelCase")]
pub struct ImportedSaves {
    /// The save folders copied in (`BESCES-50916RATCHET`).
    pub folders: Vec<String>,
    /// How many files they hold.
    pub files: u32,
    /// The backup of the saves that were there before, when there were any.
    pub backup: Option<SaveBackupInfo>,
}

/// The card file, read: its geometry and its bytes.
struct Card<'a> {
    bytes: &'a [u8],
    page_len: usize,
    raw_page: usize,
    pages_per_cluster: usize,
    clusters: u32,
    alloc_offset: u32,
    root: u32,
    ifc_list: [u32; 32],
}

fn u16_at(b: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([b[at], b[at + 1]])
}

fn u32_at(b: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([b[at], b[at + 1], b[at + 2], b[at + 3]])
}

impl<'a> Card<'a> {
    fn open(bytes: &'a [u8]) -> Result<Self, String> {
        if bytes.len() < 0x154 || !bytes.starts_with(MAGIC) {
            return Err("not a formatted PS2 memory card".into());
        }
        let page_len = u16_at(bytes, 0x28) as usize;
        let pages_per_cluster = u16_at(bytes, 0x2A) as usize;
        let clusters = u32_at(bytes, 0x30);
        if page_len == 0 || pages_per_cluster == 0 || clusters == 0 {
            return Err("the card's superblock is damaged".into());
        }
        // With ECC each page is followed by its 16 spare bytes (PCSX2's 8,650,752-byte cards).
        let pages = clusters as usize * pages_per_cluster;
        let raw_page = if bytes.len() >= pages * (page_len + 16) { page_len + 16 } else { page_len };
        if bytes.len() < pages * raw_page {
            return Err("the card file is shorter than its superblock says".into());
        }
        let mut ifc_list = [0u32; 32];
        for (i, slot) in ifc_list.iter_mut().enumerate() {
            *slot = u32_at(bytes, 0x50 + 4 * i);
        }
        Ok(Self {
            bytes,
            page_len,
            raw_page,
            pages_per_cluster,
            clusters,
            alloc_offset: u32_at(bytes, 0x34),
            root: u32_at(bytes, 0x3C),
            ifc_list,
        })
    }

    fn cluster_size(&self) -> usize {
        self.page_len * self.pages_per_cluster
    }

    /// The bytes of a cluster, by its number on the card (not relative to `alloc_offset`).
    fn cluster(&self, absolute: u32) -> Result<Vec<u8>, String> {
        if absolute >= self.clusters {
            return Err(format!("cluster {absolute} is past the card's end"));
        }
        let mut out = Vec::with_capacity(self.cluster_size());
        for page in 0..self.pages_per_cluster {
            let at = (absolute as usize * self.pages_per_cluster + page) * self.raw_page;
            out.extend_from_slice(&self.bytes[at..at + self.page_len]);
        }
        Ok(out)
    }

    /// The FAT's entry for a data cluster (relative to `alloc_offset`), through the indirect list.
    fn fat(&self, cluster: u32) -> Result<u32, String> {
        let per = (self.cluster_size() / 4) as u32;
        let fat_index = cluster / per;
        let indirect = self.ifc_list.get((fat_index / per) as usize).ok_or("the card's FAT is damaged")?;
        let fat_cluster = u32_at(&self.cluster(*indirect)?, (fat_index % per) as usize * 4);
        Ok(u32_at(&self.cluster(fat_cluster)?, (cluster % per) as usize * 4))
    }

    /// The bytes of a chain of data clusters from `start`, cut to `length`.
    fn chain(&self, start: u32, length: usize) -> Result<Vec<u8>, String> {
        let mut out = Vec::new();
        let mut cluster = start;
        // A chain longer than the card has clusters loops: the card is damaged.
        for _ in 0..self.clusters {
            out.extend(self.cluster(cluster + self.alloc_offset)?);
            if out.len() >= length {
                out.truncate(length);
                return Ok(out);
            }
            let next = self.fat(cluster)?;
            if next == FAT_END || next & FAT_USED == 0 {
                break;
            }
            cluster = next & !FAT_USED;
        }
        if out.len() < length {
            return Err("a file on the card ends early".into());
        }
        Ok(out)
    }

    /// A directory's entries, "." and ".." left out: (mode, length, first cluster, name).
    fn entries(&self, start: u32, count: usize) -> Result<Vec<(u16, u32, u32, String)>, String> {
        let raw = self.chain(start, count * ENTRY_SIZE)?;
        let mut out = Vec::new();
        for entry in raw.as_chunks::<ENTRY_SIZE>().0 {
            let mode = u16_at(entry, 0);
            let name_bytes = &entry[0x40..0x60];
            let name = String::from_utf8_lossy(&name_bytes[..name_bytes.iter().position(|&c| c == 0).unwrap_or(32)])
                .to_string();
            if mode & MODE_EXISTS == 0 || name == "." || name == ".." {
                continue;
            }
            out.push((mode, u32_at(entry, 4), u32_at(entry, 0x10), name));
        }
        Ok(out)
    }

    /// Every save folder on the card, with its files.
    fn folders(&self) -> Result<Vec<CardFolder>, String> {
        // The root's own "." entry, first in its first cluster, gives the number of entries.
        let count = u32_at(&self.chain(self.root, ENTRY_SIZE)?, 4) as usize;
        let mut out = Vec::new();
        for (mode, length, cluster, name) in self.entries(self.root, count)? {
            if mode & MODE_DIR == 0 {
                continue;
            }
            let mut files = Vec::new();
            for (mode, size, first, file) in self.entries(cluster, length as usize)? {
                if mode & MODE_FILE != 0 {
                    files.push((file, self.chain(first, size as usize)?));
                }
            }
            out.push(CardFolder { name, files });
        }
        Ok(out)
    }
}

/// The save folders on a PCSX2 card: a card file, or a folder card (its folder, or the
/// `_pcsx2_superblock` file in it, which a file picker can choose).
pub fn read_card(path: &Path) -> Result<Vec<CardFolder>, String> {
    let folder_card = if path.is_dir() {
        Some(path.to_path_buf())
    } else if path.file_name().is_some_and(|n| n == "_pcsx2_superblock") {
        path.parent().map(Path::to_path_buf)
    } else {
        None
    };
    if let Some(dir) = folder_card {
        return read_folder_card(&dir);
    }
    let bytes = fs::read(path).map_err(|e| format!("{}: {e}", path.display()))?;
    Card::open(&bytes)?.folders()
}

fn read_folder_card(dir: &Path) -> Result<Vec<CardFolder>, String> {
    let mut out = Vec::new();
    for entry in fs::read_dir(dir).map_err(|e| format!("{}: {e}", dir.display()))?.flatten() {
        let path = entry.path();
        let name = entry.file_name().to_string_lossy().to_string();
        if !path.is_dir() || name.starts_with('.') {
            continue;
        }
        let mut files = Vec::new();
        for file in fs::read_dir(&path).map_err(|e| e.to_string())?.flatten() {
            let file_name = file.file_name().to_string_lossy().to_string();
            // PCSX2's own bookkeeping (_pcsx2_index) is no save file.
            if file.path().is_file() && !file_name.starts_with("_pcsx2") {
                files.push((file_name, fs::read(file.path()).map_err(|e| e.to_string())?));
            }
        }
        out.push(CardFolder { name, files });
    }
    out.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(out)
}

/// The part of a game's save folder names its serial gives: `SCES_509.16` is `SCES-50916`, in
/// `BESCES-50916RATCHET` after the two region letters.
fn folder_code(serial: &str) -> String {
    let (letters, digits) = serial.split_once('_').unwrap_or((serial, ""));
    format!("{}-{}", letters.to_ascii_uppercase(), digits.replace('.', ""))
}

/// Whether `name` is one file or folder of its own: not empty, no separator, not `.` or `..`.
fn plain_name(name: &str) -> bool {
    !name.is_empty() && name != "." && name != ".." && !name.contains(['/', '\\', '\0'])
}

/// The game's save folders on the card.
pub fn game_folders(folders: Vec<CardFolder>, serial: &str) -> Vec<CardFolder> {
    let code = folder_code(serial);
    folders.into_iter().filter(|f| f.name.get(2..).is_some_and(|rest| rest.starts_with(&code))).collect()
}

/// Copies the game's saves from the PCSX2 card at `source` into its card in `card_dir`, after
/// a backup of the saves there. The card's other folders and its backups are kept.
pub fn import_into(card_dir: &Path, serial: &str, source: &Path) -> Result<ImportedSaves, String> {
    let found = game_folders(read_card(source)?, serial);
    if found.is_empty() {
        return Err(format!("{} has no saves of this game ({})", source.display(), folder_code(serial)));
    }
    for folder in &found {
        if !plain_name(&folder.name) || folder.files.iter().any(|(name, _)| !plain_name(name)) {
            return Err(format!("{} has a save with a name that is not a file name", source.display()));
        }
    }

    fs::create_dir_all(card_dir).map_err(|e| e.to_string())?;
    let backup = match saves::find_game_folder(card_dir) {
        Some(_) => Some(saves::backup_card(card_dir, Some("before PCSX2 import"))?),
        None => None,
    };

    let mut files = 0;
    for folder in &found {
        let dest = card_dir.join(&folder.name);
        if dest.exists() {
            fs::remove_dir_all(&dest).map_err(|e| e.to_string())?;
        }
        fs::create_dir_all(&dest).map_err(|e| e.to_string())?;
        for (name, bytes) in &folder.files {
            fs::write(dest.join(name), bytes).map_err(|e| e.to_string())?;
            files += 1;
        }
    }
    Ok(ImportedSaves { folders: found.into_iter().map(|f| f.name).collect(), files, backup })
}

/// [`import_into`] the game's own card.
pub fn import_saves(serial: &str, source: &Path) -> Result<ImportedSaves, String> {
    import_into(&saves::get_memcard_dir(serial), serial, source)
}

/// Where PCSX2 keeps its memory cards, when the folder is there: the Flatpak's, then the
/// installed program's (Linux), Documents (Windows) or Application Support (macOS).
pub fn pcsx2_memcards_dir() -> Option<PathBuf> {
    let home = crate::detect::home_dir()?;
    let candidates = [
        home.join(".var/app/net.pcsx2.PCSX2/config/PCSX2/memcards"),
        home.join(".config/PCSX2/memcards"),
        home.join("Documents/PCSX2/memcards"),
        home.join("Library/Application Support/PCSX2/memcards"),
    ];
    candidates.into_iter().find(|p| p.is_dir())
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A small card file: 64 clusters of 2 pages, data from cluster 8, the indirect list in
    /// cluster 2 and the FAT in cluster 3. The root holds one save folder with two files, the
    /// second spanning three clusters.
    fn card(ecc: bool) -> Vec<u8> {
        const PAGE: usize = 512;
        const CLUSTERS: usize = 64;
        const ALLOC: u32 = 8;
        let mut clusters = vec![vec![0u8; 2 * PAGE]; CLUSTERS];
        let sb = &mut clusters[0];
        sb[..MAGIC.len()].copy_from_slice(MAGIC);
        sb[0x28..0x2A].copy_from_slice(&(PAGE as u16).to_le_bytes());
        sb[0x2A..0x2C].copy_from_slice(&2u16.to_le_bytes());
        sb[0x2C..0x2E].copy_from_slice(&16u16.to_le_bytes());
        sb[0x30..0x34].copy_from_slice(&(CLUSTERS as u32).to_le_bytes());
        sb[0x34..0x38].copy_from_slice(&ALLOC.to_le_bytes());
        sb[0x3C..0x40].copy_from_slice(&0u32.to_le_bytes());
        sb[0x50..0x54].copy_from_slice(&2u32.to_le_bytes());
        clusters[2][..4].copy_from_slice(&3u32.to_le_bytes());

        // Data clusters: 0 and 6 the root, 1 and 7 the folder, 2 icon.sys, 3..5 save0.bin.
        for i in 0..256 {
            clusters[3][i * 4..i * 4 + 4].copy_from_slice(&FAT_END.to_le_bytes());
        }
        for (from, to) in [(0, 6), (1, 7), (3, 4), (4, 5)] {
            let at = from * 4;
            clusters[3][at..at + 4].copy_from_slice(&(to | FAT_USED).to_le_bytes());
        }

        let entry = |mode: u16, length: u32, cluster: u32, name: &str| {
            let mut e = vec![0u8; ENTRY_SIZE];
            e[..2].copy_from_slice(&mode.to_le_bytes());
            e[4..8].copy_from_slice(&length.to_le_bytes());
            e[0x10..0x14].copy_from_slice(&cluster.to_le_bytes());
            e[0x40..0x40 + name.len()].copy_from_slice(name.as_bytes());
            e
        };
        let dir = MODE_EXISTS | MODE_DIR;
        let file = MODE_EXISTS | MODE_FILE;
        let root = [entry(dir, 3, 0, "."), entry(dir, 0, 0, "..")].concat();
        clusters[ALLOC as usize].copy_from_slice(&root);
        // The root's third entry is in its second cluster.
        clusters[ALLOC as usize + 6][..ENTRY_SIZE].copy_from_slice(&entry(dir, 4, 1, "BESCES-50916RATCHET"));
        let folder = [entry(dir, 4, 1, "."), entry(dir, 0, 0, "..")].concat();
        clusters[ALLOC as usize + 1].copy_from_slice(&folder);
        let files = [entry(file, 5, 2, "icon.sys"), entry(file, 2500, 3, "save0.bin")].concat();
        clusters[ALLOC as usize + 7].copy_from_slice(&files);
        clusters[ALLOC as usize + 2][..5].copy_from_slice(b"icon!");
        for (n, c) in (3..6).enumerate() {
            clusters[ALLOC as usize + c].fill(b'a' + n as u8);
        }

        let mut out = Vec::new();
        for c in &clusters {
            for page in c.chunks(PAGE) {
                out.extend_from_slice(page);
                if ecc {
                    out.extend_from_slice(&[0u8; 16]);
                }
            }
        }
        out
    }

    #[test]
    fn reads_a_card_file_with_and_without_ecc() {
        for ecc in [false, true] {
            let bytes = card(ecc);
            let folders = Card::open(&bytes).unwrap().folders().unwrap();
            assert_eq!(folders.len(), 1);
            assert_eq!(folders[0].name, "BESCES-50916RATCHET");
            let (name, icon) = &folders[0].files[0];
            assert_eq!((name.as_str(), icon.as_slice()), ("icon.sys", &b"icon!"[..]));
            let (name, save) = &folders[0].files[1];
            assert_eq!(name, "save0.bin");
            assert_eq!(save.len(), 2500);
            assert_eq!((save[0], save[1024], save[2048]), (b'a', b'b', b'c'));
        }
    }

    #[test]
    fn refuses_what_is_no_card() {
        assert!(Card::open(&[0u8; 4096]).is_err());
    }

    #[test]
    fn matches_the_game_by_its_serial() {
        let folder = |name: &str| CardFolder { name: name.into(), files: vec![] };
        let found = game_folders(
            vec![folder("BESCES-50916RATCHET"), folder("BESCES-51607RATCHET2"), folder("BASCUS-97199RATCHET")],
            "SCES_509.16",
        );
        assert_eq!(found, vec![folder("BESCES-50916RATCHET")]);
    }

    #[test]
    fn imports_into_the_card_after_a_backup() {
        let tmp = tempfile::tempdir().unwrap();
        let source = tmp.path().join("Mcd001.ps2");
        fs::write(&source, card(true)).unwrap();
        let dest = tmp.path().join("SCES_509.16");
        fs::create_dir_all(dest.join("BESCES-50916RATCHET")).unwrap();
        fs::write(dest.join("BESCES-50916RATCHET/save0.bin"), b"old").unwrap();

        let done = import_into(&dest, "SCES_509.16", &source).unwrap();
        assert_eq!(done.folders, vec!["BESCES-50916RATCHET".to_string()]);
        assert_eq!(done.files, 2);
        assert_eq!(fs::read(dest.join("BESCES-50916RATCHET/save0.bin")).unwrap().len(), 2500);
        let backup = done.backup.unwrap();
        assert_eq!(fs::read(Path::new(&backup.path).join("BESCES-50916RATCHET/save0.bin")).unwrap(), b"old");
    }

    #[test]
    fn a_card_without_the_game_changes_nothing() {
        let tmp = tempfile::tempdir().unwrap();
        let source = tmp.path().join("Mcd001.ps2");
        fs::write(&source, card(false)).unwrap();
        let dest = tmp.path().join("SCUS_971.99");
        assert!(import_into(&dest, "SCUS_971.99", &source).is_err());
        assert!(!dest.exists());
    }
}
