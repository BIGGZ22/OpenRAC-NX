# Runtime and renderer: design

How OpenRAC gets from decompiled games to games that run on a PC, starting
with a renderer on macOS. This page records what is decided, what is proposed
and why. The evidence is in [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md) (what
the game does) and [OPENGOAL_NOTES.md](OPENGOAL_NOTES.md) (how a comparable
port did it).

Status words used below: **decided** (by the maintainer), **proposed** (the
current plan, open to change), **open** (needs a decision or a measurement).

## 1. Goals

1. The main menu, the memory card screens and the first level of Ratchet &
   Clank running on a Mac, from the user's own disc image, without a
   PlayStation 2 emulator or BIOS.
2. One runtime and one renderer for all four games (**decided**: the work
   lives in OpenRAC, not in one game's tree).
3. A path from there to the long-term goal in [AGENTS.md](../../AGENTS.md): C
   that compiles and runs on a PC, with the matching decompilation as the
   proof that the C is the game.
4. Everything written here is GPL-3.0-or-later (**decided**,
   [LICENSE.md](../../LICENSE.md)). No game data, microcode or disc content
   is ever in the repository; the runtime reads the user's disc image.

## 2. What the studies settle

- **The games speak to the hardware through one narrow contract.** A frame is
  a VIF1 DMA chain in EE memory: DMA tags, GIF packets sent with `DIRECT`,
  `UNPACK`s into VU1 data memory, VU1 programs uploaded on demand, GS
  registers set packet by packet, textures sent to GS memory by address
  (survey, sections 1 to 3). The sequels use the same family of contract and
  RAC2 shares VU1 microcode with RAC1 byte for byte in places (survey,
  section 10). A renderer that takes that contract as its input serves every
  game and does not care how the code that produced the chain was run.
- **The drawing code is not C and will not be soon.** Every renderer core
  (tfrag, tie, shrub, moby, particles, lighting, shadows) is hand-written
  assembly using VU0 macro instructions, the scratchpad and DMA channels 8
  and 9; 171 functions use COP2 and 107 more use the 128-bit integer set, and
  none of them is compiled C. The arithmetic of the nine VU1 programs is not
  documented in any OpenRAC project (survey, sections 3 and 4).
- **The main menu needs all of it.** The title screen is in the boot
  executable and draws a 3D world through the full renderer stack; 128
  functions reachable from the title loop are not decompiled (survey, section
  5).
- **The decompiled C does not compile for a 64-bit host as written**, and is
  not meant to yet: it reproduces MIPS code. Pointers are 32-bit integers in
  thousands of places, data is addressed as "global plus offset" in the
  retail layout, and all data is still assembly (survey, section 9).
- **A native process on Apple Silicon cannot live in the PS2's address
  space.** Mapping below 4 GB fails in an arm64 process and a binary with a
  small `__PAGEZERO` is killed at launch (tested 2026-10-08; survey, section
  9.5). So "compile the C natively and let it use real PS2 addresses" is not
  available on the first target machine, except as an x86_64 process under
  Rosetta.
- **OpenGOAL's renderer is the model for the hand-off, not for the content.**
  Its game builds real DMA chains and a renderer walks them on the main
  thread, which is the seam proposed here. But it drew the 3D world from
  geometry converted offline and replaced each VU1 program by hand, after its
  game code already ran natively (notes, sections 0, 3 and 4). Its GS state
  conventions carry over as they are (notes, section 9, item 4).

## 3. How the game code runs (**proposed**; this is the decision that matters)

Three ways to get the game's code to produce chains:

| | Route | First picture needs | Verdict |
|---|---|---|---|
| A | Compile the decompiled C for the host and reimplement what is not C (OpenGOAL's route) | data as C, every COP2 function rewritten, the nine VU1 programs understood, the remaining 39 % decompiled at least along the path, and a 32-bit pointer model for the host | the end state, years away as the only route |
| B | Run the retail program as a guest: an EE instruction interpreter over a 32 MB memory image, the game's hardware behind it replaced | an interpreter, a handful of hardware models and replacements for the Sony library calls | **proposed first step** |
| C | Compile the decompiled C to a 32-bit sandbox target (wasm32, then `wasm2c`) so that its pointers are offsets into the same memory image | route B's memory image, plus generated call thunks between guest and host code | **proposed bridge from B to A**, untested |

Route B gets the retail code of all four games running with the menu, the
card and the levels complete, because it does not depend on how much is
decompiled. On its own it would be an emulator for four games. What makes it
a port is what is layered on it, and each layer is something only a
decompilation makes possible:

1. **The boundary is at the libraries, by name.** The decompilations name the
   Sony and 989snd entry points and their arguments (survey, section 8 and
   Appendix B). The runtime replaces those functions outright: `sceCdRead`
   reads the disc image, the 13 libmc calls become files in a save directory,
   the pad library reads a host controller, `sceGsSyncV` presents a frame.
   There is no BIOS, no IOP processor and no SPU2 model.
2. **Functions are replaced one at a time by host code.** Any guest function
   can be overridden by a host function with the same effect on the memory
   image: first the ones worth rewriting (the renderers, section 4.4), later
   compiled decompiled C (route C), until the interpreter has nothing left to
   run. Each replacement can be checked against the original by running both
   and comparing memory, which extends the project's rule that matching is
   the proof.
3. **The renderer is ours and takes the chain**, so resolution, aspect ratio
   and frame pacing are the port's to decide.

The guest program is the user's disc image. The runtime ships no code or
data from the games; per game it carries a table of addresses and names that
the game's decompilation project generates.

Why an interpreter and not a recompiler first: correctness and inspection
come first, level programs replace code at run time (an interpreter does not
care), and the EE's 295 MHz is within reach of a plain interpreter on the
target machine. A cached interpreter or a static translation of each
program is a later, measurable step.

## 4. The renderer

```
game code (guest or host)
   │ writes DMA registers (libdma) / calls sceGs* 
   ▼
DMA channel 1 walker ──► VIF1 interpreter ──► VU1 ──► GIF ──► GS model ──► frame
                           UNPACK, MPG,      micro-          registers,     │
                           MSCAL, DIRECT     programs        local memory   ▼
                                                                         host window
```

### 4.1 Stages (**proposed**)

- **DMA walker.** Source chain mode with the tag kinds the games use (`cnt`,
  `ref`, `next`, `call`, `ret`, `refe`, `refs`, `end`), tag transfer and the
  IRQ bit. RAC1 waits on IRQ-tag fences and on the end of the chain through
  an interrupt handler (survey, section 2.2), so the walker reports those
  events to whoever runs the game code.
- **VIF1 interpreter.** All VIF codes: `STCYCL`, `OFFSET`, `BASE`, `ITOP`,
  `STMOD`, `STMASK`, `STROW`, `STCOL`, `MPG`, `MSCAL`, `MSCALF`, `MSCNT`,
  `FLUSH*`, `DIRECT`, `DIRECTHL` and every `UNPACK` format with masking,
  modes and write cycles.
- **VU1.** An interpreter for the microprograms the game uploads, with the
  flag and pipeline behaviour programs rely on. This is what makes the 3D
  world appear without anyone having understood a single program, and it is
  the reference that later native versions are checked against.
- **GIF.** PACKED, REGLIST and IMAGE packets on paths 1 (VU1 `XGKICK`), 2
  (`DIRECT`) and 3 (libgraph's image transfers).
- **GS.** The drawing registers, the 4 MB local memory with its block
  layouts, primitive assembly, and a rasteriser.

### 4.2 The GS starts as a software model (**proposed**)

The first GS back end is a software rasteriser over a real model of GS local
memory, presented through one host texture. Reasons:

- The games address GS memory explicitly: textures by block address,
  render targets at computed addresses, frame-buffer readback for the pause
  background, a full-screen blur pass (survey, sections 1.3 and 2.6). A
  software model gets all of these right by construction. A GPU back end has
  to recognise each case.
- It is the reference that a GPU back end and the native renderers are
  compared against, pixel for pixel, and it needs no emulator for
  comparison.
- It is small: registers, block tables, a triangle and sprite rasteriser,
  the texture, fog, test and blend equations.

Internal resolution scaling in the software model is possible but costs in
proportion; the answer for high resolution is 4.3 and 4.4.

### 4.3 A GPU back end (**open**: OpenGL 4.1 or SDL_GPU)

Behind the same interface as the software GS. OpenGL 4.1 core is what
OpenGOAL uses on every platform and what macOS still offers, and it would
let the GS conventions in OPENGOAL_NOTES.md be reused directly; it is
deprecated on macOS. SDL3's GPU API targets Metal, Vulkan and Direct3D 12
from one code base. Both were confirmed to work on the target machine
(SDL 3.4, OpenGL 4.1 on Metal). The choice can wait until the software model
draws the menu, because nothing before that depends on it.

### 4.4 Native renderers per VU1 program (**proposed**, later)

`MSCAL` is the natural hook: when VIF1 starts program N, a host renderer may
take over, read the same VU memory the program would have read and draw
directly on the GPU, at any resolution. That is OpenGOAL's method (notes,
section 4) applied one program at a time, with the interpreter's output as
the test. RAC1 has nine VU1 programs; RAC2 keeps the terrain and shrub
programs unchanged, so host renderers for those two serve both games. The
executables name every program and its chunks in a table of their own, which
is how the runtime can tell which program is loaded
([VU_PROGRAMS.md](VU_PROGRAMS.md)).

### 4.5 Conventions taken from OpenGOAL (**decided** by measurement there)

Depth cleared to 0 and compared with greater-or-equal; GS window coordinates
mapped by subtracting 2048 and dividing by the half extents; colour 0x80 is
1.0; alpha test reference divided by 128; the `ADC` bit carried per vertex;
`STQ` divided per pixel (notes, section 9, item 4).

## 5. The rest of the machine (**proposed**)

| Service | What the games call | Host replacement |
|---|---|---|
| Disc | `sceCdRead(lba, sectors, buffer)` and status calls; RAC1 and RAC4 by absolute sector, RAC2 by file | read the user's ISO; completion is immediate |
| Memory card | libmc, 13 calls in one state machine (RAC1) | a directory of plain files per card; completion is immediate |
| Pad | libpad2 (RAC1), libpad (later games) | SDL gamepad and keyboard |
| Sound | the 989snd API over SIF RPC to the IOP | silent at first; then a host mixer behind the same API |
| Movies | libmpeg and the IPU | skipped at first; then a software decoder |
| Time | `sceGsSyncV`, timer 1, the vsync callback | the host frame pacer; timer values derived from it |
| Kernel | thread, semaphore, interrupt and cache calls | no-ops and a small scheduler for the movie player |

Each row is replaced at the function, not at the hardware register, except
where the game itself writes registers (DMA, GS privileged registers,
timers, VIF0 for VU0 programs).

## 6. What is per game

- The disc image and how programs and levels are found on it.
- A hook table: guest address, the service it maps to. Generated from the
  game's own symbol and name files by a tool, never typed by hand, and
  checked against the checksum of the program it applies to.
- Nothing else. A game-specific fix in the shared code is a bug in the
  model.

## 7. Layout, build and dependencies

`runtime/` is a top-level component like `editor/` ([LAYOUT.md](../../docs/LAYOUT.md)).

```
runtime/
├── CMakeLists.txt
├── README.md
├── docs/            this page, the survey, the OpenGOAL notes
├── src/ps2/         the hardware contract: DMA, VIF, GIF, GS, later VU and EE
├── src/host/        window, input, files
├── src/app/         the programs: openrac-gsdemo now, the runtime later
└── tests/           unit tests, run by ctest
```

C++20, CMake and Ninja, SDL3 for the window and input (zlib licence, taken
from the system). No other dependency yet. The code is written from public
hardware documentation and from what the games' own code shows; where a
permissively licensed project's code is adapted, the file says so and
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md) carries the notice.
Sony's SDK source, samples and headers are never used
([SOURCING.md](../../docs/policy/SOURCING.md)).

## 8. Milestones

| | Milestone | Shows |
|---|---|---|
| M0 | Build, window, software GS, GIF, VIF and DMA decoding, the vector unit interpreter, tests; a demo that draws from a hand-built chain, part of it through a microprogram on VU1 | the renderer end exists (**done**, see [README](../README.md)) |
| M1 | EE interpreter boots RAC1 PAL from the ISO to the title loop with the library boundary replaced; chains reach the walker | the game code end exists (**done**) |
| M2 | The EE's vector instructions and VU0 microprograms on the same interpreter; the games' VU1 programs run; the title screen and main menu draw | first real picture (**done**) |
| M3 | Memory card as files, pad input, the save and load screens | the menu is usable (**done**: the game creates, lists, saves and loads its files in a host directory) |
| M4 | Level 0 loads and plays | first level (**done**: it loads, draws and takes input) |
| M5 | Sound; frame pacing; a faster EE core and vector unit; the drawing path on its own thread | playable (frame pacing **done**; four threads and a faster vector unit bring the first level to 40-48 of 50 frames a second on an M5; music, speech and sound effects play; the sound processor's reverb is open) |
| M6 | GPU back end and the first native VU1 renderers; wide screen and resolution | a port, visibly |
| M7 | The other games' hook tables | all four boot |
| M8 | Host-compiled decompiled C replacing guest functions (route C) | the decompilation runs (**started**: 2,178 functions of the first game build as host code and stand in for 21,157 places across its 19 level programs; all 19 levels start and draw with 450 to 530 of them running, two calls in three are host code, each function bound by checksum and checked against the retail code; see [port/README.md](../port/README.md)) |

## 9. Open decisions

1. **Route B as the first step** (section 3). The alternative is to wait for
   route A's prerequisites. Nothing in M0 depends on the answer; M1 does.
2. **GPU back end**: OpenGL 4.1 or SDL_GPU (section 4.3). Can wait for M2.
3. **How the per-game hook tables are published**: generated into
   `runtime/` from each game's files, or kept in each game's directory.
4. **Whether the movies matter early** (publisher logos and the attract
   loop are the only ones before the menu).
