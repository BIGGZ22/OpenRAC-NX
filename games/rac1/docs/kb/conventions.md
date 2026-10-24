# Code conventions

How the C is written and why. Most oddities below exist only because they
decide which instructions GCC 2.95 emits for the byte-exact build
([README](README.md#the-one-rule)). Layout: [architecture.md](architecture.md).

## 1. Symbol names

### Address names are permanent

| Form | What | Example |
|---|---|---|
| `func_<ADDR>` | executable function (core or game) | `func_001EC7C8` |
| `func_LNN_<ADDR>` | level function, address in its canonical (lowest) level `NN` | `func_L00_002180F8` |
| `D_<ADDR>` | executable or resident data (below 0x15F000, or the executable's own symbols) | `D_0013F450` |
| `D_LNN_<ADDR>` | level data in the replaced `main` range (0x15F000 and up), canonical level `NN` | `D_L17_00162108` |

**The symbol never changes**: the build and tools read address and level from
it ([OVERLAYS.md](../OVERLAYS.md) "Names"). Declarations, `INCLUDE_ASM`
lines, `__asm__` labels and `_r` definitions always spell it; definitions
may spell the readable macro (below), which tools map back through
`tools/srcnames.py`.

### Readable names are macros on top

| File | Role |
|---|---|
| `config/names.tsv` | the table. Columns: `symbol`, `name`, `tier`, `source`, `evidence`, `alternatives` |
| `include/names.h` | **generated** by `python3 tools/names.py header`; `#define UpdateAllCameras func_001EC7C8`. Never edit by hand |
| `config/names_local.tsv` | names this project gives from reading the code (~2900 functions), with evidence; `override` fixes a wrong imported name |
| `tools/names.py` | `build`, `local` (merge names_local.tsv), `header`, `apply-defs` (definitions use names), `apply` (bodies use names), `check`, lookup (`python3 tools/names.py UpdateAllCameras`) |

Tiers ([NAMES.md](../NAMES.md#tiers)): `recovered` (original identifier, e.g.
from mangled NTSC symbols) and `descriptive` (later name with evidence) go
into `names.h`; `candidate` (weaker) appears only in worker dossiers. Sources
are RaC1-only (the NTSC decomp, ReRAC, the PAL memory map), mapped to PAL by
function size alignment or fingerprints. Core/SDK names (`memcpy`,
`sceGsSync`) stay out of the header so they don't shadow library spellings.

Define and call with the readable name (`void UpdateAllCameras(int x) {`,
`UpdateAllCameras(0);`); declare with the address name
(`extern void func_001EC7C8(int);`). To add names: a row in
`names_local.tsv`, then `names.py local`, `header`, `apply-defs`, `apply`. `common.h` includes `names.h`, so both
give the same object. Names are hints and can be wrong (see
[known inconsistencies](README.md#known-inconsistencies)).

## 2. Declarations that steer codegen

### Per-file aliases (legacy)

Level files grew one landing at a time. When a later landing needs another C
type for a symbol, it declares a **suffixed alias** of the same linker symbol:

```c
/* src/overlays/shared/help_00203E98.c */
extern struct Hero D_0013F450_05278 __asm__("D_0013F450");
extern f32 func_00214358_05278(Vec4f *, s32, f32) __asm__("func_00214358");
```

The suffix is the landing function's tail (`func_L00_00205278`); only the C
type changes. Also seen:

- `_r` definitions: a function defined under another C name with an
  `__asm__` label (`func_L00_00250478_r(...) __asm__("func_L00_00250478")`),
  when the file's existing prototype has another type
  ([LEVERS.md](../LEVERS.md) "Rules"). Tools read `func_X_r` as `func_X`.
- Function-pointer macros (`#define d_update_visible ((void(*)(void))(func_00213C78))`
  in `src/overlays/shared/update_0029B6A0.c`).

An alias names one existing symbol, never an expression
(`"D_0013E633 + 0xF1D"`) or a number ([QUEUE.md](../QUEUE.md) "Rules");
hardware registers are `*(volatile int *)0x10000000`.

**New or reworked C adds no `__asm__`** (no aliases, `_r` definitions,
`SDATA()` or asm statements; exceptions in [section 5](#5-pure-c-no-permuter)).
Use the file's own declarations, value casts, real symbol names and struct
copies. Existing aliases stay until tidied with identical bytes. Where
[LEVERS.md](../LEVERS.md) or [QUEUE.md](../QUEUE.md) still allow aliases, this
stricter rule wins.

### How a global is addressed: `$gp` vs `lui`

Retail reaches some globals via `$gp` (small data, base 0x166D00), others via
`lui`/`%lo`. With `-G2`, `include/common.h` macros pick the form per declaration:

| Macro | Expands to | Effect |
|---|---|---|
| `NOT_SDA` | `__attribute__((section(".data")))` | keeps a small global **out** of `$gp` addressing: `lui`/`lo` |
| `MACRO_ADDR` | `__attribute__((section(".sdata")))` | emits the unsplit macro `lw $2,D`, i.e. retail's one-register `lui $2` / `lw $2,%lo(D)($2)`. Loads only: a store can land in a delay slot ("macro used after .set nomacro") |
| `SDATA(sym)` | `__asm__(#sym "__gp") MACRO_ADDR` | a scalar of a level file's own small data with its real type, reached through `$gp` (`tools/check_macro_slots.py` equates the `sym__gp` label). Legacy |

Older idiom: `extern short D_x;` read as `*(float *)&D_x` or `*(int *)&D_x`
(2 bytes makes it small data at `-G2`; [AGENT_WORKFLOW.md](../AGENT_WORKFLOW.md)).
A port reads these as plain globals of the cast-to type. One `MACRO_ADDR`
alias changes a symbol's form for the whole file, so every landing re-checks
the whole file.

### Assembly stubs

Macros from `include/include_asm.h`:

| Macro | Meaning |
|---|---|
| `INCLUDE_ASM("asm/overlays", func_L00_00205158);` | not yet decompiled: assembles the retail `.s` in place |
| `ASM_FUNC(folder, name)` | original hand-written or VU0 assembly (`config/handwritten_asm.txt`); counts as finished |
| `LINKER_REMNANT(folder, name)` | dead-strip leftovers (`config/linker_remnants.txt`) |
| `INCLUDE_RODATA` | retail rodata included verbatim |

Empty under `M2CTX`/`PERMUTER`. `.s` files come from `tools/setup_asm.sh` and
are never committed ([ASM_CLASSIFICATION.md](../ASM_CLASSIFICATION.md)). A
port supplies its own code for these (`nonmatching/functional/` has port-only C).

## 3. Shared headers

Use these, not private struct copies (fields: [data-structures.md](data-structures.md)):

| Header | Contents |
|---|---|
| `include/common.h` | `s8`..`u64`, `f32`, `f64`; `NOT_SDA`, `MACRO_ADDR`, `SDATA`; `qcopy`, `qcopy_nc`, `qzero`, `CpuSync`, `CpuSyncP`, `CpuEnableInt`, `CpuDisableInt`; includes `names.h` |
| `include/moby.h` | `struct Moby` (0x100 bytes; `update` 0x74, `pvars` 0x78, `oclass` 0xA6) and `struct MobyClass` |
| `include/hero.h` | `struct Hero` (`D_0013F450`) and its records (`HeroMotion`, `HeroState`, `HeroItemSlot`, `HeroManipulator`, `HeroHealth`, `HeroEase`, `HeroRandTimer`, `HeroVelocityKey`, `MobyTrail`) |
| `include/pad.h` | `struct PadState` (0x350 bytes, `D_0013CA40`) |
| `include/vector.h` | `Vec4f`, `Vec4A` (16-byte aligned), `Vec4` (union moved with one `lq`/`sq`), `VecQuad` (128-bit integer) |
| `include/records.h` | `DispatchRec` (0x14-byte function-pointer entry), `PadSlots`, `AnimSeq` |
| `include/callees.h` | prototypes of engine functions that three or more files declared the same way |
| `include/structs.h`, `include/moby_pvars.h` | older shared structs; `moby_pvars.h` (class pvars adapted from Wrench) is not included by any source file today |

`moby.h`, `hero.h`, `pad.h` and `vector.h` are adapted from Lombyte's headers
(MIT; see `THIRD_PARTY_NOTICES.md`). `unkNNN` / `pad_NNN` fields are unknown,
not unused.

### Cast the value, not the field's address

Write `x = (s32)h->motion.unk10;` / `h->field = (T)(v);`, not
`*(T *)&D_0013F450.pad_x`. An address cast folds the offset into the symbol (`%hi(D_0013F450+off)`) and
can change the code; value casts fixed about 60 hero mismatches in the header
pass. A cast into a `pad_` field means the field should be named in the header
([LEVERS.md](../LEVERS.md) lever 15).

### Private views that stay on purpose

| View | File | Why |
|---|---|---|
| `Moby_26ff10` | `src/overlays/shared/partupd_0026A130.c` | not a moby: a particle record (passed to `KillPart`, `func_L00_002688A8`) with its own vector and pvar layouts |
| `Moby_2E8210`, `Moby_2E8AC8` | `src/overlays/shared/vendor_002E1660.c` | camera-side records (`cam` at 0x70 points to a `Cam_2E8210`), not a moby past the header |
| `HeroPos_2E9978` | `src/overlays/l10_orxon/vendor_002E30F8.c` | one field at `D_0013E633 + 0xE9D` (= `gHeroPos`, 0x13F4D0) + 0x200C: retail derives it from a register holding that base, which the `Hero` symbol would fold away |
| `HeroManip4_5FF0` | `src/overlays/shared/help_00203E98.c` | four more manipulators after `manipulators[7]` at hero+0xEF0, plus a random-delay timer; `struct Hero` lays this out differently |
| `Pad_2D9F08` | `src/overlays/shared/vendor_002A5218.c` | reads `held` and `pressed` as one doubleword (`b1C0`); `PadState`'s separate `s32` fields would change the load |

*(inferred from the definitions and uses; the pad reason is from the
header-tidy notes.)* Small `fNN` data views in `help_*.c` (`S141948`, `Cam`,
`Row13E650`, `Rec2C` ...) are also still private.

## 4. Comments and provenance

- Every function gets a one- or two-sentence comment: what it does and, if
  needed, why the C has its shape, e.g.
  `/* Hero state machine: per-frame movement and physics for the current hero state. */`
- Name what a number means where known (`/* 0x1C: frames left */`, class
  names next to oClass numbers).
- **Provenance notes are licence credit** (referenced from
  `THIRD_PARTY_NOTICES.md`): never delete or reword them, even in a tidy
  pass. Ported code ends its comment with e.g.
  `Adapted from Lombyte (GPL-3.0) for PAL: src/textbin/world/startlevel.c, startlevel.`
  or `... for PAL by OpenRAC's tools/port.py: <file>, <name>.`; headers
  (`include/moby.h`, `include/hero.h`) carry the same. Names inside them
  (`FUN_L04_002aee30`) are the source project's US names.
- Old stub comments are often wrong ([LEVERS.md](../LEVERS.md) "Rules").

## 5. Pure C, no permuter

Rules for new C besides section 2's no-`__asm__` rule ([WORKER.md](../WORKER.md),
[QUEUE.md](../QUEUE.md#rules), [AGENT_WORKFLOW.md](../AGENT_WORKFLOW.md),
current policy):

- Plain C. No register pins, artificial barriers (`__asm__("" : ...)`,
  `do { } while (0)`), `volatile` only to pin an order, or reads of
  never-assigned locals; a function that only matches that way is a near miss.
- No inline assembly inside a function, except:
  - the `include/common.h` helpers: `qcopy` / `qcopy_nc` (retail's
    `lq $2` / `sq $2` quadword copy, with and without a memory clobber),
    `qzero` (`sq $0`), `CpuSync`, `CpuSyncP`, `CpuEnableInt`,
    `CpuDisableInt`. Call them; don't copy their asm;
  - since 2026-10-17, functions whose retail code is VU0 (`v*.xyzw`,
    `qmtc2`/`qmfc2`, `lqc2`/`sqc2`, `vcallms`, `ctc2`/`cfc2`) or hand-written
    assembly.
- No decomp-permuter ([PERMUTER.md](../PERMUTER.md) is unused); decompile by
  hand, led by Ghidra and the assembly.
- The build is fixed: no step that edits compiler output, no per-function
  flags. Options are per file, in `config/file_cflags.txt`.
- Never use Sony SDK source, samples or headers, or leaked material
  ([CONTRIBUTING.md](../../CONTRIBUTING.md) "Sources"). Never write a level
  address as a number: use the symbol (`D_L05_001B24D4`).

For the PC port: `qcopy(dst, src)` is a 16-byte copy, `qzero(p)` a 16-byte
clear, and the `Cpu*` helpers are ordering/interrupt instructions with no
effect on game logic; use host equivalents.

## 6. Verifying a change

From the repo root, in the toolchain container:

| Change | Command |
|---|---|
| Level code (`src/overlays/...`) | `bash tools/docker/run.sh python tools/overlay_file_check.py <file> [...]` (no args: files changed since HEAD); exits 1 on a build failure or any non-EXACT function |
| Executable (`src/game`, `src/core`, `src/libgcc`) | `bash tools/docker/run.sh bash tools/build_sn.sh` (full build, retail-address link, audit) |
| One candidate function | `bash tools/docker/run.sh python tools/try_func.py func_X cand.c [--diff]` |
| After pulling | `bash tools/docker/run.sh bash tools/setup_asm.sh` (regenerates `asm/`; stale asm causes mass mismatches) |
| Progress report | `bash tools/docker/run.sh python tools/gen_progress_report.py --no-build`, then `python3 tools/gen_progress_report.py --check` |

Comment- and rename-only edits get the check too: a changed declaration type
or section can change code.

Several agents in one checkout: `tools/claims.py`
([AGENT_WORKFLOW.md](../AGENT_WORKFLOW.md) "Several agents at once").

```sh
python3 tools/claims.py claim <owner> func_L05_002559DC   # before working on it
python3 tools/claims.py release <owner> func_L05_002559DC # giving up
python3 tools/claims.py lock <owner>                      # before writing src/
python3 tools/claims.py unlock <owner>                    # right after
```

## 7. Reading a typical level file

`src/overlays/shared/vendor_002B0068.c`: a generated header ("Shared code
following vendor", i.e. link position); includes (`common.h`,
`include_asm.h`, `hero.h`, `moby.h`, often `pad.h`, `callees.h`); then per
landing its declarations (early ones aliases like `D_0013F450_B0068`, local
types like `ShotVars_B0068`; later ones often plain names) followed by its
function; `INCLUDE_ASM` stubs for the rest. A symbol may appear under two C
names. Functions stay in retail's address order.

`Moby.pvars` (0x78, `u8 *`) is cast to a file-local struct, e.g.
`ShotVars *v = (ShotVars *)m->pvars;` in `func_L04_002B0290`
([mobys.md](mobys.md#memory-the-moby-array-and-pvars)). Other spellings
(`*(T **)&m->pvars`, `m->pvars + 0xC0`) stay as matched.
[PVARS.md](../PVARS.md) has wrong offsets and level numbers; see
[known inconsistencies](README.md#known-inconsistencies).
