# Decompilation progress

> Historical log. `tools/fix_core_spills.py`, `tools/fix_tail_calls.py`, `tools/fix_trunc_slot.py` and `tools/func_cflags.py`, which it describes, were removed on 2026-10-07 for rewriting compiler output; see docs/BUILD_FIDELITY.md for the current build.

The project's knowledge base: the levers that close near-misses, the
toolchain questions (solved and open), the dead ends with their counts,
and a per-function log. The step-by-step procedure lives in
`docs/WORKFLOW.md`; the toolchain itself in `docs/TOOLCHAIN.md`.

Verification, in short (see `docs/WORKFLOW.md` for the full procedure):

```
bash tools/build_sn.sh                                  # from-scratch build + link + audit of every decompiled function
sh tools/diff.sh func_XXXXXXXX                          # iterate on one function with asm-differ
python tools/check_match.py symbol func_XXXXXXXX 0xSIZE # one function against a freshly linked build-sn/rac1.elf
```

`check_match.py symbol` is position-independent: it reads retail's
address straight out of the `func_XXXXXXXX`/`D_XXXXXXXX` name and does
not assume our layout matches retail's, so it stays correct even when an
earlier function has drifted in size. See the tool's docstring for its
other two modes (whole-section, retail-only).

## Status

**The "known systemic artifact" is FIXED and gone.** For most of this
project's history there was a warning here that `func_00112380` compiled
8 bytes short of retail, shifting every later `core_text` function and
forcing several entries to be qualified "matches (mod. known drift)".
That is resolved: `func_00112380` is byte-exact (0/40) via the
sign-extension technique, `core_text` drift is verifiably zero, and the
five previously-qualified entries (`func_001144F0`, `func_00116408`,
`func_00119798`, `func_00119840`, `func_0011D078`) are provably exact
with no qualifier needed.

**Verification: two classes of false positive, both now caught.** Every
"match" in this file should be re-verifiable by `tools/sweep_matches.py`,
which audits *all* decompiled functions on **size and bytes** after a
full rebuild. It exists because spot-checking let two false-positive
classes through, each caught only by luck:

1. **Stale object.** A failed compile leaves the previous `.o` in place,
   whose `INCLUDE_ASM` stub still contains retail's own bytes — so the
   check reads a fictional `0/N`. Triggers seen: a conflicting `extern`
   is a *hard error* here, and `$?` after piping the compiler through
   `tail` reports **tail's** status. Always confirm the compiler's own
   exit code is 0.
2. **Correct-but-longer.** `check_match.py` compared exactly
   `retail_size` bytes, so a function right in its first N bytes but
   *longer* than retail also read `0/N` — the surplus was never looked
   at. This produced at least one recorded-then-retracted "match"
   (`func_0011AE1C`) and two stale near-miss records
   (`func_001F9B90`/`func_001F9B98`, +4 bytes each). `check_match.py`
   now fails loudly on any size disagreement, using the symbol's
   `st_size`.

**Where the numbers live.** `progress/report.json` (published on
[ByteMatch](https://bytematch.dev/OpenRAC/rac1-decomp)) is the source of
truth for what has source and what is finished. `tools/gen_progress_report.py`
regenerates it from a from-scratch build, and CI fails when it is out of
date with `src/` or the original-assembly manifests. The report counts
byte-verified handwritten assembly and dead-strip remnants as finished;
`tools/sweep_matches.py` still reports exact C matches separately. See
`docs/ASM_CLASSIFICATION.md` for the tracked list and setup procedure.
It leaves out retail's linker fill (the `0xCDCDCDCD`
runs between objects that splat also emits as 4-byte "functions"; 38 of
them, 200 bytes): fill is not code, and the build reproduces it byte for
byte.

The report also measures the executable's **data** (`total_data`,
`matched_data`, `complete_data`, in the top-level measures and the
`executable` category). The total is retail's initialised data:
`core.data`, `core.rdata`, `core.lit`, `.lit`, `.data` and the three
`lvl.*vtbl` sections (`.bss` has no bytes to compare, and the level
programs' data is not measured yet). Data still assembled from retail's
dumps in `asm/data/` is not matched. Matched data is what compiled objects
bring themselves (`config/core_rodata.txt`, and the jump tables of
decompiled game functions, `tools/jump_tables.py`), counted when the linked
image holds retail's bytes at its retail address; complete data is the part
of it in complete units. Units carry no data measures, because the retail
dumps can't be split by unit.

Totals written in prose go stale, so this file no
longer keeps a running count. Snapshot as of 2026-09-24 (`5c1d28b`):
**926 functions have source; 903 are exact on size and bytes; 23 are
same-size near-misses kept as C; 0 are size-mismatched; 26.69% of code
bytes match; 28 of 121 units complete.** Re-run `bash tools/build_sn.sh`
(which runs the sweep, the layout check and the whole-image check)
after any change rather than trusting a snapshot or any single entry.

Two tools carry most of the weight when closing a near-miss:

- `tools/diff_words.py` prints every still-differing word of every
  decompiled function as retail-vs-ours, decoded. The useful unit is
  "which instruction, and how" rather than "N bytes differ": several
  functions whose residuals decode to the same instruction pair are
  usually one fix, not several investigations.
- `tools/permute.py` answers "same instructions, wrong order" by
  compiling every ordering of the marked source lines at once and saying
  which ones reproduce retail. Reasoning about the scheduler loses to
  enumerating against it.

### Levers that close near-misses

Collected as they were found; each one closed at least one function.

**Swapped `addu` operands — type the table, don't rewrite the addition.**
Retail `addu $2,$2,$3` (base, index) against our `addu $2,$3,$2` was the
entire residual of five functions. Reordering the C addition never helps:
GCC canonicalises the PLUS before operand order is chosen. What does help
is the form of the access. Three variants of the same move:

    extern char T[]; p = T + i * STRIDE;   ->  index first
    extern Rec  T[]; T[i].field            ->  base first

    void **b = table + idx; *b;            ->  index first
    table[idx]                             ->  base first

The third is worth noting because `table` was already correctly typed:
parking the address in its own local and dereferencing it is enough to
flip the order. But the lever is not "base first always" — it is that the
indexing form selects the order, and which one retail wants depends on
which operand it materialises last. `func_0011AA38` wants index-first,
because it computes its shift in a branch delay slot before the base is
loaded, and the typed form takes it from 6/44 to 32/44.

**`slti reg,reg,0` vs `srl reg,reg,31` — it's how the boolean is
consumed.** Both extract a sign bit. Returning the comparison gives the
cheap `srl`; feeding it to a branch gives the `slti`:

    return (a*b - c*d) < 0;                       ->  srl  $v0,$v0,31
    t = a*b - c*d; if (t < 0) return 1; return 0;  ->  slti $v0,$v0,0

**A misplaced pointer increment — name the next pointer.** When a loop
near-miss is one instruction of pointer advance in the wrong slot,
neither `p++` at the body end nor `p++` in the for-increment will move
it. Writing the schedule out does:

    next = mask + 1;      /* where retail schedules it */
    ... body reads *mask ...
    mask = next;

**A call argument's declared type picks how its constant is built.** An
argument declared `long` is materialised by the compiler as ONE `dli`
macro, so the assembler expands it and can still fill the `jal` delay
slot from the instruction before it. The same argument declared `int` is
materialised as two RTL insns (`lui` + `ori`), and the scheduler drops
the OTHER argument's `li` between them, which changes what lands in the
delay slot. That one declaration was the entire residual of
`func_001F7B70` (the `0x513F1` call). The same lever supplies real
64-bit constants: `0x8000000044` only exists as one argument if the
parameter is `long`.

**A global pointer written through directly is reloaded; a local copy is
not.** Retail's packet writers (`func_00234B48`, `func_001F55C0`,
`func_00234BA0`, `func_00228458`) reload `D_00161000` before every field
store, because a store of the pointed-to type may alias the pointer
variable itself. That only happens if the two types can alias: declaring
the global `int *` and storing `int`s reproduces it exactly, while
declaring it as a pointer to a quadword struct lets the compiler keep it
in a register and the function comes out short. The opposite shape
(`Qword *p = D_00161000++;` then stores through `p`) is what retail's
`func_00235290` has, so the same symbol is declared twice, under two
types, through an `__asm__` alias.

**Two fields of one table: type the table so the base survives.** With
`extern char T[]` and byte offsets, the compiler folds the offset into
the `%hi` of the first access and addresses the second backwards from
it. With `extern int T[]` and indices, it keeps one base register and
uses two displacements, which is retail's form (`func_00204BE8`).

**Taking the address of a small-data symbol.** `&D` on a
`short`-declared (SDA) symbol compiles to the `la` macro and so to
`addiu $2,$28,-off` — retail's form when it indexes a small-data array
with a variable (`func_00228458`).

**Three more rules the sweep enforces, all learned the hard way:**

**Size mismatches must ALWAYS be reverted, never kept as
"documented-close".** A byte mismatch is inert — it is wrong in place. A
size mismatch shifts every later function in the object, so it damages
*other* functions' results. Demonstrated twice: reverting one
size-mismatched function made a different, untouched function become
exact. So: byte-mismatch *may* be kept as documented-close;
size-mismatch is always reverted.

**Post-`endlabel` padding: one cause that presents as ~179 problems.**
729 of the `.s` files carry padding *after* their `endlabel`, because
retail aligns the next function to 16 bytes while splat only emits
`.align 3`. While a function is an `INCLUDE_ASM` stub its `.s` supplies
that padding; decompiling it to C silently drops it, shifting every
later function and producing spurious 1-byte `jal` diffs *far from the
real cause*. One such function shifted 179 others at once. The fix is an
explicit directive after the function:

```c
__asm__(".align 4");
```

Nuance, so nobody chases ghosts: the common 4-byte case is normally
harmless, because the next function's own `.align 3` re-establishes
8-byte alignment anyway (~30 decompiled functions carry exactly 4 bytes
and are all exact with zero drift). It only bites when retail wanted
16-byte alignment — more than `.align 3` can account for.
`sweep_matches.py` therefore reports only the >4-byte cases.

**Varargs: neither callers nor definitions are blocked.** The skip
category further down overstates this. Functions that *call* a varargs
function match fine — `func_0023BF48` and `func_0023E450` both call the
varargs `func_001E9730` and are exact. This paragraph used to add that
only *defining* one was blocked, for lack of `stdarg.h`. That was wrong
too: `func_001E9730` itself is exact as `void func_001E9730(int arg0,
...) {}` in `src/game/stub.c`. The ellipsis alone makes the compiler
emit retail's full argument-shadow spill (`$5`-`$11` and
`$f12`/`$f14`/`$f16`/`$f18`), no body needed. Only a definition that
actually *reads* its variable arguments (`va_arg`) would need a
`stdarg.h` shim, and none has been attempted yet.

### New this round (2026-09-23)

Found while decompiling in parallel with `tools/try_func.py`. Each one
closed at least one function, named in brackets; the function's own
comment has the details.

**Read the callee, not the caller.** Several "unexplained" instructions
were arguments. `li $5,1` and a "dead" `&D_0015FFD0` were the arguments
of the empty profiling markers `func_001F2560`/`func_001F2558`, which
take `(void *, int)` [DrawMobysSetup]. A register oddly copied at entry
(`daddu $6,$4,$0`) was a later printf's third argument: read the format
string [CreateMoby]. A callee retail calls without setting `$a0` takes
nothing [DrawMobysCleanUp]. A `long` parameter changes argument
evaluation order: `func_00234C98(int, long)` [DrawMobysSetup]. Even an
unused int return moves the next temporary from `$v0` to `$v1`, so
retail's `$2`/`$3` choice tells a callee's return type [termAll].

**One variable, two registers: how a copy survives CSE.**
`n = h->count++;` keeps retail's copy of the old count [Stash_SendData].
`t = g; p = (int *)t; t += 0x10; g = t;` keeps a copy of a pointer that
`p = g; g += 0x10;` folds away [func_0020DEB0]. Advancing a local in
place (`p += 0x10; g = p;`) ties old and new to one register, where a
plain `g += 0x10` lets the store fall into a jal slot [DrawShrubs].
Updating a parameter in place keeps it in its argument register
[func_00215A98].

**Globals read at every use.** Re-reading a MACRO_ADDR global where the
source uses it, instead of caching it in a local, lets CSE reproduce
retail's load order and copies [func_00213BB8]. One `char *` local per
block that reads a global gives retail's "%hi kept, %lo rebuilt"
pattern [pause.c handlers].

**volatile.** reorg never moves a volatile access into a delay slot, and
volatile stores keep their order against the epilogue
[startDisplay, func_001219C8, VU1_sendChain].

**Return shapes.** `if (x < 0) return 0; return 1;` gives slti/xori
where `return x >= 0` gives nor/srl [audioDecCreate]. Two
`return x;` statements get cross-jumped into one; assign in each arm and
return once to keep both copies [func_00124A70]. A float compare
returned as `? 1 : 0` gives bc1t with the `li` in its slot, and folded
into `&&` it gives bc1tl [func_00207E28]. `int r = 1; if (a && !(x <= y))
r = 0; return r;` gives retail's bc1fl with `r = 0` in its likely slot
[func_00208208]. A flag shifted as `(x != 0) << k` folds into a branch;
bind the comparison to a local first [func_00222640, func_00209BB8].

**Statement order still decides allocation.** In the music play family
the first store must not need a constant in a register, or `$a0` goes
to the constant and the handle cannot load straight into it. That was
the "allocator dead end" recorded for years [func_00216290 family].

**Structure of the memory.** Reaching a table as a member of a struct
fixes addu operand order where pointer arithmetic cannot
[SndToc in music.c]. For a local pointer the order is the reverse of the
global-table rule: `items[i].ptr` is index-first and `rec = &items[i];
rec->ptr` base-first [func_00229D48].

**The toolchain, not the source.** Many residuals recorded as compiler
behaviour were retail's assembler (ps2eeas): short-loop padding, the nop
between an FP compare and its bc1, the nop after an mtc1 whose register
is read next, and its own dli sequences. `tools/ps2eeas_nops.py` and
`tools/ps2eeas_dli.py` now reproduce all four. Some near-misses had
been the right size only because two errors cancelled [func_00208208,
func_00215A98]. And `src/core/001236F0.c` is Sony's memory card library,
built with the SDK's 2.9-ee like libgcc [sceMcSync].

**Later the same day (batches G to J).**

- *Return types show in the next register.* Retail's choice of `$v0` or
  `$v1` for the first temporary after a call shows whether the callee
  returns a value. Four near-misses were a callee declared void that
  returns int, or the reverse [func_00217588, func_00209DC0,
  func_0011AA00, func_0011D3C8]. Declare the right type through an
  `__asm__` alias rather than changing the file's declaration, whose
  other callers were matched against it.
- *An argument register left alone up to a call is being passed on*
  [func_00226720 passes oClass to CreateMoby].
- *A select kept as a branch.* jump.c makes movz/movn only when the
  arm sets a full register. A `static inline short` helper returns
  through a subreg, so its select stays a branch [func_0021EDD8].
- *Reading a global back right after storing it* changes the store
  schedule (CSE folds the load) where no order of the stores did
  [func_002348E8].
- *MACRO_ADDR through an alias* fixed four residuals that old notes
  blamed on the allocator. Look for a split `lui`/`lw` that retail does
  in one register [func_001EBAF0, func_001EC098, func_00222D70,
  func_00205A50].
- *A MACRO_ADDR store at `D + 4` never goes in a delay slot*, because the
  compiler counts symbol+offset as two instructions. Declare a symbol
  at the exact address instead; undefined `D_` names resolve at link.
- *`m = G = call();`* stores the call's `$v0` directly, where
  `m = call(); G = m;` stores the saved copy.
- *`if (x) return 1;` in both arms with one shared `return 0;`* keeps a
  flag test a beqz; a result variable or a `return 0` per arm becomes
  sltu [func_002072C0].
- *Scaled indices in their own locals* (`int off = idx * 8;`) give
  base-first addu; written inline, the multiply goes first
  [func_00203548].
- *Old notes are evidence, not verdicts.* Batch H found four wrong
  decodes, and batch J a "tried, fails" store order that works.

**Batches K to N (same day).** `-dS -dR -dg -dl` and `-da` RTL dumps work
in the container (`-fsched-verbose-4` is spelled with `-N`), and several
of these came out of them.

- *The scheduler's tie-break is predictable.* It issues two insns a cycle,
  at most one memory access. Between equal priorities the insn with more
  dependents wins, then the insn that frees a register, then source
  order. So the source's last store, which also frees the base, goes
  first; stores that are the last use of their value follow in source
  order; and a value stored twice has its first store emitted last
  [InitMemSlots func_00201E10].
- *A callee's return type reorders the caller.* A call that returns a
  value clears the list of pending `$v0` readers; a void call leaves it,
  so every later call depends on them. `int` fixed func_0022EA20, a
  `void` alias func_0012CBA0.
- *A struct member cannot alias a scalar global.* A load through a
  struct member is "in struct", and gcc's fixed_scalar_and_varying_struct_p
  lets it move above fixed-address scalar stores; `*(T *)(p + off)` and
  an `int *` index wait behind them [SetupSkyGifPaging, DrawTies_2].
- *volatile on one path.* A volatile store on a fall-through path stops
  reorg's try_merge_delay_insns, leaving a duplicated delay-slot copy.
  Keep the read volatile and make the store plain where retail merges
  [DMAC_VIF1_Enable]. Make only the fields retail re-reads volatile
  [voBufIncCount].
- *A fresh pseudo per read.* A `static inline` accessor for a global gives
  each use its own pseudo: a temp in the duplicated exit test, a saved
  copy in the loop [func_0022EF68]. `i * SIZE + (int)ptr` gives the
  offset-first addu that pointer arithmetic does not.
- *Reading a value twice* (test it, then assign it) keeps a copy that
  changes reorg's fills [func_0020D9D8, func_0020D790].
- *reload_cse, not CSE,* merged three byte loads into `andi`s of a
  compare register. A base-first add that lands the table base in that
  register kills the value [func_0020D6D0].
- *One basic block* (calls do not end blocks): local-alloc ranks pseudos
  over the first scheduling pass's order, so statement order moves
  registers. Brute force works: 122 store orders compiled in one run,
  one exact [SetTfragDists].
- *A pointer assigned in the loop condition* (`while (d = D, d[4])`)
  gives the duplicated exit test its own register and retail's
  preheader copy [func_00217748].
- *`short` vs `unsigned short`:* `unsigned short &= ~8` becomes andi
  0xFFF7, `short` an AND with -9 in a register [DrawTies_2].
- *GCSE's hash buckets* decide PRE's register order: the bucket count is
  (insns at GCSE / 2) | 1, so adding or removing insns can swap two
  registers [func_0012AAC8].
- *2.9-ee* (core SDK objects): it tail-calls void functions that end in
  a call, never `return f(...)`; it has -fstrict-aliasing on by default;
  and verbatim library source often matches under it (newlib's rand,
  __sinit, _fwalk). See the 2.9-ee section below.

**2026-09-24: batches with cheaper workers (691 to 903).** Matching ran as
batches of 10-15 functions per agent, grouped by file, with
`docs/LEVERS.md` as the one-page brief and `tools/integrate.py` to check
and apply each batch's manifest. Per function matched, Sonnet workers
cost roughly 60-100K tokens on library code with a reference source,
rising past 250K once that ran dry, and 370K+ on game code (0 of 15 on
pause.c). Opus agents cost 37-79K per function on game code, closing
11-14 of 14 per batch, largely by re-deriving Sonnet's near-misses from
the assembly. What was new:

- *Reference sources match library code nearly line for line:* the MSSG
  mpeg2decode sources for libmpeg's `mpc.o` (motion vectors, headers,
  slices) and newlib's 2000-02-17 snapshot for `mprec.c`, `dtoa.c`,
  `makebuf.c`, `strtol.c`.
- *An empty asm hides a value from the optimizer.* `__asm__("" :
  "+r"(x));` stops gcc proving a small trip count and reversing a loop
  [scePad2Init], and keeps a constant address in a register where
  retail's store is not volatile [kputchar]. `do { ... } while (0)`
  around one store is a scheduling barrier [cdvd_exit]. A register pin
  (`register int i asm("$14")`) closed func_002284E8, but was not kept.
- *Signedness shows in the compare:* `sltiu` means unsigned, `slti`
  signed [pictureData0, updateTempTackData].
- *More return types:* memset returns a pointer, EIntr and DeleteSema
  return int, func_001F4868 returns a 64-bit `long`, func_0020D830 a
  float.
- *A word inside a data block, read via `$gp`:* write it as the block's
  label plus an offset (`(char *)&D_0015EDBC + 0xC`). A made-up
  `D_0015EDC8` has no definition [snd_InitVAGStreamingEx].
- *Never declare a global that the file also reaches through `lui` with a
  small type:* the assembler then makes every access to it in the file
  `$gp`-relative [draw.c's D_00161000].
- *A DMA wait:* `if (x) do {} while (x);` [vibuf.c's StopDMA]. Its old
  "short-loop erratum" note was wrong.
- *Old notes and Sonnet candidates had real decode errors:* wrong case
  labels, swapped results, an 8-byte copy read as 13, a callee given an
  argument it doesn't take.
- *What try_func can't see:* where a constant lives (above), and a
  `%hi` that ld resolves wrongly. In func_001E9808, a second HI16 for the
  same symbol before its LO16 came out as `lui 0xd390` in the image, where
  retail has a bare `lui 0x14`. It stays a stub until that is reproduced.

Build changes this round (docs/TOOLCHAIN.md): compiled core objects place
their literals and jump tables in core_rdata (`config/core_rodata.txt`);
data pieces after a cut are laid out with `.org` at retail offsets, and a
`jtbl_` cut keeps the data splat merged after the table; the bss equates
come from the objects' symbol tables (`tools/list_undefined.py`), since
ld crashes on long runs of undefined references; `__udivdi3` and
friends map to their still-assembly libgcc modules.

### Per-function log

The table below is a **chronological log**, newest entries mostly at the
top. It is not the current status: a row records what was true when it
was written, and many functions have moved since. The current state of
any function (exact, kept near-miss, or stub) is in
`progress/report.json`. Rows whose recorded status contradicted the
report on 2026-09-22 carry a bold **Update 2026-09-22** note at the end;
their original text is kept, because the dead ends it records are still
worth knowing. Newer decodes and reverts are documented in comments above
the stubs in `src/` rather than here.

| Function | Segment | Status | Notes |
|---|---|---|---|
| `func_002399A0` | text | **matches** | Thousands formatter: `arg1 >= 1000 ? func_00116248(arg0, D_00161178, arg1/1000, arg1%1000) : func_00116248(arg0, D_00161180, arg1)`. The `>= 1000` arm is the fall-through, as retail's `slti`/`bnez` polarity says. `func_00116248` declared unprototyped since the two call sites pass different argument counts. Byte-exact, first attempt. |
| `func_002391A8` | text | **matches** | **Reclaimed from a banked revert** (was 14/60). The old note had the logic right and named the fix — "needs the base-pointer-local lever" — and that is exactly what it needed: hoisting `char *b = D_001E66C0;` so one materialized base serves both the argument load at `+0x2C` and the trailing store at `+0x44`. Byte-exact. **A banked revert that names a specific untried lever is close to a free match; worth sweeping the table for others.** |
| `func_001EC210` | text | **reverted (size 92 vs 96)** | Logic certain and recorded above its stub. The missing instruction is retail restoring `$31` in the delay slot of *both* early-exit branches as well as at the end — it duplicates the epilogue reload where GCC branches to a single shared epilogue. Nested-if and mirrored-early-return forms both give 92; the latter reproduces retail's control flow exactly and still shares the epilogue, so it is the delay-slot filler rather than the source shape. |
| `func_0012C420` | core_text | **matches** | Tail call forwarding `(D_00153BD8, arg0)` to the varargs `func_0011A6C8`. Confirms again that varargs *callers* are fine — only definitions need `stdarg.h` — declared unprototyped so the call site isn't type-checked against a signature we don't know. Byte-exact. |
| `func_00218928` | text | **reverted (2/8)** | Right size and shape, and the rewriter produced the bare `j` correctly — but the `.s` carries spimdisasm's "Handwritten function" marker and its delay slot is the **trapping `addi $5,$0,0`** where GCC emits the non-trapping `addiu`. Instruction selection, so no C reaches it. A reminder to check the handwritten marker before attempting a tail call: shape filters alone don't catch these. |
| `func_00113AC8` | core_text | **matches** | **Reclaimed from a stale revert** — the old comment said retail was a bare tail jump this compiler could not produce, true when written and obsolete since `tools/fix_tail_calls.py`. `func_00114438(arg0, func_00113968)`, passing the second function's address. Compiles byte-exact from exactly the source that comment recorded. |
| `func_0012C268` | core_text | **matches** | Clears the field at `+0x848` then tail-calls `func_00127378(1)`; the constant argument setup is the jump's delay slot, so the store precedes it. Byte-exact, first attempt. |
| `func_0012CC30`, `func_0012CC40`, `func_0012CC50` | core_text | **matches** | Three more of the `func_0012C468` forwarder family (globals `D_00153C48`/`D_00153C78`/`D_00153C90`), same shape as the already-matching `func_0012CC60`. All byte-exact first attempt — family-grep again, found by the shared callee rather than by the ranking. |
| `func_0011BC70` | core_text | **reverted (size 16 vs 12)** | Documents a real limit of `fix_tail_calls.py` rather than a source problem — see the comment above its stub. Retail schedules the argument load into the jump's delay slot; GCC spends the call's delay slot on the epilogue reload instead, so the rewriter's case (a) emits nothing and the assembler fills the slot with a `nop`. Fixing it would require the rewriter to *move* an instruction into the delay slot, which it never does — every transformation it performs is a deletion, and that is what makes it safe. **Update 2026-09-22: now exact.** `tools/fix_tail_calls.py` now sinks the last body instruction into the tail jump's delay slot, the move this entry said it would need (see "SN's assembler fills delay slots only from AFTER the branch"). |
| `func_00216960` | text | **matches** | Guarded state transition on the `D_001517D0` global: if the pointer at `+0x50` is non-null and the state at `+0x5A` is 3, call `func_0012EDE0(p)`, set the state to 4 and return 1; otherwise 0. Byte-exact, first attempt. A `char *d` base local is right here (retail keeps the base in `$16`), the opposite of `func_00216D30` in the same family. |
| `func_002177F0` | text | **matches** | `D_001517D0` family: when `arg0 == 1`, zero the field at `+0x8`, then set it to 2 if `func_0012F030()` is non-zero. The zeroing store sits in the call's delay slot. Byte-exact. **Its forward declaration said `(void)` but retail reads `$4`** — the declaration was only ever used to take the function's address for a `func_0012F068` callback registration, so correcting it to `(int)` changed no codegen and left the registering function `func_00216270` exact. |
| `func_0011FE48` | libgcc | **exact** | `__adddf3` from libgcc's fp-bit.c. The "prologue save order" residual recorded here was never a source problem: this region was built by Sony's gcc 2.9-ee, and GCC's own unmodified source reproduces it byte for byte. See **libgcc is 2.9-ee** below and `src/libgcc/README.md`. |
| `func_0012BD28` | core_text | **matches** | Four-store struct setter (`p[1]=arg2; p[0]=arg1; p[2]=arg1; p[3]=arg1`). Retail emits `0xC,0x4,0x0,0x8`; the **rotation rule predicted the source order exactly** — writing them `0x4,0x0,0x8,0xC` produced retail's emitted order. Byte-exact, first attempt. |
| `func_00214D28` | text | **matches** | Float clamp-and-step: `d = target - *p`, clamp to ±`maxstep`, `*p += d`, return `fabsf(target - *p)`. The `else if (d < -maxstep)` arm shares its assignment with the first clamp via retail's negate-then-fall-through, which plain C reproduces. Calls the already-decompiled `func_001F9B88` (`fabsf`), so no extern needed. Byte-exact, first attempt. |
| `func_00216D30` | text | **reverted (30/84)** | Flag update on the `D_001517D0` global struct — full semantics recorded above its stub in `src/text.c`. **Nested ifs beat the `&&`/`\|\|` form** (38→30) because short-circuit operators let the compiler hoist the `0x3E` load above the first branch. Residual is the allocator: retail keeps `%hi` in `$7` and re-materializes the base inside the branch targets, spending the first delay slot on that copy; this compiler keeps one base in `$6` and uses a branch-likely instead. Direct `D_001517D0[...]` indexing was tried to force re-materialization and is **clearly worse (74%)** — another data point that the indexing lever is per-function. |
| `func_00128C28` | core_text | **matches** | Init sequence: store `func_00128A58(arg0, 5)` into `+0x1B4`, then if `func_00128A58(arg0, 1)` is non-zero run a second `(arg0,1)` call, `func_00128968(arg0, 7)` and `func_00129180(arg0)`. Returns 0 on both paths. Byte-exact, first attempt. |
| `func_00125078` | core_text | **close, not exact** (3/100) | Picks whichever of `p` and `p+0x80` has the smaller `+0x7C` field, indexing a two-slot stack array with the `slt` result directly. Three bytes — one instruction: retail sums into the base register (`addu $2,$2,$4`), this compiler sums into the index register. **Writing the addition the other way round changes nothing (GCC canonicalises it), so this is the allocator's destination choice rather than the documented operand-order lever.** **Update 2026-09-22: now exact.** |
| `func_00114518`, `func_001188C8` | core_text | **matches** | Last two of the `D_0015ED10` errno-wrapper family, forwarding three args to `func_00119108`/`func_00119008`. Both byte-exact, first attempt. **The family is now fully harvested — six functions, all exact**, found by grepping the remaining stubs for `D_0015ED10` rather than waiting for the ranking to surface them one at a time. Worth repeating for other shared globals: once one member of a family matches, the rest are usually near-free. |
| `func_00116108` | core_text | **matches** | Four-argument member of the `func_00112468` errno-wrapper family, forwarding three args to `func_00119088`. Byte-exact, first attempt. |
| `func_00120978` | core_text | **matches** | Swaps a global handler pointer: bail returning 0 if `func_00120F30(1)` is non-zero, otherwise capture the old `D_00159840`, install `arg0`, and call `func_0011D9A8` only when `func_0011D960()` was non-zero. The old value is captured and the new one stored unconditionally — the store sits in the branch's delay slot. Byte-exact, first attempt. |
| `func_0011B710` | core_text | **close, not exact** (8/92) | One-shot init guarded on `D_0012FDA0 == -1` — see the comment above it in `src/core_text.c`. Residual is only which of three stack-descriptor stores lands in the first call's delay slot. Notable as a **rotation-rule counter-example**: source order `(5,1,2)` rotated exactly as documented while `(1,2,5)` did not, so the rule is still per-function even after its promotion. |
| `func_00112468` | core_text | **matches** | **Reclaimed from a stale revert.** Its comment said it was blocked because this compiler spilled `$s0`/`$s1` as `sq`/`lq` where retail used `sd`/`ld` — true when written, obsolete since `tools/fix_core_spills.py`. The logic recorded in that comment was right all along and now compiles byte-exact unchanged. A reminder that reverts recorded against a since-solved blocker are worth re-running wholesale. |
| `func_00114000` | core_text | **matches** | Same errno-style wrapper as `func_00112468`, forwarding two args to `func_001191C8`. Byte-exact. |
| `func_001161E8` | core_text | **matches** | Same family again, calling `func_00119110`. **New signal:** retail materialised the sentinel with `lui`/`ori` (0xFFFFFFFF) rather than `addiu $2,$0,-1`, which made it one instruction — and so 4 bytes — longer than an `int` comparison produces. That is the tell for an **unsigned** comparison: `unsigned r; if (r == 0xFFFFFFFF)`. Byte-exact once the type was changed. |
| `func_00124010` | core_text | **matches** | Copies three words out of an uncached-mirror view of `arg0` (`arg0 \| 0x20000000`) into whatever `D_00159B28`/`D_00159B2C`/`D_00159B30` point at, each guarded by a null check on the destination pointer. Byte-exact, first attempt. |
| `func_001245F8` | core_text | **matches** | Nine-argument call to `func_0011B4C8` (eight in `$4`-`$11`, the ninth at `0($sp)`), then returns `D_0015B180`. Confirms the EABI eight-register argument convention plus stack spill for the ninth. Byte-exact, first attempt. |
| `func_00125020` | core_text | **close, not exact** (28/84) | Marks entry `arg0` of the `0x330`-stride table `D_0015B640` — see the comment above it in `src/core_text.c`. Size and addressing form are each reachable but not together: one `char *e` local coalesces to a single register and is 4 bytes short; recomputing the address per store gives the right size but folds `+4` into the address constant instead of a store displacement; two pointer locals coalesce back to one. |
| `func_00120430` | libgcc | **exact** | `__cmpdf2` from libgcc's fp-bit.c, built by gcc 2.9-ee. Same resolution as `func_0011FE48`. |
| `func_0012C058` | core_text | **close, not exact** (7/68) | `if (*(int*)(*(int*)(arg0+0x40) + 0x174) != 3) func_0012C0A0(arg0); else func_0012BF40(arg0);`. Structure is instruction-for-instruction identical including both delay slots and the shared epilogue; only the register assignment differs (retail saves `arg0` in `$7` and holds the constant `3` in `$3`, this compiler uses `$5` and `$2`, and the two loads follow). Allocator destination-reuse question. Hoisting the inner load into its own local changed nothing. Same size, so kept. |
| `func_00119718` | core_text | **reverted (30/68)** | Four-field forwarder to `func_00118E90` — full semantics recorded above its stub in `src/core_text.c`. Retail computes the `buf[3]` tag entirely before storing anything and spends the call's delay slot on that store; this compiler interleaves the tag arithmetic with the stores. Natural order, retail's emitted order, and hoisting the tag into a leading local all give 30-32/68. |
| `func_0011D9C0` | core_text | **matches** | Builds two 32-byte stack structs (`int a[8]`, `int b[8]`), sets field 1 and 2 of each to 1, and passes each to `func_00118C70`, storing the results into `D_00130420`/`D_00130424`. **A clean confirmation of the rotation rule:** with the four stores written in retail's *emitted* order the result was 4/72 with the stores rotated; writing them in plain source order (`a[1], a[2], b[1], b[2]`) produced retail's emitted order exactly. Byte-exact. |
| `func_00116168` | core_text | **close, not exact** (27/72) | Bit classifier on a 64-bit argument, same family as `func_001161B0`: `hi &= 0x7FFFFFFF; hi \|= (unsigned)(lo \| -lo) >> 31; hi = 0x7FF00000 - hi; return 1 - ((unsigned)(hi \| -hi) >> 31);`. Blocked identically to its sibling — every instruction and operand matches, the allocator just assigns the low-word and mask registers the other way round. Same size, so kept. |
| `func_002270B0` | text | **reverted, size mismatch** (72 vs 76) | 5-entry `{key,flags}` table search that clears bit 2 of the match — semantics recorded in full above its stub. Retail splits the address setup into `addiu %lo` plus `addiu +4` where this compiler folds them into one, and hoists the `-5` mask before the loop where this compiler sinks it into the taken branch. A `base` local and a `mask` local were tried to force each; the compiler folds and sinks regardless. **Update 2026-09-22: now exact,** written in its twin `func_00227068`'s shape (see "Rewrite a stubbed near-miss in its exact sibling's shape first"). |
| `func_0011B1F8` | core_text | **matches** | Nested linked-list search: walk the outer list from `arg1+0x28` (linked by `+0x14`), and for each node walk the inner list from `+0x8` (linked by `+0x38`), returning the first inner node whose first word equals `arg0`, else 0. Retail is dense with branch-likely (`beql`/`bnel`) instructions, which made it look risky; plain nested `while` loops reproduce all of them exactly. Byte-exact, first attempt. |
| `func_0023E560` | text | **close, not exact** (12/76) | Initialises five header fields then zeroes one word per element across `arg3` elements at a `0x138C0` stride, reloading the base pointer each iteration. Two documented levers moved it (retail's emitted store order as source order, and reversing the `addu` operands to `off + base`); the residual is where the compiler schedules the single `sw $zero` to `+0xC` — retail emits it first, this compiler third, and it does not move with source position. Same size, so kept. |
| `func_00214158` | text | **reverted, size mismatch** (76 vs 80) | Random angle in radians — semantics recorded above its stub. Every instruction matches including both constant materializations; the one missing instruction is a hazard `nop` retail carries between `mtc1 $2,$f0` and the `cvt.s.w` consuming it. Same class as the `lwc1` load-delay nop but from the GPR→FPU transfer side. **`tools/rank_candidates.py` now detects this** — the old rule only looked at `lwc1`, required a tiny leaf and excluded functions with calls, so this 20-instruction function with a `jal` ranked as a candidate. 33 stubs are blocked by the new rule. |
| `func_00228268` | text | **matches** | Three back-to-back `func_001F99B0(buf, 0, len)` clears over `D_001D6860`/`D_001D74C0`/`D_001D6760`. Byte-exact, first attempt. Relies on the existing unprototyped `extern void func_001F99B0();` — see the note on shared callees with incompatible call sites. |
| `func_00226CF8` | text | **matches** | Walks 24 pointer slots from `arg0+0x44`, calling `func_0020E180(*p, 1)` for each non-null one, returns 4. The counter runs `0x17` down to `-1` (`bgez`), so it is a `do/while (i >= 0)` with the decrement before the pointer bump. Byte-exact, first attempt. |
| `func_0012BC78` | core_text | **close, not exact** (12/80) | Table dispatch through `arg0+0x40`, indexed by `*arg1` with 8-byte entries; calls the handler at `+0xC` with the entry's `+0x10` field and returns **its** result. A first attempt returned the entry address instead, which kept `entry` live across the call, forced a callee-saved register and made the function 8 bytes too long — retail's `daddu $7,$2,$0` sits *after* the `jalr`, so `$2` is the callee's return value. Residual is the allocator holding `result` in `$7` vs `$6`. Same size, kept. Removing this function's stale `extern void func_0012BC78(int, void *);` (a wrong-signature guess by an earlier caller) was needed to compile; `func_0012BCC8` still matches. |
| `func_0022EAB0` | text | **matches** | State-machine step over `0x70`-byte entries of `D_0013E650`: on state 7 clear the state byte and two words, on any other non-zero state except 6 set the state to 4. **Third clean rotation-rule case:** the three stores written in natural order came out rotated by one (5/84); moving the first store to last gave retail's order exactly. Byte-exact. |
| `func_001162B8`, `func_001163A0` | core_text | **matches** | Reclaimed from a banked revert. Retail's `dsll32`/`dsra32` pairs are not sign-extensions of an `int` — they are the DImode→SImode narrowing GCC emits when a 64-bit return value is assigned to an `int`. `long` is 64 bits for this target while `word_mode` stays SI, so the callees were prototyped `long` at the call site and defined `int` at the definition — an inconsistency retail could carry because the two sides lived in different translation units. We merge a whole segment into one file, so the two views are reconciled with an asm-labelled alias (`extern long func_00116108_wide(...) __asm__("func_00116108");`), which still emits a plain `jal`. See the new lever below. |
| `func_00116320` | core_text | close, not exact (8/128, same size) | Third member of the same vtable family, same `long` aliasing. Every instruction and operand matches; retail schedules `andi $2,$2,0xEFFF` ahead of the argument load `lw $4,0x54($16)` and this build emits them the other way round — a list-scheduler tie between two independent instructions. The `&=` idiom and a named local for the handle load both leave it unchanged. |
| `func_0011DC50` | core_text | close, not exact (10/104, same size) | Reads a word through `func_00118CF0`, rewrites bits 13..15 to 1, pushes it back, re-reads and restores the original; returns whether the field read back as 0. `srl` rather than `sra` is the tell that the scratch word is `unsigned`. Residual is entirely prologue scheduling — see the gcc 2.9-ee section below. |
| `func_00120910` | core_text | close, not exact (24/104, same size) | Same stack-descriptor idiom as `func_0011BBF0`. `unsigned short arg0` is confirmed: it is what puts `andi $17,$4,0xFFFF` in the prologue instead of at the call site (an `int` parameter masked at the call is 4 bytes worse). The second argument to `func_00118B20` is one `%hi`/`%lo` pair on `func_001208E4 + 4`, i.e. a code address the source names by symbol. Residual is the same prologue-scheduling class. |
| `func_00228400` | text | **reverted, size mismatch** (80 vs 84) | Dispatch on a leading short — semantics recorded above its stub. Retail keeps `arg0` in `$16` and spends the first `jal`'s delay slot on `addiu $16,$16,0x20`, making the pointer advance free; this compiler emits a `nop` there. Three source shapes tried (offset expression at the return, advancing a separate `char *p` after the call, and advancing it before the call — best at 20/84); none put the advance in the call's delay slot. **Update 2026-09-22: now exact** with a single `return p` at the join (see "An empty delay slot can be the symptom of a fold, not the cause"). |
| `func_00236B58` | text | **matches** | Brackets two `func_001F9A98(dst, src, len)` block copies between a pair of `func_00236A98()` calls. Byte-exact, first attempt. |
| `func_0012BBF8` | core_text | **matches** | Clears field `+0x28` of six sub-objects hanging off `arg0->0x40` at offsets `0x1B8/0x1C8/0x1D8/0x1BC/0x1CC/0x1DC`, skipping null ones, returns 1. Retail uses six `bnel` branch-likelies with the store in the delay slot; a plain `if (p != 0) *(int *)(p + 0x28) = 0;` per slot reproduces every one. Byte-exact, first attempt — second case this round confirming branch-likely density is not a warning sign. |
| `func_0012C468` | core_text | **matches** | Guarded dispatch: if `arg0->0x858`, `arg0` and `arg0->0xC` are all non-null, build a 2-field stack struct `{0, arg1}` and hand it to `func_0012BC78`; otherwise call `func_0012C420(arg1)`. Note the `arg0 != 0` test is retail's own and comes *after* it has already dereferenced `arg0` — one of the dead-looking guards that must be written out. Byte-exact, first attempt. |
| `func_0020CCD0` | text | **reverted (66/84)** | Three-global predicate — semantics recorded in full above its stub. Blocked on the `%hi`-register-reuse sub-case, and heavily: retail keeps the `%hi` of `D_0013D6B8` alive in a spare register across the whole function and re-adds `%lo` a second time on the late path, where this compiler materializes the full address once up front. That one choice re-registers most of the body, which is why the residual is large despite the logic being right. |
| `func_00120670` | core_text | **matches** | Marshals three `int` args plus a 64-bit one into an on-stack `int buf[8]` (offsets 0/4/8 and a `sd` at 0x10) and passes its address to `func_0011FA38`. Byte-exact, first attempt. |
| `func_00128560` | core_text | **matches** | Writes `arg1` to hardware register `0x10002000`, then stores `D_00132F70[arg1 >> 28]` into `arg0+0x818`. Needed a **`volatile int *` pointer local** for the register address — see the new lever below. Byte-exact. |
| `func_0012AC50` | core_text | **matches** | Ring-buffer wrap: `v = *(int*)(arg0+8) + (arg1 >> 3);` then `if (v >= *(unsigned*)(arg0+0x24)) v -= *(int*)(arg0+0x28);`. The compare is `sltu`, so `v` must be `unsigned`. Byte-exact, first attempt. |
| `func_0012D4B0` | core_text | **matches** | Binary-to-packed-BCD: `((v / 10) * 6 + v) & 0xFF` on `v = arg0 & 0xFF`. `divu` so `v` is unsigned; retail keeps the div-by-zero trap guard even for the constant divisor 10. Byte-exact, first attempt. |
| `func_00119768` | core_text | **matches** | Marshals `arg0` and a sign-extended `signed char arg1` into an on-stack `int buf[4]`, calls `func_00118E90(3, buf)`. The `sll`/`sra` 24 pair comes from declaring the parameter `signed char`. Byte-exact. |
| `func_001197C0`, `func_001197F8` | core_text | **matches** | Same forwarder shape as `func_00119768` with an extra `unsigned short arg2` field, calling `func_00118E90(-5, buf)` and `(-6, buf)` respectively. Byte-exact, both first attempt. |
| `func_0011B0B0` | core_text | **close, reverted (20/48)** | Wrapping counter — semantics recorded in full above its stub in `src/core_text.c`. Every instruction matches including the div trap guard; the allocator picks the opposite registers for the divisor and `mfhi` result, which reorders the tail. Same open scratch-register question as `func_001160D8`. |
| `func_0011BF48` | core_text | **matches** | Zeroes global `D_0012FD94`, then `func_001153FC(D_001580A8, 0, 4)` (a memset-shaped call), returns 0. Byte-exact, first attempt. |
| `func_0011AA00` | core_text | **close, not exact** (4/52) | `func_001193F8(5); func_00118AD0(5, D_00154F54); D_0012FD04 = 0;`. Every instruction matches; the two `lui`s holding the globals' addresses land in `$2` where retail uses `$3`. Tried a value local and a second local — neither moved it. Same open scratch-register question as `func_001160D8`. Same size, so kept. |
| `func_001239D8` | core_text | **matches** | Passes `arg0..arg2` straight through to `func_001238B0` with a fourth argument of `0x40`; on a zero result sets `D_00132EA8 = 0xB`. Byte-exact, first attempt. |
| `func_0011D210` | core_text | **matches** | `if (func_00118E70(4) & 0x40000) { func_00118EC0(); return 1; } return 0;`. Byte-exact, first attempt. |
| `func_0011D370` | core_text | **reverted, size mismatch** | Word-at-a-time copy — semantics recorded above its stub. 52 bytes vs retail's 56: retail leaves the loop branch's *delay slot* unfilled where this compiler fills it with the pointer bump. Short-loop erratum in a second form; see the classifier note. |
| `func_001161B0` | core_text | **close, not exact** (28/56) | `isnan`-shaped bit classifier on a 64-bit argument: `hi &= 0x7FFFFFFF; hi \|= (unsigned)(lo \| -lo) >> 31; return (unsigned)(0x7FF00000 - hi) >> 31;`. Same 14 instructions in the same order as retail apart from where the `dsra32` for the high word is scheduled; registers for the low word and the mask are swapped ($3/$2 vs retail's $2/$3). Allocator question. |
| `func_00119890`, `func_001198D0` | core_text | **matches** | Ring-buffer advance: bump a counter at `+4` (up / down respectively), advance the pointer at `+0xC` / `+0x8`, and wrap it back to `self+0x10` on reaching the end. Needed `self + (*(int *)self + 0x10)` rather than `self + *(int *)self + 0x10` — see the associativity lever below. Byte-exact. |
| `func_0011ABC8`, `func_0011AC08` | core_text | **close, not exact** (11/60) | Forward six arguments to `func_0011AA90` with `0` / `1` injected as the second, seven arguments in total (EABI passes eight in `$4`-`$11`). Every move and the call match; only the prologue's `addiu $sp` lands second instead of fourth. Scheduling, not expressible in source. Same size, so kept. |
| `func_0011D4A0` | core_text | **matches** | `if (func_00118EA0() == 0x2000000) func_0011D4E0(); else func_00118EB0();`. Byte-exact. Its first reading showed 1/64 purely from drift caused by `func_0011B6B8` being short; reverting that made it exact untouched — a clean demonstration of why size mismatches are never kept. |
| `func_0011B6B8` | core_text | **reverted, size mismatch** | Three-condition predicate — semantics recorded above its stub. 56 bytes vs retail's 60: retail keeps two exit blocks (a shared `return 0` and a separate `return 1`, each with its own `jr`), this compiler merges them into one exit and sets `$v0` in the branch delay slots. Both the `&&` chain and explicit early returns merge. |
| `func_00120778` | core_text | **close, not exact** (8/64) | Spills a float arg, calls `func_001206B0(&f, buf)`, then forwards `buf[0..2]` plus `((unsigned long)buf[3] << 32) >> 2` to `func_00120670`. Instruction-identical to retail except which instruction takes the call's delay slot. Same size, so kept. |
| `func_00112380` | core_text | **close, not exact** | Logic fully understood: `return func_00116F68(arg0, 0, 10);`. Retail has an extra redundant `dsll32`/`dsra32 v0,v0,0` sign-extension pair (8 bytes) before the return this compiler doesn't emit for any variant tried. See "Open toolchain questions" below. This is the root cause of the "known systemic artifact" noted above. **Update 2026-09-22: now exact:** the pair is a `long` return narrowed to `int` (see "SOLVED: the `dsll32`/`dsra32` sign-extension question"). |
| `func_00112464` | core_text | **not a real function** | 4 bytes of `0xCDCDCDCD` — alignment padding between `func_001123A8` and `func_00112468` (rounds the latter to an 8-byte boundary), not code. Left as `INCLUDE_ASM`; nothing to decompile. |
| `func_00112468` | core_text | **close, not exact** | Logic fully understood — see the comment on it in `src/core_text.c` for the full C. Blocked on the `sq`/`lq` vs `sd`/`ld` callee-register-save question below, kept as `INCLUDE_ASM` since the byte diff isn't a small fixed offset like `func_00112380`, it cascades through the whole function. **Update 2026-09-22: now exact** once `tools/fix_core_spills.py` resolved the spill width (see its newer row near the top of this table). |
| `func_001138A8` | core_text | **matches** | `return D_0012F86C;` (returns a global pointer's value). Byte-exact. |
| `func_001144D8` | core_text | **matches** | `return D_00152470;` (returns a rodata blob's address — takes an unused `void *arg0` parameter; retail loads a value into `$a0` at every call site but the function body never reads it). Byte-exact. |
| `func_001144F0` | core_text | **matches** | `return func_001144D8(D_0012F86C);`. One `jal` target byte differs, fully explained by the `func_00112380` drift above — confirmed by disassembling the linked ELF, not just diffing raw bytes. |
| `func_00113A70` | core_text | **matches** | Struct/object initializer: writes 4 function pointers, a self-pointer, several zeroed fields, and 3 caller-supplied values into the struct at `arg0` (offsets given as raw byte offsets via `char *` casts — real field names/struct layout not established yet). Instruction-for-instruction identical to retail (same order, same registers, same offsets) once disassembled from the linked ELF; the raw bytes differ only in 4 relocated addresses, same known-drift cause. |
| `func_001160C8` | core_text | **matches** | `*(int *)((char *)D_0012F86C + 0x58) = arg0;` — setter into a struct reached through global pointer `D_0012F86C`. Byte-exact. |
| `func_001154C0`, `func_001154C8` | core_text | **matches** | Empty functions (`{}`) — auto-filled by splat itself during the initial split, not touched this session, noted here for completeness. |
| `func_00118A34`, `func_00118A50`, `func_00118A60`, and similar | core_text | **not decompile targets** | Marked `/* Handwritten function */` by spimdisasm itself — raw `syscall N` sequences, i.e. these were originally hand-written assembly (PS2 kernel syscall wrappers), not compiler output. Left as `INCLUDE_ASM` permanently; there's no C source to recover. |
| `func_00113AD8` | core_text | **not a standalone function** | Single instruction (`lw $4, 0x0($2)`) with no `jr $31` of its own — a fallthrough continuation, not independently callable. Left as `INCLUDE_ASM`; not clear yet what it's a continuation *of* without more investigation. |
| `func_00116408` | core_text | **matches** | `func_00112468(*(int **)(self + 0x54), *(short *)(self + 0xE));` — calls into the still-`INCLUDE_ASM` `func_00112468` (return value discarded). First attempt wrongly passed `self + 0x54` as the pointer arg; retail actually `lw`s a pointer *stored* at that offset first — caught by the byte diff (`lw` vs `addiu` opcodes, not just an operand), fixed. One `jal`-target byte remains, same known drift. |
| `func_001160D8` | core_text | **close, not exact** | Linear congruential PRNG (multiplier `0x41C64E6D`, increment `12345`, 31-bit mask) on the seed field `func_001160C8` sets — same operations/order/count as retail, but this compiler picks `$v1`/`$a0` for the two independent temporaries where retail picks `$a0`/`$a1`. Tried reordering source statements and separate locals; neither changed the allocation. See "Open toolchain questions". |
| `func_00115578` | core_text | **close, not exact** | Hash-bucket linked-list push (`table[idx]` head insert, idx read from the pushed node, table pointer at `arg0+0x4C`). Same register-allocation-choice issue as `func_001160D8` — identical operation sequence, different scratch-register assignment among `$v0`/`$v1`/`$a0`. **Update 2026-09-22: now exact.** |
| `func_00113AC8` | core_text | **not a small drift, real gap** | Retail is a bare 3-instruction tail jump (`j func_00114438`, no stack frame, no `$ra` save) — true sibcall elimination for a void function whose last statement is a call. This compiler builds a full call frame instead (20 bytes larger) for the equivalent `func_00114438(arg0, func_00113968);` source. Unlike the other near-misses this isn't a small fixed-offset diff, so left as `INCLUDE_ASM` rather than kept as documented-close C. **Update 2026-09-22: now exact** through `tools/fix_tail_calls.py` (see its newer row near the top of this table). |
| `func_00116068` | core_text | **not a standalone function** | Two `addiu $sp,$sp,N` instructions sandwiching `0xCDCDCDCD` padding, no `jr $31` — not real, independently-callable code. Left as `INCLUDE_ASM`. |
| `func_00115EE0` | core_text | **not a standalone function** | Single `addiu $sp,$sp,0x70`, no `jr $31` — fallthrough fragment, same category as `func_00113AD8`. |
| `func_00112464`-style padding (`func_001138A4`, `func_00113A6C`, `func_00113B6C`, `func_00113FFC`, `func_0011405C`, `func_00114514`, `func_001154BC`, `func_00116244`, `func_00116D2C`) | core_text | **not real functions** | All 4 bytes of `0xCDCDCDCD` alignment padding, same as `func_00112464`. Left as `INCLUDE_ASM`; nothing to decompile. Not exhaustively enumerated — there are likely more of these throughout the file; recognize the pattern (single `pref 0x0D, -0x3233($14)` instruction, `nonmatching ..., 0x4`) rather than re-deriving it each time. |
| `func_001151B4` | core_text | **not attempted, needs intrinsics** | Hand-optimized `memcmp`-shaped routine using 128-bit quadword loads and COP2 SIMD tricks (`pxor`/`pcpyud`) for a fast path, byte loop fallback. Not a plausible target for plain scalar C to reproduce exactly — would need compiler intrinsics/vector support this toolchain likely doesn't expose the same way. Skipped rather than force a bad attempt. |
| `func_0011A728` | core_text | **matches** | `arr[idx] = val;` where `idx`/`val` come from `arg0+0x10`/`arg0+0x14` and `arr` from `*(int **)(arg1+0x1C)`. Byte-exact. |
| `func_0011A748` | core_text | **matches** | `int v = *(int*)(arg0+0x10); *(int*)(arg1+0x8) = v; return v;`. Byte-exact. |
| `func_0011B090` | core_text | **matches** | Clears a flag bit (`flags &= ~1`) and zeroes a field. Needed `unsigned int`/`0xFFFFFFFEu` rather than plain `int`/`~1` to match — see "Unsigned-mask materialization" below, a solved instance of a technique worth knowing, not an open question. Byte-exact. |
| `func_0011A758` | core_text | **matches** | `return D_00155080[arg0];` — indexes a global `int` array. Byte-exact. |
| `func_0011AA68` | core_text | **matches** | Picks `D_00154F64` or `D_00154F6C` as a base pointer depending on `arg0`'s sign, adds `arg0<<3`, zeroes the `int` there. Needed two source-shape adjustments to match retail's scheduling/register choices: computing `offset = arg0 << 3` as its own statement *before* the `if` (so the compiler schedules it into the branch's delay slot using the original `$a0`, same as retail, instead of copying `arg0` to a temp first) and reassigning into `arg0` itself for the picked base rather than a separate `int base` local, plus `offset += arg0;` (in-place accumulate) rather than a separate final expression, matching retail's choice to accumulate into the same register it makes the final store from. Byte-exact — the last of these three source-shape changes was the difference between a 2-byte residual and an exact match. |
| `func_00119798`, `func_00119840` | core_text | **matches** | Both are `{ int local = argN; func_00118E90(<tag>, &local); }` — a small typed value gets stashed on the stack and handed to `func_00118E90` by address along with a type tag (`0x4` / `0x10`). One `jal`-target byte each, same known drift. |
| `func_0011D078` | core_text | **matches** | `return func_0011CE70(arg0, arg1, arg2, buf);` with a 16-byte stack scratch buffer (`buf[0x10]`) passed by address as the 4th arg. Getting the frame size (`buf[0x10]`, not `0x20`) and the real 3-argument-plus-passthrough signature (not 2) both took a byte-diff-guided iteration — first attempt used the wrong buffer size (retail's frame was 0x20 total, not 0x30) and wrong arg count (retail puts the buffer pointer in `$a3`, meaning `arg2` is a real third parameter this function forwards, not dead). One `jal`-target byte remains, known drift. |
| `func_00119868` | core_text | **matches** (was previously close-not-exact) | Sets `D_00154A40` to `arg0`, then fields `0x4`/`0x8`/`0xC` of the struct it heads to `0`/`self+0x10`/`self+0x10`, returns `self`. **The earlier entry for this function was wrong** — it claimed the store order was "confirmed source-order-independent, tried all orderings" and filed it under the scratch-register/scheduling open question. It is fully source-order-dependent: the store-order rotation rule gives it directly (retail emits `0x8, 0x4, 0xC`, so source is `0x4, 0xC, 0x8`), byte-exact. The old note said "all four" orderings were tried, which is already fewer than the six permutations of three stores — the rotation-predicted one was evidently among the untried. Cautionary example: "tried everything" claims in this table are worth re-testing against a later technique rather than trusted. |
| `func_001156C0` | core_text | **matches** | Count-leading-zeros-style bit scan (successive `& mask` tests halving the search range, `0x10`/`8`/`4`/`2`/`1` bit contributions, returns `0x20` for an all-zero input's top-bit-clear edge case). Byte-exact on first attempt — no register/type gotchas this time. |
| `func_00115098` | core_text | **close, not exact** | `int *dst = out ? out : &junk; if (arg2) { if (arg3) { *dst = *arg2; return *arg2 != 0; } return -1; } return 0;`. Fixed two real bugs getting here: needed `unsigned char *` for the byte reads (retail uses `lbu`, an initial `char *` attempt gave signed `lb`), and this exact if-nesting to get the `beqz $a2` polarity/target right (an early-return form emitted the opposite branch sense). Remaining diff: retail's `bnel $a3,0` reuses its own delay slot as the *first instruction of the branch target* (the `arg2` byte load) — a scheduling trick not reproduced. New instance of the same open-question category as `func_00119868`/`func_001160D8`/`func_00115578`, this time on a branch's delay slot rather than a straight-line store or register choice. **Update 2026-09-22: since reverted to `INCLUDE_ASM`:** it was 4 bytes short (56 vs 60), which drifted everything after it (see `notes/round-parallel-A.md`). |
| `func_001144CC` | core_text | **not a standalone function** | `pref 0x0D,...` padding marker immediately followed by a bare `addiu $sp,$sp,0x50` with no `jr $31` — same fallthrough-fragment category as `func_00113AD8`/`func_00115EE0`, not `func_00112464`-style pure padding (it does one real thing, just isn't independently callable). Left as `INCLUDE_ASM`. |
| `func_001138B4`, `func_00116F9C` | core_text | **not real functions** | More `0x4`-byte `0xCDCDCDCD` padding, same pattern as `func_00112464` and friends. |
| `func_001144E8` | core_text | **not a standalone function** | Single `addiu $sp,$sp,0x10`, no `jr $31` — fallthrough fragment, same category as `func_00113AD8`. |
| `func_00115248`, `func_001152F8` | core_text | **not attempted, needs intrinsics** | Hand-optimized `memcpy`/`memmove` (the latter handles overlap via direction-aware copy) using the same tiered 128-bit `lq`/`sq` -> 64-bit `ld`/`sd` -> byte-loop structure as `func_001151B4`. Same "not a plausible plain-C target" reasoning; almost certainly SDK-provided runtime routines, not per-file Insomniac code. Skipped. |
| `func_001138B8` | core_text | **skipped, callee-saved regs** | Uses `$16`/`$17`/`$18`/`$19`/`$20` — hits the `sq`/`lq` open question, not attempted. |
| `func_00116FA0` | core_text | **skipped, callee-saved regs** | Uses `$16` — hits the `sq`/`lq` open question, not attempted. **Update 2026-09-22: now exact,** the first `core_text` s-register match once the `sq`/`lq` question was solved. |
| `func_00118EC0` | core_text | **matches** | `D_0012FCF0 = 0;`. Byte-exact. |
| `func_00119100`, `func_00119108` | core_text | **matches** | Both `int f(void) { return -1; }` — trivial constant-return stubs (real logic may live behind a not-yet-decompiled caller; these two themselves genuinely just return -1 unconditionally). Byte-exact. Note: `func_00112468`'s existing comment describes calling `func_00119100(arg1)` with an argument — this disassembly shows it takes none and ignores whatever's in `$a0`; harmless (the described behavior — always returns -1 — still holds), but the parameter in that comment/prototype is not real. |
| `func_001191C0`, `func_0012BB20` | core_text | **matches** | Both `int f(void) { return 1; }` — trivial constant-return stubs, distinct addresses/callers, identical bodies. Byte-exact. |
| `func_0011AE1C` | core_text | **NOT a match — was a false positive, reverted** | Previously recorded here as "byte-exact". It was not. Retail is a bare 4-byte `jr $31` with **nothing in its delay slot**; `void f(void) {}` emits `jr $ra; nop` and gets force-aligned, costing 8 bytes. It passed verification only because `check_match.py` compared exactly `retail_size` bytes, so the surplus fell outside the compared window. Reverted to `INCLUDE_ASM`. See "Two classes of false positive" below — `tools/sweep_matches.py` now catches this class. |
| `func_00114518`, `func_001154D0`, `func_001155A8`, `func_00115808` | core_text | **skipped, callee-saved regs** | Use `$16`/`$17` (and more) — hit the `sq`/`lq` open question, not attempted. **Update 2026-09-22: `func_00114518` is now exact; `func_001154D0`, `func_001155A8` and `func_00115808` are still stubs.** Callee-saved registers are no longer a blocker. |
| `func_0011405C` | core_text | **not a real function** | 4 bytes of `0xCDCDCDCD`, same padding pattern as `func_00112464` and friends. |
| `func_00113AE0` | core_text | **skipped, callee-saved regs** | Uses `$16`/`$17` — hits the `sq`/`lq` open question, not attempted. **Update 2026-09-22: now kept as C,** a same-size near-miss. |
| `func_00115748` | core_text | **matches** | Bit-scan/shift helper on `*arg0`: a 2-bit fast path for the low 3 bits nonzero case (shift by 1 or 2, or return 0 untouched if bit 0 is set), falling back to the same binary-search bit-scan shape as `func_001156C0` for the low-3-bits-zero case, storing the shifted value back through `arg0` and returning the shift count (or `0x20` for input `0`). Took two branch-polarity fixes (writing `if (cond) {A} else {B}` instead of `if (!cond) {B} return; A`, in two different spots — see "Branch polarity via if/else shape" below) plus merging two textually-duplicate `*arg0 = v; return count;` tail statements into one shared one (the duplication cost 8 extra bytes vs. retail, which reuses a single tail via a jump — GCC only found that reuse once the C had one textual copy to reuse, not two identical ones). Byte-exact. |
| `func_001F9B88` | text | **matches** | `float func(float arg0) { return __builtin_fabsf(arg0); }` — folds to `abs.s`. Byte-exact. First function decompiled in `text.c` (everything above was in `core_text`). |
| `func_00208238`, `func_00208240` | text | **matches** | Both `int func(void) { return 1; }` — distinct addresses/callers, identical trivial bodies. Byte-exact. |
| `func_0021B278`, `func_0021B280`, `func_0021CD98`, `func_0021E1F8`, `func_0021EF30`, `func_00222840` | text | **matches** | All `int func(void) { return 0; }` — distinct addresses/callers, identical trivial bodies. Byte-exact. |
| `func_00209040`, `func_00225DF0`, `func_00225DF8`, `func_00226D48`, `func_00238D88` | text | **matches** | Empty functions (`{}`), splat auto-filled during the initial split like `func_001154C0`/`func_001154C8` in `core_text` — not touched this round, noted for completeness. |
| `func_001F9B90`, `func_001F9B98` | text | **close, not exact** | `max.s`/`min.s` single-instruction wrappers — this compiler doesn't fold `a > b ? a : b`/`a < b ? a : b` into the hardware instruction (falls back to `c.lt.s`/branch/`mov.s`), so written as inline asm for just the one instruction instead. Gets the right 2 instructions (the op, then `jr $31`) but in the opposite order from retail, which schedules the op into the `jr`'s delay slot — new instance of the delay-slot-scheduling open question. Tried hand-embedding `jr $31` before the op in the same asm block to force retail's ordering; GCC's flow analysis doesn't understand hand-written control flow inside inline asm and silently dropped the op entirely instead of emitting it (verified via byte diff: got `jr`/`nop`, not `jr`/op) — reverted, not safe to rely on inline-asm-embedded control flow here. See the comment on `func_001F9B90` in `src/text.c` for the full writeup. **Update 2026-09-22: both since reverted to `INCLUDE_ASM`:** each was 4 bytes longer than retail (see "Two classes of false positive" under Status). |
| `func_001E9080`, `func_001E94A0` | text | **not standalone functions** | Single instruction each (`nop`, `addiu $sp,$sp,0x20`), no `jr $31` — fallthrough fragments, same category as `func_00113AD8` in `core_text`. |
| `func_001E94C8` | text | **not investigated, unusual shape** | Four separate `addiu $sp,$sp,N` / `nop` pairs in a row, no `jr $31` anywhere — doesn't fit the usual fallthrough-fragment or padding patterns seen so far. Left as `INCLUDE_ASM`; flagging as worth a closer look rather than silently skipping, but not investigated this round. |
| `func_001E9088` | text | **not attempted, too complex** | ~0x414 bytes: floats, integer div/mod, a jump table (`jtbl_001E7940`), and 7 callee-saved registers (`$16`-`$22`). Hits the `sq`/`lq` open question on its own merits even before considering the complexity; not a good target until that's resolved regardless. |
| `func_001E94A8`, `func_001F65A8` | text | **skipped, gp-relative store** | Both end with `sw $reg, <negative offset>($28)` — a small-data-area (`$gp`-relative) store. This project builds with `-G0` (no SDA optimization) so this compiler never emits `$gp`-relative addressing; reproducing these would need knowing retail's actual `$gp` base value and enabling SDA, neither set up yet. Not attempted. **Update 2026-09-22: both now exact** under `-G2`. |
| `func_001F9BC0` | text | **not attempted, no plain-C representation** | `sq $0, 0x0($4)` — zeroes a 128-bit quadword at a pointer. No 128-bit integer type is available to express this as a single plain C store (unlike the `sq`/`lq` *spill* issue elsewhere, this is source code actually needing a quadword op, not a compiler codegen choice) — would need a vector/COP2 intrinsic this toolchain may not expose the same way retail's source did. Not attempted. |
| `func_001E97C8`, `func_001E97E8` | text | **matches** | Both `int func(void) { return 0; }` — trivial constant-return stubs, distinct addresses. Byte-exact. |
| `func_001E9E70` | text | **close, not exact** (new sq/lq evidence) | 5 sequential calls, no branches: `func_0022C7E0(); func_0022C188(); func_0022C870(); func_00234C98(0x47, 0x5360B); func_00234C98(0x4E, 0x1000000 \| (D_0015EF88 >> 13));`. Logic/instructions confirmed identical via objdump. Only saves `$ra` (no `$s0`-`$s7` at all) yet retail *still* spills it as `sq` here — the first case seen where retail uses `sq` for a lone `$ra` save with no s-regs involved; every other function so far had retail use `sd` for `$ra` alone. This compiler always uses `sd` for `$ra`, no exceptions found. Means the `sq`/`lq` choice isn't cleanly "s-regs vs ra" as the open question was previously framed — something more granular (per-function, maybe per-translation-unit) decides it. Open question below updated with this finding. **Update 2026-09-22: a stub (`INCLUDE_ASM`);** reverted at 20/88, see `notes/round-parallel-A.md`. |
| `func_001EC780` | text | **skipped, sq/lq + indirect call** | Same shape as `func_001EC270`/`func_001EC108`-adjacent dispatch-table pattern: `jalr` through a function pointer loaded from a per-type table, wrapped in the same lone-`$ra`-as-`sq` pattern as `func_001E9E70`. Not attempted. **Update 2026-09-22: now exact.** |
| `func_001ECC10` | text | **not attempted, no plain-C representation** | Two conditional 16-byte block copies via bare `lq`/`sq` — same category as `func_001F9BC0`. |
| `func_001EC030`, `func_001EC108`, `func_001EC208` | text | **not standalone functions** | Single `addiu $sp,$sp,N` (or dead-value computation + a store for `func_001EC108`), no `jr $31` — fallthrough fragments, same category as `func_00113AD8` in `core_text`. |
| `func_001F0F00` | text | **not a decompile target** | Marked "Handwritten function" by spimdisasm (uses `addi`, not `addiu`) — same category as the syscall wrappers in `core_text`. No C source ever existed for it. |
| `func_001F0F30` | text | **matches** | Fills 20 consecutive `int`s (offsets `0x00`-`0x4C` of `D_0018A3B0`) with `1`, iterating backwards. First attempt had the right logic but wrong instruction *scheduling* (the loop-setup instructions landed in a different order than retail — variable-initialization order in the source, not a branch or delay-slot issue); reordering the C statements to match retail's init sequence (base pointer, then the fill value, then the loop bound, then the offset add) fixed it. Byte-exact. |
| `func_001EBAF0` | text | **close, not exact** | `if (arg0 >= 0) { p = D_0013E650 + arg0*0x70; if (*(short*)(p+0x7E) == arg1 + D_0015F694 && (unsigned char)(*(unsigned char*)(p+0x74) - 1) < 2) return 1; } return 0;`. Needed the shared-tail-merging technique (single `&&`-combined condition instead of two separate early-return guard clauses) to get from 78% mismatch down to 8/88 bytes — the remaining diff is the scratch-register-allocation-choice issue (`$v1` vs `$a2` for a delay-slot copy of `arg1`), confirmed by trying the delay-slot-precompute technique too (no change). Kept as `INCLUDE_ASM`. |
| `func_001EDFD8` | text | **not a standalone function** | `daddu $2,$5,$0` then an `addiu $sp,$sp,0x20` epilogue-shaped instruction *mid-body*, then more stores, no `jr $31` anywhere — fallthrough fragment, same category as `func_00113AD8`/`func_001E9080`/`func_001E94A0`. |
| `func_001EDE08`, `func_001EDCE8`, `func_001EB300` | text | **skipped, `$gp`-relative access** | All use `($28)`-relative loads/stores — same known-skip category as `func_001E94A8`/`func_001F65A8` (this build uses `-G0`, never emits `$gp`-relative addressing). Not attempted. **Update 2026-09-22: `$gp` access is no longer a blocker (`-G2`): `func_001EB300` is now kept as C (a same-size near-miss); the other two are still stubs.** |
| `func_001EFD70` | text | **not a decompile target** | Marked "Handwritten function" by spimdisasm (raw pointer-chasing table walk with hand-scheduled instructions) — same category as the syscall wrappers and `func_001F0F00`. No C source ever existed for it. |
| `func_001E9730` | text | **not attempted, needs stdarg** | Spills `$a1`-`$t3` and float args `$f12`/`$f14`/`$f16`/`$f18` to the stack in the exact shape of a MIPS varargs prologue (`void f(int arg0, ...)` with `va_start`) — but this bare toolchain has no `stdarg.h`, and `__builtin_va_list`/`__builtin_va_start` aren't recognized without it (parse error, not just a missing declaration). Would need to hand-write a compatible `va_list`/builtin shim, or find the header some other way, before this specific function is attemptable. Worth flagging as a real, recognizable category (variadic function prologues) rather than a one-off. **Update 2026-09-22: now exact** as `void func_001E9730(int arg0, ...) {}`: no `stdarg.h` needed (see the Varargs note above the table). |
| `func_001F9BA0` | text | **not attempted, needs intrinsics** | `pminw` (packed integer min, COP2). Not plain-C-representable. |
| `func_001F9BB0` | text | **not attempted, known open question** | `max.s`/`min.s` clamp — same delay-slot-scheduling issue already documented and reverted for `func_001F9B90`/`func_001F9B98`; not re-attempted. |
| `func_001F9BC8`, `func_001F9BD8`, `func_001F9BF0`, `func_001F9C08` | text | **not attempted, needs intrinsics** | VU0 vector math (`vmr32`/`vadd.xyz`/`vsub.xyz`/lerp via `vmulx.xyz`+`vadd.xyz`) operating on `$vf` registers through `lqc2`/`sqc2` — vec3 add/sub/lerp helpers, same "not plain-C-representable" category as `func_001F9BC8`'s neighbors and `func_001ECC10`/`func_001F9BC0`. |
| `func_001F49B0` | text | **close, not exact** | `int count = D_0015F564; if (count < 0x40) { D_0018DD40[count] = arg1; D_0018DC40[count] = arg0; D_0015F564 = count + 1; }` — logic and size (0x50 both) match exactly, but ~30% of bytes differ, all from this compiler choosing different scratch registers for the arg0/arg1 pass-through than retail. Tried introducing explicit locals for arg0/arg1 before the branch (matching a technique that worked elsewhere); no change. Unlike the smaller, single-digit-byte instances of this same open question, the diff here is large enough (24/80 bytes) to cascade through most of the function, so — following the `func_00112468` precedent — kept as `INCLUDE_ASM` rather than committed as "close" C. **Re-tested against the newer techniques, still 24/80** — the blocker is the `%hi`-register-reuse sub-case in the function's *first two instructions* (retail `lui $6,%hi(D_0015F564)` / `lw $6,%lo(...)($6)` reusing one register), which precedes the array accesses entirely, so array-access-form levers can't reach it. A real negative result, not an untried case. **Update 2026-09-22: now exact** once its count was declared `MACRO_ADDR` (see "SOLVED: the one-instruction macro form"). |
| `func_001F7648` | text | **matches** | Struct/record initializer: 8 `short` fields set from register+stack arguments (`void *arg0, int a1..a8`, the 8th arg passed on the stack), plus 4 more zeroed. Byte-exact, but only after empirically finding the right *source statement order* for the four independent zero-stores — writing them in the same order as retail's final instruction sequence did **not** reproduce that sequence (the compiler reordered them anyway); the order that actually worked (`s[6]=0; s[7]=0; s[10]=0; s[11]=0;`, i.e. ascending-then-two-more, not retail's own `0x16,0xC,0xE,delay=0x14` sequence) was found by trying a few permutations and checking the byte diff each time. Refines the "Loop-setup statement order" technique below — for a block of several independent identical-shape stores, matching retail's own instruction order in source is a reasonable first guess but not guaranteed; when it fails, try other permutations rather than assuming the technique doesn't apply. |
| `func_002071D0`, `func_002071E0`, `func_002071F0`, `func_002073B8`, `func_002073C8`, `func_002073D8`, `func_002073E8`, `func_002073F8` | text | **matches** | All `int func(void) { return D_XXXXXXXX_byte != 0; }` — boolean-from-byte-flag, same shape (`lbu` a byte global, `sltu $2,$0,$2` to normalize to 0/1), distinct flag addresses. Byte-exact, all 8 first attempt. `func_002073A8` is the same shape but via a `$gp`-relative load — left alone, known skip category. |
| `func_001F3D00` | text | **matches** | GS privileged-register setup (`0x1200_00XX` = the GS's memory-mapped register block): CSR ack, PMODE, then SMODE2/DISPFB1/DISPFB2/DISPLAY1/DISPLAY2/BGCOLOR set from a 3-entry table (`D_00151888`). First attempt had 45% mismatch from one wrong address: `a0`'s `ori` (completing `0x1200_0000` to `0x1200_00A0`) is scheduled *after* an earlier store that reuses `a0` while it still only holds the upper 16 bits — that store's real target is `0x12000000` (PMODE), not `0x120000A0` (DISPLAY2) as a first read of the register's *final* value suggested. Fixed by reading each store's address off the register's value *at that point in program order*, not its eventual fully-formed value. Byte-exact after the fix. |
| `func_001FFA90` | text | **matches** (was previously close-not-exact) | `int *p = &D_0019A4E8; int v = D_001941CC; p[5] = v + 0x64000; p[4] = v;`. **The earlier entry for this function was wrong** — it filed the 14/40 diff under the scratch-register-allocation-choice question ("nearly every register field differs even though every opcode matches"). It was just store order: two stores through one materialized base, so the rotation rule applies directly (retail emits `0x10` then `0x14`, so source is `0x14` then `0x10`), byte-exact. Second cautionary case (with `func_00217EC0`) of a large all-registers-differ diff being *downstream* of one wrong store order rather than an allocator difference — a rotated store pair shifts every subsequent register assignment, which looks exactly like an allocator mismatch if you don't test the rotation. |
| `func_001FB448` | text | **matches** | `D_00152178 = (long)arg0 \| ((long)arg1<<8) \| ((long)arg2<<16) \| 0x80000000L;` (64-bit `sd` store, packs 3 small values + a flag bit into a global). First attempt used explicit `(unsigned int)` casts before widening to 64-bit, which made this compiler insert extra `dsll32`/`dsrl32`/`dsrl` masking retail doesn't have; retail just sign-extends each `int` parameter directly into 64-bit and shifts/ORs with no masking. Removing the unsigned casts (plain `(long)arg0` etc.) fixed it. Byte-exact. |
| `func_00207CB0` | text | **matches** | `if (arg1 >= 0x101) return D_001414DC == 0xF; return D_0013D4C5 != 0;` (`arg0` unused — never referenced in the disassembly). First attempt had the condition inverted (`if (arg1 < 0x101) ... else ...`) — logically equivalent but produced the opposite branch polarity from retail; flipping to match retail's actual `beqz`-tests-the-"else"-case sense fixed it. Byte-exact. |
| `func_00207E28` | text | **close, not exact** | `result = (arg0 < 0xE0 && arg1 <= 38.0f) ? 1 : 0;` (`38.0f` = `0x42180000`). Confirmed correct via objdump (same operations/registers/constant), but retail's FP-condition-to-integer materialization (`bc1t` with both the "set 1" and "reset to 0" as literal delay-slot/fallthrough instructions) isn't reproduced by any source shape tried (single `&&` expression, nested `if`, default-then-override) — all compile to a `bc1f`/`bc1tl`-based scheme instead. New instance of the delay-slot-scheduling open question, this time on FP-condition materialization rather than store/branch-target ordering. 26/52 bytes differ — too large to keep as documented-close C per the `func_00112468` precedent, left as `INCLUDE_ASM`. |
| `func_00207EC0` | text | **close, not exact** | `if (arg0 >= 0xBE) return D_0013D4E0 != 0; return (arg1 >= 58.5f) ? 1 : 0;` (`58.5f` = `0x426A0000`). The early-return half matches exactly once written with the right branch polarity (`arg0 >= 0xBE` as the `if`, not `arg0 < 0xBE`) — same fix as `func_00207CB0`. The float-threshold half hits the identical delay-slot-scheduling issue as `func_00207E28` right above. 19/64 bytes differ, left as `INCLUDE_ASM` for the same reason. |
| `func_001F9AF0` | text | **not a decompile target** (was previously "close, not exact") | DMAC register setup at base `0x1000D000` plus an `INTC_STAT` read at `0x20100000`. Hand-written assembly, same idiom set as the `func_0020C210`/`func_0020C230` block: `$1`/`$at` kept live as the base across all four stores, `ori $2,$0,0x100` instead of `addiu`, a bare `nop` between stores — and decisively, it leaves its final value in **`$3`**, not `$2`, the ABI return register, which no compiler emits for a value-returning function. **The earlier entry was wrong** to file this under the scratch-register-allocation question. |
| `func_001FA748`, `func_001FA790` | text | **close, not exact** | Angle-wrap-to-`[-pi,pi]` on `arg0+arg1` / `arg0-arg1` respectively (retail's `else if` shape confirmed by the delay-slot second-compare testing the *original* sum, only meaningful when the first branch wasn't taken). Logic and instruction sequence confirmed correct via objdump, but this compiler allocates the sum/diff into `$f12` (reusing `arg0`'s register) where retail uses a fresh `$f0` — scratch-register-allocation-choice question, now confirmed to apply to FP registers too, not just integer. Tried a separate-assignment-then-accumulate source shape instead of one combined expression; no change. Left as `INCLUDE_ASM`. |
| `func_001FA168`, `func_001FA190`, `func_001FA1C0`, `func_001FA1F8`, `func_001FA218`, `func_001FA460`, `func_001FA480`, `func_001FA4A0` | text | **not attempted, needs intrinsics** | VU0 vector-math helpers (identity-matrix builders, `vcallms` microprogram calls, bare `lq`/`sq` 128-bit block copies) — same "not plain-C-representable" category as the earlier VU0 entries. This whole address range (`func_001FA058`-`func_001FA6C0`-ish) is a vector/matrix math cluster, mostly VU0-heavy; skipped without individually re-deriving why for each one. |
| `func_001FB530` | text | **close, not exact** | Appends a 2-word GIF/DMA-style tag pair to the packet buffer `D_00161000` points at, then advances it by one qword. Confirmed via objdump: same fields/values/order/size (0x68 both), but retail re-derives `D_00161000`'s own *address* (a fresh `lui`/`lw` pair) before every field write, where this compiler computes the address once and only reloads the stored *value* each time. A new, more extreme variant of the redundant-global-reload pattern (previously only ever seen for a global's *value*, never its address, since a global's address is link-time-constant). Tried an explicit `*(unsigned int **)&D_00161000` reinterpret-cast to defeat the compiler's CSE confidence; no change. Kept as `INCLUDE_ASM`, diff too large/pervasive to call "close" C. |
| `func_001FB598` | text | **skipped** | Same `D_00161000` cluster as `func_001FB530` (hits the same address-reload issue) plus a `$gp`-relative final store — two known-skip categories stacked. **Update 2026-09-22: now exact.** |
| `func_002094E0` | text | **close, not exact** (13/64) | If the struct at `D_0013D390`'s kind field (offset `0xDC`) is `2` and its status field (`0xE4`) is negative, resets status to `7`, zeroes field `0xE8`, and sets global error code `D_0015EFB0 = 0xB`. Needed an explicit `char *s` local (materializing the struct base address once via `addiu`, matching retail) to go from 58% mismatch down to 20.3% — without it the compiler folds each field offset directly into load/store immediates instead of forming a real pointer. Remaining 13/64 bytes are the established store-order/scratch-register-choice question (retail stores both struct fields before computing `D_0015EFB0`'s address; this compiler computes that address between the two field stores, and picks a different register for the constant `0xB`). Kept as documented-close C per the `func_00112380`/`func_001EBAF0` precedent (diff is comparable in relative size to those, not the much larger diffs that got reverted). **Update 2026-09-22: now exact.** |
| `func_001FB848`, `func_001FB8A8` | text | **skipped, `$gp`-relative store** | Same `D_00161000`-derived-pointer GIF/DMA-tag-write shape (writes into `D_00151A00`/`D_00151C60` respectively via the value stored at `D_00161000`), each ending in a `sw $reg, <offset>($28)` — known skip category. **Update 2026-09-22: both now exact.** |
| `func_001FBAB8` | text | **skipped, callee-saved regs** | Uses `$16`-`$22` — hits the `sq`/`lq` open question, not attempted. |
| `func_001FE4C0`, `func_001FE580`, `func_001FF660`, `func_001FF950` | text | **not standalone functions** | Single instruction each (`addiu $sp,$sp,N` / a bare `sw`), no `jr $31` — fallthrough fragments, same category as `func_00113AD8` in `core_text`. |
| `func_001FDF10` | text | **attempted, reverted — new delay-slot instance** | `if (arg0 > 0x20000) { *arg1 = 0; *arg2 = 0; return -1; } *arg1 = D_001941C0.field4 + D_0016100C - arg0; *arg2 = D_001941C0.field8 + D_0016100C - arg0;` (`D_001941C0` fields at offsets `0x4`/`0x8`). Logic confirmed correct, but retail schedules the `t0=a1` pointer copy into the branch's delay slot where this compiler hoists it before the compare — tried an explicit early-local variant (same technique that fixed `func_0011AA68`), no change. 53% mismatch, reverted per the size-of-diff precedent rather than kept as misleading "close" C. |
| `func_001FE4D0` | text | **attempted, reverted — real structural difference** | Linear search over an array of `0x10`-byte records (checking `elem.field4 == arg0`, first element special-cased via direct pointer deref before the loop, returning the matching index or `-1`), count field at `D_001997D0+0x2C`. Logic confirmed correct via careful disassembly reading, but two source-shape attempts (early-return guards, then `result` variable with `break`) both landed at 70-78% mismatch with differences starting from the very first instructions (not just register/scheduling — retail materializes values in a different *kind* of instruction, not just a different register for the same kind). This is a bigger structural gap than the usual near-miss categories, not just delay-slot/register-allocation; reverted rather than force it. Worth a fresh attempt later with more careful attention to instruction-level structure, not source-level logic. |
| `func_001FF4F8` | text | **attempted, reverted — real structural difference** | Linear scan of a `short`-keyed table (stride 4 bytes) for `arg0`, with a `movn` (conditional-move) idiom picking between an accumulated fallback offset and `idx*4` depending on whether `arg1` is nonzero, then an optional output write through `arg2`. Logic reconstructed from the disassembly (including the exact conditional-move semantics) but the compiled C didn't reproduce the instruction shape at all (75% mismatch starting at byte 0xa) — this compiler's `if (arg1) off = idx*4;` doesn't appear to fold to the same `movn`-based shape retail's source did. Reverted; would need a specific idiom (possibly a ternary, or the `movn` genuinely requires source retail had that this rewrite didn't capture) to be worth another attempt. |
| `func_001FF668` | text | **attempted, reverted — real structural difference** | Walks a linked-list-like array of `unsigned short` keys (stride 8 bytes) counting entries until hitting `arg0` or the `0xFFFF` end sentinel. Two source shapes tried (separate `if`s per early check; a combined single read with `\|\|`) — both landed at 52-63% mismatch. Retail's actual instruction shape (two independent `beq`s against the same first-read value, sharing one return target) isn't naturally produced by either the split-`if` or combined-`\|\|` C forms in this compiler. Reverted; the "read once, do two `beq`s to a shared tail" idiom needs a more specific source shape than tried here — not obviously a small delay-slot/register tweak like the earlier documented techniques, closer to `func_001FE4D0`'s "different kind of instruction" gap. |
| `func_0023CE28` | text | **matches** | `return 1;`. Byte-exact. |
| `func_0023E000` | text | **matches** | `arg0[42] = 0;` (field at byte offset `0xA8`). Byte-exact. |
| `func_0023E050` | text | **matches** | `return arg0[42];` (same field as `func_0023E000`, getter counterpart). Byte-exact. |
| `func_0023E058` | text | **matches** | `int old = arg0[42]; arg0[42] = arg1; return old;` — same field again, swap-and-return-previous setter. Byte-exact. |
| `func_0023E698` | text | **matches** | `return arg0[3] == 0;`. Byte-exact. |
| `func_0023CE18` | text | **matches** | `arg0[1] = arg1; arg0[0] = arg2; return 1;` — needed source statements in this exact order (the reverse of retail's own instruction order: retail stores `arg2` first then `arg1`, but this compiler reverses whatever order the source states them in for this shape) to get retail's actual store order. Byte-exact. |
| `func_0023E5C8` | text | **matches** | `return (arg0[3] ^ arg0[4]) == 0;`. Byte-exact first attempt. |
| `func_0023E5B8` | text | **matches** (was previously close-not-exact) | `void func(volatile int *arg0) { arg0[3] = 0; arg0[2] = 0; }`. Previously documented as a delay-slot-scheduling near-miss (this compiler filled `jr`'s delay slot with the second store, 12 bytes vs retail's 16 with an unfilled slot). Typing the parameter `volatile` reproduces retail exactly — the unfilled delay slot was a `volatile` side effect, not a scheduling difference. Byte-exact. First confirmation that the `volatile` signature reclassifies previously-filed "open question" instances. |
| `func_0023E040` | text | **close, not exact** | `arg0[42] = 1; return 1;`. Retail materializes the constant `1` once and reuses the same register for both the store and the return value; this compiler materializes it twice into two different registers regardless of source shape (shared local, assignment-expression `return arg0[42]=1;`, bare duplicate literal — all three tried). New instance of the scratch-register-allocation-choice question, this time as "fails to reuse an identical constant" rather than picking different registers for genuinely different values. **Update 2026-09-22: now exact.** |
| `func_0023CEC8` | text | **matches** | `int v1 = ((self[2] << 4) + self[1] + 0x10) & 0xFFFFFFF; if (arg1 == v1) return 0; return (unsigned int)(arg1 - self[0]) >> 11;`. Byte-exact first attempt. |
| `func_0023CFF0` | text | **not attempted, compiler limitation** | Packs `arg1<<32 \| arg2<<28 \| arg3` into a 64-bit store. This compiler's C frontend cannot compile **any** 64-bit shift by a constant that isn't a multiple of 32** — confirmed with multiple isolated single-line test cases (`(unsigned long long)x << 28`, `x << 32 >> 4` as one expression, and even a shift-by-4 alone on an already-64-bit local variable in its own statement all fail identically: `unsupported wide integer operation`, a hard compile error from `cc1`, not a codegen quirk). Retail achieves the `<<28` via `dsll32`(by 32)+`dsrl`(by 4) — a real 64-bit shift-by-4 instruction the assembler accepts fine, so the *original* C source for this function almost certainly used inline asm for that step, not a plain C shift operator. New, real toolchain limitation, distinct from the four existing open-question categories (it's a hard compiler error, not a near-miss) — added to "Open toolchain questions" below. Not worth attempting further without inline asm. **Update 2026-09-22: now exact:** the "compiler limitation" does not apply to `long` (see the RESOLVED entry under "Open toolchain questions"). |
| `func_001EE6D0`, `func_001EE850`, `func_001EE9E8`, `func_001F0F70`, `func_001F0FF0`, `func_001F2410`, `func_001F2550`, `func_001F6CE0`, `func_001F7C50` | text | **not standalone functions** | More single/double `addiu $sp,$sp,N` (or a bare `sw`) fragments with no `jr $31` — same fallthrough-fragment category as `func_00113AD8` and friends in `core_text`. |
| `func_0023CF10`, `func_0023CF80`, `func_001F6600`, `func_001F6620`, `func_001F6640`, `func_001F7B40` | text | **skipped, callee-saved/sq-lq** | All save `$16` or `$31` via `sq`/`lq` — hits the open `sq`/`lq` question, not attempted. **Update 2026-09-22: all six now exact** with the per-segment compilers. |
| `func_001F9850`, `func_001F9878`, `func_001F9888`, `func_001F98B0` | text | **skipped, `$gp`-relative** | FP constant loads via `($28)` offsets — known skip category (`-G0` build, no SDA support configured). |
| `func_001F9AF0`-`func_001FA898` (`func_001F9B20`, `func_001F9B50`, `func_001F9C30`, `func_001F9C48`-`func_001FA898` and neighbors) | text | **VU0 cluster, not attempted** | Spot-checked several (e.g. `func_001F9C30`: `lqc2`/`qmtc2.ni`/`vmulx.xyz`/`sqc2`) — this whole address range is the same vec3/matrix VU0 math cluster already noted for `func_001FA168`-`func_001FA4A0`, just a wider span than previously scoped. Not individually re-verified one by one; recognize the range and skip rather than re-deriving per-function. |
| `func_00207E60`, `func_00207E70`, `func_00207E80`, `func_00207E90`, `func_00207EA0`, `func_00207EB0`, `func_00207F00`, `func_00207F10`, `func_00207F20`, `func_00207F30`, `func_00207F40`, `func_002081F8`, `func_002082E8`, `func_002082F8`, `func_00208308`, `func_00208318`, `func_00208328` | text | **matches** | All `int func(void) { return D_XXXXXXXX_byte != 0; }` — same shape as the `func_0020xxxx` cluster matched earlier (`func_002071D0` etc), just a later cluster of the same per-flag getter pattern. 17 functions, all byte-exact first attempt, distinct flag addresses. |
| `func_00200190`, `func_002008B0` | text | **not standalone functions** | Single `addiu $sp,$sp,N`, no `jr $31` — fallthrough fragments, same category as `func_00113AD8`. |
| `func_00200198` | text | **skipped, callee-saved regs** | Uses `$16` via `sq`/`lq` — hits the open `sq`/`lq` question. |
| `func_001FFA90` neighbor cluster (`func_001FFC48`, `func_00208248`, `func_00208338`) | text | **skipped, callee-saved regs** | All use `$16`/`$17` via `sq`/`lq` — hits the open `sq`/`lq` question. **Update 2026-09-22: `func_001FFC48` and `func_00208338` are now kept as C (same-size near-misses); `func_00208248` is still a stub.** |
| `func_001FFFA0` | text | **skipped, `$gp`-relative** | `lw`/`sw` via `($28)` offset — known skip category (`-G0` build, no SDA support). **Update 2026-09-22: now exact** under `-G2`. |
| `func_00208160`, `func_00208208` | text | **not attempted, known open question** | Float-threshold-to-bool materialization (`c.le.s`/`bc1f`/`bc1tl` scheme) — same delay-slot-scheduling issue already documented and reverted for `func_00207E28`/`func_00207EC0`; not re-attempted. **Update 2026-09-22: `func_00208208` is now kept as C (a same-size near-miss); `func_00208160` is still a stub.** |
| `func_0020CB80`, `func_0020CC10`, `func_0020CC38`, `func_0020CC60`, `func_0020CD58` | text | **matches** | All `if (D_int_flag != 0 && D_byte_flag != 0) return 1; return 0;` — distinct global addresses, same shape. Needed the `&&`-combined single-condition form (shared-tail merging), not a `!= 0` boolean-cast return or a `==0`-guard-then-cast — see "Shared-tail merging with a boolean cast" below, a refinement of the existing shared-tail technique. |
| `func_00216150`, `func_00216198` | text | **matches** | Two more functions reclaimed from the over-broad `movz`/`movn` skip category, both byte-exact first attempt. Same shape: count the nonzero bytes in a fixed-length global byte array (`0x25` of `D_0013E620` / `0x20` of `D_0013D510`), clamp negative to 0, then `return (count < LIMIT) ? count : LIMIT-1;`. All three `movn`/`movz` instructions come from ordinary C — `if (arr[i] != 0) count = count + 1;` in the loop (which lands in the branch's delay slot), the `if (count < 0) count = 0;` clamp, and the final ternary. The `if (count < 0)` guard is dead code in practice (the count cannot go negative) but retail has it, so it belongs in the source. |
| `func_002160E0` | text | **matches** | Nested-loop variant of the `func_00216150` counting shape: outer loop over 20 rows of `D_0014BFC0`, inner loop over 4 bytes per row (`for (k = 3; k >= 0; k--)`, which the compiler rotates into the `bgez` do-while retail has), counting nonzero bytes, then the same negative-clamp and `(count < 0x29) ? count : 0x28` tail. Byte-exact first attempt. |
| `func_00227018` | text | **matches** | Linear search of a 5-entry table of `{int key; int flags;}` pairs at `D_001D6448` (stride 8): returns `(e[1] & 1) ? 0x4F000 : 0x11800` on a key hit, `-1` if none of the 5 match. The `movn` is just that ternary. Written as a `do`/`while (i < 5)` with the key test first — the `i++` sits in retail's `bne` delay slot so it runs every iteration regardless. Byte-exact first attempt. |
| `func_00205728` | text | **matches** | Bidirectional slot search over `D_001A0468`: walks `i` from 0 to 4 picking `idx = arg0 ? (4 - i) : i` (that reversal is the `movz`), returning the first `idx` where `D[idx] != 0 && D[idx+5] == -1`, else `-1`. Needed **two separate base pointers** (`int *a = D_001A0468; int *b = D_001A0468 + 5;`) rather than indexing `D_001A0468[idx + 5]` — retail hoists a second base at `+0x14` and indexes both by the same scaled offset, which the `idx + 5` form doesn't produce. 42% -> byte-exact from that one change; another instance of the documented pointer-advance-vs-index lever. |
| `func_0020CBE0` | text | **matches** | Same shape as the cluster above but on two fields of one struct (`D_0013D6B8+0x40C`, `+0x3FC`) instead of two separate globals: `if (base[0x40C] != 0 && base[0x3FC] != 0) return 1; return 0;`. |
| `func_0020CC88` | text | **matches** | Same shape again, on `D_0013D5C8`'s byte fields `0x21`/`0x1F`. Needed `unsigned char *` for the base pointer (not `char *`) — see "lbu vs lb" note below. |
| `func_0020CCB8` | text | **matches** | `return D_0013D5C8_bytes[arg0] != 0;` — indexes the same global as `func_0020CC88`, byte-exact first attempt, no gotchas. |
| `func_0020CD28` | text | **matches** | `if (D_0013D9B4 != 0) { return D_0013D4B0 ? 2 : 1; } return 0;` (compiles the ternary to a `movz`). Needed the explicit `if (main-case) {...} return 0;` form (main case as the `if`-body, not `if (guard) return 0;` early-return) — same branch-polarity-family fix as `func_00207CB0`, applied to which arm is "inline" vs "jumped to" rather than `beqz`-vs-`bnez` sense. |
| `func_0020CD80` | text | **matches** | `if (D_0013D5DD != 0) return 2; return D_0013DC34 != 0;` — matched first attempt with the straightforward form (no shared-tail merge needed: this one's two "return 0" cases don't actually converge in retail, it's a plain sequential two-guard shape, not the converging-boolean shape the `&&`-cluster needed). |
| `func_0020CDA8` | text | **matches** | `return D_0013D5E7 != 0;` — simple byte-flag getter, byte-exact first attempt. |
| `func_0020CDB8` | text | **matches** | `if (base[0x1F] != 0) return 2; return base[0x21] != 0;` on `D_0013D5C8` — same `unsigned char *` fix as `func_0020CC88`. |
| `func_0020CBA8` | text | **matches** (was previously close-not-exact) | `if (D_0013D9B4 != 0 && D_0013D490[0x20] != 0 && D_0013D490[0x21] != 0) return 1; return 0;` with `extern unsigned char D_0013D490[];`. **The earlier entry for this function was wrong** — it concluded the `&&`-chain technique "didn't carry over" from 2 conditions to 3. The condition count was irrelevant; the problem was the byte-array access form. Retail materializes `D_0013D490`'s address *lazily in the first branch's delay slot*, which is precisely the documented behaviour of **direct** global indexing (`D_GLOBAL[i]`) — the previous attempt used a base-pointer local (`unsigned char *base = ...`), which forces the address up front instead. Switching to direct indexing gives it byte-exact. Note the neighbours `func_0020CC88`/`func_0020CDB8` need the *opposite* form, so this is the two-sided lever behaving as documented, not an inconsistency. |
| `func_00209048` | text | **close, not exact — 4/36, one instruction** (was 16/36) | 2D cross-product orientation test (is `(x2,y2)` left of the `(x0,y0)->(x1,y1)` edge), 6 `int` params, kept as C. **The earlier entry claimed the instruction order already matched retail; it didn't** — the four independent in-place subtractions were rotated, and fixing that (writing `x2`/`y2` before `x1`/`y1`) brought it from 16/36 to 4/36 with every register now identical. This extends the store-order rotation rule: **it applies to runs of independent *arithmetic* statements, not only stores.** The sole remaining diff is the final sign test — this compiler emits `srl $2,$2,31`, retail has `slti $2,$2,0`; same result, different instruction selection, and not steerable from source (`< 0`, `<= -1`, `< 1-1`, a named local, and `?1:0` all canonicalize to `srl`; widening to `long` gives `dsrl32`, which is worse). See the new sign-test entry under "Open toolchain questions". **Update 2026-09-22: now exact:** the `< 0` is written as an explicit `if`/`return` so it feeds a branch (see "`slti reg,reg,0` vs `srl reg,reg,31`" near the top). |
| `func_00209160` | text | **close, not exact** (18/36) | Struct field shuffle (`D_0013D390`): read field `0xC4` into a temp, zero field `0xFC`, write the temp to field `0x1C`, plus `D_0015EFB0 = 3`. Confirmed correct logic. Re-tested against the store-order rotation rule (source `0x1C` then `0xFC`, to obtain retail's emitted `0xFC` then `0x1C`): **no change, still 18/36** — a real negative result for the rule, and consistent with it being base-pointer-scoped, since the `D_0015EFB0` store goes through a second base (same failure mode as `func_00219E60`). Residual is retail materializing the literal `3` early (into `$3`, before the base's own `addiu`) and using `$1`/`$at` for `D_0015EFB0`'s `%hi`, where this compiler orders those differently and uses a normal temp. |
| `func_002098A8` | text | **close, not exact** (7/32) | `if (D_0013D3AC != 0) D_0015EFB0 = 3;`. Retail schedules the literal `3` into the branch's delay slot; this compiler schedules the `D_0015EFB0` address computation there instead. Same open question as `func_00209160` right above. Since re-checked against both newer techniques and neither helps: a `*(volatile int *)&D_0015EFB0 = 3;` store (the signature that fixed `func_0023E710`/`func_0023E5B8`) and hoisting the constant into its own local before the `if` (the documented delay-slot-steering technique) both leave it at exactly 7/32. Genuinely the scheduling question here, not a volatile or statement-order artifact. **Update 2026-09-22: now exact.** |
| `func_0020C210`, `func_0020C230` | text | **not decompile targets** (was previously "close, not exact") | Hand-written assembly, part of the same DMAC block as `func_0020C268`/`func_0020C2F8`, which spimdisasm *does* explicitly mark `/* Handwritten function */`. These two use identical idioms but escaped the marker heuristic (which keys on things like `addi` vs `addiu`), so a marker's absence is not evidence of compiler origin. Tells: the base address `0x1000D400` is kept live in **`$1`/`$at`** across all four stores — `$at` is assembler-reserved on MIPS and GCC will never allocate it — and `0x100` is materialized with `ori $2,$0,0x100` where a compiler emits `addiu`. `func_0020C230` is conclusive on its own: hand-inserted `nop` delay padding after an MMIO load, an `alabel` alternate entry point mid-function, and a hand-coded spin loop (`j func_0020C238`) back to it. **The earlier `func_0020C210` entry was wrong** — it read this as compiler output and invented a "redundant address reload, now also for bare constant addresses" codegen category to explain the 27/32 diff. That category never existed; retiring it. Cautionary example: an unexplained *large* diff in MMIO-adjacent code is worth checking against the hand-written-asm tells (`$at` as a live base, `ori` for small constants, `nop` padding, `alabel` entries) before positing a new compiler gap. |
| `func_0020E340` | text | **close, not exact** | Packs 4 values into a 64-bit field (`(arg1<<32) \| arg2 \| (arg3<<8) \| (arg4<<16)`, stored to `arg0+0x38`). Same instruction sequence/order as retail, but the widened `arg1` lands in a different register. 13/32 bytes differ. **Update 2026-09-22: a stub (`INCLUDE_ASM`);** reverted at 34%, see `notes/movn-category.md`. |
| `func_00215048`, `func_00215078` | text | **close, not exact** | Both: null-check `arg0`, read a flags halfword at `+0x34`, test bit `0x20`, then return `*(int*)(*(int**)(arg0+0x78) + N)` (`N` = `0x0`/`0x10` respectively). Logic and overall shape confirmed correct, but retail has two literal `nop` instructions between the mask and the following branch that this compiler doesn't emit — schedules straight through instead. Not one of the previously-documented delay-slot/register-allocation patterns exactly (nop *insertion*, not a different instruction choice). 16/48 bytes differ each. **Update 2026-09-22: both are stubs (`INCLUDE_ASM`).** The two `nop`s are the R5900 short-loop padding described under "Open toolchain questions". |
| `func_0021B288` | text | **matches** | `*(int *)((char *)arg0+0x44) = -1; return 0;`. Byte-exact first attempt. |
| `func_0021DAC8` | text | **matches** | `D_001A0418 = -1; return 0;`. Byte-exact first attempt. |
| `func_0023C2B0` | text | **matches** | `return *(int *)((char *)arg0+0x50) >= 0x1000;`. Byte-exact first attempt. |
| `func_00223478` | text | **matches** | Zeroes 3 fields of `arg0` (`+0x3C`, `+0x40`, `+0x50`), returns 0. Needed source order `0x40, 0x50, 0x3C` to get retail's actual store/delay-slot scheduling — none of the 4 orderings tried matched the *source* order 1:1, this compiler's scheduling for 3 independent same-shape stores doesn't correspond simply to source order; found empirically by trying permutations and checking the byte diff, same spirit as `func_001F7648`'s note on this. |
| `func_0023CD10` | text | **matches** | Zeroes a small header at `arg0+0x50000` and stores the constant `0x50000` itself into its `+8` field (not the pointer — a plain register-value misread on first pass, corrected via the byte diff/objdump before concluding it was a scheduling issue). Needed one statement-order fix (`+8`, then `+0`, then `+4`) to land the right store in the branch-... in the `jr`'s delay slot. Byte-exact. |
| `func_00217EC0` | text | **matches** (was previously reverted) | `char *p = D_0013CA40; *(short*)(p+0x18E)=0; *(int*)(p+0x190)=0;`. **The earlier entry for this function was wrong** — it claimed retail computes the two field addresses independently with a `lui`/`addiu` pair per store; retail actually materializes the base *once* (one `lui`/`addiu` into `$2`) and uses it for both stores, exactly like this compiler does with an explicit `char *` local. The real (and only) problem was store order, fixed by the rotation rule below. Byte-exact. Cautionary example: the "40% mismatch, unexplained category" note was a misreading of the disassembly, not a real new gap. |
| `func_0023C9B0` | text | **not attempted further** | `D_001612E0 = 0;`. First attempt used `$2` for the global's address where retail uses `$1` (`$at`) — an unusual register choice for a compiler to make on its own, more consistent with the surrounding DMAC/hardware-register cluster (`func_0020C230`, `func_0020C268`, `func_0020C2F8` — several explicitly marked `/* Handwritten function */` by spimdisasm) being genuinely hand-written code, not compiler output. Reverted; not worth iterating on further since the root cause is likely "this wasn't compiled from C at all," not a source-shape issue. **Update 2026-09-22: now exact,** as `*(volatile int *)&D_001612E0 = 0;` with the global declared `MACRO_ADDR` (`src/game/movie/disp.c`). The `$at` store is the assembler's macro expansion, not a sign of hand-written code (see "SOLVED: the one-instruction macro form"). |
| `func_0022F090` | text | **close, not exact** | `if (arg1 != 0) *arg1 = arg0;` (void). Missing the known `dsll32`/`dsra32` sign-extension pair on the pointer parameter before use — same open-question category as `func_00112380`, not a new instance worth re-investigating (already extensively tried there). Reverted. **Update 2026-09-22: now exact** with a `long` parameter narrowed at the use site (see "SOLVED: the `dsll32`/`dsra32` sign-extension question"). |
| `func_001FA888` | text | **matches** | `return (float)arg0;` — int-to-float conversion (`mtc1`/`cvt.s.w`). Byte-exact. |
| `func_001FA898` | text | **close, not exact** | The float-to-int inverse of `func_001FA888`. Retail's `cvt.w.s` converts in place (dest == src == `$f12`, the incoming argument register); this compiler always allocates a fresh destination register for the conversion result. Only 2/16 bytes differ — that one instruction's register field. Small new instance of the scratch-register-allocation-choice question. An extra `(float)(int)` round-trip to nudge it toward reusing `$f12` made it worse (50% mismatch), reverted. **Update 2026-09-22: a stub (`INCLUDE_ASM`), not kept as C.** |
| `func_00216F28` | text | **matches** | Writes the `short` value `4` to offsets `0x5C`/`0x40`/`0x78` of `D_001517D0`. First attempt (source in retail's own store order) came out 3/28 with the stores rotated; fixed predictively via the store-order rotation rule — see "Independent-store order: last source statement emits first" below, which this function established. Byte-exact. |
| `func_0021EF38` | text | **matches** | Resets 4 fields of `arg0`: float `0x38` = pi (`3.14159274f` = `0x40490FDB`), ints `0x34`/`0x44`/`0x48` = 0, returns 0. Byte-exact **first attempt** by applying the store-order rotation rule predictively (wrote source as `0x38, 0x34, 0x44, 0x48` to get retail's emitted `0x48, 0x38, 0x34, 0x44`) — confirms the rule holds across 4 stores and with a mixed float/int store set, not just the 3-int case it was derived from. |
| `func_0023E710` | text | **matches** | `if (arg0[3] > 0) arg0[3] = arg0[3] - 1;` — a clamped decrement, with `arg0` typed `volatile int *`. Retail reloads the field after the branch instead of reusing the already-loaded value, *and* leaves the `blez`'s delay slot empty; a non-`volatile` version compiles 8 bytes shorter (reuses the loaded value, fills the delay slot with the decrement). Adding `volatile` reproduced both retail behaviours exactly. Byte-exact. See "Redundant reload + unfilled delay slot = `volatile`" below — this is a notable finding, since it means some previously-documented "delay-slot-scheduling" near-misses may actually be volatile accesses. |
| `func_0023CDF0` | text | **close, not exact** | `int *p = (int*)((char*)arg0+0x50000); int avail = p[1]; int taken = (arg1 < avail) ? arg1 : avail; p[1] = avail - taken;` — a saturating subtract (deduct `arg1` from a counter, floor at 0), retail using `slt`+`movn` for the `min`. Logic and instruction sequence confirmed correct, and the `0x50000` pointer advance + `+4` field offset split was needed to reproduce retail's `lui 0x5`/`addu`/`lw 4($4)` shape (a single `+0x50004` offset instead materialises the whole constant first — worth knowing for other big-offset cases). Remaining diff is the scratch-register-allocation-choice question: retail reuses `$2` for both the `lui` scratch and the loaded value where this compiler takes `$3` for the scratch, and in a 9-instruction function that shifts nearly every register field (20/36 bytes). Tried the in-place-accumulate technique (`avail -= taken;`) as well; no change. Reverted per the size-of-diff precedent. |
| `func_00222950` | text | **matches** | `if (D_0013CC04 & 0x40) { D_001D5F78 = D_001D2678; } return 0;` — sets a global pointer to a fixed buffer's address when a flag bit is set. Byte-exact first attempt; notably retail schedules the pointee's `lui` into the branch delay slot, which is what this compiler does naturally (contrast `func_002098A8`, where retail wanted a *constant* in the slot and this compiler puts the address `lui` there — so the two aren't a single consistent preference). |
| `func_00234350` | text | **close, not exact** (1/40) | `if (arg0 >= 0x40) return -3;` then returns field `+4` of the `arg0`'th 16-byte record of `D_001DD568`. Needed the `>= 0x40` guard-first polarity (the `< 0x40`-first form inverts the branch and inlines the wrong arm), and the field access written as a record-offset pointer (`rec + 4`) rather than index arithmetic (`base[arg0*4+1]`, which folds the `+4` into an `ori` instead of the load's offset). Remaining single byte is the final `addu`'s commutative operand order — see the comment on it in `src/text.c` for the four forms tried. Kept as C per the tiny-isolated-diff precedent. **Update 2026-09-22: now exact.** |
| `func_0021EDD8` | text | **close, not exact — cmov vs branch** | `short v = (D_001414F4 == 1) ? 0 : 3;` stored to `+2` of the pointer at `arg0+0x34`, returns 0. Retail *branches* and stores once at the join; every single-store C form tried here (default-then-override, if/else assigning a temp, ternary) compiles branchlessly to `xori`/`movz` (23/40), and the two-store form that does branch (and even reproduces retail's exact `beq` polarity when written `!= 1`) then needs an extra `b` to join (15/40). Reverted. **Mirror image of `func_001FF4F8`**, where retail used `movn` and this compiler wouldn't produce it — so the conditional-move heuristics differ from retail's in *both* directions. Worth recognising early rather than iterating: if retail branches over a trivial value-select, or uses a cmov where plain C won't, that's this. |
| `func_0021DA98` | text | **matches** | `*(char**)((char*)arg0+0x34) = (D_0013D5CA != 0) ? D_001D0A50 : D_001D0A88; return 0;` — picks one of two fixed buffers by a byte flag and stores it into the object. Byte-exact first attempt with a plain ternary. Note retail *branches* here (with an explicit join `b`) rather than using a cmov, and this compiler agrees — selecting between two **addresses** (each needing its own `lui`/`addiu`) isn't cmov-able, so the cmov-vs-branch divergence documented for `func_0021EDD8` doesn't bite for address selects. Useful: this shape is reliably matchable. |
| `func_0021B108` | text | **close, not exact** (2/44) | Identical shape to `func_0021DA98` above (`(D_0015EF90 != 0) ? D_001D4B90 : D_001D4BC0` into `arg0+0x34`), on an `int` flag rather than a byte one. The only diff is which register holds the flag global's address: retail does `lui $2` / `lw $2,lo($2)` (loads into the same register), this compiler `lui $3` / `lw $2,lo($3)`. Notably retail itself is inconsistent between the two — `func_0021DA98` uses `lui $3`/`lbu $2` (matching this compiler exactly, hence that one being byte-exact) while this one reuses `$2`. So it's retail's allocator varying, not a rule we're failing to follow: the scratch-register-allocation-choice question. Kept as C per the tiny-isolated-diff precedent. |
| `func_0022F0F0` | text | **skipped, known sign-extension gap** | Would be `if (arg1 != 0) { *(int*)arg1 = arg0; if (arg0 == 0) { ((char*)arg1)[4] = 0; *(int*)((char*)arg1+0x18) = 0; *(int*)((char*)arg1+0x1C) = 0; } }` — but opens with `dsll32 $5,$5,0` / `dsra32 $5,$5,0`, the 32→64 sign-extension of the pointer parameter that is the `func_00112380`/`func_0022F090` open question. Not attempted; blocked before the interesting part. (It also has an unfilled `beqz` delay slot and does the first store in the *second* branch's delay slot so it runs either way — worth revisiting if the sign-extension question is ever solved.) **Update 2026-09-22: now exact** once the sign-extension question was solved. |
| `func_0021DA60` | text | **matches** | Copies 8 `int`s from `arg0+0x30` into the global array `D_00141FA0`, returns 0. Written as a `do { *dst++ = *src++; } while (--i >= 0);` with `i` starting at 7. Byte-exact **first attempt**, including retail's unfilled `nop` inside the loop body — worth noting since loops had been avoided as risky: a simple counted copy loop reproduced exactly, so loop shapes are not inherently a problem. |
| `func_00219E60` | text | **close, not exact** (23/44), reverted | Sets `D_001D5F70` (an object) field `0` to `0x2D` and fields `0xC`/`0x10`/`0x110` to 0, plus the separate global `D_0015F6E8 = 3`. Instruction count and size are right; the register assignment differs from the very first instruction (retail `lui $4`, this compiler `lui $5`) and cascades through. Also establishes that **the store-order rotation rule is base-pointer-scoped**: this function's stores go through two different bases, and the compiler reordered them non-rotationally (source `0x110, global, 0xC, 0x10, 0` → emitted `0, global, 0x10, 0x110, 0xC`); writing the source in retail's own emitted order changed nothing. **Update 2026-09-22: now exact.** |
| `func_0023CD30` | text | **matches** | `int *p = (int *)(arg0 + 0x50000); int d = p[2] - p[1]; if (d != 0) { *arg1 = (int)(arg0 + p[0]); } return d;` � same `+0x50000` page-offset shape as `func_0023CD10`/`func_0023CDF0` (see the big-constant-splitting technique). Byte-exact first attempt. |
| `func_00216EF0` | text | **matches** | Writes `-0x8000`/`0` pairs into `D_001517D0` at short-indices `0x2E`/`0x2F` (guarded by `arg0 != 0`) and `0x3C`/`0x3D`/`0x20`/`0x21` (unconditional). Two things were needed: the store-order rotation rule *per group* (source `0x2F,0x2E` and `0x3D,0x20,0x21,0x3C` to get retail's emitted `0x2E,0x2F` and `0x3C,0x3D,0x20,0x21`), and indexing the global **directly** rather than through a shared `short *p` local � see "Global-address materialization: direct indexing vs. a base-pointer local" below. Byte-exact. |
| `func_00222D70` | text | **close, not exact** (24/60) | `*(int *)((char *)arg0+0x34) = D_001D48A8[D_0015EE84 % 19]; return 0;`. Right size and shape including the real `divu` + trap guard; held entirely by the `%hi`-register-reuse allocator sub-case. Reverted. |
| `func_0021DB00` | text | **close, not exact** (15/48) | `D_0013E6A0 = (D_0015EEF0 * 8) / 10; return 0;`. Same story as `func_00222D70` � correct shape, held by `%hi`-register reuse plus the divisor `addiu`'s position. Reverted. **Update 2026-09-22: now exact.** |
| everything else in `core_text`/`text` | core_text, text | not started | Still `INCLUDE_ASM` stubs. For the current list and count see `progress/report.json` (this row used to carry a hand-written count, "~1532", which went stale). |

## A variable's SDA placement is per-translation-unit, and we have one file

**Update 2026-09-22: superseded by the next section.** The two forms
are one assembler macro, not two declarations. With `D_0015EFB0`
declared `MACRO_ADDR` (`src/game/menu.c`), `func_00209418` and all six
casualties it describes (`func_002094E0`, `func_00209698`,
`func_00209808`, `func_00209858`, `func_002098C8`, `func_00209918`) are
exact, and the monolithic `src/text.c` has since been split per
original source file anyway. The entry is kept for the record.

`D_0015EFB0` (0x15EFB0) is reached **two different ways in retail**:
via `$gp` in `func_00209418`, and via the non-SDA `lui`/`%hi` form in
the `func_00209xxx` cluster. Both cannot be true of one declaration, so
retail must have declared it differently in different source files —
which is ordinary C, but our single `src/text.c` cannot express it.

Declaring it SDA (`extern short`) gains `func_00209418` (0/44 exact) and
costs the six `func_00209xxx` near-misses, which become **4 bytes short**
each. They were already non-exact (blocked by the global-STORE
addressing question above), so nothing exact was lost — but a size
mismatch drifts everything after it, so those six are now stubbed with
their decoded semantics kept in comments.

Expect more of these collisions as the `$gp` harvest continues. When one
appears, prefer whichever declaration yields an exact match and stub the
casualties; do not keep a size-mismatched function.

## SOLVED: the one-instruction macro form — `MACRO_ADDR`, per variable

This used to be filed as "Open: global STORE addressing", with the
conclusion that neither `-msplit-addresses` nor `-mno-split-addresses`
reproduces retail's combination and that **any function that stores
directly to a non-SDA global is blocked**. That was wrong, and it had
been blocking a large category for several rounds.

Retail's "two addressing forms" are not two compiler settings. They are
**one assembler macro** (`lw $2,D` / `sw $2,D`), written by a compiler
that emitted the unsplit form for that variable, expanded by the
assembler according to where it lands:

| where the access lands | the assembler emits |
|---|---|
| ordinary load | `lui $2,%hi(D)` / `lw $2,%lo(D)($2)` — the address register reused as the destination |
| ordinary store | `lui $1,%hi(D)` / `sw $x,%lo(D)($1)` — through `$at` |
| a delay slot (only one instruction fits) | `$gp`-relative, e.g. `sw $x,-0x5D00($28)` |

`MACRO_ADDR` in `include/common.h` puts the symbol in `.sdata` so the
compiler emits the unsplit form, while its declared size stays over
`-G2` so the assembler does not route it through `$gp`. One declaration
therefore reproduces all three forms at once, which is exactly what
retail does with variables like `D_00161000`, `D_0015F718` and the
`draw.cpp` callback counts.

Consequences worth keeping:

- The recurring "`%hi` register reuse" residual **was never an allocator
  question**. `func_001F49B0` had been recorded as held by it for
  several rounds and matched immediately once its count was
  `MACRO_ADDR`.
- It works on an **array with an explicit size** too, not only scalars:
  `extern unsigned char D_0015EE58[4] MACRO_ADDR;` gives retail's
  one-register `lbu`. Without the section attribute an array compiles to
  the split `lui`/`%lo` form (and, being an array, is scheduled
  differently); without the explicit size over `-G2` a byte-sized scalar
  is small enough for the assembler to use `$gp`.
- **Hard limit, measured:** because a `MACRO_ADDR` access in a delay
  slot becomes `$gp`-relative, a symbol **outside the ±32KB small-data
  window cannot appear in a delay slot at all** — the link fails with
  `relocation truncated to fit: R_MIPS_GPREL16`. `func_0012D380` and
  `func_0012D448` load `D_001331D4`/`D_001331D6` across two delay slots
  (the `lui` in a branch's, the `lbu` in the following `b`'s), which is
  the compiler's *split* form and not a macro at all. For those, a plain
  incomplete-array extern gives retail's exact shape and the only
  residual is which register holds the address.
- `tools/check_macro_slots.py` runs on every object and fails the build
  if such an access lands where it cannot be expanded.

## SOLVED: tail calls — post-process the call-and-return

Retail forwards with a bare `j target` and no frame. GCC 2.95 has no
sibling-call optimisation — confirmed, not assumed:
`-foptimize-sibling-calls` is rejected by both SN sub-builds and `-O3`
does not help — so it compiles `return f(x);` to frame + `jal` + return.

`tools/fix_tail_calls.py` runs on the `-S` output and drops the prologue,
turns `jal` into `j`, and drops the epilogue. The body and delay slot
already match retail, because GCC schedules argument setup into the delay
slot exactly as retail does, so nothing is moved or synthesised.

Proven byte-exact on four: `func_0011DD98` (0/8), `func_0012DA28` (0/8),
`func_0012CC80` (0/12), `func_0012CC60` (0/12). **55 functions moved from
blocked to risky.**

**Two mistakes this cost, both silent, both worth knowing:**

1. **Shape-matching our own output is wrong and destructive.** The first
   version fired on any call-and-return in *our* assembly. But retail's
   compiler had no sibcall either, so `return f(x);` is call-and-return on
   **both** sides — 8 already-matching functions were rewritten into size
   mismatches and the drift took **272 exact down to 217**. The rewrite is
   now gated on `tools/tail_call_functions.txt`, the 99 functions whose
   *retail* form is a bare tail jump. Retail is the only valid authority.
2. **Do not emit a `nop` for the emptied delay slot.** The assembler runs
   under `.set reorder` and fills it; an explicit `nop` becomes a third
   instruction, giving 12 bytes against retail's 8. Retail's `nop` there
   is the assembler's, not the compiler's.

Ranked *risky* rather than candidate because the rewriter only fires on a
strict shape — of the 99, **47 touch `$sp`** (they keep a real frame) and
**22 contain a second call**, so a large share will not convert. Those
need something else.

## SOLVED: the `$gp` small-data question — the answer was `-G2`

The largest remaining blocker (~244 `text` functions using `$gp`, plus
more elsewhere) is resolved. The build now uses **`-G2`**, not `-G0`.

| `-G` | FP constants | small globals via `$gp` |
|---|---|---|
| `-G0` | inline | never — so no `$gp` function can match |
| `-G1`..`-G3` | **inline** | **yes** |
| `-G4`+ | pooled to `.lit4` | yes |

Retail does both, so its threshold is 1..3. `-G4`+ is not merely wrong
but unbuildable: `.lit4` is `$gp`-addressed and the SDA window is full.

**Proven:** `func_001F6598` (`sw $2, -0x7764($28)`) is byte-exact 0/12,
the first `$gp` function matched in this project. The switch cost
nothing — 195 exact, zero regressions.

**The trick you need:** placement follows an extern's *declared* size,
and `-G2` admits only 1-2 byte variables, while retail reaches plenty of
4-byte globals via `$gp`. Declare small, cast at use:

```c
extern short D_0015F59C;        /* declared small -> lands in SDA */
*(int *)&D_0015F59C = 1;        /* accessed as the word it really is */
```

`NOT_SDA` in `include/common.h` is the opposite lever, for the ~60
variables that must stay out because they live beyond the window's
±32KB reach.

Full detail, including two confidently-worded conclusions that turned
out to be wrong (a literal pool needing placement; retail being built
from multiple translation units with different `-G`), is in
`notes/gp-investigation.md`.

## SOLVED: the `dsll32`/`dsra32` sign-extension question

**It's a 64-bit `long` parameter narrowed to `int` at the use site.**
This was filed as an unsolved open question from the very first
decompiled function (`func_00112380`) onward, repeatedly described as a
codegen artifact no source shape reproduced. That was wrong. The pair is
exactly what this compiler emits for a parameter declared `long`
(64-bit here) and narrowed to `int` where it is used:

```c
void func_00217830(int arg0, long arg1) {
    short *p = (short *)(int)arg1;   /* <- emits dsll32/dsra32 */
    ...
}
```

Verified byte-exact (0/48) on `func_00217830`, with the compiler's own
exit code confirmed 0 before trusting the result.

**This reclaims a whole category.** Every entry filed under the old
sign-extension question is worth retrying with a `long` parameter
narrowed at the use site. Known candidates: `func_00112380` (the
original — and the root cause of the "known systemic artifact" 8-byte
drift noted at the top of this file, so fixing it may re-align many
downstream `jal` targets and turn several "matches (mod. known drift)"
entries into clean exact matches), `func_0022F090`, and `func_0022F0F0`
(whose entry explicitly says "worth revisiting if the sign-extension
question is ever solved").

**Second form: a 64-bit RETURN value narrowed to `int`.** The same
narrowing fires on the other side of a call, and this one has a wrinkle
worth knowing. If a callee is prototyped `long` and its result is
assigned to an `int`, the pair appears at the call site:

```c
extern long f(void);
int r = f();          /* <- emits dsll32/dsra32 on the result */
```

Retail carries this on functions whose callee is defined in the *same
segment* as `int`, which looks like a contradiction until you remember
retail had many translation units per segment and nothing cross-checks a
prototype against a definition at link time. We merge a whole segment
into one file, so the two views have to be reconciled explicitly — use
an asm-labelled alias declaration, which this compiler supports and
which still emits a plain `jal` to the real symbol:

```c
extern long func_00116108_wide(int *, void *, void *, void *)
    __asm__("func_00116108");
```

The callee keeps its `int` definition and stays byte-exact. Do not try
to widen the definition instead: `word_mode` is SI here even though
`long` is DI, so any 64-bit *compare* inside it (`r == -1`) is rejected
outright with `unsupported wide integer operation`. Ordered 64-bit
compares are rejected everywhere; an equality compare against a
constant compiles but costs a `dli`, which is not what retail emits, so
keep the comparison operand `int`-typed. Whether the pair lands on the
result, or is sunk to a single 32-bit store as in `func_001163A0`,
follows from whether the local itself is declared `long` or `int` —
declare it `long` when retail compares and returns the *raw* call result
and narrows only at one store.

Verified byte-exact on `func_001162B8` and `func_001163A0` (reclaiming a
banked revert whose note blamed "a sign-extension this compiler does not
emit for an int-returning callee" — a right observation with the wrong
conclusion).

Broader lesson, now twice-confirmed: entries in this file asserting
"tried everything" have twice proven wrong — this one, and the
store-order case where a note claimed "all four" orderings were tried
for what is actually six permutations. Re-test a blocked entry when you
have a new technique rather than trusting its note.

## Open: core_text prologue scheduling — we schedule against `sq`, retail didn't

A recurring residual in `core_text` is a same-size, all-operands-match
near-miss where the only difference is *where the callee-saved spills sit*
relative to the surrounding code — retail emits `sd $16` and `sd $31` back
to back and fills the first call's delay slot with an argument, while we
sink one save into the delay slot.

The cause is the build pipeline, not the C. `tools/fix_core_spills.py`
rewrites `sq`/`lq` to `sd`/`ld` *after* the compiler has already run, so
the instruction scheduler made its decisions believing those were 16-byte
stores. Retail's compiler emitted 8-byte stores natively and scheduled
accordingly.

**Evidence, and a compiler that demonstrates it.** The SN mirror contains
more than the two 2.95.3 sub-builds we use. `toolchain/sn-prodg-24/local/
sce/ee/gcc/bin/ee-gcc.exe` is Sony's own **gcc 2.9-ee-991111b/r4**, and it
emits `sd $16,16($sp); sd $31,32($sp)` natively, in retail's ascending slot
layout, with no post-processing at all. Rebuilding `core_text.c` with it
unchanged moves exactly the functions this section is about:

| function | 2.95.3 + `fix_core_spills` | gcc 2.9-ee, same C |
|---|---|---|
| `func_00116320` | 8/128 | **1/128** |
| `func_0011DC50` | 10/104 | 4/104 |
| `func_0011B710` | 8/92 | 4/92 |

(The `func_001162B8` / `func_001163A0` pair stays 0/0 under both.)

**It is not a drop-in replacement, and it was not adopted.** A whole-segment
rebuild scores 228 exact / 8 size-mismatched against 286 exact / 0
size-mismatched, and most of that gap is drift: gcc 2.9-ee performs tail-call
optimisation *natively*, and does it in eight places where retail did not
(`func_00116408`, `func_0011ABC8`, `func_0011AC08`, `func_0011CCB0`,
`func_0011D4A0`, `func_001275A0`, `func_0012C058`, `func_0012EC40` — each
one 20-28 bytes short, a whole frame gone). `func_0011D4A0` is the clearest:
its C is byte-exact at 64 bytes under 2.95.3, and 2.9-ee turns both arms of
its `if` into tail jumps for 44. So retail is not straightforwardly 2.9-ee
either — retail tail-calls in some places and not others.

Worth noting the corollary: 2.9-ee emitting tail calls natively is the first
independent explanation of *why* retail's `core_text` contains bare tail
jumps at all, which is the thing `tools/fix_tail_calls.py` exists to
reproduce by deletion.

Next step for whoever picks this up: the eight size mismatches are the whole
obstacle, and they are all the same shape. If they can be suppressed (a flag,
or a source form 2.9-ee will not tail-call), re-run the sweep before
concluding anything — the current 228 number is drift-poisoned and says
nothing about how many functions 2.9-ee would actually match.

**Update (2026-09-23): the source form exists, and core files are moving.**
2.9-ee tail-calls a void function that ends in a call, but not
`int f() { return g(); }`. So a retail call-and-return means the function
returned its callee's value, and a retail tail jump means it was void.
`func_00116408` (newlib's `__sclose`) is the first one: it returns the
close result, as newlib's does. Compiling every core file whole under
both compilers (each function's C unchanged) splits them like this:

- **989snd** is SN: 25/25 under 2.95.3, 3/25 under 2.9-ee (it also
  spills with `sq`).
- **Better under 2.9-ee, nothing lost; now built with it** (the `ee29`
  column in `config/core_text.objects`): `00113A70` (newlib's
  `std`/`__sinit`), `00116D30` (strtol), `001162B8` (stdio's
  `__sread`/`__swrite`/`__sseek`/`__sclose`), `0011DBE8`, `0011DDD0`,
  `0012A2F0` (libmpeg's bitstream reader). Seven functions went exact,
  among them the three this section's table lists.
- **Mixed**: under 2.9-ee, `00123168`, `0012CC90`, `00119D88`,
  `0012AC80` and `00125630` gain some functions and lose others. The
  losses are mostly return-type tail calls (`func_0011CCB0`,
  `func_0011D4A0`, `func_001275A0`, `func_0011ABC8`/`func_0011AC08`,
  `func_0012C058`) plus a few 4-8 byte size changes, and `00121750`,
  `001208E8` and `00116FA0` only lose. Their C was tuned against 2.95.3,
  so the losses may be C to rework, not proof of the compiler.

**Settled (2026-09-24, `1a48d5d`).** The losses were C to rework. A
migration pass checked every remaining core object against Sony's SDK
archives (452 functions match an archive member exactly) and reworked
the C that had been tuned for 2.95.3, mostly callees that return a
value. 38 more objects moved to `ee29` and 46 functions went exact. Every
core object is now 2.9-ee except 989snd, `boot`, `permcb`, `wad` (game
code) and `crt0` (assembly), all SN; `001207B8` (libsn's `vu.o`) is
undecided.

Neither compiler puts a final truncation's `dsra` in the return slot, so
`tools/fix_trunc_slot.py` runs in the 2.9-ee pipeline too.

## libgcc is 2.9-ee: 0x11FC08-0x1206A0 rebuilt from GCC's source

**The section above asked whether any of retail is 2.9-ee. Part of it is.**
`core_text` 0x11FC08-0x1206A0 is not game code. It is libgcc's soft-float
library `fp-bit.c`, and Sony's **gcc 2.9-ee-991111** built it. The
unmodified GCC 2.95.3 source for nine `L_*_df` modules compiles under that
compiler to retail byte for byte. Across 678 words nothing differs, and that
includes every resolved relocation. No rewriters are involved.

| retail | function | earlier status |
|---|---|---|
| 0x11FC08 | `_fpadd_parts` | stub |
| 0x11FE48 | `__adddf3` | close 18/88 ("prologue save order") |
| 0x11FEA0 | `__subdf3` | close ("same known residual") |
| 0x11FF08 | `__muldf3` | stub |
| 0x1201B0 | `__divdf3` | stub |
| 0x120318 | `__fpcmp_parts_d` | stub |
| 0x120430 | `__cmpdf2` | close 10/76 ("prologue save order") |
| 0x120480 | `__floatsidf` | stub |
| 0x120538 | `__fixdfsi` | stub |
| 0x1205D0 | `dptoul` (float_to_usi) | exact under 2.95.3, but only with a flipped `if` |
| 0x120670 | `__make_dp` | exact |

How it was found: `func_001205D0`'s shape matched `float_to_usi`. That
pointed to fp-bit, and fp-bit's own `add` still gave exactly the recorded
18/88 under 2.95.3. So the source was not the problem. Compiling the same
`add` with every EE `cc1` in the toolchain mirrors settled it: only
2.9-ee emits retail's `sd $16` before `sd $31` with `move $5,$sp` in the
first delay slot. Under 2.9-ee the *verbatim* `> 60` spelling of
float_to_usi matches, where 2.95.3 needed it inverted. That confirms the
compiler rather than a coincidence of spelling.

**Build shape.** One object per `L_*` module, like `libgcc.a`'s members.
Each object has 8-byte `.text` alignment, and that alone reproduces
retail's 4-byte gaps between modules. (core_text has since been split
into one file per retail object, see "core_text is ~50 objects" below.) `rac1.ld.sh` maps the
`func_` names to the libgcc names both ways. `tools/libgcc_units.py` is
the shared module list for the tools.

**Two lessons worth keeping.**
- **A family-wide residual may point to a different compiler, not a
  missing lever.** Three near-misses carried the note "treat the rest of
  this family as the same known residual". That was accurate. It was
  also a sign that the family had not been built by our compiler.
- **Library code: look for the real source first.** libgcc and the
  EE/IOP SDK were not written for the game. Where the source still
  exists, compiling it is cheaper and more faithful than decoding it.

**Extended to libgcc2 (0x11DFE8).** `__divdi3`, `__muldi3`,
`__floatdidf` and `__fixunsdfdi` match from the **unmodified** `libgcc2.c`
and `longlong.h` of GCC trunk 1999-11-02, the revision just before the
2.9-ee-991111 snapshot. Two things were needed, and neither is obvious:

- **Build through the 2.9-ee driver, not `cc1`.** Calling `cc1` directly
  drops the driver's target predefines (`__mips__`, `__R5900__`, ...).
  `longlong.h` selects its MIPS `umul_ppmm` from those, and without them
  `__divdi3` comes out 0x50 bytes too long. With the driver it is 0/443.
  fp-bit happened to match either way, but it now goes through the driver
  as well.
- **Each division module has its own static `__clz_tab`** in core_rdata.
  To own `__divdi3`'s table, the retail data blob is cut around
  D_00152B18 (`tools/split_data_s.py`).

Also learned: Sony's compiler emits soft-float libcalls under GOFAST
names (`dpadd`, `dpcmp`, `litodp`...). Linker fill between these modules
is `0xCDCDCDCD`, not zero, and splat split it out as 4-byte "functions"
(`func_0011E6D4`, `func_0011E7C4`). Whole-image check: 2478 words across
0x11DFE8-0x1206A0, 0 differ.

**Still open.** `__moddi3`, `__udivdi3`, `__umoddi3` (one extra stack
local in retail), `pack_d` (0x11FA38), `unpack_d` (0x11FB68) and `unpack_f`
(0x1206B0) differ from 2.95.3's revision, and so does a gap past
0x1206A0. They look like a different fp-bit revision.

**Update 2026-09-23: Sony's prebuilt `libgcc.a` is an exact reference.**
The `libgcc.a` shipped in every EE compiler directory of the mirrors
matches retail byte for byte for these modules (`tools/libgcc_ref.py`).
Against it, `pack_d`/`unpack_d` were not a different revision at all:
`__pack_d` needs `-DFLOAT_BIT_ORDER_MISMATCH` (GCC's MIPS makefile
fragments define it for little-endian), and `__unpack_d`/`__unpack_f`
need `-DNO_DENORMALS` plus that option's one-hunk implementation, which
trunk only got on 2000-03-16 (from Cygnus) and which is backported into
`src/libgcc/fp-bit.c`. Both modules are exact now. The three libgcc2 stubs
compile to Sony's instructions but not Sony's frame (unused stack slots),
identically across every 1999 revision and all flags tried: a compiler
build difference, recorded in `src/libgcc/README.md`. `__extendsfdf2`
(0x120778) already matches from this source. The four large leaf
functions at 0x11DFE8-0x11F4F8 are probably libgcc2's 64-bit
division family, which is the next thing to try under 2.9-ee. The
functions this section says 2.9-ee improved (`func_00116320`,
`func_0011DC50`, `func_0011B710`) are worth re-checking for the same
reason: they may be library or SDK code as well.

## Sony's SDK archives match retail: names for core's library code

**Found 2026-09-24 (batch S).** The SDK libraries in the toolchain mirror,
`toolchain/sn-prodg-24/local/sce/ee/lib/*.a`, are what retail linked,
the way `libgcc.a` is for libgcc: every one of batch S's 14 functions
matches a member of `libmc.a`, `libdbc.a`, `libpad2.a` or `libmpeg.a`
byte for byte (relocations masked). `tools/libgcc_ref.py`'s `members()`
reads any of them. The members have no type information, but their
`.symtab`/`.mdebug` give real names for functions and static data, and
their undefined symbols name the callees:

- **libmc.o** (0x1236F0): sceMcInit, sceMcOpen (0x1238B0), sceMcMkdir,
  sceMcClose, sceMcSeek, mceIntrReadFixAlign, sceMcRead (0x123C30),
  sceMcWrite, mcHearAlarm (0x123EC0), mcDelayThread, sceMcSync
  (0x123F30), mceGetInfoApdx, sceMcGetInfo, sceMcDelete, sceMcUnformat.
  Data: mcClientID D_00159B00, typeAddr/freeAddr/formAddr
  D_00159B28/2C/30, buffFileInfo D_00159B40, sifParamOrd D_00159B80,
  sifParamFname D_00159BB0, sifParamNext D_0015A000, currentDir
  D_0015A0C0, retval D_0015B0C0, mcRunCmdNo D_00132EA8, semaidRegFunc
  D_00132EAC.
- **libdbc.o** (0x1245F8): sceDbcGetModVersion, sceDbcInit (0x124650),
  sceDbcSetWorkAddr, sceDbcCreateSocket (0x124858), sceDbcGetDepNumber,
  sceDbcReceiveData (0x124A70), DPRINT (0x124B60). Data: cd_base
  D_0015B108, cd_send_data2 D_0015B130, sif_buffer D_0015B180,
  sif_dma_buf D_0015B580, link_state_table D_0015B600.
- **libpad2.o** (0x124B88): scePad2Init, scePad2CreateSocket,
  scePad2GetButtonProfile (0x124DF0), scePad2GetState, scePad2LinkDriver
  (0x125020), scePad2GetSide, scePad2CheckDma, scePad2SetButtonOrder
  (0x125160). Data: pad2_info D_0015B640 (16 x 0x330, the Ent330
  table), isInit D_00132ED0.
- **libmpeg** (0x12A2F0): _doCSC, _ch3dmaCSC, _doCSC2, _ch4dma,
  _csc_storeRefImage; bit.o (0x12AA70): _sysbitInit, _sysbitNext,
  _sysbitFlush, _sysbitGet, _sysbitMarker, _sysbitJump, _sysbitPtr.
- **Kernel and SIF callees**: func_00118CC0 WaitSema, func_00118C90
  SignalSema, func_00118B20 SetAlarm, func_00118BE0 GetThreadId,
  func_00118C00 SleepThread, func_00118D80 FlushCache, func_00119288
  SyncDCache, func_0011D960 DIntr, func_0011D9A8 EIntr, func_00118AC0
  AddDmacHandler2, func_00118AD0 RemoveDmacHandler, func_00119460
  EnableDmac, func_001193F8 DisableDmac, func_0011AD70
  sceSifWriteBackDCache, func_0011B4C8 sceSifCallRpc, func_0011B2F8
  sceSifBindRpc, func_0011AE20 sceSifInitRpc, func_0011A6C8 scePrintf,
  func_001138B8 exit, func_001153FC memset, func_00116B00 strncpy.

What follows from it:

- **One of our objects can be several of theirs.** `src/core/001236F0.c`
  is libmc.o, libdbc.o and libpad2.o back to back; no linker fill marked
  the joins. The archive members' extents are the true object
  boundaries.
- **The compiler question is settled per member.** A function that
  matches an SDK member is 2.9-ee code.
- **Near-misses get an exact reference to diff against**, with no link:
  scePad2Init (func_00124B88), scePad2LinkDriver (func_00125020) and
  _sysbitInit (func_0012AA70) are all in these archives.
- **Next:** generalise `tools/libgcc_ref.py` to every archive in the
  mirror (and newlib's libc.a), map every core_text function to its
  member and name, and put the names on the stubs.

## Retail's linker dead-stripped unreferenced functions

**The rule (2026-09-23): an unreferenced function lost its first
`floor(size/8)*8` bytes.** A function of size 0 mod 8 vanished; one of
size 4 mod 8 left its last word, the delay slot of its final jump, plus
the alignment nop. Proven on three libgcc objects against Sony's prebuilt
`libgcc.a` (details in `src/libgcc/README.md`): L__main, dp-bit.o
(`__negdf2` gone, `dptofp` left one word) and fp-bit.o, where thirteen
stripped functions left exactly the eight words their sizes predict, in
order. What follows is how it was first found, on L__main.

Sony's prebuilt
`__main.o` is `__do_global_dtors`, `__do_global_ctors`, `__main`. Retail's
copy at 0x11DF10 is the same object with its first 80 bytes gone:
everything of `__do_global_dtors` up to and including its `jr $ra`,
leaving the delay-slot `addiu $sp,$sp,0x20` and the alignment `nop`, then
the other two functions byte for byte. Nothing references
`__do_global_dtors` in this configuration (the constructors do not
register it), so the linker removed it, but measured it one instruction
short. `tools/strip_dead.py` reproduces the cut on compiler output, and
L__main now builds from GCC's source to an exact match, as do the two
soft-float objects.

That is the same shape as two things this file has long filed as
unexplained:

- the **orphan epilogue fragments** (`addiu $sp,$sp,N; nop`) at the head
  of many core_text objects, and
- the **one-instruction "fallthrough fragments"** throughout both
  segments (a lone `addiu $sp`, `lw` or `sw` with no `jr $31`), including
  runs of them like `func_001E94C8`'s four `addiu`/`nop` pairs.

Each is consistent with the delay slot of a stripped, unreferenced
function. Proven so far for libgcc's three objects only. For the others it means the
original source had a function there, and matching it from C means
writing that function and stripping it the same way, which needs the
body (the delay slot alone does not say what the function did). Until
someone does that, these fragments are not failed decompilation: they
are, as far as retail is concerned, finished.

## core_text is ~50 objects: split at retail's own linker fill

Retail's linker filled the gaps between objects with `0xCDCDCDCD`.
core_text has **49** such runs, and every one of them ends on an 8-byte
boundary, as an object start must. `text` has **none**, so its object
boundaries have to come from other evidence (see below).

core_text is now one C file per object: `src/core/<NAME>.c` for game and
SDK code, plus the libgcc objects. A file is named by its start address
until its real source is identified, then renamed. `989snd.c` is the first
renamed one. The link order and each object's start address live in one
place, `config/core_text.objects`, which Makefile.sn, rac1.ld.sh and the
tools all read. Renaming changes only the name column. 0x12DB18, where the compiler sub-build switches from `sd` to
`sq` spills, is a boundary too.

What it took, all verified:
- **The fill always ends the previous object.** A first attempt started
  objects at the fill when splat had put the fill at the head of a
  function. That ran core_text 0x48 bytes long, because those objects
  are 8-byte aligned and cannot start on the fill.
- **19 object starts were buried inside splat functions.** Some are
  orphan epilogue fragments (`addiu $sp,$sp,N; nop`) left at an object's
  head, followed by the real first function. They are declared in
  `config/symbol_addrs.txt`. That also made splat resolve one data
  pointer (`.word func_0011DD68`), independent confirmation that an entry
  point starts there.
- **Every file keeps the compile context its functions had.** It gets the
  declarations that preceded it in the old file, plus extern prototypes
  for functions that were defined earlier. A missing prototype is not
  harmless: without one, `func_0012C2F8` changed 6 words, because its
  callee `func_00127378` was implicitly `int`. Old-style (K&R)
  definitions stay unprototyped on purpose.
- **The libgcc2 `L__main` module starts at 0x11DF10**, not 0x11DF0C.
  It is `src/libgcc/nonmatching_0011DF10.c`.

Result: every loadable byte (core_text, core_data, core_rdata, text) is
identical to the build before the split, still 364 exact, and every
function is at its retail address.

**The core_text tail, corrected.** An earlier note here said the object at
0x12DB18 is 989snd.c. That was wrong. crt0 calls `func_0012DB18` as
**main**, so 0x12DB18 is boot.cpp's `main`, and splat had merged the start
of 989snd into it. The ELF entry point (0x12D868 = `_start`) gives the
NTSC-to-PAL shift for this stretch (+0x140). With it, the NTSC
split maps onto code boundaries PAL confirms:

| object | PAL | evidence |
|---|---|---|
| crt0 | 0x12D868 | e_entry; `padduw` register clearing after pad nops |
| boot.cpp | 0x12DA38 | right after `_exit`'s syscall; holds ParseBin and main (0x12DB18) |
| 989snd.c | 0x12DB68 | prologue after main's loop and pad; the 989snd.c path string is used from 0x12DBE0 |
| permcb.cpp | 0x12F308 | right after `jr $ra` and pad |
| wad.cpp | 0x12F348 | right after `jr $ra` |

`func_0012DB68` is declared in `config/symbol_addrs.txt`. The sq/sd switch
at 0x12DB18 therefore falls at `main`, inside boot.cpp, not at an object
boundary. `fix_core_spills.py` keys on the address, so the build is
unaffected. Rebuilt byte-identical, still 364 exact.

**`text` is split too (58 files, named after the originals).** Evidence and method:
- The NTSC decomp splits NTSC
  `text` into the original source files (`hud.cpp`, `camera.cpp`,
  `mobyfunc.cpp`, `movie/*.cpp`, plus handwritten asm modules such as
  `mobyproc`), and lists each file's functions in order.
- PAL shifts against NTSC, and not uniformly. Instead, the NTSC and PAL
  function-size sequences were aligned: 710 of 813 NTSC functions (87%)
  match. 37 boundaries fall on a matched function. 15 more have the same
  shift on both sides. 5 have one candidate that is a real function
  start. Every candidate was checked in the PAL code: a return right
  before it, a function start right at it.
- `skyfunc` was the one exception. The shift put it mid-function, and the
  real start is 0x22BEB0, which has the same orphan-epilogue head as the
  core_text object starts.
- It agrees with independent evidence. The functions that reference the
  `hud.cpp` and `loaders.cpp` strings lie inside `hud` and `loaders`.
- Top-level `__asm__(...)` padding in the old `text.c` is code, not a
  declaration. Copying it into later files' inherited context added bytes
  to every object after it. Also, a plain extern that is repeated later in
  a file is still needed where it is first used: dropping it changed 2
  words of `func_0020DFF8`.
- Result: every loadable byte is identical to the unsplit build, still 364
  exact. Files stay `.c` and compile as C for now, since the language
  switch alone was measured not to be a lever.

Earlier evidence for `text`, kept for reference:
- **Source-file strings in `.lit`:** `hud.cpp` (0x15F7B8) is used only by
  `func_001FF6B8`, and `loaders.cpp` (0x15FC70) only by `func_002032D0`.
  `map.cpp` (0x15FE28) has no direct reference.
- **`.lit` order tracks code order.** Take data items used only by
  functions within 0x2000 of each other, which are likely a TU's own
  statics. Of 203 such items, 169 (83%) are laid out in the same order
  as their functions. The breaks follow one pattern (0x160xxx-0x161xxx,
  then back to 0x15Fxxx), which suggests two interleaved pools, strings
  and numeric literals, each in TU order. Separating the pools should
  turn this into usable boundaries.
- **`.data` is noisier** (214 of 315 in order), because shared globals
  dominate it.
- Not done: splitting `text` on this. A wrong boundary would still build
  byte-identically, so nothing would catch it. Boundaries there need
  evidence strong enough to stand on its own.

## SOLVED: the short-loop erratum -- retail's text was assembled by SN's ps2eeas

**Update 2026-09-23.** The mirrors carry a second EE assembler: SN
Systems' own `ps2eeas` (`ee/bin/Ps2EeAs.exe`), not the GNU `ee-as` the
compiler driver calls. It works around the R5900 short-loop erratum
itself. Measured on it directly: every backward branch whose loop (target
through branch) is shorter than six instructions gets nops inserted right
before the branch until it is exactly six, in reorder and noreorder code,
branch-likely included. GNU `ee-as` never does this.

Retail's text segment has no unpadded short loop at all, while every
unpadded one in the image is in core_text or libgcc (16 objects). So text
was assembled by ps2eeas and core_text was not.

*Correction, same day:* core_text's padded loops come from GNU as after
all. The compiler driver calls its own `ee/bin/as.exe`, which pads short
loops that contain no call. The standalone `bin/ee-as.exe`, with the same
version string (2.9-ee-991111b), pads nothing, and it was that binary
the first probes used. ps2eeas pads loops with calls as well. So
core_text was assembled by the driver's as.exe, which is what this build
uses, and its call-free short loops come out padded by themselves.
`tools/ps2eeas_nops.py` measures loops in a first assembly by that same
as.exe, so it only adds what ps2eeas adds beyond it: the padding of loops
that contain a call. A padded tight loop in a core_text stub is therefore
no blocker.

*The driver's as.exe measures its own way* (batch M): it counts from the
target label to the first jump after it, not to the branch. So a
backward branch that is not a loop, a cross-jump back into a short block
that returns, gets padded where ps2eeas left it alone (3 nops in
func_00209188 and func_00209750). `tools/ps2eeas_nops.py` spots a branch
GNU as padded beyond ps2eeas's rule and writes it as a `.word` with the
offset computed from its label, which the assembler cannot pad. The
standalone `ee-as.exe` is no way out: it adds MIPS-style hazard nops
after every `mtc1` and FP compare, and a text build with it drifted
(529 exact, 3 size mismatches). A related residual remains: retail's compiler sometimes left
a short loop's branch delay slot empty where ours fills it, which puts
the moved instruction on the other side of the padding (func_001232A8).
It fills about half of these slots in both segments, and what decides
it is not known.

ps2eeas cannot simply replace `ee-as` here: it recurses without end on
some of the retail stubs INCLUDE_ASM feeds it. `tools/ps2eeas_nops.py` (then named `fix_short_loops.py`)
reproduces the padding instead, on compiled game code only: it measures
each loop in a first assembly (so macro expansion and delay slots are
never guessed) and puts the nops before the branch, inside the compiler's
own noreorder block when there is one. First results: `func_00225548` and
`func_002268F0`, both filed below as blocked by exactly this, are exact.
`tools/rank_candidates.py` no longer blocks the erratum in text.

Using ps2eeas for the whole text segment was measured too, and it is
worse. To get it to assemble at all, every text stub was reduced to raw
`.word`s. Three things crash it: a stub whose `lui %hi` feeds two distant
`%lo` uses, `labels.inc`'s default-parameter macros, and `.type`/`.size`
next to `.ent`/`.end`. Once assembled, the build was 385 exact instead of
503, with 11 size mismatches. The cause is small data: this ps2eeas has
no `-G` expansion, so every `$gp`-relative access came out as a `lui`/`lw`
pair and the layout shifted after it. Probes also found two scheduling
differences from GNU `ee-as`:

- after `mfc1`, GNU inserts a nop before the result is used (reorder
  mode) and ps2eeas does not;
- between `c.cond.s` and `bc1t`, ps2eeas inserts a nop even in noreorder
  code and GNU does not.

Switching assemblers would first need the small-data expansion
reimplemented. Until then `tools/ps2eeas_nops.py` is the pipeline.

The original analysis follows.

### (Superseded) The short-loop erratum is a toolchain blocker, measured three ways

Retail pads loops shorter than six instructions with a nop between the
store and the backward branch (the R5900 mispredicts them). 52 stubs are
blocked on it. Working `func_00225548` down to exactly that residual --
everything else, registers included, matched -- three ways to produce the
padding were measured, and all three fail:

1. **The compiler never emits it.** The generated `.s` has no nop between
   `sw` and `bgez`, and neither SN `cc1` (2.95.3, 2.9-ee) has an r5900
   erratum flag: scanning both binaries finds only optimisation strings
   (`mulsi3_mult3_r5900` and friends).
2. **The assembler only removes nops.** `ee-as --help` offers `-O` (remove
   unneeded NOPs) and `-g2` (keep them). There is no insert option.
3. **`__asm__("nop")` in the loop body does not append one.** It perturbs
   the scheduler instead -- the `jal` delay slot stops being filled --
   taking the function from 227 to 447 differing words.

So this class needs an assembler that inserts the padding, not a different
C spelling. The decoded source for `func_00225548` is kept above its stub
in `src/game/pause.c`, ready if that ever changes.

## Real function names (259), from the NTSC decomp

`config/symbol_names.txt` maps 259 of the 393 names in the NTSC decomp's
`symbols.txt` onto this PAL build. Each comes with its evidence, and only
confident mappings are included:
- **text:** the NTSC function sits on a function-size-aligned match (the
  same alignment that placed the file boundaries);
- **core tail:** the entry-point shift (+0x140) lands on a PAL function
  start (`_start`, `_exit`, `ParseBin`, `main`, the `snd_*` API);
- `__main`, which libgcc2 matches byte for byte.

Names are demangled with the SN toolchain's `ee-c__filt` (GCC 2.x ABI),
e.g. `Hud_HeapAlloc__FUiPcT1i` -> `Hud_HeapAlloc(unsigned int, char *, char *, int)`.
In `src/` they appear as comments: trailing on stubs, and on the line above
a decompiled function. `func_<ADDR>` stays the symbol name, because the
build and every tool read the address from it. The build is byte-identical
with the comments in place.

2026-09-30: these names, and those that the NTSC decomp, Lombyte and ReRAC have added
since, are now also usable in C: `config/names.tsv` and the generated
`include/names.h` (macros onto the address names), applied to the bodies
of every decompiled function. See docs/NAMES.md.

Tool fix found on the way: `rank_candidates.py` looked for documented
reverts in `src/*.c` only. After the split those files are in
subdirectories, so it silently found none, and 6 already-attempted stubs
came back as candidates. It now searches recursively. It also ignores a
trailing comment on the previous stub's line. Without that, the name
annotations alone would have hidden 131 stubs as "already attempted".

## The game is C++, and the language switch alone changes nothing

Retail's strings name its own source files: `hud.cpp`, `loaders.cpp`,
`map.cpp`, and one `snd.c`. There are no RTTI or exception symbols, so it
was built without them. The `lvl_*vtbl` sections are consistent with that.

**Measured:** `src/text.c` compiled as C++ gives 239 exact against 240 as
C, at object level with relocations masked, and all 25 near-misses are
byte-for-byte unchanged. The setup was cc1plus 2.95.3 v1.14, the whole
file wrapped in `extern "C"`, and `()` prototypes rewritten to `(...)`.
Switching the build to C++ is therefore not a missing lever by itself.

Still open, and cheap to try on store-order and allocator near-misses:
C++ *features* that change codegen. decomp.wiki documents a GCC 2.9 PS2
case where turning an `int` field into `bool` fixed a load/store order.
The end goal is readable C++ anyway, so C++ source costs nothing.

See `docs/WORKFLOW.md` for the full review against decomp.wiki, including
what was ruled out (`.lit4` float-literal nops: retail has none).

## Open toolchain questions

**RESOLVED (claim was wrong) — 64-bit shift by a non-multiple-of-32
constant.** The entry further down claims this is a hard `cc1` error for
*any* such shift, and that it blocks `func_0023CFF0`. Both parts are
wrong: **`func_0023CFF0` now matches byte-exact (0/36), `<< 28` and
all.** Shifts by 4/8/16/28 on a `long` all compile fine. Round 16's
`unsupported wide integer operation` error was genuinely observed, so
some narrower trigger exists (most likely `long long` rather than
`long`, or a specific expression shape) — but it is far narrower than
documented and does not block plain `long` shifts. **Use `long`, not
`long long`, and just try the shift.** The stale entry below should be
read as historical, not as guidance.

**SOLVED — `sq`/`lq` was never a flag: retail's two segments were built
by two different compilers, and we already have both.** This was the
dominant blocker for 23 rounds. Final answer:

| Retail segment | s-reg spills via `sq` | via `sd` | Matching sub-build |
|---|---|---|---|
| `text` | **428** | 34 | GCC 2.95.3 **SN BUILD v1.14** |
| `core_text` | 14 | **234** | GCC 2.95.3 **SN BUILD v1.36** |

Plus 520 `text` functions save `$ra` via `sq`, which v1.36 never emits.
Given identical input and `-O2 -G0`, the *only* codegen difference
between the sub-builds is the spill pair — every other instruction
agrees, verified at byte level.

*(Superseded twice since: `core_text` moved to v1.14 plus
`tools/fix_core_spills.py`, see the RESOLVED entry below, and the two
monolithic files are now split into `src/core/` and `src/game/`.)*
**`Makefile.sn` now compiles per segment**: `src/core_text.c` with v1.36
(`toolchain/sn-prodg-3.01/...`), `src/text.c` with v1.14
(`toolchain/sn-prodg-24/local/sce/ee/gcc/...`). Both mirrors were
already cloned. Verified: **zero regressions and two immediate gains**
(`func_00233FF8` 2/28 → exact, `func_0023DFC0` 3/28 → 1/28); the sweep
went 142 → 143 exact. It's safe because every currently-matched `text`
function is a leaf with no spill — which is also precisely why this hid
for so long.

**The flag search is an exhaustive negative, which is what makes the
two-builds reading solid rather than merely plausible.** The complete
`target_switches` table was extracted from `cc1.exe` (not grepped as
strings) and every plausible option tested in *both* directions,
including the one-way `-m5900`, all `-mabi=` values and all `-mips`
levels via `-S` to bypass the assembler that blocked earlier attempts.
Neither direction is reachable. `PRODG_MANUAL.pdf` turned out to be the
*debugger* manual and documents no compiler options at all.

**⚠ The "`sq`/`lq` blocked" category was substantially a
MISCLASSIFICATION — those functions need re-examining.** Range surveys
counted stubs as blocked by comparing our output against this file's
(wrong) "retail always uses `sd`" generalisation rather than against
retail's actual bytes for that function. Range A's report of *95 of 165
blocked* is the clearest case: that range lives in `text`, where retail
uses `sq` — exactly what we already emit. Six were sampled and
confirmed. Re-survey any range whose "blocked" count leaned on this.

**RESOLVED — the `core_text` s-register half is no longer blocked.**
This entry used to read "the 234 `core_text` functions that need
s-registers, where retail wants `sd` and no available compiler emits
it". That was true of the *compilers*, but the conclusion drawn from it
was wrong, because it assumed v1.36 had to be the `core_text` compiler.

The missed fact: v1.36 gets `$ra` right but lays the save slots out
**mirrored** versus retail (`$31` at offset 0, s-regs descending above
it), so no s-register function could ever match under it regardless of
source shape. **v1.14 reproduces retail's exact layout** (`$16` at 0,
s-regs ascending, `$31` at the top) and differs only in the mnemonics.

So `core_text` is now built with **v1.14** and post-processed by
`tools/fix_core_spills.py`, which narrows callee-saved `sq`/`lq` spills
to `sd`/`ld`. Because the layout already matches, this is a pure
mnemonic substitution: frame size, the 16-byte slot stride (retail keeps
16-byte stride even for 8-byte stores) and every offset stay exactly as
v1.14 emitted them. No offset arithmetic, which is what makes it safe.
The rewriter only touches `$sp`-relative spills of callee-saved
registers — a `sq`/`lq` against any other base or register is a genuine
128-bit memory operation in the source, not a spill.

Proven: `func_00116FA0` 0/68 (first `core_text` s-register match), then
`func_0012C430` 0/52, `func_0012D340` 0/64 and `func_00129180` 0/72, the
last three exact on the first attempt. `tools/rank_candidates.py` moved
**159 functions out of blocked** (candidates 147 → 269).

Still genuinely open in this area: the `core_text` "`sq $ra`" cases,
where retail is on the `sq` side for a lone `$ra` save — a real 2-byte
floor, correctly marked *risky* rather than blocked.

The earlier (superseded but still-valid) segment-correlation analysis
is kept under "Superseded (kept for the record)" at the end of this
section.

**Scratch-register allocation choice for independent temporaries.**
Seen in `func_001160D8` and `func_00115578`: when a function has 2+
short-lived temporaries with no interdependency (e.g. a computed index
and a loaded pointer), this compiler and retail sometimes pick a
different assignment among `$v0`/`$v1`/`$a0`/`$a1` for them — same
operations, same order, same instruction count, just different register
numbers. Reordering the source statements and splitting shared
expressions into separate locals did not change the outcome in either
case (tried both). Likely a plain compiler-version register-allocator
heuristic difference rather than something controllable from C source;
not investigated as deeply as the other two questions above (no
cross-sub-build check done for this one specifically) since the diffs
it produces are small and clearly benign (verified logic-identical each
time) rather than blocking anything.
**Update 2026-09-22:** `func_00115578` is now exact, so at least that
instance was steerable from source after all; `func_001160D8` is still a
same-size near-miss.

**Retail pads short backward branches; this toolchain doesn't (stray
`nop`s).** Found while working `func_00215048`/`func_00215078`, whose
entire 16/48 residual is two literal `nop`s retail has between an `andi`
and the `beqz` that consumes it. With those nops, the backward branch
spans exactly 6 instructions from target to branch inclusive; without
them it would span 4. Measuring every backward branch across all 1669
disassembled functions shows this is systematic, not incidental:

```
span (instrs, target..branch inclusive) -> count
   3 :   4        7 : 137
   4 :   7        8 :  45
   5 :  13        9 :  44
   6 : 309       10 :  25   ... smooth decay onwards
```

309 branches at exactly 6 against 137 at 7 and only 24 total below 6 is a
sharp discontinuity — a natural distribution would decay smoothly through
the low numbers, not pile up at a boundary. The pile-up at 6 is
consistent with everything that would naturally have been 3-5
instructions being padded up to 6, which is exactly what the **R5900
short-loop erratum** workaround does (mainline binutils spells it
`-mfix-r5900`).

Not reproducible with what's available here, and not cleanly a rule:
- `-mfix-r5900` is rejected by this `cc1` (`Invalid option`), and this
  assembler has no equivalent (checked `ee-as.exe --help` and a string
  dump of the binary — nothing for nop/loop/errata/fix).
- The assembler *does* document nop-removal (`-O` "remove unneeded NOPs",
  `-g` "do not remove uneeded NOPs"), which looked promising, but
  `-Wa,-g` and `-Wa,-O0` both change nothing: `cc1` never emits these
  nops in the first place, so there is nothing for the assembler to keep.
- The 24 sub-6 backward branches are all in *compiled* (not
  `/* Handwritten */`) functions, so it isn't an absolute invariant.
  `func_00116CBC` has a 3-instruction backward `beqz` into a 2-instruction
  return block — structurally the same shape as `func_00215048`'s padded
  one, and unpadded. So the precise trigger is narrower than "any short
  backward branch" and hasn't been isolated.

Practical takeaway: if a near-miss's only residual is unexplained `nop`s
sitting before a **backward** branch, this is very likely the cause —
recognise it and don't hunt for a source shape, because no source shape
produces them. Worth revisiting if a differently-built `cc1` ever turns
up (see the TImode note in the `sq`/`lq` question — same "build-time
config, not a flag" shape of problem).

**Conditional-move vs. branch heuristics differ in both directions.**
For a trivial "select one of two values, then use it" shape, retail's
compiler and this one sometimes disagree about whether to emit a branch
or a conditional move (`movz`/`movn`) — and it goes both ways, so it
isn't a matter of one compiler simply preferring cmovs:
- `func_0021EDD8`: retail *branches*; every single-store C form here
  compiles to `xori`/`movz`. Forcing a branch (by storing in both arms)
  works but costs an extra join `b`.
- `func_001FF4F8`: retail uses `movn`; no plain-C form tried here
  produced it at all.
Recognise this early. If a near-miss's diff is "retail has a branch
where we have `movz`" (or vice versa) rather than a register/scheduling
difference, iterating on source shape is unlikely to close it — the two
known instances each resisted 3+ distinct formulations. Both are
documented in their table entries with the exact forms already tried.

**Structural instruction-shape mismatches (tentative, not yet a confirmed
category).** `func_001FE4D0`, `func_001FF4F8`, and `func_001FF668`
(all attempted and reverted, see their table entries) each hit something
that *looks* different from the three questions above: the mismatch
isn't confined to a register number, a store's position, or a delay
slot — the compiled instructions differ in *kind* from the very first
few bytes, even though the overall logic (confirmed by careful
disassembly reading) is right. Multiple plausible source shapes were
tried for each (early-return vs. combined-condition, different
variable-hoisting patterns) without landing on the one that reproduces
retail's shape. This might just be three individually-hard functions
(a `movn` idiom, a two-`beq`-shared-tail idiom, a search-loop shape) each
needing more specific source-shape investigation than time allowed
rather than one real underlying category — flagged here so a future
round doesn't have to rediscover that these three are harder than the
usual near-miss, but not promoted to a full open question until a
pattern across more instances is clear.

### Superseded (kept for the record)

These entries were written while their questions were open, and have
since been answered by the SOLVED/RESOLVED entries above or by a lever
elsewhere in this file. They are kept, unedited, because each records
dead ends (flags tried, sub-builds compared, spellings that failed)
that would otherwise be repeated.

**Superseded framing — `sq`/`lq` vs `sd`/`ld` is
SEGMENT-CORRELATED, not a missing flag.** The long-standing entry below
asserts "retail always spills callee-saved registers as `sd`/`ld`". That
was generalised from a single `core_text` function and is wrong. Counting
retail's own disassembly:

| Retail segment | Callee-saved spill width |
|---|---|
| `core_text` | ~94% `sd`/`ld` |
| `text` | ~95-99% `sq`/`lq` |

Our compiler is *fixed*: `sd` for `$ra`, `sq` for `$s0`-`$s7`. So it
matches `core_text` on `$ra` and matches `text` on the s-registers — an
exact inversion between the two segments. Flag sweeps have now been run
in **both** directions (earlier rounds only ever swept the s-register
direction; the `$ra` direction has since been swept too) and nothing
changes either.

**Why this matters enormously for planning.** In the `text` segment, a
function that saves `$ra` sits exactly **2 bytes** from retail — one
`sq`→`sd` store plus its `lq`→`ld` restore — and **526 `text` functions
save `$ra`**. Functions previously binned "blocked by sq/lq" are mostly
not blocked at all; in one range alone the classifier had called 209 of
298 stubs blocked when they are 2-byte near-matches. Range surveys that
reported large "sq/lq-blocked" percentages (including range A's 95 of
165) need re-reading in this light.

**Adjusted acceptance criterion for `text`-segment functions:** a result
of `2/N` where the only differing bytes are the `sq`/`lq` store+restore
pair is the *expected best possible* outcome, and should be treated as
success-equivalent — not as a near-miss to keep iterating on. Verify the
two differing bytes really are those instructions (disassemble, don't
assume) before accepting.

**Do NOT mass-convert those functions to committed C on the strength of
the 2-byte floor alone.** The floor only bounds the spill instructions;
each function still needs its logic genuinely derived and verified, and
converting in bulk would inflate the apparent match count while hiding
unverified work. Treat the finding as "these are now worth attempting"
rather than "these are now done."

**Structural implication, and the real open question.** A per-segment
split in a codegen choice this low-level is much more consistent with
retail's two segments having been built **separately** — different
compiler version, or different flags per segment — than with one flag
nobody has found. If that holds, no single compiler configuration can
match both segments, and the project may eventually need to compile
`core_text` and `text` with *different* compilers. Testing whether one
of the other three mirrored SN sub-builds (which use `sq` for everything
including `$ra`) matches the `text` segment better than v1.36 does would
be direct evidence. That is now the live question, not "which flag did
we miss".

**Callee-saved GPR spill width (`sq`/`lq` vs `sd`/`ld`).** Retail always
spills callee-saved registers (`$ra`, `$s0`-`$s7`) as plain 64-bit
doublewords (`sd`/`ld`). This toolchain (SN ProDG GCC 2.95.3, SN BUILD
v1.36) spills `$ra` the same way but spills `$s0`-`$s7` as full 128-bit
quadwords (`sq`/`lq`) — first showed up in `func_00112468`, and will
affect essentially every non-leaf function that needs a callee-saved
register, i.e. most of the codebase. No `-m` flag found changes it
(tried: `-mno-abicalls`, `-mhalf-pic`, `-fomit-frame-pointer`,
`-msoft-float`, `-mfp32`, `-mlong32`, `-mabi=eabi -mgp64`, `-mcpu=5900`,
`-mlong64`, `-membedded-data`/`-mno-embedded-data`; `-mgp32` ICEs).

Checked all 4 SN ProDG sub-builds AngheloAlf mirrors, to see if a
different one matches retail natively:

| Mirror | Compiler | `$ra` | `$s0`-`$s7` |
|---|---|---|---|
| `SN-Systems-ProDG_for_PS2_3.01` (**in use**) | GCC 2.95.3 SN BUILD v1.36 | `sd`/`ld` (matches retail) | `sq`/`lq` |
| `SN-Systems-ProDG_for_PS2_2.0` | GCC 2.95.2 SN BUILD v2.73a | `sq`/`lq` | `sq`/`lq` |
| `sce_ps2_sdk_24`, `ee-gcc295.exe` | GCC 2.95.2 SN BUILD v2.74 | `sq`/`lq` | `sq`/`lq` |
| `sce_ps2_sdk_24`, `ee-gcc2953.exe` | GCC 2.95.3 SN BUILD v1.14 | `sq`/`lq` | `sq`/`lq` |
| retail (target) | ? | `sd`/`ld` | `sd`/`ld` |

v1.36 is already the closest of the four (right on `$ra`), so switching
sub-builds wouldn't help — this needs either a real flag not yet found,
or Insomniac's actual build differs from all four of these in some other
way (a patch, a different `-mcpu`/tuning value, something in how `sq`-
eligible register classing gets decided that isn't exposed as a simple
flag).

**Update:** the question isn't cleanly "s-regs use sq, ra uses sd" as
first framed. `func_001E9E70` (text segment) saves *only* `$ra` — no
`$s0`-`$s7` at all — and retail still spills it as `sq` there, unlike
every other lone-`$ra`-save function seen so far (which all use `sd`,
matching this compiler). Whatever decides `sq` vs `sd` in retail is more
granular than "which registers", possibly per-function or per-translation-
unit — not yet isolated further.

**Researched (no fix found, but useful context):** searched public PS2
decomp/homebrew sources for a known solution. None found. Findings:

- The visible PS2 *matching*-decomp scene (Dark Cloud's `DCDecomp`,
  Resident Evil Code: Veronica X's `recvx-decomp`) targets **Metrowerks
  CodeWarrior**, not SN ProDG GCC — "most PS2 binaries are compiled with
  Sony's proprietary GCC fork or with Metrowerks CodeWarrior" per Jacob
  Harris's PS2 decomp devlog. That plausibly explains the silence on this
  exact problem: public matching-decomp tooling effort has concentrated
  on MWCC, not GCC 2.95.
- `AngheloAlf` (this toolchain mirror's author) runs an active PS2 GCC
  decomp project, `parappa2` (github.com/AngheloAlf/parappa2), but its
  README lists a modern `gcc-mipsel-linux-gnu`/`binutils-mips-linux-gnu`
  cross-toolchain as its build dependency, not SN ProDG — unclear if it
  hits this same issue or sidesteps it (different game/build era, or
  never needed to match this idiom). No sq/lq documentation found in that
  repo. It links two Discord servers (a general PS1/PS2 decomp community,
  and a PaRappa modding community) — likely where any real answer to this
  lives, since it's exactly the kind of narrow tribal knowledge that
  doesn't get blogged. Not asked yet (outside what an agent can do).
- One real technical lead, unconfirmed: a 2013 GCC mailing-list thread
  (Jeff Law, gcc.gnu.org/legacy-ml/gcc-patches/2013-01/msg00333.html)
  shows mainline GCC's R5900 support was, as late as 2013, still getting
  patches to *enable* TImode (128-bit) support for GPRs under the o32
  ABI — implying whether GPRs are TImode-capable (spillable as
  quadwords) is normally a **GCC build-time configuration choice**, not
  a runtime flag. If that held for GCC 2.95's MIPS backend too, it would
  explain why nothing in `-m*` flag space changed the behavior across
  all 4 sub-builds tested: it may need a *differently-built* `cc1`, not
  a different flag to an existing one. Not verified against 2.95's
  actual source/config — a hypothesis, not a confirmed answer.

Next step if anyone wants to pursue this further: ask in the PS1/PS2
decomp Discord linked from `parappa2`'s README, rather than continuing
to search indexed sources.

**64-bit shift by a non-multiple-of-32 constant: hard compile error.**
Confirmed while attempting `func_0023CFF0`: this compiler's `cc1`
cannot compile *any* C-level 64-bit (`long long`) shift whose amount
isn't a multiple of 32 — not a codegen quirk, a hard `unsupported wide
integer operation` error, reproduced with several isolated single-line
test cases (a direct `<<28`, an equivalent `<<32` then `>>4` as one
expression, and even `>>4` alone on an already-64-bit local in its own
statement). Shifts by exactly 0 or 32 work fine (those compile to a
single `dsll32`/`dsrl32`/`dsra32`). Retail contains real 64-bit
shift-by-4 (`dsrl`) instructions — the assembler accepts and executes
them without issue — so the original source for any function needing a
64-bit shift by a non-32-multiple amount must have used inline asm for
that step; it's not reachable from plain C with this compiler. Distinct
from the other four questions below: this is a hard error blocking any
attempt at all, not a near-miss to iterate on. If a function needs this,
skip it (or write the specific shift as inline asm, unverified whether
that's viable here — not tried).

*(Superseded: the residual is steerable after all. Returning the
comparison gives `srl`; feeding it to a branch gives `slti`. See
"`slti reg,reg,0` vs `srl reg,reg,31`" near the top of this file.
`func_00209048` is exact written that way.)*

**Sign-test boolean materialization: `srl`-31 vs `slti`-0.** For a
function returning `x < 0` as a boolean, this compiler always emits
`srl $2,$2,31` (extract the sign bit); retail sometimes has
`slti $2,$2,0` instead. Identical result, one instruction either way,
but it is not steerable from source — on `func_00209048` every
equivalent formulation tried (`< 0`, `<= -1`, `< 1-1`, via a named
local, and `? 1 : 0`) canonicalizes to the same `srl`, and widening the
compared value to `long` only changes it to `dsrl32`, which is further
off. A minimal, self-contained category: it costs exactly 4 bytes in one
instruction and can't cascade, so a function held *only* by this is
worth keeping as documented-close C rather than reverting.

## Solved techniques worth knowing (not open questions)

**Spell the comparison the way you want the blocks laid out.** GCC 2.95
emits a two-way branch by testing the condition as *written* and letting
the other side fall through, so when a near-match differs only in which
arm is inline and which sits behind the label, invert the test in the
source instead of restructuring the function. `if (x >= 0) { A } else
{ B }` and `if (x < 0) { B } else { A }` are the same predicate but not
the same code: the first makes B the fall-through. Proven on
`func_0011B7F8` (`< 0x20` vs `>= 0x20`), `func_001247E8` and
`func_0011CBC8` (`>= 0` vs `< 0` on an RPC result). Try this before
anything else when the only remaining difference is a branch sense --
it is a one-character edit and it also decides which instruction the
delay slot can steal, so it often fixes a scheduling difference at the
same time.

**A `lui`/`addiu` constant is a symbol load, not a literal.** If splat
has invented a `D_xxxxxx` name for a `lui %hi` / `addiu %lo` pair whose
"address" is not in any section, the original was a symbolic value --
an absolute or linker-defined symbol -- not an integer constant. Both
SN sub-builds hand a plain `CONST_INT` to the assembler as `li`, and
gas expands that to `lui`+**`ori`**, never `lui`+`addiu`. No integer
spelling (`int`, `unsigned`, `long`, or a cast pointer) changes this.
Seen on `func_00123208` (`0x00FFFFFF` timeout). Don't spend rounds
rephrasing the literal.

**Spell the zero-trip guard yourself to place a loop invariant.** GCC
hoists a loop-invariant address ABOVE the `for`'s entry test; retail's
compiler puts it BETWEEN the test and the loop head. Rewriting the loop
as an explicit `if (i < n) { invariant = ...; do { ... } while (i < n); }`
puts it where retail has it. This is not cosmetic: the extra word before
the loop head is what forces retail's alignment nop, so the wrong
position costs TWO instructions per loop, not one. Took `func_00123BA0`
from 16 bytes short to exact (its two copy loops). Related: keeping the
`p + 0x10` base in its own variable instead of letting GCC fold the
offset into the `lbu` was worth another 8 bytes there.

**Bind globals to locals to order their `%hi` halves.** When a function
takes the addresses of two globals and the only difference from retail
is which `lui` comes first, assign them to locals in the order you want
them built and pass the locals. GCC folds the locals away and keeps
only the ordering effect, so nothing else in the function changes.
Fixed `func_0011BEB8` and `func_0011CDE0` from 4/140 to exact. Note the
contrast with the next entry: hoisting an ADDRESS into a local is free,
hoisting a LOADED VALUE can delete a reload the original kept.

**Sometimes you must NOT hoist a repeated load into a local.** The
usual advice is to cache a repeated field access in a variable. On
`func_001170A0` that made GCC believe the pointer it had just passed to
a call was still live in `$4` afterwards, so it dropped the reload for
the next call's argument -- four bytes short of retail, and wrong, since
`$4` is call-clobbered. Writing `*(s + 0x54)` out at each use restored
the reload and the size. If a function comes out exactly one load short
around a call, un-hoist.

**A caller that passes no arguments is telling you about the header.**
`func_0011CCB0` calls `func_0011CC38` with a bare `nop` in the delay
slot, while `func_0011CC38` plainly reads `$4`. That call cannot compile
with a prototype in scope, so retail's translation unit did not have
one. Define the callee K&R-style (`int f(a) int a; { ... }`) -- an
old-style definition creates no prototype -- and both functions match.


**Unsigned comparison tell: `lui`/`ori` sentinel vs `addiu $rt,$0,-1`.**
Seen on `func_001161E8`. Comparing an `int` against `-1` gives a single
`addiu $2,$0,-1`; comparing an **unsigned** against `0xFFFFFFFF` forces
the full 32-bit constant via `lui`+`ori`, one instruction more. So if a
sentinel comparison leaves the function exactly 4 bytes short and retail
builds the constant with `lui`/`ori`, change the variable to `unsigned`
rather than hunting for a missing statement.

**Unsigned-mask materialization.** For a bit-clear like `flags &= ~1`,
this compiler compiles `~1` (an `int`, value `-2`) to a single `li`/
`addiu` instruction — but if retail materializes the same mask via a
`lui`/`ori` pair instead, the original source almost certainly used an
*unsigned* mask (`flags &= 0xFFFFFFFEu`, or equivalently `~1u`), which
this compiler can't represent as a compact sign-extended immediate and
falls back to full `lui`/`ori` construction for, matching retail. Seen
in `func_0011B090`. Worth checking for on any near-match involving a
bitmask constant before writing it off as unexplained.

**Delay-slot/schedule steering via source shape.** Two small patterns
that changed retail-matching scheduling, seen fixing `func_0011AA68`:
(1) if retail's branch delay slot holds a computation using a function's
*original* argument register unmodified, make sure the source computes
that value from the parameter directly and as its own statement *before*
any conditional that also reads the parameter — introducing an
intermediate copy (even one that looks harmless, like `int v1 = arg0;`)
can push the compiler toward copying the argument to a scratch register
first instead of scheduling the real computation into the delay slot.
(2) if retail accumulates a final result back into the same register it
just used as an operand (e.g. `addu $3,$3,$4` — read and write `$3`),
write the equivalent C as an in-place update (`x += y;`) rather than
introducing a new variable for the sum (`z = x + y;`) — this measurably
changed which register the compiler picked in that case.

**Branch polarity via if/else shape.** Seen fixing `func_00115748`: for a
two-way branch where both arms do real work (not an early-return guard),
writing `if (cond) { A } B` (early return inside the `if`, `B` falls
through after) can make this compiler pick the opposite branch sense
(`beqz` vs `bnez`, or their likely variants) from what retail chose for
logically-equivalent code — even though the *behavior* is identical
either way. Writing the full two-armed `if (cond) { A } else { B }`
instead fixed it in both instances hit so far. When a near-match's only
diff is an inverted branch condition (same target semantics, opposite
polarity — e.g. `beqzl` where retail has `bnezl` targeting the same
logical case), try the explicit `if/else` form before anything else.

**Shared-tail merging via literal C duplication.** Also seen fixing
`func_00115748`: if two different control-flow paths both end in the
exact same few statements (e.g. `*ptr = v; return v;` after two separate
branches), and retail implements that as a shared tail block both paths
jump into (fewer total bytes than duplicating it), this compiler will
only find that reuse if the C *itself* has just one textual copy of the
shared statements for both paths to fall into — writing it twice (once
per branch, even if character-for-character identical) makes the
compiler treat them as unrelated and compile both separately, costing
real bytes. Restructure so both paths fall through to one instance of
the shared code instead of returning from within each branch.

**Loop-setup statement order.** Seen fixing `func_001F0F30`: for a
counted loop initializing a pointer/bound/fill-value before the loop
body, this compiler's *instruction scheduling* for those independent
setup values can follow a different order than retail even when the
final code is otherwise identical — not a register choice, just which
setup instruction comes first. Writing the C statements in the same
order retail computes them (check the `.s` disassembly's instruction
order before the loop) fixed it directly; no restructuring needed beyond
matching statement order to retail's computation order.

**Avoid unnecessary unsigned casts on values that get widened.** Seen
fixing `func_001FB448`: when a group of `int` parameters gets widened to
a 64-bit type and combined (shifted/ORed) into a packed value, adding
explicit `(unsigned int)` casts before the widening — even though the
values are conceptually just bit patterns, not really "numbers" — makes
this compiler insert extra masking instructions (`dsrl32`/`dsrl` to
clear the sign-extended upper bits) that retail doesn't have. Retail
just lets each `int` sign-extend naturally into the 64-bit register and
combines directly with no masking. If a near-match's extra bytes are
masking/clearing instructions right after a parameter is widened, try
removing an unsigned cast before assuming it's a deeper issue.

**Shared-tail merging with a boolean cast.** A variant of the existing
shared-tail-merging technique, seen fixing the `func_0020CB80` cluster:
when a function's retail shape is "two (or more) independent conditions
must ALL be true for the result to be 1, otherwise 0" and both/all the
"false" outcomes converge to one shared `return 0`, write it as a single
`&&`-chained condition (`if (a != 0 && b != 0) return 1; return 0;`),
not as sequential early-return guards (`if (a == 0) return 0; return b
!= 0;`) and not as a boolean-cast expression (`return a != 0 && b !=
0;`, or worse, `if (a==0) return 0; return b != 0;` which drops the
literal-1-vs-0 distinction entirely). The early-return-guard form
compiles the second comparison as a plain boolean cast (`sltu`
normalize) instead of retail's explicit branch-with-literal-return —
worked for 8 of 9 attempts in this cluster; the one that didn't
(`func_0020CBA8`) had 3 chained conditions instead of 2, so this
technique may not scale past 2.

**Independent-statement order: last source statement emits first.** For a
run of N independent same-shape statements — originally derived for
stores through one already-materialized base pointer, but **confirmed on
independent arithmetic too** (`func_00209048`'s four in-place
subtractions) — this compiler emits the **last** source statement first,
then the remaining ones in source order: source `(A, B, C)` compiles to
`(C, A, B)`. For a 4-statement run it rotated by *two* in one case
(`func_00209048`: source `(a,b,c,d)` emitted `(c,d,a,b)`), so for runs
longer than 3 treat the rotation amount as something to determine
empirically rather than assuming exactly one. So to obtain retail's emitted order `(X, Y, Z)`, write the
source as `(Y, Z, X)` — rotate retail's order left by one. Derived from
`func_00216F28` (3 `short` stores) and then confirmed *predictively* on
`func_0021EF38` (4 stores, mixed float/int, byte-exact first attempt).
This supersedes the earlier "try permutations empirically" advice from
`func_001F7648`/`func_00223478` for this shape — check the rule first, it
appears deterministic. (It also retroactively explains `func_0023CE18`'s
2-store case, where retail's order was the reverse of the source's.)
Note the rule is about the *store* statements; a trailing `return`, and
the base-pointer setup, sit outside the rotation. **The rule is
base-pointer-scoped**: it held for 3- and 4-store runs through a single
base, but `func_00219E60` (stores split between an object base *and* a
separate standalone global) reordered non-rotationally and did not
respond to source order at all — so don't expect it to apply once more
than one base is in play.

**Redundant reload + unfilled delay slot = `volatile`.** Seen fixing
`func_0023E710`: where retail re-loads a field it already has in a
register *and* leaves a nearby branch delay slot empty, the original
source almost certainly declared that pointer/field `volatile` — not a
scheduling quirk to work around. A plain (non-`volatile`) version of the
same C reuses the loaded value and fills the delay slot, coming out
several bytes shorter; adding `volatile` reproduced both retail
behaviours at once, byte-exact. **This is worth re-checking against the
existing "delay-slot-scheduling" open-question entries** — some of those
near-misses (particularly any where retail also re-reads a value it
already had, or leaves a slot unfilled, e.g. `func_0023E5B8`) may be
volatile accesses misfiled as a codegen gap rather than genuine open
questions. Hardware/MMIO-adjacent code is the obvious place to suspect
it, but `func_0023E710` is a plain counter field, so it's not limited to
MMIO.

**Splitting a large constant offset into pointer-advance + field
offset.** Seen while working `func_0023CDF0`: retail reaching a field at
`base + 0x50004` compiles as `lui $r, 0x5` / `addu` / `lw 4($r)` — i.e.
the source advanced a pointer by `0x50000` and then indexed a field at
`+4`, rather than using one `+0x50004` offset (which makes this compiler
materialize the whole constant into a register first, a visibly different
instruction shape). When a near-match's opening instructions differ
around a big structure offset, try splitting it at a round boundary the
way a real `struct`/array-of-pages access would.

**Constant integer division/modulo is *not* strength-reduced.** Unlike
a modern compiler, this one emits a real `div`/`divu` plus MIPS's
div-by-zero trap guard (`beql $r,$0` over a `break 0,7`) even when the
divisor is a plain literal constant � it never converts `x % 19` or
`x / 10` into a magic-multiply sequence. So retail code containing a
real `div` with an immediate divisor does **not** imply the original
source had a variable divisor; a plain `% 19` in C reproduces that shape
exactly. Confirmed on `func_00222D70` (`% 19`, `divu`+`mfhi`) and
`func_0021DB00` (`/ 10`, `div`+`mflo`), both of which come out the right
size and shape, differing only by the register-allocation issue below.
Worth knowing because a real `div` looks like a red flag at first
glance and would otherwise get skipped as un-reproducible.

**Important qualifier: powers of two ARE strength-reduced**, and the
result is one of the commonest `movn` sources in this codebase. A signed
`x / 8` or `x % 8` compiles to a bias-and-shift idiom — materialize
`x + 7`, `slt` against `-1`, `movn` to pick the unbiased value when
`x >= 0`, then `sra 3` (and for the modulo, `sll 3` + `subu`). Seen in
`func_0021D9C8` (`% 8`) and `func_0023DA30` (`/ 0x800`). So the rule
above is specifically about *non*-power-of-two divisors; do not read a
`movn`+`sra` pair as an exotic conditional move when a plain `% 8` in C
produces it exactly.

**Hoist a division/modulo into its own local to move the trap guard.**
Seen fixing `func_0023CD60`. This compiler emits MIPS's div-by-zero trap
guard (`beql $r,$0` over a `break 0,7`) wherever the division gets
expanded, and that position is schedulable. Writing the division inline
in a store (`b[0] = (acc + d) % cap;`) left the guard *after* the `div`,
where retail has it immediately after the loads, before any arithmetic —
45% mismatch. Assigning the result to a local first:

```c
int r;
...
r = (acc + d) % cap;   /* guard now expands here, early */
b[1] = pos + d;
b[0] = r;
```

moved the guard to retail's position and took it to byte-exact. Note it
also fixed a register-allocation difference (retail had `cap`/`pos` in
`$7`/`$8`, this compiler had them swapped) as a side effect — so a
cap/pos-style register swap alongside a misplaced trap guard is worth
treating as *one* problem, not two, and attacking via the guard.

**Recurring sub-case of the allocator question: `%hi` register reuse on
a global load.** Retail frequently loads a global with the `%hi` and the
loaded value in the *same* register (`lui $3, %hi(X)` / `lw $3, %lo(X)($3)`),
where this compiler always uses a separate register for the `%hi`
(`lui $2, %hi(X)` / `lw $3, %lo(X)($2)`). Seen in `func_00222D70`,
`func_0021DB00` and `func_0021B108`. Retail is not consistent about it
either � `func_0021DA98` uses the split-register form and matched
byte-exact � so this is retail's allocator varying rather than a rule
being missed, and no source form has been found that steers it. Two
otherwise-perfect functions are held at 15/48 and 24/60 by this alone.

**Global-address materialization: direct indexing vs. a base-pointer
local.** These two forms are *not* interchangeable, and which one is
right differs per function � check both before calling a near-miss a
scheduling gap:
- Writing `short *p = D_GLOBAL; p[i] = ...;` forces the address
  materialization to happen as its own statement up front, so it lands
  *before* any following branch.
- Writing `D_GLOBAL[i] = ...;` directly lets the compiler materialize
  the address where it likes � including scheduling the `lui` into a
  preceding branch's delay slot, and rematerializing the `addiu` twice
  (once per basic block) rather than keeping one base register live.
`func_00216EF0` needed the *direct* form (retail has `beqz` first with
the `lui` in its delay slot; the local-base form put the `lui` first and
scored 33/56 instead of an exact match). `func_002094E0` needed the
*opposite* � an explicit `char *s` local, without which the compiler
folded each field offset into load/store immediates instead of forming
a real base pointer. So: if a near-miss's opening instructions differ
around a global's address, try the other form before anything else.

**`unsigned char *`, not `char *`, for byte-field base pointers.** Seen
fixing `func_0020CC88`/`func_0020CDB8`: when dereferencing a `char *`
that's really an unsigned byte flag/field (compared against 0, not used
as a signed value), a plain `char *` may compile to `lb` (sign-extending
load) where retail uses `lbu` (zero-extending) — a real difference in
which load opcode gets encoded, not just a value/register question, even
though the *comparison result* against 0 is identical either way.
`unsigned char *` fixes it directly. Doesn't affect `int`/other wider
types, only single-byte loads.

## Method

The full procedure, including setup, publishing and commit rules, is
`docs/WORKFLOW.md`. This is the hand-decoding core of it.

1. Read the `.s` disassembly in `asm/nonmatchings/<segment>/func_XXXXXXXX.s`
   (registers are numeric post-`tools/sn_regnames.py` — `$29`=sp, `$31`=ra,
   `$4`-`$7`=a0-a3, `$2`/`$3`=v0/v1, `$16`-`$23`=s0-s7, `$8`-`$15`=t0-t7).
2. Work out the C shape by hand from the calling convention. This is
   EABI: the first **eight** integer arguments arrive in `$4`-`$11` (so
   `$8`/`$9` in a call setup are arguments, not scratch), float arguments
   in `$f12` upwards, and the result comes back in `$2` (`$f0` for a
   float).
3. Replace the function's `INCLUDE_ASM(...)` line with real C in its
   file: `src/core/<ADDR>.c` for `core_text` (named by the object's start
   address until its real source name is known), `src/game/<name>.c` for
   `text`, or `src/libgcc/` for GCC's runtime (see its README). Forward-declare any
   not-yet-decompiled callee (`extern int func_XXXXXXXX(...);`) — its own
   `INCLUDE_ASM` stub still provides the actual symbol at link time.
3a. **Always confirm the compile actually succeeded before trusting a
   verification result.** A failed compile leaves the *previous*
   `build-sn/*.o` in place, and because the function is still an
   `INCLUDE_ASM` stub in that stale object, it contains retail's own
   bytes — so `check_match.py` reports a perfect `0/N` "match" that is
   entirely fictional. This has been recorded as a real match more than
   once. `tools/build_sn.sh` and `tools/diff.sh` delete the objects first
   and refuse to continue after a failed make; use them rather than
   running the steps by hand. If you do run them by hand, two hazards:
   - Redirect to a file and check the exit status directly
     (`$CC ... > /tmp/cc.log 2>&1; echo $?`). Piping the compiler into
     `tail` and then testing `$?` reports **`tail`'s** status, not the
     compiler's, and will happily print `0` for a failed build.
   - The easiest way to trigger it accidentally: adding an `extern`
     declaration for a global that is *already* declared elsewhere in
     the same file with a different type (e.g. `extern int D_X;` when
     `extern char D_X[];` already exists further down). That is a hard
     error in this compiler, not a warning.
4. Rebuild from scratch and check the function against retail:
   `bash tools/build_sn.sh` audits every decompiled function on size and
   bytes, `sh tools/diff.sh func_XXXXXXXX` shows the instruction diff while
   iterating, and `tools/check_match.py symbol func_XXXXXXXX 0xSIZE`
   checks one function in the linked `build-sn/rac1.elf` (the size comes
   from the `.s` file's own `nonmatching <label>, <size>` header;
   `check_match.py func <vram> <size>` only prints retail's bytes). A
   match means done. A same-size near-miss may be kept, with the
   recovered source and every spelling tried recorded in a comment above
   it; a size mismatch is always reverted (see `docs/WORKFLOW.md`).
   Codegen idioms can be genuinely compiler-sub-version-specific, so
   record a near-miss rather than endlessly guessing flags.
   **Once any function's compiled size differs from retail by even one
   instruction, every function after it in that object shifts — a flat
   whole-section comparison past that point is meaningless, not evidence
   of a regression.** Only trust `tools/check_match.py section` before
   any function in that object has drifted; after that, check functions
   individually by symbol. `tools/check_layout.py` names the first
   symbol that drifted.
5. Once matching, the readability pass (real names/types/structure) can
   proceed on that function using the still-matching build as a
   regression check, per the plan in `README.md`.

## Disproved this round, with counts

- **Store order at the end of a function is not source-steerable.**
  `func_002348E8`/`func_00234948` end with four stores through the
  assembler macro. All **24 orderings** of those four source statements
  were compiled: retail emits `D_00161010`, `D_0015F71C`, `D_00161000`,
  then `D_0015F718` in the `jr` delay slot, and this compiler produces
  no ordering at all with `D_00161010` first — it always sinks that
  store past `D_0015F71C`. Both functions reach retail's exact size, so
  the recovered source is in the file as a comment.
- **Two stores of the SAME value come out in the opposite order to the
  source.** In `func_00201E10`, writing `T[5] = d;` before `T[6] = d;`
  is what reproduces retail's `T[6]`-early / `T[6]`-in-the-delay-slot
  placement. Useful, but it did not close the function: the rest is a
  scheduling tie with the same instruction multiset in a different
  order and every live value relabelled one register up.
- **`addu` operand order, again.** `func_00203548` is 3/100 and the
  residual is two reversed `addu` operand orders. Four spellings of both
  address expressions (base-first, index-first, array indexing on a cast
  pointer, base hoisted into a local) compile to the byte-identical
  instruction stream. This is the same conclusion as the "Swapped `addu`
  operands" lever above, from the other side: when the typed-access form
  does not flip it, nothing else will.
- **A constant's macro expansion belongs to the assembler.**
  `func_0022FD20` was 5/160 because retail builds `0x8000000044` with
  ps2eeas's own `dli` sequence; exact since `tools/ps2eeas_dli.py`.

## Rejected lever: reordering callee-saved spills to v1.36's order

Seven-plus core_text near-misses share one residual shape: retail saves
`$s0` before `$ra`, we save `$ra` first and sink the `$s0` store past a
couple of instructions. v1.36 emits retail's order here, and v1.14
already gives retail's slot *offsets*, so "take v1.14's layout and
v1.36's emission order" looked like a clean second sibling to
`fix_core_spills.py` (which is "v1.14's layout + v1.36's mnemonics").

Measured before building it, over the 43 core_text functions with two or
more callee-saved spills (`tools/diff_words.py` plus a one-off ordering
comparison):

    would fix    8   (v1.14 wrong, v1.36 right)
    would break 10   (v1.14 right, v1.36 wrong)

Net -2. Not adopted.

The reason it can't work as a blanket rule is visible in retail itself.
Surveying the 374 undecompiled functions with two or more spills:

    s-regs first, $ra last   283
    $ra first, s-regs after   91

Retail uses both orders, and neither sub-build reproduces both -- v1.14
emits `[31,16]` for every one of these and v1.36 emits `[16,31]` for
every one, while retail picks per function. So the order is decided by
something in the source, not by the compiler build, and no rewriter
keyed on compiler output can recover it. (A rewriter keyed on *retail's*
bytes would "work" and would also be circular -- the same mistake as the
tail-call rewriter that was keyed on our own output and cost 55 matches.)

Left as-is. If the source-level trigger is ever identified, these eight
come back for free; until then they stay byte mismatches rather than
being forced.

One thing the survey did establish, with no counterexamples in 374
functions: the slot *layout* always ascends by register number
(`$s0` lowest, `$ra` highest). That is v1.14's layout, which is why
`fix_core_spills.py` never has to touch an offset.

## Rejected: unfilling call delay slots

Three candidates in a row (`func_0022EF68`, `func_0023E5E0`, and earlier
`func_0012ABB0`) came out 4 bytes short for the same reason: retail
leaves a call's delay slot as a bare `nop` while this compiler schedules
the following load or store into it. That looked like a compiler-policy
difference worth a post-processing rewriter, in the style of
`tools/fix_tail_calls.py`.

It is not. Counting every call site in retail's own disassembly:

| delay slot | `jal` | `jalr` |
|---|---|---|
| holds a load/store | 1012 | 11 |
| bare `nop`         |  676 |  9 |
| anything else      | 3714 | 47 |

Retail fills call delay slots with memory operations more often than it
leaves them empty, so there is no rule for a rewriter to key on — the
choice is per-site. Functions blocked this way stay blocked. Recorded so
the idea does not get re-derived; `tools/fix_tail_calls.py`'s first
version was keyed off our own output rather than retail's and broke 8
matching functions, which is the same mistake in a different costume.

## `long` is the 64-bit type, `long long` is 128-bit

This compiler's `long long` is a 128-bit type: it compiles a 64-bit
clear to `por $2,$0,$0` / `sq $2,off($base)`, which is both wrong and one
instruction longer than retail's `sd $0,off($base)`. Use `long` for
64-bit values. Found while converting `func_0023D018`, where it was the
whole size mismatch.

## Operand order in C does not steer `addu`'s destination register

"allocator destination-reuse" is the largest risky bucket (71 stubs): the
instruction sequence is right, but retail accumulates into one operand's
register and we accumulate into the other's — `addu $v1,$v1,$a0` against
`addu $a0,$a0,$v1`.

Writing the operands in the other order does **not** move it. Tested on
`func_0020D960` and `func_0021F610`: swapping `base + index` to
`index + base` produced byte-identical output both times, because the
compiler canonicalises the expression tree before register allocation.
The choice is made by the allocator, not by the source. Don't spend
another round on operand order — if a residual is only a destination
register, the lever is elsewhere (which pseudo is created first, or how
long each value stays live), not the `+`.

## Two exits that end the same way get cross-jumped — arm order decides

`func_00120E98` and `func_00121930` are a two-function family: open a
service, run one `func_0011B4C8` RPC, release the lock, return the
reply. Spelled the obvious way —

```c
if (open(...) == 0) return K;
if (rpc(...) < 0) { release(); return K; }
r = *reply;
release();
return r;
```

— both come out **8 bytes short**. The early guard's `return K` and the
failure arm's `return K` both end in `v0 = K; b epilogue`, and the
compiler cross-jumps them into one: the guard branches straight into the
middle of the failure block, just past the call. Retail keeps two
separate `v0 = K` sites.

Putting the success arm inside the `if` and letting the failure path
fall through to the end stops the merge, and both matched:

```c
if (open(...) == 0) return K;
if (rpc(...) >= 0) {
    r = *reply;
    release();
    return r;
}
release();
return K;
```

This is not a compiler difference. `func_0011CBC8` in the same file is
the counterexample where retail *does* share its two exits. Which way it
goes is a per-function layout choice that the comparison's spelling
controls, so when a function is a small multiple of 4 bytes short and
has two exits with the same return value, try inverting the test before
anything else.

Related: `func_0023CE30` needed the same thing in the other direction.
Returning `0` and `arg2` directly from its two paths is 16 bytes short;
retail holds the result in `$s4` across the call, which an explicit
`int r = 0;` assigned in one arm reproduces exactly.

## The base-pointer lever, third case: name the UNOFFSET base

`func_00205830` walks three parallel `int` arrays that live at 0x278,
0x28C and 0x2A4 inside one global, indexed by two plain ints. Three
spellings, three sizes (retail is 156):

| spelling | size |
|---|---|
| `int *dst = &G[0x9E];` etc. | 152 |
| `int *base = G; int *dst = base + 0x9E;` etc. | **156, correct** |
| `G[0x9E + a]` at each use | 184 |

Naming the array with its offset folds that offset into the symbol's
`%lo`, anchoring every later access on `G+0x278` and saving an
instruction retail spends. Naming the unoffset base first keeps the raw
symbol address live and derives all three arrays with separate `addiu`s,
which is retail's shape. Writing the accesses as `G[k + i]` is worse
still: the constant is added to the index *before* the shift, so the one
shared `i*4` is lost.

The same "declare the pointer where retail materialises it" rule fixed
`func_00227D20`'s and `func_002268F0`'s bases: a base pointer declared
at the top of a function is parked in a callee-saved register across the
calls, while one declared after the last call is materialised at the
point of use, the way retail does it.

## Two C names on one asm symbol defeat address CSE

When retail re-materialises `%hi`/`%lo` of a global for a second use
instead of reusing the first use's register, and both forms cost the
same number of instructions, the compiler's choice is an arbitrary
tie-break. Binding a second C name to the same asm symbol settles it:

```c
extern char D_0015EF98[];
extern char D_0015EF98_2[] __asm__("D_0015EF98");
```

This is what got `func_00227D20` down to two `nop`s from matching. Note
the caveat found on `func_0012D2A0`: the trick reliably produces the
extra `lui`/`addiu`, but the compiler may then drop a callee-saved
register and hand the instruction straight back, so check the size
rather than assuming it is a net gain.

The same `__asm__("...")` aliasing is the way to call a function through
a different prototype than the one it is defined with in the same file
(`func_001F68E8` taking a packed 64-bit colour, `func_001FE540` taking
an id, `func_0011FE48` returning a value) — used four times this round.

## Converting a `long` to `int`: mask first

`func_0011E7C8` (the soft-float `long` → `double` conversion) is 12
bytes short if the low half is spelled `(int)x`. Retail materialises
0xFFFFFFFF with `lui`/`dsrl32`, `and`s with it, and only then
sign-extends with `dsll32`/`dsra32`. `(int)(x & 0xFFFFFFFFL)` emits
exactly that. Worth trying whenever a 64-bit value is narrowed and the
result is a few instructions short.

## Cross-jump sweep across the backlog: it explains nothing already on file

The arm-order lever above was swept against both backlog buckets, and the
result is negative in a way worth recording so nobody spends another
round on it.

- **The 50 same-size byte-mismatches: 0 explained, and by construction.**
  Merging two tails always changes a function's length, so a residual
  that leaves the size right cannot be a cross-jump. Confirmed anyway two
  ways: every one of the 50 has exactly as many `jr $31` epilogues in our
  output as retail has, and none of them differs from retail by an
  inverted branch condition (same mnemonic, different register, is what
  the diffs actually show — that is allocation, not layout).
- **The stubbed "N bytes short" bucket: 0 explained.** Not one of those
  functions has two same-valued exits in retail — no duplicated
  `addiu $v0,$0,K`, and a single epilogue each. Their documented causes
  stand: delay-slot filling (`func_0023E5E0`, `func_0022EF68`,
  `func_001209D8`), the load-delay `nop` on tiny FP leaves
  (`func_001F9878`, `func_001F98B0`), `mult` instruction selection
  (`func_0012D4E0`), and allocator destination choice
  (`func_00123650`, `func_001250E0`).

So the lever was fully harvested the round it was found. It is still
worth reaching for on **new** candidates: `func_00217628` came out
size-exact but 15/40 words wrong with the two arms emitted in the
opposite order, and inverting the test made it exact. Check it when
decoding, not when triaging the backlog.

## A value that reaches a join point: give it one definition per arm

`func_0011AFE8` hands out ids from a counter, skipping 0:

```c
v = *(int *)a + 1;
*(int *)a = v;
if (v == 1) { *(int *)a = v + 1; id = 1; }
else        { id = v; }
```

Retail rematerialises `addiu $v1,$0,1` in the taken arm even though
`$v1` already holds that value. Every spelling that computes the value
once — including writing a redundant `id = 1;` after the store — is
folded away, and the function comes out **4 bytes short**. Giving each
arm its own assignment to `id` keeps both definitions alive and matches.

This is the same family as the arm-order lever: what the compiler does
with a value at a join point is steerable by how many definitions the
source gives it, not by the value itself.

## Loop-counter signedness decides whether the loop can be reversed

`func_0011DCB8` ends with a short counted loop, `for (i = 2; i < 3; i++)`.
Declared `int`, gcc normalises it to a count-down from zero
(`addu $18,$18,-1` / `bgezl`) and the function is **4 bytes short**.
Declared `unsigned`, it keeps retail's count-up with `sltiu` and the
function is size-exact.

`sltiu` on a loop back-edge in retail's own output is the tell. Worth
checking on any short function with a small counted loop.

## Folding a subtracted index into the symbol addend

`func_00216620`/`func_002166F0` index a stride-8 table by `id - 20000`.
Spelled inline —

```c
h = *(int *)((char *)D_00137C80 + (arg0 - 0x4E20) * 8 + 0x2988);
```

— gcc folds the whole constant part into the symbol's addend and emits
`%hi/%lo(D_00137C80-149368)` with a zero displacement: five instructions
where retail has six, so the function is **4 bytes short**. Naming the
unoffset base and the index as two separate locals blocks the fold:

```c
char *base = (char *)D_00137C80;
int i = arg0 - 0x4E20;
h = *(int *)(base + i * 8 + 0x2988);
```

which reproduces retail exactly — bare `%hi`/`%lo`, a separate
`addiu $2,$13,-0x4E20`, `sll`, `addu`, and 0x2988 as the load
displacement. This is the base-pointer lever extended to a subtracted
index. (Both functions still stub out on an unrelated allocator
residual; the counts are in the stub comment.)

## Loop-invariant setup belongs in the preheader, so declare it in the loop

`func_00227A70` walks a pending list with a pointer that advances by 8.
Declaring that pointer before the `while` makes gcc hoist the %hi/%lo of
its base and the `sll`/`addu` **above the loop guard**; retail has them
in the preheader, after the guard. Declaring it inside the loop body and
letting gcc build the induction variable itself puts them where retail
has them. The function stayed size-exact either way — the loop's
alignment padding absorbed the slack — which is exactly why this kind of
drift is easy to miss.

## A constant address is not a base pointer

The base-pointer timing rule ("declared at the top of a function it sits
in a callee-saved register across calls") holds for a global's base:
`func_0021FA50` needed `char *t = D_00187040;` moved INSIDE the `if`,
after the call, or it claimed a second callee-saved register and the
function was 8 bytes long.

It does **not** hold for a literal MMIO address. `func_00128638` spins on
0x10002000 and 0x10002010; retail pins both in callee-saved registers,
and no spelling gets this compiler to pin the first one — declaring the
pointer at the top of the function does not stop it being rematerialised
as an absolute load. Three spellings, 160/120/152 against retail's 176,
all recorded in the stub. Same allocator question already on file for
`func_00128860`, which shares the wait loop.

## Two small things that cost 4 bytes each

- **`char` vs `unsigned char` for the constant 0xFF.** In
  `func_00226720` the value 0xFF is compared against a table byte and
  then stored. Stored through `char` it is the *different* constant -1
  and earns its own `li`; through `unsigned char` it shares retail's
  single callee-saved copy. The byte store must also precede the
  halfword store, or the compiler makes the QImode 255 a fresh pseudo
  instead of reusing the HImode one.
- **Store-then-reload needs a basic-block boundary.** `func_0012D688`
  decrements a byte, stores it, and tests the stored byte. Written
  straight through, the compiler forwards the value and tests it with
  `andi`/`bne`; retail re-loads it with `lbu` and tests it bare. Moving
  the store so that an `if` block falls between it and the test does buy
  the reload — but the compiler then spends the saving on a `bnel` that
  duplicates the load into the delay slot, so that function still misses,
  4 bytes the other way. The lever is real; on `func_0012D688` it just
  trades one residual for another.

## Missing prototype on a float-taking callee fails at LINK, as `fptodp`

Calling a `(void *, void *, float)` helper with no prototype in scope
applies default argument promotion, the float becomes a double, and the
link fails with an undefined reference to `fptodp`. It looks like a bad
float constant and is not: it means the `extern` declaration is further
down the file than the call. Four functions this round needed a
prototype (or an `__asm__` alias of one) hoisted above their first use.

## Describe the memory, not the arithmetic: type the base as a struct

The base-pointer lever above says "name the unoffset base". That is one
case of a larger rule, and `func_00205790` is where the larger rule
showed itself. Retail reads two parallel `int` arrays at `D_001A01F0 +
0x278` and `+0x28C` inside a loop over `i`. Three spellings, against
retail's 160 bytes:

| spelling | bytes | what it emits |
|---|---|---|
| `D_001A01F0[0xA3 + i]` | 172 | adds the constant to the index, THEN shifts — once per array |
| `int *p = D_001A01F0 + i; p[0xA3]` | 164 | right inside the loop, but the constant-folded `i == 1` peel then needs its own `addu $3,$3,4` instead of folding into the `lw` displacement |
| a `struct` with named `int` members, accessed `.flags[i]` | **160, exact** | `sll idx,2` / `addu base` / `lw CONST(reg)` in BOTH the loop and the peel |

Only the struct gives retail's shape in both places. So: **where retail
shows `sll idx,2` / `addu base` / `lw CONST(reg)`, that CONST is a member
offset — declare the member.** This does not contradict the
name-the-base rule; in `func_00205830` three arrays come off one live
base in straight-line code and naming the base wins. The statement
covering both is *describe the memory, not the arithmetic.*

The struct is usually aliased onto the asm symbol
(`extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");`) because other
functions in the same file still reach it as a flat array. That is the
"two C names on one asm symbol" trick used for a *type* rather than for
addressing.

## A 4-byte size miss over a correct instruction stream is alignment

The 164-byte spelling above is worth its own note: its extra word was
**not an extra instruction**. The body was 40 words either way. An odd
word count ahead of the loop label made gcc's `.p2align 3` emit a real
`nop`, and internal alignment padding sits INSIDE the `.ent`/`.end` pair,
so it counts toward both the symbol size and the emitted bytes.

When a function is exactly 4 bytes off and the instruction stream reads
correct, count words to the first internal loop label before looking for
a missing instruction.

## `osize` in the sweep is the true body size — measured, not assumed

A round flagged that `tools/sweep_matches.py` takes our size from the ELF
symbol and worried this included the assembler's alignment padding,
making a real 4-byte body difference indistinguishable from an artifact.
It does not, and this is settled:

* gcc brackets each function with `.ent`/`.end`, and the `.align 3` that
  pads to the next function is emitted AFTER `.end`. Many compiled-C
  functions report `st_size == 4 (mod 8)`, impossible if 8-byte padding
  were folded in.
* Restoring the known-4-bytes-short spelling of `func_0012D688` makes the
  sweep print `retail=164 ours=160`, the real body length. A padded
  measure would have read 164 and passed silently. (That one function
  alone also took the scorecard from 341 exact to 333 through downstream
  `jal` drift — rule 4 reconfirmed in passing.)

The sweep now carries a second, independent measure (span to the next
function symbol, trailing zero words stripped) used only as a
one-directional alarm: it is the weaker measure, because a function may
legitimately end in a real `nop`, so it may under-report but never
over-report. Across all decompiled functions: 328 agree exactly, 65 have
the gap measure short by a genuine trailing nop, 0 overrun.

Do not re-open this question.

## A constant hoisted above a call survives as a register class

`func_0012C2F8` stores four scratchpad constants after a call. Retail
keeps `0x70000000` in `$s1` and therefore pays an `sd`/`ld` pair — 8
bytes we did not emit, because we materialised each constant into a temp
just before its store. An earlier round tried binding them to locals
declared AFTER the call; that changes nothing, since gcc folds them
straight back into the stores.

Declared BEFORE the call, the pseudo's live range crosses the call, so
the allocator gives it a **callee-saved** register and the function pays
retail's 8 bytes. gcc still rematerialises the `lui` after the call,
exactly as retail does; the only surviving trace of the earlier
definition is the register class.

General form: a constant hoisted above a call does not survive as a
*value* — constant propagation puts it back — but it does survive as a
*register-class decision*. Where retail spends a callee-saved register on
something that appears to need no register at all, the source defined it
before the call.

## An empty delay slot can be the symptom of a fold, not the cause

`func_00228400` was reverted at 4 bytes short with the residual read as
the scheduler refusing to put a pointer advance into a `jal` delay slot.
The cause was one level up: with a `return` inside each arm, gcc folds
the advance into the return value (`addu $2,$16,32`, one instruction),
leaving nothing for the slot to take. Retail updates the pointer and
copies it to `$v0` as two instructions, in three places — which is what a
**single `return p` at the join** gives, the copy belonging to the join
block and the delay-slot filler duplicating it into both branches.

That is the exit-cross-jumping lever in the direction it usually is not
used: normally the reach is to SPLIT exits gcc merged; here retail really
does share one. When a 4-byte shortfall looks like a missing delay-slot
fill, check first whether an expression got folded that retail kept in
two steps.

## SN's assembler fills delay slots only from AFTER the branch

This explains several things at once and had been mis-attributed twice:

* it is why a bare `jal` at the end of a function gets a `nop` — there is
  nothing after it to take;
* it is why counting instructions in the compiler's `.s` output is
  unreliable. The assembler *inserts* delay-slot nops that are not in the
  `.s`. Count in the linked ELF (`tools/diff_words.py`), never in `.s`;
* it is why `tools/fix_tail_calls.py` produced `lui / lw / j / nop` where
  retail has `lui / j / lw(delay)`. Retail's compiler filled the jump's
  delay slot from BEFORE the jump, which this assembler will not do.

The rewriter now sinks the last body instruction into the tail jump's
delay slot. This is safe by construction, not by analysis: a delay-slot
instruction runs before control reaches the target so program order is
unchanged; a direct `j` reads no registers; and the existing scope guard
already rejects any function with an internal label, so the instruction
cannot be another path's branch target. Restricted to mnemonics that
assemble to exactly one word. The empty-body case is untouched.

## Rewrite a stubbed near-miss in its exact sibling's shape first

`func_002270B0` sat reverted for rounds at 72 against retail's 76, with
the residual correctly diagnosed (retail emits `addiu %lo` and `addiu +4`
separately where we folded them) but concluded unreachable. Its twin
`func_00227068`, four lines above it in the same file, had been exact the
whole time using a separate `int *base` local — which is exactly what
blocks the fold. Written in its twin's shape it is byte-exact.

Before trying anything clever on a stubbed near-miss, check whether a
sibling on the same table or the same idiom is already exact, and copy
its shape verbatim. Three of this round's matches came out of the
revert backlog this way.

## Declaration order: inert for selection, not for emission order

The recorded dead end says declaration order of two locals is
byte-identical. Sharpened on `func_00125160`: it is inert for
INSTRUCTION SELECTION — all 24 orderings of four initialised locals
compile to the same mnemonic sequence, checked in one translation unit
with `tools/permute.py` — but it is **not** inert for emission order.
Moving one local ahead of another reorders the zeroing `daddu`s in the
prologue and took that function from 10/44 to 8/44. It does not reach the
register assignment, which stays the allocator destination-choice dead
end.

## gcc 2.95 only builds a case tree above two cases

`func_0011B0E0` dispatches on two message codes. Retail uses gcc's
case-node DECISION TREE rooted at the HIGHER value, carrying the
redundant `index > root` test that a root with only a left child always
emits. We get the same routine's two-node CHAIN rooted at the lower
value, three words shorter. `balance_case_nodes` only rebalances a list
of more than two nodes; at exactly two it leaves the chain.

So retail's switch had more cases than are reachable in the function --
at least three, with the observed pivot as the median. A `switch` and an
if/else-if chain give byte-identical output, and writing the range test
by hand is folded away because its arm is empty. **Not reachable from a
two-case source**; do not spend another round on it.

## A one-line function body used to vanish from the sweep

`FUNC_DEF` in `tools/sweep_matches.py` had a greedy `.*`, so a definition
written as `void f(void) { g(x); }` on one line captured the CALLEE's
name. If that callee was still a stub, the function silently dropped out
of the audit entirely — reading as "not decompiled" rather than as a
failure. Made lazy. Worth remembering as a class: an auditing tool that
can fail by *omission* needs its own count watched, not just its verdict.
