//! Disc images in MAME's compressed format, CHD, beside plain .iso images: a DVD's
//! (`chdman createdvd`) and a CD's (`chdman createcd`, which PCSX2 plays too: each sector stored
//! with its subcode, of which the image is the first data track's 2048 bytes a sector). The
//! set-up and the game read a disc by sector from a plain image, so a CHD is unpacked once into
//! one (`to_iso`); until then the launcher reads it in place (`ChdImage`) to tell which game it
//! is. Read with the chd crate (BSD-3-Clause), every codec chdman writes.
use std::fs::File;
use std::io::{BufWriter, Read, Seek, Write};
use std::path::Path;

use chd::metadata::{KnownMetadata, MetadataTag};
use chd::Chd;

/// The bytes of a sector a plain image holds.
const SECTOR: u64 = 2048;
/// chdman pads each track of a CD to a multiple of this many frames.
const TRACK_PADDING: u64 = 4;

/// Where a CD's image is among a CHD's frames: its first data track's sectors.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
struct CdTrack {
    /// The first sector's frame, counted from the start of the CHD.
    first: u64,
    /// How many sectors.
    sectors: u64,
    /// Where a sector's 2048 bytes start in its frame: 0 for a cooked sector, 16 for a raw mode 1
    /// one (after its sync and header), 24 for a raw mode 2 form 1 one, 8 for a 2336-byte one.
    offset: u64,
}

/// The first data track of a CD's track list (chdman's CHT2 or CHTR text, one entry a track,
/// "TRACK:1 TYPE:MODE2_RAW SUBTYPE:NONE FRAMES:1234 PREGAP:0 PGTYPE:MODE1 ..."), or why not.
fn first_data_track(tracks: &[String]) -> Result<CdTrack, String> {
    let mut frame = 0u64;
    for track in tracks {
        let field = |name: &str| {
            track.split_ascii_whitespace().find_map(|f| f.strip_prefix(name).and_then(|v| v.strip_prefix(':')))
        };
        let number = |name: &str| field(name).and_then(|v| v.parse::<u64>().ok()).unwrap_or(0);
        let frames = number("FRAMES");
        // A pregap chdman stored (its type starts with V) is in the frames, before the track.
        let stored_pregap = if field("PGTYPE").is_some_and(|t| t.starts_with('V')) { number("PREGAP") } else { 0 };
        let offset = match field("TYPE").unwrap_or("") {
            "MODE1" | "MODE2_FORM1" => Some(0),
            "MODE1_RAW" => Some(16),
            "MODE2_RAW" => Some(24),
            "MODE2" | "MODE2_FORM_MIX" => Some(8),
            _ => None,
        };
        if let Some(offset) = offset {
            if frames <= stored_pregap {
                return Err("its data track is empty".into());
            }
            return Ok(CdTrack { first: frame + stored_pregap, sectors: frames - stored_pregap, offset });
        }
        frame += frames.div_ceil(TRACK_PADDING) * TRACK_PADDING;
    }
    Err("it holds no data track".into())
}

/// Whether `path` names a CHD image (by its extension, as the pickers and the scan see it).
pub fn is_chd(path: &Path) -> bool {
    path.extension().is_some_and(|e| e.eq_ignore_ascii_case("chd"))
}

/// Whether `file` begins as a CHD image does ("MComprHD"), whatever it is called: a picker can
/// give a name without its extension. Read in place, so the file's position is left as it was;
/// false when it cannot be read there (a pipe).
pub fn starts_as_chd(file: &File) -> bool {
    let mut magic = [0u8; 8];
    #[cfg(unix)]
    let read = std::os::unix::fs::FileExt::read_exact_at(file, &mut magic, 0).is_ok();
    #[cfg(windows)]
    let read = std::os::windows::fs::FileExt::seek_read(file, &mut magic, 0).is_ok_and(|n| n == magic.len());
    read && &magic == b"MComprHD"
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
    /// A CD's data track, and the bytes of each of its frames (sector and subcode); None for a
    /// DVD, whose bytes are the image's.
    cd: Option<(CdTrack, u64)>,
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
        let refs: Vec<_> = chd.metadata_refs().filter(|m| KnownMetadata::is_cdrom(m.metatag())).collect();
        let cd = if refs.is_empty() {
            None
        } else {
            let (track2, track) = (KnownMetadata::CdRomTrack2.metatag(), KnownMetadata::CdRomTrack.metatag());
            if refs.iter().any(|m| m.metatag() != track2 && m.metatag() != track) {
                return Err("this CHD image is of a GD-ROM or an old CD format this launcher cannot read".into());
            }
            let mut tracks = Vec::new();
            for m in &refs {
                let read = m.read(chd.inner()).map_err(|e| format!("this CHD image's track list: {e}"))?;
                tracks.push(String::from_utf8_lossy(&read.value).trim_end_matches('\0').to_string());
            }
            let frame = u64::from(chd.header().unit_bytes());
            let track = first_data_track(&tracks).map_err(|e| format!("this CD's CHD image cannot be read: {e}"))?;
            if frame < track.offset + SECTOR {
                return Err("this CD's CHD image has frames too small for its sectors".into());
            }
            Some((track, frame))
        };
        let hunk = chd.get_hunksized_buffer();
        if hunk.is_empty() {
            // A hunk size of zero: the image cannot be read (and reading it would divide by
            // that size).
            return Err("this CHD image has no hunk size: it cannot be read".into());
        }
        let image = ChdImage { chd, hunk, compressed: Vec::new(), current: None, cd };
        if let Some((track, frame)) = image.cd {
            if (track.first + track.sectors) * frame > image.chd.header().logical_bytes() {
                return Err("this CD's CHD image is shorter than its track list says".into());
            }
        }
        Ok(image)
    }

    /// The size of the image it holds.
    pub fn len(&self) -> u64 {
        match self.cd {
            Some((track, _)) => track.sectors * SECTOR,
            None => self.chd.header().logical_bytes(),
        }
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
        let Some((track, frame)) = self.cd else { return self.read_raw(offset, buffer) };
        // A CD: each sector's 2048 bytes from its frame.
        let mut done = 0;
        while done < buffer.len() {
            let at = offset + done as u64;
            let (sector, within) = (at / SECTOR, at % SECTOR);
            let n = (buffer.len() - done).min((SECTOR - within) as usize);
            let from = (track.first + sector) * frame + track.offset + within;
            self.read_raw(from, &mut buffer[done..done + n])?;
            done += n;
        }
        Ok(())
    }

    /// `read_at` of the CHD's own bytes (a CD's frames as they are stored).
    fn read_raw(&mut self, offset: u64, buffer: &mut [u8]) -> Result<(), String> {
        let size = self.hunk.len() as u64;
        if size == 0 {
            return Err("this CHD image has no hunk size: it cannot be read".into());
        }
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
        let mut block = vec![0u8; 1 << 20];
        while written < total {
            let n = if self.cd.is_some() {
                let n = (total - written).min(block.len() as u64) as usize;
                self.read_at(written, &mut block[..n])?;
                out.write_all(&block[..n]).map_err(|e| e.to_string())?;
                n
            } else {
                self.load(index)?;
                let n = (total - written).min(self.hunk.len() as u64) as usize;
                out.write_all(&self.hunk[..n]).map_err(|e| e.to_string())?;
                index += 1;
                n
            };
            written += n as u64;
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
        chd_with(data, hunk, 2048, &[])
    }

    /// As `chd_of`, in units of `unit` bytes, with CD track entries (CHT2) `tracks`.
    fn chd_with(data: &[u8], hunk: u32, unit: u32, tracks: &[&str]) -> Vec<u8> {
        let hunks = data.len().div_ceil(hunk as usize);
        let map_at = 124u64;
        let meta_at = map_at + 4 * hunks as u64;
        let mut meta = Vec::new();
        for (i, track) in tracks.iter().enumerate() {
            let value = [track.as_bytes(), b"\0"].concat();
            let next = if i + 1 < tracks.len() { meta_at + (meta.len() + 16 + value.len()) as u64 } else { 0 };
            meta.extend_from_slice(b"CHT2");
            meta.extend_from_slice(&(value.len() as u32 | 1 << 24).to_be_bytes());
            meta.extend_from_slice(&next.to_be_bytes());
            meta.extend_from_slice(&value);
        }
        let data_at = (meta_at + meta.len() as u64).div_ceil(u64::from(hunk)) * u64::from(hunk);
        let mut out = Vec::new();
        out.extend_from_slice(b"MComprHD");
        out.extend_from_slice(&124u32.to_be_bytes());
        out.extend_from_slice(&5u32.to_be_bytes());
        out.extend_from_slice(&[0u8; 16]); // no compressors
        out.extend_from_slice(&(data.len() as u64).to_be_bytes());
        out.extend_from_slice(&map_at.to_be_bytes());
        out.extend_from_slice(&(if tracks.is_empty() { 0 } else { meta_at }).to_be_bytes());
        out.extend_from_slice(&hunk.to_be_bytes());
        out.extend_from_slice(&unit.to_be_bytes());
        out.extend_from_slice(&[0u8; 60]); // the SHA-1s
        for i in 0..hunks as u64 {
            out.extend_from_slice(&((data_at / u64::from(hunk) + i) as u32).to_be_bytes());
        }
        out.extend_from_slice(&meta);
        out.resize(data_at as usize, 0);
        out.extend_from_slice(data);
        out.resize(data_at as usize + hunks * hunk as usize, 0);
        out
    }

    fn data(len: usize) -> Vec<u8> {
        (0..len).map(|i| (i * 7 % 251) as u8).collect()
    }

    #[test]
    fn an_image_without_a_hunk_size_is_refused() {
        let mut image = chd_of(&data(4096), 4096);
        // The header's hunk size: zero. Such an image cannot be read (reading it would
        // divide by that size), so it is refused rather than read.
        image[56..60].copy_from_slice(&0u32.to_be_bytes());
        let err = match ChdImage::new(Cursor::new(image)) {
            Ok(_) => panic!("an image without a hunk size is read"),
            Err(e) => e,
        };
        assert!(!err.is_empty());
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
    fn knows_one_by_its_first_bytes() {
        let dir = tempfile::tempdir().unwrap();
        let (chd, iso) = (dir.path().join("game"), dir.path().join("game.iso"));
        std::fs::write(&chd, chd_of(&data(4096), 4096)).unwrap();
        std::fs::write(&iso, data(4096)).unwrap();
        let file = File::open(&chd).unwrap();
        assert!(starts_as_chd(&file));
        assert!(!starts_as_chd(&File::open(&iso).unwrap()));
        // The file is still read from its start.
        assert!(ChdImage::new(file).is_ok());
    }

    /// `sectors` of `plain` as chdman createcd stores them: frames of 2448 bytes, each sector's
    /// 2048 bytes at `offset`, the rest (sync, header, error codes, subcode) filled with junk.
    fn cd_frames(plain: &[u8], offset: usize) -> Vec<u8> {
        let mut out = Vec::new();
        for sector in plain.chunks(2048) {
            let mut frame = vec![0xa5u8; 2448];
            frame[offset..offset + 2048].copy_from_slice(sector);
            out.extend_from_slice(&frame);
        }
        out
    }

    #[test]
    fn unpacks_a_cds_data_track() {
        let dir = tempfile::tempdir().unwrap();
        let plain = data(10 * 2048);
        for (kind, offset) in [("MODE2_RAW", 24), ("MODE1_RAW", 16), ("MODE1", 0), ("MODE2", 8)] {
            let track =
                format!("TRACK:1 TYPE:{kind} SUBTYPE:NONE FRAMES:10 PREGAP:0 PGTYPE:MODE1 PGSUB:NONE POSTGAP:0");
            let chd = chd_with(&cd_frames(&plain, offset), 8 * 2448, 2448, &[&track]);
            let mut image = ChdImage::new(Cursor::new(chd.clone())).unwrap();
            assert_eq!(image.len(), plain.len() as u64, "{kind}");
            let mut middle = [0u8; 3000];
            image.read_at(2000, &mut middle).unwrap();
            assert_eq!(middle[..], plain[2000..5000], "{kind}");
            let dest = dir.path().join(format!("{kind}.iso"));
            to_iso(Cursor::new(chd), &dest, |_, _| {}).unwrap();
            assert_eq!(std::fs::read(&dest).unwrap(), plain, "{kind}");
        }
    }

    #[test]
    fn finds_the_data_track_after_padding_and_a_stored_pregap() {
        let tracks = [
            "TRACK:1 TYPE:AUDIO SUBTYPE:NONE FRAMES:6 PREGAP:0 PGTYPE:MODE1 PGSUB:NONE POSTGAP:0".to_string(),
            "TRACK:2 TYPE:MODE2_RAW SUBTYPE:NONE FRAMES:160 PREGAP:150 PGTYPE:VMODE2_RAW PGSUB:NONE POSTGAP:0"
                .to_string(),
        ];
        assert_eq!(first_data_track(&tracks), Ok(CdTrack { first: 8 + 150, sectors: 10, offset: 24 }));
        assert!(first_data_track(&tracks[..1]).is_err(), "audio only");
        // A CHD whose frames end before its track list's do is refused, not read past.
        let track = "TRACK:1 TYPE:MODE1 SUBTYPE:NONE FRAMES:99 PREGAP:0 PGTYPE:MODE1 PGSUB:NONE POSTGAP:0";
        assert!(ChdImage::new(Cursor::new(chd_with(&cd_frames(&data(2048), 0), 8 * 2448, 2448, &[track]))).is_err());
    }

    #[test]
    fn refuses_what_is_not_one() {
        let dir = tempfile::tempdir().unwrap();
        let dest = dir.path().join("game.iso");
        assert!(to_iso(Cursor::new(b"not a CHD at all, just bytes".repeat(10)), &dest, |_, _| {}).is_err());
        assert!(!dest.exists() && !dest.with_extension("partial").exists());
    }
}
