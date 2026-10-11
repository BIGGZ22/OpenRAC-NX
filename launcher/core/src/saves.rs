// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

//! Save file management prototype for OpenRAC games.
//!
//! The PC runtime answers memory card requests from a directory on the host:
//! - macOS: `~/Library/Application Support/OpenRAC/memcard/<SERIAL>`
//! - Linux/other: `~/.local/share/openrac/memcard/<SERIAL>` (or `$XDG_DATA_HOME/openrac/memcard/<SERIAL>`)
//!
//! Inside each game's serial folder, the PS2 memory card folder (e.g. `BESCES-50916RATCHET`)
//! holds `icon.sys`, `static.ico`, and save files `save0.bin` ... `save4.bin`.
//! This module reads, backs up, restores, and inspects these save files.

use serde::{Deserialize, Serialize};
use std::fs;
use std::path::{Path, PathBuf};

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SaveSlotInfo {
    pub slot_index: usize,
    pub filename: String,
    pub path: String,
    pub size: u64,
    pub exists: bool,
    pub is_empty: bool,
    /// Number of bolts collected in this save file.
    pub bolts: Option<u32>,
    /// Planet ID where save was created.
    pub planet_id: Option<u32>,
    /// Human-readable planet name (e.g. Veldin, Novalis, Kerwan).
    pub planet_name: Option<String>,
    /// ISO-8601 formatted timestamp if decoded from in-game save metadata or file mtime.
    pub timestamp: Option<String>,
    pub modified_millis: Option<u64>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SaveBackupInfo {
    pub name: String,
    pub path: String,
    pub created_millis: u64,
    pub total_size: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct GameSaveStatus {
    pub serial: String,
    pub memcard_dir: String,
    pub exists: bool,
    pub game_folder_name: Option<String>,
    pub title: Option<String>,
    pub slots: Vec<SaveSlotInfo>,
    pub backups: Vec<SaveBackupInfo>,
}

/// Returns the host directory where OpenRAC stores memory cards for the given serial.
pub fn get_memcard_dir(serial: &str) -> PathBuf {
    #[cfg(target_os = "macos")]
    let base = dirs_home().join("Library/Application Support/OpenRAC");
    #[cfg(not(target_os = "macos"))]
    let base = std::env::var_os("XDG_DATA_HOME")
        .map(PathBuf::from)
        .unwrap_or_else(|| dirs_home().join(".local/share"))
        .join("openrac");

    base.join("memcard").join(serial)
}

fn dirs_home() -> PathBuf {
    std::env::var_os("HOME").map(PathBuf::from).unwrap_or_else(|| PathBuf::from("."))
}

/// Decode BCD (Binary Coded Decimal) byte: e.g. 0x26 -> 26.
fn decode_bcd(byte: u8) -> Option<u32> {
    let high = (byte >> 4) as u32;
    let low = (byte & 0x0F) as u32;
    if high <= 9 && low <= 9 {
        Some(high * 10 + low)
    } else {
        None
    }
}

/// The save's clock (record 4): the console clock as sceCdReadClock gives it, BCD
/// `stat, second, minute, hour, pad, day, month, year`. None when unset or invalid.
fn parse_save_clock(clock: &[u8]) -> Option<String> {
    if clock.len() < 8 || clock.iter().all(|&b| b == 0) {
        return None;
    }
    let s = decode_bcd(clock[1])?;
    let m = decode_bcd(clock[2])?;
    let h = decode_bcd(clock[3])?;
    let d = decode_bcd(clock[5])?;
    let mo = decode_bcd(clock[6])?;
    let y = decode_bcd(clock[7])?;
    if !(1..=12).contains(&mo) || !(1..=31).contains(&d) || h > 23 || m > 59 || s > 59 {
        return None;
    }
    Some(format!("{:04}-{mo:02}-{d:02} {h:02}:{m:02}:{s:02}", 2000 + y))
}

/// Ratchet & Clank 1 planet names by level number (games/rac1/pal/docs/OVERLAYS.md).
pub const RAC1_PLANETS: &[&str] = &[
    "Veldin", // 0
    "Novalis", // 1
    "Aridia", // 2
    "Kerwan", // 3
    "Eudora", // 4
    "Rilgar", // 5
    "Blarg Station", // 6
    "Umbris", // 7
    "Batalia", // 8
    "Gaspar", // 9
    "Orxon", // 10
    "Pokitaru", // 11
    "Hoven", // 12
    "Gemlik Base", // 13
    "Oltanis", // 14
    "Quartu", // 15
    "Kalebo III", // 16
    "Drek's Fleet", // 17
    "Veldin", // 18
];

pub fn get_planet_name(serial: &str, planet_id: u32) -> Option<String> {
    if serial.contains("509.16") || serial.contains("971.99") {
        if let Some(&name) = RAC1_PLANETS.get(planet_id as usize) {
            return Some(name.to_string());
        }
    }
    Some(format!("Planet #{planet_id}"))
}

#[derive(Debug, Default)]
struct SaveGameplayInfo {
    bolts: Option<u32>,
    planet_id: Option<u32>,
    /// Record 0 is -1 in a slot never saved to.
    empty: bool,
    timestamp: Option<String>,
}

/// Parses internal records from serialized Ratchet & Clank save data:
/// - Record 0: Current planet ID (D_0015EE84)
/// - Record 1: Current bolt count (gBolts, D_0015EE98)
/// - Record 4: the clock when saved (parse_save_clock)
fn parse_save_gameplay_info(bytes: &[u8]) -> SaveGameplayInfo {
    if bytes.len() < 24 {
        return SaveGameplayInfo::default();
    }

    let mut info = SaveGameplayInfo::default();
    let mut cursor = 16; // Skip size_a (4), size_b (4), prep_size (4), prep_csum (4)

    while cursor + 8 <= bytes.len() {
        let rec_id = i32::from_le_bytes(bytes[cursor..cursor + 4].try_into().unwrap());
        let rec_size = i32::from_le_bytes(bytes[cursor + 4..cursor + 8].try_into().unwrap());

        if rec_id == -1 || rec_size < 0 {
            break;
        }

        let sz = rec_size as usize;
        let data_start = cursor + 8;
        let data_end = data_start + sz;

        if data_end > bytes.len() {
            break;
        }

        if rec_id == 0 && sz >= 4 {
            let pid = u32::from_le_bytes(bytes[data_start..data_start + 4].try_into().unwrap());
            info.empty = pid == u32::MAX;
            if pid < 50 {
                info.planet_id = Some(pid);
            }
        } else if rec_id == 1 && sz >= 4 {
            let b = u32::from_le_bytes(bytes[data_start..data_start + 4].try_into().unwrap());
            info.bolts = Some(b);
        } else if rec_id == 4 {
            info.timestamp = parse_save_clock(&bytes[data_start..data_end]);
        }

        cursor = data_start + ((sz + 3) & !3);
    }

    info
}
pub fn inspect_saves(serial: &str) -> GameSaveStatus {
    let card_dir = get_memcard_dir(serial);
    let exists = card_dir.is_dir();

    let mut game_folder_name = None;
    let mut title = None;
    let mut slots = Vec::new();
    let mut backups = Vec::new();

    if exists {
        // Find the game folder inside the serial directory (e.g. BESCES-50916RATCHET)
        if let Ok(entries) = fs::read_dir(&card_dir) {
            for entry in entries.flatten() {
                let path = entry.path();
                let file_name = entry.file_name().to_string_lossy().to_string();

                if file_name.starts_with(".backup_") && path.is_dir() {
                    let total_size = dir_size(&path);
                    let mtime = path
                        .metadata()
                        .ok()
                        .and_then(|m| m.modified().ok())
                        .and_then(|t| t.duration_since(std::time::UNIX_EPOCH).ok())
                        .map(|d| d.as_millis() as u64)
                        .unwrap_or(0);

                    backups.push(SaveBackupInfo {
                        name: file_name,
                        path: path.to_string_lossy().to_string(),
                        created_millis: mtime,
                        total_size,
                    });
                } else if path.is_dir() && game_folder_name.is_none() {
                    game_folder_name = Some(file_name);
                }
            }
        }

        if let Some(ref folder) = game_folder_name {
            let target_dir = card_dir.join(folder);

            // Read Shift-JIS title from icon.sys if present
            let icon_sys = target_dir.join("icon.sys");
            if let Ok(bytes) = fs::read(&icon_sys) {
                if bytes.len() >= 0x100 {
                    // Title string starts around 0xc0
                    let raw_title = &bytes[0xc0..std::cmp::min(bytes.len(), 0x100)];
                    let null_end = raw_title.iter().position(|&b| b == 0).unwrap_or(raw_title.len());
                    title = decode_shift_jis_or_ascii(&raw_title[..null_end]);
                }
            }

            // Inspect slots save0.bin through save4.bin
            for i in 0..5 {
                let fname = format!("save{i}.bin");
                let save_path = target_dir.join(&fname);
                let (exists, size, mtime, is_empty, timestamp, bolts, planet_id, planet_name) = if save_path.is_file() {
                    let meta = save_path.metadata().ok();
                    let sz = meta.as_ref().map(|m| m.len()).unwrap_or(0);
                    let mt = meta
                        .as_ref()
                        .and_then(|m| m.modified().ok())
                        .and_then(|t| t.duration_since(std::time::UNIX_EPOCH).ok())
                        .map(|d| d.as_millis() as u64);

                    let data = fs::read(&save_path).unwrap_or_default();
                    let g = parse_save_gameplay_info(&data);
                    let empty = g.empty || g.timestamp.is_none();
                    let ts = if empty { None } else { g.timestamp.clone() };

                    let (b, pid, pname) = if !empty {
                        let pn = g.planet_id.and_then(|p| get_planet_name(serial, p));
                        (g.bolts, g.planet_id, pn)
                    } else {
                        (None, None, None)
                    };

                    (true, sz, mt, empty, ts, b, pid, pname)
                } else {
                    (false, 0, None, true, None, None, None, None)
                };

                slots.push(SaveSlotInfo {
                    slot_index: i,
                    filename: fname,
                    path: save_path.to_string_lossy().to_string(),
                    size,
                    exists,
                    is_empty,
                    bolts,
                    planet_id,
                    planet_name,
                    timestamp,
                    modified_millis: mtime,
                });
            }
        }
    }

    backups.sort_by_key(|b| std::cmp::Reverse(b.created_millis));

    GameSaveStatus {
        serial: serial.to_string(),
        memcard_dir: card_dir.to_string_lossy().to_string(),
        exists,
        game_folder_name,
        title,
        slots,
        backups,
    }
}

/// Creates a snapshot backup of the current save state.
pub fn backup_saves(serial: &str, note: Option<&str>) -> Result<SaveBackupInfo, String> {
    let card_dir = get_memcard_dir(serial);
    if !card_dir.is_dir() {
        return Err(format!("no save directory for {serial}"));
    }

    let timestamp =
        std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).map_err(|e| e.to_string())?.as_secs();

    let backup_name = match note {
        Some(n) if !n.trim().is_empty() => {
            let clean: String = n.chars().filter(|c| c.is_alphanumeric() || *c == '_' || *c == '-').collect();
            format!(".backup_{timestamp}_{clean}")
        }
        _ => format!(".backup_{timestamp}"),
    };

    let backup_path = card_dir.join(&backup_name);
    fs::create_dir_all(&backup_path).map_err(|e| e.to_string())?;

    for entry in fs::read_dir(&card_dir).map_err(|e| e.to_string())?.flatten() {
        let p = entry.path();
        let fname = entry.file_name().to_string_lossy().to_string();
        if p.is_dir() && !fname.starts_with(".backup_") {
            copy_dir_recursive(&p, &backup_path.join(&fname))?;
        }
    }

    let total_size = dir_size(&backup_path);
    let mtime = backup_path
        .metadata()
        .ok()
        .and_then(|m| m.modified().ok())
        .and_then(|t| t.duration_since(std::time::UNIX_EPOCH).ok())
        .map(|d| d.as_millis() as u64)
        .unwrap_or(0);

    Ok(SaveBackupInfo {
        name: backup_name,
        path: backup_path.to_string_lossy().to_string(),
        created_millis: mtime,
        total_size,
    })
}

/// Restores a snapshot backup, replacing current saves.
pub fn restore_backup(serial: &str, backup_name: &str) -> Result<(), String> {
    let card_dir = get_memcard_dir(serial);
    let backup_path = card_dir.join(backup_name);
    if !backup_path.is_dir() {
        return Err(format!("backup {backup_name} not found"));
    }

    // First, automatically backup current state as safety fallback
    let _ = backup_saves(serial, Some("auto_safety_before_restore"));

    // Remove current non-backup directories
    for entry in fs::read_dir(&card_dir).map_err(|e| e.to_string())?.flatten() {
        let p = entry.path();
        let fname = entry.file_name().to_string_lossy().to_string();
        if p.is_dir() && !fname.starts_with(".backup_") {
            fs::remove_dir_all(&p).map_err(|e| e.to_string())?;
        }
    }

    // Copy backup contents into card_dir
    for entry in fs::read_dir(&backup_path).map_err(|e| e.to_string())?.flatten() {
        let p = entry.path();
        let fname = entry.file_name().to_string_lossy().to_string();
        if p.is_dir() {
            copy_dir_recursive(&p, &card_dir.join(&fname))?;
        }
    }

    Ok(())
}

fn copy_dir_recursive(src: &Path, dst: &Path) -> Result<(), String> {
    fs::create_dir_all(dst).map_err(|e| e.to_string())?;
    for entry in fs::read_dir(src).map_err(|e| e.to_string())?.flatten() {
        let sp = entry.path();
        let dp = dst.join(entry.file_name());
        if sp.is_dir() {
            copy_dir_recursive(&sp, &dp)?;
        } else {
            fs::copy(&sp, &dp).map_err(|e| e.to_string())?;
        }
    }
    Ok(())
}

fn dir_size(dir: &Path) -> u64 {
    let mut total = 0;
    if let Ok(entries) = fs::read_dir(dir) {
        for entry in entries.flatten() {
            let p = entry.path();
            if p.is_dir() {
                total += dir_size(&p);
            } else if let Ok(meta) = p.metadata() {
                total += meta.len();
            }
        }
    }
    total
}

/// Fallback Shift-JIS decoder for wide ASCII characters used in PS2 save icons
/// (e.g. 0x82 0x71 -> 'R', 0x81 0x40 -> ' ').
fn decode_shift_jis_or_ascii(bytes: &[u8]) -> Option<String> {
    let mut s = String::new();
    let mut i = 0;
    while i < bytes.len() {
        let b = bytes[i];
        if b == 0 {
            break;
        }
        if b == 0x81 && i + 1 < bytes.len() {
            let b2 = bytes[i + 1];
            if b2 == 0x40 {
                s.push(' ');
            } else if b2 == 0x95 {
                s.push('&');
            }
            i += 2;
        } else if b == 0x82 && i + 1 < bytes.len() {
            let b2 = bytes[i + 1];
            if (0x60..=0x79).contains(&b2) {
                // 'A'..='Z'
                s.push((b'A' + (b2 - 0x60)) as char);
            } else if (0x81..=0x9a).contains(&b2) {
                // 'a'..='z'
                s.push((b'a' + (b2 - 0x81)) as char);
            } else {
                s.push('?');
            }
            i += 2;
        } else if b < 0x80 {
            s.push(b as char);
            i += 1;
        } else {
            i += 1;
        }
    }
    if s.trim().is_empty() {
        None
    } else {
        Some(s.trim().to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A save as the game writes it: the four size words, then records {id, size, data}.
    fn save(records: &[(i32, &[u8])]) -> Vec<u8> {
        let mut out = vec![0u8; 16];
        for (id, data) in records {
            out.extend_from_slice(&id.to_le_bytes());
            out.extend_from_slice(&(data.len() as i32).to_le_bytes());
            out.extend_from_slice(data);
            out.resize(out.len().div_ceil(4) * 4, 0);
        }
        out.extend_from_slice(&(-1i32).to_le_bytes());
        out
    }

    #[test]
    fn reads_planet_bolts_and_clock() {
        let clock = [0x00, 0x14, 0x14, 0x23, 0x00, 0x10, 0x10, 0x26];
        let data = save(&[(0, &3u32.to_le_bytes()), (1, &1234u32.to_le_bytes()), (4, &clock)]);
        let g = parse_save_gameplay_info(&data);
        assert!(!g.empty);
        assert_eq!(g.planet_id, Some(3));
        assert_eq!(g.bolts, Some(1234));
        assert_eq!(g.timestamp.as_deref(), Some("2026-10-10 23:14:14"));
        assert_eq!(get_planet_name("SCES_509.16", 3).as_deref(), Some("Kerwan"));
    }

    #[test]
    fn an_unused_slot_is_empty() {
        let data = save(&[(0, &u32::MAX.to_le_bytes()), (4, &[0u8; 8])]);
        let g = parse_save_gameplay_info(&data);
        assert!(g.empty);
        assert_eq!(g.timestamp, None);
    }

    #[test]
    fn decodes_shift_jis_title() {
        // "Ratchet & Clank" in PS2 SJIS:
        let bytes = [
            0x82, 0x71, 0x82, 0x81, 0x82, 0x94, 0x82, 0x83, 0x82, 0x88, 0x82, 0x85, 0x82, 0x94, 0x81, 0x40, 0x81, 0x95,
            0x81, 0x40, 0x82, 0x62, 0x82, 0x8c, 0x82, 0x81, 0x82, 0x8e, 0x82, 0x8b, 0x00,
        ];
        let decoded = decode_shift_jis_or_ascii(&bytes);
        assert_eq!(decoded, Some("Ratchet & Clank".to_string()));
    }
}
