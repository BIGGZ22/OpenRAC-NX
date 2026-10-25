# Engine knowledge across the games

What is known about Insomniac's PlayStation 2 engine across the games OpenRAC
covers, where each fact is written down, and which game and build it was
measured on. The per-game docs are the authority. This page summarises them
and links each claim to its source, so that someone starting on a subsystem
in one game can see what has already been measured in another.

## 1. How to read this page

Each claim names the game it was measured on, using these keys:

| Key | Disc | Directory |
|---|---|---|
| **RAC1 PAL** | Ratchet & Clank, `SCES_509.16` v2.00 | [games/rac1](../../games/rac1/README.md) |
| **RAC1 US** | Ratchet & Clank, `SCUS_971.99` | [games/rac1/ntsc](../../games/rac1/ntsc/README.md) (Lombyte) |
| **RAC2** | Going Commando, `SCUS_972.68` v1.01 | [games/rac2/ntsc](../../games/rac2/ntsc/README.md) |
| **RAC3** | Up Your Arsenal, `SCUS_973.53` | [games/rac3/ntsc](../../games/rac3/ntsc/README.md) |
| **RAC4** | Deadlocked, `SCUS_974.65` | [games/rac4/ntsc](../../games/rac4/ntsc/README.md) |

And how strong the evidence is:

| Level | Meaning |
|---|---|
| **retail** | Measured on the retail program or disc of that key: matched code, the project's own readers, byte comparisons. |
| **prerelease** | Measured on a demo, preview, review or prototype disc. A reference only, never the evidence for a match ([SOURCING.md](../policy/SOURCING.md#prerelease-builds)). |
| **inferred** | Concluded from names, call context or a single function, not traced through the code. |
| **reference** | Another project's description (Wrench, ReRAC, a sister decompilation, a community note) that the cited doc has not re-measured. |

Three rules follow from how the projects work:

- An address, offset or name belongs to the build it was measured on. A PAL
  address does not identify a US, RAC2 or RAC3 function. In the range a level
  replaces, every RAC1 US constant is 0x100 lower than PAL's
  ([RAC1 US overlays.md](../../games/rac1/ntsc/docs/overlays.md)).
- A name moves to another game only when the code at the new address does
  the same thing. RAC2 tested eleven prototype symbol addresses this way and
  one held ([ENGINE-SYMBOL-NAMES.md](../../games/rac2/ntsc/docs/ENGINE-SYMBOL-NAMES.md)).
- Some committed names and layouts come from prerelease builds; their status
  is undecided ([OPEN_QUESTIONS.md](../policy/OPEN_QUESTIONS.md), section 3).

## 2. The games and their programs

| Key | Boot executable | Level programs | Other programs | What the project targets |
|---|---|---|---|---|
| RAC1 PAL | `SCES_509.16`, 1,388,100 bytes | 19, one in each level's data | `IOPRP243.IMG`, an IOP reboot image | boot and all 19 levels |
| RAC1 US | `SCUS_971.99`, 1,383,028 bytes | 19 | | boot and all 19 levels |
| RAC2 | `SCUS_972.68`, 2,618,684 bytes, two loadable segments | 27 | | boot and 27 overlays rebuilt from assembly; C replaces it function by function |
| RAC3 | `SCUS_973.53`, 771,008 bytes | 51, single player and multiplayer | `frontbin.elf`, `boot_elf.elf`, `i5bootn.elf`, `ntgui.elf`, `sly2.elf` | `frontbin.elf`, `boot_elf.elf` (the main executable) and `i5bootn.elf` (the launcher), each rebuilt byte for byte ([targets.md](../../games/rac3/ntsc/docs/targets.md)); levels and the other executables counted, not compiled |
| RAC4 | `SCUS_974.65`, 1,692,216 bytes: a 20 KB loader around one compressed image, which unpacks to 17 sections (5,157,636 bytes) | 47 (24 campaign, 23 multiplayer), each with its own overlay on the disc | the IOP image and the DNAS and network GUI files | the image's core, network and level code (8,027 functions) and the overlays, function by function; nothing is linked ([RESEARCH.md](../../games/rac4/ntsc/docs/RESEARCH.md)) |

Sizes are from each `game.json`. RAC2's level identifiers run 0–20, 22–26
and 30 ([config/overlays.json](../../games/rac2/ntsc/config/overlays.json)),
and the boot's `map level` strings cover fewer levels than the disc
([README](../../games/rac2/ntsc/README.md)). RAC3's levels sit in
`levels/singleplayer` and `levels/multiplayer` of the Wrench unpack
([Setup.md](../../games/rac3/ntsc/docs/wiki/Setup.md)).

### One shape: a resident core and a replaceable program

RAC1 and RAC2 boot executables have the same sections in the same order
(**retail** on both):

| Part | RAC1 PAL ([splat.yaml](../../games/rac1/config/splat.yaml)) | RAC2 ([boot-sections.json](../../games/rac2/ntsc/config/boot-sections.json)) |
|---|---|---|
| VU microcode | `vutext` at 0x100080, 0x12300 bytes | `.vutext` at 0x100080, 86,368 bytes |
| Core, never replaced | `core_text`, `core_data`, `core_rdata`, `core_bss`, `core_lit` | `core.text` (0x115200), `core.data`, `core.rdata`, `core.bss`, `core.lit` |
| Replaced by each level | `lit` (0x15F000), `bss`, `data`, `lvl_vtbl`, `lvl_camvtbl`, `lvl_sndvtbl`, `text` (0x1E9080) | `.lit` (0x1A7C80), `.bss`, `.data`, `lvl.vtbl`, `lvl.camvtbl`, `lvl.sndvtbl`, `.text` (0x26EA00) |
| Extra load | none | 88,931 bytes at 0x01800000 |
| `$gp` | 0x166D00 (US 0x166C00), never changed by a level | 0x1AEFF0 |

- **RAC1**: the executable's program runs before the first level loads. It
  is a subset of every level's program, linked in the same order
  ([OVERLAYS.md, "Layout"](../../games/rac1/docs/OVERLAYS.md#layout);
  on US, [overlays.md](../../games/rac1/ntsc/docs/overlays.md)).
- **RAC2**: a prototype's section table calls the two halves CORE and
  FRONTEND (**prerelease**). The retail boot keeps that shape; its frontend
  main loop is at 0x0026EDE8 (**retail**,
  [ENGINE-SYMBOL-NAMES.md](../../games/rac2/ntsc/docs/ENGINE-SYMBOL-NAMES.md)).
- **RAC3**: `frontbin.elf` is itself a level overlay, the "front end level",
  with `lit`, data and the three `lvl_*` tables. `SCUS_973.53` loads it at
  0x1D5680 and calls 0x37D200 ([full_match_roadmap.md](../../games/rac3/ntsc/docs/full_match_roadmap.md),
  [Toolchain-and-Build.md](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md)).
  Its `$gp` accesses resolve against 0x1DC8B0
  ([Matching-Patterns.md](../../games/rac3/ntsc/docs/wiki/Matching-Patterns.md)).

### Loading a level's program

- **RAC1 PAL, retail.** The level data's first section is a list of records
  `(load address, size, type, entry point)`, each followed by its bytes.
  ParseBin (`func_0012DA38`, in the object the NTSC decomp's split calls
  `boot.cpp`) copies records until the entry point changes and returns it;
  the main loop (`func_0012DB18`) calls it. Every level has seven records,
  types 1, 8, 1, 1, 1, 1, 1: lit, bss, data, vtbl, camvtbl, sndvtbl, text
  ([ASSETS.md](../../games/rac1/docs/ASSETS.md#code-overlays),
  [DECOMP_PROGRESS.md](../../games/rac1/docs/DECOMP_PROGRESS.md)).
- **RAC1 US, retail.** The same records. Each level's entry point is a
  shared function, the level initialiser (0x245C28 in level 00 to 0x247F10
  in level 18) ([overlays.md](../../games/rac1/ntsc/docs/overlays.md)).
- **RAC2, prerelease.** The same record layout and stop rule; types 1, 8
  and 9 (REL) seen; seven sections in the same order; `.lit` at one address
  in all 27 levels ([LEVEL-ARCHIVE-FORMAT.md](../../games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md)).
  The retail overlays come from Wrench's unpack as 27 ELF files pinned by
  SHA-256 ([config/overlays.json](../../games/rac2/ntsc/config/overlays.json)).
- **RAC3.** Overlays are unpacked with Wrench; the loader is not documented.

### How much code

| Key | Measure | Source |
|---|---|---|
| RAC1 PAL | Executable game code 348K; about 3.2 MB of distinct level code not in the executable; about 3.5 MB of distinct game code in all, ten times the executable's (3.7 MB with `core_text`) | [ASSETS.md](../../games/rac1/docs/ASSETS.md#code-overlays) |
| RAC1 US | Level text records 1,065,136–1,200,936 bytes; distinct level code about 3.2 MB, nine times the executable's | [overlays.md](../../games/rac1/ntsc/docs/overlays.md) |
| RAC2 | 48,788,176 executable and initialised-data bytes over 28 programs; code repeats about 1.23 times across them | [README](../../games/rac2/ntsc/README.md), [SECOND-C-LOT.md](../../games/rac2/ntsc/docs/SECOND-C-LOT.md) |
| RAC3 | 51 overlays: about 100 MB of function bytes, 9.3–9.4 MB distinct; other executables about 3.0 MB of code in 15,146 functions | [Toolchain-and-Build.md](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md), [shared_code_findings.md](../../games/rac3/ntsc/docs/shared_code_findings.md) |

## 3. Mobys

Mobys are the objects with behaviour: Ratchet, crates, bolts, enemies, NPCs,
vendors, platforms ([ASSETS.md](../../games/rac1/docs/ASSETS.md#mobys)).

### The runtime record

The runtime moby record is 0x100 bytes: RAC1 PAL's `InitMobyInstance`
(`func_0020D440`) clears that many and derives the moby's slot from its
offset in the moby array, shifted right by 8 (**retail**,
[src/game/mobyfunc.c](../../games/rac1/src/game/mobyfunc.c)). RAC2's
reference notes give the same size and leave RAC2's offsets to be measured
([COMMUNITY-ENGINE-REFERENCE.md](../../games/rac2/ntsc/docs/COMMUNITY-ENGINE-REFERENCE.md)).

The field layout is still to be measured from retail code, game by game. A
field list committed in RAC1 PAL's headers has an unresolved provenance and
is not a layout to build on ([OPEN_QUESTIONS.md](../policy/OPEN_QUESTIONS.md),
section 3). Lombyte's
[moby-selector-scratch-path.md](../../games/rac1/ntsc/docs/moby-selector-scratch-path.md)
shows the method on one retail data path (RAC1 US).

### Placement records

RAC1 PAL: **reference** layouts from ReRAC, checked on all 19 PAL levels
([ASSETS.md](../../games/rac1/docs/ASSETS.md#mobys),
[editor/mobys.py](../../editor/mobys.py)).

- The gameplay file's word at +0x44 points to a count, 12 bytes of padding,
  then 0x78-byte instance records: 16,232 on the 19 levels.
- Class at +0x18, instance scale at +0x1c, spawn flags and spawn id at
  +0x08 and +0x0c (save-state tests that decide whether the moby is
  created), position at +0x30, Euler angles at +0x3c with
  R = Rz · Ry · Rx, pvar index at +0x58.
- Ties (+0x34, 0xe0-byte records) and shrubs (+0x3c, 0x70-byte records) are
  placed the same way.

RAC2, RAC3 and RAC4: **retail**, Wrench's layouts read on every level of
the three NTSC-U discs: 0x88-byte moby records in RAC2 and RAC3 (gameplay
+0x4c), 0x70-byte ones in RAC4, ties of 0x60 bytes, shrubs as RAC1's
([formats/LATER_GAMES.md](formats/LATER_GAMES.md#29-instances)). No
placement format is documented for RAC1 US.

### Classes

- **RAC1 PAL, reference checked on retail data.** Core index +0x18 holds
  32-byte class entries (core offset, class number, 16 texture slots); a
  class blob has a 0x48-byte header. The 19 levels carry 2,972 blobs, 2,934
  with a mesh. The 21 gadget classes keep theirs WAD-compressed in the
  gadget table (core index +0x80), not decoded. Class 0 is Ratchet; his
  sequences are the 256 offsets at core index +0x78
  ([ASSETS.md](../../games/rac1/docs/ASSETS.md#moby-classes)).
- **RAC2, retail.** Wrench enumerates 1,476 classes; the dispatch tables use
  1,158 distinct identifiers, 1,119 of them in Wrench's list
  ([MOBY-DISPATCH-TABLES.md](../../games/rac2/ntsc/docs/MOBY-DISPATCH-TABLES.md)).

### Update dispatch tables

Every RAC1 and RAC2 level program carries three dispatch tables in its own
records; RAC3's `frontbin.elf` and levels have the same three sections
([Toolchain-and-Build.md](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md)).

| Table | RAC1 PAL and US, retail | RAC2, retail |
|---|---|---|
| `vtbl` | 12 bytes: `{oClass, update function, pointer to a 6-word table}`; 100–189 classes per level (US); ends at oClass -1 | 3 words: `{oClass, handler, auxiliary pointer}`; 5,788 records over 27 levels; ends on padding; third word the same for every record of a level in 26 of 27 levels |
| `camvtbl` | 20 bytes: `{id, init, activate, update, exit}`; 6–9 per level; ends at -1 | the same five words; 217 records; ends at 0xffffffff |
| `sndvtbl` | 8 bytes: `{id, function}`; 0–6 per level; ends at -1 | 2 words; 32 records; ends at 0xffffffff |

Sources: [OVERLAYS.md, "Roles"](../../games/rac1/docs/OVERLAYS.md#roles),
[overlays.md](../../games/rac1/ntsc/docs/overlays.md),
[MOBY-DISPATCH-TABLES.md](../../games/rac2/ntsc/docs/MOBY-DISPATCH-TABLES.md).

- An update function receives the moby in `$a0` (RAC1 PAL). PAL names 690
  functions by table role (`UpdateMoby_<oClass>`, `InitCamera_<id>`,
  `SoundFunc_<id>`); the oClass numbers are RAC1's own.
- RAC1 table ids agree between builds: for all 3,669 class, camera and sound
  ids found in both, the US handler maps to the PAL one
  ([OVERLAYS.md](../../games/rac1/docs/OVERLAYS.md#us-map)).
- In RAC2, 96.7% of handlers follow a `jr ra` within three words. Identifiers
  from a 2003 prototype's tables transfer to retail (282 of 282 merged pairs),
  giving 6,356 proposed names (**prerelease** identifiers).
- The editor's RAC1 class names come from Lombyte, which joins each level's
  dispatch table with Wrench's class names
  ([editor/moby_classes.tsv](../../editor/moby_classes.tsv)). Lombyte also
  lists `update/mobyNNN.cpp` module names seen in 2002 development builds
  (**prerelease**, [engine-source-layout.md](../../games/rac1/ntsc/docs/engine-source-layout.md)).

## 4. Level data

### RAC1 PAL

Measured on the PAL disc and its code; field names and most layouts come
from Wrench and ReRAC, as [ASSETS.md](../../games/rac1/docs/ASSETS.md)
says for each.

- **Disc.** ISO 9660 names three files: `SYSTEM.CNF`, `SCES_509.16`,
  `IOPRP243.IMG`. Everything else is addressed by absolute sector.
- **Table of contents.** `func_0012F3F8` reads 0x2960 bytes at LBA 1500:
  (LBA, size) groups for global data (debug font, save game, HUD, help,
  MPEG video, IOP modules, audio and more). Group names are Wrench's; three
  are confirmed by their loaders.
- **Levels.** A table at +0x28c8 gives 19 (header LBA, total sectors)
  entries. `func_0012F4A8` reads a 0x2434-byte header: sector ranges for
  level data, NTSC gameplay, PAL gameplay (both on the PAL disc, the same
  size) and occlusion, then audio and scene references (not decoded).
- **WAD compression.** `func_0020C468`: a 16-byte `"WAD"` header, then an
  LZO-like stream whose block markers skip to the next 0x2000-byte block.
  Wrench aligns that skip to 0x1000; the PAL code uses 0x2000.
- **Level data**: eleven (offset, size) pairs: code overlay, sound bank,
  core index, GS RAM, HUD header, five HUD banks, core data. The
  WAD-compressed core index locates the terrain, occlusion, sky and
  collision blocks, the class, texture and gadget tables.
- **Collision** (core index +0x14): a three-level grid of 4-unit cells,
  quantised vertices, a type byte per face whose low five bits are a surface
  id, and hero-only groups. Layout from ReRAC, checked on PAL
  ([ASSETS.md](../../games/rac1/docs/ASSETS.md#collision),
  [editor README](../../editor/README.md#collision-layer)).

### RAC1 US

Lombyte keeps each level's disc location and program records as JSON under
`config/overlays/us/` ([overlays.md](../../games/rac1/ntsc/docs/overlays.md)).
Its level loader agrees with the PAL reading above: the header ranges at
0x10 and 0x18 are the two gameplay archives, not sound (corrected there on
2026-10-06, Lombyte pull request 112). It documents the sky,
tfrag and gameplay file layouts and the class tables at the functions that
read them (`src/world/loaders/relocate_sky_definition.c`,
`src/rendering/draw_tfrag.c`).
ReRAC finds four fewer moby class blobs and slightly lower collision totals
on the US disc ([ASSETS.md](../../games/rac1/docs/ASSETS.md#collision)).

### RAC2

**Prerelease**: measured on the 9 September 2003 PAL review disc, re-checked
on the 6 August and 7 September discs
([LEVEL-ARCHIVE-FORMAT.md](../../games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md)).

- The disc carries 97 named files, unlike RAC1's three: `RC2.HDR` at LBA
  1001 and a `G/LEVELn.WAD` and `G/SCENEn.WAD` per level.
- `RC2.HDR` entry *i* is at +0x5000 + *i* × 0x3000, with the level WAD's LBA
  at +0x4 and the scene WAD's at +0x1804. 27 levels; room for 64.
- `LEVELn.WAD` opens with a 0x60-byte header: level id at +0x08, a reverb
  value at +0x0c, four (offset, size) couples in sectors at +0x10 (level data
  first) and six chunk couples at +0x30, used on 8 levels.
- The level data header has twelve 8-byte slots; the code overlay is slot 0,
  and slots 4–9 start with the `"WAD"` container.

A community note's layout does not hold on these discs; the doc lists each
difference. On the retail v1.01 disc the 0x60-byte level header is the same
(level id, reverb, the four ranges from +0x10, chunks at +0x30, used on 8
levels), measured on 2026-10-20 ([formats/LATER_GAMES.md](formats/LATER_GAMES.md#22-the-level-header)).

### RAC3

Levels are unpacked with Wrench; the repository documents no level format
([Toolchain-and-Build.md](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md)).

### RAC2, RAC3 and RAC4 retail, as the editor reads them

**Retail**, 2026-10-20: the editor reads every level of the three NTSC-U
discs ([formats/LATER_GAMES.md](formats/LATER_GAMES.md)). The table of
contents is at sector 1001 on all three; RAC2's 27 levels, RAC3's 51 and
RAC4's 47 are numbered by their level headers (RAC2: 0–20, 22–26, 30, as
above). The level core is RAC1's format with a few per-game values
([editor/games.py](../../editor/games.py)); large levels stream part of
their terrain and collision in chunks; RAC4 keeps its instances in the level
data and swizzles its textures.

## 5. Rendering

The RAC1 and RAC2 docs name the same four geometry kinds: terrain fragments
(tfrags), ties, shrubs and mobys. RAC3's docs do not cover rendering yet.

| Kind | RAC1 PAL, measured on retail data ([ASSETS.md](../../games/rac1/docs/ASSETS.md)) |
|---|---|
| Terrain (tfrag) | 0x40-byte fragments with LOD-2, shared, LOD-1 and refinement streams of VIF unpacks; `func_002352C8` confirms the streams and DMA sizes, not the VU arithmetic |
| Ties | Class header with per-LOD packet counts and scale; `func_00236A98` confirms the material table; 0xe0-byte placements |
| Shrubs | Class header, packets, 24 stored normals, optional billboard; 0x70-byte placements |
| Moby meshes | VIF UNPACK lists, a 512-entry vertex cache, up to three joints per vertex, index streams; normals decoded as ReRAC does (Wrench swaps x and y, which the faces contradict) |
| Sky | `func_00203118`; up to eight shells. Untextured shells hold colours, which Wrench reads as texture coordinates |
| Textures | 8-bit indices into 32-bit palettes listed in the GS RAM table, index bits 3 and 4 swapped; `func_00203958` uploads them |

- RAC1's executable objects carry names from the NTSC decomp's split of the
  US build, with boundaries checked on PAL: `tfragfunc`/`tfragproc`,
  `tiefunc`/`tieproc`, `shrubfunc`/`shrubproc`, `skyfunc`/`skyproc`,
  `partproc`/`partupd`, `shadowproc`, `draw`, `vuchain`, `mobyproc` and
  others ([config/text.objects](../../games/rac1/config/text.objects)).
- RAC2's reference notes name the four kinds from debug memory-map strings
  ([COMMUNITY-ENGINE-REFERENCE.md](../../games/rac2/ntsc/docs/COMMUNITY-ENGINE-REFERENCE.md)).
  In the retail RAC2 overlays, functions that carry diagnostic messages are
  labelled, among them `TfragTextureOverflow` (level 0) and the camera
  collision test's grid warning (25 overlays)
  ([ASSERT-MESSAGE-NAMES.md](../../games/rac2/ntsc/docs/ASSERT-MESSAGE-NAMES.md)).
  RAC1 PAL's retail strings name `MB_CheckCollPill` (used by `func_00214550`)
  and `Camera_CollPrimTest` ([config/strings.tsv](../../games/rac1/config/strings.tsv)).

How a frame reaches the hardware in RAC1 PAL (the display list as a VIF1 DMA
chain, the passes of the world renderer in order, which program each draw
path uploads, every function that touches a hardware register) is in
[port/RAC1_PAL_SURVEY.md](../port/RAC1_PAL_SURVEY.md),
sections 1 to 4, with a comparison across the sequels in its section 10.

### VU microprograms

- **RAC2, retail.** `.DVP.ovlytab` has 50 records `{name offset, EE address,
  VU address}` into `.vutext`; the section names
  `.DVP.overlay..<vu_offset>.<program_id>.<line>.<chunk>` group them into 13
  programs. `.vutext` and the table hash the same on three 2003 prerelease
  discs and on v1.01 ([VU-MICROPROGRAMS.md](../../games/rac2/ntsc/docs/VU-MICROPROGRAMS.md)).
- **RAC1 against RAC2.** The RAC1 boot compared has 43 chunks in 12 programs.
  All 12 ids recur in RAC2, which adds program 56883; 21 of 49 comparable
  chunks are byte-identical, including all 8 of program 55907. The `<line>`
  field is unchanged where code is unchanged: it is the source line.
- **Roles** (tfrag, tie, shrub, moby, sprite, particles, VU0 helpers) are
  ReRAC's attributions for RAC1, a **reference** RAC2 has not traced. The
  code that uploads the chunks is not located in RAC2.
- RAC1 PAL keeps `vutext` as a binary blob; RAC1 US rebuilds the DVP blobs as
  raw data ([progress-metrics.md](../../games/rac1/ntsc/docs/progress-metrics.md)).

### Hand-written and vector code

- **RAC1 PAL**: 212 functions (83,520 bytes) are classified hand-written
  ([ASM_CLASSIFICATION.md](../../games/rac1/docs/ASM_CLASSIFICATION.md)).
  The 51 large VU functions (67K) count as hand-written: no compiler stack
  frame, trapping `add`/`sub` ([SIBLING_DECOMPS.md](../../games/rac1/docs/SIBLING_DECOMPS.md)).
- **RAC1 US** leaves SIMD, VU0/MMI and COP2 helpers out of its C goal
  ([progress-metrics.md](../../games/rac1/ntsc/docs/progress-metrics.md)).
- **RAC3**: VU0 macro-mode code in 101 `frontbin` functions matches as inline
  assembly in C; a hand-written VU0 clipper spans 0x3CC880–0x3CD548.
  `frontbin` writes DMA and GS registers and the scratchpad directly
  ([full_match_roadmap.md](../../games/rac3/ntsc/docs/full_match_roadmap.md)).
- **Scratchpad.** RAC1 moby class hierarchies store parents as 0x70000000 +
  0x40 × parent (ReRAC, [ASSETS.md](../../games/rac1/docs/ASSETS.md#moby-skeletons-and-animations));
  Lombyte's data-flow note finds 0x40-byte scratch slots at 0x70000000 +
  0x40 × *i* in the transform path, without calling them joints.

## 6. Other subsystems

- **Sound.** RAC1 PAL: `989snd.c` is a core object built with the game
  compiler; its path string and RPC diagnostics are in the executable
  ([config/strings.tsv](../../games/rac1/config/strings.tsv)). Each level
  has a sound bank section and a `sndvtbl`. RAC2: `989snd` identifiers sit
  in the boot's data, and five functions Lombyte files under `audio/` are
  byte-identical in RAC2 ([THIRD-C-LOT.md](../../games/rac2/ntsc/docs/THIRD-C-LOT.md)).
- **Memory card and save data.** RAC1 PAL: `libmc.o` (0x1236F0) matches the
  SDK archive; the 25 state names `CS_INIT` … `CS_PROMPT_BEGIN_NOSAVE` are
  retail strings, and RAC1 US finds the 25-pointer handler table the
  dispatcher indexes ([overlays.md](../../games/rac1/ntsc/docs/overlays.md)).
  Save block ids are listed in
  [DATAMINED_REVERSE_ENGINEERING.md](../../games/rac1/docs/DATAMINED_REVERSE_ENGINEERING.md),
  attributed to `func_0020BBC8`. RAC2's `card.h` and `save.h` repeat them
  (**reference**; no C file includes them). RAC3's CRC `func_00399748`
  follows the algorithm of RAC1 US's memory-card CRC, without byte equality
  ([Cross-Repository-Resources.md](../../games/rac3/ntsc/docs/wiki/Cross-Repository-Resources.md)).
- **IOP.** RAC1 PAL: `func_00201E88` checks the region in `SYSTEM.CNF` and
  loads `IOPRP243.IMG`; the table of contents holds one compressed bundle of
  IOP modules ([ASSETS.md](../../games/rac1/docs/ASSETS.md#the-disc)). The
  EE side of the IOP stash uses structures attributed to a June 2002
  prototype (**prerelease**, [OPEN_QUESTIONS.md](../policy/OPEN_QUESTIONS.md)).
  RAC2: strings of IOP-side libraries (989snd, libcdvd, libdma, libpad2,
  libdbc/libmc, SIF) sit in the boot's data with no code reference
  ([ASSERT-MESSAGE-NAMES.md](../../games/rac2/ntsc/docs/ASSERT-MESSAGE-NAMES.md)).
  RAC3: the other executables carry `.irx` sections.
- **Input.** RAC1 PAL links `libpad2.o` (0x124B88) and a game `pad` object.
  RAC2 carries libpad2 identifiers; its reference notes say RAC3 and
  Deadlocked use `scePad` (**reference**).
- **Menus and HUD.** RAC1 PAL has `hud`, `menu`, `help`, `pause` and `vendor`
  objects; each level loads a HUD header and five HUD banks.
- **Video.** RAC1 PAL links `libmpeg` (0x12A2F0) and `movie/*` objects; its
  movie code is to be redone from assembly alone ([SOURCING.md](../policy/SOURCING.md)).
- **Game state.** RAC1 placements carry spawn flags and ids tested against
  the save state (section 3); PAL's level order matches each level's debug
  strings ([OVERLAYS.md](../../games/rac1/docs/OVERLAYS.md#levels)).
  RAC2's `flags.h` naming comes from prototype strings (**prerelease**).
- **Gadgets and weapons.** RAC1 PAL has a gadget table at core index +0x80
  and 21 gadget classes (**retail**). A 29-entry gadget enum sits in RAC1
  PAL's `structs.h` (no source named) and RAC2's `gadgets.h` (prototype
  strings). RAC2's weapon mod bits (`weapons.h`) are **reference**.

## 7. Sony and runtime libraries

Facts about the binaries only; SDK source, samples and headers are never a
reference ([SOURCING.md](../policy/SOURCING.md)).

| Key | What is identified |
|---|---|
| RAC1 PAL | 452 `core_text` functions match SDK archive members byte for byte: `libmc`, `libdbc`, `libpad2`, `libmpeg` (with `bit.o`), kernel and SIF calls. Also newlib (`mprec`, `dtoa`, `makebuf`, `strtol` follow its 2000-02-17 snapshot nearly line for line), libgcc built from GCC's source, `989snd`, `crt0` and libsn's `vu.o`. Version strings: libcdvd 2530, libdbc 2500, libkernl 2540, libpad2 2500. `core_text` is about 50 objects, split at the linker's `0xCDCDCDCD` fill; the linker dropped unreferenced functions, sometimes leaving their last word ([DECOMP_PROGRESS.md](../../games/rac1/docs/DECOMP_PROGRESS.md), [core_text.objects](../../games/rac1/config/core_text.objects)) |
| RAC1 US | `src/sdk/` and `src/runtime/newlib/`, with newlib's licence under `licenses/` |
| RAC2 | The IOP-library strings above; a `FlushCache` syscall stub (syscall 100) at 0x0011AEA0 (**retail**). SDK 2.5.5 and a libpad2 2500 string are **reference** claims ([COMMUNITY-ENGINE-REFERENCE.md](../../games/rac2/ntsc/docs/COMMUNITY-ENGINE-REFERENCE.md)); RAC1 PAL has the same libpad2 string |
| RAC3 | The launcher `i5bootn.elf`: libgcc's soft-float, 64-bit division and `__main`, rebuilt from GCC 2.95.x source with Sony's no-denormals change, and small libc and SIO helpers, all Sony 2.9-ee-991111 builds; newlib's `exit()` from 2.96-ee-001003-1 ([compiler_matrix_i5bootn.md](../../games/rac3/ntsc/docs/compiler_matrix_i5bootn.md)). 644 `boot_elf.elf` functions equal `frontbin` functions, "probably shared SDK/engine code" (**inferred**, [shared_code_findings.md](../../games/rac3/ntsc/docs/shared_code_findings.md)) |

## 8. Code shared between games and versions

### RAC1 PAL and RAC1 US

- PAL's [us_map.tsv](../../games/rac1/config/overlays/us_map.tsv) pairs
  US functions with PAL ones from the code alone: 99.3% of the boot's bytes
  and 99.7% of the levels' ([OVERLAYS.md, "US map"](../../games/rac1/docs/OVERLAYS.md#us-map)).
- PAL has ported 112 Lombyte functions (105 in two waves, 7 later), but not
  the 73 movie functions ([SIBLING_DECOMPS.md](../../games/rac1/docs/SIBLING_DECOMPS.md)).
  Lombyte credits rac1-decomp and the NTSC decomp as references. PAL's names
  come from the NTSC decomp, Lombyte and ReRAC through the US map
  ([NAMES.md](../../games/rac1/docs/NAMES.md)).
- Merging the two trees waits on the compiler question
  ([OPEN_QUESTIONS.md](../policy/OPEN_QUESTIONS.md), section 4).

### RAC1 and RAC2

- 24 reviewed C bodies in the Lombyte tree compile to bytes found in RAC2.
  RAC2 integrated 19 ([SECOND-C-LOT.md](../../games/rac2/ntsc/docs/SECOND-C-LOT.md),
  [THIRD-C-LOT.md](../../games/rac2/ntsc/docs/THIRD-C-LOT.md)), refused 2
  and deferred 3 that exist only in its levels: interpolation, a
  point-in-polygon test, sound and streaming, serial I/O and SDK helpers.
  Some are level-only code in RAC1 but sit in RAC2's boot program too.
- 21 of 49 comparable VU chunks are identical, and the dispatch tables and
  level code records share a layout (sections 2, 3, 5; differences in 9).

### RAC3 and the others

- Only algorithmic relationships are recorded: a CRC, linear interpolation
  (RAC2 `FUN_002AA140`, RAC3 `func_003BEB48`) and cosine interpolation (RAC1
  PAL `func_00214220`, RAC3 `func_003BE6A8`). Byte equality between the games
  has not been measured ([Cross-Repository-Resources.md](../../games/rac3/ntsc/docs/wiki/Cross-Repository-Resources.md)).
- RAC2's reference notes record a community claim that RAC3's multiplayer
  branched from RAC2's JP/KR/Greatest Hits code; nothing here tests it.

### Program to program within a game

| Key | Finding |
|---|---|
| RAC1 PAL | 428 level functions have a relative at 75% similarity or more, nearly all in another level; 137 variants differ only in a constant, float or struct offset ([OVERLAYS.md](../../games/rac1/docs/OVERLAYS.md)) |
| RAC2 | 13 of the first 17 boot bodies occur in all 27 levels; 82 boot bodies per level are placed ([LEVEL-INTEGRATION-PLAN.md](../../games/rac2/ntsc/docs/LEVEL-INTEGRATION-PLAN.md), [MOBY-DISPATCH-TABLES.md](../../games/rac2/ntsc/docs/MOBY-DISPATCH-TABLES.md)) |
| RAC3 | About 95% of overlay code is shared between overlays; 644 `boot_elf.elf` functions (about 30% of its code) equal `frontbin` functions; the 33 functions in `src/levels/common/` are mostly adapted `frontbin` C ([shared_code_findings.md](../../games/rac3/ntsc/docs/shared_code_findings.md), [common_level_c.md](../../games/rac3/ntsc/docs/common_level_c.md)) |

### Compilers differ, which limits reuse

Identical C gives identical bytes only under the same code generation.
What each project measured (each game's own docs have the details):

| Key | Compiler matching retail | Callee-saved registers |
|---|---|---|
| RAC1 PAL | SN GCC 2.95.3 (build v1.14) for game code and 989snd; Sony 2.9-ee-991111 for SDK code and libgcc ([TOOLCHAIN.md](../../games/rac1/docs/TOOLCHAIN.md)) | `text`: `sq`, 16-byte slots; `core_text`: `sd` |
| RAC1 US | GNU EE-GCC 2.9, assembled with Ps2EeAs; some units need a patched 2.9-ee-991111-01 ([patched-toolchain.md](../../games/rac1/ntsc/docs/patched-toolchain.md), [overlays.md](../../games/rac1/ntsc/docs/overlays.md)) | the patch adds `sq`/`lq` saves |
| RAC2 | GNU-EE 2.9-ee-991111b plus a patch stack, for the bodies integrated so far ([COMPILER-NOTES.md](../../games/rac2/ntsc/docs/COMPILER-NOTES.md)) | `sd`, 8-byte slots |
| RAC3 | SN ee-gcc 2.95.3 v1.36, `-O2 -G8 -fopt-stack -mno-check-zero-division`, per-file `-mno-split-addresses`, per-function Ps2EeAs ([Toolchain-and-Build.md](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md)) | `sd`, 8-byte slots |

The RAC1 bodies RAC2 reused make no calls; RAC2 notes that on leaf bodies
without saves, calls or `lq`/`sq`, GCC 2.9 and 2.95 emit the same code.

### Measured across all five versions

[SHARED_CODE.md](SHARED_CODE.md) reports the function map `tools/xmap.py`
builds from the discs (2026-10-04). In short: the engine core carries over
between games (RAC1 to RAC2 70% of the core, RAC3 to Deadlocked 69%), game and
level code mostly do not, 319 functions are in every version, and each project
has between 186 and 437 functions open that another project has already
matched in identical code.

### Not measured yet

- **Level functions that changed slightly between games.** The map pairs
  changed functions by similarity only in the boot executables and frontend.
- **RAC3's boot** is indexed by the map (Wrench's unpacked `boot_elf.elf`).
  The project split it on 2026-10-06 and seeded its front end from
  frontbin's C ([boot_elf.md](../../games/rac3/ntsc/docs/boot_elf.md)); the map has not been
  regenerated against those matches.
- **Deadlocked.** Its image and overlays are split and catalogued
  ([RESEARCH.md](../../games/rac4/ntsc/docs/RESEARCH.md), [OVERLAYS.md](../../games/rac4/ntsc/docs/OVERLAYS.md)): the
  same section names as RAC2's boot (`core.text`, `lvl.vtbl`, `lvl.camvtbl`,
  `lvl.sndvtbl`), assigned by position, plus `net.text`. It shares 567 KB of
  identical functions with RAC3 ([SHARED_CODE.md](SHARED_CODE.md)).

## 9. Gaps and contradictions

Stated as found; none is resolved here.

1. **RAC2 prototype date.** [COMMUNITY-ENGINE-REFERENCE.md](../../games/rac2/ntsc/docs/COMMUNITY-ENGINE-REFERENCE.md),
   `include/flags.h` and `include/gadgets.h` say "Aug 8 2002 prototype";
   [MOBY-DISPATCH-TABLES.md](../../games/rac2/ntsc/docs/MOBY-DISPATCH-TABLES.md)
   and [PROTOTYPE-BUILDS.md](../../games/rac2/ntsc/docs/PROTOTYPE-BUILDS.md)
   date the builds 2003, and [ENGINE-SYMBOL-NAMES.md](../../games/rac2/ntsc/docs/ENGINE-SYMBOL-NAMES.md)
   finds the "aug8" linker script matches the 6 August 2003 disc.
2. **RAC2 target edition.** The reference doc's version table calls v2.00
   (Greatest Hits) the primary target; the project targets v1.01, as the
   doc's own header warns.
3. **RAC1 addresses in RAC2 headers.** `include/engine.h` gives
   `MB_CheckCollPill` and `Camera_CollPrimTest` as 0x001E8890 and 0x001E7A50;
   `include/card.h` cites strings at 0x0015FE78 and 0x001E83F0. These are
   RAC1 PAL string addresses ([config/strings.tsv](../../games/rac1/config/strings.tsv)).
   In RAC2 v1.01 they fall in `core.bss` and frontend `.bss`
   ([ENGINE-SYMBOL-NAMES.md](../../games/rac2/ntsc/docs/ENGINE-SYMBOL-NAMES.md)).
4. **Shared state machine.** [SIBLING_DECOMPS.md](../../games/rac1/docs/SIBLING_DECOMPS.md)
   calls RAC2's memory-card state machine identical to RAC1's; on the RAC2
   side that rests on item 3, not on RAC2 code.
5. **Dispatch tables.** RAC1 ends all three at id -1 and reads the third
   `vtbl` word as a pointer to a 6-word table; RAC2 ends the moby table on
   padding and finds one shared value per level. RAC1 PAL names sound
   handlers `SoundFunc_<id>`, RAC2 `UpdateSound_<id>`.
6. **Lombyte's scope.** [engine-source-layout.md](../../games/rac1/ntsc/docs/engine-source-layout.md)
   puts level overlays out of scope until the executable is done;
   [overlays.md](../../games/rac1/ntsc/docs/overlays.md) and
   [progress-metrics.md](../../games/rac1/ntsc/docs/progress-metrics.md)
   count them in the goal.
7. **Which RAC1 boot.** [VU-MICROPROGRAMS.md](../../games/rac2/ntsc/docs/VU-MICROPROGRAMS.md)
   compares RAC2 with "the RAC1 boot we hold" without naming the region.
8. **RAC2's compiler.** Its [README](../../games/rac2/ntsc/README.md)
   describes the SN ProDG 3.01 `ee-gcc2953` profile and the reference doc
   names SN GCC 2.95.3; [COMPILER-NOTES.md](../../games/rac2/ntsc/docs/COMPILER-NOTES.md)
   moved to GNU-EE 2.9 because SN emits `sq` saves. RAC3 gets retail's `sd`
   saves from SN 2.95.3 with `-fopt-stack`; RAC2 does not record trying it.
9. **Float divide padding.** [SIBLING_DECOMPS.md](../../games/rac1/docs/SIBLING_DECOMPS.md)
   traces `nop; nop` before `div.s`/`sqrt.s` to Sony's 2.96 compiler and
   finds none in RAC1 PAL. RAC3 sees 0 to 3 `nop`s with no rule, supplied
   from a table ([Matching-Patterns.md](../../games/rac3/ntsc/docs/wiki/Matching-Patterns.md)),
   and its roadmap attributes one function's to inline assembly.
10. **Frontend programs.** RAC1 PAL's table of contents has a one-entry
    group Wrench names `frontbin`, not decoded
    ([ASSETS.md](../../games/rac1/docs/ASSETS.md#table-of-contents-groups)).
    Whether it, RAC2's FRONTEND module and RAC3's `frontbin.elf` are one
    mechanism is not measured.

## Adding to this page

Write a measurement in the game's own docs first, with the build it was
measured on. Then add a line here with the game key, the evidence level and
a link. A claim without a per-game source does not belong on this page
([LAYOUT.md](../LAYOUT.md)).
