// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

//! The save editor: reads one save slot of a Ratchet & Clank memory card, shows what a player
//! cares about (bolts, nanotech, weapons and gadgets, ammo, gold weapons, gold bolts, skill points,
//! planets), and writes it back so that the game still loads it.
//!
//! # The format
//!
//! Both rac1 discs (PAL `SCES_509.16`, US `SCUS_971.99`) write the same file: their descriptor
//! tables (PAL `D_001A05C0` / `D_001A08C0`, US `0x1A04C0` / `0x1A07C0`) list the same chunks with
//! the same ids and sizes, in the same order; only the addresses differ. So one decoder serves both,
//! picked by the card's serial ([`edits_serial`]).
//!
//! `saveN.bin` is the game's `memcard_MakeWholeSave` (US `src/storage/memory_card/
//! memcard_make_whole_save.c`; PAL `src/game/memcard.c`):
//!
//! ```text
//! +0  u32 global section size (0x1530)    +4  u32 level section size (0xaa4)
//! +8  the global section, then 20 level sections (one per planet slot)
//! section: u32 data size, u32 crc16(data), data = chunks, then {s32 -1, u32 0}
//! chunk:   s32 id, u32 size, size bytes, padding to 4
//! ```
//!
//! On load (`memcard_RestoreGame`, `memcard_RestoreData`) the game checks the two sizes, then for
//! each section the checksum (`memcard_TestChecksum`: stored value non-zero and equal to
//! `memcard_Checksum` of the data), and counts a chunk with an unknown id or a different size as an
//! error. [`crc16`] is `memcard_Checksum`; [`SaveFile::to_bytes`] writes a fresh checksum for every
//! section, and the editor keeps every chunk's id, size and padding, so an edited save passes all
//! of these checks.
//!
//! What each chunk holds comes from the decompiled code that uses it (the PAL decomp's `pause.c`,
//! `vendor.c`, `memcard.c` and the level code) and was cross-checked against ReRAC's
//! `docs/plan/game_state.md` and `crates/rc-game/src/game_state.rs` (ISC; layout facts only, no
//! code taken), which itself checked its chunk names against Wrench's `savegame.wtf`. Only fields
//! whose meaning is plain from that code are offered.

use crate::saves::{self, SaveBackupInfo, RAC1_LOCATIONS, RAC1_PLANETS};
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::{Path, PathBuf};

// ---------------------------------------------------------------------------------------------
// The checksum and the container.

/// `memcard_Checksum` returns 0, a checksum that always fails, above this length.
pub const CRC_MAX_LEN: usize = 0x1800;

/// `memcard_Checksum` (PAL `func_0020BB10`, US `calculate_crc16`): a CRC-16, most significant bit
/// first, polynomial 0x1F45, register seeded with 0xEDB88320 (only its low 16 bits reach the result),
/// no final xor.
pub fn crc16(data: &[u8]) -> u16 {
    if data.len() > CRC_MAX_LEN {
        return 0;
    }
    let mut crc: u32 = 0xEDB8_8320;
    for &b in data {
        crc ^= (b as u32) << 8;
        for _ in 0..8 {
            crc = if crc & 0x8000 != 0 { (crc << 1) ^ 0x1F45 } else { crc << 1 };
        }
    }
    (crc & 0xFFFF) as u16
}

fn align4(n: usize) -> usize {
    (n + 3) & !3
}

fn u32_at(b: &[u8], at: usize) -> Option<u32> {
    b.get(at..at + 4).map(|w| u32::from_le_bytes(w.try_into().unwrap()))
}

/// One `{id, size, bytes}` chunk. `pad` is the bytes between the data and the next multiple of 4:
/// the game never reads them, but they are inside the checksum, so they are kept as they were.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Chunk {
    pub id: i32,
    pub data: Vec<u8>,
    pub pad: Vec<u8>,
}

/// A section: its chunks in file order.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Section {
    pub chunks: Vec<Chunk>,
}

impl Section {
    /// Reads a section at the start of `bytes`; fails on a bad checksum, as the game's load does.
    pub fn decode(bytes: &[u8]) -> Result<Section, String> {
        let size = u32_at(bytes, 0).ok_or("a section is cut short")? as usize;
        let stored = u32_at(bytes, 4).ok_or("a section is cut short")?;
        let data = bytes.get(8..8 + size).ok_or("a section is longer than the file")?;
        if stored == 0 || crc16(data) as u32 != stored {
            return Err("a section's checksum does not match: the game would not load this save".into());
        }
        let mut chunks = Vec::new();
        let mut at = 0;
        loop {
            let id = u32_at(data, at).ok_or("a section has no end marker")? as i32;
            let len = u32_at(data, at + 4).ok_or("a section has no end marker")? as usize;
            if id == -1 {
                break;
            }
            let body = data.get(at + 8..at + 8 + align4(len)).ok_or("a chunk runs past its section")?;
            chunks.push(Chunk { id, data: body[..len].to_vec(), pad: body[len..].to_vec() });
            at += 8 + align4(len);
        }
        Ok(Section { chunks })
    }

    /// `memcard_PrepData`: `{size, crc16}`, the chunks, `{-1, 0}`. A checksum of 0 would make the
    /// game reject the section, so then one padding byte (never read by the game) is changed until
    /// the checksum is not 0.
    pub fn encode(&self) -> Vec<u8> {
        let mut out = self.encode_raw();
        if u32_at(&out, 4) == Some(0) {
            let mut fixed = self.clone();
            if let Some(at) = fixed.chunks.iter().position(|c| !c.pad.is_empty()) {
                for v in 1..=255u8 {
                    fixed.chunks[at].pad[0] = v;
                    out = fixed.encode_raw();
                    if u32_at(&out, 4) != Some(0) {
                        break;
                    }
                }
            }
        }
        out
    }

    fn encode_raw(&self) -> Vec<u8> {
        let mut v = vec![0u8; 8];
        for c in &self.chunks {
            v.extend_from_slice(&c.id.to_le_bytes());
            v.extend_from_slice(&(c.data.len() as u32).to_le_bytes());
            v.extend_from_slice(&c.data);
            let n = align4(c.data.len()) - c.data.len();
            v.extend((0..n).map(|k| c.pad.get(k).copied().unwrap_or(0)));
        }
        v.extend_from_slice(&(-1i32).to_le_bytes());
        v.extend_from_slice(&0u32.to_le_bytes());
        let size = (v.len() - 8) as u32;
        let crc = crc16(&v[8..]) as u32;
        v[0..4].copy_from_slice(&size.to_le_bytes());
        v[4..8].copy_from_slice(&crc.to_le_bytes());
        v
    }

    fn get(&self, id: i32) -> &[u8] {
        &self.chunks.iter().find(|c| c.id == id).expect("the layout check guarantees every chunk").data
    }

    fn get_mut(&mut self, id: i32) -> &mut Vec<u8> {
        &mut self.chunks.iter_mut().find(|c| c.id == id).expect("the layout check guarantees every chunk").data
    }

    fn layout(&self) -> Vec<(i32, usize)> {
        self.chunks.iter().map(|c| (c.id, c.data.len())).collect()
    }
}

/// The chunks of the global section, `(id, size)` in file order, as both discs' tables list them.
pub const GLOBAL_LAYOUT: &[(i32, usize)] = &[
    (0, 4),
    (1, 4),
    (2, 4),
    (3, 4),
    (4, 8),
    (5, 128),
    (7, 12),
    (8, 32),
    (9, 148),
    (10, 37),
    (11, 37),
    (12, 12),
    (13, 32),
    (19, 4),
    (14, 20),
    (20, 80),
    (15, 1936),
    (16, 1184),
    (17, 288),
    (18, 296),
    (21, 4),
    (22, 4),
    (23, 4),
    (24, 4),
    (25, 4),
    (26, 4),
    (27, 4),
    (28, 1),
    (29, 1),
    (30, 40),
    (31, 4),
    (32, 28),
    (33, 1),
    (34, 4),
    (35, 4),
    (36, 4),
    (37, 12),
    (1000, 148),
    (1001, 148),
    (1002, 148),
    (1003, 4),
    (1004, 4),
    (1005, 4),
    (1008, 4),
    (1009, 4),
    (1010, 150),
    (1011, 4),
];

/// The chunks of each of the 20 level sections.
pub const LEVEL_LAYOUT: &[(i32, usize)] = &[
    (3001, 1),
    (3002, 2048),
    (3003, 4),
    (3004, 16),
    (3005, 256),
    (3006, 256),
    (3007, 8),
    (3008, 16),
    (4000, 4),
    (4001, 4),
    (4002, 4),
];

/// Level sections in a save (`memcard_MakeWholeSave` loops 20 times; 19 planets are used).
pub const LEVEL_SLOTS: usize = 20;

fn section_size(layout: &[(i32, usize)]) -> usize {
    8 + layout.iter().map(|&(_, n)| 8 + align4(n)).sum::<usize>() + 8
}

/// A whole `saveN.bin`.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SaveFile {
    pub global: Section,
    pub levels: Vec<Section>,
}

impl SaveFile {
    /// Reads a save, refusing anything the game would not load or the editor does not know.
    pub fn parse(bytes: &[u8]) -> Result<SaveFile, String> {
        let gs = u32_at(bytes, 0).ok_or("the file is too short to be a save")? as usize;
        let ls = u32_at(bytes, 4).ok_or("the file is too short to be a save")? as usize;
        if gs != section_size(GLOBAL_LAYOUT) || ls != section_size(LEVEL_LAYOUT) {
            return Err("this save was not written by a version of the game the editor knows".into());
        }
        if bytes.len() != 8 + gs + LEVEL_SLOTS * ls {
            return Err(format!("the file is {} bytes; a save is {}", bytes.len(), 8 + gs + LEVEL_SLOTS * ls));
        }
        let global = Section::decode(&bytes[8..8 + gs])?;
        let mut levels = Vec::with_capacity(LEVEL_SLOTS);
        for l in 0..LEVEL_SLOTS {
            let at = 8 + gs + l * ls;
            levels.push(Section::decode(&bytes[at..at + ls])?);
        }
        let file = SaveFile { global, levels };
        let known = file.global.layout() == GLOBAL_LAYOUT && file.levels.iter().all(|l| l.layout() == LEVEL_LAYOUT);
        if !known {
            return Err("this save holds data the editor does not know; it is left as it is".into());
        }
        Ok(file)
    }

    /// `memcard_MakeWholeSave`: the two sizes, then every section with a fresh checksum.
    pub fn to_bytes(&self) -> Vec<u8> {
        let g = self.global.encode();
        let levels: Vec<Vec<u8>> = self.levels.iter().map(Section::encode).collect();
        let mut v = Vec::with_capacity(8 + g.len() + levels.iter().map(Vec::len).sum::<usize>());
        v.extend_from_slice(&(g.len() as u32).to_le_bytes());
        v.extend_from_slice(&(section_size(LEVEL_LAYOUT) as u32).to_le_bytes());
        v.extend_from_slice(&g);
        for l in &levels {
            v.extend_from_slice(l);
        }
        v
    }
}

// ---------------------------------------------------------------------------------------------
// What the chunks mean.

/// Global chunk ids (the PAL addresses they are saved from in brackets).
mod id {
    /// Planet the save resumes on, -1 in a slot never saved to (`D_0015EE84`).
    pub const PLANET: i32 = 0;
    /// Bolts (`gBolts`, `D_0015EE98`).
    pub const BOLTS: i32 = 1;
    /// The clock when saved, BCD (`D_0015EF98`).
    pub const CLOCK: i32 = 4;
    /// Story flags u8[128]: [4] Premium Nanotech, [5] Ultra Nanotech (`gSpecialItems`, `D_0013D490`).
    pub const FLAGS: i32 = 5;
    /// Skill points u8[32], 30 used (`gSkillPoints`, `D_0013D510`).
    pub const SKILL_POINTS: i32 = 8;
    /// Ammo s32[37] by item (`D_0013D530`).
    pub const AMMO: i32 = 9;
    /// Item owned u8[37] (`D_0013D5C8`: `gHaveHeliPack` is [2], `gHavePersuader` [35]).
    pub const OWNED: i32 = 10;
    /// Item ever acquired u8[37] (`D_0013D5F0`).
    pub const ACQUIRED: i32 = 11;
    /// Vendor stock u8[12]: low 6 bits the item, 0x40 owned (the vendor sells its ammo), 0xff empty.
    pub const VENDOR: i32 = 12;
    /// Quick select s32[8]: item ids, 0 empty (`D_00141FA0`).
    pub const QUICK_SELECT: i32 = 13;
    /// Planets unlocked u8[20]: the ship's destinations (`D_0013DE48`).
    pub const PLANETS: i32 = 14;
    /// Max health s32: 4, 5 with Premium Nanotech, 8 with Ultra (`gMaxNanotech`, `D_0015EEA0`).
    pub const MAX_HEALTH: i32 = 19;
    /// The galaxy map's planets in the order they were unlocked, s32[20] (`gGalaxyList`, `D_0013D618`).
    pub const MAP_ORDER: i32 = 20;
    /// Gold weapons u8[40] by item (`D_0013E620`: `gGoldSuckCannon` is [9]).
    pub const GOLD: i32 = 30;
    /// Equipped s32[7]: hand, feet, head, back (`D_00141760`).
    pub const EQUIPPED: i32 = 32;
    /// Per level: the planet's gold bolts collected, u8[4] (`D_0014BFC0`).
    pub const GOLD_BOLTS: i32 = 3003;
}

/// The bolt counter's limit (the game's own counters stop at 9,999,999).
pub const MAX_BOLTS: u32 = 9_999_999;
/// Skill points the game counts (`count_nonzero_entries_up_to_30`).
pub const SKILL_POINT_COUNT: usize = 30;

/// Gold bolts on each planet, by level (the boot executable's table at PAL `D_001E0B88`, the
/// same on both discs): 40 in all.
pub const GOLD_BOLTS_PER_PLANET: [usize; 19] = [0, 3, 4, 3, 1, 2, 2, 2, 2, 2, 2, 1, 2, 1, 4, 2, 2, 2, 3];

/// The planets a player can unlock with the ship: Novalis (1) to Gadgetron Site (16). Veldin,
/// Drek's Fleet and the last Veldin are reached by the story only.
pub const SHIP_PLANETS: std::ops::RangeInclusive<usize> = 1..=16;

/// What kind of thing an item is, for the page.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum ItemKind {
    Weapon,
    Gadget,
}

/// An item the editor offers. `max_ammo` is the item record's limit (`+0xE` of the 0x18-byte
/// records at PAL `D_001E02B0`, the vendor's "full" test; the same on both discs), 0 for an item
/// with no ammo. `gold` is set for the ten with a gold version (a gold price in that record).
#[derive(Clone, Copy, Debug)]
pub struct ItemDef {
    pub id: usize,
    pub name: &'static str,
    pub kind: ItemKind,
    pub max_ammo: u32,
    pub gold: bool,
}

const fn w(id: usize, name: &'static str, max_ammo: u32, gold: bool) -> ItemDef {
    ItemDef { id, name, kind: ItemKind::Weapon, max_ammo, gold }
}
const fn g(id: usize, name: &'static str) -> ItemDef {
    ItemDef { id, name, kind: ItemKind::Gadget, max_ammo: 0, gold: false }
}

/// The items, in id order (names as the game's menus give them).
pub const ITEMS: &[ItemDef] = &[
    g(2, "Heli-Pack"),
    g(3, "Thruster-Pack"),
    g(4, "Hydro-Pack"),
    g(5, "Sonic Summoner"),
    g(6, "O2 Mask"),
    g(7, "Pilot's Helmet"),
    w(9, "Suck Cannon", 0, true),
    w(10, "Bomb Glove", 40, true),
    w(11, "Devastator", 20, true),
    g(12, "Swingshot"),
    w(13, "Visibomb Gun", 20, false),
    g(14, "Taunter"),
    w(15, "Blaster", 200, true),
    w(16, "Pyrocitor", 240, true),
    w(17, "Mine Glove", 50, true),
    w(18, "Walloper", 0, false),
    w(19, "Tesla Claw", 240, true),
    w(20, "Glove of Doom", 10, true),
    w(21, "Morph-o-Ray", 0, true),
    g(22, "Hydrodisplacer"),
    w(23, "R.Y.N.O.", 50, false),
    w(24, "Drone Device", 10, false),
    w(25, "Decoy Glove", 20, true),
    g(26, "Trespasser"),
    g(27, "Metal Detector"),
    g(28, "Magneboots"),
    g(29, "Grindboots"),
    g(30, "Hoverboard"),
    g(31, "Hologuise"),
    g(32, "PDA"),
    g(33, "Map-o-Matic"),
    g(34, "Bolt Grabber"),
    g(35, "Persuader"),
];

/// Items the vendors sell (the boot executable's per-planet vendor table, PAL `0x1E0098`, and
/// the Bomb Glove the first vendor slot always holds): the vendor stock lists them.
const VENDOR_ITEMS: &[usize] = &[10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 24, 25];

const VENDOR_OWNED: u8 = 0x40;
const VENDOR_EMPTY: u8 = 0xff;

/// Health upgrades: Premium Nanotech (max 5) and Ultra Nanotech (max 8), as challenge mode
/// restores them (PAL `func_00227DB0`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub enum Nanotech {
    Standard,
    Premium,
    Ultra,
}

impl Nanotech {
    pub fn max_health(self) -> i32 {
        match self {
            Nanotech::Standard => 4,
            Nanotech::Premium => 5,
            Nanotech::Ultra => 8,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ItemState {
    pub id: usize,
    pub owned: bool,
    /// Ammo, for an item that has it.
    pub ammo: Option<u32>,
    /// The gold version, for an item that has one.
    pub gold: Option<bool>,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct PlanetState {
    pub id: usize,
    pub unlocked: bool,
    /// One per gold bolt on the planet: found or not.
    pub gold_bolts: Vec<bool>,
}

/// The fields the page edits, sent back unchanged or changed.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SaveFields {
    /// The planet the save loads on (a level number, 0..=18).
    pub planet: usize,
    pub bolts: u32,
    pub nanotech: Nanotech,
    pub items: Vec<ItemState>,
    /// Every planet that has gold bolts or can be unlocked, by id.
    pub planets: Vec<PlanetState>,
    /// The 30 skill points, in the order the game keeps them.
    pub skill_points: Vec<bool>,
}

/// Names and limits for the page: what it shows next to the fields.
#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ItemInfo {
    pub id: usize,
    pub name: String,
    pub kind: ItemKind,
    pub max_ammo: Option<u32>,
    pub has_gold: bool,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct PlanetInfo {
    pub id: usize,
    pub name: String,
    pub location: String,
    /// Whether the player may lock or unlock it (a ship destination).
    pub can_unlock: bool,
    /// Whether the save can be moved there in the default mode (a ship destination).
    pub can_land: bool,
}

/// A slot as the page shows it.
#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SaveView {
    pub serial: String,
    pub slot: usize,
    /// A digest of the file as read: a write is refused if the file changed since.
    pub fingerprint: String,
    /// Where the save resumes, "Kerwan" (read only).
    pub planet_name: Option<String>,
    /// When it was saved (read only).
    pub saved_at: Option<String>,
    pub fields: SaveFields,
    pub items: Vec<ItemInfo>,
    pub planets: Vec<PlanetInfo>,
    /// Every level, 0..=18, for the planet choice and the advanced editor.
    pub levels: Vec<PlanetInfo>,
    pub max_bolts: u32,
    /// Every chunk of the file, for the advanced editor.
    pub chunks: Vec<ChunkView>,
}

/// One chunk as the advanced editor shows it: where it is, its id and size, what it is (in a
/// player's words where it is known), and its bytes as hex.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ChunkView {
    /// `None` for the global section, else the planet's level number.
    pub section: Option<usize>,
    pub id: i32,
    pub size: usize,
    pub name: String,
    /// Whether its meaning is decoded; unknown chunks are raw bytes only.
    pub known: bool,
    pub hex: String,
}

/// A raw replacement of one chunk's bytes (the advanced editor). The size cannot change.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct RawPatch {
    pub section: Option<usize>,
    pub id: i32,
    pub hex: String,
}

/// What a write changes: the checked fields of the default mode, or raw chunks (advanced mode).
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(tag = "kind", rename_all = "camelCase")]
pub enum SaveChange {
    Fields { fields: SaveFields },
    Raw { chunks: Vec<RawPatch> },
}

/// What each chunk is, as far as the code shows it (see the module notes); `false` for a chunk
/// whose meaning is not decoded.
pub fn chunk_name(level: bool, chunk: i32) -> (&'static str, bool) {
    if level {
        return match chunk {
            3001 => ("Visited (0 never, 1 visited, 2 left)", true),
            3002 => ("Map: the parts uncovered (packed)", false),
            3003 => ("Gold bolts found", true),
            3004 => ("Missions (0xff done)", true),
            3005 => ("Enemies and crates destroyed (bits)", false),
            3006 => ("Bolts from crates and enemies (pairs)", false),
            3007 => ("Arrivals: count, time, levels", false),
            3008 => ("Metal Detector spots", false),
            4000 => ("Bolts collected here", true),
            4001 => ("Hits taken here", true),
            4002 => ("Deaths here", true),
            _ => ("Unknown", false),
        };
    }
    match chunk {
        0 => ("Planet", true),
        1 => ("Bolts", true),
        2 => ("Times completed (challenge mode)", true),
        3 => ("Time played (1/60 s)", true),
        4 => ("When saved (clock)", true),
        5 => ("Story flags", false),
        7 => ("Cheats on", false),
        8 => ("Skill points", true),
        9 => ("Ammo by item", true),
        10 => ("Items owned", true),
        11 => ("Items ever had", true),
        12 => ("Vendor stock", true),
        13 => ("Quick select", true),
        14 => ("Planets unlocked", true),
        15 => ("Map landmarks", false),
        16 => ("Help messages seen", false),
        17 => ("Move help seen", false),
        18 => ("Gadget help seen", false),
        19 => ("Max health", true),
        20 => ("Galaxy map order", true),
        21 => ("Last item in hand", true),
        22 => ("Wrench in hand", true),
        23 => ("Thruster-Pack on the back", true),
        24 => ("Unused", false),
        25 => ("Camera left/right (1 normal)", true),
        26 => ("Camera up/down (1 normal)", true),
        27 => ("Camera speed (0 to 2)", true),
        28 => ("HelpDesk voice", true),
        29 => ("HelpDesk text", true),
        30 => ("Gold weapons", true),
        31 => ("Game beaten", true),
        32 => ("Equipped: hand, feet, head, back", true),
        33 => ("Subtitles", true),
        34 => ("Stereo (1) or mono", true),
        35 => ("Music volume (0 to 1024)", true),
        36 => ("Effects volume (0 to 1024)", true),
        37 => ("Cheats ever used", false),
        1000 => ("Ammo bought by item", true),
        1001 => ("Ammo picked up by item", true),
        1002 => ("Ammo used by item", true),
        1003 => ("Play time (ticks)", true),
        1004 => ("Hits taken", true),
        1005 => ("Deaths", true),
        1008 => ("Bolt rate: ticks", false),
        1009 => ("Bolt rate: bolts", false),
        1010 => ("Help log", false),
        1011 => ("Help log position", false),
        _ => ("Unknown", false),
    }
}

fn to_hex(b: &[u8]) -> String {
    b.iter().map(|x| format!("{x:02x}")).collect::<Vec<_>>().join(" ")
}

fn from_hex(text: &str) -> Result<Vec<u8>, String> {
    let digits: Vec<char> = text.chars().filter(|c| !c.is_whitespace()).collect();
    if !digits.len().is_multiple_of(2) {
        return Err("the hex has an odd number of digits".into());
    }
    digits
        .chunks(2)
        .map(|p| {
            u8::from_str_radix(&p.iter().collect::<String>(), 16)
                .map_err(|_| "the hex has a character that is not hex".to_string())
        })
        .collect()
}

/// The planets a save can be moved to in the default mode: the ship's destinations.
/// Veldin (0 and 18) and Drek's Fleet (17) are reached through the story: their arrival
/// scenes and the state they need are not in a save made elsewhere.
pub fn can_land(p: usize) -> bool {
    SHIP_PLANETS.contains(&p)
}

/// Whether the editor knows the saves of the game with this disc serial: both Ratchet & Clank
/// discs, which write the same format.
pub fn edits_serial(serial: &str) -> bool {
    matches!(serial, "SCES_509.16" | "SCUS_971.99")
}

fn i32_at(b: &[u8], i: usize) -> i32 {
    i32::from_le_bytes(b[i * 4..i * 4 + 4].try_into().unwrap())
}

fn set_i32(b: &mut [u8], i: usize, v: i32) {
    b[i * 4..i * 4 + 4].copy_from_slice(&v.to_le_bytes());
}

/// The planets the page lists: those with gold bolts or that the ship can reach.
fn listed_planets() -> impl Iterator<Item = usize> {
    (0..GOLD_BOLTS_PER_PLANET.len()).filter(|&p| GOLD_BOLTS_PER_PLANET[p] > 0 || SHIP_PLANETS.contains(&p))
}

impl SaveFile {
    /// Whether the slot holds a game (the planet is -1 in a slot never saved to).
    pub fn is_empty(&self) -> bool {
        i32_at(self.global.get(id::PLANET), 0) < 0
    }

    pub fn fields(&self) -> SaveFields {
        let g = &self.global;
        let flags = g.get(id::FLAGS);
        let nanotech = if flags[5] != 0 {
            Nanotech::Ultra
        } else if flags[4] != 0 {
            Nanotech::Premium
        } else {
            Nanotech::Standard
        };
        let (owned, ammo, gold) = (g.get(id::OWNED), g.get(id::AMMO), g.get(id::GOLD));
        let items = ITEMS
            .iter()
            .map(|d| ItemState {
                id: d.id,
                owned: owned[d.id] != 0,
                ammo: (d.max_ammo > 0).then(|| i32_at(ammo, d.id).max(0) as u32),
                gold: d.gold.then(|| gold[d.id] != 0),
            })
            .collect();
        let unlocked = g.get(id::PLANETS);
        let planets = listed_planets()
            .map(|p| PlanetState {
                id: p,
                unlocked: unlocked[p] != 0,
                gold_bolts: (0..GOLD_BOLTS_PER_PLANET[p]).map(|k| self.levels[p].get(id::GOLD_BOLTS)[k] != 0).collect(),
            })
            .collect();
        SaveFields {
            planet: i32_at(g.get(id::PLANET), 0).clamp(0, 18) as usize,
            bolts: i32_at(g.get(id::BOLTS), 0).max(0) as u32,
            nanotech,
            items,
            planets,
            skill_points: g.get(id::SKILL_POINTS)[..SKILL_POINT_COUNT].iter().map(|&b| b != 0).collect(),
        }
    }

    /// Applies `want` after checking it, changing only what differs from the save, the way the game
    /// itself keeps related chunks together (owning an item also marks it acquired and puts it in the
    /// vendor's stock; giving one up takes it out of the quick select and the equipped slots).
    pub fn apply(&mut self, want: &SaveFields) -> Result<(), String> {
        validate(want)?;
        let have = self.fields();
        for (w, h) in want.planets.iter().zip(&have.planets) {
            if w.unlocked != h.unlocked && !SHIP_PLANETS.contains(&w.id) {
                return Err(format!("{} is reached by the story, not unlocked by hand", RAC1_PLANETS[w.id]));
            }
        }
        if want.planet != have.planet {
            if !can_land(want.planet) {
                return Err(format!(
                    "{} is reached by the story; a save cannot be moved there",
                    RAC1_PLANETS[want.planet]
                ));
            }
            if want.planets.iter().any(|s| s.id == want.planet && !s.unlocked) {
                return Err(format!("{} has to be unlocked to load there", RAC1_PLANETS[want.planet]));
            }
        }

        if want.bolts != have.bolts {
            set_i32(self.global.get_mut(id::BOLTS), 0, want.bolts as i32);
        }

        if want.nanotech != have.nanotech {
            let flags = self.global.get_mut(id::FLAGS);
            flags[4] = (want.nanotech != Nanotech::Standard) as u8;
            flags[5] = (want.nanotech == Nanotech::Ultra) as u8;
            set_i32(self.global.get_mut(id::MAX_HEALTH), 0, want.nanotech.max_health());
        }

        for (w, h) in want.items.iter().zip(&have.items) {
            if w.owned != h.owned {
                if w.owned {
                    self.give(w.id);
                } else {
                    self.take(w.id);
                }
            }
            if let (Some(a), Some(old)) = (w.ammo, h.ammo) {
                if a != old {
                    set_i32(self.global.get_mut(id::AMMO), w.id, a as i32);
                }
            }
            if let (Some(gw), Some(gh)) = (w.gold, h.gold) {
                if gw != gh {
                    self.global.get_mut(id::GOLD)[w.id] = gw as u8;
                }
            }
        }

        for (w, h) in want.planets.iter().zip(&have.planets) {
            if w.unlocked != h.unlocked {
                if w.unlocked {
                    self.unlock_planet(w.id);
                } else {
                    self.lock_planet(w.id);
                }
            }
            if w.gold_bolts != h.gold_bolts {
                let bolts = self.levels[w.id].get_mut(id::GOLD_BOLTS);
                for (k, &found) in w.gold_bolts.iter().enumerate() {
                    bolts[k] = found as u8;
                }
            }
        }

        // The level start unlocks the planet it loads and adds it to the galaxy map; doing it here
        // keeps the save as the game would have it.
        if want.planet != have.planet {
            set_i32(self.global.get_mut(id::PLANET), 0, want.planet as i32);
            if self.global.get(id::PLANETS)[want.planet] == 0 {
                self.unlock_planet(want.planet);
            }
        }

        if want.skill_points != have.skill_points {
            let sp = self.global.get_mut(id::SKILL_POINTS);
            for (k, &on) in want.skill_points.iter().enumerate() {
                sp[k] = on as u8;
            }
        }
        Ok(())
    }

    /// Every chunk, for the advanced editor.
    pub fn chunk_views(&self) -> Vec<ChunkView> {
        let one = |section: Option<usize>, c: &Chunk| {
            let (name, known) = chunk_name(section.is_some(), c.id);
            ChunkView { section, id: c.id, size: c.data.len(), name: name.to_string(), known, hex: to_hex(&c.data) }
        };
        let mut out: Vec<ChunkView> = self.global.chunks.iter().map(|c| one(None, c)).collect();
        for (l, sec) in self.levels.iter().enumerate() {
            out.extend(sec.chunks.iter().map(|c| one(Some(l), c)));
        }
        out
    }

    /// Replaces chunks' bytes as given (the advanced editor), the sizes unchanged.
    pub fn apply_raw(&mut self, patches: &[RawPatch]) -> Result<(), String> {
        for p in patches {
            let section = match p.section {
                None => &mut self.global,
                Some(l) => self.levels.get_mut(l).ok_or(format!("there is no planet section {l}"))?,
            };
            let chunk =
                section.chunks.iter_mut().find(|c| c.id == p.id).ok_or(format!("there is no chunk {} there", p.id))?;
            let bytes = from_hex(&p.hex).map_err(|e| format!("chunk {}: {e}", p.id))?;
            if bytes.len() != chunk.data.len() {
                return Err(format!("chunk {} is {} bytes, not {}", p.id, chunk.data.len(), bytes.len()));
            }
            chunk.data = bytes;
        }
        Ok(())
    }

    /// The parts of `GiveItem` that are saved: owned, acquired, and the vendor's stock.
    fn give(&mut self, item: usize) {
        self.global.get_mut(id::OWNED)[item] = 1;
        self.global.get_mut(id::ACQUIRED)[item] = 1;
        if VENDOR_ITEMS.contains(&item) {
            let stock = self.global.get_mut(id::VENDOR);
            if let Some(s) = stock.iter().position(|&b| b != VENDOR_EMPTY && (b & 0x3f) as usize == item) {
                stock[s] |= VENDOR_OWNED;
            } else if let Some(s) = stock.iter().position(|&b| b == VENDOR_EMPTY) {
                stock[s] = item as u8 | VENDOR_OWNED;
            }
        }
    }

    /// Gives an item up: not owned, not sold as owned, not in the quick select or a hand, foot,
    /// head or back slot (an empty hand becomes the Bomb Glove at the next level start).
    fn take(&mut self, item: usize) {
        self.global.get_mut(id::OWNED)[item] = 0;
        let stock = self.global.get_mut(id::VENDOR);
        for b in stock.iter_mut().filter(|b| **b != VENDOR_EMPTY && (**b & 0x3f) as usize == item) {
            *b &= !VENDOR_OWNED;
        }
        for chunk in [id::QUICK_SELECT, id::EQUIPPED] {
            let c = self.global.get_mut(chunk);
            for i in 0..c.len() / 4 {
                if i32_at(c, i) == item as i32 {
                    set_i32(c, i, 0);
                }
            }
        }
    }

    /// The planet unlock (ReRAC's `unlock_planet`, the game's L01 0x2756d0): unlocked, and added to
    /// the galaxy map after the planets unlocked before it.
    fn unlock_planet(&mut self, p: usize) {
        let n = self.global.get(id::PLANETS).iter().filter(|&&u| u != 0).count();
        self.global.get_mut(id::PLANETS)[p] = 1;
        let order = self.global.get_mut(id::MAP_ORDER);
        if n < order.len() / 4 {
            set_i32(order, n, p as i32);
        }
    }

    /// The reverse: locked, and taken off the galaxy map, the later planets moving up.
    fn lock_planet(&mut self, p: usize) {
        let n = self.global.get(id::PLANETS).iter().filter(|&&u| u != 0).count();
        self.global.get_mut(id::PLANETS)[p] = 0;
        let order = self.global.get_mut(id::MAP_ORDER);
        let len = order.len() / 4;
        let mut list: Vec<i32> = (0..n.min(len)).map(|i| i32_at(order, i)).collect();
        if let Some(at) = list.iter().position(|&q| q == p as i32) {
            list.remove(at);
        }
        for i in 0..n.min(len) {
            set_i32(order, i, list.get(i).copied().unwrap_or(0));
        }
    }
}

/// Checks a request against the save's limits before anything is changed.
pub fn validate(f: &SaveFields) -> Result<(), String> {
    if f.planet >= GOLD_BOLTS_PER_PLANET.len() {
        return Err("there is no such planet".into());
    }
    if f.bolts > MAX_BOLTS {
        return Err(format!("bolts go up to {MAX_BOLTS}"));
    }
    if f.items.len() != ITEMS.len() || f.items.iter().zip(ITEMS).any(|(s, d)| s.id != d.id) {
        return Err("the item list does not match the game's".into());
    }
    for (s, d) in f.items.iter().zip(ITEMS) {
        match (s.ammo, d.max_ammo) {
            (Some(a), max) if a > max => return Err(format!("{} holds at most {max} ammo", d.name)),
            (Some(_), 0) => return Err(format!("{} has no ammo", d.name)),
            (None, max) if max > 0 => return Err(format!("{}'s ammo is missing", d.name)),
            _ => {}
        }
        if s.gold.is_some() != d.gold {
            return Err(format!("{} has no gold version", d.name));
        }
    }
    let planets: Vec<usize> = listed_planets().collect();
    if f.planets.len() != planets.len() || f.planets.iter().zip(&planets).any(|(s, &p)| s.id != p) {
        return Err("the planet list does not match the game's".into());
    }
    for s in &f.planets {
        if s.gold_bolts.len() != GOLD_BOLTS_PER_PLANET[s.id] {
            return Err(format!("{} has {} gold bolts", RAC1_PLANETS[s.id], GOLD_BOLTS_PER_PLANET[s.id]));
        }
    }
    if f.skill_points.len() != SKILL_POINT_COUNT {
        return Err(format!("there are {SKILL_POINT_COUNT} skill points"));
    }
    Ok(())
}

fn clock_text(clock: &[u8]) -> Option<String> {
    let bcd = |b: u8| (b >> 4 <= 9 && b & 15 <= 9).then_some((b >> 4) as u32 * 10 + (b & 15) as u32);
    if clock.iter().all(|&b| b == 0) {
        return None;
    }
    let (s, m, h, d, mo, y) =
        (bcd(clock[1])?, bcd(clock[2])?, bcd(clock[3])?, bcd(clock[5])?, bcd(clock[6])?, bcd(clock[7])?);
    Some(format!("{:04}-{mo:02}-{d:02} {h:02}:{m:02}:{s:02}", 2000 + y))
}

/// FNV-1a 64 of the bytes, as hex: enough to notice that the file changed under the page.
pub fn fingerprint(bytes: &[u8]) -> String {
    let mut h: u64 = 0xcbf2_9ce4_8422_2325;
    for &b in bytes {
        h ^= b as u64;
        h = h.wrapping_mul(0x0000_0100_0000_01b3);
    }
    format!("{h:016x}")
}

/// The page's view of a decoded save.
pub fn view(serial: &str, slot: usize, bytes: &[u8], file: &SaveFile) -> SaveView {
    let planet = i32_at(file.global.get(id::PLANET), 0);
    SaveView {
        serial: serial.to_string(),
        slot,
        fingerprint: fingerprint(bytes),
        planet_name: usize::try_from(planet).ok().and_then(|p| RAC1_PLANETS.get(p)).map(|s| s.to_string()),
        saved_at: clock_text(file.global.get(id::CLOCK)),
        fields: file.fields(),
        items: ITEMS
            .iter()
            .map(|d| ItemInfo {
                id: d.id,
                name: d.name.to_string(),
                kind: d.kind,
                max_ammo: (d.max_ammo > 0).then_some(d.max_ammo),
                has_gold: d.gold,
            })
            .collect(),
        planets: listed_planets()
            .map(|p| PlanetInfo {
                id: p,
                name: RAC1_PLANETS[p].to_string(),
                location: RAC1_LOCATIONS[p].to_string(),
                can_unlock: SHIP_PLANETS.contains(&p),
                can_land: can_land(p),
            })
            .collect(),
        levels: (0..GOLD_BOLTS_PER_PLANET.len())
            .map(|p| PlanetInfo {
                id: p,
                name: RAC1_PLANETS[p].to_string(),
                location: RAC1_LOCATIONS[p].to_string(),
                can_unlock: SHIP_PLANETS.contains(&p),
                can_land: can_land(p),
            })
            .collect(),
        max_bolts: MAX_BOLTS,
        chunks: file.chunk_views(),
    }
}

// ---------------------------------------------------------------------------------------------
// The card on disk.

/// Save slots on a card (`save0.bin` .. `save4.bin`).
pub const SLOTS: usize = 5;

fn slot_path(card_dir: &Path, slot: usize) -> Result<PathBuf, String> {
    if slot >= SLOTS {
        return Err(format!("there is no slot {}", slot + 1));
    }
    let folder = saves::find_game_folder(card_dir).ok_or("there are no saves on this card yet")?;
    Ok(folder.join(format!("save{slot}.bin")))
}

fn read_slot(card_dir: &Path, serial: &str, slot: usize) -> Result<(PathBuf, Vec<u8>, SaveFile), String> {
    if !edits_serial(serial) {
        return Err("the save editor does not know this game's saves yet".into());
    }
    let path = slot_path(card_dir, slot)?;
    let bytes = fs::read(&path).map_err(|e| format!("slot {} cannot be read: {e}", slot + 1))?;
    let file = SaveFile::parse(&bytes)?;
    if file.is_empty() {
        return Err(format!("slot {} is empty", slot + 1));
    }
    Ok((path, bytes, file))
}

/// Reads slot `slot` of the card in `card_dir` for the editor.
pub fn read_save(card_dir: &Path, serial: &str, slot: usize) -> Result<SaveView, String> {
    let (_, bytes, file) = read_slot(card_dir, serial, slot)?;
    Ok(view(serial, slot, &bytes, &file))
}

/// Whether two saves hold the same bytes in every chunk (padding aside).
fn same_data(a: &SaveFile, b: &SaveFile) -> bool {
    let sec = |x: &Section, y: &Section| {
        x.layout() == y.layout() && x.chunks.iter().zip(&y.chunks).all(|(c, d)| c.data == d.data)
    };
    sec(&a.global, &b.global) && a.levels.iter().zip(&b.levels).all(|(x, y)| sec(x, y))
}

/// What a write did: the backup taken before it, and the save as it now is.
#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct WriteResult {
    pub backup: SaveBackupInfo,
    pub view: SaveView,
}

/// Writes `change` into slot `slot`, safely:
/// 1. `in_use` must say no game is using the card (it names the game when one is);
/// 2. the file must still be the one the page read (`fingerprint`);
/// 3. the request is checked against the game's limits;
/// 4. the whole card is backed up beside it (`.backup_<time>_before_edit_slotN`, restorable from
///    the Backups list);
/// 5. the new file goes to a temporary file in the same folder, is read back and checked, and then
///    replaces the save in one rename.
pub fn write_save(
    card_dir: &Path,
    serial: &str,
    slot: usize,
    fingerprint_seen: &str,
    change: &SaveChange,
    in_use: impl Fn() -> Result<Option<String>, String>,
) -> Result<WriteResult, String> {
    if let Some(game) = in_use()? {
        return Err(format!("{game} is running with this memory card: close it first, so the save is not overwritten"));
    }
    let (path, bytes, mut file) = read_slot(card_dir, serial, slot)?;
    if fingerprint(&bytes) != fingerprint_seen {
        return Err("the save changed since it was opened (the game saved?): open it again".into());
    }
    match change {
        SaveChange::Fields { fields } => file.apply(fields)?,
        SaveChange::Raw { chunks } => file.apply_raw(chunks)?,
    }
    let out = file.to_bytes();
    let check = SaveFile::parse(&out)?;
    let as_asked = match change {
        SaveChange::Fields { fields } => check.fields() == *fields,
        SaveChange::Raw { .. } => same_data(&check, &file),
    };
    if !as_asked {
        return Err("the edited save did not read back as asked; nothing was written".into());
    }
    let backup = saves::backup_card(card_dir, Some(&format!("before_edit_slot{}", slot + 1)))?;
    let tmp = path.with_file_name(format!(".save{slot}.bin.editing"));
    fs::write(&tmp, &out).map_err(|e| format!("could not write the save: {e}"))?;
    if fs::read(&tmp).ok().as_deref() != Some(&out[..]) {
        let _ = fs::remove_file(&tmp);
        return Err("the save did not write back as it should; it was not replaced".into());
    }
    fs::rename(&tmp, &path).map_err(|e| format!("could not replace the save: {e}"))?;
    Ok(WriteResult { backup, view: view(serial, slot, &out, &check) })
}

// ---------------------------------------------------------------------------------------------
// Is the game running with this card?

/// The program names a rac1 version runs under: the build's file name (`openrac-rac1-pal`, also in
/// the macOS bundle's folder) and the bundle's own name.
fn program_marks(serial: &str) -> &'static [&'static str] {
    match serial {
        "SCES_509.16" => &["openrac-rac1-pal", "Ratchet & Clank (PAL)"],
        "SCUS_971.99" => &["openrac-rac1-ntsc", "Ratchet & Clank (NTSC)"],
        _ => &[],
    }
}

/// The program's own name in a command line: the text before the first option, quotes and folders
/// taken off (`.../Ratchet & Clank (PAL).app/Contents/MacOS/Ratchet & Clank (PAL)` gives the last
/// part). A shell that started the game holds the game's name only after its own options, so it
/// does not count.
fn program_name(cmdline: &str) -> &str {
    let program = cmdline.split(" --").next().unwrap_or("").trim().trim_matches('"');
    if [" -", "&&", ";", "|"].iter().any(|s| program.contains(s)) {
        return "";
    }
    let name = program.rsplit(['/', '\\']).next().unwrap_or(program);
    name.strip_suffix(".exe").unwrap_or(name)
}

fn is_game(cmdline: &str) -> bool {
    let name = program_name(cmdline);
    name.starts_with("openrac-rac") || name.starts_with("Ratchet & Clank")
}

/// Whether a process with this command line uses the card in `card_dir` (whose last folder is the
/// serial). The port takes `--cards DIR`, `--no-card`, or else the default card for its serial.
/// Command lines from the system's process list cannot be split safely (paths have spaces), so
/// the test leans towards "in use": a relative `--cards` naming the serial counts.
pub fn uses_card(cmdline: &str, card_dir: &Path, serial: &str) -> bool {
    if !is_game(cmdline) || cmdline.contains("--no-card") {
        return false;
    }
    if let Some(at) = cmdline.find("--cards") {
        let value = cmdline[at + "--cards".len()..].trim_start_matches(['=', ' ', '"']);
        let dir = card_dir.to_string_lossy();
        let dir = dir.trim_end_matches(['/', '\\']);
        if let Some(rest) = value.strip_prefix(dir) {
            return rest.is_empty() || rest.starts_with([' ', '/', '\\', '"']);
        }
        let absolute = value.starts_with('/') || value.get(1..3) == Some(":\\");
        return !absolute && value.split([' ', '/', '\\', '"']).any(|part| part == serial);
    }
    program_marks(serial).contains(&program_name(cmdline))
}

/// The command lines of the running processes.
fn process_lines() -> Result<Vec<String>, String> {
    #[cfg(windows)]
    let output = {
        use std::os::windows::process::CommandExt;
        std::process::Command::new("powershell")
            .args(["-NoProfile", "-Command", "Get-CimInstance Win32_Process | ForEach-Object { $_.CommandLine }"])
            .creation_flags(0x0800_0000)
            .output()
    };
    #[cfg(not(windows))]
    let output = std::process::Command::new("ps").args(["-axww", "-o", "args="]).output();
    let output = output.map_err(|e| format!("could not check whether the game is running: {e}"))?;
    if !output.status.success() {
        return Err("could not check whether the game is running".into());
    }
    Ok(String::from_utf8_lossy(&output.stdout).lines().map(str::to_string).collect())
}

/// Names the game when one is running with the card in `card_dir`.
pub fn card_in_use(card_dir: &Path, serial: &str) -> Result<Option<String>, String> {
    let lines = process_lines()?;
    Ok(lines.iter().any(|l| uses_card(l, card_dir, serial)).then(|| "The game".to_string()))
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A save as a fresh game writes it, built in code: zeros, except what the new game sets.
    fn blank() -> SaveFile {
        let section = |layout: &[(i32, usize)]| Section {
            chunks: layout.iter().map(|&(id, n)| Chunk { id, data: vec![0; n], pad: vec![0; align4(n) - n] }).collect(),
        };
        let mut f = SaveFile {
            global: section(GLOBAL_LAYOUT),
            levels: (0..LEVEL_SLOTS).map(|_| section(LEVEL_LAYOUT)).collect(),
        };
        set_i32(f.global.get_mut(id::MAX_HEALTH), 0, 4);
        f.global.get_mut(id::VENDOR).fill(VENDOR_EMPTY);
        f.global.get_mut(id::CLOCK).copy_from_slice(&[0x00, 0x14, 0x14, 0x23, 0x00, 0x10, 0x10, 0x26]);
        set_i32(f.global.get_mut(id::PLANET), 0, 3);
        f
    }

    #[test]
    fn the_checksum_is_the_games() {
        // Computed independently (a straight transcription of memcard_Checksum in Python).
        assert_eq!(crc16(b"123456789"), 0xd989);
        assert_eq!(crc16(b""), 0x8320);
        assert_eq!(crc16(&vec![0; CRC_MAX_LEN + 1]), 0);
    }

    #[test]
    fn a_save_round_trips_byte_for_byte() {
        let mut f = blank();
        // Padding the game left behind is kept.
        f.global.chunks.iter_mut().find(|c| c.id == id::OWNED).unwrap().pad = vec![0xaa, 0xbb, 0xcc];
        let bytes = f.to_bytes();
        assert_eq!(bytes.len(), 0xea08);
        assert_eq!(u32_at(&bytes, 0), Some(0x1530));
        assert_eq!(u32_at(&bytes, 4), Some(0xaa4));
        let back = SaveFile::parse(&bytes).unwrap();
        assert_eq!(back, f);
        assert_eq!(back.to_bytes(), bytes);
    }

    #[test]
    fn a_damaged_or_unknown_save_is_refused() {
        let mut bytes = blank().to_bytes();
        bytes[20] ^= 1;
        assert!(SaveFile::parse(&bytes).unwrap_err().contains("checksum"));
        let mut other = blank();
        other.global.chunks.pop();
        assert!(SaveFile::parse(&other.to_bytes()).is_err());
        assert!(SaveFile::parse(&[0; 8]).is_err());
    }

    #[test]
    fn a_zero_checksum_is_avoided_through_a_padding_byte() {
        // Search a small section whose checksum comes out 0, then encode it.
        let mut found = None;
        for v in 0..=u32::MAX {
            let s = Section {
                chunks: vec![
                    Chunk { id: 1, data: v.to_le_bytes().to_vec(), pad: vec![] },
                    Chunk { id: 2, data: vec![7], pad: vec![0; 3] },
                ],
            };
            if u32_at(&s.encode_raw(), 4) == Some(0) {
                found = Some(s);
                break;
            }
        }
        let s = found.unwrap();
        let out = s.encode();
        assert_ne!(u32_at(&out, 4), Some(0));
        let back = Section::decode(&out).unwrap();
        assert_eq!(back.chunks[0].data, s.chunks[0].data);
        assert_eq!(back.chunks[1].data, vec![7]);
    }

    #[test]
    fn reads_the_fields() {
        let mut f = blank();
        set_i32(f.global.get_mut(id::BOLTS), 0, 1234);
        f.global.get_mut(id::OWNED)[10] = 1;
        set_i32(f.global.get_mut(id::AMMO), 10, 33);
        f.global.get_mut(id::GOLD)[10] = 1;
        f.global.get_mut(id::FLAGS)[4] = 1;
        f.global.get_mut(id::PLANETS)[1] = 1;
        f.levels[2].get_mut(id::GOLD_BOLTS)[1] = 1;
        f.global.get_mut(id::SKILL_POINTS)[29] = 1;
        let v = view("SCES_509.16", 0, &f.to_bytes(), &f);
        assert_eq!(v.planet_name.as_deref(), Some("Kerwan"));
        assert_eq!(v.saved_at.as_deref(), Some("2026-10-10 23:14:14"));
        let s = v.fields;
        assert_eq!(s.bolts, 1234);
        assert_eq!(s.nanotech, Nanotech::Premium);
        let bomb = s.items.iter().find(|i| i.id == 10).unwrap();
        assert_eq!((bomb.owned, bomb.ammo, bomb.gold), (true, Some(33), Some(true)));
        assert_eq!(s.items.iter().find(|i| i.id == 2).unwrap().ammo, None);
        assert!(s.planets.iter().find(|p| p.id == 1).unwrap().unlocked);
        assert_eq!(s.planets.iter().find(|p| p.id == 2).unwrap().gold_bolts, vec![false, true, false, false]);
        assert!(s.skill_points[29] && !s.skill_points[0]);
        assert_eq!(GOLD_BOLTS_PER_PLANET.iter().sum::<usize>(), 40);
    }

    #[test]
    fn editing_bolts_changes_only_the_global_section() {
        let f = blank();
        let before = f.to_bytes();
        let mut e = f.clone();
        let mut want = e.fields();
        want.bolts = 150_000;
        e.apply(&want).unwrap();
        let after = e.to_bytes();
        let back = SaveFile::parse(&after).unwrap();
        assert_eq!(back.fields().bolts, 150_000);
        let gs = 8 + 0x1530;
        assert_eq!(before[gs..], after[gs..], "the level sections are untouched");
        let differ: Vec<usize> = (0..gs).filter(|&i| before[i] != after[i]).collect();
        // The bolts word (+0x24) and the checksum (+0xC..+0x10) only.
        assert!(differ.iter().all(|&i| (0x24..0x28).contains(&i) || (0xC..0x10).contains(&i)), "{differ:?}");
    }

    #[test]
    fn the_limits_are_checked() {
        let f = blank();
        let ok = f.fields();
        let mut w = ok.clone();
        w.bolts = MAX_BOLTS + 1;
        assert!(validate(&w).is_err());
        let mut w = ok.clone();
        w.items.iter_mut().find(|i| i.id == 10).unwrap().ammo = Some(41);
        assert!(validate(&w).unwrap_err().contains("40"));
        let mut w = ok.clone();
        w.items.iter_mut().find(|i| i.id == 13).unwrap().gold = Some(true);
        assert!(validate(&w).is_err(), "the Visibomb Gun has no gold version");
        let mut w = ok.clone();
        w.items.iter_mut().find(|i| i.id == 2).unwrap().ammo = Some(1);
        assert!(validate(&w).is_err());
        let mut w = ok.clone();
        w.planets[0].gold_bolts.push(true);
        assert!(validate(&w).is_err());
        let mut w = ok.clone();
        w.skill_points.pop();
        assert!(validate(&w).is_err());
        let mut w = ok;
        w.items.swap(0, 1);
        assert!(validate(&w).is_err());
    }

    #[test]
    fn owning_and_giving_up_keep_the_related_chunks() {
        let mut f = blank();
        let mut w = f.fields();
        w.items.iter_mut().find(|i| i.id == 16).unwrap().owned = true; // Pyrocitor, sold by a vendor
        w.items.iter_mut().find(|i| i.id == 28).unwrap().owned = true; // Magneboots, not sold
        f.apply(&w).unwrap();
        assert_eq!(f.global.get(id::ACQUIRED)[16], 1);
        assert_eq!(f.global.get(id::VENDOR)[0], 16 | VENDOR_OWNED);
        assert_eq!(f.global.get(id::VENDOR)[1], VENDOR_EMPTY);
        set_i32(f.global.get_mut(id::QUICK_SELECT), 3, 16);
        set_i32(f.global.get_mut(id::EQUIPPED), 0, 16);
        let mut w = f.fields();
        w.items.iter_mut().find(|i| i.id == 16).unwrap().owned = false;
        f.apply(&w).unwrap();
        assert_eq!(f.global.get(id::OWNED)[16], 0);
        assert_eq!(f.global.get(id::VENDOR)[0], 16, "still stocked, to buy again");
        assert_eq!(i32_at(f.global.get(id::QUICK_SELECT), 3), 0);
        assert_eq!(i32_at(f.global.get(id::EQUIPPED), 0), 0);
        assert_eq!(f.global.get(id::OWNED)[28], 1);
    }

    #[test]
    fn nanotech_sets_the_flags_and_the_health() {
        let mut f = blank();
        let mut w = f.fields();
        w.nanotech = Nanotech::Ultra;
        f.apply(&w).unwrap();
        assert_eq!((f.global.get(id::FLAGS)[4], f.global.get(id::FLAGS)[5]), (1, 1));
        assert_eq!(i32_at(f.global.get(id::MAX_HEALTH), 0), 8);
        w.nanotech = Nanotech::Standard;
        f.apply(&w).unwrap();
        assert_eq!(i32_at(f.global.get(id::MAX_HEALTH), 0), 4);
        assert_eq!(f.fields().nanotech, Nanotech::Standard);
    }

    #[test]
    fn planets_keep_the_galaxy_map_in_order() {
        let mut f = blank();
        let set = |f: &mut SaveFile, p: usize, on: bool| {
            let mut w = f.fields();
            w.planets.iter_mut().find(|s| s.id == p).unwrap().unlocked = on;
            f.apply(&w).unwrap();
        };
        set(&mut f, 1, true);
        set(&mut f, 2, true);
        set(&mut f, 3, true);
        let order = |f: &SaveFile| (0..4).map(|i| i32_at(f.global.get(id::MAP_ORDER), i)).collect::<Vec<_>>();
        assert_eq!(order(&f), vec![1, 2, 3, 0]);
        set(&mut f, 2, false);
        assert_eq!(order(&f), vec![1, 3, 0, 0]);
        assert_eq!(f.global.get(id::PLANETS)[2], 0);
        let mut w = f.fields();
        w.planets.iter_mut().find(|s| s.id == 18).unwrap().unlocked = true;
        assert!(f.apply(&w).is_err(), "the last Veldin comes with the story");
    }

    #[test]
    fn gold_bolts_and_skill_points_are_written() {
        let mut f = blank();
        let mut w = f.fields();
        w.planets.iter_mut().find(|s| s.id == 14).unwrap().gold_bolts = vec![true, true, false, true];
        w.skill_points = vec![true; SKILL_POINT_COUNT];
        f.apply(&w).unwrap();
        assert_eq!(f.levels[14].get(id::GOLD_BOLTS), &[1, 1, 0, 1]);
        assert_eq!(&f.global.get(id::SKILL_POINTS)[..30], &[1; 30]);
        assert_eq!(&f.global.get(id::SKILL_POINTS)[30..], &[0, 0], "the two unused bytes stay");
        assert_eq!(SaveFile::parse(&f.to_bytes()).unwrap().fields(), w);
    }

    fn fx(f: &SaveFields) -> SaveChange {
        SaveChange::Fields { fields: f.clone() }
    }

    #[test]
    fn the_planet_can_be_changed_to_a_ship_destination() {
        let mut f = blank();
        let mut w = f.fields();
        assert_eq!(w.planet, 3);
        w.planet = 5; // Rilgar, never visited, locked
        f.apply(&w).unwrap_err(); // the page unlocks the planet with the choice
        w.planets.iter_mut().find(|s| s.id == 5).unwrap().unlocked = true;
        f.apply(&w).unwrap();
        assert_eq!(i32_at(f.global.get(id::PLANET), 0), 5);
        assert_eq!(f.global.get(id::PLANETS)[5], 1);
        assert_eq!(i32_at(f.global.get(id::MAP_ORDER), 0), 5);
        let back = SaveFile::parse(&f.to_bytes()).unwrap();
        assert_eq!(back.fields(), w);
        assert_eq!(view("SCES_509.16", 0, &f.to_bytes(), &back).planet_name.as_deref(), Some("Rilgar"));
        // The story's planets are not offered.
        for p in [0, 17, 18] {
            let mut w2 = back.fields();
            w2.planet = p;
            assert!(back.clone().apply(&w2).unwrap_err().contains("story"), "{p}");
        }
        let mut w3 = back.fields();
        w3.planet = 19;
        assert!(validate(&w3).is_err());
    }

    #[test]
    fn a_save_on_a_story_planet_keeps_it_when_other_fields_change() {
        let mut f = blank();
        set_i32(f.global.get_mut(id::PLANET), 0, 18);
        let mut w = f.fields();
        assert_eq!(w.planet, 18);
        w.bolts = 5;
        f.apply(&w).unwrap();
        assert_eq!(i32_at(f.global.get(id::PLANET), 0), 18);
    }

    #[test]
    fn raw_chunks_round_trip_and_keep_their_size() {
        let mut f = blank();
        f.global.chunks.iter_mut().find(|c| c.id == id::OWNED).unwrap().pad = vec![1, 2, 3];
        let before = f.to_bytes();
        let views = f.chunk_views();
        assert_eq!(views.len(), GLOBAL_LAYOUT.len() + LEVEL_SLOTS * LEVEL_LAYOUT.len());
        assert_eq!(views.iter().filter(|v| v.section.is_none()).count(), 47);
        let bolts = views.iter().find(|v| v.section.is_none() && v.id == 1).unwrap();
        assert_eq!((bolts.size, bolts.known, bolts.name.as_str()), (4, true, "Bolts"));
        assert!(!views.iter().find(|v| v.id == 3002).unwrap().known);
        // Every chunk written back as it is: the same file, padding included.
        let same: Vec<RawPatch> =
            views.iter().map(|v| RawPatch { section: v.section, id: v.id, hex: v.hex.clone() }).collect();
        f.apply_raw(&same).unwrap();
        assert_eq!(f.to_bytes(), before);
        // A change, in any spacing, lands where it was asked, with fresh checksums.
        f.apply_raw(&[
            RawPatch { section: None, id: 1, hex: "40E2 0100".into() },
            RawPatch { section: Some(7), id: 3003, hex: "01 00 01 00".into() },
            RawPatch { section: Some(7), id: 4000, hex: "0a000000".into() },
        ])
        .unwrap();
        let back = SaveFile::parse(&f.to_bytes()).unwrap();
        assert_eq!(back.fields().bolts, 123_456);
        assert_eq!(back.levels[7].get(id::GOLD_BOLTS), &[1, 0, 1, 0]);
        assert_eq!(i32_at(back.levels[7].get(4000), 0), 10);
        // Wrong size, bad hex, unknown chunk or section: refused.
        assert!(f.apply_raw(&[RawPatch { section: None, id: 1, hex: "00".into() }]).unwrap_err().contains("4 bytes"));
        assert!(f.apply_raw(&[RawPatch { section: None, id: 1, hex: "zz000000".into() }]).is_err());
        assert!(f.apply_raw(&[RawPatch { section: None, id: 6, hex: "".into() }]).is_err());
        assert!(f.apply_raw(&[RawPatch { section: Some(20), id: 3001, hex: "00".into() }]).is_err());
    }

    #[test]
    fn a_raw_write_goes_through_the_same_guards() {
        let card = card_with(&blank());
        let v = read_save(card.path(), "SCES_509.16", 1).unwrap();
        let raw = SaveChange::Raw { chunks: vec![RawPatch { section: None, id: 1, hex: "07000000".into() }] };
        assert!(
            write_save(card.path(), "SCES_509.16", 1, &v.fingerprint, &raw, || Ok(Some("The game".into()))).is_err()
        );
        assert!(write_save(card.path(), "SCES_509.16", 1, "0", &raw, || Ok(None)).is_err());
        let r = write_save(card.path(), "SCES_509.16", 1, &v.fingerprint, &raw, || Ok(None)).unwrap();
        assert_eq!(r.view.fields.bolts, 7);
        assert!(r.backup.name.ends_with("before_edit_slot2"));
        let serialized = serde_json::to_string(&raw).unwrap();
        assert!(serialized.starts_with(r#"{"kind":"raw","chunks":"#), "{serialized}");
    }

    fn card_with(f: &SaveFile) -> tempfile::TempDir {
        let dir = tempfile::tempdir().unwrap();
        let game = dir.path().join("BESCES-50916RATCHET");
        fs::create_dir_all(&game).unwrap();
        fs::write(game.join("save1.bin"), f.to_bytes()).unwrap();
        let mut empty = blank();
        set_i32(empty.global.get_mut(id::PLANET), 0, -1);
        fs::write(game.join("save2.bin"), empty.to_bytes()).unwrap();
        dir
    }

    #[test]
    fn a_write_backs_up_first_and_replaces_the_file() {
        let card = card_with(&blank());
        let v = read_save(card.path(), "SCES_509.16", 1).unwrap();
        let mut want = v.fields.clone();
        want.bolts = 42;
        let r = write_save(card.path(), "SCES_509.16", 1, &v.fingerprint, &fx(&want), || Ok(None)).unwrap();
        assert_eq!(r.view.fields.bolts, 42);
        assert!(r.backup.name.starts_with(".backup_") && r.backup.name.ends_with("before_edit_slot2"));
        let kept = fs::read(Path::new(&r.backup.path).join("BESCES-50916RATCHET/save1.bin")).unwrap();
        assert_eq!(SaveFile::parse(&kept).unwrap().fields().bolts, 0, "the backup is the save before the edit");
        assert_eq!(read_save(card.path(), "SCES_509.16", 1).unwrap().fields.bolts, 42);
        // A second edit in the same second keeps the first backup.
        let v2 = read_save(card.path(), "SCES_509.16", 1).unwrap();
        let r2 = write_save(card.path(), "SCES_509.16", 1, &v2.fingerprint, &fx(&v2.fields), || Ok(None)).unwrap();
        assert_ne!(r2.backup.name, r.backup.name);
        let kept = fs::read(Path::new(&r.backup.path).join("BESCES-50916RATCHET/save1.bin")).unwrap();
        assert_eq!(SaveFile::parse(&kept).unwrap().fields().bolts, 0);
        assert!(!card.path().join("BESCES-50916RATCHET/.save1.bin.editing").exists());
    }

    #[test]
    fn a_write_is_refused_when_it_is_not_safe() {
        let card = card_with(&blank());
        let v = read_save(card.path(), "SCES_509.16", 1).unwrap();
        let before = fs::read(card.path().join("BESCES-50916RATCHET/save1.bin")).unwrap();
        let running =
            write_save(card.path(), "SCES_509.16", 1, &v.fingerprint, &fx(&v.fields), || Ok(Some("The game".into())));
        assert!(running.unwrap_err().contains("running"));
        let unsure =
            write_save(card.path(), "SCES_509.16", 1, &v.fingerprint, &fx(&v.fields), || Err("no process list".into()));
        assert!(unsure.is_err());
        let stale = write_save(card.path(), "SCES_509.16", 1, "0000", &fx(&v.fields), || Ok(None));
        assert!(stale.unwrap_err().contains("changed"));
        let mut bad = v.fields.clone();
        bad.bolts = MAX_BOLTS + 1;
        assert!(write_save(card.path(), "SCES_509.16", 1, &v.fingerprint, &fx(&bad), || Ok(None)).is_err());
        assert_eq!(fs::read(card.path().join("BESCES-50916RATCHET/save1.bin")).unwrap(), before);
        let backups = fs::read_dir(card.path())
            .unwrap()
            .flatten()
            .filter(|e| e.file_name().to_string_lossy().starts_with(".backup_"))
            .count();
        assert_eq!(backups, 0, "nothing written, nothing backed up");
        assert!(read_save(card.path(), "SCES_509.16", 2).unwrap_err().contains("empty"));
        assert!(read_save(card.path(), "SCUS_973.53", 1).is_err(), "another game's card");
        assert!(read_save(card.path(), "SCES_509.16", 5).is_err());
    }

    #[test]
    fn both_rac1_discs_use_the_editor() {
        assert!(edits_serial("SCES_509.16") && edits_serial("SCUS_971.99"));
        assert!(!edits_serial("SCUS_972.68"));
    }

    #[test]
    fn tells_which_processes_use_the_card() {
        let card = Path::new("/Users/you/Library/Application Support/OpenRAC/memcard/SCES_509.16");
        let s = "SCES_509.16";
        let bundle = "/x/apps/openrac-rac1-pal/Ratchet & Clank (PAL).app/Contents/MacOS/Ratchet & Clank (PAL) --data /d --window";
        assert!(uses_card(bundle, card, s));
        assert!(uses_card("/x/openrac-rac1-pal --data /d", card, s));
        assert!(!uses_card("/x/openrac-rac1-ntsc --data /d", card, s), "the other version's default card");
        assert!(!uses_card("/x/openrac-rac1-pal --data /d --no-card", card, s));
        let elsewhere = "/x/openrac-rac1-pal --cards /tmp/copy/SCES_509.16x --data /d";
        assert!(!uses_card(elsewhere, card, s));
        let here = format!("/x/openrac-rac1-ntsc --cards {} --window", card.display());
        assert!(uses_card(&here, card, s), "any version pointed at this card");
        assert!(uses_card("/x/openrac-rac1-pal --cards copy/SCES_509.16 --data /d", card, s), "relative: assume it is");
        assert!(!uses_card("/usr/bin/vim notes.txt", card, s));
        assert!(!uses_card("/Applications/OpenRAC Launcher.app/Contents/MacOS/openrac-launcher", card, s));
        let shell =
            format!("/bin/zsh -c cd x && port/build/release/openrac-rac1-pal --cards {} --window", card.display());
        assert!(!uses_card(&shell, card, s), "the shell that started a game is not the game");
        let windows = r#""C:\\Games\\OpenRAC\\openrac-rac1-pal.exe" --data C:\\d --window"#;
        assert!(uses_card(windows, card, s));
    }
}
