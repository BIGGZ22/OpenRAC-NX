//! Disc images in MAME's compressed format, CHD (`chdman createdvd`), beside plain .iso images.
//! The set-up and the game read a disc by sector from a plain image, so a CHD is unpacked once
//! into one (`to_iso`); until then the launcher reads it in place (`ChdImage`) to tell which
//! game it is. A CHD of a CD (`chdman createcd`, sectors with their subcode) is not a PlayStation
//! 2 DVD's image and is refused. Read with the chd crate (BSD-3-Clause), every codec chdman writes.
use std::fs::File;
use std::io::{BufWriter, Read, Seek, Write};
use std::path::Path;

use chd::metadata::{KnownMetadata, MetadataTag};
use chd::Chd;

/// Whether `path` names a CHD image (by its extension, as the pickers and the scan see it).
pub fn is_chd(path: &Path) -> bool {
    path.extension().is_some_and(|e| e.eq_ignore_ascii_case("chd"))
}

/// The plain image a CHD is unpacked into: `Game (Europe).chd` gives `Game (Europe).iso`.
pub fn iso_name(name: &str) -> String {
    let stem =
        name.len().checked_sub(4).filter(|&n| name.is_char_boundary(n) && name[n..].eq_ignore_ascii_case(".chd"));
    format!("{}.iso", stem.map_or(name, |n| &name[..n]))
}

/// A CHD image read by byte offset, as the plain image it holds; the last hunk read is kept.
pub struct ChdImage<F: Read + Seek> {
    chd: Chd<F>,
    hunk: Vec<u8>,
    compressed: Vec<u8>,
    current: Option<u32>,
}

impl ChdImage<File> {
    pub fn open(path: &Path) -> Result<Self, String> {
        let file = File::open(path).map_err(|e| format!("{}: {e}", path.display()))?;
        Self::new(file)
    }
}

impl<F: Read + Seek> ChdImage<F> {
    pub fn new(file: F) -> Result<Self, String> {
        let mut chd = Chd::open(file, None).map_err(|e| format!("not a CHD image this launcher can read: {e}"))?;
        if chd.header().has_parent() {
            return Err("this CHD image needs its parent CHD; unpack it with chdman first".into());
        }
        if chd.metadata_refs().any(|m| KnownMetadata::is_cdrom(m.metatag())) {
            return Err(
                "this CHD image is of a CD (chdman createcd); a PlayStation 2 DVD's is made with chdman createdvd"
                    .into(),
            );
        }
        let hunk = chd.get_hunksized_buffer();
        Ok(ChdImage { chd, hunk, compressed: Vec::new(), current: None })
    }

    /// The size of the image it holds.
    pub fn len(&self) -> u64 {
        self.chd.header().logical_bytes()
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    fn load(&mut self, index: u32) -> Result<(), String> {
        if self.current != Some(index) {
            self.current = None;
            let mut hunk = self.chd.hunk(index).map_err(|e| format!("hunk {index}: {e}"))?;
            hunk.read_hunk_in(&mut self.compressed, &mut self.hunk).map_err(|e| format!("hunk {index}: {e}"))?;
            self.current = Some(index);
        }
        Ok(())
    }

    /// Fills `buffer` from `offset` of the image; an error past its end.
    pub fn read_at(&mut self, offset: u64, buffer: &mut [u8]) -> Result<(), String> {
        if offset.checked_add(buffer.len() as u64).is_none_or(|end| end > self.len()) {
            return Err("past the end of the image".into());
        }
        let size = self.hunk.len() as u64;
        let mut done = 0;
        while done < buffer.len() {
            let at = offset + done as u64;
            self.load((at / size) as u32)?;
            let from = (at % size) as usize;
            let n = (buffer.len() - done).min(self.hunk.len() - from);
            buffer[done..done + n].copy_from_slice(&self.hunk[from..from + n]);
            done += n;
        }
        Ok(())
    }

    /// Writes the whole image to `out`, reporting (written, total) every 64 MB and at the end.
    fn unpack(&mut self, out: &mut impl Write, progress: &impl Fn(u64, u64)) -> Result<(), String> {
        let total = self.len();
        let (mut written, mut reported) = (0u64, 0u64);
        let mut index = 0;
        while written < total {
            self.load(index)?;
            let n = (total - written).min(self.hunk.len() as u64) as usize;
            out.write_all(&self.hunk[..n]).map_err(|e| e.to_string())?;
            written += n as u64;
            index += 1;
            if written - reported >= 64 << 20 {
                progress(written, total);
                reported = written;
            }
        }
        progress(written, total);
        Ok(())
    }
}

/// Unpacks the CHD `from` into the plain image `dest`, through a file beside it; an image already
/// there with the right size is kept as it is. Reports (written, total).
pub fn to_iso(from: impl Read + Seek, dest: &Path, progress: impl Fn(u64, u64)) -> Result<(), String> {
    ChdImage::new(from)?.save_iso(dest, progress)
}

impl<F: Read + Seek> ChdImage<F> {
    /// `to_iso`, for an image already open (its size known first, to check the space for it).
    pub fn save_iso(&mut self, dest: &Path, progress: impl Fn(u64, u64)) -> Result<(), String> {
        let total = self.len();
        if dest.metadata().is_ok_and(|m| m.len() == total) {
            progress(total, total);
            return Ok(());
        }
        if let Some(dir) = dest.parent() {
            std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
        }
        let partial = dest.with_extension("partial");
        let result = (|| {
            let file = File::create(&partial).map_err(|e| format!("{}: {e}", partial.display()))?;
            let mut out = BufWriter::with_capacity(1 << 20, file);
            self.unpack(&mut out, &progress)?;
            out.flush().map_err(|e| e.to_string())?;
            drop(out);
            std::fs::rename(&partial, dest).map_err(|e| format!("{}: {e}", dest.display()))
        })();
        if result.is_err() {
            let _ = std::fs::remove_file(&partial);
        }
        result
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;
    use std::io::Cursor;

    /// An uncompressed CHD v5 of `data` in hunks of `hunk` bytes, units of 2048 (createdvd's).
    pub(crate) fn chd_of(data: &[u8], hunk: u32) -> Vec<u8> {
        let hunks = data.len().div_ceil(hunk as usize);
        let map_at = 124u64;
        let data_at = (map_at + 4 * hunks as u64).div_ceil(u64::from(hunk)) * u64::from(hunk);
        let mut out = Vec::new();
        out.extend_from_slice(b"MComprHD");
        out.extend_from_slice(&124u32.to_be_bytes());
        out.extend_from_slice(&5u32.to_be_bytes());
        out.extend_from_slice(&[0u8; 16]); // no compressors
        out.extend_from_slice(&(data.len() as u64).to_be_bytes());
        out.extend_from_slice(&map_at.to_be_bytes());
        out.extend_from_slice(&0u64.to_be_bytes()); // no metadata
        out.extend_from_slice(&hunk.to_be_bytes());
        out.extend_from_slice(&2048u32.to_be_bytes());
        out.extend_from_slice(&[0u8; 60]); // the SHA-1s
        for i in 0..hunks as u64 {
            out.extend_from_slice(&((data_at / u64::from(hunk) + i) as u32).to_be_bytes());
        }
        out.resize(data_at as usize, 0);
        out.extend_from_slice(data);
        out.resize(data_at as usize + hunks * hunk as usize, 0);
        out
    }

    fn data(len: usize) -> Vec<u8> {
        (0..len).map(|i| (i * 7 % 251) as u8).collect()
    }

    #[test]
    fn names_the_plain_image() {
        assert_eq!(iso_name("Ratchet & Clank (Europe).chd"), "Ratchet & Clank (Europe).iso");
        assert_eq!(iso_name("GAME.CHD"), "GAME.iso");
        assert_eq!(iso_name("disc"), "disc.iso");
        assert!(is_chd(Path::new("/x/Game.CHD")));
        assert!(!is_chd(Path::new("/x/Game.iso")));
    }

    #[test]
    fn reads_across_hunks() {
        let plain = data(3 * 4096 + 100);
        let mut image = ChdImage::new(Cursor::new(chd_of(&plain, 4096))).unwrap();
        assert_eq!(image.len(), plain.len() as u64);
        let mut buffer = vec![0u8; 5000];
        image.read_at(4000, &mut buffer).unwrap();
        assert_eq!(buffer, plain[4000..9000]);
        let mut tail = [0u8; 100];
        image.read_at(3 * 4096, &mut tail).unwrap();
        assert_eq!(tail, plain[3 * 4096..]);
        assert!(image.read_at(plain.len() as u64 - 10, &mut [0u8; 11]).is_err());
    }

    #[test]
    fn unpacks_into_a_plain_image() {
        let dir = tempfile::tempdir().unwrap();
        let plain = data(5 * 4096 + 2048);
        let dest = dir.path().join("discs/game.iso");
        let seen = std::cell::Cell::new((0, 0));
        to_iso(Cursor::new(chd_of(&plain, 4096)), &dest, |done, total| seen.set((done, total))).unwrap();
        assert_eq!(std::fs::read(&dest).unwrap(), plain);
        assert_eq!(seen.get(), (plain.len() as u64, plain.len() as u64));
        assert!(!dest.with_extension("partial").exists());
        // Already there: kept.
        to_iso(Cursor::new(chd_of(&plain, 4096)), &dest, |_, _| {}).unwrap();
        assert_eq!(std::fs::read(&dest).unwrap(), plain);
    }

    #[test]
    fn refuses_what_is_not_one() {
        let dir = tempfile::tempdir().unwrap();
        let dest = dir.path().join("game.iso");
        assert!(to_iso(Cursor::new(b"not a CHD at all, just bytes".repeat(10)), &dest, |_, _| {}).is_err());
        assert!(!dest.exists() && !dest.with_extension("partial").exists());
    }
}
