# Reconstructed GNU EE compiler: provenance and measured compatibility

**Measured 2026-10-03 against the pinned retail boot** (`SCUS_972.68`, USA v1.01,
sha256 `36d5814d…`, `config/target.json`).

The integrated C bodies are reproduced byte-for-byte by **GNU-EE 2.9-ee-991111b**
— the same compiler lineage as the first Ratchet game — plus a cumulative
retail-behaviour patch stack. The previous profile (the SN ProDG 3.01 kit,
`ee-gcc2953`/2.95.3) reproduced the simple leaf bodies but could not reproduce
three families, which is what drove the move:

| Family | Retail | SN 2.95.3 |
|---|---|---|
| callee-saved register saves | `sd` in 8-byte slots | `sq` in 16-byte slots |
| calls at the end of a function | plain `jal` + full epilogue | sibling call (`j target`) |
| GP→FP transfers (`mtc1`) | a `nop` in *some* cases only | n/a (different assembler) |

## Instrument identity and provenance

Native programs may use the default `-O2 -G0 -ffunction-sections` or the narrowly
supported `-O2 -G8 -ffunction-sections` profile with the same pinned instruments.
The boot retains its default flags. Every native review binds its own catalog
flags, source, checker and object, and still requires complete byte equality.
Other optimization or small-data flag variants are rejected by the native loader.

The Barlow queue launcher provides the measured small-data case. Its reference
loads a four-byte mode word at `0x001A8F00` relative to pinned GP `0x001AEFF0`.
The reconstructed backend disables GP optimization and early known-size SDA
extern emission at `-G0`; positive `-G8` with optimization enables that path.
The maintained assembler uses its eight-byte default threshold. This changes
address generation, not the compiler binary or the source algorithm.

A separate four-byte control object at `0x0018C0B4` lies outside that GP's signed
16-bit reach and has a pinned absolute getter. An explicit `nosda` declaration
preserves this measured binding instead of allowing the positive-G size heuristic
to select an impossible relocation. All fifteen existing Barlow controls remain
exact at `-G0` with that binding. The same source at `-G8` matches all sixteen
complete functions, including the 200-byte launcher. Failed unbound qualification
and the default-profile launcher refusal remain in the campaign register.

This per-program qualification does not establish original SDK flags or make a
near-match acceptable. Fresh reviews and complete loaded-byte/metadata gates are
required before integration; GP, declarations and flags remain explicit proof inputs.

`8bed6eae` is the SHA-256 prefix of the locally rebuilt `cc1` binary, not a compiler name or version. The source lineage is GNU EE 2.9-ee-991111b. The local build recipe starts from `gnu-ee-binutils-gcc-1.1.tar.gz`, applies the RAC1/Lombyte `sce-991111b` patch stack, and makes the measured RAC2 adjustments described below. The profile was introduced for this repository in commit `b2b9101` after comparing the earlier SN ProDG 3.01 GCC 2.95.3 profile against call-bearing retail bodies.

Matching the qualified bodies establishes compatibility with those bodies. It does not establish the exact compiler binary, patch set, flags or source directives used by Insomniac for the original game. Future bodies can falsify this compatibility profile.

Authored C uses the reconstructed GNU `cpp`/`cc1`/`as` through `scripts/wsl_chain.py`. Linking still uses the SN SDK `ld.exe`. The reconstructed assembly path uses `Ps2EeAs.exe` separately. The prior SN compiler remains available locally; the SN toolchain directory passed to the current C checker supplies its linker, and does not mean that the checker invokes `ee-gcc2953`.

## The evidence

Seven campaign bodies were compiled, assembled and linked with the reconstructed
chain, then compared to the retail bytes: `FUN_002889B8`, `FUN_002A77E0`,
`FUN_002A7820`, `FUN_002A7940`, `FUN_003512B8`, `FUN_00351268` (the six bodies of
the call-bearing family) and `FUN_00300540` (an aiguillage) — **all byte-exact**.

The stronger check: the **96 bodies already integrated under the previous
profile were re-verified under the new chain — 96/96 still byte-exact**. The
profile switch therefore costs nothing that was already proven, and the same
reconstruction reproduces the save-bearing family.

## Why the two profiles agree on the simple bodies

The 96 leaf bodies contain no saves, no calls and no `lq`/`sq`; on that subset the
2.9 and 2.95 code generators emit the same instructions. The divergence appears
precisely in the prologue/epilogue and in block moves — which is why the
campaign's first 96 matches never exercised it.

## The compiler-side rules that had to be measured

Five changes separate this reconstructed profile from the released 2.9 sources; each was
found from a witness pair in the retail image, then validated on the full corpus:

1. **Save width.** `prologue/epilogue` save GPRs with `sd` in 8-byte slots. The
   released source widens register 0 to `TImode`, which makes every slot 16
   bytes; the retail build does not (`FUN_002889B8`: `sd $s0,0($sp); sd $ra,8($sp)`,
   frame 16).
2. **Save order.** GPR saves and restores are emitted in ascending register order
   (`s0, s1, …, ra`); the released `save_restore_insns` loop descends from `$ra`.
   FPR saves retain their descending order and precede the GPR block. Restores
   retain GPR before FPR. The two emission blocks and all stack offsets are
   preserved, with their shared base initialized before the first block.
   The public [block-reordering patch](../tools/compiler/transformers/reorder_save_blocks.py)
   applies after the ascending-GPR change. This ordering reproduces
   `FUN_002A7878` (92 bytes) and `FUN_002A78D8` (104 bytes); all 103 previously
   integrated bodies still match under the new compiler.
3. **No sibling calls, by default.** The Cygnus 2.9 sibcall pass, absent from the
   SN compiler, must stay inert (`FUN_003512B8` ends in `jal 0x11ac40` plus a
   full restore, not in a tail jump).
4. **The `mtc1` hazard nop.** The initial boot survey (`veille/mesure-regle-mtc1.py`,
   598 transfers whose next instruction reads the written FPR): a `nop` is present
   in **587** of them, at **every** distance of the source GPR (1, 2, 3, 4, 6 … 86)
   and even when that GPR is never written in the function (20 cases). The 1999
   source says the same (`gas/config/tc-mips.c`, `INSN_WRITE_FPR_T` branch: nop as
   soon as the next instruction uses the FPR). The rule is therefore the plain
   "next instruction reads the written FPR" test. In that original survey,
   the 11 cases without a `nop` sit in 5 functions
   (`0x00283ce0`, `0x00283c28`, `0x002a677c`, `0x002e0408`, `0x002e0558`), and in
   `FUN_00283CE0` the `mtc1` is the first instruction of the function. The
   earlier compatibility implementation exempted transfers after cleared
   assembler instruction history, which can also occur within a function;
   it is not a function-boundary detector. Eight retail first-transfer witnesses
   falsify that blanket exemption: four require a nop and four do not.
   The [restricted exemption transformer](../tools/compiler/transformers/restrict_mtc1_exemption.py)
   retains only two measured producer/consumer patterns after cleared history:
   `a0` to `f0` followed by `cvt.s.w f0,f0`, and zero to `f0` followed by
   `c.lt.s f12,f0`. Other combinations retain the existing hazard logic.
   This is an empirical compatibility rule for the qualified C corpus,
   rather than recovery of the original compiler or ordering directives.
   An earlier attempt narrowed the rule to "the GPR was written two instructions
   back" (two data points) — it refused the `nop` in `FUN_002A7878`, where the
   retail has one.
5. **Short-loop padding after delay-slot scheduling.** The cumulative stack's
   patch 0054 disables the post-DBR hook because its original pipeline uses
   Ps2EeAs to pad short loops. This repository's C pipeline uses GNU `as`,
   which has no compensating loop-padding pass. Restore the existing
   `mips_r5900_pad_loops` hook after the other 0054 changes, using
   [`enable_loop_padding.py`](../tools/compiler/transformers/enable_loop_padding.py).
   The two polling loops of `FUN_0034FB20` otherwise lack the padding needed
   to reach the measured minimum of seven instructions. With the hook and
   the correct local call view, the complete body matches at 168 bytes.
   All 114 previously accepted bodies are unchanged under this compiler.
   Count a `TRAP_IF` using its machine-description instruction length: the
   R5900 division guard expands to a branch and a break, rather than one
   encoded instruction. Counting it as one overpadded a seven-word loop in
   `FUN_0028B950`; the [counter patch](../tools/compiler/transformers/count_trap_length.py)
   removes that extra padding while preserving all 143 earlier bodies.

The original 598-case survey covered one floating-point source field. A fresh
survey covering both source operands finds **915 immediate dependencies: 897
with a delay and 18 without**. It retains all 598 original cases. The ten cases
in `0x00283c28`, `0x002a677c`, `0x002e0408` and `0x002e0558` are reproduced by
explicit instruction-ordering control; their reordering-mode witnesses and two
positive controls retain the delay. The private diagnostic passed 52 fixtures.
This establishes a sufficient assembler mechanism without recovering the
original source directives. Synthetic alignment and branch witnesses trigger
the compatibility exemption inside a function. ISA selection supplies another
sufficient mechanism, so source provenance remains unresolved. This diagnostic
does not qualify every transfer in the game. The later restricted exemption
preserves all 165 accepted bodies and makes the two additional integer-to-float
bodies exact. With the independently authored RLE body, the combined gate is
168/168. All 52 original fixtures remain unchanged. Fourteen distance controls,
twelve boundary controls and eleven first-transfer controls bring the private
fixture suite to 89 cases. Synthetic controls have no retail oracle; the retail
first-transfer sample contains only four distinct patterns. A conflicting exact
body or a retail witness contradicting the restricted patterns would falsify
the compatibility claim. The four exception functions remain explained by a
sufficient mechanism; their original source directives remain unresolved.

6. **A TImode zero is folded into the store that consumes it.** The earlier
   `allow_zero_ti_store.patch` (retired on 9 October 2026 and no longer in the
   tree; its text stays in the file's history) admitted
   constraint `J` on the register alternatives of `movti_internal` and printed
   that alternative with `%z1`, so an actual zero reaching a 128-bit store
   selected `sq` from architectural zero. It earned no witness of its own: the
   control sources that store a
   zero through a `mode(TI)` pointer — including the `FUN_00282C88` spelling —
   are byte-identical with and without it, and it only ever moved the *memory*
   alternatives, which already accepted `J`.
   Removing it bare is not neutral either: the retail writes a TImode zero by
   materialising it once and storing the register, and two published bodies
   that the patch used to fold to `sq $zero` regress — `FUN_00282C88` (8 bytes)
   and the complete 168-byte `FUN_002E5FE0`, whose counted 52-entry loop needs
   the register form. The [fold transformer](../tools/compiler/transformers/fold_zero_ti_store.py)
   restores both without the constraint: `rac2_fold_zero_ti_store` rewrites a
   `(set (mem:TI) (reg))` back to architectural zero when that register is dead
   after the store, and leaves the register in place when it feeds several
   stores, which is what the families that need the shared
   `por $rd,$zero,$zero` require.
   **Placement is part of the identity.** A naive repair at the obvious hook
   (`MACHINE_DEPENDENT_REORG_AFTER_DBR_2`, i.e. after reload but after the
   second scheduler has already run) loses `FUN_002E5FE0`: sched2 never sees
   the folded form and emits `addiu $v1,$v1,-1` before the store instead of
   after. The pass must run from `toplev.c` **before
   `flag_schedule_insns_after_reload`** through the new
   `MACHINE_DEPENDENT_REORG_AFTER_RELOAD` hook.
   A fresh current-profile qualification reproduces the complete 8-byte
   `FUN_00282C88` (one 128-bit zero assignment) and the complete 168-byte
   `FUN_002E5FE0`. The `FUN_00282C88` witness is recorded in the
   experiment register; it does not imply
   that every zero-store spelling selects the same form.
7. **Preserve the generic frame scheduler by default.** The cumulative P21
   option forces emission order between two frame-related instructions. The
   earlier RAC2 recipe enabled that option. The
   [default transformer](../tools/compiler/transformers/disable_frame_order_default.py)
   restores the generic scheduler default; the explicit opt-in remains.
   `FUN_002B7170` requires `s2`, `s0`, `s1` saves rather than the forced order.
   Its source also needs the measured constant lifetimes and final store
   order. With that source, the previous profile differs only in the three
   prologue words; the new profile reproduces the complete 372-byte body.
   All 176 earlier qualified bodies remain exact, for a combined 177/177.
   This qualifies the current corpus, not every frame layout in the game.

Two complete builds reproduce release compiler `1ae7dceb` with the zero-store
change, and another two reproduce `8bed6eae` with the frame option default off.
`cpp` and GNU `as` retain their hashes. The complete release rebuilds exclude
all diagnostic buffer/ranking instrumentation. The intermediate object-rebuild
hashes remain private diagnostics rather than release identities. The
`5fed4e23` compiler was built from the four sources the
[fold transformer](../tools/compiler/transformers/fold_zero_ti_store.py) produces; applying
that script to the qualified source tree was re-verified on 9 October 2026 to
reproduce the four file hashes below byte for byte.

Source-level lessons the witnesses also pinned down:

- A 16-byte copy must go through a 128-bit integer type
  (`__attribute__((mode(TI)))`) to reach `lq`/`sq`; the aggregate path builds
  `ld`/`sd` pairs or a `memcpy` call (`FUN_002A8C00`).
- The callee's prototype decides `$v0` vs `$v1` for a rematerialised constant
  after a call: a value-returning prototype keeps `$v0` busy (`FUN_002889B8`).
- A result variable distinct from the floating-point input parameter avoids an
  extra register copy in `FUN_002A78D8`; this source change is required in
  addition to the reordered prologue.
- A local `void`-returning function-pointer view can preserve the shared
  value-returning declaration while reproducing the register allocation at a
  particular call. The accepted call still targets the same measured address;
  this does not recover the library's original C prototype.

## Adoption of the folded zero store (9 October 2026)

**Identity.** `cc1` moves from `8bed6eae…` to `5fed4e23…`. `cpp` (`2ac3d8d3…`)
and GNU `as` (`cda1a4e4…`) are unchanged on that date, so the assembler half of
the identity is untouched by this adoption. (The `as` was separately superseded
by `d81f2e93…` in the division-erratum change documented below; `cc1` was not
affected by it.) Four sources change: `config/mips/mips.md` (constraint and printer
change retired), `config/mips/mips.c` (the fold pass), `config/mips/mips.h` (the
hook) and `toplev.c` (the call site). The retired
`allow_zero_ti_store.patch` was removed from the tree;
`git show 2690e43:scripts/compiler/allow_zero_ti_store.patch` still returns its
text. The public
[`fold_zero_ti_store.py`](../tools/compiler/transformers/fold_zero_ti_store.py) produces
the four sources, and applying it to the qualified source tree reproduces their
hashes byte for byte (checkpoint table above).

**Why the patch went.** It earned no witness of its own. The control sources
that store a 128-bit zero through a `mode(TI)` pointer — including the
`FUN_00282C88` spelling — are byte-identical with and without it, because it
only ever moved the *memory* alternatives of `movti_internal`, which already
accepted `J`. What it did do was suppress the register form the retail uses when
one materialised zero feeds several stores, and that is what parked the
`41eb487e64fb6b76` family (444 bytes × 27 placements): 110 of its 111
instructions matched and the missing one was the shared
`por $v0,$zero,$zero`.

**What it costs to revert naively.** Two published bodies regress when the
constraint is removed without a replacement: `FUN_00282C88` (8 bytes, the
`j $31 ; sq $0,0($4)` witness) and `FUN_002E5FE0` (168 bytes, the counted
52-entry loop). The pass restores both, and keeps the register form where the
retail keeps it.

**The pitfall.** The pass must run **before the second scheduler**. Hooked where
the other reload-range passes run (after reload, i.e. after sched2),
`FUN_002E5FE0` comes out with `addiu $v1,$v1,-1` before the store instead of
after: sched2 never saw the folded form. It is called from `toplev.c` before
`flag_schedule_insns_after_reload` through `MACHINE_DEPENDENT_REORG_AFTER_RELOAD`.

**Requalification.** The whole published corpus was re-qualified on that
tree with `5fed4e23`, function by function: **6996 / 6996 complete C functions
exact — 309 in boot (17 148 bytes), 5289 across the 27 native units
(472 932 bytes) and 1398 across the 27 small-data units (81 664 bytes)**, with
zero non-exact bodies. Every published review differs from the `8bed6eae`
artifact in exactly one line, the recorded `cc1` hash: no function record, no
object hash, no candidate ELF hash and no matched byte moved. The boot object is
byte-identical (`object_sha256` `b5a2be9b…` before and after), and the full
boot-and-27-overlay loaded-image gate passes unchanged on the new chain
(79,486,851 loaded bytes compared, `failures: []`, 812,824 integrated C bytes —
the same totals as under `8bed6eae`). The batch-specific intermediate candidate
images differ from the previous batch only inside the build path they embed;
the compared `PT_LOAD` content and every published function record are identical.

The pass is extended on 10 October 2026 to admit one *reconstructible* reader —
the function-value `use` — in the section below.

**Family `41eb487e64fb6b76`.** With the fold pass the recovered body reaches the
retail's full size at every one of its 27 placements — 111 instructions /
444 bytes, against 110 / 440 under `8bed6eae` — and the shared
`por $v0,$zero,$zero` feeding three `sq $v0` stores comes back. It is still
**not** byte-exact and stays unintegrated: on the seed placement
`0_aranos_tutorial@0x002B91C0`, 110 of the 111 instructions are identical and in
the retail order, and the single remaining difference is the `== 2` constant
materialised in `$v0` where the retail has it in `$a0`. The other placements
carry overlay-specific callee addresses, so each still needs its own placement
verification before any promotion.

## Reproducing the chain

Source: `gnu-ee-binutils-gcc-1.1.tar.gz`, sha256
`1f518043e252d6eda726386971d52eda26541ab936ea73a9783d73712b595f92` (the ps2dev
archive of the Sony/Cygnus EE compiler sources, target `mips64r5900-sf-elf`).

Patch stack: the cumulative `sce-991111b` stack published with the **Lombyte**
project (github.com/mateuszklysz/Lombyte, `patches/sce-991111b/`), minus its
`saves` widening, plus the qualified adjustments above. The build host is WSL with 32-bit
support (`gcc -m32`) and bison 1.28; the compiler and assembler are hashed in the
proofs:

| Tool | sha256 |
|---|---|
| `cc1` | `8bed6eaeec23dba7b10c94e3d907416cf9931c1ddc69ce5ffd2068497a02ad5d` |
| `cpp` | `2ac3d8d3ca177e6705ac2cbdd1bd9e9a7181ac3e40f6230dea6875c3218ec155` |
| `as` | `cda1a4e43dc8eaef2670d2445d6916050137330b2051a0695fe0d2631f3d7876` |

(The current `cc1` is `37704f48…` and the current `as` is `c71a15db…`; this
table is the 7 October milestone those descend from, through the
folded-zero-store identity, the division-erratum assembler, the two
division fixes of 9 October 2026, the save-block order and the return-value
zero fold of 10 October 2026 documented below.)

(An earlier `as`, `87a1a012…`, carried the two-point `mtc1` rule described above
and has been superseded. The earlier `cc1`, `3e7628b7…`, emitted the GPR save
block first. `c9952c1b…` reordered the save blocks; `158e5c20…` additionally
restores the post-DBR loop hook. `dff08a34…` additionally counts division-guard
expansions by their MD length. The new compiler was completely rebuilt in
two separate source/build directories, with identical hashes. `cpp` and the
`as` at that compiler milestone was unchanged. The subsequent `cda1a4e4`
assembler narrows the cleared-history exemption; `20c5f50b` is retained as its
predecessor. Two complete builds from fresh archive extractions produce identical
`cc1`, `cpp` and `as` hashes, and each passes 168/168 bodies and 89 fixture cases.)

Apply the restricted exemption patch after the earlier transfer-hazard patch
and before building gas. An earlier incremental diagnostic produced assembler
hash `5a0c6e9e`; compiling the identical `tc-mips.c` as `./config/tc-mips.c`,
as the normal Makefile does, accounts for the release hash difference. A controlled
recompilation changing only that source argument reproduces `cda1a4e4`.

The 1999 Makefile omits a dependency from `flow.o` to `insn-flags.h`, so a
clean parallel build can race the generated headers. Generate `insn-flags.h`,
`insn-codes.h` and `insn-config.h` before the parallel `cc1`/`cpp` build. This
build-order repair does not change the resulting compiler hash.

The linker stays the SDK `ld.exe` used before. The pipeline drives the chain
through `scripts/wsl_chain.py` (the 1999 tools are 32-bit Linux binaries: they
run under WSL, and every source is compiled under its bare name inside the WSL
filesystem, so the object is reproducible from any checkout).

There are two assembly paths in the integrated build. Reviewed C goes through
`cpp`/`cc1` and GNU `as` to produce its C object. The remaining reconstructed
assembly inputs still go through `Ps2EeAs.exe` in `scripts/build.py`. That build
then links both sets of objects; Ps2EeAs does not reassemble the already produced
C object. The C path was introduced in commit `b2b9101`. A padding assumption
about Ps2EeAs therefore does not automatically apply to the GNU-assembled C.

## Complete rebuild recipe

A partial recipe silently yields a different `mips.c` and a different `cc1`: the
source adjustments below are part of the qualified identity, not optional
clean-ups. Run every step and check the three source hashes at the end.

**Host.** Ubuntu 26.04.x under WSL2 with 32-bit host support (`gcc -m32`) and a
locally built **bison 1.28** first on `PATH`. The instruments are ordinary
host-built binaries, so their hashes depend on the distribution: a build on
another host produces different `cpp`/`cc1`/`as` hashes from identical sources.
No binaries are distributed; build them here and compare with the hashes above.

**1. Sources.**

```sh
tar xzf gnu-ee-binutils-gcc-1.1.tar.gz        # sha256 1f518043e252d6eda726386971d52eda26541ab936ea73a9783d73712b595f92
mv gnu-ee-binutils-gcc src
```

**2. Lombyte `sce-991111b` stack, minus the saves widening.** Apply, in this
order, from the stack published with the Lombyte project:

```
0000-modern-host-fixes 0001-r5900-quad-saves 0015-no-sibcall 0016-no-edge-lcm-default
0019-r5900-post-dbr-loop-pad 0020-gas-absolute-unknown-symbol
0021-sched-keep-frame-related-order 0022-sibcall-default-off 0025-annul-dead-delay-slots
0026-frame-save-first 0027-gas-inline-float-literals 0028-annul-ne-zero-default
0029-call-clobber-pending 0030-pad-before-preceding 0031-annul-traced-comparison
0032-anchor-all-pads 0033-ra-not-vs-nonframe 0034-r5900-extern-buffer-optin
0036-retire-frame-save-pref 0037-game-no-strict-aliasing 0044-sda-nosda-attributes
0045-encode-section-info-sda-nosda 0046-r5900-pad-unfilled-loops
0047-pathb-reload1-localalloc-regclass-2952 0048-pathb-cse-2952
0049-sibcall-pass-needs-placeholder 0050-r5900-dli-retail-form 0051-r5900-fpr-hazard-exact
0052-r5900-dli-retail-general 0053-gas-la-absolute-unknown-symbol
0054-r5900-assembler-pads-loops 0055-r5900-no-second-hilo 0056-sda-extern-before-use
```

then reverse the saves widening, because the retail saves in `sd` (8-byte slots),
not `sq` (16):

```sh
patch -R -p1 -s < "$P/0001-r5900-quad-saves.patch"
```

**3. Source adjustments.** Each one is measured; omitting any of them changes
`mips.c` and therefore `cc1`.

| File | Adjustment | Reason |
| --- | --- | --- |
| `gcc/config/mips/mips.c` | the `if (TARGET_MIPS5900)` line preceding `mips_reg_mode[0] = TImode;` becomes `if (0 && TARGET_MIPS5900)` | the retail does not use TImode in that position |
| `gcc/config/mips/mips.h` | add `#define MACHINE_DEPENDENT_REORG_AFTER_DBR(X) mips_r5900_pad_loops (X)` before `extern void mips_r5900_pad_loops ();` | patch `0054` disables the hook for the Ps2EeAs assembler; the C path uses GNU `as` and must keep it |
| `gcc/config/mips/mips.c` | in `mips_r5900_pad_loops`, `n++;` becomes `n += pat == TRAP_IF ? get_attr_length (insn) : 1;` | a division guard expands to two MD words (`beql` + `break`) |
| `gcc/config/mips/mips.c` | emit the GPR save loop in **ascending** register order, with `gp_offset -= GET_MODE_SIZE (mips_reg_mode[0]) * (n_rac2 - 1)` pre-computed when more than one register is saved and the per-register decrement turned into `+=` | the retail saves in ascending order with the same layout and offsets |
| `gcc/config/mips/mips.c` | run the FPR save block before the GPR save block (and keep GPR restores before FPR restores) | the retail orders the two intact blocks that way; patch `0026` covers the frame-save case, this completes it |
| `gas/config/tc-mips.c` | insert the `rac2_mtc1_nop_ok()` helper and guard both `++nops` sites with it: a `nop` follows `mtc1` when the next instruction reads the written FPR, except when `mtc1` is the function's first instruction | measured 587 nop in 598 cases; the single exception is `FUN_00283CE0` |
| `gas/config/tc-mips.c` | add a measured SDK 3.01 incoming division floor; count emitted NOPs, retain division labels on padding, isolate code-label state, and bound the counter at two | SDK 3.01 warnings and NOP runs; this is a limited subset with explicit fragment and noreorder failures; see [the division-erratum section](#division-erratum-padding-measured-sdk-301-subset-9-october-2026) |
| `gcc/config/mips/mips.md` | the `movti_internal` register alternatives lose the `J` constraint (`"d,R,m,dJ,dJ,…"` becomes `"d,R,m,d,d,…"`) and the two store alternatives print `%1` again instead of `%z1` | the retired `allow_zero_ti_store.patch` earned no witness of its own; the architectural-zero fold is now the back-end pass below |
| `gcc/config/mips/mips.c`, `gcc/config/mips/mips.h`, `gcc/toplev.c` | add `rac2_reg_live_after_store_p` and `rac2_fold_zero_ti_store` before `machine_dependent_reorg`, declare `MACHINE_DEPENDENT_REORG_AFTER_RELOAD`, and call it from `toplev.c` **before** `flag_schedule_insns_after_reload` | the retail materialises a TImode zero once and re-folds it into the store; running the pass after the second scheduler loses `FUN_002E5FE0` |

**4. Public transformers.** Every measured adjustment above is shipped as a
script that carries its exact replacement text; the five source adjustments are
`neutralise_timode_anchor.py`, `enable_loop_padding.py`,
`count_trap_length.py`, `ascending_save_order.py` and `reorder_save_blocks.py`,
and the ones that touch the assembler and the machine description are
`fold_zero_ti_store.py`, `disable_frame_order_default.py`,
`restrict_mtc1_exemption.py`, `pad_div_erratum_nops.py`,
`neutralise_div_dslot.py` and `pad_div_erratum_branch.py`.
`pad_div_erratum_branch.py` applies **after** `pad_div_erratum_nops.py`: it
rewrites that rule's generated anchors, so the label trigger keeps the exact
text the 9 October 2026 label correction validated.
`fold_zero_ti_store.py` takes the `gcc` source
directory rather than one file, because it retires a machine-description change
and inserts a back-end pass across three more sources at once; the
`allow_zero_ti_store.patch` it replaces was removed from the tree on
9 October 2026 and must not be applied again (its text is preserved in that
file's history). `fold_zero_ti_return_value.py` (the fourth chain fix, 10
October 2026) also takes the `gcc` source directory and applies **after**
`fold_zero_ti_store.py`: it rewrites the guard that pass inserts, so the guard
must exist first. Applying the table above as prose instead of
running these files yields a *different* `mips.c`: two of the replacements carry
annotation text, and a differently worded comment changes the hash even though
the generated code is identical. The order of the last three is immaterial —
they touch disjoint regions of the machine description, `mips.c` and
`gas/config/tc-mips.c` — but the relative order of the five source adjustments is not:
the save-block reorder rewrites the region the ascending-order replacement has
already produced.

**5. Configure and build.**

```sh
export CC='gcc -m32'
export CFLAGS='-O2 -fno-strict-aliasing -fcommon -std=gnu89 -D_GNU_SOURCE'
./configure --target=mips64r5900-sf-elf --host=i686-linux-gnu --build=i686-linux-gnu     --disable-nls --enable-languages=c --without-headers
(cd libiberty && make -j16 CC="$CC" CFLAGS="$CFLAGS")
# The 1999 Makefile omits a dependency from flow.o to insn-flags.h, so a clean
# parallel build can race the generated headers. Build them first.
(cd gcc && make -j1 LANGUAGES=c CC="$CC" CFLAGS="$CFLAGS" insn-flags.h insn-codes.h insn-config.h)
(cd gcc && make -j16 LANGUAGES=c CC="$CC" CFLAGS="$CFLAGS" cc1 cpp)
(cd bfd && make -j16 CC="$CC" CFLAGS="$CFLAGS")
(cd opcodes && make -j16 CC="$CC" CFLAGS="$CFLAGS")
(cd gas && make -j16 CC="$CC" CFLAGS="$CFLAGS")     # produces gas/as-new
```

**6. Checkpoints.** The build is the qualified one only when all eight identities
match:

| Artifact | sha256 |
| --- | --- |
| `gcc/config/mips/mips.c` | `4a6a1ae14448e504eef4f58450973ebcd6c3ba45fdf204961642f0b97138fb8a` |
| `gcc/config/mips/mips.h` | `87d59c06d047cf7252349cb02aebc266f8b4dcecd537c827e6ec3966a10bbfc3` |
| `gcc/config/mips/mips.md` | `9720897dc353fd246c6f94d5882c3beb16c81fa9e42438ee8d184c8a78a581ad` |
| `gcc/toplev.c` | `38d52727addc0b0b299700835788808cf23fe59c863a961cdd6f28170faaf3cc` |
| `gas/config/tc-mips.c` | `b22dfff81e41d3d378b1f35a231d2c11b60700967d6faf70b820f6300caf81d6` |
| `gcc/cc1` | `37704f483fba7269791576879b3573445cd455ce0473e6d58a3bcceb45105f95` |
| `gcc/cpp` | `2ac3d8d3ca177e6705ac2cbdd1bd9e9a7181ac3e40f6230dea6875c3218ec155` |
| `gas/as-new` | `c71a15dba889fc273b0b699986de7316d056b94ae2155ad2c2573eaa62b8d902` |

The whole recipe was last verified end to end on 7 October 2026 (six identities,
then-current `cc1`). The `gcc` rows and `cc1` above are the 10 October 2026
identities: the whole list above was replayed from a fresh archive extraction on
that date and reproduced all eight hashes, and the previous `mips.c`/`cc1` pair
(`7952e5da…`/`4d069ae4…`) was reproduced the same way immediately before it.
The list was replayed a third time from a fresh extraction for the return-value
fold of 10 October 2026 and reproduced all eight hashes above; the same list with
the new transformer omitted reproduced the previous eight
(`58d25b2c…`/`adb1c1b4…`) on the same host in the same run, so the two rows that
move are the transformer's work and not a build difference.
The 9 October 2026 identities were the same apart from those two rows; the same date's `mips.md` was `177caa69…` before
`neutralise_div_dslot.py` and is `9720897d…` after it, and its `tc-mips.c` was
`61e51c1e…` before `pad_div_erratum_nops.py`, `b014add9…` after it and
`b22dfff8…` after `pad_div_erratum_branch.py`. A rebuild of the unmodified
qualified tree reproduced `5fed4e23…` and `d81f2e93…` on that same host, so the
two new rows are the transformers' work and not a build difference. A `mips.c`
that hashes differently means a step above is missing
— a rebuild that skips the adjustments produces a compiler that still matches the
measured corpus on simple bodies and diverges elsewhere, which is exactly the
failure mode this section exists to prevent.

## Division-erratum padding: measured SDK 3.01 subset (9 October 2026)

**Identity and reproducibility.** The published GNU `as` moves from
`cda1a4e4…` to `d81f2e93…`. The unchanged compiler and preprocessor remain
`5fed4e23…` and `2ac3d8d3…`. Applying
[`pad_div_erratum_nops.py`](../tools/compiler/transformers/pad_div_erratum_nops.py) to
the qualified `61e51c1e…` source reproduces `b014add9…` byte for byte.
A fresh archive extraction and the public recipe reproduced all eight
checkpoints above. The intermediate, unmerged assembler `a7d0c916…` is
superseded: its NOP-only tests missed displaced labels.

**Oracle provenance.** The padding oracle is authentic SN ProDG **3.01**
`Ps2EeAs`, SHA-256
`cb5adda955e64626564212ef7e0c1434708c4e1ef423344a92ec8033306ed3aa`.
The private diagnostic copy is byte-identical to that SDK executable; it is
not an instrumented assembler. It differs from the SN ProDG **2.0** tool used
for full-image ASM reconstruction, SHA-256
`c839dd63facabe7b76573c114056be61eaa7b3aa2329dd546930ab8f98898c76`.
The two versions disagree on 44 of the 97 synthetic `.text` outputs tested.
Do not identify either executable as the proven shipping-game assembler or
claim that this change reproduces both versions. Tools, objects, game bytes
and the private diagnostic bank remain outside the repository.

**Measured incoming floor.** In the tested SDK 3.01 contexts, `div.s`,
`sqrt.s` and `rsqrt.s` require two emitted slots after a code label. The label
denotes the start of the padding, not the division after it. A required
coprocessor hazard NOP contributes to that minimum rather than adding to it.
An immediately preceding `sync.p` supplies another one-NOP minimum; `sync`
and `sync.l` do not trigger it. Integer HI/LO division does not trigger this
floor, and the oracle rejects the double-precision forms. No rule is keyed
to a game address, program or function name.

**Bounded implementation.** The counter saturates at two and includes NOPs
actually emitted by `emit_nop`. Code labels alone update the state; a data
label cannot contaminate the current text stream. For a code label in the
current fragment, the helper measures emitted byte distance. Other fragments
use the instruction/NOP fallback. The existing label-relocation loop is
skipped only for the active division floor, so a branch to the division
label lands on its padding. A second transformation is refused without
changing the file, including under Python `-O`. Generated comments are English.

**Stronger validation.** The original 49-witness test checked only preceding
NOP run lengths; it was not proof of full text, label or branch equivalence.
The retained intermediate objects fail 13 strict comparisons: 11 displaced
label cases and two pre-existing integer macro differences. The final
assembler passes **47 of those 49** strict comparisons, fixing all 11 labels.
The expanded 97-witness battery compares every `.text` byte, required source
label presence and offsets, branch targets and delay words, and canonical
relocations. Its final result is **84 PASS, 13 FAIL, 0 ERROR** against genuine
SDK 3.01. None of the previously passing cases regressed. Eight tests of the
comparison harness pass. These counts are qualification evidence for the
stated subset, not full-assembler equivalence or matching credit.

The 13 strict failures remain explicit: two integer macro expansions, two
local relocation encodings with different raw addends, three post-division
hazard cases at a noreorder transition, four ordinary noreorder cases that
differ between SDK versions, and two cross-fragment `.align` / `.word 0`
distance cases. No byte masks, crops, relocation exemptions or acceptance
waivers turn these failures into matches. In particular, the same-fragment
helper does **not** establish general support for `.word` directives,
alignment, relaxation or removal of speculative NOP fragments.

**Delay-slot limitation.** SDK 3.01 emits a warning that automatic padding
cannot take place for a division in a branch delay slot; it still returns
success and produces an object. Calling that a compilation refusal was
incorrect. This patch does not change compiler scheduling, add padding
inside a branch delay slot, or reproduce SDK 3.01 ordinary noreorder floors.
A missing code label or unsupported fragment transition can still leave a
candidate short. The two retained division-family attempts remain mismatches
and receive no credit. The compiler half of the same problem — a division the
scheduler moved *into* a delay slot — is addressed separately by
[`neutralise_div_dslot.py`](../tools/compiler/transformers/neutralise_div_dslot.py), and
the label trigger above is extended to its branch form by
[`pad_div_erratum_branch.py`](../tools/compiler/transformers/pad_div_erratum_branch.py);
both are documented in the section below.

Current-corpus acceptance remains the separate exact-byte requirement:
27 native and 27 small-data object qualifications reproduce their prior
objects, candidate ELFs and every function result; the boot review reproduces
all 309 complete definitions. Full boot and 27-overlay integration gates,
fresh catalogue/binding exports and CI must pass on this identity before
publication. No C body, declaration, prototype or placement is changed.

## The division vein: delay-slot barrier and branch-group floor (9 October 2026)

**Identity.** `cc1` moves from `5fed4e23…` to `4d069ae4…` and GNU `as` from
`d81f2e93…` to `c71a15db…`; `cpp` (`2ac3d8d3…`) is unchanged, so the compiler
half and the assembler half move for different reasons. Two sources move:
`gcc/config/mips/mips.md` (`177caa69…` → `9720897d…`) and
`gas/config/tc-mips.c` (`b014add9…` → `b22dfff8…`). The public transformers are
[`neutralise_div_dslot.py`](../tools/compiler/transformers/neutralise_div_dslot.py) and
[`pad_div_erratum_branch.py`](../tools/compiler/transformers/pad_div_erratum_branch.py);
applying them to the qualified source tree reproduces both source hashes byte
for byte, and a rebuild of the **unmodified** tree in the same build environment
reproduced `5fed4e23…`/`d81f2e93…`, so the two new rows are the transformers'
work and not a difference of host or build order.

**Why the compiler half.** `dbr_schedule` (`reorg.c`), reached from `toplev.c`
under `#ifdef DELAY_SLOTS` whenever `optimize > 0 && flag_delayed_branch`, fills
delay slots with a backward scan and no `may_trap_p` guard, and accepts a
candidate whose `dslot` attribute is "no" with length one — which `div.s` is.
The retail build never does that: a census of the pinned reference image finds
**0 of 19 806 `div.s` occupancies in a delay slot**, while integer HI/LO
division occupies **61**. The oracle agrees: on the emitted assembly of the
affected family the authentic SDK 3.01 assembler refuses a `div.s` in a delay
slot ("Automatic padding cannot take place"), and accepts the same body once the
scheduler leaves the division in place. The transformer adds the
single-precision COP1 division/root family to the `dslot` barrier list, and
leaves integer HI/LO division eligible.

**Why the assembler half.** The floor merged on the same day is keyed to a
**code label**; the witness family has no label between the function entry and
its division. Probing the authentic SDK 3.01 assembler with complete snippets
shows the trigger of that site is a **branch group**: `bc1t ; ld ; div.s` gains
two `nop`, `bc1t ; ld ; addu ; div.s` gains one, `bc1t ; ld ; addu ; addu ;
div.s` gains none. Conditional branches, branch-likely forms (`beql`, `bc1tl`)
and the unconditional `b` all trigger it; `j`, `jal`, `jr` and `jalr` do **not**
(`jal ; nop ; div.s` gains nothing, and so does `jal ; nop ; addu ; div.s`), so
the generated test is the opcode class and not a mnemonic table. `sqrt.s` and
`rsqrt.s` carry the same floor and integer HI/LO division does not. The
transformer is applied after the label rule and rewrites that rule's own
generated anchors; the label trigger keeps the text the earlier correction
validated. It is deliberately **not** cleared by an immediately preceding
`.set noreorder` region: the witness site keeps its branch and delay slot inside
a compiler-generated `.set noreorder` block with the division in the following
`.set reorder` block, and the oracle still pads it.

**Zero measured impact on the published corpus.** Compiling the 55 published
candidate sources with the new compiler and assembling them with the new
assembler reproduces the previous chain's objects **byte for byte (55/55)**; the
`.s` files are identical too, so the assembler half alone also moves nothing.
The regenerated boot review differs from the published one in exactly
`tools.cc1`, `tools.as` and `verified_at` — all 309 complete boot definitions,
the object hash, the candidate ELF hash and the read-only sections are
unchanged. The 27 native and 27 small-data unit reviews differ in exactly the
two recorded instrument lines each (5 289 + 1 398 complete definitions,
472 932 + 81 664 bytes). No published function changes status, and no C body,
declaration, prototype or placement changes.

**What the vein yields, measured.** Of the 4 657 unclaimed families, 460 carry a
`div.s`; 117 of those have no mixed GPR+FPR frame, 72 carry a run of two `nop`
at a division site, and 9 carry the branch trigger. The witness family
(`4fbed03b15cb87e5`, 572 bytes × 27 placements) now reaches the retail size —
**572 bytes / 143 words, against 560 under the qualified `5fed4e23` chain and
564 with the compiler half alone** — and its division region `bc1t ; ld ; nop ; nop ; div.s ;
jal ; nop` is byte-identical to the retail; it still does not match (85 words
differ), because its prologue interleaves the GPR and FPR saves and hits the
mixed-frame wall from the third word. The one non-vector branch-triggered family
(`ad157649cfeed6f2`, 220 bytes × 8 placements) reproduces its branch-triggered
division site as well but comes out 208 bytes: the retail also copies the
incoming register (`mov.s $f3,$f12`) and carries two `nop` in front of its
**first** division that the SDK 3.01 oracle does not insert for that instruction
sequence, so they are compiler-emitted in the retail build. Both remain
unintegrated.

**Declared reserves.** (1) The compiler mechanism is **sufficient**, not the
recovered original rule: the retail compiler's own reason for keeping `div.s`
out of delay slots is not known. (2) `reorg.c` and `rtlanal.c` are byte-identical
to the pristine `gnu-ee-binutils-gcc-1.1.tar.gz` archive in the qualified tree,
and no patch of the `sce-991111b` stack touches `mips.md`, so "this is not a lost
RAC2 adjustment" rests on that file identity. (3) `sqrt.s` and `rsqrt.s` carry
**no witness at all** in the retail image (0 occurrences of each); extending the
barrier to them is an extrapolation from the same COP1 family, and GNU `as`
does not assemble `rsqrt.s` at all. (4) The branch trigger is bounded: a
division emitted *inside* a noreorder region still receives no padding, and the
majority of the retail's two-`nop` runs at a division site follow a non-branch
instruction, are not produced by the SDK 3.01 assembler from that sequence
(probed directly), and therefore have a compiler-side cause this lot does not
recover.

## Save-block order: the FPR-first inversion is retired (10 October 2026)

**Identity.** `cc1` moves from `4d069ae4…` to `adb1c1b4…`. `cpp`
(`2ac3d8d3…`) and GNU `as` (`c71a15db…`) do **not** move: the assembler half of
the identity is untouched, because this change is entirely inside
`config/mips/mips.c`. One source moves — `mips.c` `7952e5da…` → `58d25b2c…` —
and `mips.h`, `mips.md`, `toplev.c` and `tc-mips.c` are byte-identical to the
previous qualified tree. The change is confined to the transformer that
introduced it: [`reorder_save_blocks.py`](../tools/compiler/transformers/reorder_save_blocks.py)
now emits the stock order instead of the inverted one.

**The rule that was wrong.** The transformer used to force the FPR emission
block ahead of the GPR block on saves, and the GPR block ahead of the FPR block
on restores. That is not what the retail image does, and it is not what the
original source does either. The SCE source in the qualified tree
(`config/mips/mips.c`, `save_restore_insns`) runs one loop and emits **the GP
block first, then the FP block, in both directions**.

A byte-level census of the pinned reference, decoding the save sequence of every
complete body, agrees. The measurement below is "the first save the body emits
is a GPR", over the frames that save at least one GPR **and** at least one FPR:

| program | mixed GPR+FPR frames | first save is a GPR |
| --- | ---: | ---: |
| `0_aranos_tutorial` | 1 102 | **92.3 %** |
| the retail boot | 533 | **90.6 %** |
| all 28 programs | 31 278 | **92.5 %** |

The weaker test — the whole GP block emitted before any FPR save — holds for
34.8 % of the tutorial's mixed frames, because the retail interleaves the two
groups in most bodies. That is a property of the scheduler, not of the block
order, and it is the reason the classifier above is stated as the first save
rather than as a block boundary.

**Dating the regression.** It is not a lost RAC2 adjustment but a defect of the
save-block recipe itself. Two `cc1` builds of 3 October 2026 bracket it: the
00:13 build emits the GP block first, the 06:23 build no longer does. The
inversion therefore entered with the recipe, and it is undone inside the recipe.

**Why not restore the pre-patch file.** Replacing `mips.c` with its pre-recipe
form does not build: the qualified `mips.c` also carries `fold_zero_ti_store.py`
(`rac2_fold_zero_ti_store`, called from `toplev.c`), and the link fails with
`undefined reference to rac2_fold_zero_ti_store`. The reversion belongs in the
transformer, and `ascending_save_order.py` must still run before it: this
transformer rewrites the region that one has already produced.

**How little moves.** The selector alone decides the emitted order. Both block
bodies are order-symmetric — each initialises the shared save-area base when it
runs first and reuses an already established base when it runs second — so the
two-pass loop, both block bodies, their offsets, their directions and the shared
base logic are untouched. A diff of the qualified `mips.c` against the new one
changes six lines: the transformer's three-line annotation, the two words inside
it that name the block, and the selector line.

**Reproduction.** Applying the transformer list to a fresh extraction of
`gnu-ee-binutils-gcc-1.1.tar.gz` reproduced all eight checkpoints above on
10 October 2026, and the same list with the previous revision of
`reorder_save_blocks.py` reproduced the previous eight on the same host — so the
`cc1` move is the transformer's work and not a build difference.

**Composition guard.** This change must be *added to* the adopted chain, never
substituted for it. The variant that first measured it was built from a working
tree that predates the two division fixes, so its `cc1` (`883d5abe…`) is **not**
the chain to install: it lacks the delay-slot barrier and the branch-group
floor. What was installed is the current qualified tree plus the selector flip
alone. Four behaviours were re-measured on the installed chain, each against a
negative control:

| behaviour | negative control | installed chain |
| --- | --- | --- |
| a single-precision division is not taken into a delay slot | `cc1 5fed4e23…` (pre-barrier) emits `jal g` then `div.s` | `div.s` standalone, then `jal g` |
| the assembler pads a division after a branch group | `as d81f2e93…` (label floor only) emits 0 `nop` | `bc1t ; lw ; div.s` gains 2, one interposed instruction gains 1, two gain 0; `b` triggers, `jal` does not |
| the assembler pads a division after a code label | — | label + 0 slots gains 2, +1 gains 1, +2 gains 0 |
| `FUN_00282C88` and `FUN_002E5FE0` | — | both byte-exact, `j $31 ; sq $0,0($4)` and the counted loop unchanged |

The assembler half is preserved by identity as well as by measurement: the
installed `as` is the same binary as the outgoing one
(`c71a15db…`), and `tc-mips.c` and `mips.md` are byte-identical.

**Control measurement on the published corpus.** All 55 published candidate
sources (boot and the 27 native and 27 small-data units) compile and assemble to
**byte-identical objects** under both chains — 55/55, 4 586 164 bytes either
side, and the intermediate `.s` files are identical too. The corpus does not
contain a body whose frame the inversion changed, which is exactly why it was
worth measuring.

**What the vein yields.** Six shared families were blocked by this and are now
byte-exact at every placement; all six are integrated in the same lot and the
requalification below counts them.

| family | body | placements | measured placements × size |
| --- | ---: | ---: | ---: |
| `8411efa90fa80b88` | 384 B | 53 native + 1 boot | 20 736 B |
| `2c74c194eb670e6a` | 104 B | 174 | 18 096 B |
| `458c670ec3072df2` | 540 B | 22 | 11 880 B |
| `40487154aa250cc8` | 296 B | 52 | 15 392 B |
| `6eb4f363e5305bed` | 172 B | 80 | 13 760 B |
| `c10c12168bf625ba` | 516 B | 27 | 13 932 B |

Three of them — `8411efa90fa80b88`, `2c74c194eb670e6a` and
`40487154aa250cc8` — close outright. `458c670ec3072df2` additionally needs one
invariant data address (`0x00189E20`) bound explicitly; the bound word is not
masked. `6eb4f363e5305bed` and `c10c12168bf625ba` were measured *before* this
change and did not match; they were re-measured against the installed chain and
close. One further family, `eb99aa89` (208 bytes), reproduces its retail body
under this order but is refused by the promoter on an unmodelled data role, so it
is measured and not integrated.

The promotion tool's own dry run verifies one placement byte by byte and the
others structurally, through their relocation signatures. The complete byte
equality that a promotion needs is established by the maintained unit
qualification, which asserts `different_bytes == 0` and an exact produced size
for **every** catalogued function of every unit.

**What it does not close.** The first four families below were parked on this
same wall by other agents and re-measured against the installed chain with the
pre-change compiler as a control; the last two come from the lot that first
measured the order. Every one of them *is* reached by the change — the mixed
GPR+FPR frame is corrected in all of them — but a second defect takes over, so
none becomes exact:

| family | residual before | residual after | what remains |
| --- | ---: | ---: | --- |
| `a61ca9b9c41fa885` (544 B) | 22 words | **8 words** | instruction ordering in the entry block around the first two calls, not a save-block problem |
| `a9d9dba317e30df2` (544 B) | 34 words | **27 words** | register allocation, frame-relative against register-relative addressing, and `bnez` against `bgtz` |
| `afeb08d657b74409` (548 B) | 16-word gap | unchanged (560 B candidate, 548 B retail) | an 8-byte alignment hole after a multi-word macro (the documented `as` artefact) plus constant materialisation order |
| `59491c4e9e39388c` (504 B) | 7 words short | unchanged | the only body that exists for it is a draft attempt, not a solution |
| `bdcda1de2c05fafe` (420 B) | 173 bytes | 172 bytes | the retail interleaves one FPR save *below* its siblings, which neither block order expresses |
| `3936f04294b84d4b` (628 B) | 620 B, 8 bytes short | unchanged | not a save-order wall |

The declared reserve is that the order alone is not a general key: it is the
retail's order for the frames that were measured, and the families above show
that a mixed frame is necessary but not sufficient.

**Requalification.** The whole published corpus was re-qualified on the
installed tree: **6 009 complete C functions exact across the 27 native units
(625 536 bytes) and 1 398 across the 27 small-data units (81 664 bytes)**, the
boot review reproduces all 324 complete definitions, and the full
boot-and-27-overlay loaded-image gate passes with no failure. No previously
matched body changed status; the run adds only the families this change closes.
For comparison, the first run of this requalification on the installed chain,
with four of the six families promoted, reported 5 905 native functions /
598 704 bytes on the same instruments — so the two families re-measured after
that run account for the difference.

## The function-value zero fold: `904cc63b097639e5` unblocked (10 October 2026)

**Identity.** `cc1` moves from `adb1c1b4…` to `37704f48…`. `cpp` (`2ac3d8d3…`)
and GNU `as` (`c71a15db…`) do **not** move, and `mips.h`, `mips.md`, `toplev.c`
and `tc-mips.c` stay byte-identical to the previous qualified tree, so the
assembler half of the identity is untouched. One source moves — `mips.c`
`58d25b2c…` → `4a6a1ae1…` — and the whole change is one transformer,
[`fold_zero_ti_return_value.py`](../tools/compiler/transformers/fold_zero_ti_return_value.py),
which rewrites the guard `fold_zero_ti_store.py` inserts and therefore applies
**after** it.

**The rule it extends, cited as it stands.** `rac2_fold_zero_ti_store` folds a
`(set (mem:TI) (reg))` to architectural zero, and deletes the materialisation
that produced the register, exactly when the store is the register's last use:

```c
static int
rac2_reg_live_after_store_p (insn, reg)
     rtx insn;
     rtx reg;
{
  rtx scan;

  for (scan = NEXT_INSN (insn); scan; scan = NEXT_INSN (scan))
    {
      ...
      if (reg_referenced_p (reg, pat))
	return 1;			/* consomme avant toute reecriture */

      set = single_set (scan);
      if (set != 0 && GET_CODE (SET_DEST (set)) == REG
	  && REGNO (SET_DEST (set)) == REGNO (reg))
	return 0;			/* registre reecrit : le store etait la
					   derniere utilisation */
    }

  return 0;
}
```

The guard is not a comfort: it is the safety condition of the deletion. A single
reader left alive, and the deletion leaves a reader on a register nothing writes.

**The case that was missing.** For the family `904cc63b097639e5` the RTL before
the pass (`-dg`, `src.i.greg`) is:

```
(insn 210 (set (reg:TI 2 v0) (const_int 0)) 238 {movti_internal})
(insn  28 (set (mem:TI (reg/v:SI 22 s6) 0) (reg:TI 2 v0)) 238 {movti_internal})
(insn  31 (use (reg/i:SI 2 v0)) -1)
(jump_insn 33 (set (pc) (label_ref 190)) 450 {jump})
```

`movti_internal` accepts no constant zero on its store alternatives
(`"d,R,m,d,d,J,K,L,M,i"`: the stores are alternatives 3 and 4, source `d`), so
reload must materialise the zero in a register; and the pre-reload return
`(set (reg/i:SI 2 v0) (const_int 0))` degenerated into `(use (reg/i:SI 2 v0))`
because the same zero serves both the store and the return, which is why reload
placed it in `$v0`, the function-value register. `rac2_reg_live_after_store_p`
then meets that reference and refuses the fold, so the chain printed
`por $v0,$zero,$zero` + `sq $v0,0($base)` where the retail prints
`sq $zero,0($base)` + `move $v0,$zero`.

The reader is a read of *the constant being folded*, so its value is
reconstructible — which is what makes an exception admissible at all. The
transformer admits exactly one, bounded by four conditions: the pattern is
exactly `(use (reg/i ...))`, the register is the same one,
`REG_FUNCTION_VALUE_P` is set, no jump or call intervenes between the store and
the use, and there is exactly one such use. The store then folds to
architectural zero **and** the `use` is rewritten to
`(set (reg/i:SI 2 v0) (const_int 0))` — the pre-reload form, which prints
`move $v0,$zero`. Both changes go into the **same** `validate_change` /
`apply_change_group` group: if either pattern is not recognised the whole group
is abandoned and nothing is folded, a degradation that is safe and can never
leave broken RTL. A zero shared with any other reconstructible consumer (a call
argument, a phi) stays refused.

**The two counter-proofs.** Both were measured, and both are recorded here
because they are what stops a later simplification of this file:

| route | measured result |
| --- | --- |
| treating the `(use (reg v0))` as an end of life (variant C, `cc1` `b2429e74…`) | **wrong**: the store folds but no `move $2,$0` is emitted, so the body returns whatever `$v0` holds — the null path's `jal CALLEE0` result. Silent corruption, not a one-instruction miss: the return value must be re-materialised, which is the second half of this transformer |
| re-admitting constraint `J` on the store alternatives of `movti_internal` (variant B, `cc1` `d22de10d…`, `mips.md` `774f6b60…`) | reproduces this family as well and passes the 55 published sources, but **regresses** `41eb487e64fb6b76`: the shared `por $2,$0,$0` disappears and its three stores become `sq $0,64/80/96($sp)`, with the prologue reordered over 20+ lines. The constraint is a **global** rule — it changes what `cse` may substitute — so it removes the shared register from the families that need it. The pass is local and its new case is bounded. The constrained route is not reopenable as it stands |

**What it yields.** On the seed placement the difference is exactly two words:

```
40c40
< 	por $2,$0,$0
---
> 	sq $0,0($22)
44c44
< 	sq $2,0($22)
---
> 	move	$2,$0
```

`scripts/check_candidates.py` reports `MATCH` at the seed
(`0_aranos_tutorial@0x002B91C0`, 320 bytes), the promoter's owner gate
reproduces it again on the *rendered* fragment (the aliases the units actually
carry), and the family's own proof replays **27 of 27** level placements byte
for byte. The boot placement is not bindable by the family harness
(`family.py bindings` refuses a boot member by design); it is verified instead
by the boot review itself, which reproduces it complete and exact against the
retail image — `325/325` definitions with the promoted body.

| | |
| --- | --- |
| family | `904cc63b097639e5`, 320 bytes |
| placements | 28 — 26 native levels, the boot, and one small-data placement (level `4_barlow`) |
| promoted | 27 (26 native + boot) = **8 640 bytes** |
| kept out | the small-data placement (level `4_barlow`): the native-profile promotion skips it, by the promoter's documented rule for a nonzero-`-G` unit, and the same record cannot be promoted again with the small-data profile — the boot placement has no small-data unit and a mixed family is refused. Tried, not assumed |
| state | byte-exact on every promoted placement, boot included |

Census of the class in the retail images: 28 placements carry the tight
signature (a 16-byte store from `$zero` within three words of
`move $v0,$zero`), all of them in this one family, and 0 of the published paired
placements carry a `por $zero` word. The wider class — a `por $rd,$zero,$zero`
that feeds exactly one store — counts 4 164 sites, but the retail keeps the
register in those families and the fold is not what they need; nothing here
claims them.

**One source prerequisite, measured.** The promoted fragment declares a 128-bit
type, and this compiler refuses the `mode(TI)` spelling while a *typedef named
`TI`* is in scope: with `typedef int TI __attribute__((mode(TI)));` earlier in
the unit, a later `typedef … __attribute__((mode(TI))) …` fails to parse
(`parse error before 'TI'`), in either attribute position, measured on both a
two-line fixture and the boot unit. The same trap was hit independently, from
the other side, by the family lot merged before this one: its typedef extractor
took `TI` — the *mode* name inside `__attribute__((mode(TI)))` — for a type name
and rewrote the attribute to `mode(TI_F…)`, breaking three families. The boot
source carried exactly that typedef. It and its three users (the two bodies that clear or copy a 128-bit
object, `FUN_002A8C00` and `FUN_00282C88`) now spell the type `TI128`;
`src/boot/03-ported-utilities.cfrag` and `src/boot/12-aligned-clear.cfrag`
change, nothing else. The rename is code-generation-neutral and was checked that
way: the emitted assembly and the linked object are byte-identical before and
after, under both chains, and the boot review reproduces every definition
(325/325 with the promoted body). A typedef name does not reach the instruction
stream; the `FUN_00282C88` witness is still `j $31 ; sq $0,0($4)`.

**Composition guard.** This is *added to* the qualified chain, never substituted
for it: the tree is the current qualified tree plus this transformer alone.
Every previously adopted behaviour was re-measured against its own negative
control:

| behaviour | negative control | installed chain |
| --- | --- | --- |
| a single-precision division is kept out of a delay slot | `cc1` `5fed4e23…` (pre-barrier) emits `jal g` then `div.s` | `div.s` standalone, then `subu`/`sd`/`jal g` |
| the assembler pads a division after a branch group | `as` `d81f2e93…` (label floor only) emits 0 `nop` after `bc1t ; lw ; div.s` | that sequence gains 2, one interposed instruction gains 1, two gain 0; `b` triggers, `jal` does not |
| the assembler still pads after a code label | — | label + 0 slots / +1 / +2 give 2 / 1 / 0 `nop`, byte-identical to the outgoing assembler, which is the same binary (`c71a15db…`) |
| save blocks are emitted GPR first, then FPR | `cc1` `4d069ae4…` (pre-V3) emits `s.s $f22`/`s.s $f21`/`s.s $f20` before `sd $16`/`sd $31` on the dedicated probe | `sd $16`/`sd $31` first, then the FPR block; restores unchanged. The probe reads the emission order with `-fno-schedule-insns2` (probe-only: in the qualified profile sched2 interleaves both orders) |
| `FUN_00282C88` and `FUN_002E5FE0` | — | both byte-exact and byte-identical to the outgoing chain: `j $31 ; sq $0,0($4)` and the counted 52-entry loop |

**The day's refusals, re-tested rather than assumed.** The refused bodies that
mention `por` or `sq $zero` were measured under the new chain instead of being
carried over: `f_41eb487e64fb6b76`, `f_445bd756…`, `f_5df05f81…`,
`f_9f5f0a99…`, `f_50d4f5f2…`, `f_8977f392…`, the `122b6336` and `r09` witnesses
and the `w1` control all compile to byte-identical assembly under both chains —
their wall is the opposite one (the retail keeps the register there) and this
fold neither closes nor breaks them. Only `904cc63b…` changes. `ad5b681b…` and
`6c1e201f…` have no retained body to re-measure; `c10c1216…` was closed by the
save-block lot of the same morning.

**Control measurement on the published corpus.** At the lot's base, all 55
published candidate sources (boot and the 27 native and 27 small-data units)
compile and assemble to **byte-identical objects** under both chains — 55/55,
zero `.s` differing, 4 933 212 bytes either side. The corpus does not contain a
body in which the new case occurs: the only source the fold changes is the
family this lot promotes, and its fragment is added to the units *after* this
control, so the promotion is the one intended difference and it is measured by
the unit qualification and the image gate rather than hidden in this count.

Measured again *after* the promotion, on the final corpus, the two chains now
differ in exactly the 27 sources that carry the family — the boot and the 26
native levels — and by exactly **8 bytes each**, the two words the fold moves;
the other 28 sources (the 27 small-data units and the one native level with no
placement) are still byte-identical. Nothing else moves with the chain.

**Requalification.** The whole published corpus was re-qualified on the
installed tree: **6 035 complete C functions exact across the 27 native units
(633 856 bytes)** and **1 398 across the 27 small-data units (81 664 bytes)**,
the boot review reproduces all **325** complete definitions, and the full
boot-and-27-overlay loaded-image gate passes with no failure. Physical coverage
moves from 968 180 to 976 820 of 48 788 176 bytes (1.9845 % to 2.0022 %) and the
combined-reference boot binding stays at **22 576** static edges. No previously
matched body changes status; the run adds only the family this change closes.

The *unique* ledger does not move on this lot, and the reason is the placement
the promoter keeps out: its policy counts a structural class only when **every**
verified placement of the class is integrated, and this family still has its
small-data placement outside. `any_c_unique_bytes` does grow by the family's 320
bytes (292 728 → 293 048); `matched_c_unique_bytes` stays at 286 752 until that
placement is integrated too.

## Scope

This document claims what was measured: the named bodies and the 96 previously
integrated bodies are reproduced byte-for-byte; the profile is not offered as a
general RAC2 compiler qualification. Bodies that exercise VU/MMI instructions or
`$gp` are outside it. The four rules above are the ones the corpus could
falsify; a future body that disagrees with them is a measurement, not a surprise.

## English source transformers

The operand-exemption and frame-default adjustments are now expressed as
English Python source transformers, replacing patch files that included French
source annotations. They retain the same narrowly checked input states.

The exemption transformer reproduces the qualified `tc-mips.c` exactly
(SHA-256 `61e51c1ebcdf860db4503b6cc6a11c40596d1f3c969daf66ee56a45f454130ca`).
The frame transformer changes only the source comment relative to the qualified
frame-off source: its source hash is
`c76c0bec5b56c198381ab2a4fc60c161a4287e8312d7d1fdea3d1e6a0e1af614`,
and host preprocessing with the release flags produces identical output.
The active compiler and assembler remain the qualified binaries recorded above;
this documentation change does not claim a new binary rebuild.

## Small-data symbols under the pinned default profile (2026-10-07)

The default profile is `-O2 -G0 -ffunction-sections` and emits no `$gp` access
of its own. A body whose retail bytes address a global through `$gp` is
nevertheless reachable, because the choice is the assembler's, not the
compiler's:

* cc1 emits a bare symbol operand for a load or store it can expand as a macro;
  `gas` then decides per site — `lui`+`%lo` in ordinary flow, a one-instruction
  `$gp` form inside a `.set nomacro` region, which is where a compiler delay
  slot lands.
* `__attribute__((sda))` on the declaration restores cc1's one-instruction model
  for that symbol. It does not by itself force `$gp` anywhere; it makes cc1 emit
  the form that leaves the per-site decision to the assembler, which is what the
  retail build did. Declaring the symbol `nosda` instead makes cc1 materialise
  the address explicitly, and that two-instruction model shifts register
  allocation and scheduling away from the retail bytes.

Measured consequence: a body that a `nosda` assignment refuses can still be
exact when the same symbol is declared `sda`. The campaign driver therefore
retries a refused body with every measured `nosda` flipped to `sda` and keeps
only the variant the owner gate verifies on every placement.

The boundary that remains: an access that retail performs through `$gp` in
ordinary flow — not in a delay slot — needs the `.extern name, size` directive
that the reconstructed backend only emits under `-G8`. Such families stay on the
small-data unit route.

A related format constraint, removed on 2026-10-11: the level catalogue used to
require every external address to be word aligned, so a byte global at an odd
address was bound through its aligned base with a constant index. That clause
had no measured support — the level catalogue binds an external as an absolute
symbol in the linker script (`NAME = 0xADDR;`), which carries no alignment
requirement, and no other check on the native path held those addresses word
aligned: `scripts/campaign.py` does not look at a level external's alignment at
all, and the native catalogue's externals reach `scripts/integration.py` through
`compile_level_c`, which merges them without an alignment test. (The `% 4` still
in `integration.py` belongs to `level_catalog`, the separate shared placement
read from `config/level-catalog.json`, which this change does not touch.)
The byte flags 0x1A7B95 / 0x1A7B94 (family `c558ca7050ec6154`) and 0x1A7BB2 /
0x1A7BB3 (family `b7feb89380591f87`) link byte-identically through the qualified
chain, so a catalogue now names them directly. Only a non-integer type and an
address outside 0..0xFFFFFFFF remain refused. Binding a byte global through its
aligned base with a constant index stays valid where a body needs it.
