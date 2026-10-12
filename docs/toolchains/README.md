# Toolchains across the games

What the four projects know about the compilers, assemblers and linkers that
built each game, and what each project uses to match. Every statement links to
the per-game file it comes from. Where the projects disagree, this page gives
both readings and does not pick one. The download commands are in
[toolchains/README.md](../../toolchains/README.md). Toolchains are proprietary
and never committed ([SOURCING.md](../policy/SOURCING.md)).

The measurements were taken in September and October 2026. Treat counts as
dated: the per-game files are current.

Names used on this page:

- **SN 2.95.3 v1.14 / v1.36**: `ee-gcc2953.exe`, "GCC 2.95.3 (SN BUILD v1.xx)", from SN ProDG packages.
- **SN 2.95.2 v2.73a**: `ee-gcc295.exe` from the SN ProDG 2.0 package.
- **2.9-ee**: the Sony/Cygnus GNU EE compiler `2.9-ee-991111` and its variants (`991111a`, `991111b`, `991111-01`).
- **Ps2EeAs**: SN's own EE assembler, `ee/bin/Ps2EeAs.exe` (ps2eeas 1.9.25.758 in ProDG 3.01).
- **ee-as / as.exe**: the GNU assemblers in the SN packages, `bin/ee-as.exe` and `ee/bin/as.exe`.

## 1. Summary

### Compilers and flags

| Game | Game code | Flags | SDK and runtime code | How far it is proven |
|---|---|---|---|---|
| RAC1 PAL `SCES_509.16` ([games/rac1/pal](../../games/rac1/pal/README.md)) | SN 2.95.3 v1.14 for game code in both segments; nothing edits its output ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)) | `-O2 -G2 -Iinclude -Wa,-I,.` ([Makefile.sn](../../games/rac1/pal/Makefile.sn)), plus options for whole files only ([file_cflags.txt](../../games/rac1/pal/config/file_cflags.txt): `-mno-split-addresses`, `-fno-force-mem`, `-fno-schedule-insns`, `-fno-sched-interblock`) | Sony 2.9-ee-991111 (`ee-gcc.exe`) for objects marked `ee29` ([core_text.objects](../../games/rac1/pal/config/core_text.objects)) and for libgcc; a Linux 2.9-ee-991111-01 built from source for its last three modules | Whole image links at retail addresses. Container build reproduced the Windows report byte for byte ([TOOLCHAIN.md](../../games/rac1/pal/docs/TOOLCHAIN.md)) |
| RAC1 NTSC-U `SCUS_971.99` ([games/rac1/ntsc](../../games/rac1/ntsc/docs/building.md), Lombyte) | "Game compiler": GNU EE 2.9-ee-991111b rebuilt from source with the production patch stack `0000`..`0056` ([patches/sce-991111b](../../games/rac1/ntsc/patches/sce-991111b/README.md)) | `-O2`, no `-g`, no `-G` option; per-unit flags in `GAME_COMPILER_FLAG_UNITS` ([configure.py](../../games/rac1/ntsc/configure.py)) | EE-GCC 2.9-ee-991111-01 ("SDK compiler") for every unit below `GAME_TEXT_START = 0x12D8F8`, with `-DMATCHING_DECOMP -O2 -g2 -gstabs` ([configure.py](../../games/rac1/ntsc/configure.py)) | 1258 of 1331 C units byte-identical on their placement compiler (2026-09-27); 18 units in `ROUTE_EXCEPTIONS` still use another route |
| RAC2 NTSC-U v1.01 `SCUS_972.68` ([games/rac2/ntsc](../../games/rac2/ntsc/README.md)) | Game code: 2.9-ee-991111b, the Lombyte stack minus its save widening, plus the measured transformers ([tools/compiler/](../../games/rac2/ntsc/tools/compiler/README.md)); SDK units: Sony's 2.9-ee-991111-01, unmodified ([tools/compiler/sdk/](../../games/rac2/ntsc/tools/compiler/sdk/build.py)) | Per file ([config/cflags.txt](../../games/rac2/ntsc/config/cflags.txt)): `-O2 -G0 -ffunction-sections` by default | Checked by output | Every C function links into a program that equals retail (`tools/build.py all`) |
| RAC3 NTSC-U `SCUS_973.53`, `frontbin.elf` ([games/rac3/ntsc](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md)) | SN 2.95.3 v1.36 | `-O2 -G8 -fopt-stack -mno-check-zero-division` on every range. `-mno-split-addresses` on 110 and `@ps2as` on 112 of the 358 lines of [text_parts.txt](../../games/rac3/ntsc/tools/text_parts.txt) | Not measured | 2043/2043 text functions byte-identical in a full build (2026-09-25) ([compiler_matrix_findings.md](../../games/rac3/ntsc/docs/compiler_matrix_findings.md)) |
| RAC4 Deadlocked `SCUS_974.65` ([games/rac4/ntsc](../../games/rac4/ntsc/docs/RESEARCH.md)) | SN 2.95.3 v1.36 | `-O2 -G8 -fopt-stack -mno-check-zero-division`; `-mno-split-addresses` per file, named in a `/* cflags: */` comment ([build.sh](../../games/rac4/ntsc/tools/build.sh)) | Sony 2.9-ee-991111 with `-O2 -G2` for libgcc and libm | Per function only, with most immediates masked and no linked image. Chosen from five functions: v1.36 gave 4 of 5 exact, 2.9-ee 3, SN v1.14 and SN 2.74 one each ([RESEARCH.md](../../games/rac4/ntsc/docs/RESEARCH.md#the-compiler)) |

### Assembler, linker, host, sources

| Game | Assembler | Linker | Host | Where the binaries come from | Pinned by checksum |
|---|---|---|---|---|---|
| RAC1 PAL | Compiled code: the GNU as that the v1.14 driver calls (`-c`), with `ps2eeas_nops.py` and `ps2eeas_dli.py` imitating Ps2EeAs for game code (each such step listed with its evidence and the matches that depend on it in [BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)). Data: `ee-as.exe` (ProDG 3.01) | `ee-ld.exe` (ProDG 3.01); GNU make 3.77 on Windows | Windows (Git Bash), or Docker/Podman `linux/386` Debian bookworm with classic 32-bit Wine, emulated on ARM Macs ([Dockerfile](../../games/rac1/pal/tools/docker/Dockerfile), [CONTAINERS.md](../../games/rac1/pal/docs/CONTAINERS.md)) | Two community mirrors cloned into `toolchain/`: `SN-Systems-ProDG_for_PS2_3.01` and `sce_ps2_sdk_24` ([README.md](../../games/rac1/pal/README.md)) | Executable SHA-1. The clone commands name no commit or checksum. |
| RAC1 NTSC-U | Game code: Ps2EeAs 1.9.25.758 from ProDG 3.01 with its divbug padding patched out (6 bytes, [patch-ps2eeas.py](../../games/rac1/ntsc/scripts/patch-ps2eeas.py)). `INCLUDE_ASM` wrappers: the patched GNU as of the 991111b tree | `mips-ps2-decompals-ld` (binutils-mips-ps2-decompals v0.10) | Linux x86-64 with glibc 2.38+ (Ubuntu 24.04+, Debian 13+) or WSL; Docker `linux/amd64` Ubuntu 24.04 elsewhere; Wine for the Windows tools; `gcc -m32` to build the compiler ([setup.sh](../../games/rac1/ntsc/setup.sh)) | Downloaded by `setup.sh`. Game compiler built from `gnu-ee-binutils-gcc-1.1.tar.gz`, an Internet Archive copy of the ps2dev download ([build-game-compiler.py](../../games/rac1/ntsc/scripts/build-game-compiler.py)) | Every download by SHA-256: binutils, objdiff, SDK compiler, SN 2.95.2 archive, Ps2EeAs before and after the patch, two GCC headers, source archive, bison 1.28. Patch files by SHA-256. The built `cc1` hash is reported, not enforced, because it depends on the host |
| RAC2 | C: the patched GNU `as` of the same tree (`cda1a4e4…`). Reconstructed assembly: `Ps2EeAs.exe` from ProDG 2.0 ([build.py](../../games/rac2/ntsc/tools/build.py)) | `ee/bin/ld.exe` | 32-bit Linux binaries rebuilt from source ([tools/compiler/](../../games/rac2/ntsc/tools/compiler/README.md)); any host through the project's linux/386 Docker image | ProDG 2.0 supplied locally. Compiler built from the same archive as RAC1 NTSC-U | the five patched sources' SHA-256 (the build stops otherwise); source archive SHA-256; a compiler is checked by its output |
| RAC3 | `bin/ee-as.exe` (Aug 2000) by default; `ee/bin/Ps2EeAs.exe` per range (`@ps2as`); `ee/bin/as.exe` (May 2001) per range (`@newas`) | `bin/ee-ld.exe`, then `ee-objcopy` | Windows with SN `make.exe`; Linux and macOS through wibo 1.0.0-beta.1 ([build.py](../../games/rac3/ntsc/tools/build.py)) | SN ee-gcc 2.95.3 v1.36 package; [Setup](../../games/rac3/ntsc/docs/wiki/Setup.md) points at the `SN-Systems-ProDG_for_PS2_3.01` mirror | `frontbin.elf` SHA-1, on input and on output. Toolchain not pinned |
| RAC4 | The v1.36 driver's GNU as, with rac1/pal's `ps2eeas_dli.py` and `ps2eeas_nops.py` imitating Ps2EeAs (minimum loop span 5) ([cc.sh](../../games/rac4/ntsc/tools/cc.sh)) | None yet: nothing is linked | Docker/Podman `linux/386` with Wine, rac1/pal's image | The same two mirrors as RAC1 PAL ([CONTRIBUTING.md](../../games/rac4/ntsc/CONTRIBUTING.md)) | Boot executable SHA-1; the rebuilt ELF's SHA-1 in `config/splat.yaml`; the newlib snapshot commit |

## 2. Per game

### RAC1 PAL (rac1-decomp)

- **Two SN sub-builds, told apart by spills.** Retail `text` spills
  callee-saved registers with `sq` 428 times and `sd` 34 times; `core_text`
  is the other way round, 14 against 234. 520 `text` functions save `$ra`
  with `sq`, which v1.36 never emits
  ([Makefile.sn](../../games/rac1/pal/Makefile.sn),
  [sqlq-investigation.md](../../games/rac1/pal/notes/sqlq-investigation.md)).
  v1.14 lays the save slots out as retail does, with `$16` at 0, the s-registers
  ascending and `$31` at the top. v1.36 reverses it, with `$31` at 0 and the
  s-registers descending. The `core_text` objects that save with `sd` are
  Sony SDK code built with 2.9-ee, which emits `sd` natively; the game-code
  core objects save with `sq` like `text`. The step that once narrowed v1.14's
  `sq` to `sd` (`fix_core_spills.py`) carried no match and is gone (2026-10-07,
  [BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)). A search of
  `cc1.exe`'s whole `target_switches` table found no flag that changes either
  behaviour ([DECOMP_PROGRESS.md](../../games/rac1/pal/docs/DECOMP_PROGRESS.md),
  "Open toolchain questions").
- **SDK code is 2.9-ee.** 452 core functions match members of the SDK
  archives in `sce_ps2_sdk_24` exactly. Every core object is built with
  2.9-ee except 989snd, `boot`, `permcb`, `wad` (game code, SN) and `crt0`
  (assembly); `001207B8` (libsn's `vu.o`) is undecided. 989snd scored 25/25
  under 2.95.3 and 3/25 under 2.9-ee
  ([DECOMP_PROGRESS.md](../../games/rac1/pal/docs/DECOMP_PROGRESS.md),
  "Open: core_text prologue scheduling").
- **libgcc** is built from GCC's `libgcc2.c` (trunk 1999-11-02) and `fp-bit.c`
  through the 2.9-ee driver, not `cc1`. The driver supplies the
  `__mips__`/`__R5900__` predefines that `longlong.h` needs. Sony's objects
  were built on Linux, while the mirrors hold a Windows `2.9-ee-991111b/r4`
  `cc1`, which left three modules (`__moddi3`, `__udivdi3`, `__umoddi3`)
  differing in unused stack slots. A Linux 2.9-ee-991111-01 built from
  source matches them too, so every libgcc module is exact (2026-10-07,
  [src/libgcc/README.md](../../games/rac1/pal/src/libgcc/README.md),
  [TOOLCHAIN.md](../../games/rac1/pal/docs/TOOLCHAIN.md)).
- **`-G2`.** At `-G0` nothing uses `$gp`, and at `-G4` and above float
  constants are pooled into `.lit4`, which retail never does. Retail does both
  inline floats and `$gp`, so the threshold is 1 to 3. Placement follows the
  declared size: `MACRO_ADDR` and `NOT_SDA` steer it
  ([gp-investigation.md](../../games/rac1/pal/notes/gp-investigation.md)).
- **Assembler.** Retail `text` has no unpadded short loop, so it was assembled
  by Ps2EeAs; `core_text` by the driver's `as.exe`. Ps2EeAs cannot replace
  GNU as in this build. It recurses on some stubs, and it has no `-G`
  expansion: a text build with it scored 385 exact against 503, with 11 size
  mismatches. Its effects are reproduced instead: short-loop padding
  (`ps2eeas_nops.py`) and 64-bit `dli` sequences (`ps2eeas_dli.py`, checked on
  871 constants) ([DECOMP_PROGRESS.md](../../games/rac1/pal/docs/DECOMP_PROGRESS.md),
  "SOLVED: the short-loop erratum"). Retried on 2026-10-07 with files that are
  C throughout: Ps2EeAs 1.9.6.516 and 1.9.25.758 both overflow their stack on
  v1.14's output, and v1.14 ignores `-mgpopt`. Measured with each step as a
  pass-through, the padding, `dli` and delay-slot `$gp` steps together carry
  1,078 matched functions, 46.1% of the code
  ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)).
- **Build fidelity (2026-10-07).** No step edits what the compiler emitted,
  and options apply to whole files, the only granularity GCC 2.95 has
  ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)). The
  rewriters `fix_tail_calls.py`, `fix_trunc_slot.py` and `fix_core_spills.py`
  and the per-function flags were removed after measuring what depended on
  them: 12 functions, 0.06% of the code; seven keep their flag as a file's,
  five are assembly again. The remaining steps only model the assembler and
  linker (`ps2eeas_nops.py`, `ps2eeas_dli.py`, `check_macro_slots.py`,
  `fix_orphan_hi.py`, `fix_macro_load_delay.py` for 989snd and `wad`,
  `fix_volatile_slot.py` for 2.9-ee objects, `fix_jump_tables.py`,
  `strip_dead.py`), each with its evidence and measured dependence;
  `tools/check_build_fidelity.py` fails `--check` on anything else. GCC 2.95
  has no sibling calls (`-foptimize-sibling-calls` is rejected by both SN
  sub-builds), so a bare `j` tail call outside the 2.9-ee objects cannot be
  matched in C with v1.14.
- **Assembler and linker quirks.** SN `ee-as` accepts only numeric register
  names. Stubs need `.set noreorder`/`.set noat`. It has no `.aent`.
  `ee-ld.exe` does not advance after a `NOLOAD` section, and it crashes on a
  long run of undefined references ([TOOLCHAIN.md](../../games/rac1/pal/docs/TOOLCHAIN.md)).
- **C or C++.** The retail strings name `.cpp` files. Compiling as C++ with
  cc1plus 2.95.3 v1.14 gave 239 exact against 240 as C, so the language alone
  is not a lever ([DECOMP_PROGRESS.md](../../games/rac1/pal/docs/DECOMP_PROGRESS.md)).
- **Hosts.** Rosetta cannot run Wine's 32-bit code in an amd64 Linux machine
  (`rosetta error: invalid gdt selector index 4`, measured in OrbStack). This
  is why the image is `linux/386` under QEMU
  ([TOOLCHAIN.md](../../games/rac1/pal/docs/TOOLCHAIN.md)).

### RAC1 NTSC-U (Lombyte)

- **The compiler follows placement.** The SDK libraries sit as one block
  ahead of the game code. Units below `GAME_TEXT_START` use the SDK compiler,
  units from there on use the game compiler
  ([configure.py](../../games/rac1/ntsc/configure.py),
  [decompilation-tips.md](../../games/rac1/ntsc/docs/decompilation-tips.md)).
  `configure.py` states: "SN is not a compiler of the retail build".
- **The game compiler is a reconstruction.** Selected patches from the
  production stack ([patches/sce-991111b/README.md](../../games/rac1/ntsc/patches/sce-991111b/README.md)):
  `0001` R5900 quadword saves; `0015`/`0022` sibling calls off by default,
  with `-mastra-sibcall` per unit; `0020`/`0053` the gas absolute-or-`$gp`
  choice by whether the symbol's size is known at the use; `0027` inline `li.s`
  instead of `.lit4`; `0029` GCC 2.95.2's call-clobber analysis; `0037` no
  strict aliasing for game code; `0047`/`0048` gcc-2.95.2's whole `reload1.c`
  and `cse.c` on the 2.9 tree; `0050`/`0052` the original assembler's `dli`
  algorithm; `0054` loop and `div` padding left to Ps2EeAs; `0055` no
  `mult1`/`div1` (retail game code has 436 multiplies and none on pipeline 1);
  `0056` `.extern` placed before the first use of `sda` variables. Building
  the compiler on a modern host needs `-fno-strict-aliasing`. Without it,
  `real.c` sends negative float constants to `.sdata`.
- **Ps2EeAs.** It must be the 1.9.25.758 build from ProDG 3.01. The older one
  in the ProDG 2.0 archive "pads loops differently and fails the gate". Its
  divbug workaround is patched out because none of RAC1's 346 executable
  `div` words and 26,539 overlay `div` words is preceded by `nop; nop`
  ([building.md](../../games/rac1/ntsc/docs/building.md),
  [patch-ps2eeas.py](../../games/rac1/ntsc/scripts/patch-ps2eeas.py)).
- **Remaining routes.** `ROUTE_EXCEPTIONS` holds 4 units on `cc_sn` (SN cc1
  2.95.2), 9 on `cc_sn_padless` (SN cc1 plus Ps2EeAs) and 5 on the patched
  991111-01 profile ([configure.py](../../games/rac1/ntsc/configure.py)). That
  profile adds opt-in `-mastra-*` flags, `sq`/`lq` saves, classic
  `mult`/`mflo` and in-place `cvt.w.s` to the public `ps2-ee-toolchain`
  snapshot at `b595ded` ([patched-toolchain.md](../../games/rac1/ntsc/docs/patched-toolchain.md),
  [its README](../../games/rac1/ntsc/patches/ee-gcc-2.9-991111-01/README.md)).
  Without it the build rebuilds those units from the retail oracle and does
  not score them.
- **The NTSC decomp.** The NTSC decomp, a third-party decompilation of the
  same executable, used EE-GCC 2.95.2 with `-G8 -O2 -ffast-math -fno-exceptions`
  ("SN's assembler optional"), plus per-object `-fno-schedule-insns`,
  `-fno-schedule-insns2`, `-G0` and `-mno-split-addresses`. The PAL project
  measured that those object flags do not carry over to its build
  ([SIBLING_DECOMPS.md](../../games/rac1/pal/docs/SIBLING_DECOMPS.md)).

### RAC2 (rac2-decomp)

- **The first profile was SN 2.95.3 from ProDG 3.01**, with
  `-O2 -G0 -ffunction-sections` ([README.md](../../games/rac2/ntsc/README.md)).
  It matched 96 leaf bodies. Leaves have no saves, no calls and no `lq`/`sq`,
  and on them "the 2.9 and 2.95 code generators emit the same instructions".
  It failed three families: `sd` saves in 8-byte slots, `jal` plus a full
  epilogue at the end of a function, and an `mtc1` nop in some cases only
  ([COMPILER-NOTES.md](../../games/rac2/ntsc/docs/COMPILER-NOTES.md)).
- **The current profile** is 2.9-ee-991111b with these changes to the stack:
  `sd` saves in 8-byte slots; GPR saves ascending, with the FPR block first;
  sibling calls inert; the `mtc1` hazard nop whenever the next instruction
  reads the FPR (587 of 598 transfers in the first survey, 897 of 915 in the
  second), with a restricted exemption; the post-DBR short-loop padding
  turned back on, because RAC2's C goes through GNU `as`, not Ps2EeAs, plus a
  fix that counts a `TRAP_IF` at its full length; the zero TImode store folded by a back-end pass;
  and the generic frame scheduler as the default. The transformers are in
  [tools/compiler/transformers/](../../games/rac2/ntsc/tools/compiler/transformers/). The 96 bodies
  re-verified 96/96 under the new chain.
- **Scope.** Bodies with VU/MMI instructions or `$gp` are outside the
  qualification. "a byte match does not establish the original compiler
  identity" ([CONTRIBUTING.md](../../games/rac2/ntsc/CONTRIBUTING.md)). The
  README declares RAC1's `-O2 -G2` "deliberately **not** valid for RAC2".
- **Assembly rebuild.** Splat output is adapted to Ps2EeAs by
  [expand_asm.py](../../games/rac2/ntsc/tools/expand_asm.py), which keeps
  RAC1's `vadda` operand swap and writes `.float -0` as a word.
- **A complete rebuild recipe** (2026-10-07): the source archive's SHA-256,
  Lombyte's stack minus the saves widening, the five measured source
  adjustments shipped as scripts that carry their exact replacement text, and
  the configure and build steps. A partial recipe yields a different
  `mips.c`, and the built hashes depend on the host distribution
  ([COMPILER-NOTES.md](../../games/rac2/ntsc/docs/COMPILER-NOTES.md), "Complete rebuild recipe").
- **SDK and small data.** Exact SDK boot objects (CPR8, sysbit flush, IPU DMA
  restart) under a separately qualified SDK profile
  ([SDK-PROFILE-QUALIFICATION.md](../../games/rac2/ntsc/docs/SDK-PROFILE-QUALIFICATION.md)); a
  separate small-data unit per overlay, and `$gp` facts proved per function
  ([SMALL-DATA-UNIT-EVIDENCE.md](../../games/rac2/ntsc/docs/SMALL-DATA-UNIT-EVIDENCE.md),
  [LOCAL-GP-PROOF.md](../../games/rac2/ntsc/docs/LOCAL-GP-PROOF.md)). It also drew on rac1-decomp's
  findings ([RAC1-DECOMP-FINDINGS.md](../../games/rac2/ntsc/docs/RAC1-DECOMP-FINDINGS.md),
  [RAC1-FP-BIT-EVIDENCE.md](../../games/rac2/ntsc/docs/RAC1-FP-BIT-EVIDENCE.md)).

### RAC3 (rac3-uya-decomp)

- **The matrix.** 15 compiler builds times 8 flag sets, over 271 C functions
  and 47 problem cases. SN v1.36 reproduces 270/271, Sony 2.95.3-136 gives
  identical output, and no other build tops 189 (Sony 2.9-990721). "the 2.9
  builds are not better than SN for any tested group"
  ([compiler_matrix_findings.md](../../games/rac3/ntsc/docs/compiler_matrix_findings.md)).
- **Three former ceilings turned out to be settings.** `-fopt-stack`, an
  SN-only `cc1` option, gives `$s` saves as `sd` in 8-byte slots.
  `-mno-check-zero-division` drops the `break 7` trap. The `mtc1` nop
  comes from assembling with `bin/ee-as.exe` instead of `ee/bin/as.exe`.
- **Ps2EeAs** (ps2eeas 1.9.25) is single-pass. It uses `$gp` for a global
  only when it already knows the global is small at that point, and it forces
  a macro access in a delay slot to `$gp`. Its `mtc1` nops depend on the next
  instruction, it builds `li.s` inline, it pads short loops, and its strings
  include "DIV related opcode too near branch instruction". It cannot read
  GNU `macro.inc`, so it cannot assemble `INCLUDE_ASM` stubs.
- **`-mno-split-addresses` is per source file**: 18 clean address runs. No
  declaration reproduces the no-split code in split mode.
- **Handled outside the compiler.** 13 functions save `$ra` with `sq` in a
  16-byte slot. `asm_filter.py` rewrites them from
  [sq_ra_funcs.txt](../../games/rac3/ntsc/tools/sq_ra_funcs.txt). SN ProDG 2.0
  v2.73a emits that frame natively, but saves the registers in the opposite
  order. Retail pads most `div.s`/`sqrt.s` with 0 to 3 nops, and "no rule
  predicts the count", so [divs_nops.txt](../../games/rac3/ntsc/tools/divs_nops.txt)
  puts them back ([Matching-Patterns](../../games/rac3/ntsc/docs/wiki/Matching-Patterns.md)).
  `bin/ee-as.exe` lacks `lq`/`sq`/`lqc2`/`sqc2`, so stubs carry them as
  `.word` ([fix_quadword_ops.py](../../games/rac3/ntsc/tools/fix_quadword_ops.py)).
- **Never produced from C:** `sq $zero` (C gives `por` then `sq`) and `lq $at`.
  `long` is 64-bit and `long long` is 128-bit; `ULL` is rejected.
- **`-fno-force-mem` for a whole file** (`3958F0.c`), the same option rac1/pal
  gives two level files.
- **Three programs** (2026-10-07). `boot_elf.elf`, the main executable, and
  `i5bootn.elf`, the launcher, build byte for byte as well
  ([targets.md](../../games/rac3/ntsc/docs/targets.md)). The launcher was linked from three
  compilers' objects: Sony 2.9-ee-991111(-01) for libgcc and small libc
  helpers, 2.96-ee-001003-1 for newlib's `exit()`, and SN 2.95.3 for `main`
  (at `-O0`) and one dispatcher; 2.9-ee emits sibling calls, SN 2.95.3 never
  does ([compiler_matrix_i5bootn.md](../../games/rac3/ntsc/docs/compiler_matrix_i5bootn.md)).

### RAC4 (rac-deadlocked-decomp)

- **Compiler.** SN 2.95.3 v1.36 with RAC3's flags, picked by compiling five
  small functions with every compiler in the two mirrors; the UYA project's
  matrix supplied the flags ([RESEARCH.md](../../games/rac4/ntsc/docs/RESEARCH.md#the-compiler)).
- **Why each flag.** `-fopt-stack`: retail saves registers with `sd`/`ld` in
  8-byte slots. `-G8`: retail's `$gp` accesses include 18 eight-byte `ld`/`sd`.
  `-mno-split-addresses`: per file, as in RAC3.
  `-mno-check-zero-division`: not needed yet.
- **Assembler.** Retail was assembled by Ps2EeAs; the project imitates it with
  rac1/pal's two passes. Its retail histogram of backward branches starts at
  5 instructions, so the short-loop minimum is 5 here, where rac1/pal uses 6.
  Level code has 722 `mtc1; nop; cvt.s.w` sequences and 93 without.
- **Libraries.** libgcc and libm (newlib's fdlibm, snapshot 2000-02-17) are
  rebuilt from their sources with Sony's 2.9-ee driver at `-O2 -G2`: 24 of 36
  and 27 of 40 functions match. Sony's prebuilt `libgcc.a` matches the three
  libgcc functions the sources do not, so retail linked that archive.
- **Open there.** Two nops before most `div.s` (281 of 435 in level code) that
  no compiler in the mirrors emits; a link-time comparison.

## 3. Cross-game findings

| Finding | RAC1 PAL | RAC1 NTSC-U | RAC2 | RAC3 | Status |
|---|---|---|---|---|---|
| SDK libraries built by a 2.9-ee-991111 variant, separately from game code | yes; SDK `.a` members match retail | yes; SDK compiler is 991111-01 | exact SDK boot objects under a separately qualified SDK profile ([SDK-PROFILE-QUALIFICATION.md](../../games/rac2/ntsc/docs/SDK-PROFILE-QUALIFICATION.md)) | yes: `i5bootn`'s libgcc and libc helpers are 2.9-ee-991111(-01) ([compiler_matrix_i5bootn.md](../../games/rac3/ntsc/docs/compiler_matrix_i5bootn.md)) | Every game measured |
| The compiler switch sits at `boot.cpp` | `boot.cpp` at 0x12DA38, NTSC-to-PAL shift +0x140 there ([DECOMP_PROGRESS.md](../../games/rac1/pal/docs/DECOMP_PROGRESS.md)) | `GAME_TEXT_START` 0x12D8F8 (+0x140 = 0x12DA38) | | | Agreed for RAC1 |
| Callee-saved saves | `sq`, 16-byte slots, `$ra` too, in all game code; `sd` in the SDK objects of `core_text`, which 2.9-ee builds | `sq` saves (patch `0001`) | `sd`, 8-byte slots | `$s`: `sd`, 8-byte slots (`-fopt-stack`); 13 functions `sq $ra` | Differs per game. A signature, not a constant |
| Bare tail jumps | only in 2.9-ee objects, where the compiler emits them; the rewrite for SN code is gone ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)) | sibling calls off, on per unit | none in the measured family | SN 2.95.3 never emits them; 2.9-ee-991111 does ([compiler_matrix_i5bootn.md](../../games/rac3/ntsc/docs/compiler_matrix_i5bootn.md)) | A tail jump means "not SN 2.95.3" (RAC1 PAL, RAC3) |
| Short loops padded by the assembler | Ps2EeAs for `text`: six instructions, target through branch | Ps2EeAs; earlier `cc1` patches padded to "shorter than 7" | `cc1` hook, "measured minimum of seven" | Ps2EeAs pads; ee-as stubs need raw branches | Same rule in three games. Patch `0046`'s example (`jal; nop x4; bnez; nop`) is seven words counting the delay slot |
| `nop`s before `div`/`div.s` | none among 1,671 divides | none among 346 + 26,539 | not documented | 0 to 3 before most `div.s`/`sqrt.s` | Differs between RAC1 and RAC3 |
| `mtc1` hazard nop | handled in the build ([LEVERS.md](../../games/rac1/pal/docs/LEVERS.md)) | patch `0051` | rule plus a narrow exemption | ee-as or Ps2EeAs per range | Present in three games; details differ |
| 64-bit `dli` built top-down, Ps2EeAs style | `ps2eeas_dli.py` | gas patches `0050`/`0052` | | `@ps2as` with an `unsigned long` literal | Same finding in RAC1 (both projects) and RAC3 |
| Inline `li.s` rather than `.lit4` | kept inline by `-G2` with GNU as | gas patch `0027` | | Ps2EeAs only | Same retail form; three mechanisms |
| Delay-slot global access becomes `$gp`-relative | `MACRO_ADDR` + `check_macro_slots.py` | Ps2EeAs rule in delay slots | outside scope | Ps2EeAs rule | Independent findings in RAC1 (both) and RAC3 |
| `-G` | `-G2` | none passed | `-G0` | `-G8`; "-G2 fails every 4-byte `$gp` variable" | Not settled; see section 4 |
| `-mno-split-addresses` | per file ([file_cflags.txt](../../games/rac1/pal/config/file_cflags.txt)); `pause.c` whole breaks 81 of 124, so its function is assembly again | per unit | not documented | per file, 18 runs | Retail used it in RAC1 and RAC3 |
| `-fno-force-mem` | per file: two level functions in files of their own ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)) | | | per file (`3958F0.c`) | Same option, per file, in RAC1 and RAC3 |
| Steps that edit compiler output | none since 2026-10-07; `check_build_fidelity.py` enforces it ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)) | none: behaviours are compiler and assembler patches | none: behaviours are compiler source adjustments ([COMPILER-NOTES.md](../../games/rac2/ntsc/docs/COMPILER-NOTES.md)) | `asm_filter.py`: `sq $ra` for 13 functions, `div.s` nops from a table | Differs; PAL models only the assembler and linker |
| libgcc rebuilt from GCC's source | every module, the last three with a Linux 2.9-ee-991111-01 built from source | GPL-2.0 reconstruction | `fp-bit` unpacker from vendored source | 26 of `i5bootn`'s, from GCC 2.95.x `fp-bit.c` with Sony's no-denormals change | Sony's libgcc is reproducible from source in three games |
| Zero-divide trap | present with default flags ([SIBLING_DECOMPS.md](../../games/rac1/pal/docs/SIBLING_DECOMPS.md)); `-mno-check-zero-division` for a `div` without it ([LEVERS.md](../../games/rac1/pal/docs/LEVERS.md)) | | present in some bodies (`TRAP_IF`) | absent: `-mno-check-zero-division` | Differs |
| `sq $zero` | wall: C adds `por` | | compiler patch | wall under SN 3.01 and 2.0 | No stock compiler emits it from C |
| Linker dead-strip remnants and `0xCDCDCDCD` fill | both; rule `floor(size/8)*8` | | | 203 remnants; fill around jump tables | RAC1 and RAC3 |
| `long` 64-bit, `long long` 128-bit | yes | | | yes | SN 2.95.3 in RAC1 and RAC3 |

Findings that transfer: the SDK/game split and the SDK archives as references
(RAC1, two projects; RAC3's launcher). The assembler behaviours of Ps2EeAs,
found independently in RAC1 and RAC3: short-loop padding, the `dli` form,
`$gp` in delay slots, size-at-use, inline `li.s`. Per-file
`-mno-split-addresses` and `-fno-force-mem` (RAC1, RAC3). Dead-strip
remnants (RAC1, RAC3). Sibling calls as a compiler signature: SN 2.95.3 emits
none, 2.9-ee does (RAC1 PAL, RAC3). A lead for RAC1 PAL: its `_exit`
(`func_0012DA28`, a bare `j` with `$a0` cleared in the delay slot) went back
to assembly on 2026-10-07 because v1.14 cannot emit it, while RAC3's launcher
has an `_exit` and its jump stubs that match from C under 2.9-ee-991111. If
rac1/pal's `crt0` object turns out to be 2.9-ee library code, its `_exit` is
plain C again.

Measured on one game only: `-fopt-stack` and `@ps2as` per range (RAC3); the
`mtc1` statistics and the `sq $zero` patch (RAC2); `-G2` and the spill
rewriting (RAC1 PAL); the patched game compiler (RAC1 NTSC-U, reused for
RAC2).

## 4. Open questions and contradictions

1. **Which compiler built RAC1's game code.** The two regions' projects give
   different answers:
   - PAL: two SN 2.95.3 sub-builds. `text` matches SN BUILD v1.14. The
     `core_text` objects that save with `sd` are SDK code built with
     2.9-ee; the game code there is v1.14 as well, and nothing rewrites its
     output ([TOOLCHAIN.md](../../games/rac1/pal/docs/TOOLCHAIN.md), [BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)).
   - NTSC-U (Lombyte): a patched 2.9-ee-991111b. Its `configure.py` says
     "SN is not a compiler of the retail build", yet the
     [sce-991111b README](../../games/rac1/ntsc/patches/sce-991111b/README.md)
     speaks of "the retail SN cc1".
   - A third reading, the NTSC decomp's EE-GCC 2.95.2 `-G8 -ffast-math`, is
     recorded in [SIBLING_DECOMPS.md](../../games/rac1/pal/docs/SIBLING_DECOMPS.md).

   They agree on the SDK compiler family, the switch point at `boot.cpp`,
   Ps2EeAs assembling game code, no divide padding and the `dli` form. They
   disagree on 989snd. PAL measured it at 25/25 under 2.95.3 against 3/25
   under stock 2.9-ee. Lombyte builds the same stretch with its game compiler:
   `snd_init_vag_streaming_ex` at 0x12EB20 needs patch `0046`, and lies
   inside PAL's 989snd object after the +0x140 shift. Two `audio/rpc` units
   are still built on SN there. Lombyte's stack now carries gcc-2.95.2's
   `cse.c`, `reload1.c` and call-clobber analysis (`0029`, `0047`, `0048`).

   Both projects port matched C to each other (PAL's `tools/lombyte.py`;
   "PAL import" units in [configure.py](../../games/rac1/ntsc/configure.py)).
   A function whose C matches under both toolchains does not tell the
   readings apart; RAC2 makes the same point about leaf functions. Merging the
   regions waits on this question ([OPEN_QUESTIONS.md](../policy/OPEN_QUESTIONS.md),
   item 4).
2. **RAC2's C compiler.** Settled (2026-10-11): one recipe builds the game compiler
   (2.9-ee-991111b, [tools/compiler/build.sh](../../games/rac2/ntsc/tools/compiler/build.sh), which stops
   unless the five patched sources hash as recorded), and the boot's SDK units use Sony's
   unmodified 2.9-ee-991111-01 ([tools/compiler/sdk/](../../games/rac2/ntsc/tools/compiler/sdk/build.py)).
   SN ProDG's 2.0 assembler and linker stay for the reconstructed assembly and the link.
3. **Does SN 2.95.3 emit sibling calls?** Settled (2026-10-07): it does not.
   RAC3's launcher matrix found that no SN 2.95.3 build emits one, while
   Sony's 2.9-ee-991111 does ([compiler_matrix_i5bootn.md](../../games/rac3/ntsc/docs/compiler_matrix_i5bootn.md), "Correction"). PAL had measured that
   `-foptimize-sibling-calls` is rejected by both SN sub-builds and that
   `return f(x);` compiles to a call and a return
   ([DECOMP_PROGRESS.md](../../games/rac1/pal/docs/DECOMP_PROGRESS.md), "SOLVED:
   tail calls"); Lombyte's patch `0049` and RAC2's item 3 agree. RAC2's table
   entry giving SN 2.95.3 a sibling call is the outlier.
4. **Would `-fopt-stack` fit RAC2?** RAC2 left SN partly because of `sd`
   saves in 8-byte slots. RAC3 gets exactly that layout from SN 3.01 with
   `-fopt-stack`. RAC2's documents do not mention the option.
5. **Ps2EeAs and `$gp`.** PAL first reported that ps2eeas "has no `-G`
   expansion": every `$gp` access came out as `lui`/`lw`. RAC3 and Lombyte
   describe a single-pass rule: `$gp` only once the size is known, and gcc
   writes its `.extern` sizes at the end of the file. PAL now reads its own
   result the same way ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md), "Why not the real ps2eeas"); its retry on
   2026-10-07 stopped earlier, with both versions overflowing their stack on
   v1.14's output, and v1.14 ignores `-mgpopt`. Lombyte gets `$gp` by having
   its compiler declare small-data symbols before use (patch `0056`).
6. **What `-G` did retail use?** PAL's case for 1 to 3 rests on inline float
   constants, measured with GNU as. RAC3 shows that Ps2EeAs builds `li.s`
   inline at `-G8`, and Lombyte patched its gas to do the same. If retail was
   assembled by Ps2EeAs, the float argument may not bound `-G`. In both SN
   projects the threshold also interacts with declared sizes.
7. **GNU assembler behaviour.** PAL found the standalone `bin/ee-as.exe` pads
   no short loop, and that it adds hazard nops after every `mtc1` and FP
   compare; a text build with it drifted. RAC3 uses that assembler by default
   for its `mtc1` nops, and found it pads short loops in reorder-mode stubs
   with `-mcpu=5900` ([fix_short_loops.py](../../games/rac3/ntsc/tools/fix_short_loops.py)).
   The two probes may have used different copies of the assembler or
   different options; the documents do not say.
8. **Where the divide padding comes from.** RAC1 has none and RAC3 has it,
   with a count that no rule predicts. Two candidate sources are on record:
   Ps2EeAs's divbug workaround (up to two nops after a branch or a label,
   [patch-ps2eeas.py](../../games/rac1/ntsc/scripts/patch-ps2eeas.py)), and Sony's
   2.96-ee-001003-1 compiler, whose default template emits `nop; nop; div.s`
   ([SIBLING_DECOMPS.md](../../games/rac1/pal/docs/SIBLING_DECOMPS.md)).
9. **Build labels.** RAC3's matrix lists "Sony 2.95.3-114" (183/271), while PAL
   compiles game text with "SN BUILD v1.14". Neither document says whether
   these are the same binary. PAL reports that v1.14 saves `$ra` with `sq`.
   RAC3's notes on its 13 `sq $ra` functions describe tests with SN 3.01 and
   ProDG 2.0, and do not say whether the "-114" build was run on them.
10. **Two SDK compilers.** PAL uses `ee-gcc.exe` from `sce_ps2_sdk_24` (Windows
    `2.9-ee-991111b/r4`); Lombyte uses EE-GCC 2.9-ee-991111-01. Both match SDK
    code. PAL's three libgcc2 modules that the Windows `cc1` could not match
    now match with a Linux 2.9-ee-991111-01 built from source (2026-10-07,
    [src/libgcc/README.md](../../games/rac1/pal/src/libgcc/README.md)), as
    their debug info suggested; RAC3's launcher libgcc pins 991111-01 too.
    Lombyte runs both of its compilers natively on Linux
    ([configure.py](../../games/rac1/ntsc/configure.py)).
11. **macOS hosts.** PAL measured that Wine's 32-bit code fails under Rosetta
    in an amd64 Linux machine. Lombyte's `--docker` image is `linux/amd64`
    with `wine32:i386` ([setup.sh](../../games/rac1/ntsc/setup.sh)), and its
    Wine 9 fails on Apple Silicon under Rosetta and under QEMU. Measured in
    OpenRAC on 2026-10-04: with the Wine 8 of PAL's `linux/386` image run in
    its place, Lombyte's boot ELF and all 1,540 of its overlay functions match
    ([games/rac1/ntsc/host](../../games/rac1/ntsc/host/README.md)).

## 5. Choosing a toolchain for Deadlocked

Deadlocked's project began on 2026-10-04 and chose RAC3's profile, SN 2.95.3
v1.36, from a five-function comparison ([RESEARCH.md](../../games/rac4/ntsc/docs/RESEARCH.md#the-compiler),
section 2 above). That is step 2 of the list below, which was written before
the project existed. The rest still applies to what it leaves open: the
`div.s` nops, and a comparison at link addresses. Based on the other three
games:

1. **Measure the retail signatures first.** Before choosing a compiler, look
   for each of the forms that separated the compilers in the earlier games:
   - spill width, slot size and save order, for `$ra` and the `$s` registers;
   - bare tail jumps;
   - `nop`s before `div`/`div.s`/`sqrt.s`;
   - short loops padded to six instructions;
   - `mtc1` hazard nops;
   - the `dli` form of 64-bit constants;
   - inline `li.s`;
   - `$gp` use in delay slots;
   - `break 7` after `div`;
   - `mult1` in game code;
   - `0xCDCDCDCD` fill and dead-strip remnants between objects.

   Read the gp value from `.reginfo`, as PAL and RAC2 do.
2. **Start with RAC3's profile.** It is the nearest game in time, and the
   profile reproduces all of `frontbin.elf`: SN ee-gcc 2.95.3 v1.36 with
   `-O2 -G8 -fopt-stack -mno-check-zero-division`, `-mno-split-addresses`
   per file, and ee-as or Ps2EeAs chosen per range
   ([Toolchain-and-Build](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md)).
3. **If the saves are `sq` with `$ra` in a 16-byte slot,** add SN v1.14 (RAC1
   PAL text) and SN ProDG 2.0 v2.73a (RAC3's 13 functions) to the candidates.
4. **If SN fails on a family,** the patched 2.9-ee-991111b stacks of
   [Lombyte](../../games/rac1/ntsc/patches/sce-991111b/README.md) and
   [RAC2](../../games/rac2/ntsc/docs/COMPILER-NOTES.md) can be rebuilt from a
   pinned public source archive and are the next thing to try.
5. **Expect SDK code to need its own compiler,** as in RAC1, and check it
   against the SDK library archives.
6. **Test candidates on functions with saves and calls.** Leaf functions do not
   discriminate (RAC2). Run a matrix like RAC3's, and include what RAC3's
   matrix left out: the ProDG 2.0 package and the patched 2.9-ee builds.
7. **Pick a host that is known to work:** Windows, wibo (RAC3), or a
   `linux/386` Wine container (PAL).
