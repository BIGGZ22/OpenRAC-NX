# Toolchain

What builds the matching image today, and why each piece is the way it
is. The procedure (setup, iterate, verify, publish) is in
`docs/WORKFLOW.md`; how each choice was established is in the SOLVED
sections of `docs/DECOMP_PROGRESS.md`. How the project got here is kept
under "History" at the end of this file.

## Where the binaries come from

Two community mirrors of SN Systems / Sony PS2 toolchains, cloned into
`toolchain/` (gitignored and never committed; the clone commands are in
the README). They are third-party mirrors of commercial software.

| Binary | Mirror, as cloned | Used for |
|---|---|---|
| `bin/ee-gcc2953.exe`: GCC 2.95.3, **SN BUILD v1.14** | `sce_ps2_sdk_24` → `toolchain/sn-prodg-24/local/sce/ee/gcc/` | compiles all game code (`src/core/`, `src/game/`), both segments; also assembles every compiler-generated `.s` (`-c`) |
| `bin/ee-gcc.exe`: Sony's **gcc 2.9-ee-991111** (or native Linux ELF `bin/ee-gcc` in `toolchain/ee-gcc-2.9-991111-01/`) | decompme/compilers or `sce_ps2_sdk_24` | compiles libgcc (`src/libgcc/`) and `ee29` SDK objects (`src/core/`) to `.s` |
| `bin/ee-as.exe` | `SN-Systems-ProDG_for_PS2_3.01` → `toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/` | assembles the standalone data objects (`tools/build_sn_data.sh`) |
| `bin/ee-ld.exe` | same | links everything at retail addresses |
| `bin/make.exe` (GNU make 3.77) | same | runs `Makefile.sn` (on native Linux, host `make` is used directly) |
| `bin/ee-size.exe` | same | prints object sizes at the end of `make` |

The other SN sub-builds in the mirrors, v1.36 (`ee-gcc2953.exe` in
`sn-prodg-3.01`) and v2.74 (`ee-gcc295.exe` in `sn-prodg-24`), are not
used; see the next section.

## Compilers and flags

All game code is compiled with **`-O2 -G2 -Iinclude -Wa,-I,.`**.

- **v1.14 for both segments.** Retail's two code segments were built by
  two different SN sub-builds. `text` spills callee-saved registers with
  `sq`/`lq`, exactly as v1.14 does. `core_text` spills them with
  `sd`/`ld`: v1.36 emits those mnemonics but lays the save slots out
  mirrored, while v1.14 has retail's slot layout. The `core_text` code
  with `sd`/`ld` saves turned out to be Sony SDK code, built with the SDK's
  own 2.9-ee (the `ee29` objects), which emits them natively; the few game
  objects in `core_text` (crt0, boot, permcb, wad, 989snd and the one at
  0x1207B8) are built with v1.14 like `text`. No step narrows spills any
  more (docs/BUILD_FIDELITY.md, "Removed").
- **`-G2`, not `-G0`.** Retail's small-data threshold is between 1 and 3:

  | `-G` | float constants | small globals via `$gp` |
  |---|---|---|
  | `-G0` | inline | never, so no `$gp` function can match |
  | `-G1`..`-G3` | **inline** | **yes** |
  | `-G4`+ | pooled into `.lit4` | yes (and `.lit4` has nowhere to live: the small-data window is full) |

  Retail inlines float constants *and* uses `$gp`. Placement follows an
  extern's *declared* size, which is what `NOT_SDA` and `MACRO_ADDR` in
  `include/common.h` steer. See `notes/gp-investigation.md`.
- **`-Wa,-I,.`**: plain `-I` only reaches the preprocessor. The assembler
  needs its own include path to resolve the `.include "asm/..."` that
  every `INCLUDE_ASM` expands to.
- **libgcc** is built by Sony's 2.9-ee through its **driver**, never `cc1`
  directly: the driver passes the target predefines (`__mips__`,
  `__R5900__`, ...) that `longlong.h` picks its MIPS multiply and divide
  primitives from. It runs with `-O2 -G2 -S`, and v1.14's driver assembles
  the result. See `src/libgcc/README.md`.

### Native Linux vs Wine / Containers

- **SN Systems ProDG**: The original retail compiler suite was only ever distributed
  as 32-bit Windows x86 PE binaries (`ee-gcc2953.exe`, `ee-as.exe`, `ee-ld.exe`). There
  is no native Linux build of SN ProDG in existence.
  - However, **native Linux execution without containers** is fully supported via `Wine`:
    `tools/toolchain.sh` auto-detects `Linux` and uses host `WINE=wine` and host `make`.
  - The container (`tools/docker/run.sh`) is provided for platforms without 32-bit Wine
    (such as macOS Apple Silicon or immutable container hosts like Fedora CoreOS).
- **Sony EE-GCC 2.9-991111-01**: Unlike SN ProDG, Sony's compiler exists as a native
  32-bit Linux ELF binary (`toolchain/ee-gcc-2.9-991111-01/bin/ee-gcc`). When present,
  `Makefile.sn` automatically runs this native Linux binary directly.

### Distinguishing Sony 2.96 vs SN ProDG in SDK / newlib

In retail ELF libraries and SDK code (e.g. `boot_elf`, newlib), code built with
Sony's GCC (such as 2.96 or 2.9-ee) can be distinguished from SN Systems ProDG code
by instruction scheduling heuristics:
- **`div.s` / `sqrt.s` 2-nop padding**: Sony GCC / GAS inserts 2 `nop` instructions
  after `div.s` and `sqrt.s` before the result is read, reflecting hardware pipeline
  hazard mitigation. SN ProDG schedules independent instructions or uses different nop counts.
- **`ee29` marker**: SDK objects compiled with Sony 2.9-ee are designated with `ee29` in
  the 3rd column of `config/core_text.objects`.


## What happens to each object

`Makefile.sn` takes the link order and object start addresses from
`config/core_text.objects` and `config/text.objects`.

| Source | Steps |
|---|---|
| `src/core/<ADDR>.c` (`core_text`) | v1.14 `-S` → `tools/check_macro_slots.py` → assemble (989snd and wad also `tools/fix_macro_load_delay.py` and `tools/ps2eeas_nops.py`) |
| `src/core/<ADDR>.c` marked `ee29` in `config/core_text.objects` (the memory card library, the C library's printf, sprintf, stdio and strtol, libmpeg's bitstream reader, ...) | 2.9-ee `-S` (with its own include directory, for `stdarg.h`) → `tools/fix_volatile_slot.py` → `tools/check_macro_slots.py` → assemble; SDK code built with the SDK's own compiler, like libgcc (`EE29_CORE` in `Makefile.sn`). 2.9-ee spills with `sd` and tail-calls a void function ending in a call by itself, but not `return f(...)` |
| `src/game/**.c` (`text`) | v1.14 `-S` → `tools/fix_jump_tables.py` → `tools/ps2eeas_dli.py` → `tools/check_macro_slots.py` → `tools/fix_orphan_hi.py` → assemble → `tools/ps2eeas_nops.py` → assemble |
| `src/libgcc/libgcc2.c` | 2.9-ee `-S`, one object per `L_*` module, like `libgcc.a`'s members → assemble; L__main also goes through `tools/strip_dead.py` |
| `src/libgcc/fp-bit.c` | 2.9-ee `-S`, whole file twice (`dp-bit.o`, `fp-bit.o` with `-DFLOAT`) → assemble → `tools/strip_dead.py` → assemble |
| `src/libgcc/nonmatching_*.c` | asm stubs for the modules that do not match yet, and for linker fill |
| `asm/data/*.s` | `ee-as.exe` directly (`tools/build_sn_data.sh`); `tools/split_data_s.py` cuts `core_rdata` around the read-only data compiled objects provide (`config/core_rodata.txt`: `__divdi3`'s `__clz_tab`, SDK functions' literals and jump tables) and `data` around the jump tables compiled game functions bring (`tools/jump_tables.py`). Every piece after a cut places its blocks with `.org` at their retail offsets, and a `jtbl_` cut keeps any data splat merged after the table |

Then `rac1.ld.sh` writes `build-sn/rac1.ld`, placing every object at its
retail address, `tools/gen_bss_equs.py` supplies the bss-only symbols, and
`ee-ld.exe` links `build-sn/rac1.elf`. `tools/build_sn.sh` runs all of it
from scratch and finishes with the audit (`tools/sweep_matches.py`,
`tools/check_layout.py`, and `tools/check_image.py`, which compares every
loaded section with retail and allows differences only inside decompiled
near-misses).

## The post-processors

Each one models something retail's assembler or linker did that ours does
not; none changes an instruction the compiler wrote.
[BUILD_FIDELITY.md](BUILD_FIDELITY.md) is the authoritative list, with the
evidence for each step, how many matches depend on it, and the rules for
adding one (`tools/check_build_fidelity.py` enforces them). Every source
file is compiled with the flags above plus any that `config/file_cflags.txt`
gives that whole file; there are no per-function flags.

- **`tools/ps2eeas_nops.py`** (game code only): adds the nops SN's own
  assembler, `ps2eeas`, added to retail's text segment and GNU as does
  not. It pads every loop shorter than six instructions before its
  backward branch (the R5900 short-loop erratum), and puts a nop between
  an FP compare and a directly following `bc1` (never adjacent in retail
  text, 191 of 191). Two-pass: both are measured in a first assembly. See
  "SOLVED: the short-loop erratum" in `docs/DECOMP_PROGRESS.md`.
- **`tools/ps2eeas_dli.py`** (game code only): rewrites each `dli` into
  the instructions ps2eeas uses for that 64-bit constant. GNU as picks a
  different sequence for many values (`0x8000000044`: ps2eeas `ori 0x8000;
  dsll 24; ori 0x44`, GNU `addiu 0x80; dsll32 0; ori 0x44`). The algorithm
  was reconstructed from ps2eeas's output and checked on 871 constants.
- **`tools/strip_dead.py`**: removes a function the way retail's linker
  dead-stripped unreferenced code: its first `floor(size/8)*8` bytes, so a
  function of size 4 mod 8 leaves its last word (optionally named, e.g.
  `func_0011DF10`) and one of size 0 mod 8 vanishes. It reads the sizes
  from a first assembly of the same file. Used for libgcc's L__main,
  dp-bit.o and fp-bit.o; see "Retail's linker dead-stripped unreferenced
  functions" in `docs/DECOMP_PROGRESS.md`.
- **`tools/check_macro_slots.py`**: a `MACRO_ADDR` global access that the
  compiler put in a branch delay slot is rewritten to the `$gp`-relative
  form retail's toolchain produced there. Anything it cannot handle (an
  `la` in a slot) fails the build, and a symbol outside the ±32 KiB
  small-data window fails the link loudly (`R_MIPS_GPREL16` truncated).

## Assembler and linker quirks the build relies on

- **Numeric register names.** SN's `ee-as` rejects `$ra`, `$sp`, `$t6`
  and so on. `tools/sn_regnames.py` rewrites `asm/` to `$31`, `$29`,
  `$14` (VU registers stay symbolic). `tools/setup_asm.sh` runs it, and
  so does every `make` (the `regnames` target); it is idempotent.
- **`.set noreorder` / `.set noat` around every stub.** Without them
  `ee-as` moves a different instruction into a delay slot than retail
  had. `INCLUDE_ASM` (`include/include_asm.h`) wraps each `.s` in both.
- **No `.aent`.** This GAS build rejects it, so `alabel` in
  `include/labels.inc` omits it. It is debug information only and emits
  no bytes.
- **`jlabel` is global.** A jump table can live in a different object
  (rodata) from the code that uses it, and a `.local` symbol cannot
  satisfy a reference from another object.
- **A decompiled `switch` brings its own jump table.** Retail keeps the
  text objects' read-only data (strings, jump tables) at the end of the
  `data` segment. `tools/fix_jump_tables.py` gives each compiled table a
  section named after the retail table it replaces
  (`.rodata.jtbl_001E8C90`), `tools/build_sn_data.sh` cuts those `jtbl_`
  blocks out of `asm/data/data.data.s` (`tools/split_data_s.py`), and
  `rac1.ld.sh` links the pieces and the tables in address order.
  `tools/jump_tables.py` derives the list from the decompiled functions,
  so a new switch needs no manual step. Nothing places a string literal
  yet, so game code declares its strings `extern` (fix_jump_tables stops
  the build on one).
- **Denormal floats are written as words.** The assembler reads
  spimdisasm's `.float 1.401298464e-45` (the word 1) back as 0.
  `tools/setup_asm.sh` runs `tools/fix_denormal_floats.py`, which writes
  every denormal as a `.word` from its raw bytes.
- **No `NOLOAD`.** `ee-ld.exe` does not advance the location counter after
  a `NOLOAD` section, so the next section silently overlaps it. The bss
  regions are therefore real, zero-filled sections (`.skip` in the
  `core_bss_pad`/`bss_pad` objects that `tools/build_sn_data.sh` writes).
- **bss symbols by address.** Symbols that exist only as bss are equated
  from their splat names (`D_0015ED10` → 0x0015ED10) by
  `tools/gen_bss_equs.py`. `tools/build_sn.sh` takes the undefined names
  from the objects' own symbol tables (`tools/list_undefined.py`), not from
  ld's errors: this ld crashes ("Unhandled illegal instruction") on a long
  run of undefined references, sometimes before printing any.
- **libgcc names for assembly modules.** libgcc modules that still build
  from retail's assembly define only their address names; `rac1.ld.sh`
  maps `__moddi3`, `__udivdi3` and `__umoddi3` to them, for compiled C
  that calls them by name.
- **Delay slots are filled only from after a branch.** The assembler
  inserts delay-slot `nop`s that are not in the compiler's `.s`, so count
  instructions in the linked ELF (`tools/diff_words.py`), never in `.s`.
  See "SN's assembler fills delay slots only from AFTER the branch" in
  `docs/DECOMP_PROGRESS.md`.
- **VU0 macro-mode instructions** assemble natively with `ee-as`.
  `tools/setup_asm.sh` still runs `tools/fix_vu0_macro.py`, which writes
  them as `.word` with the instruction's own encoding; the bytes are
  identical either way.

## Running it on macOS and Linux

The toolchain is 32-bit Windows programs that import nothing but
`KERNEL32.dll` (the C runtime is linked in statically), so Wine runs them
unchanged. `tools/toolchain.sh`, and `tools/toolchain.py` for the Python
tools, choose how: on Windows the programs run directly and the build
uses SN's `make.exe`; anywhere else each program runs through `wine` and
the build uses the host's GNU make with `-j`. `WINE=...` and
`MAKE_SN=...` override either.

`tools/docker/` packages that as an image: 32-bit (`linux/386`) Debian
bookworm with classic 32-bit Wine, make, and a Python venv holding the
pinned `requirements.txt` plus the asm-differ and m2c prerequisites.
`bash tools/docker/run.sh <command>` builds it on first use and runs the
command with the repository mounted at the same path. A Linux x86 host
with 32-bit Wine installed can run the scripts directly instead.

Why a 32-bit container rather than an amd64 one:

- **Rosetta cannot run 32-bit x86 code under Linux.** Wine's WoW64 mode,
  the only way a 64-bit Wine runs 32-bit programs, switches to the
  32-bit code segment (selector `0x23`). Measured in an OrbStack amd64
  machine: a minimal program that does that dies with `rosetta error:
  invalid gdt selector index 4`, and Wine never finishes setting up its
  32-bit half, so every toolchain program fails to start. A `linux/386`
  container instead runs entirely under QEMU's user-mode emulator, where
  everything is 32-bit and Wine works.
- **Native macOS Wine is not a good default either.** Homebrew disabled
  its Wine casks on 2026-09-01 (they do not pass Gatekeeper), and macOS
  27 is the last release with full Rosetta 2.

Measured on an Apple M5 Max (18 cores) under QEMU emulation:
`tools/setup_asm.sh` takes about 1.5 minutes, one small object about 3
seconds, and `tools/build_sn.sh` (clean build of every object, link and
audit, `make -j18`) about 37 seconds. The first image build takes about 15
minutes, most of it compiling Levenshtein for asm-differ.

**Verified equivalent.** On 2026-09-23 a from-scratch container build
reproduced the committed `progress/report.json` byte for byte: all 1,688
functions' match percentages, generated on Windows, came out identical
(485 exact, 81 same-size near-misses, 0 size mismatches).

## History

How the toolchain was found. Several measurements here are still cited
elsewhere, so the text is kept as written; wherever it disagrees with the
sections above, the sections above are current.

### First attempt: modern ps2dev GCC under WSL (removed)

GCC 15.2 from ps2dev cannot produce matching code; the SN toolchain
replaced it, and its `Makefile`, `tools/build.sh` and
`tools/setup_ps2dev.sh` were removed on 2026-09-23. What survives from it
is `tools/fix_vu0_macro.py` (see "The post-processors").

### Update: found the real era-accurate compiler

*(Its assembler findings still hold. This mirror still provides the
`ee-as` that assembles the data objects, plus `ee-ld` and `make`. Game
code, however, is now compiled by v1.14 from the other mirror, not by
this v1.36; see the top of this file.)*

`toolchain/sn-prodg-3.01/` (gitignored — see below) is a clone of
[AngheloAlf/SN-Systems-ProDG_for_PS2_3.01](https://github.com/AngheloAlf/SN-Systems-ProDG_for_PS2_3.01),
a mirror of SN Systems' **ProDG for PS2 3.01**: `ee-gcc2953.exe`, a real
2002-era **GCC 2.95.3 (SN BUILD v1.36)**, with its own native-Windows
`ee-as.exe`/`ee-ld.exe`/`ee-objdump.exe`. This is the standard way PS2
matching-decomp projects (this game very likely included — SN ProDG was
extremely common for this era/region) obtain an era-correct compiler. It's
a mirror of old **commercial** software, not open source — kept local only
(`toolchain/` is gitignored, same treatment as `baserom/`), never
committed.

Two real findings testing `ee-as.exe` directly against our disassembly
(see `tools/sn_regnames.py` and the verification below):

1. **It natively supports VU0 macro-mode COP2 instructions** (`vaddq`,
   `vmulax`, `vdiv`, the whole accumulate-register family) that modern
   binutils 2.45.1 can't assemble at all. No `.word`-encoding workaround
   needed with this toolchain — `tools/fix_vu0_macro.py` was a
   modern-binutils-specific stopgap, not a fundamental limitation of the
   game's instruction set.
2. **It doesn't recognize symbolic GPR names** (`$ra`, `$sp`, `$t6`) —
   only numeric (`$31`, `$29`, `$14`). `tools/sn_regnames.py` does that
   translation (VU float regs like `$vf5`/`Q`/`ACC` are untouched, those
   work symbolically already).
3. **It reorders branch delay slots by default** unless `.set noreorder`
   (and `.set noat`) are active — without them it happily moves a
   *different* instruction into a delay slot than the original had. Every
   nonmatching function needs those two directives active before its body
   (`include/labels.inc`'s macros assume this is already the case, per its
   own header comment: "This file is used by the original
   compiler/assembler").

**Verified**: took `func_00125298` (one of the VU0-instruction functions),
applied `sn_regnames.py`, wrapped it in `.set noat` / `.set noreorder`,
assembled with `ee-as.exe`, and diffed all 26 instruction words against
the raw bytes spimdisasm recorded from the retail binary —
**zero mismatches, byte-for-byte identical**, including every VU0
instruction. This is strong evidence this is the right toolchain family
for actual matching decompilation of this game, not just something that
happens to assemble.

### Update: the C compiler side is wired up and looks very promising (superseded)

*(Superseded: the build now compiles both segments with v1.14 at `-G2`
with the post-processors above, and `src/core_text.c` / `src/text.c`
have been split into `src/core/` and `src/game/`.)*

`Makefile.sn` builds every `src/*.c` through `ee-gcc2953.exe` directly
(`-O2 -G0 -Iinclude -Wa,-I,.` — the `-Wa,-I,.` is required: plain `-I`
only affects the C preprocessor, not where the assembler resolves the
`.include` paths inside each `INCLUDE_ASM`-pulled `.s` file). Run it with
the SN toolchain's own bundled `make.exe` (this environment's git-bash has
no `make` on PATH):

```
toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin/make.exe -f Makefile.sn
```

One more fixup was needed: `include/labels.inc`'s `alabel` macro (used for
72 functions with an alternate entry point) emitted `.aent`, which this
GAS build doesn't implement (`Unknown pseudo-op`) — removed it, since it's
only a debug-info marker and doesn't affect emitted bytes.

Both objects now build clean. `tools/check_match.py` compares an object's
`.text` bytes directly against the retail baserom's corresponding ELF
section:

```
build-sn/core_text.o vs retail 'core.text':  size 119296 vs 119288, 6.05% byte mismatch
build-sn/text.o      vs retail '.text':      size 349872 vs 349872 (EXACT), 7.02% byte mismatch
```

`text.o`'s size is an **exact** match already. The remaining ~6-7% of
bytes differ, but the very first mismatch in `core_text.o` is a `jal`
target address — expected, since nothing is linked yet (every function is
still assembled as an independent standalone object; call targets and
data references have no real relocated address to encode). This is not
proof of a full function match anywhere yet, but it's a strong signal the
codegen itself (instruction selection, register allocation, scheduling)
is landing very close to the original, not just "an object file that
happens to assemble."

### Update: linked, whole-binary result — 99.99% byte-exact

*(The first link. `tools/build_sn.sh` replaces the manual commands at
the end, and the per-section numbers are from that first link.
Re-measured on 2026-09-23: the data residual is unchanged, 84 bytes in
`.data` and 1 in `.lit`; every other differing byte in the image now
belongs to one of the kept same-size near-misses in `.text` and
`.core_text`.)*

`rac1.ld.sh` generates `build-sn/rac1.ld` from the same addresses as
`config/splat.yaml`, placing every object (both C files, all 8
data/rodata objects, plus bss padding) at its real retail address, and
`tools/gen_bss_equs.py` resolves the ~240 symbols that only exist as bss
variables (never declared anywhere as real data) by parsing their address
straight out of the splat-assigned name (`D_0015ED10` -> `0x0015ED10` —
splat's naming convention makes the address self-describing for anything
not yet manually analyzed).

Two more toolchain bugs found and worked around along the way:

- **`jlabel`'s default `local` visibility breaks cross-object jump
  tables.** `include/labels.inc` had `jlabel` default to
  `visibility=local` (splat's own default). A `.local` symbol can't
  satisfy an undefined reference from a *different* object at link
  time — and a function's jump table often lives in a separate rodata
  object from the code that uses it. Changed the default to `global`.
- **This `ee-ld.exe` (v2.3.7.513-era) doesn't advance the location
  counter after a `NOLOAD` section.** An explicit `. = X;` placed right
  after a `SECTIONS` entry marked `(NOLOAD)` is silently ignored — the
  next real section lands back at the `NOLOAD` section's own start
  address instead of `X`, silently overlapping everything after it.
  Worked around by making bss regions **real, zero-filled loaded
  sections** (`.skip N` in an assembled object) instead of `NOLOAD` — see
  `core_bss_pad`/`bss_pad` in `tools/build_sn_data.sh`.

Result, comparing every linked section's bytes directly against the
retail ELF's corresponding section:

```
.core_text    0/119288   (0.00%) mismatch  -- EXACT
.core_data    0/142656   (0.00%) mismatch  -- EXACT
.core_rdata   0/7840     (0.00%) mismatch  -- EXACT
.core_lit     0/608      (0.00%) mismatch  -- EXACT
.lit          1/9008     (0.01%) mismatch
.data         84/538920  (0.02%) mismatch
.lvl_vtbl     0/12       (0.00%) mismatch  -- EXACT
.lvl_camvtbl  0/20       (0.00%) mismatch  -- EXACT
.lvl_sndvtbl  0/8        (0.00%) mismatch  -- EXACT
.text         0/349872   (0.00%) mismatch  -- EXACT

TOTAL: 85/1168232 (0.01%) mismatch
```

Every mismatched byte is a small integer off by exactly 1 (e.g. retail
`0x01` vs. ours `0x00`, retail `0x80` vs. ours `0x7f`) — the signature of
a count/size field computed as an address *difference* against one of
`gen_bss_equs.py`'s approximated placeholder addresses, not a real
codegen difference. **Every code section (`.text`/`.core_text`) is a
100% exact match already.**

To reproduce the full build + link + verify:

```
toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin/make.exe -f Makefile.sn
bash tools/build_sn_data.sh
bash rac1.ld.sh
toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin/ee-ld.exe \
    -T build-sn/rac1.ld build-sn/bss_equs.o -o build-sn/rac1.elf
```

Not yet done: chasing the remaining 85-byte residual to zero (would need
real bss symbol declarations with correct sizes rather than
address-guessed placeholders); STL/runtime header availability for
anything beyond plain C; actually decompiling any function into real
(non-`INCLUDE_ASM`) C and confirming it individually matches — this
result proves the *toolchain and disassembly* are sound, not that any
particular function has been understood/renamed/rewritten yet.
