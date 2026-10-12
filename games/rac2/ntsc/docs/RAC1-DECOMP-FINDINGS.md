# Findings from rac1-decomp

[rac1-decomp](https://github.com/OpenRAC/rac1-decomp) is a matching decompilation
of the first game, PAL v2.00 `SCES_509.16`, at 22.6% of its code (2,974 of 5,109
functions) on 5 October 2026. Several of its measured results concern compiler,
library and SDK code that the engine's later games share. This page summarises
them for RAC2 with links to the original records.

Everything below was measured on **RAC1's** executable. None of it is a RAC2
match, adds RAC2 credit or relaxes this repository's acceptance rules. A RAC2
use still needs its own catalogue, fresh objects and complete loaded-byte gates.

## 1. Library code: compile the real source first

RAC1's retail `core_text` links GCC's runtime library. rac1-decomp rebuilds it
from GCC's own sources rather than decompiling it
([`src/libgcc/README.md`](https://github.com/OpenRAC/rac1-decomp/blob/cb22f0b0d3a171d1fd4b6851b86fe214a22c9822/src/libgcc/README.md),
[`docs/DECOMP_PROGRESS.md`](https://github.com/OpenRAC/rac1-decomp/blob/cb22f0b0d3a171d1fd4b6851b86fe214a22c9822/docs/DECOMP_PROGRESS.md),
"libgcc is 2.9-ee").

- **Compiler.** Sony's **gcc 2.9-ee-991111**, the lineage of this repository's
  current GNU EE profile, not the SN 2.95.3 that built RAC1's game code.
- **Sources.** At commit `cb22f0b0d3a171d1fd4b6851b86fe214a22c9822`,
  `libgcc2.c` and `longlong.h` come from GCC trunk `31cf01446d`
  (1999-09-09). GCC 2.95.3's `fp-bit.c` has two marked changes: shared
  `__thenan_df` storage and the `NO_DENORMALS` backport from trunk
  `2672543458` (2000-03-16). The build also selects
  `FLOAT_BIT_ORDER_MISMATCH` and the GOFAST names.
- **Result.** fp-bit (`_fpadd_parts`, `__adddf3`, `__subdf3`, `__muldf3`,
  `__divdf3`, `__fpcmp_parts_d`, `__cmpdf2`, `__floatsidf`, `__fixdfsi`, ...)
  and libgcc2's `__divdi3`, `__muldi3`, `__floatdidf` and `__fixunsdfdi` match
  RAC1 byte for byte, relocations included, with no post-processing. Across
  0x11DFE8-0x1206A0, 2,478 words compare equal.
  That whole-image total includes the remaining assembly stubs and linker
  fill; it is not a claim that every byte in the region comes from C.
- **Build through the driver, not `cc1`.** The driver passes the target
  predefines (`__mips__`, `__R5900__`, ...) from which `longlong.h` selects its
  MIPS multiply and divide primitives. Called through `cc1` directly,
  `__divdi3` came out 0x50 bytes too long.
- **One object per `L_*` module**, as in `libgcc.a`. Each object's 8-byte
  `.text` alignment reproduces retail's gaps between modules. Linker fill
  between some modules is `0xCDCDCDCD`, which splat had split out as tiny
  "functions". Each division module owns a static `__clz_tab`.
- **Still open in RAC1.** `__moddi3`, `__udivdi3` and `__umoddi3` remain
  assembly stubs. Retail reserves additional unused stack space: 0x20,
  0x10 and 0x20 bytes respectively. The source and flag trials recorded
  upstream did not resolve those frame differences.
- **How it was found.** A family of near-misses shared one "known residual".
  Compiling one member with every available EE `cc1` showed that only 2.9-ee
  produced retail's prologue. A family-wide residual can point to a different
  compiler or to library code rather than a missing source lever.

In RAC2, the stopped boot helpers `FUN_00123400` (144 bytes) and
`FUN_001234F0` (64 bytes) follow the integrated `FUN_00123268` (44 bytes) with
the sizes and order of RAC1's `__unpack_f`, `fptodp` and `__make_dp`. That is a
correspondence hypothesis, proposed separately as campaign evidence. RAC2 has
not established whether its boot links the same modules, revision or flags.

## 2. SDK archives as a source of names

The EE SDK archives (`libmc.a`, `libdbc.a`, `libpad2.a`, `libmpeg.a`, ...)
matched RAC1 retail members byte for byte with relocations masked. Their
`.symtab`/`.mdebug` give real function and static-data names, for example
`sceMcInit`, `sceMcOpen`, `sceMcRead` and `sceMcSync` in `libmc.o`. In this
repository such names are naming research only and add no credit.
rac1-decomp took the archives from a community mirror of the SDK; RAC2's
toolchain policy does not accept unverified SDK sources, so any use needs an
archive whose provenance meets that policy.

## 3. Compiler behaviour measured on RAC1

These explain differences RAC2 has also recorded. The workarounds rac1-decomp
uses for its game code rewrite compiler-generated assembly, which this
repository's acceptance rules do not allow; they are listed for understanding,
not adoption.

- **Two SN GCC 2.95.3 sub-builds behave differently.** v1.14 (in the
  `sce_ps2_sdk_24` kit) lays out callee-saved slots like RAC1 retail; v1.36
  (`ee-gcc2953` in ProDG 3.01) uses a mirrored layout. No command-line flag
  changes either. This matches the `sd`/`sq` difference in
  [COMPILER-NOTES.md](COMPILER-NOTES.md), which compared the ProDG 3.01 kit.
- **GCC 2.95.3 has no sibling-call optimisation**: `-foptimize-sibling-calls` is
  rejected and `-O3` does not help. RAC1's bare `j target` tail calls came from
  post-processing. Sony's 2.9-ee performs tail calls natively, also in eight
  RAC1 functions where retail did not.
- **Small data.** RAC1's game code uses `-G2`: float constants stay inline and
  small globals are reached through `$gp`. `-G0` never uses `$gp`; `-G4` and
  above pool floats into `.lit4`. Placement follows an extern's declared size.
- **Short-loop erratum.** SN's `ps2eeas` pads every backward branch whose loop
  is shorter than six instructions with nops. The 2.9-ee driver's own `as.exe`
  pads call-free short loops; the standalone GNU `ee-as` of the same version
  pads none. RAC2's [RAC1-TO-RAC2.md](RAC1-TO-RAC2.md) records the same
  hazard-nop behaviour for the ProDG 3.01 assembler.

## 4. Candidate-generation aids

rac1-decomp uses these to propose candidates. These observations do not
permit source permutations on parked RAC2 targets: the concrete reopening
evidence required by CAMPAIGN-WORKFLOW.md still applies.

- **m2c with generated context** for a first C sketch. Its output is never
  matching as emitted and mis-decodes branch-likely conditions.
- **[decomp-permuter](https://github.com/simonlindholm/decomp-permuter)**
  ([`docs/PERMUTER.md`](https://github.com/OpenRAC/rac1-decomp/blob/cb22f0b0d3a171d1fd4b6851b86fe214a22c9822/docs/PERMUTER.md)).
  Effective on store-order and small register-allocation near-misses: one RAC1
  function reached an exact candidate at iteration 105, under two minutes, by
  reordering two stores. Less effective on control-flow shapes such as loop
  reversal or branch inversion. A hit is a lead, not a proof.
- **Exhaustive reordering** of a few marked statements (rac1's
  `tools/permute.py`) covers every ordering of up to about eight lines.
