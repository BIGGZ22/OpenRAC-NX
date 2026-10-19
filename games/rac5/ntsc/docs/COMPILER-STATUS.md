# Compiler evidence and a limited boot-only profile

The boot executable is an ELF32 MIPS R5900 image with one loadable segment,
`.text` at `0x01E62500`, entry `0x01E62508` and `$gp = 0x01F300F0` in its
`.reginfo` section. It contains `.sndata` and `.gcc_except_table` sections.
The SNR2 level modules expose the mangled symbol
`lib_main__FiPPc`; several imports have C++-style mangled names. Together with
the SNR2 packaging, this is evidence of C++ and SN ProDG tooling in the
programs. It does **not** identify the exact compiler executable, version,
flags or assembler for each source unit.
The boot also retains 1,803 names in its embedded SNR2 table
([`BOOT-SYMBOLS.md`](BOOT-SYMBOLS.md)); names and address ranges are useful
for selecting witnesses but do not prove a compiler profile.

The current catalogue records **80,548 boot `.text` bytes in 635 C bodies**
across several code areas, built by the locally reconstructed GNU EE chain and,
for 163 of them, by one of four further pinned compilers
([witness evidence](COMPILER-WITNESSES.md)).
All of them link together into the complete boot text while preserving the
retail raw `PT_LOAD` bytes and program header
([combined witness](BOOT-C-WITNESS.md#combined-provisional-c-witness)); the
generated `config/boot/witness_boundaries.tsv` catalogue lists each linked body
with its compiled extent, its following zero alignment and the kind of control
transfer it ends in. Five hundred and sixty-one reviewed placements (57,420 bytes,
starting with
`MEMCARD_Busy__Fv`) are in `src/boot/`; the other bodies remain in
`nonmatching/` pending their own acceptance reviews.
The [boot gate](BOOT-TRIAL-GATE.md) pins the isolated compiler identities,
rebuilds all 635 objects from authored C and checks that full link in one run;
its machine-readable report accepts 605 boot-only placements (74,760 bytes) after
passing, each from its own owned compiled range.
The original trial compiler missed the `FOG_PreUpdate__Fv` loop by one extra
`nop`. An [isolated RAC5 short-loop threshold trial](COMPILER-LOOP-TRIAL.md)
now reproduces that function while leaving all earlier object hashes unchanged.
An independent string-length loop matches at two boot addresses, and a
third short loop with an unfilled branch delay now matches in
`GLOBALWAD_ReleaseTransitionBank__Fv` under the same threshold
([loop evidence](COMPILER-LOOP-TRIAL.md)). Further loop lengths and source
families still need controls before qualifying a general compiler profile.

In particular, RAC2's reconstructed GNU EE-GCC game profile and its Sony SDK
profile have **not** been qualified for Size Matters. The repository now has
a [boot-only compiler profile](../config/boot/compiler_trial.json) accepted
for its 31 reviewed placements and one
[RAC5-only patch](../tools/compiler/patches/0001-rac5-short-loop-six.patch).
Its per-source compiler and options live in
[`config/cflags.txt`](../config/cflags.txt); it is not a game-wide C/C++
qualification and publishes no game-wide percentage.

## Three compilers in the boot

The boot is not one build. Classifying the prologue of every named `.text`
symbol separates three register-save conventions, and each one has been tied to
a compiler by byte-exact C bodies at their own retail addresses:

| Convention | Prologue signature | Compiler measured |
|---|---|---|
| 8-byte saves, ascending | `sd` at `0x0`, `0x8`, `0x10`… | the pinned chain: GNU EE-GCC 2.9-ee-991111b plus RAC2's stack |
| 16-byte `sd`, `$ra` at the top | `sd $s0,0x0` … `sd $ra,0x10/0x20` | unmodified Sony 2.9-ee-991111 (the SDK compiler) |
| 16-byte `sq`, `$ra` at `0x0` | `sd $ra,0x0` then `sq $s0,0x10`… | SN ProDG 3.01 `ee-gcc2953`, GCC 2.95.3 |

Classifying the 1,316 named `.text` symbols by the save instruction gives 436
that save nothing, 274 whose one save does not discriminate, **292 that save in
8-byte slots, 196 whose save is a 16-byte `sd` and 111 whose save is an `sq`**,
plus 7 that use `sw`. Reading the whole frame rather than the one instruction
gives the two families the routes act on: **107 bodies in the P layout (16-byte
`sq` saves with `$ra` at `0x0`) and 195 in the Q layout (16-byte `sd` saves with
`$ra` on top)**, the remainder having neither. The layouts also occupy different
address ranges: Q covers libgcc/libstdc++/exception handling, libcd,
fileio/intr/DMA/sceSif and the timers, P covers ps2fio, ps2dlib, cdvdfs and
newlib libc.

Two mechanical details matter, because they are not the same cause:

- The **stride** comes from `compute_frame_size`, which adds
  `GET_MODE_SIZE (mips_reg_mode[0])` per saved register. The pinned chain
  neutralises the R5900 rule (`if (0 && TARGET_MIPS5900) mips_reg_mode[0] =
  TImode;`), so it allocates 8-byte slots. Re-enabling the anchor widens the
  stride but does **not** reproduce P: `save_restore_insns` then emits the store
  with `mips_reg_mode[regno]`, and the pinned chain also keeps RAC2's ascending
  emission order while the retail P bodies **descend**. The anchor alone scores
  98/108 on `sceCdSyncS`; the retail layouts need a different compiler, not a
  flag.
- The detector for the 16-byte families is "two or more `$sp` saves at offsets
  that are multiples of `0x10`". Looking only for `sq`/`lq` misses about half of
  them, because Q saves with `sd` at the same widened stride.

The layouts coexist in one binary, so no global switch can serve both: a single
compiler choice would break the bodies that already match. The selector belongs
per file.

## A second pinned compiler (route P)

`config/cflags.txt` now names a compiler per line. A bare token in front of the
flags selects one of the compilers the boot profile pins besides its own chain;
without a token the line still extends the pinned chain's global `-quiet -O2
-G0`, exactly as before, so no pre-existing object moved. With a token the flags
are that compiler's complete flag set for that file.

Its first selector is `prodg2953`: SN Systems' *ProDG for PlayStation 2*
3.01 `ee-gcc2953.exe`, which reports GCC 2.95.3. It is an external binary that
the profile pins by SHA-256 together with the `cpp.exe` and `cc1.exe` it
consults (`config/boot/compiler_trial.json`, `extra_compilers`); it is never
rebuilt here and never committed. The gate runs it through WSL's Windows
interop from an ignored staging directory, then assembles its output with the
same pinned GNU `as` as every other object, so the campaign still has one
assembler and one object format.

Five P bodies are routed and each fills its owned interval exactly:

| Source | Placement | Body | Pinned chain (negative control) |
|---|---|---|---|
| `ps2fioClose.c` | `0x01EBBCB0` | 124/124 | 114/124 |
| `ps2fioDopen.c` | `0x01EBBD30` | 160/160 | 140/160 |
| `ps2fioGetStat.c` | `0x01EBBEB0` | 92/92 | 78/92 |
| `ps2fioFormatDevice.c` | `0x01EBC180` | 76/76 | 66/76 |
| `cdvdfsFreeFS.c` | `0x01EC2218` | 76/76 | 68/76 |

Controls for this route:

1. **Non-regression.** Recompiling the campaign with the route enabled leaves
   all **406** pre-existing objects byte-identical to their pins; only the five
   new objects are added (411 total). Neither the profile's global flags nor any
   existing `config/cflags.txt` line changed.
2. **Negative control.** The same five sources without their selector score
   66-140 of their bytes and `tools/compile_boot_trial.py` refuses the resulting
   object against its pin — measured, not assumed.
3. **The 8-byte family stays where it is.** `MC_SetError__Fi` (92 bytes) and
   `MCPTRANS_FALL_Shutdown__Fv` (72 bytes) are byte-exact under the pinned chain
   and **not** under `ee-gcc2953` (80/92 and 62/72). Routing a body to the wrong
   compiler is therefore visible, and the selector is not a free upgrade.
4. **Requalification.** The complete gate still passes at the final commit:
   498,900/498,900 boot `.text` bytes, 809,872/809,872 raw `PT_LOAD` bytes,
   856,120/856,120 ELF bytes, and
   `tools/boot_promotion_audit.py --write --check --require-gate` is
   `current_exact`. The 31 accepted placements and 1,064 counted bytes are
   unchanged; the five new owners are provisional witnesses pending their own
   semantic review.

The related body `sceCdSyncS` is **not** route P: it is byte-exact under the
unmodified Sony 2.9-ee SDK compiler (108/108) and reaches 100/108 under
`ee-gcc2953`. That is the Q family, pinned separately below.

## The Sony SDK compiler (route Q)

The Q layout — 16-byte save slots with `$ra` on top — is the second 16-byte
family, and its compiler is the one the SDK shipped: Sony's EE-GCC
`2.9-ee-991111-01`. `config/cflags.txt` selects it per line with the
`sdk991111` token, and `config/boot/compiler_trial.json` pins it in
`extra_compilers` beside `prodg2953`.

This pin differs from route P in one mechanical way that the profile states
explicitly, because it decides where the source can live: `"host": "linux"`.
The driver is an ELF executable, and its 2.9 front end cannot stat a DrvFs path
at all — run from a Windows-visible directory it fails with `cpp: x.c: Value
too large for defined data type` — so the routed sources are staged under
`--output-dir`, which the [gate contract](BOOT-TRIAL-GATE.md) already requires
to be a native Linux directory. `prodg2953` states `"host": "windows"` and
keeps its Windows-visible staging directory. Everything else is shared: the
same `driver -B<dir>/ -E` and `driver -S <flags>` shape, the same
`config/cflags.txt` selector, the same `c_source_policy.py` check on the
preprocessed input, and the same pinned GNU `as` for the emitted assembly, so
the campaign still has one assembler and one object format. A pin that names
neither host, or a host the tool does not implement, is rejected instead of
guessed.

The binary is never rebuilt here and never committed. It is RAC2's
`tools/compiler/sdk/build.py` output: the public snapshot
`github.com/SSXModding/ps2-ee-toolchain` at `b595ded`, built for a 32-bit
Linux host, with a `bison-grammar` patch that only lets the old grammar parse
under a modern bison and changes no generated code. The driver reports
`2.9-ee-991111`, and the profile pins `xgcc`, `cpp` and `cc1` by SHA-256.

Five Q bodies are routed and each fills its owned interval exactly:

| Source | Placement | Body | Pinned chain (negative control) |
|---|---|---|---|
| `sceCdSyncS.c` | `0x01EC2B18` | 108/108 | 96/108 |
| `read.c` | `0x01ED12A8` | 116/116 | 94/116 |
| `write.c` | `0x01ED1228` | 124/124 | 102/124 |
| `sceSifSetDma.c` | `0x01ED3580` | 136/136 | 118/136 |
| `dpadd.c` | `0x01E96400` | 88/88 | 73/88 |

They fall in the ranges the layout table predicts rather than in one library:
libcd (`sceCdSyncS`), the SN libc tty layer (`read`, `write`), libsif
(`sceSifSetDma`) and libgcc's soft-float entry `dpadd` — the last one is the
body earlier batches had parked as unreachable from C because the pinned chain
allocates 8-byte slots.

**A second batch adds 17 more** (3,088 bytes), measured by sweeping every
authored body through the pinned Sony compiler and the pinned GNU `as` rather
than by assuming which families the layout predicts: libgcc's soft-float
conversions and comparisons (`dpdiv`, `dpmul`, `dpcmp`, `dpsub`, `fptodp`,
`fptoui`, `litodp`, `dptoli`, `dptoul`, `dptofp`), its 64-bit helpers
(`__floatdidf`, `__fixunsdfdi`), the runtime entry points `__main` and `exit`,
`ps2fioLseekPartition`, `sceSifSendCmd` and `sceDmaPutEnv`. Each is byte-exact
under `sdk991111 -O2 -G0`, and each scores a fraction of itself under the
pinned chain. Two more measured exact, `isceSifSendCmd` and
`sceSifRemoveCmdHandler`, are **not** integrated: a following function entry
sits inside their interval's alignment padding, and the campaign only claims
bytes a compiled body owns.

**A third batch adds the 15 C++ RTTI accessors** (1,184 bytes). They looked
unintegrable at first: `ld.exe` reports their missing externals as
`bad_exception type_info node` and `type_info type_info function`, names that
appear in no symbol table. Those names are **only a diagnostic** — the SN linker
embeds a GCC 2.9 C++ demangler and prints `__ti13bad_exception` demangled. The
objects reference ordinary `__ti*`/`__tf*`/`STR_name` names, and the fix is the
one the model already gives: define those data symbols absolutely. Two rules
follow from it. Never define absolutely a name a compiled body owns: the symbol
becomes `OBJECT ABS`, the catalogue no longer sees a linked function at that
span, and the body silently leaves the count. And `ld.exe` **exits zero without
writing an ELF** when references are unresolved, so a caller must check the
output file rather than the return code. `__tf9type_info` also needed one C
correction: a `(const char *)` literal for its name argument produced `ori`
where retail has `addiu`, and declaring that name as `extern const char
STR_name[]` — a real object — fixes it.

Controls for this route:

1. **Non-regression.** The complete gate was run twice from unambiguous trees:
   once on the branch's base commit (`research/boot-promotion-30-20261016`, the
   campaign before route Q) and once with the route enabled. All **411**
   pre-existing objects come back byte-identical; exactly five objects are
   added (416 total). Neither the profile's global flags nor any pre-existing
   `config/cflags.txt` line changed.
2. **Negative control.** The same five sources without their selector score
   73-118 of their bytes under the pinned chain (table above), and
   `tools/compile_boot_trial.py` refuses the resulting object against its pin —
   measured by removing the five lines, not assumed.
3. **The 8-byte family stays where it is.** `MC_SetError__Fi` (92 bytes) and
   `MCPTRANS_FALL_Shutdown__Fv` (72 bytes) are byte-exact under the pinned chain
   and **not** under the Sony compiler (72/92 and 59/72). The two 8-byte bodies
   that pass under both (`MCPI_PowerLock__Fv`, `MEMCARD_Test__Fi`) are 8-byte
   leaves that save no register, so they do not discriminate and prove nothing
   either way.
4. **Requalification.** The complete gate passes at the final commit:
   498,900/498,900 boot `.text` bytes, 809,872/809,872 raw `PT_LOAD` bytes,
   856,120/856,120 ELF bytes, and
   `tools/boot_promotion_audit.py --write --check --require-gate` is
   `current_exact` (361 bodies / 416 owners). The 31 accepted placements and
   1,064 counted bytes are unchanged; the five new owners are provisional
   witnesses pending their own semantic review.

Two campaign entries had to give way to the routed objects rather than
duplicate them: the `sceSifSetDma` constant in the linker-symbol table, and
`sceSifCheckStatRpc` — which already had its own C body — would have shadowed
the definitions this route introduces, and neither may stay.

## What route Q does not establish

- **Scope, not identity.** The five bodies prove that *these* sources reproduce
  under the Sony SDK compiler. They do not prove that all 195 prologue-Q
  symbols came from it, nor which flags, headers or library versions the
  original build used.
- **How much of Q is reachable is only sampled.** Of the 195 prologue-Q
  symbols, 46 already have an authored body in the private kit; compiling those
  46 under this pin with `-O2 -G0` makes **25 byte-exact** and leaves 21 short
  (41/88 for `__do_global_dtors`, 26/120 for `__floatdidf`, 123/148 for
  `__cp_push_exception`). None of the 46 is exact under the pinned chain, so
  the family split holds, but the other 149 symbols are simply unwritten: the
  reachable share is unknown, and the family is not uniform.
- **The anchor is not the same compiler.** Re-enabling the RAC2 16-byte-save
  anchor scores 98/108 on `sceCdSyncS` and reaches 16 of those 46 bodies, but
  never the routed five. A widened stride is not the SDK compiler, and the
  routed bodies are the reason to prefer the real one.
- **The 292 8-byte bodies are still the pinned chain's**, and route P's five
  owners are unaffected.
- **No game-wide figure follows.** These five bodies are provisional witnesses;
  acceptance remains a separate per-body review, and the boot-only scope
  ([BOOT-MATCH-SCOPE.md](BOOT-MATCH-SCOPE.md)) is unchanged.

## What route P does not establish

- **Scope, not identity.** The five bodies prove that *these* sources reproduce
  under SN ProDG 3.01. They do not prove that every one of the 107 prologue-P
  symbols came from that compiler, nor which flags, headers or library versions
  the original build used. The count of P bodies that are actually reachable is
  unknown: 107 is the number of symbols with that prologue, and reachability
  additionally depends on the source being recoverable.
- **Route Q is a separate pin, not a consequence.** The 195 prologue-Q symbols
  need the Sony SDK compiler, which is pinned [below](#the-sony-sdk-compiler-route-q)
  with its own five bodies; nothing measured for P carries over to Q. Both
  routes are provisional, and neither widens the other's scope.
- **The 292 8-byte bodies are still the pinned chain's**, and nothing here
  changes their standing.
- **No game-wide figure follows.** These five bodies are provisional witnesses;
  acceptance remains a separate per-body review, and the boot-only scope
  ([BOOT-MATCH-SCOPE.md](BOOT-MATCH-SCOPE.md)) is unchanged.

To expand this beyond the accepted boot placements, establish complete
assembly and loaded-program baselines for the applicable level modules, then
compare candidate C++/C toolchains and flags with independent functions and
controls across code families. A local slice alone does not qualify a match.
The [boot-only scope](BOOT-MATCH-SCOPE.md) records the accepted criterion,
exact packaged ELF and remaining source/runtime limitations.
For level modules, a separate
[narrow shared-function trial](LEVEL-C-WITNESS.md) now verifies two functions
(8 and 72 bytes) at all twenty numbered placements, with a 24-byte level 01 and
12-byte frontend control. It does not qualify a general level compiler.

## The assembler is a per-source choice too (route R)

The boot is not one assembler build any more than it is one compiler build. Two
measured bodies show it, and the two fixes are mutually exclusive: the assembler
that repairs one is hundreds of objects worse than the pinned `as` on the other.

1. **64-bit `dli` expansion.** `__pack_d` (304 bytes, 0x01E95FF0) reaches
   **249/304** under the pinned `as`, and the cc1 output is byte-right either
   way. At byte 243 one `fraction |= QUIET_NAN` becomes a single
   `dli 0x800FFFFFFFFFFFFF`, which the pinned `as` expands with `ori` first
   (`ori; dsll; ori; dsll; ori; dsll; ori`, seven instructions) where retail
   starts with `lui` and uses six. Under the Sony EE SDK assembler the same
   source is **300/300** and fills its interval exactly.
2. **`li.d` of a `double`.** `sceKernelGetSystemTimeLow__Fv` (160 bytes,
   0x01E86280) reaches **155/156** under the pinned `as`: the two double
   constants of the C are `li.d` pseudo-ops, which the pinned `as` loads from
   the `.rdata` pool in two instructions where retail inlines four. Under SN
   Ps2EeAs 2.0 — the pinned SN ProDG 2.0 assembler the boot assembly base
   already uses — the same source is **160/160**.

[`config/cflags.txt`](../config/cflags.txt) therefore carries `as=NAME` beside
the compiler selector, and the profile pins the two extras in
`extra_assemblers`. An entry may be nothing but an assembler selector:
`nonmatching/boot/__pack_d.c  as=ee-sdk` says the pinned chain keeps its global
flags and another `as` runs. Naming a compiler without a flag is still refused,
because an empty complete flag set is not a measurement.

**Why the selector is per source and not global.** A sweep of all 682 authored
boot bodies through five assemblers found exactly these two whose equality with
retail improves anywhere, and each needs the other binary. Comparing `.text`
sections object by object, the SDK assembler moves 19 of 679 away from the
pinned chain's bytes and Ps2EeAs 2.0 moves 313 of 675 — neither can replace the
pinned `as` for the campaign. The pinned chain itself is untouched: a source
with no entry is assembled exactly as before, all 416 pre-existing objects
compile to their pinned SHA-256, and at that bench the requalified gate linked
484 of them into the same complete 856,120-byte boot.

**A second bench on the same selector.** Three libm bodies (`__kernel_cosf`
344/344, `__kernel_sinf` 260/260, `scalbnf` 352/352) are exact only when the
Sony compiler's output is assembled by the **SDK** `as` run with `-G0`. Without
that flag the assembler lowers a `li.s` float to a GP-relative `lit4` entry and
the link stops on `R_MIPS_LITERAL lit4`; with it the pseudo-op is inlined as the
pinned `as` does. The flag belongs to the assembler pin
(`extra_assemblers.ee-sdk.flags["as"]`), because it says how that binary was
measured rather than what one file needs, so `config/cflags.txt` still carries
one selector per line and no per-file assembler flags. The same three objects
also emit an unreferenced `.rodata` — the fdlibm constants, which `-G0`
materialises inside the body instead — and the campaign's linker script now
discards the compiled objects' `.rodata` so the retail data keeps its
addresses. The pinned chain reproduces 281/344, 187/260 and 346/352 of those
three bodies, which is this bench's negative control.

**A fourth batch needs a patched compiler, and the patch is measured.** The
retail R5900 code pads every single-precision division with two nops and never
schedules one into a delay slot: over 208 COP1 sites (158 `div.s`, 50 `sqrt.s`)
136 carry exactly two nops, all 22 sites in the SDK libm window do, and **none**
is scheduled into a delay slot. The assembler is not the cause — the
`ee-as-sdk` family produces none of the 208 — so the padding comes from the
compiler. `tools/compiler/patches/0002-rac5-div-pad.patch` carries four
anchored edits to the Sony source (the `dslot` attribute makes DIV/SQRT/RSQRT
ineligible, and a `mips.c` hook inserts the two nops before `shorten_branches`),
and the profile pins that build as `sdk-divpad`: the same driver and
preprocessor as `sdk991111`, a different `cc1` (SHA-256 `11b995bd…`) and the
patch's own digest beside it, so a pin whose patch moved no longer describes the
binary it names. With that profile `__ieee754_expf` is **692 of its 696 bytes
exact** and `__ieee754_fmodf` **592/592**; without it the same sources score
560/676 and 175/576. Non-regression: the 406 units of that build compile
byte-identically with and without the patch (no unit contains a division), and
the complete gate is unchanged. The `sqrt.s` half of the rule is a family
extrapolation — no `sqrt.s` exists inside the SDK libm window — and three
near-misses stay out of reach: `logf` 801/804 (what remains is the `.rodata`
pool), `tanf` (missing frame) and `atan2f` (body eight bytes short).

**Controls.** Removing the two selectors makes `tools/cflags.py` refuse both
lines ("no cc1 flags"), and the per-body harness prints 249/304 and 155/156 for
the same sources under the pinned chain, which is what the trial compiler
refuses against their pinned object hashes. `__pack_d` needed one more change:
its name was also an absolute `LINKER_SYMBOLS` entry, added while `__make_dp`
called it, and an absolute definition leaves the linked image without a function
symbol at that address — the state the 55 uncatalogued spans are in. The line is
gone, since the compiled body now defines the symbol; the catalogue gained both
bodies (361 → 363 witness bodies) and the uncatalogued count stayed at 55.

## The backlog, and the end of the uncatalogued spans

Twenty bodies that earlier waves had verified were never integrated — the kit
carried them, the campaign did not. They are now: `MCP_Update`,
`MCP_UpdateCameraCB`, `APP_Init`, three `SYSTEM_*` matrix queries, three
`scee::Psp2Gu` setters, four `VEC_*` helpers, `__pack_f`, `__fpcmp_parts_d`,
`copysign`, `_lo0bits`, `dlib_displist_base_offset_mscal`, `fiprintf` and
`sprintf`. With them the catalogue grew from 400 to **475 bodies** and from
23,004 to **29,044 compiled bytes** — the difference is larger than the twenty
bodies because the same pass made 55 previously invisible spans visible.

That is the same cause the earlier profile recorded as open work: an absolute
`LINKER_SYMBOLS` entry for a name a compiled body already defines turns that
symbol into `OBJECT ABS`, and the catalogue no longer sees a linked function at
the span. The fifty-nine such entries are gone, the objects' own definitions
take over at the same addresses, **no span is uncatalogued**, and the complete
ELF is unchanged byte for byte.

The rule is now mechanical in the private kit: a resolver reads the exported
objects' undefined symbols directly — never `ld.exe`'s demangled diagnostics,
which are what made the RTTI batch look impossible — refuses to define a name a
compiled body owns, and removes an absolute line a body has taken over.

## Changing a compiler profile

A compiler-source patch is not an instruction patch or an artificial C shape.
Before changing tools or flags, identify the first mismatch and explain why it
comes from code generation rather than the recovered ABI, types or source. Record
the public source, patch provenance and intended affected family in the PR.

Keep the previous qualified tool/profile and its known-good C/object controls
privately. Compare old and proposed outputs across the affected families, then
recompile every affected accepted body and rerun its complete-program gates at
all placements. Confirm that altered tools, flags, source pins and missing C
substitutions are rejected. Only then update the reviewed profile and generated
status. Do not pick a compiler or a one-function option merely because it raises
a similarity score, and never change emitted instructions to pass a gate.

An option measured for one source file belongs in
[`config/cflags.txt`](../config/cflags.txt), not in the campaign profile: the
profile keeps its global flags, so the objects of every other file stay pinned
and need no requalification. The same file carries the per-line compiler
selector, for the same reason — a family that needs another compiler must not
force one on the files that already match. `tools/cflags.py` owns the file and
rejects an entry that is duplicated, names a missing or non-campaign path,
names a selector the profile does not pin, or carries neither a flag nor an
assembler selector; `compile_boot_trial.py` and `level_c_gate.py` apply the
entries they compile, and the complete-gate receipt is bound to the file's
bytes. A preview compiler is added by pinning its binaries in the profile's
`extra_compilers`, with the host its driver runs on — never by rebuilding or
patching an existing one, and never by widening the pinned chain's global flags
to cover a family. An assembler is a pin of the same kind, in
`extra_assemblers`, selected per source by `as=NAME`; the pin names one binary
and its SHA-256, and the gate takes its directory with `--as-bin`.

Qualification is scoped evidence, not a claim that this is the original compiler
or that all future functions are supported. A failed control invalidates the
proposed expansion; retain the previous accepted profile until repaired.

## Measured limits of the pinned trial profile (2026-10-16)

A second authoring pass over boot bodies of **65-160 bytes** (282 targets, after
setting aside 22 handwritten-assembly ranges that no C source can supply) added
110 more byte-exact intermediate bodies, and measured the following limits. Each
one is a first-mismatch classification with a source-level cause, so a future
profile change can be justified and controlled under the rules above. None of
them is a claim that the pinned profile is wrong for the bodies it already
reproduces.

1. **R5900 register-save stride.** The pinned `cc1` carries the RAC2
   neutralisation `if (0 && TARGET_MIPS5900) mips_reg_mode[0] = TImode;`
   (`config/mips/mips.c`). `GET_MODE_SIZE(mips_reg_mode[0])` is the save-slot
   stride used by `compute_frame_size` and `save_restore_insns`, so the pinned
   chain allocates 8-byte callee-saved slots where retail uses 16-byte ones
   (`sq $s0,0x10($sp)` or `sd $s1,0x10($sp)` beside `sd $ra,0x0($sp)`). The
   detector is "two or more `$sp` saves at offsets that are multiples of 0x10";
   looking only for `sq`/`lq` misses about half of them. **155 of the 304
   candidates in the 65-160 byte range** are affected. Both conventions coexist
   in the retail binary: `0x01E94128-0x01E95CB0` (dlib, sound) uses 8-byte
   slots and matches the pinned chain, while `0x01ECF000-0x01EDA400` (Sony
   prebuilt, libgcc, C++ runtime) and the EH tables use 16-byte slots. A body
   like `sceCdSyncS` reproduces **96 of 108 bytes with an instruction-identical
   body**, differing only in the prologue and epilogue. Any change here needs a
   per-file opt-in, because a global switch would break the bodies that already
   match with 8-byte slots.
2. **Zero-division guard — measured lever, option retained per source.** The
   pinned chain always emits `beql $rt,$zero ; break 0,7` before `div`
   (`mips.md`, `divmodsi4`, under `!TARGET_NO_CHECK_ZERO_DIV`); a sweep of the
   retail `.text` finds **104 of its 111** `div`/`divu` sites unguarded, so the
   retail build used `-mno-check-zero-division`. The option is carried by one
   `config/cflags.txt` entry for
   `nonmatching/boot/dlib_displist_direct_end__FP13DLIB_DISPLIST.c`, the first
   dividing retail body recovered under it. Controls: compiling every campaign
   object with and without the flag gives identical SHA-256 (no pre-existing
   object contains a division, so no pinned object moves), and that body is
   128 bytes and byte-exact only **with** the flag — without it the chain emits
   the 2-instruction guard, the object is 136 bytes, and the trial compiler
   refuses it against its pinned object hash. The requalified gate links every
   campaign object and body into the same complete 856,120-byte boot.
3. **Sibling calls.** Retail emits a direct `j` for some tail calls; the pinned
   chain never does (the reconstructed sources comment the option out as
   "retail behaviour" for RAC2). A measured set of bodies ends in an
   unguarded `j`.

Non-profile limits, recorded for triage rather than for a profile change: bodies
whose jump table lives in `.rodata` cannot be checked by the per-body harness
(which places `.text` only); `va_start` from a wrapper needs the two lines of
`gcc/ginclude/stdarg.h` copied in, and the pinned `cpp` has no include path; the
retail lays forwarded stack arguments at 8-byte intervals where the pinned chain
uses 4; and one unlabelled fragment of neighbouring code may follow a compiled
body inside its next-label interval, which the catalogue records as an
uncatalogued span rather than as part of the body.

A span whose compiled object exposes no function symbol in the linked image is
reported by the catalogue tool and left uncatalogued rather than counted. No span
is in that state today, and the complete-program gate still
checks their bytes. They are the owners whose name is also listed in
`LINKER_SYMBOLS` (`tools/boot_c_campaign.py`): the linker script definition is
absolute, so the linked image keeps the address but exposes no function symbol
for the catalogue. `dlib_displist_direct_end__FP13DLIB_DISPLIST` reproduced that
state and lost it again when its `LINKER_SYMBOLS` line was dropped, which is
what placed it in the catalogue. Relabelling the remaining 55, and confirming
that none of them is referenced only through that absolute definition, is open
work.

## A third GNU chain: the pre-patch trial cc1

The three Gu rotation helpers (`RotateX`/`RotateY`/`RotateZ`, 260 bytes each)
reproduce the retail **descending** callee-saved save order (`sd $s1,0x0` then
`sd $ra,0x8` for the smallest frame) only with a second locally built GNU 2.9
`cc1` (sha256 `37704f48…`) — the same reconstructed EE source tree as the
pinned chain, built **without** the RAC2 "increasing callee-saved saves" patch
that the pinned `cc1` (sha256 `9614ecee…`) carries. The measured cause is that
patch: with it, `save_restore_insns` walks the save registers upward, and the
prologues of these three functions come out in the opposite order (252/260,
252/260 and 252/260 bytes equal). `RotateY` additionally needs its three
product terms written `m0*s + m8*c`, which fixes the temporary order.

The profile is a per-source selector (`gnu-ee-trial`, pinned in
`config/boot/compiler_trial.json` with the driver, `cpp` and `cc1` SHA-256
values) and only the three files carry a `config/cflags.txt` line. The stage
directory is private (`/root/gnu-ee-trial-profile`), built by copying the
pinned chain's `cpp`, the trial `cc1`, and the Sony SDK `xgcc` driver whose
`-B` path selects them; it is not committed. Controls: the three sources fail
under the pinned chain, the gate recompiles every other object with its
declared chain and reproduces all other object pins unchanged, and the
complete ELF is byte-exact.
