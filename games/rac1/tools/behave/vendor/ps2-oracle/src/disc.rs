//! Reading the game's code out of the user's own disc image: ISO 9660, `SYSTEM.CNF`, PS2 ELFs, and putting
//! a program into the machine.
//!
//! The disc is the *only* source of the original's code. Nothing it holds is ever copied into a
//! repository — not a byte of it, not a table extracted from it — which is why every one of these
//! functions takes a path and reads at test time. Which disc, which files and which load addresses is the
//! caller's business (a config file, a game crate's constants): nothing here knows any game.
//!
//! The reader is deliberately minimal: one directory walk, no Joliet, no Rock Ridge, no multi-extent
//! files. PS2 discs are plain ISO 9660 with 2048-byte sectors, and the files wanted here are the boot
//! ELF and a handful of overlays.

use crate::Ee;
use std::collections::BTreeMap;
use std::io::{Read, Seek, SeekFrom};
use std::path::{Path, PathBuf};

const SECTOR: u64 = 2048;

/// Where the game's files are read from: the ISO image, or a directory it was already extracted to.
pub enum Disc {
    Iso { file: std::fs::File, dir: BTreeMap<String, (u64, usize)>, path: PathBuf },
    Dir(PathBuf),
}

impl Disc {
    /// The ISO image if it exists, else the directory it was extracted to. The error names both, because a
    /// test's whole job on a machine with no disc is to say so and skip.
    pub fn open(iso: &Path, extracted: &Path) -> Result<Disc, String> {
        if iso.exists() {
            return Disc::iso(iso);
        }
        if extracted.is_dir() {
            return Ok(Disc::Dir(extracted.to_path_buf()));
        }
        Err(format!("no disc: {} does not exist, and neither does the extracted copy in {}",
                    iso.display(), extracted.display()))
    }

    /// Open an ISO 9660 image and index its files.
    pub fn iso(path: &Path) -> Result<Disc, String> {
        let mut file = std::fs::File::open(path).map_err(|e| format!("{}: {e}", path.display()))?;
        let mut dir = BTreeMap::new();
        // the primary volume descriptor sits at sector 16; its root directory record is at +156
        let pvd = read_at(&mut file, 16 * SECTOR, SECTOR as usize)?;
        if &pvd[1..6] != b"CD001" {
            return Err(format!("{}: not an ISO 9660 image (no CD001 at sector 16)", path.display()));
        }
        let (extent, size) = (le32(&pvd[156 + 2..]), le32(&pvd[156 + 10..]));
        walk(&mut file, extent, size as usize, "", &mut dir, 0)?;
        Ok(Disc::Iso { file, dir, path: path.to_path_buf() })
    }

    /// Where this disc came from, for messages.
    pub fn describe(&self) -> String {
        match self {
            Disc::Iso { path, dir, .. } => format!("{} ({} files)", path.display(), dir.len()),
            Disc::Dir(p) => format!("{} (extracted)", p.display()),
        }
    }

    /// One file's bytes. The name is a disc path without the leading slash: `SYSTEM.CNF`,
    /// `DATA/GAME.BIN`. Case and the ISO `;1` version suffix are ignored.
    pub fn read(&mut self, name: &str) -> Result<Vec<u8>, String> {
        let want = name.trim_start_matches('/').to_uppercase().replace('\\', "/");
        match self {
            Disc::Dir(root) => {
                let p = root.join(&want);
                std::fs::read(&p).map_err(|e| format!("{}: {e}", p.display()))
            }
            Disc::Iso { file, dir, path } => {
                let (at, size) = *dir.get(&want)
                    .ok_or_else(|| format!("{} has no file {want:?}", path.display()))?;
                read_at(file, at, size)
            }
        }
    }

    /// Every file on the disc, as `(path, size)`.
    pub fn files(&self) -> Vec<(String, usize)> {
        match self {
            Disc::Iso { dir, .. } => dir.iter().map(|(k, v)| (k.clone(), v.1)).collect(),
            Disc::Dir(root) => {
                let mut out = Vec::new();
                let mut stack = vec![root.clone()];
                while let Some(d) = stack.pop() {
                    for e in std::fs::read_dir(&d).into_iter().flatten().flatten() {
                        let p = e.path();
                        if p.is_dir() {
                            stack.push(p);
                        } else if let Ok(m) = e.metadata() {
                            let rel = p.strip_prefix(root).unwrap_or(&p).to_string_lossy().to_uppercase();
                            out.push((rel, m.len() as usize));
                        }
                    }
                }
                out.sort();
                out
            }
        }
    }

    /// `SYSTEM.CNF`'s `BOOT2` line as a disc path and as a serial: `cdrom0:\SLUS_200.00;1` gives
    /// `("SLUS_200.00", "SLUS-20000")`.
    pub fn boot2(&mut self) -> Result<(String, String), String> {
        let cnf = String::from_utf8_lossy(&self.read("SYSTEM.CNF")?).to_string();
        let line = cnf.lines().find(|l| l.trim_start().to_uppercase().starts_with("BOOT2"))
            .ok_or("SYSTEM.CNF has no BOOT2 line")?;
        let v = line.split_once('=').ok_or("SYSTEM.CNF: malformed BOOT2 line")?.1.trim();
        let name = v.rsplit(['\\', '/', ':']).next().unwrap_or(v).split(';').next().unwrap_or(v).trim();
        Ok((name.to_uppercase(), serial_of(name)))
    }

    /// Refuse to go on unless the disc's boot serial is `want` (`SLUS-20000` or `SLUS_200.00`).
    ///
    /// Every address in an oracle's address list belongs to one build of one game. Running them against
    /// another region or revision does not fail loudly — it silently calls the wrong code and reports
    /// numbers that look plausible, which is far worse than an error.
    pub fn check_serial(&mut self, want: &str) -> Result<String, String> {
        let (_, found) = self.boot2()?;
        let want = serial_of(want);
        if want.is_empty() {
            return Err("no serial to check the disc against".into());
        }
        if found != want {
            return Err(format!("this disc is {found}, not {want}\n\
                                Addresses only mean anything for the build they were found in."));
        }
        Ok(found)
    }
}

/// `SLUS_200.00` → `SLUS-20000`, the form PINE and everyone else prints.
pub fn serial_of(boot_name: &str) -> String {
    let s: String = boot_name.chars().filter(|c| c.is_ascii_alphanumeric()).collect::<String>().to_uppercase();
    if s.len() > 4 { format!("{}-{}", &s[..4], &s[4..]) } else { s }
}

fn read_at(file: &mut std::fs::File, at: u64, size: usize) -> Result<Vec<u8>, String> {
    let mut buf = vec![0u8; size];
    file.seek(SeekFrom::Start(at)).map_err(|e| e.to_string())?;
    file.read_exact(&mut buf).map_err(|e| format!("read {size} bytes at {at}: {e}"))?;
    Ok(buf)
}

fn le32(b: &[u8]) -> u32 {
    u32::from_le_bytes([b[0], b[1], b[2], b[3]])
}

/// Index one directory extent, recursing into subdirectories.
fn walk(file: &mut std::fs::File, extent: u32, size: usize, prefix: &str,
        out: &mut BTreeMap<String, (u64, usize)>, depth: u32) -> Result<(), String> {
    if depth > 8 {
        return Ok(()); // ISO 9660 allows 8 levels; deeper means a loop
    }
    let data = read_at(file, extent as u64 * SECTOR, size.max(SECTOR as usize))?;
    let mut i = 0usize;
    while i + 33 < data.len() {
        let len = data[i] as usize;
        if len == 0 {
            // the rest of this sector is padding
            i = (i / SECTOR as usize + 1) * SECTOR as usize;
            continue;
        }
        let (ext, sz, flags, nlen) = (le32(&data[i + 2..]), le32(&data[i + 10..]), data[i + 25], data[i + 32] as usize);
        let name = String::from_utf8_lossy(&data[i + 33..i + 33 + nlen]).to_string();
        let name = name.split(';').next().unwrap_or(&name).trim_end_matches('.').to_uppercase();
        let is_dir = flags & 2 != 0;
        if nlen == 1 && (data[i + 33] == 0 || data[i + 33] == 1) {
            // "." and ".."
        } else if is_dir {
            let sub = if prefix.is_empty() { name.clone() } else { format!("{prefix}/{name}") };
            walk(file, ext, sz as usize, &sub, out, depth + 1)?;
        } else {
            let path = if prefix.is_empty() { name } else { format!("{prefix}/{name}") };
            out.insert(path, (ext as u64 * SECTOR, sz as usize));
        }
        i += len;
    }
    Ok(())
}

/// A PS2 executable's loadable segments, as `(virtual address, bytes)`.
///
/// PS2 ELFs are 32-bit little-endian MIPS with everything in `PT_LOAD` segments at their final
/// addresses — no relocation, no dynamic linking. `memsz` beyond `filesz` is `.bss`, which starts
/// zeroed, so only the file bytes are returned.
pub fn elf_segments(elf: &[u8]) -> Result<Vec<(u32, &[u8])>, String> {
    if elf.len() < 0x34 || &elf[..4] != b"\x7fELF" {
        return Err("not an ELF (no \\x7fELF magic) — is it the boot file?".into());
    }
    if elf[4] != 1 || elf[5] != 1 {
        return Err("not a 32-bit little-endian ELF, so not a PS2 executable".into());
    }
    let (phoff, phentsize, phnum) =
        (le32(&elf[0x1c..]) as usize, u16::from_le_bytes([elf[0x2a], elf[0x2b]]) as usize,
         u16::from_le_bytes([elf[0x2c], elf[0x2d]]) as usize);
    let mut out = Vec::new();
    for k in 0..phnum {
        let p = phoff + phentsize.max(32) * k;
        if p + 32 > elf.len() {
            return Err("ELF program header table runs past the end of the file".into());
        }
        let (ptype, off, va, filesz) = (le32(&elf[p..]), le32(&elf[p + 4..]) as usize, le32(&elf[p + 8..]),
                                        le32(&elf[p + 16..]) as usize);
        if ptype == 1 && filesz > 0 {
            if off + filesz > elf.len() {
                return Err(format!("ELF segment at {va:#x} runs past the end of the file"));
            }
            out.push((va, &elf[off..off + filesz]));
        }
    }
    if out.is_empty() {
        Err("ELF has no loadable segments".into())
    } else {
        Ok(out)
    }
}

impl Ee {
    /// `ram` bytes of zeroed RAM with a boot ELF's segments loaded, then each `(load address, bytes)` overlay
    /// copied raw over it in order — what a game's own overlay loader does.
    pub fn from_program(ram: usize, elf: &[u8], overlays: &[(u32, &[u8])]) -> Result<Ee, String> {
        let mut ee = Ee::bare(ram);
        for (va, bytes) in elf_segments(elf)? {
            ee.load(va, bytes);
        }
        for &(va, bytes) in overlays {
            ee.load(va, bytes);
        }
        Ok(ee)
    }

    /// A RAM image saved from a running game (PCSX2's "save EE memory", or the EE memory out of a save
    /// state): a flat dump of EE RAM from address 0.
    ///
    /// The answer whenever a function reads a global you have not identified. Two things to watch: *when*
    /// the dump was taken (it is a moment, with whatever mode, level and objects were live then), and *which
    /// build* it came from — [`Ee::holds`] answers the second one.
    pub fn from_ram_file(path: impl AsRef<Path>) -> Result<Ee, String> {
        let p = path.as_ref();
        let ram = std::fs::read(p).map_err(|e| format!("cannot read the RAM image {}: {e}", p.display()))?;
        if ram.len() < 0x10_0000 {
            return Err(format!("{} is only {} bytes — that is not an EE RAM image (expected 32 MB)",
                               p.display(), ram.len()));
        }
        Ok(Ee::with_ram(ram))
    }

    /// Whether memory at `va` holds `bytes` — a RAM image checked against the disc's overlay (the first few KB
    /// of its code are plenty), so a dump from another build is caught before it calls the wrong code.
    pub fn holds(&self, va: u32, bytes: &[u8]) -> bool {
        bytes.iter().enumerate().all(|(k, &b)| self.u8_at(va + k as u32) == b)
    }
}
