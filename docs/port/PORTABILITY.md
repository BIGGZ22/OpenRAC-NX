# From matching C to C a PC compiles

The decompiled C reproduces MIPS code for the PlayStation 2's compiler. This
page counts what in it assumes the console (32-bit pointers, the retail
memory layout, the console's types and arithmetic, the tricks that make the
compiler emit the retail bytes) and records what a PC build has to do about
each. It is about Ratchet & Clank (PAL), `games/rac1`; paths are
relative to it unless they start at the repository root.

How it was counted, on 2026-10-09 at `7954152`: regular-expression passes
over `src/**/*.c` (385 files, about 252,000 lines; regex counts can run a
little high), and two compile passes of every source file through
`clang -fsyntax-only` with pointer-width warnings on, one for a 32-bit
target (wasm32) and one for x86_64 Linux (LP64). No file was written.

## 1. Pointers

What the compiler reports on x86_64:

| Warning | Sites | Files |
|---|---:|---:|
| pointer cast to a smaller integer | 1,047 | 150 |
| integer cast to a pointer | 448 | 112 |
| integer cast to `void *` | 180 | 33 |
| `void *` cast to an integer | 39 | 18 |
| function result cast to a pointer (mostly int-returning or undeclared functions) | 1,261 | 71 |
| 64-bit value shortened to 32 | 68 | 18 |
| implicit integer conversion | 40 | 15 |
| implicitly declared function | 63 | 35 |
| cast that raises alignment (the `char *p` + offset style) | 32,241 | 249 |

The bigger problem is what no compiler sees:

- **Pointers stored in data at 4-byte spacing.** `*(T **)(p + 0xNN)`
  appears at 2,477 sites in 156 files; on a 64-bit PC each is an 8-byte
  access at an offset laid out for 4. `*(T *)(x + 0xNN)` in general: 26,240
  sites in 228 files. `include/structs.h` declares pointer fields at 4-byte
  spacing (`MobyInstance`, 0x100 bytes: the class at 0x24, the update
  function at 0xA8, the variables at 0xAC, the parent at 0xB8), but only
  two source files use it; everything else uses raw offsets.
- **Pointers held in integers.** 127 `= (int)…` stores in 48 files, 736
  `(int|u32)expr` casts in 151 files, and the display list itself: a DMA
  tag's address word is a pointer stored as a 32-bit integer
  (`game/vuchain.c:461` `D_00161000[1] = (int)data;`).
  `game/initonce.c:201-210` computes a table address from a link symbol and
  follows self-relative 32-bit offsets inside an asset. Eleven relocators
  (seven decompiled) add a load address to 32-bit fields in disc data.
- **Global plus offset.** `D_xxxxxxxx + 0xNNN` at 3,534 sites in 161 files,
  2,932 of them with an offset of 0x400 or more; another 780
  `(char *)&D_ + off`. `D_0013E633` alone has 2,805 uses, 2,319 of them
  `+0xE1D` (0x13F450, the hero and game state block). These depend on the
  retail data being where it is, next to what it is next to.
- **Type-punned globals.** 2,004 `*(float *)&D_x` reads in 145 files; 1,605
  globals declared with a deliberately wrong type to steer the compiler's
  `$gp` addressing.
- **Fixed addresses.** 286 hardware-register literals in 34 files (194 as
  `(volatile T *)0x…`); 72 scratchpad addresses (0x7000xxxx) in 20 files;
  57 uncached aliases (`| 0x20000000`); 26 address masks
  (`& 0x0FFFFFFF`) in the DMA code; the top of RAM as a symbol.
- **Function pointers in data**: 506 in the executable's data, plus every
  level's moby, camera and sound dispatch tables, all 4-byte words.

**Conclusion.** Pointer width is assumed everywhere, not in a few places:
about 150 files have explicit pointer and integer casts, about 160 address
globals by offset, and offset-based access is in essentially every file.

## 2. Types and arithmetic

- **`long` is 64 bits** on the console (`typedef long s64`), used 1,687
  times in 186 files, mostly for GS register values; 214 `L` constants.
  Fine on LP64 Linux and macOS, wrong on Windows (LLP64): use fixed-width
  types, as the NTSC tree does (`games/rac1/ntsc/include/types.h`).
- **128-bit values**: `__attribute__((mode(TI)))` at 206 sites in 106
  files. A PC build should use a 16-byte aligned struct; `__int128` does
  not exist in MSVC.
- **Floats**: the console's FPU has no infinities, NaNs or denormals and
  rounds toward zero; division by zero gives the largest float, and float
  to integer saturates. The removed experiment reproduced this bit for bit
  and found it necessary for matching results ("without this a division by
  zero leaves an infinity where the retail code leaves the largest number,
  and everything computed from it differs"). A port chooses between exact
  console arithmetic (slow) and IEEE with the divisions and conversions
  clamped where the game relies on them.
- **Doubles** are software floating point on the console (31 uses in the
  source).
- **Integers**: `char` is signed (`-fsigned-char`), signed overflow wraps
  (`-fwrapv`), aliasing is loose (`-fno-strict-aliasing`).

## 3. Matching artefacts

| Artefact | Count | For a PC build |
|---|---|---|
| `INCLUDE_ASM` stubs | 1,614 (91 libraries, 201 game, 754 shared level, 562 level) | missing code: needs C |
| `ASM_FUNC` | 212 in 32 files | hand-written assembly: needs C |
| `LINKER_REMNANT` | 229 in 103 files | define away |
| `__asm__("symbol")` aliases | 1,934 in 245 files | give one symbol several prototypes; become typed declarations or casts |
| file-scope `nop` padding | 48 in 26 files | define away |
| inline `sync` | 5 sites | define away |
| `volatile` | 269 in 28 files | almost all hardware registers |
| `NOT_SDA`, `MACRO_ADDR`, `SDATA` (`include/common.h`) | 2,243, 2,131 and 168 uses | section attributes for matching: defined empty |
| `qcopy`, `qcopy_nc`, `qzero` (inline `lq`/`sq`) | 2,327, 5 and 45 uses | 16-byte copies: `memcpy`/`memset` |

**Code that matches but misbehaves on a PC:**

- relies on the stack layout (`overlays/shared/vendor_002D3DF8.c:495-512`
  passes `float v[2]` to a function that reads three floats);
- reads locals that were never set (`func_L00_00263D68` sums four floats of
  which three were set);
- narrows 64-bit results through `s32` prototypes (`game/pause.c:667`,
  `func_0021C6C0`), where the console's register moves keep all 64 bits.

The 416 staged near misses in `nonmatching/` are C too, close to the retail
bytes, and usable by a port. The NTSC tree keeps descriptive C next to its
assembly under `#ifndef NON_MATCHING` (227 files), another source of
portable C for the same game.

## 4. What the removed experiment learned

`runtime/port/` (removed on 2026-10-09; in git history before that, e.g. at
`7954152`) compiled the decompiled C for a 32-bit sandbox and ran it in
place of the retail functions. That approach is not the port's, but what it
found about the C holds for any PC build. It compiled 352 of 371 source
files, 2,471 functions, and left out:

- **narrowing**: 20 callers that keep a 64-bit result (`func_001F4868`
  returns a GS TEX0 value) in an `int`;
- **one name for several functions**: functions identical except for their
  globals share one catalogue name (29 left out; the fix is a name each);
- **stack layout**: 20 functions found from unoptimised LLVM IR as array or
  struct locals written but never read, plus three by hand
  (`func_L08_00309050`, `func_L14_002FDE28`, `func_L06_002EADC8`);
- **unset reads**: `func_L00_00263D68`, `func_L00_00265050`;
- **14 files with conflicting prototypes** (13 "conflicting types", one
  "too few arguments"), the same 14 on x86_64:
  `l01_novalis/vendor_002FABE8.c`, `l11_pokitaru/vendor_00312BD8.c`,
  `l13_gemlik/vendor_002EBD00.c`, `l14_oltanis/vendor_002ACCC0.c`,
  `l17_fleet/vendor_002F1558.c`, `l18_veldin2/vendor_002A8400.c`,
  `shared/help_0020CDF0.c`, `shared/help_00214D60.c`,
  `shared/mobyutil_0027C260.c`, `shared/vendor_002A1B58.c`,
  `shared/vendor_002A5218.c`, `shared/vendor_002C99E0.c`,
  `shared/vendor_002EB0D8.c`, `shared/vendor_002F7700.c`
  (all under `src/overlays/`). These are fixes for the decompilation itself.

It also wrote **31 of the VU0 and FPU helpers in C**
(`runtime/port/hand/rac1-pal/vector.c` and `small.c` in history), each
"operation for operation and in the same order, so that rounding agrees":
FastVecAdd, FastVecSub, lerp, FastVecScale (xyz and xyzw), FastVecDot,
FastVecLength (xyz and xy), FastVecDist (xyz and xy), normalise to a length
(and its level copy and xy form), 4 × 4 matrix by vector, 3 × 4 matrix by
vector, square root, clear 16 bytes, the packed-colour conversions both
ways, `scale_ticks` (the 60/50 Hz tick scale), float to int, FastAddRots and
FastSubRots (wrap to (−π, π]), FastTweenColor, two matrix copies, the
quaternion product, fmod and modf by truncation, a byte countdown, and a
64-bit field packer. They are a starting point for the port's versions of
the same functions. What it did not write: helpers that depend on VU0
register state an earlier call left (the cross product stores a fourth
field it never computed), and those that call VU0 microprograms (sine,
cosine, the matrix builders).

## 5. The platform layer

The game calls Sony's libraries by address name (`include/names.h` gives
405 readable names). [DECOMP_STATUS.md](DECOMP_STATUS.md), section 7, lists
them. Beyond the calls:

- **Threads and interrupts.** The game is single-threaded and polls.
  Threads and semaphores appear only in the movie player. The interrupt
  callbacks are the vsync callback `func_0012F308`, the VIF1 DMA handler
  `func_00235118` and the movie's vblank handler `func_0023C7A8`. Timer 1 is
  the frame clock.
- **DMA and scratchpad from C.** The display list at `D_00161000`; direct
  GIF transfers in `core/001224B0.c` and `core/00122818.c`; VU0 program
  uploads `func_002347F0`; a distance table copied to the scratchpad; 174
  functions touch hardware registers or the scratchpad, 103 of them
  assembly or not decompiled.

## 6. Choices for the port

The pointer model is the decision everything else follows. The options, as
the counts above bear on them:

| | Model | For | Against |
|---|---|---|---|
| a | Keep the console's 32-bit address space: the game's memory in one region below 4 GB, pointers that fit in 32 bits, the retail data layout reproduced | the C works nearly as written; global plus offset keeps working | not available in a native process on macOS on Apple Silicon (mapping below 4 GB fails; tested 2026-10-08); the data must be laid out exactly as retail |
| b | OpenGOAL's model: the game's memory is one buffer, and every game pointer is a 32-bit offset into it | portable to every platform; OpenGOAL's kernel works this way | every pointer use in the C becomes an offset access: a large source change, brittle given section 1 |
| c | Real C: typed structs with real pointers, data as typed C, the console's layout given up | the end state of a port; what makes the code maintainable | rewrites most of the code's data access; the matching build can no longer prove the PC code |

These are the options, not a decision: the maintainers choose
([DESIGN.md](DESIGN.md), open decisions). Whatever the model, the PC build
also needs:

1. **A header dialect** for the port: fixed-width types, a 16-byte struct
   for 128-bit values, `qcopy` and `qzero` as `memcpy` and `memset`, the
   matching attributes and the `INCLUDE_ASM` family defined away, flags
   `-fno-strict-aliasing -fwrapv -fsigned-char -std=gnu89`.
2. **The 1,934 symbol aliases** replaced by typed declarations, and the 14
   conflicting-prototype files and the left-out functions fixed in the
   decompilation itself.
3. **Hardware accesses** (about 200 sites in C) routed to the platform
   layer, with the uncached and physical address aliases treated as the
   same memory.
4. **A floating-point policy** (section 2).
5. **Data as C** (or built at load time from the player's disc), and a way
   for the 19 level programs to share one copy of the engine code with
   their own data and dispatch tables.
6. **Platform replacements** at the library calls, as OpenGOAL did: the
   frame's sync points become the renderer's, disc reads become file reads
   of the extracted assets, the memory card becomes save files, 989snd is
   replaced at its API, the pad library reads SDL.

## 7. Platform notes

What one platform ships differently from the others, and why.

- **Intel Macs ship without the probe baker** (decided by the maintainers).
  The baker, `openrac-probe-bake`, links Embree 4 as a static library
  ([port/enhanced/lighting](../../port/enhanced/lighting/README.md)), and
  with AppleClang Embree's static library takes a single ISA, while x86-64
  needs several (SSE2, AVX, AVX2: `EMBREE_X86_ISAS` in
  `tools/build_port.py`), so it cannot be built on an Intel Mac. There,
  `tools/build_port.py --probe-baker` neither downloads nor builds Embree:
  it says so in one sentence and builds the game and the extractor, a
  successful build. The launcher's Enhanced graphics say the same instead of
  offering a bake (`launcher/core/src/enhanced.rs`, `NO_BAKER_HERE`). The
  enhanced graphics there keep the original lighting: the port lights a level
  without a bake the way the game does
  (`port/enhanced/viewer/probe_lighting.cpp`).
  Apple silicon Macs, Windows and Linux build the baker. Each side reads the
  machine it runs as: `platform.machine()` for the build, the launcher's own
  target for the launcher.
