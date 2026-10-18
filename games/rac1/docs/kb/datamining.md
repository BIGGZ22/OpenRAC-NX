# Datamined findings

What the retail data (PAL, SCES-50916) reveals about how the game was built.
Documentation only; nothing here proposes a code change.

## Top findings

- **Hidden 20-button code system** (verified code, untested in game): hold R2+L1 in the pause menu; 91 codes give items, planets, special items, skill points ([§1](#1-hidden-button-code-system)).
- **"I-5" debug menu** in every level, game mode -1; no retail code found that enters it (inferred) ([§2](#2-the-i-5-debug-menu-game-mode--1)).
- **`printf` is an empty stub** (verified); all diagnostics, incl. a per-level `*** MEMORY MAP ***`, are formatted and discarded ([§3](#3-debug-leftovers), [§7](#7-memory-budget-the-memory-map-report)).
- **Dev environment** (inferred from verified strings): project code `I5`, Maya-authored levels, header `walk.h`, spelling `Ratchett` ([§4](#4-development-environment)).
- **SDK 2.5.x libraries**, libpad2 + libdbc, 989snd ([§8](#8-libraries-and-versions)).
- **Region and save folder come from `SYSTEM.CNF`** at boot: `BESCES-50916RATCHET` (verified) ([§9](#9-file-system-region-and-language)).
- **Designer errors name ~25 moby class ids** (Footsoldier 28, MineBot 923/1401, ...) ([mobys.md](mobys.md#classes-named-by-retail-debug-strings)).
- **Corrections** to `DATAMINED_REVERSE_ENGINEERING.md` ([§12](#12-corrections-to-datamined_reverse_engineeringmd)).

Labels: **verified** = read from `baserom/` bytes or matched C; **inferred**
= reasonable reading, not proven; **untested in game** = static only.

Addresses: `func_L00_*` / `D_L00_*` are level-00; other levels' addresses
are in `config/overlays/functions.tsv`. `config/strings.tsv` addresses in
0x15F000-0x161FFF and 0x165580-0x1E8FFF are the ELF's copy; each level lays
them out differently (debug-menu strings 0x40 lower in every `lit.bin`;
memory map 0x1E8B50 in level 00 vs 0x1E8080 in the ELF), so the xref
columns there are unreliable.

Older notes (source files, `CardState`, save chunk ids, hub functions):
[`docs/DATAMINED_REVERSE_ENGINEERING.md`](../DATAMINED_REVERSE_ENGINEERING.md).

---

## 1. Hidden button-code system

`func_L00_00284410` (`src/overlays/shared/pause_00277208.c`, matched, 512
bytes, identical in all 19 levels), called each frame from the page-menu
update `func_L00_00277A88` (line 356). The page menu is case 3 of the
game-mode switch in `func_L00_002465F8` (`map_002465F8.c`): pause / map /
planet select. **verified**

### 1.1 Input

Reads `D_0013CA40` (`PadState`; bits: [data-structures.md](data-structures.md#button-bits)):

- `held_unmasked & 0xF == 6`: **R2 and L1 held, L2 and R1 not**
  (shoulder-bit names **inferred** from the standard layout).
- Each frame `pressed_unmasked & 0xF0A0` -> symbol: Up 0, Down 1, Left 2,
  Right 3, Square 4, else (Circle) 5. First in that order wins; Triangle
  and Cross ignored.
- Stick flicks count as D-pad presses; `_unmasked` undoes the mirror swap,
  so physical buttons always apply. **verified (code)**
- Symbols go into `D_L00_001BA838` (`short[20]`). The count
  `D_L00_001604B8` resets **only when the chord is released**; after 20
  presses input stops, so one attempt per chord hold. **verified**

### 1.2 Matching

On the 20th symbol, try `index = 2 .. 0x92`: match if
`buf[j] == T[((j + 1) * index) & 0xFF]` for all 20 `j`, `T` = 256-byte
table `D_L00_001B9F20` (values 0-5). First match wins; `code = index - 2`.
`T` is identical in all levels (0x1B9F20 in 00/07, 0x1BA320 in
01/10/12/16, ...). Codes 0x7D, 0x7F, 0x88, 0x8C duplicate lower codes and
never match; all four are in the no-effect range. **verified**

### 1.3 Effects

| Codes | Effect | Target (verified) | Meaning |
|---|---|---|---|
| 0x00-0x24 | `D_0013D5C8[c] = 1; D_0013D5F0[c] = 1` | 37-byte per-item arrays | item owned / item obtained (**verified**: `GiveItem` `func_L00_002618D8` in `mobyutil_00258BC8.c` sets the same two arrays; `func_0020C940` in `src/game/missionfunc.c` tests them as condition types 2 and 3) |
| 0x25-0x36 | `UnlockPlanet(c - 0x24)` (`func_L00_00261848`) | `D_0013DE48[1..18]` + `gGalaxyList` | planets 1-18, Novalis to Veldin (return) (**verified** call; planet order from `docs/OVERLAYS.md`) |
| 0x37-0x3C | `gSpecialItems[c - 0x37] = 1` (`D_0013D490`) | first 6 special-item flags | index 0 is the Zoomerator in the memory map that `config/names.tsv` cites (**inferred** for 1-5) |
| 0x3D-0x5A | if `gSkillPoints[c - 0x3D] == 0`: set it, `func_0022EE28(1,0,0)`, `ShowBanner(0x53DB, -1)` | 30 skill-point flags (`D_0013D510`) | the same three steps the game uses for every real skill-point award (e.g. `l08_batalia/vendor_002EAF48.c:1338`, `shared/help_00237B00.c:1469`) (**verified**) |
| 0x5B-0x90 | nothing | | |

- Item codes set the raw flags only: no banner, starting ammo
  (`D_0013D530`), quick-select entry (`D_00141FA0`) or equip, unlike
  `GiveItem`. **verified (code comparison)**
- Item names: † = pinned by names.tsv anchors (`gHaveHeliPack` 0x13D5CA =
  id 2, `gHaveMorphORay` 0x15, `gHaveMagneboots` 0x1C, `gHaveHologuise`
  0x1F, `gHaveMapOMatic` 0x21, `gHavePersuader` 0x23). Others follow the
  usual RaC1 order (**inferred**), supported by level-00 data:
  - `D_L00_00179BC0` item type (stride 0x4C): ids 2-4 back, 5-7 head,
    0x1C-0x1E feet, 8-0x20 hand.
  - Ammo table `D_L00_001C43B0`: nonzero for exactly the 11 ammo weapons
    (0xA, 0xB, 0xD, 0xF, 0x10, 0x11, 0x13, 0x14, 0x17, 0x18, 0x19).
  - Banner text ids `D_L00_001B06C0[id]` look like `level × 1000 + n`
    (3015 Heli-Pack → Kerwan, 11012 Thruster-Pack → Pokitaru, 12006
    Hydro-Pack → Hoven, 10011 Magneboots → Orxon, 5014 R.Y.N.O. → Rilgar),
    matching where they are found (**inferred**).
- Likely a leftover QA unlock system, unrelated to `gCheats`
  (`D_0015EEB0`) or the save's cheat blocks (**inferred**). The flags are
  saved: test on a throwaway save.

### 1.4 All 91 working codes

Hold **R2 + L1** (not L2/R1) in the pause menu, press the 20 buttons in
order. ↑↓←→ = D-pad (or left stick), □ = Square, ○ = Circle. Static
derivation, **untested in game**.

| Code | Index | Buttons (hold R2+L1, then press) | Effect |
|---|---|---|---|
| 0x00 | 2 | `□ ○ ← ← → → → □ ↓ □ ↑ ↑ ↓ ← → → ○ ↑ ○ ↑` | item 0x00 (id 0, no item type): owned + obtained flags |
| 0x01 | 3 | `↓ ← ↓ → ← ↓ ↓ ↑ ↑ → □ ↑ □ ↑ ○ ↓ ↑ □ ← ←` | item 0x01 (id 1, back-slot item, unidentified): owned + obtained flags |
| 0x02 | 4 | `○ ← → □ □ ↑ ← → ↑ ↑ ↑ ↓ ↓ → ← ↓ ○ ↑ → ←` | item 0x02 Heli-Pack †: owned + obtained flags |
| 0x03 | 5 | `□ → ← □ □ → □ ↑ ○ ○ ← ← ↑ ↑ ○ ← → ↑ → □` | item 0x03 Thruster-Pack: owned + obtained flags |
| 0x04 | 6 | `← → ↓ ↑ → ↑ ↑ ↓ □ ← → ↑ → ↑ ↑ ↓ ↑ ↓ ○ ↓` | item 0x04 Hydro-Pack: owned + obtained flags |
| 0x05 | 7 | `↓ → ↓ ← □ ↑ → → ↓ ↑ → ↑ ↓ ↑ → → ← □ □ ↓` | item 0x05 Sonic Summoner: owned + obtained flags |
| 0x06 | 8 | `← □ ↑ → ↑ ↓ → ↓ ↑ ← ← ↓ ← → ↓ → □ □ → ○` | item 0x06 O2 Mask: owned + obtained flags |
| 0x07 | 9 | `↓ ↓ ↑ ↑ ○ □ ↓ ↑ ↓ ↑ ← ↓ ○ □ ↓ □ → ○ ↑ ↓` | item 0x07 Pilot's Helmet: owned + obtained flags |
| 0x08 | 10 | `→ □ → ↑ ○ ← ↑ ← ↑ □ □ ↓ ↓ ↓ ↑ ○ ↑ ↓ □ →` | item 0x08 OmniWrench: owned + obtained flags |
| 0x09 | 11 | `← ↑ □ ↑ ← → → ← ← □ ↑ ↓ ← ○ □ ○ ↑ ← ← ○` | item 0x09 Suck Cannon: owned + obtained flags |
| 0x0a | 12 | `→ ↑ ↑ ↓ ← ↑ ↑ ↓ ↓ ↓ ↓ □ ↑ ← ↓ ← ← ↑ ↓ ←` | item 0x0a Bomb Glove: owned + obtained flags |
| 0x0b | 13 | `↓ ↓ □ ↓ ↑ → ↓ ← ○ ↓ ← ↑ ○ ○ ← ↓ ← → ↓ ○` | item 0x0b Devastator: owned + obtained flags |
| 0x0c | 14 | `→ ← ↑ → ↑ ↑ ↑ → □ ↓ ○ ← ○ ↓ □ → ↓ ← → ↑` | item 0x0c Swingshot: owned + obtained flags |
| 0x0d | 15 | `← → ○ ← ○ ↑ → ↓ ↓ ↑ □ ↓ ← □ ↑ ← → → ○ ↑` | item 0x0d Visibomb Gun: owned + obtained flags |
| 0x0e | 16 | `□ → ↓ ↓ ← ↓ → → □ ○ ○ ← ↓ → ← → □ → ↓ ↓` | item 0x0e Taunter: owned + obtained flags |
| 0x0f | 17 | `→ ○ ↑ ○ → ↑ ← □ → ↑ ↑ ← ← ↓ → □ □ ○ □ ↑` | item 0x0f Blaster: owned + obtained flags |
| 0x10 | 18 | `↓ ↑ □ ↑ ↑ ↓ □ □ ○ ↓ ← ↑ → ← → → ○ ○ ← ←` | item 0x10 Pyrocitor: owned + obtained flags |
| 0x11 | 19 | `→ ○ ← → → ○ □ → ↑ □ ← ↓ ↓ → ○ ↓ □ ← → □` | item 0x11 Mine Glove: owned + obtained flags |
| 0x12 | 20 | `□ ↑ ← ← □ ↓ ↓ ○ ↓ → ○ ← ○ ↑ ↑ ↓ ↑ ← □ □` | item 0x12 Walloper: owned + obtained flags |
| 0x13 | 21 | `↓ ↑ ↓ ↑ → □ ← ← ○ □ ○ ← → ○ ↑ ← ○ ← ← □` | item 0x13 Tesla Claw: owned + obtained flags |
| 0x14 | 22 | `↑ ↑ → ← □ ↓ ○ ○ ← ○ → ← → ↓ ○ ↓ ← ↓ ○ □` | item 0x14 Glove of Doom: owned + obtained flags |
| 0x15 | 23 | `↓ □ □ ↑ ○ → ↑ □ ← → ○ □ ↓ → ↓ → ↓ → → ←` | item 0x15 Morph-o-Ray †: owned + obtained flags |
| 0x16 | 24 | `↑ ↓ ↑ ↓ ↓ □ ← ← ↑ ← ← → → ← ← → → ○ → →` | item 0x16 Hydrodisplacer: owned + obtained flags |
| 0x17 | 25 | `□ ○ ○ □ □ ↑ □ → ↑ □ → ↑ □ ← ← □ ○ ↑ ← ○` | item 0x17 R.Y.N.O.: owned + obtained flags |
| 0x18 | 26 | `↓ ↓ → ← ↓ ↑ ○ ↓ → ○ → → ↑ ↓ ↑ ○ → ↑ ↓ ←` | item 0x18 Drone Device: owned + obtained flags |
| 0x19 | 27 | `↑ □ ↓ ↓ ↓ ○ ○ ↑ ← → ← ○ → ← □ ○ ↑ → ↓ ←` | item 0x19 Decoy Glove: owned + obtained flags |
| 0x1a | 28 | `← → ↑ → ↓ ← ↓ → ← ↑ ↓ ← ↓ □ □ ← ○ → □ ↓` | item 0x1a Trespasser: owned + obtained flags |
| 0x1b | 29 | `○ → → ○ ↓ ↓ ↑ → □ ○ ↓ ↑ ↑ ↑ □ ↓ → → □ ○` | item 0x1b Metal Detector: owned + obtained flags |
| 0x1c | 30 | `→ ← ↑ ↓ ↑ ↓ □ ← → ↑ ○ ← ↑ □ ↑ → ○ ← → ←` | item 0x1c Magneboots †: owned + obtained flags |
| 0x1d | 31 | `○ ↓ → □ ↑ → → → ↓ □ → ○ ← □ ← ← ← □ → ↓` | item 0x1d Grindboots: owned + obtained flags |
| 0x1e | 32 | `→ ↓ ↓ → ○ ← → → → ↓ ↓ → ○ ← → → → ↓ ↓ →` | item 0x1e Hoverboard: owned + obtained flags |
| 0x1f | 33 | `□ → ← ↓ □ ← ○ ← ← ○ ← ↓ → ○ → □ → ↑ ○ ↑` | item 0x1f Hologuise †: owned + obtained flags |
| 0x20 | 34 | `○ ○ ↑ □ ↑ ← ↓ □ ○ ↑ ← → → ○ ○ → → □ ↑ ←` | item 0x20 PDA: owned + obtained flags |
| 0x21 | 35 | `□ ↑ → ↓ □ □ □ ↑ ↑ ← → □ → → ↓ ↓ ↓ ← → ←` | item 0x21 Map-o-Matic †: owned + obtained flags |
| 0x22 | 36 | `↑ ↑ ↓ □ ↓ ↑ ← → ○ ← ↓ ○ ↑ → ← ↓ □ □ ↓ ↓` | item 0x22 Bolt Grabber: owned + obtained flags |
| 0x23 | 37 | `□ ○ ← ↑ → ↓ ↓ ↑ → ○ ↓ ← ↑ ← ↓ ← ○ ○ → ↓` | item 0x23 Persuader †: owned + obtained flags |
| 0x24 | 38 | `○ → ○ → □ ↓ → ↓ ← □ ○ → ↓ □ → ↓ ↑ ↓ □ →` | item 0x24 (id 0x24, unidentified): owned + obtained flags |
| 0x25 | 39 | `□ → ○ ↑ ← → → → → ↑ → ↑ ↑ ○ ← → ↓ □ □ →` | UnlockPlanet(1) — Novalis |
| 0x26 | 40 | `↑ ← ↓ ○ → ← ↑ ↓ ← □ □ → ← ↓ ← → ← ↓ → →` | UnlockPlanet(2) — Aridia |
| 0x27 | 41 | `← ↑ → □ ↓ □ ○ ↑ ↑ ○ ← ← ↓ ↓ □ □ → → ← ↓` | UnlockPlanet(3) — Kerwan |
| 0x28 | 42 | `↑ ↑ □ ← □ ← ○ ← ← □ ○ → ○ → ← ○ ○ ○ → ↑` | UnlockPlanet(4) — Eudora |
| 0x29 | 43 | `↓ ← → ↓ ← □ ○ ← ↓ ↓ → ○ → ↑ □ ○ ← ← → ↑` | UnlockPlanet(5) — Rilgar |
| 0x2a | 44 | `↑ ← ↓ ○ ○ ← ↓ ↓ ↓ □ ↓ □ ← ← ↑ ← ← ↑ ○ →` | UnlockPlanet(6) — Blarg Station |
| 0x2b | 45 | `○ ↑ ↓ ↓ ↑ → ↑ ← □ ↑ → ← ← ← □ ↓ ○ ↑ → ↓` | UnlockPlanet(7) — Umbris |
| 0x2c | 46 | `□ ↑ → □ → □ → → → ← □ ↑ ← ↓ □ → → ← ↑ →` | UnlockPlanet(8) — Batalia |
| 0x2d | 47 | `→ ← □ ← □ ↓ ← ↓ ○ → □ ↓ ← ↑ ↑ ← ○ → □ ↓` | UnlockPlanet(9) — Gaspar |
| 0x2e | 48 | `↓ ↓ □ ← ← → ← → ○ → □ ↓ → ○ ↓ → ↓ ↓ □ ←` | UnlockPlanet(10) — Orxon |
| 0x2f | 49 | `→ ↑ ← ↓ □ ○ → □ → → ↑ → □ ↓ ↓ □ ↑ ○ □ ↑` | UnlockPlanet(11) — Pokitaru |
| 0x30 | 50 | `○ □ ↑ → □ ↑ ← □ ↑ ○ ○ ← → ← ↓ → ↑ ↓ ○ →` | UnlockPlanet(12) — Hoven |
| 0x31 | 51 | `↑ ↑ → ← → ○ ○ → ↑ ○ → □ ↓ ○ ○ ↓ ← ↑ ↑ ←` | UnlockPlanet(13) — Gemlik Base |
| 0x32 | 52 | `↓ ← ↑ ↓ ○ → ↓ ○ ↑ ← ← → □ ↑ → ↓ ○ ← ○ □` | UnlockPlanet(14) — Oltanis |
| 0x33 | 53 | `○ ↑ ↓ ↑ ↓ ↓ ○ ← ← ↓ ↓ □ ↑ → ↑ ← □ → → ↑` | UnlockPlanet(15) — Quartu |
| 0x34 | 54 | `□ ↓ ○ ↑ → ○ ← ○ → ← ↑ □ □ ○ ↑ ↓ ↑ ← □ →` | UnlockPlanet(16) — Kalebo III |
| 0x35 | 55 | `← □ □ ○ → ○ → □ → ○ → ↑ ↑ □ ← → ○ ↓ ↓ →` | UnlockPlanet(17) — Drek's Fleet |
| 0x36 | 56 | `→ → ← → ↑ ← □ ← → ↓ ← ○ ↑ □ ↑ → □ ← ↑ ↓` | UnlockPlanet(18) — Veldin (return) |
| 0x37 | 57 | `← ○ ↑ ↓ ○ ← ← → ↓ → ○ ↓ □ → → □ ↑ □ ↑ ○` | special-item flag 0 (Zoomerator †) |
| 0x38 | 58 | `→ ○ ↓ → ○ ↑ ↑ ↓ → ○ □ □ → ↑ ↑ ○ ↓ □ → □` | special-item flag 1 |
| 0x39 | 59 | `↑ ← ↑ ← □ ↑ ○ ↑ → → ○ ↓ → → ○ ○ □ ○ ↓ ↑` | special-item flag 2 |
| 0x3a | 60 | `← ↓ ↓ ← ↑ ← □ → ← ← ↑ ↓ → ↑ ↓ ← ← → ○ ○` | special-item flag 3 |
| 0x3b | 61 | `→ ← ↑ ○ → □ ↑ → □ ↑ ↓ ○ □ ← ← ↓ ↓ ○ ↓ ↓` | special-item flag 4 |
| 0x3c | 62 | `↓ □ → → □ ○ □ ← □ ↓ ↑ → ○ □ ○ → → ↑ ○ ↑` | special-item flag 5 |
| 0x3d | 63 | `↓ □ ○ ← ↑ ← → → ← ← → ○ ↑ ○ ↑ ← → □ → ←` | skill point 0 (Take Aim †) |
| 0x3e | 64 | `↓ → ← → ↓ → ← → ↓ → ← → ↓ → ← → ↓ → ← →` | skill point 1 |
| 0x3f | 65 | `↑ ↓ ← ○ □ ↑ → ← ← → ↑ → → □ ← □ ↓ ↑ → □` | skill point 2 |
| 0x40 | 66 | `→ ↓ ← ← ○ ↓ ○ □ ↑ ↑ → ↑ ↑ ↑ ↓ → ↑ □ → ↑` | skill point 3 |
| 0x41 | 67 | `□ ↑ ↑ → ↓ ↑ ○ ↑ ↓ → ↑ ↑ □ ↑ → ↓ ○ ○ □ ←` | skill point 4 |
| 0x42 | 68 | `○ □ ← □ ↑ → ○ → □ ← ← ↓ ○ □ ← ↓ ↓ → → ←` | skill point 5 |
| 0x43 | 69 | `□ → ← □ ↓ → ↓ ↑ ○ □ ↓ ← → ← ← ← □ ↓ ○ □` | skill point 6 |
| 0x44 | 70 | `↑ ↓ □ ↑ ← □ → ↓ ← ← □ ↑ □ ↑ ↓ ↓ □ ← ○ ↓` | skill point 7 |
| 0x45 | 71 | `↓ □ ○ ← ← ↑ → → → ← ↓ ↑ ↑ → ← → ↑ ○ □ ↓` | skill point 8 |
| 0x46 | 72 | `↑ □ ↑ → ← ○ → ↓ □ ↓ ↑ ↓ ← ← → → → □ ← ○` | skill point 9 |
| 0x47 | 73 | `← ↑ ← ↑ ○ ○ → ↑ ↓ ↓ □ ↓ → ○ ↓ □ → ○ ← ↓` | skill point 10 |
| 0x48 | 74 | `○ ↑ ↓ ↑ ○ ← ← ← ○ ↓ □ ↓ ↑ → ← ○ → ↓ □ →` | skill point 11 |
| 0x49 | 75 | `○ ↑ ↑ ↑ ← ↑ ↓ ← □ ↓ ← ↓ ← ↓ ○ ○ ↑ ↑ ↓ ○` | skill point 12 |
| 0x4a | 76 | `→ → ↓ ↓ □ → □ ↓ ↓ → ○ □ ○ ↑ ○ ← → ← □ ←` | skill point 13 |
| 0x4b | 77 | `→ ○ ○ ↓ → ○ ↑ ← → □ ↓ ↑ □ □ ↓ ↓ ○ ↑ ↑ ○` | skill point 14 |
| 0x4c | 78 | `→ ↑ → → ↑ ↑ ○ → □ → ↑ ← □ ○ ↑ → □ □ ○ ↑` | skill point 15 |
| 0x4d | 79 | `↓ → → ← ↑ ↓ ← ↓ → ↑ ○ ↓ ↓ ↑ ↑ ← ↓ □ ← ↑` | skill point 16 |
| 0x4e | 80 | `← ○ ← ↓ □ → ↓ → ↓ → → ← □ ↓ ○ → ← ○ ← ↓` | skill point 17 |
| 0x4f | 81 | `↓ ○ ← ○ □ → ← □ → ↑ → ← ○ □ → □ ↓ □ ↓ ↑` | skill point 18 |
| 0x50 | 82 | `↑ □ □ ↑ ○ ← ↓ □ → ↓ ↑ ↑ ↑ □ ○ → ○ ↓ ↑ ←` | skill point 19 |
| 0x51 | 83 | `↓ □ □ → ↓ → □ → □ ↓ ↓ ↓ ← → ← ↓ ↓ → ← □` | skill point 20 |
| 0x52 | 84 | `↑ ← ← ← □ → → ○ ○ ↑ ↑ ← ○ → ← ↓ ↑ → ← □` | skill point 21 |
| 0x53 | 85 | `→ ↑ → ↑ ○ ○ ↓ ← ○ ↑ ○ ← ↓ □ ↑ ← □ □ ↓ □` | skill point 22 |
| 0x54 | 86 | `← ↓ □ ← ↓ ○ ↑ ○ ← ↑ □ ← ← ↓ → ↓ ○ → ↑ □` | skill point 23 |
| 0x55 | 87 | `→ ↓ □ ↑ □ → ↓ □ ← ↑ ○ □ ← ↑ □ → → → ○ ←` | skill point 24 |
| 0x56 | 88 | `← ○ ← ↓ □ □ ← ← ↑ → → → ↓ ↓ ↑ → ↑ ↓ □ →` | skill point 25 |
| 0x57 | 89 | `↓ □ ← □ ○ ↑ ← → □ ← → ↑ □ ↓ ← □ □ → ↑ ○` | skill point 26 |
| 0x58 | 90 | `↑ ↓ → ← ↑ ← ← ↓ ↑ ↓ ↓ → ↑ ← ↑ ○ □ ↑ ↓ ←` | skill point 27 |
| 0x59 | 91 | `↓ ○ → ↓ → ○ □ ↑ ↑ □ □ ○ ↓ □ → ○ ← ↑ ↑ ←` | skill point 28 |
| 0x5a | 92 | `↑ □ □ → ← ↑ ↓ → ← → ○ ← ↓ ← □ ← ← ↓ ↑ ↓` | skill point 29 |

Reproduce it with this script (`python3 -I`, from the repo root):

```python
import json
m = json.load(open('baserom/overlays/level_00/manifest.json'))
d = next(r for r in m['records'] if r['name'] == 'data')
T = open('baserom/overlays/level_00/data.bin', 'rb').read()[0x1B9F20 - d['address']:][:256]
for index in range(2, 0x93):
    print(index - 2, ''.join('UDLRSC'[T[((j + 1) * index) & 0xFF]] for j in range(20)))
```

---

## 2. The "I-5" debug menu (game mode -1)

**verified:**

- `func_L00_002465F8` switches on `D_L00_0015F6A8` (`gGameMode`);
  `case -1` calls `func_00216270`, `func_L00_001F4918` (4932 bytes) and
  `func_L00_001F5C60` (4884 bytes), present in all 19 levels (candidate
  names `DebugCameraUpdate` / `DebugCameraRender`).
- `func_L00_001F5C60` (matched, `src/overlays/shared/draw_001F3A78.c:523`)
  draws the menu: 71 lui/addiu refs into level `lit` strings
  0x15F108-0x15F3B8 (+3 from `func_L00_001F4918`). The ELF never
  references them (hence 0 xrefs in `strings.tsv`). Title at (0x100,
  0x10): `I-5`.
- State `debug_menu60` = `D_L00_0016C158`, initialised on every level start
  (`map_002465F8.c`: 0x8 = 15, 0x38 = -1, ...).

Pages (draw code + level-00 value tables):

| Page | Rows (value strings) |
|---|---|
| UPDATE | Ratchett, Mobys, Parts, Camera, Step (`off`/`on`, bits of `draw_bits`) |
| MODE | Control (`normal`/`camera`/`cam+chr`/`volume`), Profile (`off`/`bars`/`render`/`mobys`/...), Occl (`off`/`freeze`/`active`), Bookmark, Invinc (`off`/`on`), Drawdist (`normal`/`near`), Coll (`hotspt`/`camera`/`sound`), Tv Mode (`ntsc`/`pal`), Screen (`normal`/`small`/...), Scene (`%d`/`none`), Segment (`%d = %d`) |
| DRAW | Darken, Env Coll, Moby Coll, Hero Coll, Bspheres, Cameras, Cuboids, Samples, Text |
| AMMO | SuckCannon, Morpha-Ray, Inflato R, R.Y.N.O., D. Droids, Bomb, Devastator, Visibomb, Agents, Pyrocitor, Tesla Claw, Mine Glove, Blaster, Bolts |
| FOG | NearDst, NearInt, FarDst, FarInt, Color_R/G/B (`%.02f`) |
| MISC | BattleCam, Actuator, AddSample, AddStart |

Value tables: `D_L00_00160CD8` (off/on), `D_L00_00160D00` (control modes),
`D_L00_00160D68`, `D_L00_00160D80`, `D_L00_00160DA0`, `D_L00_00160DB0`,
`D_L00_00160DC8`, `D_L00_001C4128` (profiler modes).

- AMMO uses development weapon names (**verified** strings): `Morpha-Ray`
  (Morph-o-Ray), `Inflato R`, `D. Droids` (Drone Device, **inferred**),
  `Agents` (Decoy Glove?, **inferred**).
- `AddSample` / `AddStart` and `occ_samp.dat` / `occ_sample_deltas.dat`
  (refs from `func_L00_001F4918` in every level): an in-game occlusion
  sample recorder (**inferred**); `TJB - No env sample point found!`
  (`func_L00_002510F0`) is the run-time lookup (**inferred**).
- Unreferenced pointer table at level-00 data `0x16C140`: `    sfx`,
  `  music`, ` dialog`, `ambient`, `  shell`, padded to 7 chars like a
  mixer header (**inferred**).
- **Reachability (inferred):** stores to `0x15F6A8` across all level texts
  use constants 0, 1, 3-7 or a restored saved mode (`func_001FD3E8`'s copy
  at L00 0x202E48, `func_L00_00299B68`); none store -1, nor does the ELF.
  names.tsv cites TCRF for "0xFFFFFFFF: Debug Menu", consistent with
  memory poking. The code is live, but no button path was found.
  **untested in game**

---

## 3. Debug leftovers

### 3.1 `printf` is a stub

`func_001E9730` (`STUB_printf`, `src/game/stub.c`, matched) is
`void f(int, ...) {}`: 22 callers in the ELF, plus every level (memory map,
`Transition to level %d`, `loaded irx modules`, `Unsupported language %d`,
...). An emulator breakpoint there would show all diagnostics
(**inferred**, untested).

### 3.2 Diagnostics and where they are used

"19/19" = lui/addiu-referenced in every level's copy.

| String (verbatim) | Referenced from | Status |
|---|---|---|
| `\n*** MEMORY MAP ***\n\n` + 22 lines | `func_L00_00244AE0` (loaders), 19/19 | live, printed into the stub |
| `TJB - No env sample point found!\n` | `func_L00_002510F0`, 19/19 | live |
| `Camera_CollPrimTest WARNING! - grid out of bounds! (RAR)\n` | 19/19 | live |
| `!!!! Too many directional lights !!!! - Lighting will look bogus as a penalty.\n` | 19/19 + ELF | live |
| `Tried to create moby and couldn't. Time: %d, oClass: %d\n` | `func_0020D348` copy, 19/19 | live |
| `vu chain overflow - mobys dropped\n` | `func_0020E2B0` copy, 19/19 | live |
| `Why doesn't this work?` | `func_00235218` copy, 19/19 + ELF | live |
| `Paradox: This message does not exist` | `func_001FE540` (33 callers), 19/19 + ELF | live: fallback text for a missing message id (**inferred**) |
| `Transition to level %d\n` | `func_L00_00277A88`, 19/19 | live |
| `main loop squirm! should never get here!` | 19/19 | live |
| `camera update`, `sound update`, `system misc` | 19/19 | profiler bar labels (**inferred**) |
| `sizeof(w_2legData) = %d bytes` | level 04 (Eudora) only | live in one level |
| `assertion "%s" failed: file "%s", line %d\n` | none, ELF 0x1E7A90 | dead (library `__assert` format, **inferred**) |
| `cdrom0:\CODE\I5\PARAM.TXT;1` | none | dead |
| `host0:z:/i5/levels/level%d/npcs/scene_%d/scene.dat`, `scenep.dat` | none in 18 levels; level 07 shows numeric matches that look coincidental (not confirmed) | dead (**inferred**) |
| `*** frame %d ***\n`, `tried to draw path -1 \n` | none | dead |

### 3.3 File/line literals

No assert `__FILE__`/`__LINE__` pair survives. Only `hud.cpp`,
`loaders.cpp` and `map.cpp` are still used (every level; five `map.cpp`
refs in level 14), passed with line numbers to the heap allocator
(`DATAMINED_REVERSE_ENGINEERING.md` §1). `/usr/local/989snd/ee/989snd.c` is
the sound library. **verified**

---

## 4. Development environment

- **Project code "I5"** (**inferred**): menu title `I-5`,
  `cdrom0:\CODE\I5\PARAM.TXT;1`, `host0:z:/i5/levels/level%d/...` (`host0:`
  = devkit host PC, `z:` a mapped drive). Dead in retail (§3.2).
- **Maya** (**inferred**): `Moby class %d maya instance %d needs a
  rocketPath!` (01), `Moby Maya Index %d deleted.` (10): instances were
  identified by Maya scene index.
- **Original names** (**verified** strings): `walk.h`, `walk_init2LegAnim`
  (`Invalid anim detected on pLD->anims[%d]->anim in
  walk_init2LegAnim.\nSee walk.h and check your initialization function for
  moby class %d`, all levels), `w_2legData`, `SUCKSTATE_DEATH`,
  `M774_ShipChallengeDone`, `MB_Sucking`, `Camera_CollPrimTest`.
- **"Ratchett"** is the in-house spelling: debug row `Ratchett`, memory map
  `ratchett seqs:`. **verified**
- Some strings carry tags (`- RAR`, `TJB -`); no claim is made about who
  wrote them.
- **VU microcode as DVP overlays** (**verified**): 43
  `.DVP.overlay..<vu addr>.<id>.<line>.<n>` sections (12 groups of 1-8
  chunks, up to 0x800 bytes). `.DVP.ovlytab` maps each to its EE address
  in `.vutext` (0x100080, 0x12290 bytes) and VU address: the `dvp-as` VU1
  overlay mechanism. `<id>`/`<line>` look like source hash and line
  (**inferred**).
- **ELF layout** (**verified**, `readelf -S`): resident `core.text`,
  `core.data`, `core.rdata`, `core.bss`, `core.lit`; replaceable main
  segment `.lit`, `.bss`, `.data`, `lvl.vtbl`, `lvl.camvtbl`,
  `lvl.sndvtbl`, `.text`. The ELF's `.text` is 0x556B0 bytes and
  `lvl.vtbl` 0xC, vs ~1.1 MB text and ~0x800 vtbl per level: a reduced boot
  program (**inferred**). No `.mdebug` or other debug section.

---

## 5. Per-level strings and NPC names

In one or a few levels, not the ELF. Stray leading bytes (`?`, `@`, `=`,
`A`) and float noise dropped. Presence means the code is linked in, not
that the NPC appears. **verified** strings; level names from
`docs/OVERLAYS.md`. Class-id errors: [mobys.md](mobys.md#classes-named-by-retail-debug-strings).

| Level | Names and notable strings |
|---|---|
| 00 Veldin | none of its own. The overlay has only 11 level-unique functions (8 KB), against 42-136 for the other levels |
| 01 Novalis | `Big Turret Guy`, `Water Pump Worker`, `Mystery 774`, `?mouse already completed`, `WARNING! - Moby 774 wasn't expecting a M774_ShipChallengeDone call. ...` |
| 02 Aridia | `Surfer Agent`, `McGuffin Surfer`, `Shark with no path. UID: %d`, `Ship with no path. UID: %d` |
| 03 Kerwan | `Helga`, `train end path has no STOP NODE!!!\n  Bailing out of the end cutscene before the train goes out of control and kills us all!`, `train has no master!`, `train engine has no cars!` |
| 04 Eudora | `Moby 1120 %d has no segment switch!`, `moby 427, instance %d requires both path fields to run!!! DELETING MOBY FROM LEVEL!!`, `sizeof(w_2legData)` (only referenced here) |
| 05 Rilgar | `Bouncer`, `Race Girl`, `Shifty Salesman` |
| 06 Blarg | `Scrawny Scientist` |
| 07 Umbris | `ERROR! Moby 1142 Equilizer Ammo Spawner has no trigger cuboid!`, moby 1059 and 871 errors, `hey, shouldn't be in init!` |
| 08 Batalia | `Big Turret Guy`, `Commando`, `Deserter`, `Water Pump Worker`, `Water Worker`, `Mystery 774`, `Paratrooper with an invalid jumpPath. UID = %d`, `M671 Warning: texture w count (%f) is greater than max texture tile count (8)` |
| 09 Gaspar | `Bigship moby %d (oClass:%d) can't find parent! dist:%f`, `Floating ship piece UID:%d oclass:%d missing trigger moby! Will just float there.`, `Kamikaze with an invalid boundPath.` |
| 10 Orxon | `Moby947 maya instance %d HAS NO PATH!!!\n\n\nMoron.\nMobyDeleted`, `moby class 1067 REQUIRES a link to dataProcessor class 1073!!`, `Moby Class %d Instance %d has no pVars!  May be in the art file.` |
| 11 Pokitaru | `Get Pilot Hat!`, `Raritanium Acquired`, `I'm really sorry, but you can't have more than 128 water tiles in this level` |
| 12 Hoven | `Raritanium Acquired`, the 128 water tiles message |
| 13 Gemlik Base | `tractor off screen, abort`, `tractor too long, abort`, `25s min adj`, `moby:%d(%d) opened at %d` / `closed` / `failed` |
| 14 Oltanis | `Captain Qwark`, `Scrap Merchant`, Footsoldier, hoverboat, wind-control, Enemy Fighter, Flying Car, MineBot and Hovertank errors |
| 15 Quartu | Footsoldier errors, `Get Pilot Hat!` |
| 16 Kalebo III | `MineBot (moby1401) ...`, `Kamikaze with an invalid boundPath.` |
| 17 Drek's Fleet | `magnacut cuboid error: detect:%d hero:%d init:%d end:%d`, `no final place for hero during victory, so just fall` |
| 18 Veldin (return) | none of note |

In every level: `Already got pilot helmet... deleting.`,
`?SUCKSTATE_DEATH: should not be called any more!`,
`Hey, M186 init should be earlier!`,
`Moby 1290 %d needs a cut scene!`, `MB_Sucking: Tried to suck a NULL pointer!`,
`cleaned up moby:%d (oclass:%d)`, `jumpdist = %f`,
`mpeg memory alloc error (%d bytes)`.

---

## 6. Moby class ids named by strings

Moved to [mobys.md](mobys.md#classes-named-by-retail-debug-strings)
(verbatim strings, levels, update functions). Extra notes:

- `docs/PVARS.md` names 774 the Plumber (Novalis).
- 794 reads as needing an effect moby (10).
- Kinds named without class ids: `Hovertank`, `Turret`, `Paratrooper`,
  `Kamikaze`, `Shark`, `Ship`, `Bigship`, `train`.

---

## 7. Memory budget (the MEMORY MAP report)

`func_L00_00244AE0` (loaders, all levels, still `INCLUDE_ASM`): called with
`(1, 0)` after each level load and `(0, full)` on quit. Nonzero first
argument prints, into the stub (**verified**):

```
*** MEMORY MAP ***
code mem / dead space / vu chain bufs / gadget buffer / tfrag geom (or "unknown")
occlusion / collision / shared vram / particle vram / effects vram
mobys / ties / shrubs / ratchett seqs / tie insts / shrub insts / moby insts
moby pvars / paths / part insts
end of mem: %.1fM
estimated mem: %.1fM
```

All but the last two lines are `%.1fK`. The order is likely the post-load
memory layout (**inferred**). "ratchett seqs" = hero animation sequences;
tfrag = terrain fragment. Hooking the stub would print real per-level
numbers (**untested**).

---

## 8. Libraries and versions

**verified** (version tags in the ELF):

| Tag | Library |
|---|---|
| `PsIIlibkernl2540`, `PsIIlibmc   2540` | kernel, memory card (2.5.4) |
| `PsIIlibcdvd 2530` | CD/DVD (2.5.3) |
| `PsIIlibdma`, `libgraph`, `libipu`, `libmpeg`, `libpad2`, `libdbc`, `libscf`, `libvib` (`2500`) | 2.5.0 libraries |
| `/usr/local/989snd/ee/989snd.c` | 989 Studios sound library (EE side, RPC to the IOP) |
| `cdrom0:\IOPRP243.IMG;1` | IOP replacement image loaded by `InitOnce` (IOP 2.4.3, **inferred** from the name) |

Pad stack: libpad2 + libdbc (`scePad2CreateSocket`, `pad.h`) + libvib, not
old libpad (**inferred**).

---

## 9. File system, region and language

- **Boot reads `SYSTEM.CNF`** (**verified**): `InitOnce` (`func_00201E88`,
  `src/game/initonce.c`) reads LBA 0x121:
  `BOOT2 = cdrom0:\SCES_509.16;1\r\nVER = 2.00\r\nVMODE = PAL`.
  PAL flag `D_0015EE80 = D_0015EF90 = (buf[0x33] != 'N')`. `func_00209A60`
  (`src/game/memcard.c`) fills `/BA****-*****RATCHET` (byte 0x12 `E`
  replaces `A`, `P` would give `I`; product letters/digits copied): save
  folder `BESCES-50916RATCHET` with `icon.sys`, `static.ico`, `save%d.bin`.
- **IRX order** (**verified**, matched): `LoadIRXModule` on entries 0x1C,
  0x1E, 0x20, 0x22, 0x24, **0x26 twice**, 0x2A, 0x2C, then 0x28.
- **Language** (**verified** code, **inferred** library identity):
  `func_0012D380` reads the OSD config word (bits 16-20, or bit 4 when bits
  13-15 are zero), like libscf `sceScfGetLanguage`. `InitOnce` maps it to
  `D_0015EE88`: English 1 → 0, French 2 → 2, Spanish 3 → 4, German 4 → 3,
  Italian 5 → 5; anything else prints `Unsupported language %d\n` and uses
  English 0. Six-slot per-language tables are indexed `language - 1`
  clamped to 0 (`src/game/space.c:1842`).
- **Movies** (**verified** names, referenced from every level's data):
  `\DEMOLOGO.NTS`, `\INFOBOT2.PSS`..`\INFOBOT4.PSS`, `\REPORTER.PSS;1`,
  `\INFOBOT7.PSS;1`, `\IBOT5_F/G/S/I.PSS;1` (languages **inferred** from the
  letters). See `docs/MOVIE.md`.
- **Level data**: `cdrom0:\DATA\LEVELS\LEVEL` + number (3 refs per level).

---

## 10. Unused code

A rough scan (scratch script, not in the repo) found **319 of 4180**
functions (33 KB; 241 shared, 51 level-only, 27 exe-kind) with no `jal`,
lui/addiu or lui/ori pair (400-instruction window), or 32-bit word pointing
at them in any level's lit/data/vtbl/text or the resident ELF code. An
**upper bound, not a dead-code list**:

- False positives: the level entry `func_L00_002465F8` (via `manifest.json`
  `entry_point`) and `InitOnce`.
- Moby updates (e.g. Batalia turret bolt `func_L08_002E4BE8`, 5448 bytes)
  are probably bound through class tables loaded from disc (**inferred**).
- Odd-aligned entries (e.g. `func_L00_001F080C`) are catalogue boundary
  fragments (see `nonmatching/shared/func_L00_001F8750.c`).

Verified dead **data**: §3.2.

Functions per level (all / level-unique / unique bytes): smallest 00
2336/11/8 KB; largest 14 Oltanis 2556/136/151 KB, 18 Veldin (return)
2489/127/156 KB, 08 Batalia 2480/86/156 KB.

---

## 11. Open questions

- What set `gGameMode` to -1 in development builds, and can the saved-mode
  field still do it?
- Names of item ids 1 and 0x24, special items 1-5, skill points 1-29
  (needs the help-text archive, not in the overlays).
- Does the code system work in game as derived (pad 0 only, while `unk18C`
  is zero)?

---

## 12. Corrections to `DATAMINED_REVERSE_ENGINEERING.md`

- **`RaC1GadgetId`**: its 29 values don't match retail ids (`D_0013D5C8` /
  `GiveItem`: 0-0x24, Heli-Pack 2, Morph-o-Ray 0x15, Magneboots 0x1C,
  Hologuise 0x1F, Persuader 0x23; §1.3). The enum in `include/structs.h` is
  unused.
- **"Recovered from unstripped debug types"**: the ELF has no debug
  sections (§4), so that `MobyInstance` is reconstructed. Use
  `include/moby.h`.
- **`camera.c` (0x001E7A50)**: no such string; that row is the
  `Camera_CollPrimTest` warning. `hud.cpp`, `loaders.cpp`, `map.cpp` do
  exist.
- Its string addresses are ELF-image addresses (see the top of this page).
