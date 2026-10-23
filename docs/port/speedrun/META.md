# The Ratchet & Clank speedrun meta

What a runner needs to know about Ratchet & Clank's own speedrunning, to
route the port's version of it.

Everything in the leaderboard tables was read from speedrun.com's public API
and can be re-checked at the links given. Facts about the game's own
mechanics are marked with their source.

## 1. The game on speedrun.com

**Ratchet & Clank** (2002), abbreviation `rac1`, game id `y655j36e`.
<https://www.speedrun.com/rac1>

The ruleset, from the API:

```json
{
  "show-milliseconds": true,
  "require-verification": true,
  "require-video": true,
  "run-times": ["realtime"],
  "default-time": "realtime",
  "emulators-allowed": true
}
```

- **Real Time only.** There is no in-game-time column and no load-removed
  column on the `rac1` game. (Ratchet & Clank 2, 3 and 4 all have
  `realtime_noloads`; `rac1` never got one.)
- **Milliseconds are shown.**
- **Video is required and verification is required.**
- **Emulators are allowed**, but see §6.

Platforms: PS2, PS3, PS Vita, PS TV, PS Now, and backwards-compatible PS3.

> **Note on the PS3 entries.** The PS3 platform on this entry is the HD/PSN
> release of Ratchet & Clank 1 (`NPEA00385` / `BCES01503`), not the PS2 disc.
> Nearly all the fastest overall times are on it. When a table says "PS2", it
> means the PS2 disc.

## 2. Categories

Five per-game categories and one per-level category type.

| Category | Rules, verbatim from the API |
|---|---|
| **Any%** | "Beat the game from a fresh file. File manipulation from files other than the one used for the run is banned. A pre-made save is allowed for quicker resets but must be created with 0 bolts collected, 10 Bomb Glove ammo, with the wrench out." |
| **NG+** | "Beat the game from a fresh NG+ (Challenge Mode) file. All file manipulation is allowed. You may pre-select what two weapons you have out on Veldin instead of the wrench, as well as changing your quick-select layout." |
| **All Gold Bolts** | "From a fresh file, collect all 40 Gold Bolts and beat the game… Please include the scorecard after the credits to verify all 40 Gold Bolts have been collected." |
| **All Checkmarks** | "From a fresh file, get all 3 Checkmarks (All Gold Bolts, All Skill Points and All Gold Weapons) and beat the game. The run can be completed on as many playthroughs as necessary as long as the same file is utilised the entire run… Quartu Grind Patch: There's a strat on the bolt grind where if you enter the Giant Clank fight while the robot mechs are exploding, it'll deactivate the anti-farm mechanics for the exploding robot." |
| **All Missions** | "From a fresh file, complete all planet missions and beat the game… Please include the mission menu upon timewarping after defeating Drek to verify the run." |
| **Complete Race** *(per-level)* | "Respawn skips allowed. **Use the in-game timer.**" |

Two level categories, both hoverboard races: **Rilgar Hoverboard Race** and
**Kalebo III Hoverboard Race**.

Not present on this game: 100% (it is called *All Checkmarks* here), All
Weapons, All Swings, No Death, Glitchless, All Collectibles, All Skill Points.

## 3. The records

All times are the PS2 platform, read from
`https://www.speedrun.com/api/v1/leaderboards/y655j36e/category/<id>?platform=n5e17e27`
on 2026-10-22. Every run listed is `verified`.

### Any% (`vdozvykp`)

| # | Time | Runner | Date | Region |
|---|---|---|---|---|
| 1 | **36:36.196** | CptGallant | 2025-06-29 | USA |
| 2 | 36:54.214 | cank2 | 2024-06-12 | EUR (PAL) |
| 3 | 41:17.477 | Alekhine | 2026-05-28 | USA |
| 4 | 45:45.745 | quietdudek | 2021-03-19 | EUR (PAL) |
| 5 | 51:33.093 | Touh | 2017-08-06 | EUR (PAL) |
| 6 | 53:25.205 | yamanaru | 2017-05-13 | JPN |
| 7 | 57:34.454 | SkylarFerret | 2022-04-11 | USA |
| 8 | 1:07:36.056 | CaptainLazarus | 2015-04-19 | USA |
| 9 | 1:10:42.242 | BoltClock | 2025-11-30 | USA, **emulated** |
| 10 | 1:17:18.638 | Jimmie1717 | 2014-05-09 | USA |
| 11 | 1:18:58.738 | CreepNT | 2020-06-30 | EUR (PAL) |
| 12 | 1:29:59.399 | BellumZeldaDS | 2020-06-26 | EUR (PAL) |
| 13 | 2:17:04.224 | Drunkhouse | 2023-05-06 | USA |
| 14 | 8:50:27.827 | Midnight | 2017-01-05 | EUR (PAL) |

### NG+ (`9d84vwkn`)

The top of the board (the rest is in the linked API response):

| # | Time | Runner | Date | Region |
|---|---|---|---|---|
| 1 | **27:18.638** | Ricky | 2018-05-02 | EUR (PAL) |
| 2 | 27:54.674 | cank2 | 2024-12-31 | EUR (PAL) |
| 3 | 28:03.683 | Scaff | 2017-10-15 | EUR (PAL) |
| 4 | 28:07.687 | Insomniac | 2017-05-07 | USA |
| 5 | 28:33.713 | cool_spongebob_420 | 2024-12-30 | USA, **emulated** |
| 6 | 28:58.738 | zbm | 2018-04-04 | USA |
| 6 | 28:58.738 | marathonman | 2024-07-16 | USA |
| 8 | 30:28.828 | (guest) | 2015-02-28 | USA |
| 9 | 30:30.830 | Scottobozo | 2018-01-01 | USA |
| 10 | 30:35.835 | Touh | 2017-07-31 | EUR (PAL) |

### All Gold Bolts (`xd1gmzd8`)

| # | Time | Runner | Date | Region |
|---|---|---|---|---|
| 1 | **59:45.585** | CptGallant | 2025-10-15 | USA |
| 2 | 1:09:24.164 | marathonman | 2024-07-23 | USA |
| 3 | 1:10:39.239 | cank2 | 2023-06-23 | EUR (PAL) |
| 4 | 1:18:45.725 | isaka | 2023-06-06 | EUR (PAL) |

### All Checkmarks (`xd173wwd`)

| # | Time | Runner | Date | Region |
|---|---|---|---|---|
| 1 | **3:23:47.227** | CptGallant | 2024-11-03 | USA |
| 2 | 6:09:49.189 | Ratchetmania | 2017-09-09 | USA |
| 3 | 6:33:52.632 | TOTOzigemm | 2016-06-18 | USA |

### All Missions (`n2yywr82`)

| # | Time | Runner | Date | Region |
|---|---|---|---|---|
| 1 | **1:00:51.651** | CptGallant | 2026-05-21 | USA |

### The hoverboard races (Complete Race, `wk6p8rqd`)

These are the only categories with an **in-game timer**, and they are the
closest thing the game has to a speedrun timer.

**Rilgar Hoverboard Race** — PS2
(`https://www.speedrun.com/api/v1/leaderboards/y655j36e/level/592zng96/wk6p8rqd?platform=n5e17e27`):

| Time | Runner | Date | |
|---|---|---|---|
| **1:10.860** | cisco | 2024-07-21 | |
| 1:13.350 | PhaseRun | 2022-03-19 | |
| 1:13.350 | smallvans | 2023-08-17 | |
| 1:18.880 | Clanks | 2019-01-07 | |
| 1:19.600 | SanosukeEindride | 2026-09-09 | emulated |
| 1:19.960 | Sylvamy | 2026-09-09 | emulated |
| 1:21.700 | TheGamersChemLab | 2024-07-17 | emulated |
| 1:22.280 | Nuka | 2014-02-22 | |
| 1:34.860 | jethi_ | 2025-10-13 | emulated |

**Kalebo III Hoverboard Race** — PS2
(`https://www.speedrun.com/api/v1/leaderboards/y655j36e/level/29v3z1wv/wk6p8rqd?platform=n5e17e27`):

| Time | Runner | Date | |
|---|---|---|---|
| **0:51.580** | Clanks | 2019-02-24 | |
| 0:51.860 | jun | 2009-11-03 | |
| 0:52.300 | Asgarion | 2024-09-11 | emulated |
| 1:24.800 | Flyincars | 2017-10-29 | |
| 1:39.200 | (guest) | 2008-07-31 | |
| 2:03.120 | Touh | 2016-02-12 | |
| 2:04.080 | AuronSuper95 | 2010-04-29 | |

Both boards are listed sorted by time; the site's own placement puts a couple
of the older runs lower than their time would suggest.

The overall records for both races are on PS3: Rilgar 1:10.880 and Kalebo III
0:40.510 (`https://www.speedrun.com/api/v1/levels/<id>/records`).

### Cross-platform context

The PS3 release of Ratchet & Clank 1 is much faster than the PS2 disc, because
its loading times are far shorter. The community's guide puts the PS2's
penalty at **around eight minutes**
(<https://www.speedrun.com/rac1/guides/gnz18>).

| Category | PS2 | PS3 (HD/PSN) | Vita |
|---|---|---|---|
| Any% | 36:36.196 | 20:14.214 | 48:39.919 |
| NG+ | 27:18.638 | 15:49.949 | 38:22.302 |
| All Gold Bolts | 59:45.585 | 44:27.667 | — |
| All Checkmarks | 3:23:47.227 | 1:28:57.337 | — |
| All Missions | 1:00:51.651 | 46:15.775 | — |

**This is the number to keep in mind for the port**: a native port has no
loading at all, so it starts from the PS3's footing and goes further. The PS2
Any% record of 36:36 is roughly eight minutes of loading and about
twenty-eight of play; a port Any% should therefore land in the high twenties
— an estimate, not a measurement, but the right order of magnitude. Whatever
it is, it will not be comparable to either column.

## 4. The movement tech

From the community's **Movement Guide** by ThaRixer
(<https://www.youtube.com/watch?v=V8TG2sCofwM>), whose description carries
the full chapter list. "All the tricks in the video work on all versions of
Ratchet & Clank unless stated otherwise."

| | Technique | |
|---|---|---|
| 01:31 | **Lag Jump** | |
| 02:13 | **Neutral Long Jump (NLJ)** | "a long jump method that gains speed" |
| 03:20 | **Neutral Cancels** | |
| 05:06 | **Mid Air Long Jump** | |
| 07:18 | **DC Neutral Long Jump** | |
| 07:59 | **Thruster Pack Long Jump** | |
| 09:35 | **Ground Pound Cancel (GPC)** | "with the thruster-pack to allow for a little extra height gain" |
| 11:28 | **Thruster Pack Lag Jump** | |
| 11:56 | **Infinite Long Jump (ILJ)** | speed gain over mud with the Heli-Pack (Rilgar, Umbris, Hoven) |
| 14:30 | **Sinaflips** | |
| 15:50 | **Infinite Sideflips (ISF)** | infinite height with the Thruster-Pack |
| 16:56 | **Ground ILJs** | |
| 17:29 | **Clankless Infinite Jump** | |
| 19:29 | **Heli-Pack Infinite Jump** | |
| 19:52 | **Thruster Pack Infinite Jump** | |
| 20:24 | **PDA Sideflip** | |
| 21:44 | **PDA Stops & sPDAs (SpeedyA)** | "gain speed quickly via opening the PDA after each long jump" |
| 22:54 | **PDA Glide** | |
| 24:01 | **PDA Death** | |
| 24:23 | **ReILJ / ReSinaflips** | |
| 25:30 | **Walloper Punches** | |
| 28:48 | **Quick Switching** | |
| 30:09 | **Ghost Ratchet** | |
| 31:09 | **Slope Intercept (SI)** | "used in almost every game in the series to climb up sloped surfaces" |
| 32:12 | **Swingshot Swag** | |
| 32:57 | **Pod Warp** | |
| 33:50 | **Decoy Clips** | "a lot easier on PS2" |
| 34:57 | **Jackpot Glitch** | "used in any% runs to get more bolts from crates than intended" |
| 36:24 | **Tree Clip & Clank Proxy** | |
| 37:16 | **Clank Boost** | |
| 37:45 | **Visibomb Canceling** | |
| 38:29 | **Drek Skip Setup** | **PS3 only** |
| 39:34 | **Pyro Glitch** | **PS3 only** |

Other acronyms from the community's resources document: `IJ` Infinite Jump
(with the PDA), `GPC` Ground Pound Cancel, `ILJ` Infinite Long Jump, `ISF`
Infinite Sideflip, `NLJ` Neutral Long Jump, `sPDA` SpeedyA, `ILM` In-Level
Movie (opened from the Goodies Menu), `BCJ` Big Clank Jump, `OoB` Out of
Bounds, `Clipping`, `Proxy`.

### A caution about names

A number of trick names in circulation belong to *other* games in the series
or to no game at all. **"Kogor Reforge", "Toren", "Flotsam", "Rave City",
"Summit", "Gas Python", "Sakkara", "Zolar", "Koltira", "Darsigner",
"Viscidity", "Ravine", "Saffi", "Kechse", "Klon" and "Tren" are not Ratchet &
Clank 1 techniques** — most are Ratchet & Clank 2 locations. Likewise
"zipper", "moonwalk", "bunny hop" and "crouch-jump" are not R&C1 terms. If a
route calls for one of these, check the source before assuming it applies.

Also note: **the game's tricks are frame-exact**. A movement tech that works
on a console at 60 Hz may behave differently in the port's 60 Hz PAL mode,
and differently again at 50 Hz. The port runs the PAL game at 60 Hz, so it
matches the NTSC behaviour — but the *script* of a level is PAL's.

## 5. Per-level routing

The nineteen levels, from the decompilation's own overlay table
(<https://github.com/OpenRAC/rac1-decomp/blob/master/docs/OVERLAYS.md>) and
the port's `port/game/rac1/host/front_end.c:12-16`:

| | | | |
|---|---|---|---|
| 0 Veldin | 5 Rilgar | 10 Orxon | 15 Quartu |
| 1 Novalis | 6 Blarg Station | 11 Pokitaru | 16 Kalebo III |
| 2 Aridia | 7 Umbris | 12 Hoven | 17 Drek's Fleet |
| 3 Kerwan | 8 Batalia | 13 Gemlik Base | 18 Veldin (return) |
| 4 Eudora | 9 Gaspar | 14 Oltanis | |

Strats found in the community's All Checkmarks route
(<https://pastebin.com/sHHfLKn0>) and the Backup Strat Archive
(<https://docs.google.com/spreadsheets/d/19FwKjaBPNCl0fkMsW2HKlEbJGxmtUucW5O995KKnqqI/edit>):

| Level | Notable strats |
|---|---|
| **Kerwan** | Any% train skip; "GB under spawn"; "GB + Jankpot"; "Skip swingshot" |
| **Eudora** | Gold bolt via the Suck Cannon |
| **Blarg** | **Decoy clips** (the "Blarg Clips"), for both Any% and NG+ |
| **Rilgar** | ILJ to the maze area; Morph-O-Ray on the tank; gold bolt in the maze; "Resetting death timer" |
| **Umbris** | Ship to boss for Any%; **"Umbris Swingshotless Ghost Ratchet"** for NG+ |
| **Batalia** | **Slope Intercept** route; the Trespasser room for bolts; jankpot activation; teleport in NG+ |
| **Orxon** | **The Orxon 1 "tree skip"** for Any%; skip Magneboots; the Clank Orxon NG+ skip |
| **Gaspar** | Turret skip; the Pilots Helmet; **the Drek Skip is PS3-only** |
| **Pokitaru** | The Suck Cannon (fast) |
| **Hoven** | Turret boss fight; ILJ to the Raritanium; Re-ISF |
| **Oltanis** | **Ledge proxy to the PDA**; the infobot; Morph-O-Ray; "rare instaskip" (a lag-related instant skip) |
| **Quartu** | **The Quartu bolt grind** — the one the All Checkmarks rules mention; "Use codebot (skip checkpoint and die)" |
| **Drek's Fleet** | Codebot (then die); a camclip out of the water area; skip the electricity-water trigger |
| **Veldin 2** | **Slope Intercept** into OOB; GILJ + chain to the Taunter gold bolt; ILJ + IJ into the arena |

## 6. The emulator question

The community's position, from its resources document: *"Emu: Emulator. Fully
allowed for portable games, timegated or slower for Ratchet 1, 2 & 3."*

A moderator on the forum: *"There is a timegate for emu on this game"* — and
another: *"We are very stringent with our measures in terms of allowing
emulators on the boards, especially for main series games like rac1."*

Emulated PS2 runs **do** exist on the boards (BoltClock's 1:10:42 Any%, and
Asgarion's 0:52.300 Kalebo III race). Both runners asked for emulated runs to
be shown separately — one wrote: *"Emulator runs should be shown separately."*

**A native port is not an emulator.** It is a different platform with its own
rules, and it needs its own leaderboard. See [RULES.md](RULES.md).

## 7. The community's resources

| Resource | Where |
|---|---|
| **Ratchet & Clank Speedrunning Resources** (the hub) | <https://docs.google.com/document/d/1DelfZuUbrm8wXm5gwhlFDedIuAVCq8GDY8qchgAIhWg/edit> |
| **Version Differences** | <https://docs.google.com/document/d/1QxLoN_tXby3QQfGMmG3CnTOE5S-P5VFE3mYmLWeFRVo/edit> |
| **Getting started with Ratchet 1 speedrunning** | <https://www.speedrun.com/rac1/guides/gnz18> |
| **Movement Guide** (ThaRixer) | <https://www.youtube.com/watch?v=V8TG2sCofwM> |
| **Camera Tech Guide** (marathonman) | <https://www.youtube.com/watch?v=JndUzgdIzrc> — camflips, ledge proxies, camera proxy |
| **All Checkmarks route (2025)** | <https://pastebin.com/sHHfLKn0> |
| **Backup Strat Archive** | <https://docs.google.com/spreadsheets/d/19FwKjaBPNCl0fkMsW2HKlEbJGxmtUucW5O995KKnqqI/edit> |
| **NG+ splits** (a LiveSplit `.lss`, by clippy) | <https://www.speedrun.com/rac1/resources/6jco3> |
| **PS2 NTSC USA NG+ save** | <https://www.speedrun.com/rac1/resources/dcuje> |
| **NG+ save, PAL PS3** (clippy) | <https://www.speedrun.com/rac1/resources/fgaqd> |
| **Community Sum of Best** | <https://docs.google.com/spreadsheets/d/1yvSrYm2bVf49v8rwx5t21ts0KBhTlWIu48UxI5_-Cmw> |
| **Game IDs List** (community RAM addresses) | <https://docs.google.com/spreadsheets/d/1mR9PifX1z_TsptxFUIoIKDTDihqvXSCB3I2-V-a5yp8/edit> |
| **The Discord** | <https://discord.gg/uburntFvQU> |
| **RaCMan** (a practice trainer for the PS3 release and RPCS3) | <https://github.com/MichaelRelaxen/racman> |

Note what is **not** there: the resources tab has **no tools and no patches**
for `rac1`. There is no LiveSplit autosplitter for Ratchet & Clank 1 anywhere
— the official autosplitter index contains exactly one Ratchet entry, *Rift
Apart*. The `live-splits` mod for the port is the first thing of its kind for
this game.

## 8. What the port changes for a runner

| | On the PS2 | On the port |
|---|---|---|
| **Load times** | ~8 minutes of the run | None |
| **PAL at 50 Hz** | The PAL disc's default | The port runs **60 Hz** on both discs |
| **Tricks that need 60 Hz** | The Orxon 1 Clank proxy needs it, and cannot be done at 50 Hz | Always available |
| **Cutscene skipping** | No in-game skip. The trick is "hold L1 L2 R1 and R2 and press start" | `skip_cutscenes = 1` (default) makes Start press the game's own skip buttons; `--skip-movies` skips the movies entirely |
| **Save states** | PCSX2 has them | **None** — see [SAVESTATES.md](SAVESTATES.md) |
| **Frame rate** | Fixed | 60 ticks/s always; the picture is decoupled |
| **Slow motion / turbo** | PCSX2: Shift-Tab and Tab | The overlay's F7/F8, down to 1/16 |
| **Level select** | None | `--level N`, `OPENRAC_DIRECT=1`, or F1 in developer mode |
| **Auto-splitter** | None | The `live-splits` mod |

## 9. Sources

- speedrun.com's public API, read 2026-10-22:
  - <https://www.speedrun.com/api/v1/games/y655j36e> (the game and its ruleset)
  - <https://www.speedrun.com/api/v1/games/y655j36e/categories> (the categories and their rules)
  - <https://www.speedrun.com/api/v1/games/y655j36e/levels> (the level categories)
  - <https://www.speedrun.com/api/v1/leaderboards/y655j36e/category/<id>?platform=n5e17e27> (the PS2 boards)
  - <https://www.speedrun.com/api/v1/leaderboards/y655j36e/level/<id>/category/wk6p8rqd?platform=n5e17e27> (the race boards, PS2)
  - <https://www.speedrun.com/api/v1/levels/<id>/records> (the race boards, all platforms)
  - <https://www.speedrun.com/api/v1/regions> (the region ids)
- The community's documents and guides, linked in §7.
- The port's own source, cited by file and line throughout.
