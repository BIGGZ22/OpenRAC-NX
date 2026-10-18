# Architecture of the game code

Layout and runtime flow of *Ratchet & Clank* (PS2, PAL v2.00, `SCES_509.16`).
C conventions: [conventions.md](conventions.md); the byte-exact rule:
[README](README.md#the-one-rule). *(inferred)* marks a reading of the evidence,
not something a table or matched function states.

## 1. The binary

`SCES_509.16` is a MIPS R5900 ELF. `config/splat.yaml` splits it into two
code segments:

| Segment | vram | What it holds | Sources |
|---|---|---|---|
| `core` | 0x100080 | VU microcode (`vutext`), then `core_text` from 0x112380: Sony SDK libraries, newlib, the 989snd sound library, libgcc, start-up and the main loop; plus `core_data`, `core_rdata`, `core_bss` | `src/core/` (53 files named by start address, e.g. `src/core/0012AC80.c`), `src/libgcc/` |
| `main` | 0x15ED80 | the game program: `core_lit`, `lit` (0x15F000), `bss`, `data`, the three level vtables (`lvl_vtbl`, `lvl_camvtbl`, `lvl_sndvtbl`) and `text` from 0x1E9080 | `src/game/` |

Object-to-address maps in retail link order (read by the Makefile, linker
script and tools): `config/core_text.objects` (start address per object,
boundaries from retail's `0xCDCDCDCD` linker fill; column 3 `ee29` marks the
49 SDK objects built with Sony's 2.9-ee) and `config/text.objects` (game
objects, e.g. `build-sn/game/bmain.o 0x001E9808`).

Compilers ([TOOLCHAIN.md](../TOOLCHAIN.md)): SN Systems GCC 2.95.3 (build
v1.14) for game code; Sony's 2.9-ee-991111 for SDK objects and libgcc.

### Original source file names

Each `src/game/*.c` header comment gives the original C++ unit, e.g.
`bmain.cpp in the original source; text 0x1E9808-0x1E9E70`; compiled as C, it
gives the same code. Evidence:
[DATAMINED_REVERSE_ENGINEERING.md](../DATAMINED_REVERSE_ENGINEERING.md).
Game units in link order (`config/text.objects`): actuator, bloaders, stub,
bmain, transition, camera, effects, cmecpu, collproc, draw, drawquad,
fastfunc, framebuf, freeze, help, hud, init, initonce, lights, loaders, map,
menu, memcard, miscproc, missionfunc, mobyfunc, mobyproc, mobyutil, music,
stream, pad, partproc, partupd, pause, shadowproc, shrubfunc, shrubproc,
skyfunc, skyproc, sound, space, stash, tfragfunc, vuchain, tfragproc,
tiefunc, tieproc, update, vendor, then `movie/*` (MPEG player; see
[MOVIE.md](../MOVIE.md)).

## 2. Level overlays

`main` only runs before the first level loads. **Each of the 19 levels ships
its own, larger build of the game program** that replaces all of `main`;
`core` stays resident ([ASSETS.md](../ASSETS.md) "Code overlays",
[OVERLAYS.md](../OVERLAYS.md)). A level's code is seven records, dumped by
`tools/overlays.py dump` to `baserom/overlays/level_NN/` (`manifest.json`).
Level 00:

| Record | Address | Bytes |
|---|---|---|
| `lit` | 0x15F000 | 11,968 |
| `bss` | 0x161F00 | 16,816 |
| `data` | 0x166100 | 542,000 |
| `vtbl` | 0x1EA680 | 1,284 |
| `camvtbl` | 0x1EAB00 | 160 |
| `sndvtbl` | 0x1EAC00 | 24 |
| `text` | 0x1EAD80 | 1,078,552 |

`lit` starts at 0x15F000 and `$gp` is 0x166D00 in every level; the other
records are packed per level, so one function sits at different addresses in
different levels. ParseBin (`func_0012DA38`) copies the records and returns
the entry point; `main` (`func_0012DB18`) calls it.

### Level index to planet

Levels go by index `NN` everywhere except the source directory
([OVERLAYS.md](../OVERLAYS.md#levels)):

| NN | Dir | NN | Dir | NN | Dir |
|---|---|---|---|---|---|
| 00 | `l00_veldin1` | 07 | `l07_umbris` | 14 | `l14_oltanis` |
| 01 | `l01_novalis` | 08 | `l08_batalia` | 15 | `l15_quartu` |
| 02 | `l02_aridia` | 09 | `l09_gaspar` | 16 | `l16_kalebo3` |
| 03 | `l03_kerwan` | 10 | `l10_orxon` | 17 | `l17_fleet` (Drek's Fleet) |
| 04 | `l04_eudora` | 11 | `l11_pokitaru` | 18 | `l18_veldin2` |
| 05 | `l05_rilgar` | 12 | `l12_hoven` | | |
| 06 | `l06_blarg` (Blarg Station) | 13 | `l13_gemlik` | | |

### Function kinds

`config/overlays/functions.tsv` lists each distinct level function
(deduplicated with address fields masked, `identity()` in `tools/overlays.py`)
with kind, size, fingerprint, level count and places (`level:address`).

| Kind | Count | Meaning | Name | C lives in |
|---|---|---|---|---|
| `exe` | 749 | same code as an executable function | `func_XXXXXXXX` | `src/game/` |
| `shared` | 1,963 | in two or more levels, not in the executable (1,159 in all 19) | `func_LNN_XXXXXXXX` | `src/overlays/shared/` (140 files) |
| `level` | 1,468 | in exactly one level | `func_LNN_XXXXXXXX` | `src/overlays/lNN_<planet>/` |

`NN` is the lowest level with the function (*canonical level*), `XXXXXXXX`
its address there.

Overlay files are named after the **executable unit they follow in link
order** plus their first function (`hud_00235960.c`), not their subject
([OVERLAYS.md](../OVERLAYS.md) "Plan" step 2): `help_002180F8.c` holds hero
code (1.17 MB of level code lies between `help` and `hud`), `vendor_*.c` many
moby updates. The grouping is provisional.

### The dispatch tables: which function updates moby class N

Three tables per level ([OVERLAYS.md](../OVERLAYS.md#roles); class binding in
[mobys.md](mobys.md#binding-classes-to-update-functions)):

| Record | Entry | Terminator |
|---|---|---|
| `vtbl` | 12 bytes: `{oClass, update function, pointer to a 6-word table}`, one per moby class in the level | oClass -1 |
| `camvtbl` | 20 bytes: `{camera id, init, activate, update, exit}` | id -1 |
| `sndvtbl` | 8 bytes: `{id, function}` | id -1 |

`tools/overlays.py names` decodes them into `config/overlays/names.tsv`:
roles `UpdateMoby_<oClass>`, `Init/Activate/Update/ExitCamera_<id>`,
`SoundFunc_<id>`, with levels. Many roles means generic (e.g. the empty
`func_001E9768` in `src/game/stub.c`); `tools/dossier.py` writes the role
into a worker's `CONTEXT.md`. Recipe (class 726):

```sh
grep -P '^726\t' tools/extract/moby_classes.tsv          # 726  novalis_elevator
grep -oP '^\S+(?=\t.*UpdateMoby_726:)' config/overlays/names.tsv   # func_L01_002BB068
grep -oP 'UpdateMoby_726:[0-9/]+' config/overlays/names.tsv      # levels 01/03/13
grep -rln func_L01_002BB068 src/overlays                         # shared/vendor_002B90A8.c
```

## 3. Frame and engine flow

### Boot and level start

- `main` (`func_0012DB18`, core): outer loop; loads level code via ParseBin.
- `src/game/bmain.c`: `func_001E99D8` is start-up before the first level
  (clears level bss, intro and loading images, intro movie, sound bank, video
  mode, starts the level load). `func_001E9808(level)` starts a level from
  the table at `D_00137C80` (alternate set while `D_0015EE80` is set), waits
  for the loader and loads it: `StartLevel` (the NTSC name `showDebugFont`
  does not fit; overridden in `config/names_local.tsv`).
- One-time set-up: `InitOnce` (`func_00201E88`, `initonce.c`),
  `LoadIRXModule` (`func_00201D58`, `init.c`), `LoadHudBanks`
  (`func_002032D0`, `loaders.c`).

### The moby update loop

`func_00213C78` (`MobyUpdateLoop`, candidate) in `src/game/mobyutil.c`, still
assembly. Per moby in list order: skip if `(s8)state < 0`; pre pass
`func_0020E3D0` unless flag 0x40; `m->update(m)`; post pass `func_0020ED48`
unless flag 4. The list head is also stored at `$gp-0x6CDC`. Reconstruction
and details: [mobys.md](mobys.md#the-per-frame-update-loop).

A typical caller is the vendor-menu world update `func_L00_0029D988`
(`src/overlays/shared/update_0029B6A0.c`): `func_L00_00299108`, the moby
loop, `func_0022EF68` (`enqueue_voice_request`, candidate),
`func_L00_002076E8`, then `UpdateParts` (`func_00218A80`), each gated by
flags and separated by calls to the empty `func_001E9768`. *(inferred)* Those
were profiler timing hooks, compiled out of the shipped build.

### Profiler stage labels

Draw stages, in order, from the executable's string table
(`config/strings.tsv`, 0x00160D80-0x00160EA0), stages separated by `|`:
render setup | sky texs, sky draw | tfrag texs, tfrag draw (terrain
fragments) | tie texs, tie draw (instanced static geometry) | shrub texs,
shrub draw | moby texs, moby draw | effects texs, vu effects, moby effects |
part draw (particles) | post effects, screen overlays, aa blur | debug text.

Update labels: `moby update`, `hero update`, `part update` at 0x00161148 in
the executable; `camera update`, `sound update`, `system misc` only in the
level `lit` record (level 00: 0x1610E8, 0x1610F8, 0x15FD78). *(inferred)* The
order is the frame's draw order; `DrawDebugProfiler` (`func_001F3D78`)
prints them.

### The hero and the pad

Field tables: [data-structures.md](data-structures.md).

- **Hero**: global `D_0013F450`, `struct Hero` (`include/hero.h`).
  `D_0013F4D0` (`gHeroPos`) is the position block.
- **Pad**: `D_0013CA40`, `struct PadState` (0x350 bytes, `include/pad.h`).
- **Hero code is level code.** The largest functions are per-level builds of
  the hero state machine, all matched C:

| Function | Size | File | Comment in source |
|---|---|---|---|
| `func_L00_002180F8` | 39,324 | `src/overlays/shared/help_002180F8.c` (6 levels) | per-frame movement and physics for the current hero state |
| `func_L17_00214F68` | 36,168 | `src/overlays/l17_fleet/help_00214F68.c` | same, level 17 build |
| `func_L16_00214C88` | 36,052 | `src/overlays/l16_kalebo3/help_00214C88.c` | same, level 16 build |
| `func_L18_0021F1D8` | 33,392 | `src/overlays/l18_veldin2/help_0021F1D8.c` | same, level 18 build |
| `func_L00_0022A340` | 33,312 | `src/overlays/shared/help_0022A340.c` (6 levels) | per-frame transition logic for the current hero state |

`HeroUpdate` (`func_L01_00228E08`) is in `src/overlays/shared/help_002274A8.c`.
Copies that differ in one constant or offset are separate functions
([OVERLAYS.md](../OVERLAYS.md) "Names"), so a port must run the build that
belongs to the current level.

## 4. Progress snapshot

From `progress/report.json` (2026-10-18):

| Measure | Value |
|---|---|
| Functions matched | 4,805 / 5,118 (93.88%) |
| Code matched | 3,486,816 / 3,712,772 bytes (**93.91%**) |
| Executable (core + game + libgcc) | 1,601 / 1,689 functions, 89.62% of code |
| Level code | 3,204 / 3,429 functions, 94.53% of code |
| Shared level code | 1,778 / 1,980, 87.34% |
| Level-specific code | 1,426 / 1,449, 98.75% |

Levels 00, 02, 09, 11, 13, 16, 17 and 18 are at 100%.

Remaining work: `INCLUDE_ASM` stubs (see
[conventions.md](conventions.md#assembly-stubs)); near misses in
`nonmatching/<dir>/`, closest first in `nonmatching/README.md`
([NONMATCHING.md](../NONMATCHING.md)); port-only C that behaves like retail
without matching in `nonmatching/functional/` and
`nonmatching/functional.tsv` (never built). Original assembly
counts as finished: `config/handwritten_asm.txt` (245 entries, including 33
VU0 helpers in `fastfunc.o` added 2026-10-17) and `config/linker_remnants.txt`
([ASM_CLASSIFICATION.md](../ASM_CLASSIFICATION.md)).

## 5. Where do I look for X

Readable names come from `config/names.tsv` / `include/names.h`.

| Looking for | Start here |
|---|---|
| Update function of moby class N | `config/overlays/names.tsv` role `UpdateMoby_N`, class names in `tools/extract/moby_classes.tsv` (recipe in [section 2](#the-dispatch-tables-which-function-updates-moby-class-n)) |
| A moby's private state | `Moby.pvars` (0x78) cast to the class's struct in the update function's file; [mobys.md](mobys.md), [PVARS.md](../PVARS.md) |
| Mobys | `CreateMoby` (`func_0020D348`), `DeleteMoby` (`func_0020D678`), `InitMobyInstance`, `MobyAnimProc` in `src/game/mobyfunc.c`, `mobyproc.c`, `mobyutil.c` (loop `func_00213C78`); `MobyProc` (`func_L00_002569F8`) in level code |
| Hero movement and states | `src/overlays/shared/help_002180F8.c`, `help_0022A340.c` and their per-level copies (`src/overlays/l1[678]_*/help_*.c`); `include/hero.h` |
| Controller input | `UpdatePad` (`func_00218908`), `ClearPadInput` in `src/game/pad.c`; `include/pad.h` (`D_0013CA40`) |
| HUD | `HudDraw` (`func_001FFFB8`), `HudSprite`, `Hud_*` in `src/game/hud.c`; banks in `src/game/loaders.c` (`LoadHudBanks`); level `HudFrame` (`func_L00_0023BAB8`) |
| Camera | `UpdateAllCameras` (`func_001EC7C8`), `UpdateCamera` (`func_001EDE50`), `camera_update` (`func_001ED080`, candidate), `Camera_*` in `src/game/camera.c`; per-level cameras via `camvtbl` roles `InitCamera_<id>` / `UpdateCamera_<id>`; `src/overlays/shared/camera_*.c` |
| Sound and music | `sound_update` (`func_0022DD68`), `sound_*` in `src/game/sound.c`; `music_*` in `src/game/music.c`; level `sndvtbl` roles `SoundFunc_<id>`; 989snd `snd_*` in `src/core/` |
| Save / memory card | `memcard_Save` (`func_0020BFC8`), `memcard_PrepData`, `memcard_Restore*` in `src/game/memcard.c`; `gGameSaveData` (`D_001A05C0`), `gLevelSaveData` (`D_001A08C0`); card states and save chunk ids in [DATAMINED_REVERSE_ENGINEERING.md](../DATAMINED_REVERSE_ENGINEERING.md) |
| Menus and pause | `DrawItemsMenu`, `DrawMapScreen`, `DrawCheatsMenu`, `DrawMissionsMenu`, `PauseAllSounds` in `src/game/pause.c`, `src/game/menu.c` |
| Particles | `UpdateParts` (`func_00218A80`), `PartProc` (`func_00218B10`) in `src/game/partproc.c`, `partupd.c`; level `CreatePart2` / `KillPart` |
| Rendering | `drawNormalFrame` (`func_001F45F0`), `AddDrawCallback`, `ExecuteDrawCallbacks` (`func_001F4A00`) and variants 2-4 in `src/game/draw.c`; `DrawMobys` (`func_0020E2B0`), `DrawTfrag` (`func_002346C0`), `DrawTies_1/2`, `DrawShrubs`, `SkyDrawShell*` in the `*proc.c` units (`tfragproc.c`, `tieproc.c`, `shrubproc.c`, `skyproc.c`); `src/game/vuchain.c` |
| Level loading | `src/game/bmain.c` (`func_001E9808`, `func_001E99D8`), `src/game/loaders.c`, ParseBin in `src/core/`; on-disc layout in [ASSETS.md](../ASSETS.md) |
| A symbol's readable name | `python3 tools/names.py func_001EC7C8` or `... UpdateAllCameras` ([NAMES.md](../NAMES.md)) |
| Which file holds function X | `grep -rln func_X src/`; for level code, `config/overlays/functions.tsv` gives kind and places |
| Retail assembly of X | `asm/nonmatchings/{text,core_text}/func_X.s`, `asm/overlays/func_LNN_X.s` (after `tools/setup_asm.sh`) |
| Strings | `config/strings.tsv` (address, symbol, xrefs, value) |

Other docs: [WORKFLOW.md](../WORKFLOW.md) (set-up, iterate, verify, publish),
[BUILD_FIDELITY.md](../BUILD_FIDELITY.md) (what the build may do).
