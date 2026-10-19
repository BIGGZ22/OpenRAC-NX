# Measured starting point and order of work

1. **Qualify the input (done for the supplied USA ZIP).** The complete ISO has
   the declared ISO9660 length and matches a public catalogue SHA-1. Its boot
   ELF is pinned in `config/target.json`. Keep that image as the reference;
   the earlier 150-sector-short copy is excluded.
2. **Map the programs (in progress).** The boot is an ELF32 MIPS R5900 executable with one
   loadable segment, entry `0x01E62508`, and a `.text` section at `0x01E62500`
   (size `0x79CD4`). The disc also has `LVL/FRONTEND.REL` and numbered
   `LVL/LEVEL_*.REL` files. Their SNR2 headers, relocation tables and exported
   symbols have been checked across all 21 modules (`docs/REL-FORMAT.md`).
   The boot's embedded SNR2 table provides 1,803 named symbols
   (`docs/BOOT-SYMBOLS.md`).
   A metadata-only comparison with MultiworldGG's USA PS2 runtime map confirms
   several boot globals and the 12-word loader-site signature
   (`docs/MULTIWORLD-CROSSCHECK.md`).
   The boot loader's two relocation sources and all 21 packed local-fixup
   streams are now mapped (`docs/REL-FORMAT.md`). The complete loaded file
   bytes for the frontend and level 01 have been reproduced and compared
   directly to EE memory before constructors (`docs/RELOCATED-IMAGE.md`).
   Candidate code intervals for all 21 placements are backed by exact
   jump-relocation site sets and private assembly comparisons
   (`docs/SNR2-ASSEMBLY-CAMPAIGN.md`). The
   [frontend and level 01 program gate](LEVEL-PROGRAM-GATE.md) now combines
   exact assembly with the full original module and two measured relocated
   images. Further review of other boundaries, shared code, data layout and
   runtime behavior remains necessary; do not reuse RAC2's
   WAD layout.
3. **Establish an assembly baseline (in progress).** The entire 498,900-byte
   boot `.text` section now reassembles exactly with a measured local SN
   assembler/linker combination (`docs/ASSEMBLY-STATUS.md`). Copying the
   pinned non-code sections into ignored local assembly also reproduces all
   809,872 file-backed boot load bytes. Candidate code intervals in all 21
   SNR2 placements also reassemble exactly (36,341,888 cumulative bytes by
   placement), but their remaining bytes are not independent assembly
   baselines. The first two complete SNR2 module files now have private
   integration gates, with non-code bytes supplied by the pinned disc. Verify
   BSS/runtime behavior
   before calling this a full-program baseline. Non-code copying is not
   decompilation.
4. **Identify compiler profiles.** Mangled C++ exports and ProDG SNR2 modules
   are evidence, not a compiler version (`docs/COMPILER-STATUS.md`). Measure
   signatures on code and SDK-library ranges. Do not copy RAC2's `sdk` or game
   compiler routing without witness functions and baseline controls. One
   [boot C-object integration witness](BOOT-C-WITNESS.md) preserves all
   raw retail `PT_LOAD` bytes and header. A combined trial now preserves them
   initially with twenty C functions linked together (`docs/BOOT-C-WITNESS.md`). The
   current pure-C campaign recompiles 619 objects and catalogues 619 body extents;
   inline-assembly experiments are excluded. That
   evidence supports a reviewed boot placement set, not a general compiler or
   full runtime behavior. The later packaging gate rebuilds the entire ELF
   file exactly from the linked load image and numeric metadata.
   The [boot gate](BOOT-TRIAL-GATE.md) now rebuilds all C objects from
   SHA-pinned local tools and checks the complete load image reproducibly.
   561 reviewed boot placements are accepted under this limited profile;
   level programs and the original compiler identity remain open.
   The [VU overlay audit](DVP-OVERLAYS.md) privately reproduces the 31 DVP
   sections while preserving the exact load segment. An older direct SN-linker
   output differs in section layout and CRC; the metadata packaging gate now
   recreates the pinned complete file exactly.
5. **Catalogue functions.** The boot now has generated next-label bounds for
   1,315 distinct code addresses (`config/boot/code_intervals.tsv`). They are
   search limits, not validated function sizes. The linked C witnesses now
   also have verified candidate body/padding extents
   (`config/boot/witness_boundaries.tsv`). Review remaining boundaries,
   padding, aliases and placements before producing a retail function
   catalogue; commit metadata only.
   Across levels, the [candidate inventory](LEVEL-FUNCTION-INVENTORY.md)
   covers all 21 placements and separates bounded call targets from pointer
   entries and unresolved spans. Review those boundaries and masked duplicate
   leads before declaring a unique-code denominator.
   An independent Ghidra pass now corroborates 91,350 candidate boundaries;
   2,415 masked groups pass data-target and closed local-callgraph checks;
   1,209 internal-block leads also have a same-body direct edge. Review the
   remaining disagreements, indirect targets and data references
   before treating these leads as one shared source family each.
   A Ghidra body-range pass also flags 1,590 candidate starts inside another
   body. A full-module pass assigns them to flow-evidence review categories,
   including 21 newly called entries and 229 linear cases without recovered
   indirect references. Confirm real ownership and embedded data before
   accepting function boundaries.
   A stricter two-analysis pass selects 6,245 returning code segments within
   3,729 oversized candidate intervals. Internal gaps must be zero; 294,836
   surrounding bytes remain unique. These are shared-code review leads, not
   accepted function sizes or a game-wide denominator.
   A separate full-module audit now identifies 1,117 contiguous switch bodies
   split by pointer-only case labels; only 62 families survive later context,
   data-target, callgraph and collision checks.
   A broader site-paired role audit finds 141,319 conditional links beyond the
   strict context audit, for a 10,088,660-byte normalized-code scenario.
   Thirteen reference modules reproduce the same savings as all twenty
   retained modules under the current proof rules.
   Different callee implementations can fill the same call role; validate
   source identity and all remaining function boundaries before using that
   scenario as a game-wide denominator.
6. **Start scoped C/C++ matching.** `MEMCARD_Busy__Fv` is the first
   accepted 16-byte boot placement. The
   [level C gate](LEVEL-C-WITNESS.md) accepts two authored C functions,
   8 and 72 bytes, at twenty level placements, plus a 476-byte frontend
   initializer. Other functions still need reviewed
   boundaries, compiler profiles and placement
   checks. The [scoped progress snapshot](../progress/README.md) records
   measured status without publishing a premature game-wide percentage.

The tools complete step 1, the SNR2 metadata part of step 2, and the boot's
file-backed assembly/load-image part of step 3. Runtime and level-program
baselines and game-wide matching C/C++ remain open. See the
[boot-only acceptance scope](BOOT-MATCH-SCOPE.md) before counting more.
