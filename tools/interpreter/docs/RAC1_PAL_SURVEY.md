# Ratchet & Clank (PAL) as the source of a native build: a survey

What the decompilation of `SCES_509.16` shows about the path from the game's
code to the PlayStation 2's hardware: the frame loop, the display list, the
VU microprograms, the way to the main menu, the disc, the memory card, the
Sony library surface and what stands between the C as written and a host
compiler. [DESIGN.md](DESIGN.md) is built on it.

Measured on 2026-10-08 on rac1-decomp at `a35ffbee` (5,109 functions, 3,686
matched, 60.98 % of code), the tree [games/rac1/pal](../../games/rac1/pal/README.md)
is a copy of. Paths such as `src/game/vuchain.c`, `docs/ASSETS.md` and
`config/names.tsv` are relative to that directory. Every address is a PAL
address ([docs/engine/README.md](../../docs/engine/README.md), section 1). No
retail bytes are reproduced here beyond single constants.

In OpenRAC's evidence levels, **[C]**, **[asm]** and **[data]** below are
**retail**, **[ReRAC]** and **[doc]** are **reference** unless the cited
document measured the retail program itself, and **[inferred]** is
**inferred**.

How to read the tags used below:

- **[C]** read from the decompiled C in `src/`. **[asm]** read from the generated assembly under `asm/` (said explicitly each time, because most of the interesting code is still assembly). **[data]** read from the retail executable image or disc via the project's own readers. **[doc]** stated by a project document. **[ReRAC]** stated by the local ReRAC docs (a Rust re-implementation of the *US* game; its addresses are US addresses, boot text differs from PAL by about +0x380, level code by +0x100). **[inferred]** a conclusion, not shown directly by code.
- "Matched" in the project's headline figure includes `ASM_FUNC` (original hand-written assembly kept as finished work) and `LINKER_REMNANT` (leftover words). Real compiled C is smaller; see Table 0.1.
- Counts come from one-off scripts over `asm/` and `src/` (Appendix D describes the method; they are not in the repository). Function names are the project's `func_ADDR` names (`func_LNN_ADDR` for level code, address in the canonical level); names after a slash come from `config/names.tsv` (NTSC decomp, Lombyte, ReRAC) and are suggestions.

---

## Key findings (read this first)

1. **A frame is one VIF1 DMA chain built in EE RAM** (double-buffered, 1.4 MB each), kicked at the top of the next loop iteration, so the GS draws frame N-1 while the EE builds frame N. About 70 executable functions append to it (`D_00161000`); level code never touches the write pointer, it calls those helpers (`VU1_addGSregister` has 86 level callers). The chain contains CNT/REF/END tags with `DIRECT` GIF packets, UNPACKs into VU1 memory, VU1 program blobs added by REF, and IRQ-tagged fences that an interrupt handler turns into flags the CPU waits on. A native renderer can sit behind exactly this format (sections 1-2).
2. **The hardware-facing surface is narrow:** 174 functions reference EE hardware registers or the scratchpad (51 core, 77 executable, 46 level code); 103 of them are original asm or not yet decompiled (section 2.3, Appendix A).
3. **All VU microcode is in the executable** (`vutext`, nine VU1 programs and three VU0 images), uploaded on demand through the chain. **Every renderer core is hand-written assembly** using VU0 macro mode, the scratchpad and DMA channel 8/9 (tfrag, tie, shrub, moby, particles, lighting, shadows): these are "matched" only as original asm and would have to be re-implemented, not ported (sections 3, 4).
4. **COP2 appears in 171 functions (7,870 instructions) and in none of the compiled C**; it is used for collision and animation as well as drawing, and even `FastSin`/`FastCos` call VU0 microcode. Real MMI is in 107 more functions, again none in compiled C.
5. **The main menu is in the boot executable**, not in level code: boot stage `func_001E99D8` -> title loop `func_001EBB48` with a "title world" drawn by the full 3D renderer plus the page-menu code in `pause.c`. No level program is loaded for it. About 167 undone executable functions (95 KB) are reachable from boot, 128 of them from the title loop (section 5).
6. **Disc access is one primitive:** `sceCdRead(lba, sectors, buf)` on a 2048-byte-sector disc addressed by absolute LBA through a table of contents at LBA 1500; audio streaming is done by the IOP, movies by libmpeg (section 6).
7. **Memory card:** libmc (13 calls, all in one state machine) under a 25-state status monitor under UI; all of it compiled C except two dialog functions; the save is `save%d.bin` blocks described by two descriptor tables (section 7).
8. **Sound is the 989snd API** (about 42 entry points, all compiled C except the three RPC-layer functions) over SIF RPC to IOP modules; pad is libpad2 behind `UpdatePad`; threads/semaphores exist only in the movie player (section 8).
9. **Portability:** the C is written to reproduce MIPS code. The blockers are code that is not C (1,519 undone functions, 212 asm functions, all COP2/MMI), data that is not C (277k lines of generated data), 3,859 "global + big offset" accesses that assume the retail data layout, 32-bit pointer assumptions, about 200 hardware-address accesses, 2,174 `__asm__` symbol aliases and 2,467 inline-asm quadword copies. A fixed 32-bit address space would sidestep most pointer problems on Linux/Windows; a native macOS/arm64 process cannot have one (tested, section 9.5).
10. **Across the sequels** the hardware contract (VIF1 chain, GIF packets, VU1 programs on demand) looks the same family-wide, VU1 microcode is partly byte-identical RAC1 to RAC2, but front-end placement, file layer and VU memory maps differ (section 10).

---

## 0. Orientation: what the program is made of

### 0.1 Layout [data][doc]

- `SCES_509.16` is a plain ELF with one PT_LOAD at 0x100080 (file offset 0x1000, 0x13E6B0 bytes), entry 0x12D868, `$gp` = 0x166D00. In address order: `vutext` 0x100080-0x112380 (VU0/VU1 microcode, 0x12300 bytes), `core_text` 0x112380-~0x12F580 (SDK and runtime, 683 functions), core data/rodata/bss, then the "main" segment: literals at 0x15F000, bss, `data` (539 KB), three dispatch-table records, and `text` 0x1E9080-0x23E730 (349,872 bytes, 1,036 game functions) (`config/splat.yaml`).
- **Every level carries its own full build of the game program** and replaces the whole main segment (lit, bss, data, `lvl_vtbl`, `lvl_camvtbl`, `lvl_sndvtbl`, text) when it loads. Level text is 1.07-1.21 MB; the data records share only 44-84 % of their bytes with the executable (docs/ASSETS.md, docs/OVERLAYS.md). The executable's game code is a subset of every level's program but sits at different addresses in each level; only `vutext` and `core_*` (SDK, runtime, libgcc, 989snd, wad) stay resident. Distinct game code: about 3.5 MB (3.7 MB with core); 1.19 MB is shared by two or more levels and not in the executable (594K in all 19), 2.03 MB is single-level (about 107K per level).
- The entry point of each level program is the same shared function, `func_L00_002465F8` (level 0 address): the `entry_point` of all 19 `baserom/overlays/level_NN/manifest.json` agrees with the catalogue, 19 of 19. That function is the in-level main loop (section 1). It is `INCLUDE_ASM` (not decompiled), 2,248 bytes, in `src/overlays/shared/map_002465F8.c`.
- Level-specific behaviour is attached through three tables in each level's data: `vtbl` (12-byte `{oClass, update fn, ptr to 6-word table}` per moby class), `camvtbl` (20 bytes: `{id, init, activate, update, exit}`), `sndvtbl` (8 bytes) (docs/OVERLAYS.md "Roles").

### 0.2 What is C and what is not [C][asm]

A tally over every function file in `asm/` (5,150 files; report says 5,109 because some stubs are joined):

| Region | functions | real C (matched) | ASM_FUNC (orig. asm) | INCLUDE_ASM (undone) | remnants |
|---|---:|---:|---:|---:|---:|
| `src/core` (SDK, newlib, 989snd, wad, crt0) | 683 | 406 (79.9 KB) | 126 (9.0 KB) | 96 (28.2 KB) | 54 |
| `src/game` (executable's game code) | 1,036 | 692 (163.0 KB) | 86 (74.5 KB) | 167 (107.9 KB) | 91 |
| `src/overlays/shared` (code in >=2 levels, not in exe) | 1,980 | 1,177 (547.5 KB) | 0 | 733 (651.7 KB) | 70 |
| `src/overlays/lNN_*` (single-level code) | 1,451 | 912 (1,378.6 KB) | 0 | 523 (667.6 KB) | 14 |
| total | 5,150 | 3,187 (2,170 KB = 58.4 % of bytes) | 212 (83.5 KB = 2.2 %) | 1,519 (1,455 KB = 39.2 %) | 229 |

(Table 0.1.) So the project's 61 % is 58.4 % compiled C + 2.2 % original hand-written assembly. The executable's own text is only 46.6 % compiled C by bytes (163 of 349 KB); 21 % of it is hand-written assembly kept as asm and 31 % is not yet decompiled.

No data is in C: all initialised data (core_data 142 KB, exe `data` 539 KB, rodata, lit) is generated assembly `.word` tables (`asm/data/*.s`, about 277,000 lines). A host build has to turn these into C data first (section 9).

---

## 1. Frame loop

### 1.1 From entry to the first frame [asm][C]

| step | function | file | status |
|---|---|---|---|
| program entry `_start` (0x12D868): zero all registers, clear bss 0x154200-0x15ED28 with `sq`, `$gp` = 0x166D00, syscall 0x3C (SetupThread, stack top 0x1FFC000, 0x4000) and 0x3D (SetupHeap at 0x1FF8000, 0x4000), `FlushCache`, `ei`, call `main(argc, argv at 0x15EB40)` | `func_0012D868` | `src/core/crt0.c` | ASM_FUNC (original asm) |
| `__main` (constructors) | `func_0011DFC8` | libgcc | C |
| **`main`** | `func_0012DB18` | `src/core/boot.c` | **C, exact** |
| ParseBin: copies overlay records `(dst, size, type, entry)` from the loaded level block pointed to by `D_0015EF4C` until the entry address changes, returns it | `func_0012DA38` | `src/core/boot.c` | C |
| boot stage ("startlevel" in the NTSC/ReRAC naming) | `func_001E99D8` | `src/game/bmain.c` | **INCLUDE_ASM, 1,176 B** |
| level main loop (every level's entry point) | `func_L00_002465F8` | `src/overlays/shared/map_002465F8.c` | **INCLUDE_ASM, 2,248 B** |

`main` is exactly (quoted from `src/core/boot.c`):

```c
void func_0012DB18(void) { /* main */
    EntryFn entry;
    func_0011DFC8();
    entry = func_001E99D8;
    do {
        entry();
        entry = func_0012DA38();      /* ParseBin(): loads the next overlay, returns its entry */
        func_00118D80(0);             /* FlushCache(0) */
        func_00118D80(2);             /* FlushCache(2) */
    } while (1);
}
```

So the executable runs the **boot stage** once; then a level program is copied over the main segment and its entry (the level loop) runs until the level asks for a transition. The level loop calls `DoSpaceTransition` (`func_00233308`, C, space.c) which streams the next level's data and returns; `main` then ParseBins the new overlay and calls its entry.

The boot stage `func_001E99D8` is read from its assembly (there is no C or note for it). In order [asm]: set front-end flag `D_0015F6C8` = 1; `InitOnce` (`func_00201E88`, C); zero `[D_00161380, D_00165530)`; `VU1_initChain`; `DMAC_VIF1_Enable` (`func_00235018`, C); reset GS registers (`func_001F3C10`, `func_001F3D00`, C); `sceGsSyncV`; `PutDispBuffer`; memory-card check loop (`func_00209BB8`, C: 0 ok / 1 no card / 2 not enough room, with a full-screen warning image drawn until fixed or any button); play the publisher-logo movie (`func_0023B670`, C; chooses TOC slot `D_00139478` or `D_00139480` by the PAL flag); `FadeToBlack(ticks(12))`; load the sound bank (`func_0022EA20`, C) and `snd_ResolveBankXREFS`; draw the boot still (`draw_bootImage` `func_00201AF0`, C); then **`func_001EBB48` (the title / front-end loop)**; afterwards, if the video setting changed, `sceGsResetGraph` + `SetPalMode`; finally `D_0015F6E4 = D_0015EE84; D_0015EE84 = -1; DoSpaceTransition()`. This agrees with ReRAC's description of the US boot (`startlevel`, docs/plan/progression.md).

### 1.2 One iteration of the in-level main loop (`func_L00_002465F8`) [asm]

Read from all 627 lines of the assembly. The loop head is at 0x2468B0 (level-0 addresses) and exits when `D_L00_0015F650` is non-zero (then `DoSpaceTransition` runs and the function returns to `main`). Per iteration, in order:

1. **Frame clock.** Read EE timer **1** count (`0x10000800`; the C comments in places say "T0"/"T2", but 0x10000800 is T1_COUNT), add it to the 64-bit total `D_0015EE40`, write 0 back. `InitOnce` programs T1_MODE (`0x10000810`) = 0x82 (BUSCLK/256, count enable), i.e. 576,000 ticks/s [C: `func_00201E88`]. 9,600 ticks = one 60 Hz frame; 11,520 = one 50 Hz frame. The vsync handler `func_0012F308` (registered with `sceGsSyncVCallback`) increments a vsync counter `D_0015EE48` and stores `D_0015EE40 + T1_COUNT` in `D_0015EE50` [C].
2. `func_L00_001F70F0` (C, 28 B): rotates two words in a per-frame struct.
3. **Kick the chain built last iteration:** `VU1_sendChain` `func_002349B8` (C), then **`VU1_swapChain`** `func_00234948` (C). Start the new chain with the two GS draw-environment packets and the anti-alias pass: `PutDrawBufferSmall` `func_001FB598` (C), `func_001FB848` ("AA_BlurPass", C), `PutDrawBufferLarge` `func_001FB498` (C).
4. `snd_FlushSoundCommands` `func_0012DDC0` (C): sends the batched 989snd commands to the IOP.
5. **Mode dispatch** on `D_L00_0015F6A8` through jump table `jtbl_L00_001E8E50`. Every mode except 1 first calls a 32-byte wrapper the catalogue names `func_00216270` (the executable's copy of that name, `src/game/music.c`, is `func_0012F068(func_002177F0)`, a CD-stream callback setter). The catalogue conflates tiny `lui $a0; j target; addiu` wrappers that differ only in call target, and ReRAC/the NTSC names call this per-mode call `UpdatePad`; the executable's `UpdatePad` is `func_00218908` = `func_00217F68(D_0013CA40)` (the pad reader), which is called by the boot, title, space-transition and movie loops but by no `jal` in level code. So the per-mode call is probably the pad update (level-0 build of the wrapper); the L00 target is not confirmed [inferred]. From the assembly:

| mode | update | render | notes |
|---|---|---|---|
| -1 | `func_L00_001F4918` (INCLUDE_ASM, 4,932 B) | `func_L00_001F5C60` (INCLUDE_ASM, 4,884 B) | debug camera/menu |
| 0 | `func_L00_00299250` (C, 2,328 B; "in-level frame update": moby loop, callbacks, hero, particles) | `func_L00_001F91B0` (C) -> `DrawDebugProfiler` `func_001F3D78` | **normal gameplay**; `DrawDebugProfiler` is really the main world renderer (section 2.5) |
| 1 | `func_L00_0029AD18` (C, "MovieModeUpdate") | (the movie code draws itself) | PSS movie |
| 2 | `func_L00_0029A300` (INCLUDE_ASM, 1,228 B) | `func_001F45F0` (C) -> `DrawDebugProfiler` | in-engine cutscene |
| 3 | `func_L00_00277A88` (C, 1,868 B "PageMenuUpdate") | `func_L00_001F92C0` (C) -> `func_L00_002781D8` | pause / map / planet page menu |
| 4 | `func_001FD3E8` (INCLUDE_ASM, 2,856 B) | `func_L00_001F9248` (C) + `func_001FBE80` (INCLUDE_ASM, 5,480 B) | card/confirm dialogs ("freeze") |
| 5 | `func_L00_0029D988` (INCLUDE_ASM, 5,164 B) | `func_L00_002A1540` (INCLUDE_ASM, 1,096 B) | Gadgetron vendor |
| 6 | `func_00230A90` (C, 5,996 B; game-state update, runs moby updates) | `func_00232200` (C) | ship / space travel |
| 7 | `func_L00_0029AFB8` (INCLUDE_ASM, 756 B) | `func_L00_001F92E8` (INCLUDE_ASM, 248 B) | image slideshow |

   (The mode names are ReRAC's, via `config/overlays/rerac_notes.tsv`; the order and callees are from the jump table.)
6. If the flag `D_L00_0015F6BC` is set: `VU1_syncChain(1)`, `VU1_initChain`, and go straight back to the loop head (a no-draw path) [asm; purpose inferred].
7. `func_00209E68` (C, 7,064 B): the memory-card driver (section 7); `func_00209070` (C): the card-status handler table `D_001A0538[state]()`.
8. For some card states (values 3-6, 9-12, 15-20 of `D_0013D355+0x117`) it draws a rotating `HudSprite` icon (`func_00200198`, `func_00200248`, `func_00200468`, `func_00200E38`) [asm; probably the "saving/loading" spinner, inferred].
9. `func_L00_001F3A78` (C): flush queued debug-draw entries; two empty hook calls (`func_001E9768`).
10. **`VU1_syncChain(1)`** (`func_00234AC8`): spin (with `FlushCache`-style yield `func_001F9988`) until bit 0 of `D_00160FE0` clears. That bit is cleared by the DMAC interrupt handler when the *end tag* of the chain sent in step 3 completes (section 2.2). Timeout 100,000 spins then prints and calls `ResetVideoPipeline`.
11. Read T1_COUNT again and store `ticks / 9600.0` (or `/ 11520.0` when the PAL flag `D_0015EE80` is set) as the float frame time `D_0015F6B4` (`func_001FA888` is int to float). If in mode 0 or 2 and the frame took longer than one frame period (9,600 or 11,520 ticks) it runs **one more logic update with no render** (pad + `func_L00_00299250`, or the cutscene update) before presenting: a catch-up/frame-skip.
12. **Present:** `sceGsSyncV(0)` = `func_00122598` (C): it polls `INTC_STAT` bit 2 (VBLANK_START) in software via `func_00118ED0` (libkernel `VSync`, no interrupt wait), then returns the field bit of GS_CSR; `func_00228110` (C): the 1/60 s game clock (`D_0015EF24`, with +10 every 50th frame on PAL); `ResetGsRegistersPr` `func_001F3D00` (C): rewrite GS PMODE (0xFFA1), SMODE2, DISPFB1/2, DISPLAY1/2, BGCOLOR from a 3-word table in `D_00151880` (see 1.3).
13. If a transition flag is set (mode 0 and `D_0013E633+0xE1D+0x20B1`): `snd_reset_state_and_flush_commands`, `sound_StopAllSounds`, `music_Stop`, `LoadLevelCoreData` `func_L00_00244AE0` (INCLUDE_ASM, 4,264 B), `music_StartTrack`, `init_hud`, then continue. Per-level play-time statistics are updated from the frame time (an 8-byte record per level at `D_0014171B+0x65`).

There are several other places that repeat the same "sync, vsync, `D_0015F538++` (global vsync counter), `VU1_initChain`/`VU1_sendChain`/`VU1_swapChain`" pattern: `FadeToBlack` `func_001F4E08` (C; it blocks and loops frames itself), `DoSpaceTransition`'s load loop (`func_00233308`), the boot stage, the title loop (1.4), the movie player. A native frame pacer has to cover all of them or be hooked at `sceGsSyncV` (`func_00122598`, 24 exe + 12 level call sites), which they all go through.

### 1.3 How a frame is "finished" and presented [C][asm; partly inferred]

- A frame is **one VIF1 source chain** in EE RAM (section 2). There is no explicit "end frame" call: the chain is closed (`0x70000000` end tag appended) and DMA-kicked by the *next* iteration's `VU1_sendChain`, so the GS renders frame N-1 while the EE builds frame N (one frame of pipelining) [asm/C].
- Display: `PutDispBuffer` (`func_001FB470` = `sceGsPutDispEnv`) is called at init; each vsync `func_001F3D00` rewrites the GS display registers from `D_00151880`'s table. There is **no per-frame swap of the displayed frame-buffer address** in the loop. The struct set up by `func_001FAB40` (INCLUDE_ASM, named `SetupFS_AA_buffer` upstream) looks like a `sceGsDBuff` (two disp envs, giftag+draw env pairs at +0x30/+0x40 and +0xC0/+0xD0); each frame the chain starts with draw env at +0xC0 ("Small"), an AA pass (`func_001FB848`, REF to `D_00151A00`, 0x26 qwords), then draw env +0x30 ("Large"). The likely reading: one displayed colour buffer plus a second small buffer used for a full-screen-AA blur pass [inferred from names and layout; not verified].
- Screen geometry [asm `func_001F3890`]: PAL (`D_0015EE80 != 0`) buffers 512x448 at GS addresses FB0 = 0, FB1 = 0x100000, Z = 0x1E0000, textures from 0x2C0000; NTSC 512x416, FB1 = 0xE0000, Z = 0x1B0000, textures from 0x280000.

### 1.4 Other frame loops in the executable [asm]

- **Title / front-end loop** `func_001EBB48` (INCLUDE_ASM 1,256 B, `src/game/transition.c`): per vsync `VU1_sendChain, VU1_swapChain, PutDrawBufferSmall, framebuf_appendSmallSetup, PutDrawBufferLarge, func_00209E68 (card driver), func_00209070, UpdatePad, Help_LoadMsgs, sound-slot housekeeping, func_001EB458 (update, INCLUDE_ASM 868 B), func_001EB7C0 (draw, C 816 B), VU1_syncChain(1), sceGsSyncV, D_0015F538++`, until `D_0015F690` is set. After `ticks(1500)` idle frames (0x5DC through `func_001F98C0`) it calls `func_001E9808` (C, bmain.c) which plays an attract movie. This is the same iteration shape as the level loop.
- `DoSpaceTransition` (C): after the story cards/movies it loops `VU1_sendChain, swap, PutDraw*, func_00218908, func_00230A90, func_00232200, func_00209E68, func_00209070, VU1_syncChain(1), sceGsSyncV, D_0015F538++, func_00228110` until `D_0015F6FC` is set, while `func_00204C60` (INCLUDE_ASM, 864 B) streams the level in the background.

---

## 2. How drawing reaches the hardware

### 2.1 The display list: a VIF1 DMA source chain built in EE RAM [C]

`src/game/vuchain.c` holds the primitives (all C, exact). State: write pointer `D_00161000`; two buffers `D_00160FF8[2]` (their base is the memory slot table `D_001941C0[1..2]`, each `D_0016100C` bytes, 0x160000 by default, per-level sizes from `D_001DE338`); `D_00161010` is the buffer index; limits `D_0015F718/1C`; high-water mark `D_00161014`; flags `D_00160FE0`.

Quoted examples (exact):

```c
/* VU1_addGSregister(reg, data): one A+D write as a 2-qword DIRECT packet */
D_00161000[0] = 0x10000002;  D_00161000[1] = 0;          /* DMAtag: CNT, QWC=2           */
D_00161000[2] = 0;           D_00161000[3] = 0x50000002; /* VIFcode NOP, DIRECT 2 qwords */
D_00161000[4] = 0x8001;      D_00161000[5] = 0x10000000; /* GIFtag NLOOP=1 EOP, FLG=PACKED REGS=A+D */
D_00161000[6] = 0xE;         D_00161000[7] = 0;
*(long *)((char *)D_00161000 + 0x20) = arg1;  D_00161000[10] = arg0;   /* data, register address */

/* VU1_addDataRef(ptr, qwc): reference a prebuilt packet (also how VU1 programs are added) */
D_00161000[0] = qwc | 0x30000000;  D_00161000[1] = (int)data;  /* REF tag */

/* VU1_sendChain */
D_00161000[0] = 0x70000000; ...                /* END tag */
chan = sceDmaGetChan(1);  *chan |= 0xC0;  FlushCache(0);  sceDmaSend(chan, D_00160FF8[D_00161010]);
```

Tag/packet kinds seen in the C (`D_00161000[n] = ...`):

| pattern | meaning | where |
|---|---|---|
| `0x1000000N`, `0x5000000N` | CNT tag followed by inline data, VIF `DIRECT N` (GIF PATH2) | `VU1_addGSregister`, `VU1_setScissor`, `DrawTexturedQuad` (`func_001F5800`), `HudSprite` family, `framebuf` rectangles |
| `0x3000000N` + address, `0x5000000N` | REF tag to a prebuilt GIF packet in data, then DIRECT | `PutDrawBufferSmall/Large` (draw envs at `D_0015EFB8+0xC0/+0x30`), `VU1_texFlush` (`D_001DF180`), `VU1_gsRegsNormal` (`D_001DE740`), `func_00234FA8` (`D_0013D010`), AA pass |
| `qwc|0x30000000` + program address | REF to a VU1 program blob (FLUSH + MPG...) | `VU1_addDataRef` (section 3) |
| `qwc|0x10000000, 0, 0x1000404, arg|(qwc<<16)|0x6C000000` | CNT + STCYCL + UNPACK V4-32 into VU1 data memory | `write_vif_unpack_packet` `func_00234BA0`, `func_001F7868` (camera matrices) |
| `arg + 0x90000000` | **CNT with IRQ bit**, with a code in tag bits 16-25 | `Vif1ChainCmd` `func_00235290` (fence) |
| `0x70000000` | END | `VU1_sendChain` |

DMA tag convention: word0 = QWC | (ID << 28) | IRQ << 31; word1 = ADDR; words 2/3 = the VIFcodes sent with the tag (TTE).

### 2.2 How the chain is sent [C][asm]

- `VU1_sendChain` (`func_002349B8`): sets `D_00160FE0 |= 0x1F` ("in flight" bits), computes size, complains if over `D_0016100C` (prints and drops the chain), appends the END tag, then **libdma**: `sceDmaGetChan(1)` (`func_001232E0`, returns the VIF1 channel register block `D_00132E70[1]`), sets chcr bits 0xC0, `FlushCache(0)`, `sceDmaSend` (`func_001235C8`: resets the channel record, sets `CHCR |= 0x105` = STR + chain mode + direction). So the actual CHCR/MADR/TADR/QWC writes are in libdma (C) and the library works through the channel struct, not by named constants.
- The VIF1 channel interrupt handler is hand-written asm [asm]: `func_00235118` (ASM_FUNC): reads `D1_CHCR` (0x10009000) and `D1_TADR` (0x10009030); it dispatches on bits 16-25 of the stopped tag: IRQ-tag codes 0x0201/0x0202/0x0204/0x0208 clear bit 1/2/3/4 of `D_00160FE0`; the END tag (ID 7 in the top bits) clears all of 0x1F. `VU1_syncChain(mask)` waits for `(D_00160FE0 & mask) == 0`. So **bit 0 = "chain finished"; bits 1-4 = fences that render code can wait on mid-chain**, set up by `Vif1ChainCmd(code)` [inferred from the code constants; the callers' masks are not enumerated]. It is installed by `DMAC_VIF1_Enable` (`func_00235018`: sets `D_STAT.CIM1`, `AddDmacHandler2(1, func_00235118)` and `(0xF, print_register_values_and_halt)`, `EnableDmac(1)`) and removed by `DMAC_VIF1_Disable` (`func_002350A8`).
- Errors: `func_00235218` prints `D1_CHCR`/`D1_TADR` and hangs.
- GIF channel (PATH3 / D2): direct kicks are only used by libgraph (`sceGsPutDrawEnv` `func_001224B0` writes `D2_QWC/MADR/CHCR` (0x1000A020/10/00); `sceGsExecLoadImage` `func_00122958`, `sceGsExecStoreImage` `func_00122AD8` (INCLUDE_ASM)). A source address in scratchpad is flagged by `(p & 0x70000000) == 0x70000000 -> madr = (p & 0x0FFFFFFF) | 0x80000000` (the SPR bit) [C].
- VU0 programs are loaded by `VU0_loadMicroProgram` (`func_002347F0`, C): waits on `D0_CHCR` (0x10008000) bit 8, writes 0 to 0x10008020 (QWC), `prog & 0x0FFFFFFF` to 0x10008030 (TADR), then `0x145` to CHCR (STR | chain | TTE | from-memory), then waits again. The C file names these `VIF0_STAT/FBRST/BASE`; they are DMA channel 0 registers [C].

### 2.3 Every place that touches EE hardware directly

174 distinct functions reference EE hardware registers or scratchpad addresses (core 51, exe game 77, level code 46); of those 103 are original asm or not decompiled (core 10, exe 56, level code 37). By subsystem (full address-by-address table in Appendix A):

| subsystem | registers | functions (file; status) |
|---|---|---|
| Timers | T0 0x10000000/10 (profiling counter; the title loop sets T0_MODE = 0x83, probably H-blank clocked [inferred]), **T1 0x10000800/10 (frame clock)**, T2 mode 0x10001810 | `func_00201E88` InitOnce (C), `func_0012F308` vsync handler (permcb.c, C), `func_L00_002465F8` (asm), `func_001F3D78` (asm), `func_001EBB48` (asm), `func_0011DE38` (kernel init, C), `partupd_*` (level C, writes T0 = 0) |
| INTC / vsync | `INTC_STAT` 0x1000F000 | `func_00118ED0`, `func_00118F60` (libkernel VSync/VSync2, C) |
| DMAC global | D_CTRL/STAT/PCR/SQWC/RBSR/RBOR 0x1000E000-50, D_ENABLER/W 0x1000F520/590 | libdma `func_00123308` sceDmaReset, `func_001233E8` sceDmaPutEnv, `func_00123650` sceDmaPause (C); `InitDma` `func_0020C268` (ASM_FUNC); `func_00235018/002350A8` (C); movie `initAll/termAll` |
| VIF1 ch1 | 0x10009000/30 | `func_00235118`, `func_00235218` (C), `func_00120858` (sceGsSyncPath, INCLUDE_ASM) |
| VIF0 ch0 | 0x10008000/20/30 | `VU0_loadMicroProgram` `func_002347F0` (C) |
| GIF / VIF1 control | GIF_CTRL 0x10003000, GIF_STAT 0x10003020, VIF1_STAT/FBRST/ERR 0x10003C00-20, VIF1_FIFO 0x10005000 | `func_001207B8` sceGsResetPath, `func_00120858` sceGsSyncPath (INCLUDE_ASM; called 20+7 times from game code) |
| GIF ch2 | 0x1000A000/10/20 | `func_001224B0`, `func_00122958` (C), `func_00120858` |
| GS privileged | PMODE 0x12000000, SMODE2 ..20, DISPFB1 ..70, DISPLAY1 ..80, DISPFB2 ..90, DISPLAY2 ..A0, EXTDATA ..C0, EXTWRITE ..D0, BGCOLOR ..E0, CSR 0x12001000, BUSDIR 0x12001040 | `func_00122140` sceGsPutDispEnv (C), `func_001F3D00` (C), `func_00121B78` sceGsResetGraph (C), `func_00122598` sceGsSyncV (C), `func_00122AD8` sceGsExecStoreImage (INCLUDE_ASM), `func_0023C7A8` movie vblank handler (ASM_FUNC) |
| fromSPR / toSPR ch8/9 | 0x1000D000.., 0x1000D400.. | `func_001299E8` (libmpeg, C), `func_001F9AF0` write_dma_channel (INCLUDE_ASM), `DmaToSpr/DmaToSprSync` `func_0020C210/230` (INCLUDE_ASM), `func_0020C468` WAD decompress (ASM_FUNC), `func_001EFE10` collision (ASM_FUNC), `func_001F91B8`, `func_00212578`, `func_00212658`, `func_00218B10`, `func_00229F00`, `func_002352C8`, `func_00236F00` (hand-written renderers), `func_L00_001FE688`, `func_L00_002535C8`, `func_L00_00254A70`, `func_L00_00257848`, `func_L02_002100E8` (level asm) |
| IPU and ch3/ch4 FIFOs | IPU_CMD/CTRL/BP/TOP 0x10002000-30, IPU in-FIFO 0x10007010, D3/D4 0x1000B000-30/B400-30 | libmpeg (`func_00127378`... `func_0012D068`, mostly C), movie `viBuf*` (`func_0023D090` ... `func_0023DCF0`, mostly INCLUDE_ASM) |
| SIF0 / SIF DMA | 0x1000C000, D_STAT | `func_0011A780` (libkernl sif, INCLUDE_ASM), `sceSifSetDma` syscall stubs |
| Scratchpad | 0x70000000-0x70003FFF | 101 functions; see 2.6 |
| Misc | 0x1000F130/F180 | `func_00119D88` (kernel TTY) |

Level code (`src/overlays`) never writes the chain pointer `D_00161000`: a search of every overlay assembly file finds 0 hits. Level code draws by calling the executable's packet helpers: `VU1_addGSregister` has 48 exe and 86 level-code callers, `SetupSkyGifPaging` 19 level callers, `HudSprite` 14, `DrawSpriteHelper_A` 27 [asm call graph].

### 2.4 2D and text path [C][asm]

HUD, menus, fades, rectangles, and text are written as GIF packets straight into the chain with VIF `DIRECT`, no VU1 program: `DrawTexturedQuad` (`func_001F5800`, C), `emit_rgba_draw_packet` (`func_001F55C0`, C), `DrawRectOverlay` (`func_001F5650`), `draw_hud_sprite_*`/`HudSprite` family (`func_00200468`..`func_002017C8`, C), `FadeToBlack`, `draw_letterbox_bars`, `draw_framebuffer_rect` (`func_001FBAB8`), `func_001FB908` (full-screen clear), `draw_bootImage`. A glyph is one textured sprite: `FontPrint` (`func_001F6668`, INCLUDE_ASM 640 B) and the C glyph emitter `func_001F69F0`, which calls `func_001F5BB8` (C, 680 B) once per glyph: one textured sprite as a GIF packet with `RGBAQ`, `UV`, `XYZ2` pairs (the sibling `DrawTexturedQuad` shows the shape: `p[3] = (v << 20) + s0; p[4] = x0 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32)`). Three font tables `D_001DF3D0/F770/FB10` and 3 text modes; `FontPrintWindow` (`func_001F7070`, INCLUDE_ASM 1,264 B) is the wrapped variant. The screen is addressed in GS fixed point with the viewport record `D_0013E600` (`x*16 + D_0013E600[4] - 8`).

### 2.5 The 3D world path (`DrawDebugProfiler` / "DrawWorld") [asm call order]

`func_001F3D78` (INCLUDE_ASM, 2,168 B; upstream names it `DrawDebugProfiler`, ReRAC `DrawWorld`) is the single world-render entry for gameplay (mode 0), cutscene (2), and (through the same pieces) the title world (`func_001EB7C0`). From its `jal` sequence, the passes run in this order: clear/setup (`framebuf_appendLargeSetup`), `build_rotation_view_proj`, `prune_moby_references`, `UpdateOcclusion`, `ResetGsRegisters`, sky (`Transition_DrawSky` / `SkyDrawShells`), **tfrags** (`DrawTfrag` `func_002346C0` -> hand-written `TfragProc` `func_002352C8`), IRQ fence (`func_00235290`), **ties** (`DrawTies_1/_2` `func_00236BE0/CA8` -> `TieProc` `func_00236F00`), GIF paging (`SetupGifPaging/DoGifPaging` `func_001F4630/4748`), draw-callback lists (`func_001F4A00/4A78/4AF0/4BB8`), **shrubs** (`DrawShrubs` `func_00229E50` -> `ShrubProc` `func_00229F00`), **mobys** (`DrawMobys` `func_0020E2B0` -> `MobyProc` `func_00212658`, `func_0020EF30`, `func_0020FC38`, `func_002108E0`, ...), **particles** (`PartProc` `func_00218B10`), light quads (`func_001F4C30`), lens flare (`func_001EDFF8`), then the 2D layer (HUD `HudDraw` `func_001FFFB8`, `draw_help` `func_001FF1B0`, subtitles `func_001F4F90`, letterbox/fade `func_001F5148`, `DrawScreenEffect` `func_001F5368`). The conditions around each call (which are skipped when) are not decoded. This agrees with ReRAC's render pipeline notes in order and naming.

Lighting is computed on the EE side with VU0 macro code before drawing: `LightTfrags` `func_002362B0`, `LightTies` `func_00238688`, `LightShrubs` `func_0022B8F8` (all ASM_FUNC).

### 2.6 Scratchpad, GS memory, render-to-texture, readback

- **Scratchpad** (16 KB at 0x70000000): 101 functions reference it. The hand-written renderers use 0x70003FA8-0x70003FF8 as a register save area, SPR ping-pong buffers around 0x70001000/0x70002000/0x70003000, a 0x380-byte per-class distance table at 0x70003A00 (`InitMobyClassDists/StashMobyClassDists/RestoreMobyClassDists`, C), and 0x70000000 + 0x40 x parent as the joint-matrix slots. Level C also uses it (`func_L00_002422D8`, 9,264 B, `ripple` code in `l01`, `drawquad_0021E3E8.c` in l09, and so on).
- **GS memory is addressed explicitly** for textures and render targets. Textures are uploaded by `upload_texture_images` (`func_00203958`, C) via `sceGsSetDefLoadImage` (INCLUDE_ASM) / `sceGsExecLoadImage`, from the level's "GS RAM table" (PSM, size, GS offset); tfrag/tie/shrub/moby texture tables patch `TEX0.TBP0` into prebuilt GIF packets at load (`PatchTfragGifs`, `PatchTieGifs`, `PatchShrubGifs`, `PatchMobyGifs`, all C; `tie_ad_gif_convert`, `shrub_class_init`, `RelocateTfrags` are INCLUDE_ASM). `Dma*Textures` (`DmaTfragTextures` etc., C) append REF packets for texture streaming.
- **Render-to-texture:** `func_001FB608` (C) builds a (1<<a) x (1<<b) render target at a GS address and a draw environment for it (used by effects and menus).
- **Frame-buffer readback and re-upload:** `DownloadFrameBuffer` `func_L00_002A21A8` and `UploadFrameSnapshot` `func_L00_002A2258` (both C, shared) copy the screen out and in with `sceGsSetDefStoreImage` + `sceGsExecStoreImage` and `sceGsSetDefLoadImage`; `GrabFrameSnapshot` `func_L00_001F9D40` (C) uses it for the pause/menu background. A host renderer needs a "download the colour buffer to a buffer in EE-style memory" operation.
- Sprites with alpha, `TEST_1`, `ALPHA_1`, scissor and fog colour are set with A+D register writes (`VU1_setScissor`, `ResetGsRegisters` writes `FOGCOL` 0x3D, etc.), so a renderer must honour per-packet GS state (alpha blend, depth test, scissor, texture, CLAMP, fog).

---

## 3. VU1 (and VU0) microprograms

### 3.1 Where they live and how they are uploaded [data][C][asm]

- The microcode is **only in the executable's `vutext` section** (0x100080-0x112380, 0x12300 bytes). A scan of the `lit`, `data` and `text` records of levels 0, 5 and 13 for the `FLUSH` + `MPG` VIFcode pair gives 0 hits. The decompressed per-level core data was not scanned, but the format docs describe its VIF lists as UNPACKs only. OpenRAC places all programs in the boot image; ReRAC says levels "may carry additional VU code in their .data; not yet checked". The scan found none in the code records, so there is probably **no per-level VU code**, with that one gap.
- Each VU1 program is stored as a ready-made DMA payload: a 16-byte header whose first halfword is the payload size in quadwords, then the payload itself (`FLUSH`, `MPG` blocks of up to 256 instructions with the program's data, further `MPG`s at consecutive addresses). Example [data, 0x10E800]: the header gives 0x128 quadwords and the payload begins with `FLUSH` and an `MPG` code. The code adds a program by `VU1_addDataRef(blob + 0x10, *(u16 *)blob)` = a REF tag in the VIF1 chain, so **uploading is part of the chain and happens on demand** (e.g. `func_001F7868`: `VU1_addDataRef(D_0010E810, D_0010E800)`).
- A resident-program id in `D_0015F704` avoids re-adding: written 7 by `func_001F7868` (sprite), 6 by `DrawMobysSetup` `func_0020E0C8` (moby), -1 and 8 by `func_001F3D78`/`func_001EB7C0`/transition code; the tfrag/tie/shrub/particle renderers add their programs themselves (they do not appear to use the id) [asm].
- VU0 programs are loaded through VIF0 DMA by `VU0_loadMicroProgram` (2.2); the blob is `DMAtag END` + `MPG` (e.g. the one at 0x100AE0).

### 3.2 The programs (PAL addresses) [data][asm]; roles per ReRAC where marked

The nine VU1 payload headers were found by scanning `vutext` for `FLUSH`+`MPG` after a tag quad, and their consumers by `%hi/%lo` references in the assembly.

| blob (header addr) | payload qwords | consumer(s) in PAL code | role |
|---|---:|---|---|
| 0x101070 | 0x6D | `PartProc` `func_00218B10`; `SkySpriteProc` `func_0022CEB8` | particles (ReRAC id 221571, 1 chunk); the sky sprite path reuses it [asm] |
| 0x101750 | 0xB7 | `ShrubProc` `func_00229F00` | shrub, first list (ReRAC 56467) |
| 0x1022D0 | 0x128 | `ShrubProc` | shrub, second list (ReRAC 912339) |
| 0x103560 | 0x3AA | `TfragProc` `func_002352C8` | tfrag main (ReRAC 55907, 8 chunks) |
| 0x107010 | 0x1DF | `TfragProc` | tfrag second/fallback (ReRAC 903379) |
| 0x108E10 | 0x1EF | `TieProc` `func_00236F00` | tie (ReRAC 13507) |
| 0x10AD10 | 0x37A | `TieProc` | tie, second program (ReRAC 224979) |
| 0x10E800 | 0x128 | `func_001F7868` (C), shared `func_L01_002BA150`, `func_L02_002A52D0`, `func_L09_002C2B08` (sprite helpers) | textured sprite/billboard, resident id 7 (ReRAC 57843) |
| 0x10FA90 | 0x287 | `DrawMobysSetup` `func_0020E0C8` (C), `func_001F3D78` | moby, resident id 6 (ReRAC 13859) |

That is **9 VU1 programs**; ReRAC counts 43 chunks of 0x800 bytes in 12 programs including three VU0 programs, which matches (PAL's ELF has no section table, so the 43 section names cannot be read from it). VU0 images:

| blob | loader | roles [asm: entry points seen in `vcallms`] |
|---|---|---|
| 0x100080 | `DrawMobysSetup` `func_0020E0C8` | ReRAC 104691, "moby-side VU0 helper" [ReRAC]. Which `vcallms` entry belongs to which image is not mapped; the entries seen are 0x0, 0x160, 0x2C0, 0x420 (below) |
| 0x100AE0 | `func_001E9EC8`, `func_001EB7C0`, `func_001F3D78`, `func_L00_002422D8` (level start, transition and world-render code) | ReRAC 436083, "end of frame / transitions"; ReRAC also ties `LightShrubs` to this program |
| 0x10E4C0 | `InitOnce` | "VU0 patch" placed at VU0 address 0xC80 (its VIFcode is `MPG num 0x5E, addr 0x190`): sine/cosine/Euler entries 0xC80, 0xC90, 0xD18 used by `FastCos` `func_001F9F90`, `FastSin` `func_001F9FA8`, `euler_to_matrix` `func_001FA218`, `func_001FA1F8`, `moby_build_rotation` `func_0020ED48`, and the shadow/sky code |

`vcallms` entry points by caller: 0x0 in `func_001EE9F8` (cmecpu.c), `func_0022B8F8`, `func_L00_001EEBD8`; 0x160 `func_0022B8F8` (`LightShrubs`); 0x2C0 and 0x420 `func_00238688` (`LightTies`); `vcallmsr $vi27` (indirect start address) in `func_001EE9F8` and `func_L00_001EF944`; 0xC80/0xC90 in `FastCos`/`FastSin`, `func_00229098`, `func_002291E8`, `func_0022CEB8`, `func_L01_002635C8`; 0xD18 in the Euler-matrix functions.

So even `FastSin`/`FastCos` (24 bytes each) run VU0 microcode (polynomial) rather than libm: a host port gets different low bits.

### 3.3 Which draw path selects which program; MSCAL/MSCNT

Selecting is simply "which blob the renderer adds to the chain" (table above). MSCAL/MSCNT VIFcodes that appear as immediates in code [asm]:

- `MSCAL 0` (0x14000000): shrubs `func_002298B0` (shadows, ASM_FUNC), moby `func_00212658`, `func_L00_002569F8` ("MobyProc"), `func_L01_00263338` (ripple).
- `MSCAL 6` (0x14000006): `TieProc` `func_00236F00` (two sites).
- `MSCAL 0xA`: moby `func_00212658` and `func_L00_00257848`.
- `MSCALF 0` (0x15000000): sprite/billboard (`func_001F7868`, `func_001F8B6C`, `func_L00_001FDE48`, `func_L01_002BA150`, `func_L01_0021F9C8`, ...).
- Tfrag, particles, and the other moby entries are not visible as immediates: the program start/continue codes are probably in the prebuilt packets in level data or the program blobs (not traced). `MSCNT` appears in no PAL function as a literal.

### 3.4 What is and is not known

- Known (retail, code): the nine VU1 blobs and their consumers, chain-based on-demand upload, the three VU0 images and entry points, the packet-level interface: **the vertex data reaches VU1 as UNPACKs from prebuilt level data** (tfrag/tie/shrub packets, moby index streams; layouts in docs/ASSETS.md and `tools/extract`), and the EE-built per-draw setup packets (camera matrices `0x6C0C43A4` etc. in `func_001F7868`, "TEX0 patched into GIF packets at load").
- Known by others [doc][ReRAC]: roles above; ReRAC lists five VU opcodes missing from OpenGOAL's tables (ITOF4, SUBA, MADDi, MADDAi, FMOR); programs are disassembled in ReRAC's `work/vu` (not in this repo).
- **Not known / not done:** the arithmetic of the VU1 programs in this repo (only the tfrag stream constants are confirmed, "not the VU program's arithmetic": docs/ASSETS.md), all per-program VU memory maps (ASSETS.md gives some: moby texcoords at VU address 0xC2, index stream at 0x12D), the exact MSCAL starts for tfrag/particles/most moby draws, and the meaning of ids 8/-1 of `D_0015F704`.

---

## 4. VU0 and other CPU-side specials in game code

Counts come from scanning mnemonics in every function's assembly (including functions that are already matched C, whose asm is the compiler's output).

### 4.1 COP2 (VU0 macro mode) [asm]

171 functions, 7,870 instructions, 120.6 KB use COP2 (`lqc2/sqc2/qmtc2/qmfc2/cfc2/ctc2` and the `v*` arithmetic, `vcallms`):

| directory | functions with COP2 | COP2 instructions | state |
|---|---:|---:|---|
| `src/core` | 11: libvu0 (`func_001252A0`, `125300`, `125358`, `125380`, `1253F8`, `1254A0`, `125548`, remnants `125340`, `1255F0`) and the libsn-style `sceGsResetPath`/`sceGsSyncPath` (`func_001207B8`, `func_00120858`; `ctc2/cfc2` only) | 95 | 3 ASM_FUNC, 6 INCLUDE_ASM, 2 remnants |
| `src/game` (exe) | 87 | 4,233 | 48 ASM_FUNC (hand-written), 39 INCLUDE_ASM |
| `src/overlays/shared` | 70 | 3,465 | 70 INCLUDE_ASM |
| `src/overlays/lNN_*` | 3 | 77 | 3 INCLUDE_ASM |
| **total** | **171** | **7,870** | **0 in compiled C** |

**No compiled C function contains a COP2 instruction.** Every use is original assembly or not yet decompiled, so a port must write replacements for roughly 171 functions, not just port C. Heaviest, by file: `mobyproc.c` (9 functions, 1,347 instr: `func_00212658` moby draw 253, `func_0020EF30` 287, `func_0020FC38` 249, `func_002108E0` 283, `func_00211808` animation eval chain 230, ...), shared `mobyproc_00251A78.c` (18 functions, 1,452 instr: `MobyProc` `func_L00_002569F8`, `MobyAnimEval` `func_L00_00252018`, ...), shared `effects_001EE2E0.c` (8 functions, 1,357 instr: the collision functions `coll_capsule` 324, `coll_sphere` 237, `coll_sphere_mobys` 170, ...), `collproc.c` `func_001EFE10` (293), `cmecpu.c` `func_001EE9F8` (229), `shadowproc.c` (6 functions, 500), `tieproc.c`/`shrubproc.c`/`tfragproc.c` (lighting and draw, 366/321/235), `drawquad.c`, `fastfunc.c` (45 functions, vector/matrix helpers, small), `partproc.c`, `skyproc.c`. So COP2 is used for **gameplay collision and animation as well as rendering**.

`vcallms`/`vcallmsr`: 15 functions run VU0 microprograms: `func_001EE9F8`, `func_001F9F90`, `func_001F9FA8`, `func_001FA1F8`, `func_001FA218`, `func_0020ED48`, `func_00229098`, `func_002291E8`, `func_0022B8F8`, `func_0022CEB8`, `func_00238688`, `func_L00_001EEBD8`, `func_L00_001EF944`, `func_L00_00251E30`, `func_L01_002635C8`. Entry addresses used: 0x0, 0x160, 0x2C0, 0x420, 0xC80, 0xC90, 0xD18 and `vcallmsr $vi27`. Uploads: 3.2.

### 4.2 MMI (128-bit integer) and 128-bit data [asm][C]

- Hand-written/undone code uses the real MMI set (`pextlh`, `pextuh`, `ppach`, `ppacb`, `psraw`, `pcpyld/pcpyud`, `pminw/pmaxw`, `padduw`, `qfsrv`, `prot3w`, ...): 107 functions (exe 41, core 28 (libmpeg/IPU, memcpy-class), shared 37, level 1); **none in compiled C**.
- Compiled C contains only: `por` (171 uses: the compiler's 128-bit register move / `por $2,$0,$0; sq` zeroing idiom), `madd` (13: EE three-operand multiply-add) and `mult1` (4). `lq`/`sq` with a non-stack base appear in about 770 compiled C functions (16-byte struct copies); callee-save spills (`sq $s0,...($sp)`) are everywhere.
- 128-bit in source: `typedef ... u128 __attribute__((mode(TI)))` in 111 files (694 mentions: core 1, game 6, shared 48, levels 57); and the sanctioned inline-asm helpers in `include/common.h`: `qcopy(dst, src)` = `lq $2,0(src); sq $2,0(dst)`, `qcopy_nc`, `qzero(p)` = `sq $0,0(p)`: **2,467 uses in 175 files** (levels 1,568, shared 796, game 103). These are the only inline MIPS asm in the matched code besides `__asm__ __volatile__("sync")` in two kernel functions.
- Other EE specifics in the generated code: `madd`/`mult1`, `pref` (92 sites), `sync`, `cache` in kernel stubs, `break 0,7` divide-by-zero guards after every `div` (1,162 `break`), 64-bit `ld/sd` for `long`, and the EE FPU (no NaN/Inf/denormals, truncating results) under the 47,000 `lwc1` float loads.

### 4.3 Hand-written assembly beyond COP2 [asm]

The 86 exe `ASM_FUNC`s (74.5 KB) are: the 9-10 renderers above (shrub 5,956 B, tie 5,332, moby 5,152 + 3,332 + 3,240 + 3,176 + 2,640, tfrag 3,112, particles 3,060), collision/CME (`func_001EFE10` 4,336 B, `func_001EE9F8` 4,984), lighting (3 funcs), shadow/sky sprite code, WAD decompression (`func_0020C468`, 720 B, uses scratchpad + D9), `InitDma`, `FastMemCopy`-class helpers (`func_001F99xx`), and the VIF1 interrupt handler. In core: about 70 `klib` syscall stubs, `FlushCache`, interrupt enable/disable, TLB exception handlers (`func_0011D700`), libvu0 rotations.

---

## 5. The path to the main menu

### 5.1 Where the menu code is, and which level's program is loaded [asm][C][ReRAC]

- **The title screen and main menu are in the boot executable, not in level code.** The title/front-end loop is `func_001EBB48` (`src/game/transition.c`), called directly from the boot stage before any level overlay has been loaded; `ParseBin` has not run yet. Its per-frame update is `func_001EB458`, its draw `func_001EB7C0`, and the page menu itself is `func_0021A1A0` (update, INCLUDE_ASM, 1,132 B) and `func_0021A610` (draw, INCLUDE_ASM, 1,732 B), both in `src/game/pause.c` (exe `0x219C08-0x228xxx`, about 60 KB of page-menu code) [call graph]. `func_001EB458` calls `func_0021A1A0`, `func_001FD3E8` (card dialog update), `sound_update` `func_0022DD68`; `func_001EB7C0` calls `func_0021A610`.
- The `src/overlays/shared/menu_00249720.c` and `pause_*.c` files are the *in-game* menus (pause/map/vendor: mode 3 = `func_L00_00277A88`). They are a different copy of similar functionality used while a level is loaded.
- **No level program is loaded for the title.** The "world" behind the menu is the global TOC group at offset 0x14E8 ("unknown_wad"), which `func_001EABE8` (INCLUDE_ASM, 1,812 B) reads (`lw ...0x14E8($17)`/`0x14EC`) and feeds to the *same* level-core loaders (`RelocateTfrags`, `shrub_class_init`, `tie_ad_gif_convert`, `register_moby_class`, `load_sky`, `upload_texture_images`, `ParseParticleTexs`). `func_001EB7C0` then draws it with the ordinary tfrag, tie, shrub, moby, particle and sky renderers (its callees include `DrawTfrag`, `DrawTies_1`, `DrawShrubs`, `DrawMobys`, `PartProc`, `Transition_DrawSky`, `ExecuteDrawCallbacks`). ReRAC independently describes this ("title world", lump `unknown_14e8.bin`) [ReRAC docs/plan/progression.md]. ReRAC's note that "the level loaded behind it is Novalis" is a statement about its own port, not about the retail disc; Nothing in the retail code loads a level for the front end.
- After "New Game"/"Load Game" the menu returns; the boot stage does `D_0015F6E4 = D_0015EE84; D_0015EE84 = -1; DoSpaceTransition()`: it loads the chosen level (new game = level 0, Veldin, after the story cards and movies; load = the saved level) and returns to `main`, which `ParseBin`s that level's overlay and enters its loop (section 1).

### 5.2 Boot-to-title call tree with status

(C = compiled and matched; **asm** = original asm kept; **UNDONE** = INCLUDE_ASM)

```
_start func_0012D868                                         asm
  main func_0012DB18 (src/core/boot.c)                       C
    __main func_0011DFC8                                     C
    func_001E99D8  boot stage ("startlevel")                 UNDONE 1,176 B
      InitOnce func_00201E88 (initonce.c)                    C
        sceGsResetPath func_001207B8                         UNDONE 160 B (COP2 ctc2/cfc2)
        sceDmaReset func_00123308                            C
        sceCdInit func_001211B0, sceCdDiskReady func_00121490   UNDONE 736 B, 504 B
        sceSifRebootIop("cdrom0:\IOPRP243.IMG") func_0011D248, sceSifSyncIop func_0011D210   C
        sceSifInitRpc func_0011AE20 (C), sceFsReset func_0011BF48 (C), sceSifInitIopHeap func_0011CB40 (UNDONE 136 B)
        sceCdMmode func_00121688                             C
        boot-info sector (LBA 0x121 = SYSTEM.CNF): wad_GetSectors func_0012F348 -> sceCdRead func_00121750 (UNDONE 480 B)
        PAL/NTSC flag D_0015EE80, save-folder name func_00209A60 (memcard.c)   C
        sceGsResetGraph func_00121B78                        C
        InitDma func_0020C268                                asm
        SetPalMode func_001F3890                             UNDONE 768 B
        VU0 patch program func_002347F0(0x10E4C0)            C
        TOC read func_0012F3F8 (6 sectors at LBA 1500)       C
        IOP module bundle: read 'irx' group (TOC 0x12C0), WAD decompress func_0020C468 (asm), then
          LoadIRXModule func_00201D58 x10 (C): sio2man, mcman, mcserv, Dbc_Manager, sio2d, pdman, IOP_stash_daemon, libsd, 989snd [data]
        init_pads func_00217EE8: sceDbcInit func_00124650, scePad2Init func_00124B88, scePad2CreateSocket func_00124BC8  C
        sceGsSyncVCallback(func_0012F308) func_00123168      C
        InitMemSlots func_00201E10; memcard_Init func_0020BAA8 (sceMcInit)   C
        InitViewContext func_001F3008 (C), UpdateViewContext func_001F3140 (UNDONE 1,564 B)
        VU1_initChain func_002348E8 (C)
        SoundMasterVolumeInit func_0022DBE8 (UNDONE 380 B) -> snd_StartSoundSystem func_0012DB68 (UNDONE 600 B)
        LoadDebugFont func_001E96B8 (C) -> Load() func_002176C8 (C)
        Stash init func_00233FF8 (C), func_001F7BF8 (C), sceScfGetLanguage func_0012D380 (C) -> language D_0015EE88
      DMAC_VIF1_Enable func_00235018 (C); ResetGsRegisters func_001F3C10 / Pr func_001F3D00 (C); PutDispBuffer func_001FB470 (C)
      card check func_00209BB8 (C)  + warning image  func_001F7680 (UNDONE 488 B), HudSprite family (C)
      logos movie func_0023B670 (C) -> movie.c: func_0023B740 (UNDONE 1,024 B), initAll func_0023BB90 (UNDONE 676 B),
          libmpeg (mostly C), IPU/viBuf (mostly UNDONE), threads/semaphores
      FadeToBlack func_001F4E08 (C); sound bank func_0022EA20 (C); snd_ResolveBankXREFS (C)
      title loop func_001EBB48                               UNDONE 1,256 B
        func_00209DC0 (C): default save template from TOC 'save_game' (D_00137C80[4..5])
        func_001EABE8: load title world (TOC 0x14E8)         UNDONE 1,812 B
        Load() x5 TOC 0x1500..0x1524 = HUD banks, +0x14F8 hud header (C); init_hud func_001FF6B8 (C); LoadHudBanks func_002032D0 (C)
        per frame:  VU1_sendChain/swap/PutDrawBuffer*, memcard driver func_00209E68 (C), func_00209070 (C), UpdatePad func_00218908 (C),
                    Help_LoadMsgs func_001EB300 (C), func_001EB458 update (UNDONE 868 B) -> page menu func_0021A1A0 (UNDONE),
                    func_001EB7C0 draw (C) -> world renderers + func_0021A610 page-menu draw (UNDONE 1,732 B),
                    VU1_syncChain, sceGsSyncV
      DoSpaceTransition func_00233308 (C) -> level streaming func_00204C60 (UNDONE 864 B), func_00232278 (UNDONE 1,704 B) ...
  ParseBin func_0012DA38 (C) -> level entry func_L00_002465F8 (UNDONE 2,248 B)
```

### 5.3 What the menu draws with [asm][C]

- A **3D background**: the title world through the full world renderer stack (sky, tfrag, tie, shrub, moby, particles) plus mobys for the hero/NPCs (`Transition_UpdateMovieCamera` `func_001EB338`, C, drives a scripted camera; `update_moby_animation_state`, `moby_build_rotation`).
- **2D quads**: logo, "PRESS START", page frames, list widgets and cursor (`HudSprite` family, `DrawTexturedQuad`, `draw_stretchable_ui_frame`, `draw_framed_text`, `draw_menu_selection_marker`, all C).
- **Text** through `FontPrint` / `FontPrintWindow` (UNDONE) with the three font tables and the localised all-text bank (`Help_LoadMsgs`, TOC 0x1528).
- Movies (logos, attract) through libmpeg + IPU.

### 5.4 Blockers: unmatched functions reachable from boot

Computed as the transitive closure of `jal` targets plus address-taken function references (`%hi/%lo(func_..)`) from `main` and `func_001E99D8`, restricted to executable functions (a level overlay is not loaded yet). This is an **upper bound**: address-taken references pull in everything reachable through the large dispatch structures. Result: 1,088 functions, of which 167 are `INCLUDE_ASM` larger than a 4-byte fragment (94.8 KB; 172 counting fragments) and 81 exe + 70 core `ASM_FUNC` (original asm, 70.5 KB + 4.1 KB). Restricting to the title loop `func_001EBB48`: 836 functions, of which 128 are UNDONE (73.8 KB). Appendix C lists all 167 with sizes (`T` marks the title-loop subset, `V` COP2, `M` real MMI). The ones verified to be directly on the path:

- Boot: `func_001E99D8` (1,176), `func_001207B8`/`func_00120858` (sceGsResetPath/SyncPath), `func_001211B0`, `func_00121490`, `func_00121750` (sceCd*), `func_001F3890` SetPalMode, `func_001F3140` UpdateViewContext, `func_0012DB68`/`func_0022DBE8` (sound start), `func_0011B4C8` (sceSifCallRpc, 492 B), `func_0011B868`, `func_0011BCB0`, `func_0011A0A0` (printf core, 1,516 B), `func_0011A780` (sif init/exit).
- Title: `func_001EBB48`, `func_001EB458`, `func_001EABE8`, `func_001E9EC8` (3,356 B, level-init), `func_0021A1A0`, `func_0021A610`, `func_001FD3E8`, `func_001FBE80`, `func_001F6668`/`func_001F7070` (FontPrint), `func_001F4868` (GetEffectTex), `func_00200248` (GetFrameTex), `func_001FF958`, `func_001FAB40` (`SetupFS_AA_buffer`, 2,308 B), `func_001F7680`.
- World renderers: the nine hand-written renderers are *ASM_FUNC* (counted finished but not C): `TfragProc`, `TieProc`, `ShrubProc`, `MobyProc` and friends, `PartProc`, `LightTfrags/Ties/Shrubs`, shadow/sky sprite code; the load-time `shrub_class_init`, `tie_ad_gif_convert`, `RelocateTfrags`, `MobyClassRelocate`, `BuildMobyAdGif` and `select_world_object_resource_tables` are UNDONE.
- Audio: `sound_update` `func_0022DD68` (3,252 B), `music_Update` `func_00217130` (1,108 B), `SoundSlotAlloc`, and the 989snd RPC layer `func_0012E820`, `func_0012EB18`.
- Input: `func_002181F0` (pad.c, 1,816 B).

---

## 6. Files and discs

### 6.1 What the game reads and how [C][asm][data]

- ISO9660 is touched only for `SYSTEM.CNF`, `SCES_509.16` (by the BIOS) and `IOPRP243.IMG` (the IOP is rebooted with `cdrom0:\IOPRP243.IMG`; `func_0011D248` = `sceSifRebootIop`). `InitOnce` reads the sector at LBA 0x121 (289 = `SYSTEM.CNF`) raw and tests byte 0x33 against 'N' to set the PAL/NTSC flag. **Everything else is addressed by absolute 2048-byte sector (LBA)** through tables read at boot (docs/ASSETS.md):
  - Table of contents: `func_0012F3F8` reads 6 sectors at LBA 1500 and keeps 0x2960 bytes in `D_00137C80` (groups of `(LBA, size)`, see ASSETS.md "Table of contents groups"; offsets like 0x14E8 title world, 0x1500 HUD banks, 0x12C0 IOP modules, 0x1528 all text, 0x17F8 MPEG list, 0x1AB8 help audio LBAs).
  - Level table at +0x28C8: 19 `(header LBA, sectors)`; `func_0012F4A8(level)` reads the 0x2434-byte header (5 sectors) into `D_0013A5E0`, which gives the sector ranges for level data, NTSC gameplay, PAL gameplay, occlusion, plus music LBAs and scene audio/WAD LBAs.
- Primitive calls (all through libcdvd, `src/core/001208E8.c` and `00121750.c`):
  - `sceCdInit`, `sceCdDiskReady`, `sceCdMmode(2)` (DVD) at init (UNDONE: `func_001211B0`, `func_00121490`).
  - **`sceCdRead(lba, nsectors, dst, mode)`** `func_00121750` (UNDONE, 480 B). `mode` = 4 bytes `{0x20 (trycount), D_0015EE58[0] (spindle ctrl), 0, 0}`.
  - `sceCdSync(1)` `func_00120F30` as a polled "is busy" check (10 exe sites); `sceCdGetError` `func_00121930` (retry loop if non-zero); `sceCdBreak` `func_001219C8`; `sceCdReadClock` `func_00121A80` (timestamps for saves).
- Wrappers: `wad_GetSectors(lba, n, buf)` `func_0012F348` (retry loop; 2 sites, InitOnce); `submit_audio_stream_io_request` `func_002175C8` (stream.c, the common request used by level streaming); `start_audio_stream_read` `func_00217628` -> `snd_StreamSafeCdRead` `func_0012EE98` (the IOP-aware read); **`Load(dst, lba, nsectors)` `func_002176C8`** = request + pump loop `{sceGsSyncV, music_Update, snd flush...}` until done (8 callers: debug font, title loop, memcard, `func_L00_00246EC0`, ...); movie streaming `func_0023CE30` (`strfile.c`) calls `sceCdRead` directly.
- The big loads run as a state machine: `func_00204C60` (UNDONE, 864 B) polls `sceCdSync`/`sceCdGetError`/`sceCdBreak`, loads sound banks (`snd_BankLoadFromEE_CB`), and the level core is WAD-decompressed (`func_0020C468`, hand-written, DMA-assisted) and relocated by the `loaders.c` functions. Level program overlays (section 0.1) are copied by `ParseBin`.
- Compressed groups use "WAD": a 16-byte header, then an LZO-like stream with 0x2000-byte block refills through scratchpad (docs/ASSETS.md "WAD compression").
- **Audio streaming is done by the IOP, not the EE:** `snd_PlayVAGStreamByLocEx_CB` (`func_0012ED48`, 11 exe sites) and the music code pass LBAs to the 989snd IOP module, which reads the disc itself. Music LBAs are in the level header (+0x148). The movie player reads PSS/MPEG data from the sector list at TOC 0x17F8 (byte sizes) itself.
- Other file-like calls: `sceOpen/sceClose/sceLseek/sceRead/sceWrite` (libkernl fileio RPC): only debug code (`func_001F1088`, `func_L00_001F4918`), libscf (reads `rom0:` config for language/clock), and libgraph's model check (`func_00121D18`). No `host:`/`cdrom0:` file reads of game assets.

### 6.2 What a host file layer has to answer

1. `read(lba, nsectors) -> bytes` on the 2048-byte-sector disc image (a single mmap'd ISO is enough) and an immediate-or-polled completion: `sceCdSync`, `sceCdGetError == 0`. The EE-side callers are `sceCdRead` (UNDONE) and `wad_GetSectors`; everything above them takes LBAs.
2. For boot: sector 0x121 raw (region), `IOPRP243.IMG`/IOP bundle (not needed if IOP is replaced), TOC at LBA 1500, level table and level headers, then the level's four ranges.
3. A replacement for the IOP-side streaming API (989snd): LBA-addressed VAG streams and music; `rom0:` language/clock queries.
4. Movie sectors for the PSS/MPEG files (byte-size entries at TOC 0x17F8), or a pre-extracted file mapping.

---

## 7. Memory card

### 7.1 Layers [C]

All of these are **matched C** except the UI dialogs:

1. **libmc** (`src/core/001236F0.c`, Sony 2.9-ee, exact; names from `libmc.a` by size and order): `sceMcInit` `func_001236F0`, `sceMcOpen` 1238B0, `sceMcMkdir` 1239D8, `sceMcClose` 123A10, `sceMcSeek` 123AC8, `sceMcRead` 123C30, `sceMcWrite` 123D48, `sceMcSync` 123F30, `sceMcGetInfo` 124068, `sceMcGetDir` 1241F0, `sceMcFormat` 124340, `sceMcDelete` 124410, `sceMcUnformat` 124528 (the last two names by archive order; the dead-stripped others are absent). All game call sites are inside `func_00209E68` (plus `func_00209BB8`, `sceMcInit` in `memcard_Init`). The IOP side is `mcman` + `mcserv` (loaded by `InitOnce`).
2. **Driver** `func_00209E68` (`memcard.c`, 7,064 B): non-blocking: each call either polls `sceMcSync(1,...)` (if the previous command is in flight) or issues one command according to `state`. State 0: `sceMcGetInfo(port, slot, &type, &free, &format)`; 1: store result/`request`; 2: idle, pick up queued requests (`D_0013D390+0xDC`); 3-4: `sceMcFormat`; 5-6: the second maintenance call; 7: `sceMcGetDir` of the game's folder; 8 (substates 0-5): list/validate the folder, `sceMcOpen(...,1)`, `sceMcRead` an 8-byte header (two descriptor sizes), compare them with this build's `memcard_GetDataSize`, `sceMcClose` (load path); 9 idle; 10 (substates 0-21): save path: `sceMcMkdir`, `sceMcOpen(..., 0x203)` for `icon.sys`, the icon, `save%d.bin`, `sceMcWrite` each, `sceMcClose`, compute sizes; 11-12: `sceMcGetDir` + `sceMcDelete` (delete save/folder). Results are reported in the `err` field of the state struct (`D_0013D390`): 1-0x12 and 0x2710-0x271A. The structs and 25-state naming were adapted from Lombyte; a single card (`card[1]`, port 0, slot 0 = zero-initialised bss) is polled.
3. **Card status monitor** `func_00209070` (`menu.c`): `D_0015EFB0` = card state, 25 handlers in the table `D_001A0538` (all matched, `menu.c`), whose names in the data are `CS_INIT, CS_GOOD_SAVE, CS_WARNING, CS_NOCARD, CS_WAIT_FOR_CARD, CS_UNFORMATTED, CS_PROMPT_FORMAT, CS_FORMAT_PENDING, CS_FORMATTING, CS_FORMATTED, CS_CHECK_SAVE, CS_CHECKING_SAVE, CS_NOSAVE, CS_PROMPT_CREATE_SAVE, CS_CREATE_SAVE_PENDING, CS_CREATING_SAVE, CS_NEWCARD, CS_FORMAT_FAILED, CS_CREATE_FAILED, CS_NO_ROOM, CS_LOAD_FAILED, CS_SAVE_FAILED, CS_SAVING, CS_PROMPT_BEGIN_UNFORMATTED, CS_PROMPT_BEGIN_NOSAVE` [data strings at 0x15FE78 and 0x1E83F0]. `D_0015EFB4` carries request bits (0x80 = save failed, 0x100 = load failed, 0x40, 0x200).
4. **Save API:** `memcard_Save(slot, level)` `func_0020BFC8` (C), `memcard_MakeWholeSave` `func_0020BA00`, `memcard_RestoreGame` `func_00209CE8`, `memcard_PrepData` `func_0020BBC8`, `memcard_RestoreData` `func_0020BD70`, `memcard_RestoreInfo` `func_0020BCB0`, `memcard_Checksum` `func_0020BB10` (16-bit shift-register, polynomial 0x1F45, returns 0 for blocks over 0x1800 bytes), `memcard_TestChecksum` `func_0020BB88`, `memcard_GetDataSize` `func_0020BAD8`. `memcard_Save` stamps the slot with the clock (`sceCdReadClock` then `sceScfGetLocalTimefromRTC` into `D_0015EF98`, 8 bytes).
5. **UI:** the card dialogs (mode 4, "freeze") `UpdateModeFreeze` `func_001FD3E8` (2,856 B) and draw `func_001FBE80` (5,480 B) are INCLUDE_ASM; the save/load pages are in the pause menu (`func_00227DB0` 860 B UNDONE, `func_00227C78/D20` C, `func_00226D50` UNDONE).

### 7.2 Save data layout [C][data]

- Folder: template `"/BA****-*****RATCHET"` (`D_0013D2D0`) filled from the boot info (`func_00209A60`): `B A` + region letter (E or `P`->`I`) + serial digits from the SYSTEM.CNF boot line; ReRAC gives the US one as `BASCUS-97199RATCHET`, so PAL should be `BASCES-50916RATCHET` [inferred]. File name templates in data: `/<folder>/*` (dir listing), `/<folder>/icon.sys`, `/<folder>/static.ico`, `/<folder>/save%d.bin`, and a folder-named placeholder (ReRAC: 0x3C04-byte zero-filled reservation).
- `save%d.bin` (up to 5 slots; `D_0013D390` keeps per-slot name, bolts, level and play time for the load menu): `[int sizeA][int sizeB]` then one **global block** described by table `D_001A05C0` then **20 per-level blocks** described by `D_001A08C0` (indexed by level, 19 levels + 1). Each block: `{int size; int checksum16}` then records `{int id; int size; bytes (4-aligned)}` terminated by `{-1, 0}`. A descriptor is `{pointer to the live variable, byte size, id, 0}`; the per-level descriptor adds `slot * size` to its base (e.g. `D_0013DE60` visited flags id 0xBB9). The global table covers the current level `D_0015EE84`, the bolt count `D_0015EE98` (`gBolts`), `D_0015EF20`, play time `D_0015EF24`, the 8-byte save stamp `D_0015EF98`, `D_0013D490` (0x80 B), `D_0015EEB0` (12 B), `D_0013D510` (0x20), `D_0013D530` (0x94), `D_0013D5C8` (0x25), ... (the full list is the `.word` table in `asm/data/data.data.s` at `D_001A05C0`).
- DATAMINED_REVERSE_ENGINEERING.md lists block ids attributed to `func_0020BBC8`'s callers.

### 7.3 Native replacement

The whole stack above `libmc` is host-independent C. What must be provided is the libmc surface (13 functions, 38 game call sites, all `async + sceMcSync(1, &cmd, &result)`), or a replacement of `func_00209E68`'s 13 call sites by synchronous file operations in a host directory. ReRAC implements exactly this ("the native calls complete at once", folder `<config>/rerac/memcard/BASCUS-97199RATCHET/`).

---

## 8. Other Sony library surface

Counts are call sites (`jal`/`j` instructions) in the 1,036 executable game functions ("exe") and in the distinct level functions (3,431 function files; a function shared by 19 levels counts once) ("level"). Function names for library code come from `src/core` notes, `config/names.tsv`, and a match of function sizes and order against the SDK archives in `toolchain/sn-prodg-24/local/sce/ee/lib/*.a` (symbol table, not code); ambiguous ones are marked. Calls through function pointers (125 `jalr` sites in the whole program) are not counted. Appendix B is the complete per-function list.

| library (retail address range) | entry points the game calls | exe / level sites | decompiled in `src/core`? |
|---|---:|---:|---|
| newlib libc (0x112380-0x118A40) | 13: memcpy/memset/memcmp/strlen/strcpy/strncpy/strcmp/strncmp/strchr (INCLUDE_ASM), sprintf, rand, srand, strstr (C) | 112 / 257 | partly: libc string asm not decompiled |
| libkernl: syscall stubs (0x118A40-0x118EB0), intr/thread (0x119328), sif/fileio/RPC (0x11A0A0-0x11D960) | 36: FlushCache 53+15, Add/RemoveIntcHandler, Add/RemoveDmacHandler, Enable/DisableDmac, CreateThread/DeleteThread/StartThread/TerminateThread, ChangeThreadPriority, GetThreadId, Create/Delete/Signal/WaitSema, sceSifSetDma/DmaStat, sceSifInitRpc/BindRpc/CallRpc, fileio, sceSifRebootIop/SyncIop/LoadModuleBuffer, IOP heap, DIntr/EIntr | 128 / 34 | stubs are original asm (counted finished); RPC core `sceSifCallRpc` and `sceSifInitIopHeap` not decompiled |
| libgcc (soft float, 64-bit div) | `__extendsfdf2` 6 / 56 sites, others internal | | C (exact) |
| libsn `vu.o` look-alike (0x1207B8) | `sceGsResetPath`, `sceGsSyncPath`: 4+16 exe, 0+7 level | 20 / 7 | INCLUDE_ASM (both) |
| libcdvd (0x1208E8-0x121B78) | `sceCdSync` 10+1, `sceCdInit`, `sceCdDiskReady` 5, `sceCdMmode`, `sceCdRead` 2, `sceCdGetError`, `sceCdBreak`, `sceCdReadClock` 4 | 26 / 1 | `Init`/`DiskReady`/`Read` INCLUDE_ASM, rest C |
| libgraph (0x121B78-0x123168) | `sceGsResetGraph` 4, `sceGsSetDefDispEnv`, `sceGsPutDispEnv`, `sceGsSetDefDrawEnv` 3+2, `sceGsPutDrawEnv`, **`sceGsSyncV` 24+12**, `sceGsSetDefLoadImage` 18+7, `sceGsExecLoadImage` 10+3, `sceGsSetDefStoreImage`, `sceGsExecStoreImage`, `sceGsSyncVCallback` 3+1 | 62 / 31 (+4 / 1 for SyncVCallback) | mixed: Def*Image, DefDispEnv, StoreImage INCLUDE_ASM |
| libdma (0x123168-0x1236F0) | `sceDmaGetChan`, `sceDmaReset`, `sceDmaSend` (+ internal PutEnv, Pause) | 7 / 0 | C |
| libmc (0x1236F0-0x1245F8) | 13 (section 7) | 38 / 0 | C (exact) |
| libdbc / libpad2 / libvib (0x1245F8-0x1252A0) | `sceDbcInit`, `scePad2Init/CreateSocket/Read/GetButtonProfile/GetState`, `sceVibGetProfile` (all from `init_pads` and the pad reader) | 7 / 0 | C (exact; `sceVibSetActParam` dead-stripped) |
| libvu0 (0x1252A0-0x125630) | `RotMatrixX/Y/Z`, UnitMatrix-class helpers, 2 internal entries | 16 / 34 (the level count includes two mid-function entry points with 22 and 4 level callers) | asm / INCLUDE_ASM |
| libmpeg + libipu + IPU DMA (0x125630-0x12D2A0) | 10 (`sceMpegInit`, `Create`, `Delete`, `GetPicture`, `AddCallback`, `AddStrCallback`, `Reset`, `DemuxPssRing`, ...) | 14 / 0 | C mostly; decoder core `func_00125630..` UNDONE (9 functions, 6 KB) |
| libscf (0x12D2A0-0x12D868) | `sceScfGetLanguage`, `sceScfGetLocalTimefromRTC` | 5 / 0 | C |
| 989snd (0x12DB68-0x12F308) | 42 (`snd_FlushSoundCommands` 23+7, `snd_SetMasterVolume` 16, `snd_PlayVAGStreamByLocEx_CB` 11, bank load/unload, VAG streams, movie sound, reverb, doppler) | 128 / 15 | C except `snd_StartSoundSystem`, `snd_SendIOPCommandNoWait`, `snd_SendCurrentBatch` (RPC layer, UNDONE) |
| game `wad.c`/`permcb.c` (0x12F308-) | `wad_GetSectors`, TOC and level header reads | 5 / 0 | C |

By functional area:

- **Pad:** libpad2 over the IOP `PsIIpdman`/`dbcman`/`sio2man` modules. `init_pads` (`func_00217EE8`, C) = `sceDbcInit`, `scePad2Init(0)`, `scePad2CreateSocket(...)`; the per-frame reader `func_00217F68` (C, 540 B) calls `scePad2Read`/`GetState`/`GetButtonProfile`/`sceVibGetProfile` and fills a 16 x 0x330-byte `pad2_info` table `D_0015B640`/`D_0013CA40`; `UpdatePad` `func_00218908`; `func_002181F0` (UNDONE, 1,816 B) post-processes. Vibration is computed in `actuator.c` (C). Single caller each: this is a thin, easily replaced layer, but `func_002181F0` is open.
- **Sound and IOP modules:** all sound goes through the 989 Studios library `989snd.c` (C, ~42 entry points) over SIF RPC to the IOP module `989snd_Library` + `Sound_Device_Library` (libsd). The RPC layer is `sceSifCallRpc` (UNDONE) with batching in `snd_SendCurrentBatch`. A native build should replace the **`snd_*` API** (and the `sound.c`/`music.c`/`stream.c` layer above it) with a host mixer; it is the cleanest cut. The bank file formats are the 989 `.bnk` containers (level header "sound bank", TOC 0x14E0).
- **IOP modules loaded by `InitOnce`** [data: names inside the decompressed bundle at TOC 0x12C0]: `sio2man`, `mcman`, `mcserv`, `Dbc_Manager` (dbcman), `PsIIsio2d` (sio2d), `PsIIpdman` (pdman), `IOP_stash_daemon` (the game's own "stash" daemon used by `Stash_SendData` `func_00234158`/`func_L00_00295010` to ship data EE to IOP via `sceSifSetDma`), `libsd`, `989snd`. The IOP image `IOPRP243.IMG` (rebooted at startup) supplies cdvd, sifcmd, etc.
- **Threads, semaphores, interrupts:** the game is single-threaded and polls. Threads/semaphores/Intc handlers are used only by the movie player (`movie.c` `initAll`/`termAll`: 1 CreateThread, 1 StartThread, 1 ChangeThreadPriority, 9 `SignalSema` + 9 `WaitSema` in `vibuf.c`, Add/RemoveIntcHandler for the IPU/vblank) and the VIF1 DMAC handler. The only game interrupt callbacks are `func_0012F308` (vsync), `func_00235118` (VIF1 DMAC), `func_0023C7A8` (movie vblank).
- **Timers:** T1 as the frame clock (every loop), T0 as a profiling counter (written 0 in `partupd_*`, 4 uses), T2 mode read in kernel init.
- **cdvd:** section 6. **sif/RPC:** `sceSifInitRpc` (2 sites), `sceSifBindRpc`, `sceSifCallRpc`, `sceSifSetDma` (3+1: `LoadIRXModule`, `Stash_SendData`, movie ADPCM), `sceSifDmaStat` (2). **graph, dma, mc:** above.
- **Kernel misc:** `FlushCache` is called at 68 sites (53 exe + 15 level) and must remain a no-op that keeps order; `EnableCache`; `GetMemorySize`-class stubs only inside the kernel.

---

## 9. Portability hazards in the C as written

Counts are regex-based over `src/**/*.c` (excluding libgcc unless noted); they overcount slightly where one expression matches two patterns, so read them as order of magnitude.

### 9.1 Pointers, integers and absolute addresses

| hazard | count | where | examples |
|---|---:|---|---|
| Integer-typed pointers and `(int)ptr` casts | ~350 `(int)&x` / `(int)ptr`-style casts, and an extremely common idiom of `int` globals/fields holding addresses | core 32, game 41, shared 56, levels 218 | `D_00161000[1] = (int)data;` (`vuchain.c`), `D_00161004 = (int)D_00161000;`, `func_0012E318(*(int *)(D_0015F714 + 0x1C))` (`space.c`, `DoSpaceTransition`), `(unsigned int)&D_00154A10 | 0x20000000` (`00119328.c:206`; uncached alias) |
| Struct access by byte offset: `*(T *)(p + 0xNN)` | ~29,000 (levels 18,126, shared 7,652, game 1,325, core 412) | everywhere; `char *p` + hex offset instead of structs | `*(short *)(moby + 0xA6)`, `*(char **)(p + 0x10) = D_001A2EF0[...]` (`missionfunc.c:155`) |
| Fixed absolute hardware addresses | ~200 sites (core 177) | libs and `draw.c` | `*(volatile int *)0x1000E010 = 0x20000;` (`vuchain.c`), `*(volatile long *)0x120000E0 = 0;` (`draw.c:673`) |
| Fixed scratchpad addresses | 40 sites in 9 files (+ ~100 asm functions) | menu.c, loaders, drawquad, ripple | `FastMemCopy(D_001B3200, (void *)0x70003A00, 0x380)` (`draw.c`, mirrored in `mobyfunc.c`), `(unsigned char *)0x70002000` (`menu.c:758`), `(int *)0x70000000` (`l09_gaspar/drawquad_0021E3E8.c:35`) |
| Other fixed addresses | ~250 | InitOnce etc. | `D_1FF8000 - (D_00137C80[0x4B1] << 0xB)`, `(D_0024272F & 0xFFFFC000)` (`initonce.c`) |
| **Global + large offset** (one global used as the start of a big contiguous block) | **3,859 sites; 3,050 with offset >= 0x400** | levels 2,586, shared 1,126, game 130 | `D_0013E633 + 0xE1D` (2,858 uses; the hero/game-state block), `D_0014171B + 0x65`, `D_0013F450 + 0x...` (198), `D_0013A5E0 + 0x2460` (75). These rely on the exact adjacency of formerly separate globals in the retail image. |
| Tables of pointers in data | `.word D_/func_` in `asm/data/*.s` (506 function pointers in `data.data.s`, 23 in `core_data`, 12 in rodata) and the level `vtbl`/`camvtbl`/`sndvtbl` records | | `D_001A05C0` save descriptor table; `D_001A0538` card-state handler table; dispatch tables `D_001E8F80` |
| `$gp`-relative small data | thousands of accesses; the C marks them with the attributes below | | `MACRO_ADDR` (2,222 uses), `NOT_SDA` (2,256), `SDATA` (202): all `__attribute__((section(".sdata")))`/`.data` and symbol-alias tricks to reproduce retail's addressing modes |

Consequences: the dominant pattern is "a global label plus a hex offset" and "a `char *` plus a hex offset". These are fine on a 64-bit host **only if the data keep their retail layout and every pointer stored inside that data stays 4 bytes**. The save descriptors (`D_001A05C0`: pointer + size) and the dispatch/vtable records (4-byte function pointers in data) are such places.

### 9.2 Word sizes

- Pointers and `int` are 32 bits in the assumptions; `long` is 64 bits (`typedef long s64; typedef unsigned long u64;` in `include/common.h`). `long` / `u64` / `s64` appear ~1,770 times (game 779, core 501, levels 243, shared 178), e.g. all GS register data (`VU1_addGSregister(int, long)`), `*(long *)((char *)D_00161000 + 0x20) = arg1;`, `(long)x0 | ((long)x1 << 16) | ((long)y0 << 32) | ((long)y1 << 48)` (`vuchain.c`, scissor). Fine on LP64 Linux/macOS, **wrong on Windows (LLP64)**.
- `u128` via `__attribute__((mode(TI)))` (GCC extension; clang supports `__int128` on LP64 targets but not on 32-bit or MSVC) and the inline asm `qcopy/qzero` (MIPS `lq/sq`): 111 and 175 files respectively. They can be redefined as `memcpy(dst, src, 16)` / `memset(p, 0, 16)` in one header, which also removes ~2,500 uses at once.
- Division by zero traps (`break 0,7` after every `div`) and the R5900 FPU (no denormals/NaN, truncating arithmetic) are baked into the codegen; behaviour differs in rare cases on IEEE hardware.

### 9.3 Compiler-specific constructs

- `__asm__("func_XXXX")` / `__asm__("D_XXXX")` symbol aliases to rename or retype retail symbols: 2,174 uses in 246 files (declared in the C as `extern long func_00116108_wide(...) __asm__("func_00116108")`). On a host these must become real `#define` or wrapper names.
- `INCLUDE_ASM` (1,519 stubs: core 96, game 167, overlays 1,256), `ASM_FUNC` (212), `LINKER_REMNANT` (229): macros that `.include` generated MIPS assembly; `__asm__(".section .text\n\tnop\n")` padding in 24 files (45 sites). All of these are nonsense to a host compiler and are *missing code*, not just syntax.
- Section attributes (`NOT_SDA`, `MACRO_ADDR`) are invalid on Mach-O (`section("...")` needs `segment,section`): define them empty for a host build.
- Retail-matching idioms that a modern compiler may optimise differently (uninitialised locals, strict aliasing violations via `*(float *)&D_x`, pointer/int punning, signed overflow): use `-fno-strict-aliasing -fwrapv` and expect to hit UB in places [inferred]. `char` is signed in the MIPS ABI (also on Apple arm64, unsigned on Linux/ARM and Windows/ARM): force `-fsigned-char`.

### 9.4 Level code at fixed addresses, called through pointers in data

- Level programs are linked at slightly different addresses per level but all expect to **overwrite the same data/bss/lit region (0x15F000...)** and the same `$gp` (0x166D00); code finds data by absolute address. The executable and every level share the *same function bodies* at different addresses (differences are relocated fields only; "shared code differs between levels only in relocated fields (20 functions compared)": docs/OVERLAYS.md). That is a favourable property: a host build compiles the common engine once and a small per-level module (~107 KB of distinct code per level, 19 levels), if the per-level data (lit/bss/data records: 44-84 % shared bytes) can be reproduced as C data.
- Dispatch through data: `vtbl` (about 100-190 moby classes per level), `camvtbl` (7-10), `sndvtbl` (1-7), the card-state table, and ~125 `jalr` call sites; `jr` jump tables for `switch` are generated by the compiler and need no handling.
- Each level's data record contains absolute pointers (to its own code and data) and the game relocates asset blobs by adding the load address to 32-bit fields (`relocateObjectPointers`, `RelocateTfrags`, `register_moby_class`, `load_sky`, ...: 7 of these are C and 4 are UNDONE).

### 9.5 What a 64-bit host build would have to change (an assessment)

Mandatory, mechanical: (1) define `INCLUDE_ASM`-class stubs away and supply C/hand ports for the missing code; (2) turn `asm/data/*.s` into C arrays/structs with relocations (277k lines of generated data); (3) define `NOT_SDA/MACRO_ADDR/SDATA` empty, replace the `__asm__("name")` aliases with real names; (4) redefine `qcopy/qzero/u128` portably; (5) make `long` a fixed 64-bit type (`int64_t`); (6) route every hardware address access (1000+ sites in ~170 functions) through a platform layer.

Structural: (7) every `(int)pointer`, `int` field holding an address, 32-bit pointer field inside retail data, and DMA tag address word (`D_00161000[1] = (int)ptr`) breaks if pointers are 64-bit.

**Running inside a 32-bit address space that matches the PS2's.** Feasible in principle on Linux (`mmap` at fixed low addresses, 32 MB at 0x0 plus scratchpad at 0x70000000, all pointers < 4 GB, the retail image layout reproduced), and on Windows x64 (a fixed `VirtualAlloc` in the low 2 GB is normally available). **It does not work in a native process on macOS/Apple Silicon** (tested 2026-10-08, macOS 27 on an M5 Max): `mmap` with `MAP_FIXED` below 4 GB fails in an arm64 process, and an arm64 binary linked with a smaller `__PAGEZERO` (`-Wl,-pagezero_size,0x4000`) is killed at launch. The same test built for x86_64 and run under Rosetta can map 0x10000000 and 0x70000000, so the option exists there, as a translated process. Where it works, it removes the need to touch the ~3,900 global+offset sites, the `(int)` casts, the pointer tables in data and the DMA address words: the renderer then reads `ee_ram[addr]` directly from the chain. The alternatives, each with a cost: (a) compile the C to **wasm32** and run it with a wasm runtime or `wasm2c` (a 4 GB linear memory with the SPR at 0x70000000 fits; all pointers are 32-bit offsets and `(int)ptr` is exact; hardware accesses become imports) [untested]; (b) a clang-based source transform that turns pointer types into 32-bit offsets (large, and the idioms above make it brittle); (c) rewrite per subsystem (what OpenGOAL does for the engine) and keep only the C that is genuinely portable.

Either way, the first things that stop a build are the missing code (section 4: 171 COP2 functions; section 5.4: ~170 undone functions on the boot path; 1,519 undone functions overall) rather than C syntax.

---

## 10. Across the sequels (what the OpenRAC docs say)

Evidence classes as in OpenRAC's `docs/engine/README.md` (**retail**, **reference**, **inferred**).

| aspect | RAC1 (PAL/US) | Going Commando (RAC2, `SCUS_972.68`) | Up Your Arsenal (RAC3, `SCUS_973.53`) | Deadlocked (RAC4, `SCUS_974.65`) |
|---|---|---|---|---|
| Program shape | boot exe + one full program per level replacing the "main" segment (`lit/bss/data/vtbl/camvtbl/sndvtbl/text`); entry = level initialiser/main loop; `vutext` and `core.*` resident | **same section list and order** (`.vutext` at 0x100080, `core.*`, then `.lit .bss .data lvl.vtbl lvl.camvtbl lvl.sndvtbl .text`); 27 levels; frontend main loop at 0x26EDE8 (retail); `$gp` 0x1AEFF0; an extra 88,931-byte load at 0x1800000 [doc] | `frontbin.elf` is itself a level overlay ("front-end level") loaded by the main exe at 0x1D5680, entry 0x37D200; 51 overlays [doc] | 20 KB loader + one compressed image (17 sections, 5.16 MB); 47 overlays (24 campaign + 23 multiplayer) each with its own overlay on disc; sections named like RAC2's; adds `net.text` [doc] |
| Frame loop | level main loop `func_L00_002465F8` | not decompiled; main loop address known | front-end main loop `func_0037D200` matched C; level loop undocumented | undocumented |
| Display list / DMA | VIF1 chain + `sceDma*`; hand-written renderers use scratchpad + D8/D9 | boot memory-map strings list "vu chain bufs", "shared vram", "particle vram", "effects vram"; same four geometry kinds (tfrag, tie, shrub, moby) from debug strings [reference] | frontbin writes DMA/GS registers and the scratchpad directly (819 SPR references, 101 VU0-macro functions) [doc] | not documented |
| VU microcode | 9 VU1 + 3 VU0 programs in `vutext`, on-demand REF uploads | **same convention (`.DVP.overlay` sections); 13 programs; program ids all recur; 21 of 49 comparable chunks byte-identical including all 8 of the tfrag program 55907; 56883 is new (role unknown); 13507 and 224979 grow by one chunk; `.vutext` hash identical on three 2003 prototypes and v1.01** [doc]. The uploading code is not located in RAC2 | not measured | not measured |
| File layer | absolute-LBA TOC at LBA 1500; ISO9660 holds 3 files; IOP streams audio | real file table: 97 ISO files, `RC2.HDR` (LBA 1001) -> `G/LEVELn.WAD` + `G/SCENEn.WAD` (prerelease discs; retail v1.01 not recorded) | levels unpacked with Wrench; loader not documented | ISO holds only the loader, IOP image, DNAS/net GUI; everything else from absolute sectors (RAC1-like) |
| Memory card | 25-state `CS_*` monitor + libmc driver (above) | `card.h`/`save.h` repeat the same state names (**reference**; no C file uses them; cited addresses are RAC1 PAL's) | `func_00397490` is the card state machine (2,163 instructions), CRC "follows the algorithm of RAC1 US" | not documented |
| Title screen | real-time 3D world in the executable | an IPU/FMV sofa animation [reference, community note] | same note | not documented |
| Sound/IOP | 989snd over SIF RPC; libpad2, libmc, libdbc, libcdvd | 989snd identifiers in boot data; the same IOP-library strings | `scePad` per community note | not documented |

What is shared at the code level [OpenRAC SHARED_CODE.md]: RAC1 and RAC2 share 70 % of RAC1's `core.text` (81 KB) but 8 % of the game code and almost no level code; RAC3 and RAC4 share 69 % of RAC3's core (106 KB); RAC1 shares 27 % of its core with RAC3/RAC4; 319 functions (38 KB, 30 KB of it in core) are identical in every version, mostly Sony libraries and the C runtime. Level code is each game's own (under 1 % identical across games except RAC3/RAC4 at 6-10 %). RAC1's "executable game code is a subset of every level" structure is not claimed for the others.

Design implications for a renderer shared by all four [inferred]:

- The **hardware-facing contract is the same family in all four**: a VIF1 DMA chain of CNT/REF/END tags with DIRECT packets for GS register writes, prebuilt GIF packets referenced from level data, UNPACKs into VU1 data memory, VU1 programs uploaded on demand from a resident microcode blob, and GS memory addressed explicitly. A renderer designed around "interpret the chain + GIF packets and re-implement each VU1 program's draw as a renderer class" (OpenGOAL style) transfers across games; the per-program VU memory maps and vertex formats do not (RAC2 edits tie, moby and particle programs; adds one).
- Do not assume RAC1's layout of the front end. In RAC1 the title and menu are the first-stage program that lives in the boot image's replaceable segment (RAC4's docs say the same: the resident `.text` is the first stage, the menus; RAC2's boot has a FRONTEND half of the same shape). In RAC3 the front end is a separate overlay, `frontbin.elf`, loaded over the executable. A platform layer should treat "load a program image, call its entry" as a generic operation and not assume which side the menus are on.
- The file layer differs enough (absolute LBA TOC vs. named files vs. Wrench-unpacked WAD) that the host layer should expose "read these sectors / this WAD entry" at a lower level than the game's own loaders and be per-game above that.
- Memory-card and pad layers are structurally similar (libmc/libpad2 over the IOP) but only RAC1's is analysed here.

---

# Appendix A. Hardware and scratchpad touch points (address-level)

Source: scan of every function's assembly for fully-formed constants in 0x10000800-0x1000FFFF, 0x12000000-0x12001FFF and 0x7000xxxx. `[K]` = kind: C = compiled, A = original asm (ASM_FUNC), U = INCLUDE_ASM. Excludes addresses that were really DMA-tag/VIF constants (0x1000000N, 0x1100xxxx).

```
func_00118ED0 C  1000F000                 VSync core (INTC_STAT)            func_00118F60 C 1000F000   VSync2
func_00119D88 C  1000F130 1000F180        TTY SIO
func_0011A780 U  1000C000 1000E010        sif init (SIF0 DMA, D_STAT)
func_0011DE38 C  10001810                 timer 2 mode (kernel init)
func_001207B8 U  10003000 10003C10 10003C20 10005000   sceGsResetPath      func_00120858 U 10003020 10003C00 10009000 1000A000  sceGsSyncPath
func_00121B78 C  12001000                 sceGsResetGraph (GS_CSR)
func_00122140 C  12000000 12000020 12000070 12000080 12000090 120000A0 120000C0 120000E0   sceGsPutDispEnv
func_00122598 C  12001000                 sceGsSyncV (field bit)
func_00123308 C  1000E000 1000E010        sceDmaReset                       func_001233E8 C 1000E000..1000E050 sceDmaPutEnv
func_00123650 C  1000E000 1000F520 1000F590  sceDmaPause
func_00125630 U  1000D400                 libmpeg decoder core
func_001273A0 C  10002010-30 1000B000-20 1000B400-20 1000F520/590      libmpeg IPU/DMA glue (func_00127378, 127D40, 128560..12D068 all C)
func_001299E8 C  1000D000-80 1000D400-80  libmpeg SPR DMA
func_0012A0F8 U  1000D400 1000D420 1000D430 1000D480
func_0012A718 A / func_0012A7E8 U  1000B400-20 1000E010 10002010
func_0012F308 C  10000800                 vsync callback (T1)
func_001F3D00 C  12000000 12000020 12000070 12000080 12000090 120000A0 120000D0 120000E0   ResetGsRegistersPr (every vsync)
func_001F3D78 U  10000800                 world render + profiler (T1/T0)
func_001F91B8 A  1000D400                 SPR DMA (drawquad)
func_001F9AF0 U  1000D000                 write_dma_channel
func_00201E88 C  10000800 10000810        InitOnce (T1 mode 0x82)
func_0020C210/C230 U 1000D400             DmaToSpr / DmaToSprSync
func_0020C268 A  1000E000 1000E020        InitDma
func_00212578 A  1000D000                 MobyAnimProc
func_002347F0 C  10008000 10008020 10008030   VU0_loadMicroProgram (D0)
func_00235018/002350A8 C  1000E010        DMAC_VIF1_Enable/Disable
func_00235218 C  10009000 10009030        print_register_values_and_halt
func_00235118 A  10009000 10009030        VIF1 DMAC handler
func_0023BB90 U  1000E000 1000E010        movie initAll       func_0023BE38 C 1000E000  termAll
func_0023C7A8 A  12001000                 movie vblankHandler
func_0023CF10/CF80 U  1000B000/1000B400 1000F520/590   setD3_CHCR / setD4_CHCR
func_0023D090..D988, 0023DCF0 U/C  1000B410-30 10002000-30 ...   viBuf* (movie ring buffers)
level code: func_L00_002465F8 U 10000800; func_L00_001FE688, 002535C8, 00254A70 U 1000D400; func_L00_00257848 U 1000D000; func_L02_002100E8 U 1000D400; func_L02_00250C78 U 1000E020 (+ ~30 asm renderer/anim functions using only 0x7000xxxx)
```

SPR (0x70000000-0x70003FFF) users: 101 functions. Compiled C: `sceGsPutDrawEnv`, `sceGsExecLoadImage`, libmpeg `func_0012A2F0/B918/C2F8`, `draw_map_overlay` `func_00205E70`, `decode_map_mask` `func_00206F40`, `func_00208458`, `AttachManipulator` `func_0020D960`, `ProcessMobyAnimData` `func_0020DFF8`, `InitMobyClassDists`/`Stash..`/`Restore..` `func_0020E040/68/98`, `func_00217AE8`, `func_0021D420/D4C0`, `func_00228860`, `VU1_sendChain`, level `func_L00_002422D8` (9,264 B), `func_L00_0028AF90`, `func_L00_0028BF60`, ripple (`func_L01_002B90E8`, `func_L01_002B9DC0`), `func_L02_002A59F8`, `func_L03_00292AC0`, `func_L09_0021E770`, `func_L12_002BD3D0`. The rest (hand-written renderers, collision, animation, particles) are asm.

# Appendix B. Sony / kernel / runtime entry points called from game code

(generated; names for libkernl syscall stubs come from the syscall number in each retail stub matched against the symbol list of `libkernl.a`'s `klib.o`; other names from `config/names.tsv` or by size/order match against the SDK archive symbol tables; "?" marks a guess.)

**newlib libc** (13 entry points called from game code; 112 exe sites, 257 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_001151B4 | memcmp | 1 | 0 | INCLUDE_ASM (not decompiled) |
| func_00115248 | memcpy | 8 | 2 | INCLUDE_ASM (not decompiled) |
| func_001153FC | memset | 16 | 18 | INCLUDE_ASM (not decompiled) |
| func_001160C8 | srand | 1 | 1 | C (matched) |
| func_001160D8 | rand | 9 | 158 | C (matched) |
| func_00116248 | sprintf | 51 | 71 | C (matched) |
| func_00116428 | strchr | 1 | 0 | INCLUDE_ASM (not decompiled) |
| func_001165B8 | strcmp | 3 | 0 | INCLUDE_ASM (not decompiled) |
| func_001166FC | strcpy | 6 | 1 | INCLUDE_ASM (not decompiled) |
| func_00116810 | strlen | 5 | 6 | INCLUDE_ASM (not decompiled) |
| func_00116948 | strncmp | 2 | 0 | INCLUDE_ASM (not decompiled) |
| func_00116B00 | strncpy | 8 | 0 | INCLUDE_ASM (not decompiled) |
| func_00116CC0 | strstr | 1 | 0 | C (matched) |

**libkernl (syscalls, intr, thread, tty, sif)** (36 entry points called from game code; 128 exe sites, 34 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00118A90 | AddIntcHandler (syscall 0x10) | 1 | 0 | original asm (counted finished) |
| func_00118AA0 | RemoveIntcHandler (syscall 0x11) | 1 | 0 | original asm (counted finished) |
| func_00118AB0 | AddDmacHandler (syscall 0x12) | 3 | 0 | original asm (counted finished) |
| func_00118AD0 | RemoveDmacHandler (syscall 0x13) | 3 | 0 | original asm (counted finished) |
| func_00118B50 | CreateThread (syscall 0x20) | 1 | 0 | original asm (counted finished) |
| func_00118B60 | DeleteThread (syscall 0x21) | 1 | 0 | original asm (counted finished) |
| func_00118B70 | StartThread (syscall 0x22) | 1 | 0 | original asm (counted finished) |
| func_00118B80 | TerminateThread (syscall 0x25) | 1 | 0 | original asm (counted finished) |
| func_00118BA0 | ChangeThreadPriority (syscall 0x29) | 1 | 0 | original asm (counted finished) |
| func_00118BC0 | RotateThreadReadyQueue (syscall 0x2B) | 1 | 0 | original asm (counted finished) |
| func_00118BE0 | GetThreadId (syscall 0x2F) | 1 | 0 | original asm (counted finished) |
| func_00118C70 | CreateSema (syscall 0x40) | 1 | 0 | original asm (counted finished) |
| func_00118C80 | DeleteSema (syscall 0x41) | 1 | 0 | original asm (counted finished) |
| func_00118C90 | SignalSema (syscall 0x42) | 9 | 0 | original asm (counted finished) |
| func_00118CB0 | WaitSema (syscall 0x44) | 9 | 0 | original asm (counted finished) |
| func_00118D60 | EnableCache (syscall 0x61) | 1 | 0 | original asm (counted finished) |
| func_00118D80 | FlushCache (syscall 0x64) | 53 | 15 | original asm (counted finished) |
| func_00118E10 | sceSifDmaStat (syscall 0x76) | 2 | 0 | original asm (counted finished) |
| func_00118E20 | sceSifSetDma (syscall 0x77) | 3 | 1 | original asm (counted finished) |
| func_00119328 | EnableIntc? (intr.o) | 1 | 0 | original asm (counted finished) |
| func_00119390 | DisableIntc? (intr.o) | 1 | 0 | original asm (counted finished) |
| func_001193F8 | DisableDmac | 2 | 0 | original asm (counted finished) |
| func_00119460 | EnableDmac | 2 | 0 | original asm (counted finished) |
| func_0011AE20 | sceSifInitRpc | 2 | 0 | C (matched) |
| func_0011B2F8 | sceSifBindRpc | 1 | 0 | C (matched) |
| func_0011B4C8 | sceSifCallRpc | 2 | 0 | INCLUDE_ASM (not decompiled) |
| func_0011B6B8 | sceSifCheckStatRpc (size match, not unique) | 1 | 0 | C (matched) |
| func_0011BF48 | sceFsReset (size match) | 1 | 0 | C (matched) |
| func_0011BF80 | sceOpen | 3 | 3 | C (matched) |
| func_0011C208 | sceClose | 3 | 3 | C (matched) |
| func_0011C388 | sceLseek | 6 | 6 | C (matched) |
| func_0011C5C0 | sceRead | 1 | 1 | C (matched) |
| func_0011C820 | sceWrite | 5 | 5 | C (matched) |
| func_0011CB40 | sceSifInitIopHeap | 1 | 0 | INCLUDE_ASM (not decompiled) |
| func_0011CBC8 | sceSifAllocIopHeap (inferred from LoadIRXModule) | 1 | 0 | C (matched) |
| func_0011CCB0 | sceSifFreeIopHeap (inferred) | 1 | 0 | C (matched) |

**libkernl (eeloadfile, IOP reboot, TLB, initsys)** (5 entry points called from game code; 11 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_0011D078 | sceSifLoadModuleBuffer (inferred) | 1 | 0 | C (matched) |
| func_0011D210 | sceSifSyncIop | 1 | 0 | C (matched) |
| func_0011D248 | sceSifRebootIop | 1 | 0 | C (matched) |
| func_0011D960 | DIntr | 4 | 0 | original asm (counted finished) |
| func_0011D9A8 | EIntr | 4 | 0 | original asm (counted finished) |

**VU1/DMA reset (libsn vu.o?)** (2 entry points called from game code; 20 exe sites, 7 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_001207B8 | sceGsResetPath | 4 | 0 | INCLUDE_ASM (not decompiled) |
| func_00120858 | sceGsSyncPath | 16 | 7 | INCLUDE_ASM (not decompiled) |

**libcdvd** (9 entry points called from game code; 30 exe sites, 1 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00120F30 | sceCdSync | 10 | 1 | C (matched) |
| func_001211B0 | sceCdInit | 2 | 0 | INCLUDE_ASM (not decompiled) |
| func_00121490 | sceCdDiskReady | 5 | 0 | INCLUDE_ASM (not decompiled) |
| func_00121688 | sceCdMmode | 1 | 0 | C (matched) |
| func_00121750 | sceCdRead | 2 | 0 | INCLUDE_ASM (not decompiled) |
| func_00121930 | sceCdGetError | 1 | 0 | C (matched) |
| func_001219C8 | sceCdBreak | 1 | 0 | C (matched) |
| func_00121A80 | sceCdReadClock | 4 | 0 | C (matched) |
| func_00121B78 | sceGsResetGraph | 4 | 0 | C (matched) |

**libgraph** (9 entry points called from game code; 58 exe sites, 31 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00121DC8 | sceGsSetDefDispEnv | 1 | 1 | INCLUDE_ASM (not decompiled) |
| func_00122140 | sceGsPutDispEnv | 1 | 0 | C (matched) |
| func_001222C8 | sceGsSetDefDrawEnv | 3 | 2 | C (matched) |
| func_001224B0 | sceGsPutDrawEnv | 1 | 0 | C (matched) |
| func_00122598 | sceGsSyncV | 24 | 12 | C (matched) |
| func_00122630 | sceGsSetDefLoadImage | 18 | 7 | INCLUDE_ASM (not decompiled) |
| func_00122818 | sceGsSetDefStoreImage | 0 | 3 | INCLUDE_ASM (not decompiled) |
| func_00122958 | sceGsExecLoadImage | 10 | 3 | C (matched) |
| func_00122AD8 | sceGsExecStoreImage | 0 | 3 | INCLUDE_ASM (not decompiled) |

**libgraph SyncVCallback + libdma** (4 entry points called from game code; 10 exe sites, 1 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00123168 | sceGsSyncVCallback | 3 | 1 | C (matched) |
| func_001232E0 | sceDmaGetChan | 2 | 0 | C (matched) |
| func_00123308 | sceDmaReset | 2 | 0 | C (matched) |
| func_001235C8 | sceDmaSend | 3 | 0 | C (matched) |

**libmc** (13 entry points called from game code; 38 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_001236F0 | sceMcInit | 1 | 0 | C (matched) |
| func_001238B0 | sceMcOpen | 5 | 0 | C (matched) |
| func_001239D8 | sceMcMkdir | 1 | 0 | C (matched) |
| func_00123A10 | sceMcClose | 9 | 0 | C (matched) |
| func_00123AC8 | sceMcSeek | 2 | 0 | C (matched) |
| func_00123C30 | sceMcRead | 4 | 0 | C (matched) |
| func_00123D48 | sceMcWrite | 4 | 0 | C (matched) |
| func_00123F30 | sceMcSync | 3 | 0 | C (matched) |
| func_00124068 | sceMcGetInfo | 2 | 0 | C (matched) |
| func_001241F0 | sceMcGetDir | 3 | 0 | C (matched) |
| func_00124340 | sceMcFormat (by archive order) | 1 | 0 | C (matched) |
| func_00124410 | sceMcDelete | 2 | 0 | C (matched) |
| func_00124528 | sceMcUnformat (by archive order) | 1 | 0 | C (matched) |

**libdbc** (1 entry points called from game code; 1 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00124650 | sceDbcInit | 1 | 0 | C (matched) |

**libpad2** (5 entry points called from game code; 5 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00124B88 | scePad2Init | 1 | 0 | C (matched) |
| func_00124BC8 | scePad2CreateSocket | 1 | 0 | C (matched) |
| func_00124D18 | scePad2Read | 1 | 0 | C (matched) |
| func_00124DF0 | scePad2GetButtonProfile | 1 | 0 | C (matched) |
| func_00124EE0 | scePad2GetState | 1 | 0 | C (matched) |

**libvib** (1 entry points called from game code; 1 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00125218 | sceVibGetProfile | 1 | 0 | C (matched) |

**libvu0** (6 entry points called from game code; 16 exe sites, 34 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_001252C0 | libvu0 internal entry (mid-function) | 0 | 22 | label |
| func_00125328 | libvu0 internal entry (mid-function) | 0 | 4 | label |
| func_00125358 | libvu0 helper (UnitMatrix or InterVector) | 4 | 2 | INCLUDE_ASM (not decompiled) |
| func_001253F8 | sceVu0RotMatrixZ | 4 | 2 | original asm (counted finished) |
| func_001254A0 | sceVu0RotMatrixX | 4 | 2 | original asm (counted finished) |
| func_00125548 | sceVu0RotMatrixY | 4 | 2 | original asm (counted finished) |

**libmpeg + IPU DMA** (10 entry points called from game code; 14 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_0012AD10 | sceMpegDemuxPssRing | 1 | 0 | C (matched) |
| func_0012B008 | sceMpegAddStrCallback | 1 | 0 | C (matched) |
| func_0012B870 | sceMpegInit | 1 | 0 | C (matched) |
| func_0012B918 | sceMpegCreate | 1 | 0 | C (matched) |
| func_0012BB20 | sceMpegDelete | 1 | 0 | C (matched) |
| func_0012BB30 | sceMpegGetPicture | 1 | 0 | C (matched) |
| func_0012BB88 | libmpeg (tiny, not unique) | 1 | 0 | C (matched) |
| func_0012BB98 | libmpeg (tiny, not unique) | 1 | 0 | C (matched) |
| func_0012BBA8 | sceMpegReset? | 1 | 0 | C (matched) |
| func_0012BC50 | sceMpegAddCallback | 5 | 0 | C (matched) |

**libscf** (2 entry points called from game code; 5 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_0012D380 | sceScfGetLanguage | 1 | 0 | C (matched) |
| func_0012D818 | sceScfGetLocalTimefromRTC | 4 | 0 | C (matched) |

**989snd** (42 entry points called from game code; 128 exe sites, 15 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_0012DB68 | snd_StartSoundSystem | 1 | 0 | INCLUDE_ASM (not decompiled) |
| func_0012DDC0 | snd_FlushSoundCommands | 23 | 7 | C (matched) |
| func_0012E060 | snd_BankLoadByLoc | 1 | 0 | C (matched) |
| func_0012E1C8 | snd_BankLoadFromEE_CB | 2 | 0 | C (matched) |
| func_0012E2E8 | snd_ResolveBankXREFS | 6 | 0 | C (matched) |
| func_0012E318 | snd_UnloadBank | 2 | 0 | C (matched) |
| func_0012E348 | snd_SetMasterVolume | 16 | 0 | C (matched) |
| func_0012E380 | snd_SetPlaybackMode | 3 | 0 | C (matched) |
| func_0012E3C0 | (unnamed) | 1 | 0 | C (matched) |
| func_0012E3F8 | snd_SetGroupVoiceRange | 3 | 0 | C (matched) |
| func_0012E448 | (unnamed) | 2 | 0 | C (matched) |
| func_0012E4A8 | (unnamed) | 2 | 0 | C (matched) |
| func_0012E4F8 | snd_StopAllSounds | 2 | 0 | C (matched) |
| func_0012E528 | snd_PauseAllSoundsInGroup | 2 | 2 | C (matched) |
| func_0012E558 | snd_ContinueAllSoundsInGroup | 1 | 3 | C (matched) |
| func_0012E588 | snd_SoundIsStillPlaying_CB | 3 | 0 | C (matched) |
| func_0012E600 | (unnamed) | 2 | 0 | C (matched) |
| func_0012EC30 | (unnamed) | 6 | 1 | C (matched) |
| func_0012EC40 | snd_reset_state_and_flush_commands | 8 | 1 | C (matched) |
| func_0012EC60 | snd_InitVAGStreamingEx | 1 | 0 | C (matched) |
| func_0012ED10 | snd_StopAllStreams | 1 | 0 | C (matched) |
| func_0012ED48 | snd_PlayVAGStreamByLocEx_CB | 11 | 0 | C (matched) |
| func_0012EDB0 | snd_PauseVAGStream | 1 | 0 | C (matched) |
| func_0012EDE0 | snd_ContinueVAGStream | 3 | 0 | C (matched) |
| func_0012EE10 | snd_GetVAGStreamTimeRemaining_CB | 1 | 0 | C (matched) |
| func_0012EE40 | snd_IsVAGStreamBuffered_CB | 1 | 0 | C (matched) |
| func_0012EE70 | snd_StreamSafeCheckCDIdle | 2 | 0 | C (matched) |
| func_0012EE98 | snd_StreamSafeCdRead | 1 | 0 | C (matched) |
| func_0012EF48 | snd_StreamSafeCdSync | 4 | 1 | C (matched) |
| func_0012EFE8 | snd_StreamSafeCdBreak | 1 | 0 | C (matched) |
| func_0012F030 | snd_StreamSafeCdGetError | 1 | 0 | C (matched) |
| func_0012F068 | snd_StreamSafeCdCallback | 2 | 0 | C (matched) |
| func_0012F0A8 | (unnamed) | 3 | 0 | C (matched) |
| func_0012F0E8 | snd_PreAllocReverbWorkArea | 1 | 0 | C (matched) |
| func_0012F120 | snd_AutoReverb | 1 | 0 | C (matched) |
| func_0012F1A8 | (unnamed) | 1 | 0 | C (matched) |
| func_0012F1E8 | snd_ResetMovieSound | 1 | 0 | C (matched) |
| func_0012F220 | (unnamed) | 1 | 0 | C (matched) |
| func_0012F248 | snd_StartMovieSound | 1 | 0 | C (matched) |
| func_0012F288 | snd_UpdateMovieADPCM | 1 | 0 | C (matched) |
| func_0012F2B8 | snd_GetMovieNAX | 1 | 0 | C (matched) |
| func_0012F2E0 | snd_GetDopplerPitchMod | 1 | 0 | C (matched) |

**wad (disc sector table)** (3 entry points called from game code; 5 exe sites, 0 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_0012F348 | wad_GetSectors_FiiPv | 2 | 0 | C (matched) |
| func_0012F3F8 | load_disc_sectors_into_global_buffer | 1 | 0 | C (matched) |
| func_0012F4A8 | read_file_entry_with_retry | 2 | 0 | C (matched) |

**libgcc** (1 entry points called from game code; 6 exe sites, 56 level-code sites)

| address | name | exe sites | level-code sites | in src/core as |
|---|---|---:|---:|---|
| func_00120778 | __extendsfdf2 (libgcc) | 6 | 56 | C (matched) |
# Appendix C. Unmatched (INCLUDE_ASM) executable functions reachable from boot

Closure of `jal` targets plus address-taken function references from `main` and `func_001E99D8`, executable functions only (`T` = also in the closure of the title loop `func_001EBB48`; `V` = uses COP2; `M` = uses real MMI). 167 functions, 94,828 bytes (sizes in bytes). The closure is an upper bound (section 5.4). Names are suggestions from `config/names.tsv`.

```
167 94828
src/core/00114518.c 5 1000
    func_001150D4   224 T-M 
    func_001151B4   148 T-M 
    func_00115248   176 T-- 
    func_001152F8   260 T-- 
    func_001153FC   192 T-M 
src/core/001162B8.c 5 1796
    func_001165B8   324 T-M 
    func_001166FC   276 T-M 
    func_00116810   312 T-M 
    func_00116948   440 T-M 
    func_00116B00   444 T-M 
src/core/00119D88.c 7 4344
    func_00119EA8   144 T-- 
    func_0011A0A0  1516 T-- 
    func_0011A780   640 T-- 
    func_0011B4C8   492 T-- 
    func_0011B868   900 T-- 
    func_0011BCB0   516 T-- 
    func_0011CB40   136 --- 
src/core/001207B8.c 2 300
    func_001207B8   160 TV- 
    func_00120858   140 TV- 
src/core/001208E8.c 3 1388
    func_00120B28   148 T-- 
    func_001211B0   736 --- 
    func_00121490   504 --- 
src/core/00121750.c 1 480
    func_00121750   480 T-- 
src/core/00121D18.c 1 884
    func_00121DC8   884 T-M sceGsDefDispEnv
src/core/00122630.c 1 484
    func_00122630   484 T-M sceGsSetDefLoadImage
src/core/001252A0.c 2 156
    func_00125358    40 TV- 
    func_00125380   116 TV- 
src/core/00125630.c 9 6144
    func_00125630   592 T-- 
    func_00125880  1796 T-- 
    func_00125F88  1052 T-- 
    func_001263A8   540 T-M 
    func_00129690   692 T-- 
    func_00129CA0   396 T-- 
    func_00129E30   272 T-- 
    func_00129F40   440 T-- 
    func_0012A0F8   364 T-- 
src/core/0012A2F0.c 3 1124
    func_0012A558   448 T-- 
    func_0012A7E8   648 T-- 
    func_0012AAA8    28 T-- 
src/core/0012AC80.c 3 1040
    func_0012AC80   136 T-- 
    func_0012C608   680 T-- 
    func_0012C8B0   224 T-- 
src/core/989snd.c 3 1580
    func_0012DB68   600 --- snd_StartSoundSystem
    func_0012E820   704 T-- snd_SendIOPCommandNoWait
    func_0012EB18   276 T-- snd_SendCurrentBatch
src/game/bmain.c 1 1176
    func_001E99D8  1176 --- 
src/game/draw.c 8 8336
    func_001F3140  1564 T-- UpdateViewContext
    func_001F3890   768 T-- SetPalMode
    func_001F3D78  2168 --- DrawDebugProfiler
    func_001F4868   328 T-- GetEffectTex
    func_001F5E60  1116 --- 
    func_001F6668   640 T-- FontPrint
    func_001F7070  1264 T-- FontPrintWindow
    func_001F7680   488 --- 
src/game/drawquad.c 2 288
    func_001F7DD8   192 -VM 
    func_001F7E98    96 -V- 
src/game/fastfunc.c 44 2264
    func_001F98B0    16 --- 
    func_001F98C0    40 T-- 
    func_001F9AF0    48 T-- write_dma_channel
    func_001F9B20    40 -V- 
    func_001F9B50    32 -V- vec_length
    func_001F9BB0    16 T-- 
    func_001F9BC8    16 -V- 
    func_001F9BD8    24 TV- FastVecAdd
    func_001F9BF0    24 TV- FastVecSub
    func_001F9C08    40 -V- 
    func_001F9C30    24 TV- FastVecScale
    func_001F9C48    24 TV- vec_scale
    func_001F9C60    24 -V- 
    func_001F9C78    40 TV- FastVecDot
    func_001F9CA0    24 TV- FastVecCross
    func_001F9CB8    48 TV- FastVecLength
    func_001F9CE8    40 TV- 
    func_001F9D10    56 TV- FastVecDist
    func_001F9D48    48 -V- VecDistance2
    func_001F9DC0    80 TV- 
    func_001F9E10    72 -V- 
    func_001F9EC0    40 TV- matrix_mul_vec3
    func_001F9EE8    48 TV- 
    func_001F9F18    24 -VM FastVectorFromPackedChars
    func_001F9F30    40 -VM FastVectorToPackedChars
    func_001F9F90    24 TV- FastCos
    func_001F9FA8    24 TV- FastSin
    func_001F9FC0   152 TV- FastArcSin
    func_001FA190    48 TV- 
    func_001FA1C0    56 -V- 
    func_001FA218    32 -V- euler_to_matrix
    func_001FA460    32 -V- 
    func_001FA480    32 --- matrix_copy_rows
    func_001FA4A0    80 TV- 
    func_001FA588    64 -V- 
    func_001FA5C8   128 -V- 
    func_001FA648   120 -V- 
    func_001FA6C0   136 -V- 
    func_001FA748    72 T-- FastAddRots
    func_001FA790    72 --- FastSubRots
    func_001FA7D8   120 --- FastNormalizeAngle
    func_001FA850    56 T-- FastDiffRots
    func_001FA898    16 T-- truncate_float_to_s32
    func_001FA8A8    72 TVM FastTweenColor
src/game/framebuf.c 1 2308
    func_001FAB40  2308 T-M SetupFS_AA_buffer
src/game/freeze.c 2 8336
    func_001FBE80  5480 T-- 
    func_001FD3E8  2856 T-- 
src/game/hud.c 2 856
    func_001FF958   312 T-- Hud_SendResidentBank
    func_00200248   544 T-- GetFrameTex
src/game/loaders.c 7 6532
    func_00202AA8  1104 --- 
    func_002035B0   600 T-- BuildMobyAdGif
    func_00203B70   772 T-- MobyClassRelocate
    func_00203F68   980 T-M tie_ad_gif_convert
    func_00204340  1492 T-- shrub_class_init
    func_00204918   720 T-- RelocateTfrags
    func_00204C60   864 --- 
src/game/miscproc.c 2 88
    func_0020C210    32 T-- DmaToSpr
    func_0020C230    56 T-- DmaToSprSync
src/game/mobyutil.c 1 148
    func_00213C78   148 T-- MobyUpdateLoop
src/game/movie/audiodec.c 1 588
    func_0023C390   588 T-- sendADPCM
src/game/movie/disp.c 2 528
    func_0023C5E0   452 T-- setImageTag
    func_0023C910    76 T-- handler_endimage
src/game/movie/movie.c 2 1700
    func_0023B740  1024 T-- 
    func_0023BB90   676 T-- initAll
src/game/movie/read.c 2 532
    func_0023CAF8   228 T-- pcmCallback
    func_0023CBE0   304 T-- cpy2area
src/game/movie/vibuf.c 13 3672
    func_0023CEC8    68 T-- getFIFOindex
    func_0023CF10   108 T-- setD3_CHCR
    func_0023CF80   108 T-- setD4_CHCR
    func_0023CFF0    36 T-- scTag2
    func_0023D018   120 T-- viBufCreate
    func_0023D090   352 T-- viBufReset
    func_0023D1F0   244 T-- viBufBeginPut
    func_0023D340   508 T-- viBufAddDMA
    func_0023D540   272 T-- viBufStopDMA
    func_0023D650   824 T-- viBufRestartDMA
    func_0023DA88   340 T-- viBufModifyPts
    func_0023DBE0   272 T-- viBufPutTs
    func_0023DCF0   420 T-- viBufGetTs
src/game/music.c 1 1108
    func_00217130  1108 T-- music_Update
src/game/pad.c 1 1816
    func_002181F0  1816 T-- 
src/game/pause.c 8 5796
    func_00219E90   544 T-- 
    func_0021A1A0  1132 T-- 
    func_0021A610  1732 T-- 
    func_00226D50   340 T-- 
    func_00227DB0   860 T-- 
    func_002282D0   304 T-- 
    func_002284E8   424 T-- 
    func_00228690   460 T-- 
src/game/skyfunc.c 3 1776
    func_0022C5A0   576 --- DrawSkyShells
    func_0022CA00   572 T-- SkyDrawShellTextured
    func_0022CC40   628 T-- SkyDrawShellGouraud
src/game/sound.c 3 4260
    func_0022DBE8   380 --- SoundMasterVolumeInit
    func_0022DD68  3252 T-- sound_update
    func_0022EB08   628 T-- SoundSlotAlloc
src/game/space.c 7 8964
    func_0022F738  1188 --- 
    func_0022FDC0  2012 --- SpaceLoadingLoop
    func_002305A0   808 --- DrawWorldPaused
    func_00232278  1704 --- EnterSpaceLoadingLoop
    func_00232B90   860 --- 
    func_00232EF0  1048 --- play_story_transition
    func_00233AB8  1344 --- ShipDrawCallback
src/game/transition.c 4 7292
    func_001E9EC8  3356 T-- 
    func_001EABE8  1812 T-- 
    func_001EB458   868 T-- 
    func_001EBB48  1256 T-- 
src/game/update.c 1 516
    func_00238D90   516 T-- moby_screen_rect
src/game/vendor.c 1 756
    func_0023B210   756 --- RippleHeightQuery
src/libgcc/nonmatching_0011E860.c 1 1640
    func_0011E860  1640 T-- 
src/libgcc/nonmatching_0011EF28.c 2 2832
    func_0011EF28  1488 T-- 
    func_0011F4F8  1344 T-- 
```

# Appendix D. Method, limits, and what is not covered

Method: no builds or Docker; Python scripts over `asm/*`, `src/*`, `progress/report.json`, `config/*`, `baserom/overlays/*/manifest.json`, the executable image, and (read-only) the ISO through `tools/extract/disc.py`/`formats.py` (to read the IOP module bundle at TOC 0x12C0). SDK archive symbol tables in `toolchain/sn-prodg-24/local/sce/ee/lib/*.a` were read only for names and sizes, not code. The scripts were: `build_table.py` (per-function kind/COP2/MMI table, `table.json`), `surface.py` (library call-site counts), `hw.py`/`hw2.py` (hardware addresses), `closure.py`/`closure2.py` (call graph and reachability), `hazards.py` (portability counts), `namelibs.py`/`arsyms.py`/`appB.py` (library names).

Weak or unverified points, in order of how much a design might lean on them:

1. **Display buffer handling** (1.3): no per-frame colour-buffer flip was found; a single displayed buffer plus an AA pass is inferred from `func_001FAB40`'s layout, and that function is not decompiled and was only skimmed.
2. **MSCAL/MSCNT map** (3.3): only the immediates visible in the code are listed. Tfrag, particle and most moby start addresses were not found, and which VU0 `vcallms` entry belongs to which VU0 image is not mapped.
3. **Boot-path closure** (5.4) is a reachability upper bound; the real set depends on conditions that are not decoded. The mode-dispatch table (1.2) uses ReRAC's mode names for the updates/renders, but the order and callees come from the jump table.
4. **Library names** (8, App. B): function-size/order matches against the SDK archive symbol tables; ambiguous ones are marked. Syscall names come from the stub's syscall number matched against `klib.o`'s own stubs (reliable), not from SDK headers.
5. **The "32-bit address space is feasible" assessment** (9.5) is reasoning from the code's patterns for Linux and Windows; nothing was run there. The macOS result in 9.5 was tested.
6. **Sections of the cross-game table (10)** repeat OpenRAC's own evidence classes; RAC2/RAC3/RAC4 rows are as thin as those documents are. The RAC2-4 projects' own source trees were not opened.
7. Not covered: the GS texture/CLUT format handling in `upload_texture_images`; the exact per-frame draw ordering conditions in `DrawDebugProfiler`; the collision/animation VU0 code semantics; the `movie/` player internals; PS2 sound formats; anything in the decompressed level core data beyond what `docs/ASSETS.md` already documents.
