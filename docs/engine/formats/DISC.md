# The disc: file system, table of contents, level headers

How Ratchet & Clank finds its data on the disc: an almost empty ISO 9660 file
system, a table of contents at a fixed sector, and one header per level that
points at everything the level streams. The layouts are ReRAC's, written from
Wrench's reader and checked by ReRAC on the NTSC-U disc (`SCUS_971.99`); the
PAL column gives what OpenRAC measured on `SCES_509.16` where it has
([ASSETS.md](../../../games/rac1/docs/ASSETS.md)).

**Games.** Everything below is measured on RAC1. The comparison with the
sequels in the last sections is Wrench's knowledge as ReRAC records it
(**reference**); only the RAC2 points marked as such are measured in OpenRAC,
on prerelease discs ([LEVEL-ARCHIVE-FORMAT.md](../../../games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md)).

All integers are little-endian; sectors are 0x800 bytes; an LBA is a sector
number from the start of a 2048-byte-per-sector image.

## 1. Range types

The disc tables use four kinds of range:

| Name | Size | Layout | Meaning |
|---|---:|---|---|
| `Sector32` | 4 | `s32 sector` | A start sector alone; `<= 0` is absent |
| `SectorRange` | 8 | `s32 offset; s32 size` | Both in sectors |
| `SectorByteRange` | 8 | `s32 offset; s32 size_bytes` | Start in sectors, exact size in bytes |
| `ByteRange` | 8 | `s32 offset; s32 size` | Bytes, relative to the enclosing file; used inside level data, never in the disc tables. An absent entry is `{-1, 0}` |

## 2. The ISO 9660 file system

The file system exists for the console's boot ROM. Once the executable runs,
the game reads everything else by absolute sector, so its content cannot be
listed by walking directories.

| File | NTSC-U | PAL ([ASSETS.md](../../../games/rac1/docs/ASSETS.md#the-disc)) |
|---|---|---|
| `SYSTEM.CNF` | root, LBA 289 | LBA 289, 58 bytes |
| Boot executable | `SCUS_971.99` | `SCES_509.16`, LBA 290 |
| IOP reboot image | not recorded by ReRAC | `IOPRP243.IMG`, LBA 968 |

- **`SYSTEM.CNF` is pinned at LBA 289.** ReRAC records that the game uses
  that sector number directly; if the file is elsewhere, the game picks the
  wrong memory-card directory. On PAL, `func_00201E88` reads that sector and
  checks the region. Wrench gives sector 1000 for the sequels (**reference**).
- **Its text**: `BOOT2 = cdrom0:\<executable>;1`, `VER = <version>` and
  `VMODE = NTSC|PAL`, CRLF line ends. RAC1 has no space before each CRLF and
  ends with a blank line; the sequels put one space before each CRLF.
- **The volume identifier** is `RATCHETANDCLANK`, space-padded to 32
  characters.
- **The whole file system lies before sector 1500**, where the table of
  contents starts.
- **Sectors 0–11** hold the console's boot logo, obfuscated: XOR every byte
  with the region's first byte, then rotate left by 3. The bitmap is 8-bit
  grey, 384 × 64 (NTSC) or 344 × 71 (PAL).

## 3. Identifying the build

The boot executable's file name is the product code. ReRAC's list for RAC1
(from Wrench):

| Executable | Region | Build |
|---|---|---|
| `SCUS_971.99` | US | original and Greatest Hits |
| `SCUS_972.09`, `SCUS_972.40` | US | demos |
| `SCES_509.16` | Europe | Black Label and Platinum |
| `SCED_510.75` | Europe | demo |
| `SCPS_150.37` | Japan | original |

Wrench's fallback when the name is unknown searches the executable for
`Deadlocked`, `Up Your Arsenal`, `Going Commando`, then `Ratchet & Clank`, in
that order, because the sequels contain the first game's title too. OpenRAC
identifies builds by the executable's serial, size and SHA-1 instead
(`games/<game>/game.json`, [docs/port/DESIGN.md](../../port/DESIGN.md)).
ReRAC's NTSC-U disc (`VER = 1.00`, image 4,214,095,872 bytes) has the
executable SHA-1 `72fd1de3…f3fd3cd5` that `games/rac1/game.json` records.
Frame rates: NTSC 59.94 Hz, PAL 50 Hz.

## 4. The table of contents (LBA 1500)

The game reads six sectors at LBA 1500 and keeps 0x2960 bytes:

| | NTSC-U | PAL |
|---|---|---|
| Loader | 0x12F2B8 | `func_0012F3F8` |
| Copy in memory | 0x137B80 | `D_00137C80` |
| Level table in the copy (+0x28C8) | 0x13A448 | `D_0013A548` |

The structure is one flat record of absolute sector ranges:

| Offset | Count | Type | Wrench's name | Content |
|---|---:|---|---|---|
| 0x0000 | 1 | s32 | version | 1, the magic |
| 0x0004 | 1 | s32 | header_size | 0x2960 |
| 0x0008 | 1 | SectorRange | debug_font | paletted 8-bit texture |
| 0x0010 | 1 | SectorRange | save_game | save template |
| 0x0018 | 28 | SectorRange | ratchet_seqs | WAD-compressed animation sequences |
| 0x00F8 | 20 | SectorRange | hud_seqs | WAD-compressed |
| 0x0198 | 1 | SectorRange | vendor | |
| 0x01A0 | 37 | SectorRange | vendor_audio | VAG |
| 0x02C8 | 12 | SectorRange | help_controls | 8-bit textures, not compressed |
| 0x0328 | 15 | SectorRange | help_moves | WAD-compressed 8-bit textures |
| 0x03A0 | 15 | SectorRange | help_weapons | the same |
| 0x0418 | 14 | SectorRange | help_gadgets | the same |
| 0x0488 | 7 | SectorRange | help_ss | the same (screenshots) |
| 0x04C0 | 7 | SectorRange | options_ss | the same |
| 0x04F8 | 1 | SectorRange | frontbin | a code overlay in the format of section 7 |
| 0x0500 | 81 | SectorRange | mission_ss | WAD-compressed textures |
| 0x0788 | 19 | SectorRange | planets | the same, one per level |
| 0x0820 | 38 | SectorRange | stuff2 | unknown, WAD-compressed |
| 0x0950 | 10 | SectorRange | goodies_images | WAD-compressed textures |
| 0x09A0 | 19 | SectorRange | character_sketches | the same |
| 0x0A38 | 19 | SectorRange | character_renders | the same |
| 0x0AD0 | 31 | SectorRange | skill_images | the same |
| 0x0BC8 | 5 × 12 | SectorRange | epilogue_<language> | English, French, Italian, German, Spanish |
| 0x0DA8 | 30 | SectorRange | sketchbook | WAD-compressed textures |
| 0x0E98 | 4 | SectorRange | commercials | the same |
| 0x0EB8 | 9 | SectorRange | item_images | the same |
| 0x0F00 | 240 | Sector32 | qwark_boss_audio | VAG, start sector only |
| 0x12C0 | 1 | SectorRange | irx | WAD-compressed bundle of IOP modules, layout unknown |
| 0x12C8 | 4 | SectorRange | spaceships | |
| 0x12E8 | 20 | SectorRange | (unknown, animation-like) | |
| 0x1388 | 6 | SectorRange | space_plates | WAD-compressed texture lists |
| 0x13B8 | 1 | SectorRange | transition | WAD-compressed; see [CUTSCENES.md](../systems/CUTSCENES.md) |
| 0x13C0 | 36 | SectorRange | space_audio | VAG |
| 0x14E0 | 1 | SectorRange | sound_bank | the global 989snd bank |
| 0x14E8 | 1 | SectorRange | (unknown) | Wrench's comments put this and the next field at 0x14E0 by mistake |
| 0x14F0 | 1 | SectorRange | music | VAG |
| 0x14F8 | 1 | SectorRange | hud_header | |
| 0x1500 | 5 | SectorRange | hud_banks | not compressed here, unlike in level data |
| 0x1528 | 1 | SectorRange | all_text | the localised interface text; see [HUD_TEXT.md](../systems/HUD_TEXT.md) |
| 0x1530 | 28 | SectorRange | things | unknown |
| 0x1610 | 1 | SectorRange | post-credits sequence | WAD-compressed |
| 0x1618 | 18 | SectorRange | post_credits_audio | VAG |
| 0x16A8 | 20 | SectorRange | credits_images_ntsc | raw RGBA, 512 × 416 |
| 0x1748 | 20 | SectorRange | credits_images_pal | raw RGBA, 512 × 448 |
| 0x17E8 | 2 | SectorRange | wad_things | unknown, WAD-compressed |
| 0x17F8 | 88 | SectorByteRange | mpegs | PSS movies, exact sizes ([PSS.md](PSS.md)) |
| 0x1AB8 | 900 | Sector32 | help_audio | VAG, start sector only |
| 0x28C8 | 19 | SectorRange | levels | the level table (section 5) |

The field sizes sum to exactly 0x2960, which confirms the offsets. The same
bytes read as `SectorRange[479]`, `Sector32[240]`, `SectorRange[167]`,
`SectorByteRange[88]`, `Sector32[900]`, `SectorRange[19]` after the two header
words; a tool that moves sectors relocates them as one block. OpenRAC's PAL
table of the groups in use is in [ASSETS.md](../../../games/rac1/docs/ASSETS.md#table-of-contents-groups).

**Sizes of bare start sectors.** For a `Sector32`, read the data: a VAG
header (`VAGp`, big-endian payload size at +0x0C) gives 0x30 + that size; a
`WAD` header gives its compressed size at +3 ([WAD.md](WAD.md)); otherwise one
sector. A third way, used for the sequels: sort every referenced start sector
and take the distance to the next.

## 5. The level header (0x2434 bytes)

Each level table entry's offset is the absolute LBA of the level's one
header; the size field is not used (Wrench writes 1). On PAL, `func_0012F4A8`
(NTSC-U 0x12F368) reads five sectors of it and `func_00204C60` (NTSC-U
0x204428) loads the first three ranges.

| Offset | Type | Name | Content |
|---|---|---|---|
| 0x0000 | s32 | id | level number |
| 0x0004 | s32 | header_size | 0x2434, the signature |
| 0x0008 | SectorRange | data | level data ([LEVEL.md](LEVEL.md)) |
| 0x0010 | SectorRange | gameplay_ntsc | WAD-compressed gameplay file |
| 0x0018 | SectorRange | gameplay_pal | the same for PAL; same size, just after the NTSC one |
| 0x0020 | SectorRange | occlusion | not compressed ([OCCLUSION.md](OCCLUSION.md)) |
| 0x0028 | 36 × SectorByteRange | bindata | level audio; slot meanings unknown |
| 0x0148 | 15 × Sector32 | music | VAG music streams |
| 0x0184 | 15 × SceneRecord | scenes | the level's cutscenes |

- **The two gameplay files** are both complete; they differ in the help
  text. Both are on both discs.
- **Scene records are 0x250 bytes, 15 of them.** ReRAC corrected Wrench's
  reading (30 records of 0x128 bytes of six "sounds" and 68 "wads") from the
  game's code, which indexes 0x13A664 + id × 0x250 (NTSC-U):

  | Offset | Count | Content |
  |---|---:|---|
  | 0x000 | 6 | speech VAG per language: 0 English, 1 unused, 2 French, 3 German, 4 Spanish, 5 Italian |
  | 0x018 | 71 | NTSC chunk sectors, then a one-sector all-zero sentinel |
  | 0x134 | 71 | PAL chunk sectors, the same (PAL chunks are 80 ticks long) |

  A chunk's size is the difference to the next entry; ReRAC checked that
  this equals the padded WAD size for all 4,081 chunks. The chunks and their
  playback are in [CUTSCENES.md](../systems/CUTSCENES.md).
- **No stored group sizes.** A level's sectors fall in three groups: level
  (data, gameplay, occlusion), audio (bindata, music) and scene. Their
  extents are inferred from the union of what each references; either of the
  last two can be absent. The audio and scene data can sit before the
  header.
- **Discovery.** Wrench scans the table of contents in steps of 8 from +8
  and accepts a sector whose header has 0x2434 at +4; on retail RAC1 the hits
  are exactly the 19 level table entries.
- **Wrench's rewritten headers.** When Wrench unpacks a RAC1 level, it writes
  three files of its own with headers of 0x30 (level), 0x164 (audio) and
  0x22B8 (scene) bytes and sector numbers relative to the file. These sizes
  identify files Wrench produced; they never occur on a disc.

## 6. Level sector ranges

PAL figures are in [ASSETS.md](../../../games/rac1/docs/ASSETS.md#level-ranges).
ReRAC's disc reader gives byte-identical level lumps to its earlier extractor
on all 19 NTSC-U levels (an image of 2,057,664 sectors).

## 7. Code overlays ("Ratchet executable")

A level's code, and the table of contents' `frontbin`, are a bare sequence
of 16-byte headers each followed by its data; no file header, no count.

| Offset | Field |
|---|---|
| 0x0 | destination address |
| 0x4 | byte count that follows |
| 0x8 | ELF section type (1 PROGBITS, 8 NOBITS); not used by the game |
| 0xC | entry point, the same in every header |

The game stops when a header's entry point differs from the first one's, so
it reads one header past the data; a reader should also stop at the end of
the lump. PAL's loader is ParseBin, `func_0012DA38` (NTSC-U 0x12D8F8), called
from `main` `func_0012DB18` (NTSC-U 0x12D9D8). Every level has seven records
in Wrench's section order `.lit`, `.bss`, `.data`, `lvl.vtbl`, `lvl.camvtbl`,
`lvl.sndvtbl`, `.text` ([README.md, section 2](../README.md#loading-a-levels-program)).
RAC1's boot executable is an ordinary ELF.

## 8. How the sequels differ

From Wrench, as ReRAC records it. The table of contents, the level table and
the level headers were measured on the three NTSC-U retail discs on
2026-10-20, when the editor learned to read them: [LATER_GAMES.md](LATER_GAMES.md)
has what was found.

| | RAC1 | Going Commando, Up Your Arsenal, Deadlocked |
|---|---|---|
| Table of contents | LBA 1500, version word 1 | LBA 1001, no magic (RAC2 prerelease: `RC2.HDR` at LBA 1001, measured) |
| `SYSTEM.CNF` sector | 289 | 1000 |
| Global data | one table, the table of contents | 8 to 10 separate global WADs (mpeg, misc, hud, bonus, audio, space, scene, gadget, armor, online), each header starting `{size, sector}` |
| Level table | one entry per level, one 0x2434 header with absolute sectors | three entries (level, audio, scene; RAC3 and Deadlocked order audio first), each header relative to its file |
| Assets as named files | no | RAC2 only (`LEVELn.WAD`, `SCENEn.WAD`, measured on prerelease discs) |
| Boot executable | plain ELF | RAC2 plain; RAC3 and Deadlocked an ELF stub with a WAD-compressed image inside (RAC4 measured: [RESEARCH.md](../../../games/rac4/ntsc/docs/RESEARCH.md)) |
| Level chunks | none | three streamed chunks, each `{tfrags, collision}` with its own sound bank |

Wrench classifies the sequels' headers by their size alone (for example
RAC2 level 0x60, audio 0x1018, scene 0x137C; Deadlocked level 0xC68). The
table is in ReRAC's `disc_layout.md` section 4; it is ambiguous by
construction. The editor ([wads.py](../../../editor/wads.py)) uses only the
sizes the three NTSC-U discs have, and checks each against its game's
level header size.

## Open

- RAC1: the 36 `bindata` slots, the `irx` bundle's layout, the unknown
  table-of-contents groups.
- RAC2–RAC4: the global WADs are not read; the audio and scene headers are
  located, not decoded. RAC2's prerelease layout is in its own doc.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/disc_layout.md` and
`docs/formats/wad_layouts_rac1.md` sections 0–1; PAL facts from
[ASSETS.md](../../../games/rac1/docs/ASSETS.md) and
`games/rac1/config/overlays/us_map.tsv`.
