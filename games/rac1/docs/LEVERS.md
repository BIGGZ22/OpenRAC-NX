# Matching cheat sheet

The short version of `docs/DECOMP_PROGRESS.md`, for whoever is turning
assembly into matching C. Read this instead; go to the long file only
for a lever named here.

## Commands

Everything runs from the repo root, inside the toolchain container:

```
bash tools/docker/run.sh python tools/try_func.py func_X c1.c [c2.c ...] [--diff]
```

It puts each candidate in place of `func_X` (a stub or existing C) in a
scratch copy of its file, compiles it the way the build does and compares
with retail. Verdicts: `EXACT`, `BYTES n/size` (same size, n bytes off),
`SIZE` (wrong size: never keep), `COMPILE` (see `build-sn/try/func_X/log.txt`).

- Retail assembly: `asm/nonmatchings/{text,core_text}/func_X.s`.
- Strings: `grep -n D_XXXXXXXX -A2 asm/data/*.s` (printf is `func_001E9730`).
- Extra compiler flags for one run: set `TRY_CFLAGS` inside the container,
  `bash tools/docker/run.sh sh -c "TRY_CFLAGS=-mno-split-addresses python tools/try_func.py func_X pN.c"`.
- For the orchestrator, not workers: `tools/compiler_sweep.py FILE.c` (a core
  file compiled whole under both compilers) and RTL dumps (`-da`, see
  `build-sn/try/func_0020D6D0/dump.sh`).

try_func masks relocations, so `EXACT` means the instructions match; the
full build at landing also checks which symbol each one reaches. Two
stores to different globals in the wrong order, or a call to the wrong
function, pass try_func and fail there.

Registers: `$4`-`$11` = arguments (EABI), `$2`/`$3` = v0/v1, `$16`-`$23` =
saved, `$29` = sp, `$31` = ra, `$28` = gp (0x166D00). Float arguments
go in `$f12`, `$f13`, `$f14`... `long` is 64-bit, `long long` 128-bit.

## Rules

- Write only in `build-sn/try/`. Never touch `src/`, `include/`, `config/`,
  `tools/`, `docs/`, never build or commit; others work in the tree.
- Never redeclare a symbol with another type. Use a cast or an alias:
  `extern int f_i(int) __asm__("func_...");`. Watch declarations later in
  the file too.
- Define the function under its own name. No string literals (declare
  `extern char D_xxx[];`). One exception: when the file already declares
  the function with another type (boilerplate like
  `extern int func_X(void *);`, or a `void (void)` update-pointer type),
  define it as `T name(args) __asm__("func_X")` and say so in NOTES.md;
  the file's prototype gets fixed when it lands. When fixing it would
  change the callers (a callback type, another parameter order that
  matched code relies on), the definition may stay under the alias,
  named `func_X_r`: `T func_X_r(args) __asm__("func_X");` on one line,
  then `T func_X_r(args) { ... }`. The report, the file check and
  `tools/apply_candidate.py` read a definition of `func_X_r` as `func_X`.
- `CONTEXT.md` is written when the wave is planned. A neighbour may have
  landed since: check the file for declarations it doesn't list.
- Read the note above a stub, but don't trust it: many were wrong.

## Start here

1. **Lombyte first.** `python3 tools/lombyte.py func_X` finds the same
   function in Lombyte, the matching decompilation of the US build
   ([SIBLING_DECOMPS.md](SIBLING_DECOMPS.md)). If it is matched there, port
   it: rename functions and globals to ours, keep its control flow.
2. **List every global and how retail reaches it**: through `$gp`
   (sized declaration), `lui`/`%lo` split across two registers (plain
   declaration), `lui` then a load into the same register (`MACRO_ADDR`),
   or never through `$gp` (`NOT_SDA`). That list decides the declarations
   before any statement is written.
3. **Check the known walls below.** If the difference is one of them, say
   so in NOTES.md and stop.

## The build already does these, so never write them in C

Short-loop nop padding, the nop between an FP compare and `bc1`, the nop
after an `mtc1` read next, 64-bit `dli` sequences, tail calls in game code,
a final truncation's `dsra` in the return slot, jump tables, loop alignment.
Post-endlabel nops in retail (`nop` lines after `endlabel`) must be
emitted explicitly: `__asm__(".section .text\n\tnop\n\tnop\n");` after
the function. try_func masks relocations, so it can't see where a
constant lives: if a core function's float/double literals compile into
`.rodata`, say so in RESULT.md with the retail labels they load (they go
in `config/core_rodata.txt`).

## Two compilers

- Game code (`src/game/`) and 989snd: SN gcc 2.95.3.
- Sony SDK code (objects marked `ee29` in `config/core_text.objects`):
  Sony's 2.9-ee. It tail-calls a void function ending in a call but never
  `return f(...)`, has strict aliasing on, and pads short loops itself.
  Library code often matches with the library's own source: newlib's
  2000-02-17 snapshot, and MSSG mpeg2decode for libmpeg's decoder.
  Sony's archives in `toolchain/sn-prodg-24/local/sce/ee/lib/` match retail
  and give real names.

## Levers, most productive first

1. **Callee signatures.** `$v0` vs `$v1` for the first temporary after a
   call shows whether the callee returns a value; an argument register
   untouched up to a call is being passed on; a callee's return type also
   reorders the caller. Fix with an alias. `sltiu` vs `slti` tells an
   unsigned compare from a signed one.
2. **`MACRO_ADDR`** (`include/common.h`) on a global that retail loads
   `lui`+`lw` in one register; in a delay slot it becomes `$gp`-relative.
   `NOT_SDA` for globals that must not use `$gp`. Never declare a global
   the file also reaches through `lui` with a small type: every access in
   the file then goes `$gp`-relative. A word inside a data block read via
   `$gp` is the block's label plus an offset, not a new symbol.
3. **Statement order.** The scheduler's ties go to the source's last store
   first; a value stored twice has its first store last. In a function
   with no branches, order moves registers: try the permutations.
4. **Copies that survive.** Read a value twice (test, then assign);
   `n = x++;`; re-read a global at each use; a `static inline` accessor
   per read; assign a pointer in the loop condition. The other way round:
   a value derived from a load (`bp & 0x7F`) gets its own local right
   after the load, so the raw value dies there instead of taking a saved
   register across a call.
5. **Return shape.** One return with the value set per arm, or
   `if (x) return 1;` per arm with one shared `return 0;`. Failure path last.
6. **Memory shape.** Structs, not byte offsets (a struct member can't alias
   a scalar global, so it can move above one); one `char *` local per block
   reading a global; scaled indices in their own locals (base-first `addu`).
   An array element (`bins[2]`), not a cast pointer (`*(T **)(base + 8)`),
   lets the offset fold into the load. Field types matter: an
   all-`u_char` struct copies with unaligned `ldl`/`ldr`, so give a
   struct the member types its loads and stores show. Pointer
   arithmetic instead of integer arithmetic (or back) changes what the
   compiler shares between expressions: it can stop multiplies from
   merging.
7. **`volatile`** keeps an access out of delay slots and keeps store order;
   make only the fields retail re-reads volatile.
8. **Siblings.** Find a matched function of the same shape in the file and
   copy it first, and look in `include/` for macros and types already
   reconstructed before writing an expression by hand (only from
   permitted sources: CONTRIBUTING.md, "Sources").
9. **Not allowed:** register pins (`register int x __asm__("$14")`), inline
   assembly inside a function, and artificial barriers (`__asm__("" : "+r"(x))`,
   or `do { ... } while (0)` used to block scheduling). Upstream bans them
   ([LLM_DECOMP_INSTRUCTIONS.md](LLM_DECOMP_INSTRUCTIONS.md)), and
   `tools/integrate.py` refuses a candidate that uses one. `__asm__` is only
   for file-scope aliases (`extern T D_x_alias __asm__("D_x");`) and padding
   directives. The one exception is retail's own vector copy: `lq $2,0(a)`
   then `sq $2,0(b)` is `qcopy(dst, src)` in `include/common.h`, or
   `qcopy_nc(dst, src)` where retail keeps a value live across the copy.
   A 128-bit zero store (`sq $zero,0(p)`) is `qzero(p)` there too:
   `*(long long *)p = 0` adds a `por` first.

10. **Flags for a whole file.** Retail built some code with other options,
    for example `-mno-split-addresses` ([SIBLING_DECOMPS.md](SIBLING_DECOMPS.md)).
    There a global is one assembler macro: every access is `lui $at` +
    `%lo(sym)($at)` (loads, stores and FP ones alike), no `%hi` survives a
    call, and gcc never puts such an access in a delay slot. If that's what
    retail shows, or what's left is `%hi` values in saved registers, run the
    candidate with `TRY_CFLAGS=-mno-split-addresses` (a diagnostic: it
    applies to the whole file). GCC 2.95 takes options per translation unit,
    so a match with a flag only counts when the flag can hold for a whole
    file (docs/BUILD_FIDELITY.md, "Flags"):
    - every C function of the file still matches with it: the file goes in
      `config/file_cflags.txt` (`src/game/movie/disp.c` is built that way);
    - a level file (`src/overlays/`, groupings this project chose): the
      function can move to a file of its own that has the flag;
    - an executable file whose neighbours break with the flag: its objects
      follow retail's, so the flag does not make the function a match
      (func_002282D0 and func_0011CB40 went back to assembly for this).
    A small global (declared `MACRO_ADDR`) still goes through `$gp` when it
    lands in a delay slot. A `div` without the zero-divide trap wants
    `-mno-check-zero-division`.

11. **Orphan `%hi`.** When loop optimisation hoists a global's `lui`
    and never pairs it with a `%lo` (retail does this too, e.g. a `%hi`
    copied to a saved register nothing reads), our linker fills that
    `lui` wrongly while try_func, which cannot resolve an unpaired `%hi`
    and masks it, says `EXACT`.
    `tools/fix_orphan_hi.py` (run by the build and try_func) writes such
    a `%hi` of a `D_`/`func_` symbol as a constant, so this is handled;
    if the full build still disagrees on one `lui`, look here first.

12. **Found this session, each with a matched example:**
    - Operand order of an `addu`: `base - (-(i * 4))` keeps the base
      first where `base + i * 4` puts the index first (func_00205220,
      from Lombyte).
    - A zeroed 16-byte vector cleared with `por`/`sq`: a partial
      initializer of a 16-byte aligned type (`typedef float V[4]
      __attribute__((aligned(16)))`, like sceVu0FVECTOR); a plain
      `float[4]` gets a memset call (func_00208248). An aligned struct
      assignment gives a schedulable `lq`/`sq` copy where `qcopy`'s asm
      is a barrier (func_002282D0, func_002153E8).
    - A GIF/DMA packet step per block: block-scoped base pointers give
      each step its own register (func_001F5650).
    - A callee that never returns (`exit`, func_001138B8): declare it
      `__attribute__((noreturn))`, as newlib does (func_00124650).
    - A value retail keeps as a 64-bit constant (`lui 0xFFFF; ori 0xFFFF`
      for -1): the operand is `unsigned int`, compared or stored as
      `0xFFFFFFFFU` (func_001235C8, func_0012E1C8).
    - A copy of constant size in retail's code but a call for a variable
      size: the builtin `memcpy` (`memcpy` is mapped to func_00115248 in
      rac1.ld.sh) (func_0011CE70).
    - A function whose callee's result sits in `$v0` untouched at the end
      under 2.9-ee: `return callee(...)` (func_0012BB30).
    - `lq`/`sq` through `$v0` that stays inside a loop, while values read
      before it are not reloaded after it: `qcopy`, with the values it
      must not clobber held in locals (func_001F4C30), or `qcopy_nc`
      when a local does not do it (func_L05_00256148, case 107).
    - A `lui`-reached global that also shows up `$gp`-relative in branch
      delay slots: plain `MACRO_ADDR` does both (func_001F5148). Keep
      short aliases away from such a symbol: the assembler takes the
      `.extern` size of the file's first declaration of the name.
    - `x < CONST` where retail compares against a register holding CONST:
      compare against a variable (`end = (char *)0x70002000`), since
      `fold` rewrites a literal to `x <= CONST-1`.
    - A loop invariant that retail spills to the stack in the preheader:
      declare it inside the loop body, so loop motion (not sched1)
      places it (func_00220690, near-miss).
    - A register freed only because an argument was evaluated earlier:
      pass the call-containing expression inline as the argument, and the
      other arguments are computed before the call (func_0021AEF8).
    - A 64-bit global that retail reaches with a `lui` macro outside
      delay slots, where MACRO_ADDR alone lets a delay slot turn it
      `$gp`-relative: a `volatile long` MACRO_ADDR alias (func_0012DDC0).
    - Toolchain passes added for retail assemblers: 2.9-ee objects get a
      volatile store moved into the next unfilled `jal` slot
      (tools/fix_volatile_slot.py; func_0012C990, func_00128F90), and
      989snd.o gets a load-delay nop after a `lui` macro load
      (tools/fix_macro_load_delay.py; func_0012E060) plus ps2eeas's
      short-loop padding (tools/ps2eeas_nops.py; func_0012E688).
    - Replacing a stub whose `.s` has nops after `endlabel`: reproduce
      them with a file-scope `__asm__(".section .text\n\tnop...")`
      (func_0022F258), or the whole segment shifts.

13. **From rac3-uya-decomp** (the same compiler family, working with
    us): [RAC3_PATTERNS.md](RAC3_PATTERNS.md) has what carries over:
    int/float order in prototypes, arguments retail never sets, float
    constants and gcse, loop constants, stack slot order, switch tables,
    cross-jumping. For a register tie, `bash tools/docker/run.sh python
    tools/regalloc.py func_X CANDIDATE.c` prints the allocator's order and
    priorities: change what outranks or overlaps the variable.

14. **Address copies are the compiler's.** When retail copies an address
    into a second register (`addiu $s2,$s0,0x10` ... `move $a0,$s2`), do
    not answer with a pointer local (`float *pos = moby + 0x10;`): it
    makes another pseudo and other registers. Global CSE inserts the
    computation where every path needs it and a later pass turns it into
    the copy. Write the address out at every use (`moby + 0x10`,
    `&d->v30`, `TABLE[d->idx].field`, `d->slots[i]->field`) and drop the
    pointer locals. Nine near misses out of nine matched on this
    (2026-10-09). With it:
    - The first local in the frame is never kept in a register; every
      other stack address is, once it has been passed to a call.
    - In a loop with calls, an invariant used once stays in the loop;
      used twice it is hoisted (`Rec *t = TABLE;` inside the loop, used
      twice, when retail holds the table in a saved register).
    - A constant address is derived from a register that already holds
      the same symbol at another offset: a block-local `h = HERO;` after
      the call that takes `D_0013E633 + 0xE9D`. A struct symbol folds the
      member offsets into the symbol instead.
    - A tail retail has once after two arms is often written in both:
      the extra references decide who gets the saved register, and
      cross-jumping merges the copies.
    - An 8-byte block copied with `ldl`/`ldr`/`sdl`/`sdr` is a struct of
      two ints copied by assignment.

15. **How an address is spelled.** Twenty near misses matched on this
    alone (2026-10-09), no statement changed:
    - A struct symbol folds the member offset into the symbol
      (`%hi(sym+off)`). Where retail keeps the base in a register and
      the offsets in the accesses, write `((T *)D_sym)->field` at every
      use with `D_sym` a `char []` alias. Choose per block of data.
    - Separate blocks get separate symbols (`D_0013F450` hero,
      `D_0013E650` voice slots, `D_0013CA40` pad, `D_0013F4D0` hero
      position). As offsets of one symbol, CSE derives one address from
      the other (`addiu $a0,$s0,-0xE00`), which moves registers, can stop
      a cross-jump and move a delay slot.
    - On a `MACRO_ADDR` symbol, symbol+offset counts as two instructions
      and never fills a delay slot; the bare symbol counts as one and
      can. A `float[4] MACRO_ADDR` with stores to `[3]` and `[2]` keeps
      both out of the slot.
    - A plain extern pointer (no `MACRO_ADDR`) where retail has
      `beqz` / `lui` in the slot / `lw` on one register.
    - `extern short X_n __asm__("X");` is not private: it writes
      `.extern X, 2`, and the assembler goes by the last `.extern` it
      reads, so the file's `lui` accesses to X can turn `$gp`-relative.
      Use `SDATA(X)` with the real type (lever 2).

## Known walls: stop and report

No plain-C wording has reached these. Name the one you hit in NOTES.md
and stop, rather than spending the budget on it:

- A 128-bit zero store at a non-zero offset (`sq $zero,16(p)`): C adds a
  `por` first, and `qzero()` stores at offset 0 only
  (`src/game/fastfunc.c`, func_001F9BC0).
- A register allocation that three different wordings leave unchanged
  (`WORKER.md`'s stop rule).
- Hand-written code: trapping `add`/`addi`, `$at` used as an ordinary
  register, a result left outside `$v0`, or COP2 control registers
  (`cfc2`/`ctc2`). The original was assembly.
- Callee-saved registers stored with `sd` where our compiler emits `sq`
  (no available compiler reproduces it; see UYA's compiler matrix).
- A lone `%hi` whose `%lo` comes after a call can link as the wrong half
  in the full build (func_001E9808): report it, it's a tooling issue.

## Overlay functions

`func_LNN_XXXXXXXX` is level code ([OVERLAYS.md](OVERLAYS.md)), stubbed in
`src/overlays/`. `CONTEXT.md` covers most of this for your function.

- Assembly: `asm/overlays/<name>.s`, same format as the executable's.
- `D_XXXXXXXX` and `func_XXXXXXXX` below 0x15F000 are the resident core,
  declared as usual. `D_LNN_XXXXXXXX` is level data: nothing declares it
  yet, so declare it in the candidate, typed from how the assembly uses it
  (lever 2 applies). Keep the name. Never write a level address as a
  number (`*(int *)0x1B24D4`): the data sits elsewhere in other levels,
  and try_func refuses a literal where retail's assembly has a symbol.
  A `$gp` offset at 0x15F000 or above is level data too.
- No m2c sketch. Start from the function's relative in `CONTEXT.md`
  (usually the same source built for another level): if it has matched C,
  adapt that. Otherwise start from a matched neighbour in the file, or the
  assembly.
- `EXACT` is strict: try_func links the candidate at the function's
  address in its level and compares every byte, relocations included. A
  wrong callee or global fails here.
- Don't build the executable to check one: it doesn't include
  `src/overlays/`.
- A candidate that is exact alone and does not compile in its file, or
  changes a neighbour's size there, clashes with the file's declarations:
  the same type name defined twice, a callee with another prototype, or a
  global the file reaches another way (a plain `extern short X;` makes X
  a `$gp` symbol for the whole file). `python3 tools/privatize.py IN.c
  OUT.c <address> --func <FUNC>` gives every type, callee and global of
  the candidate a name of its own, statements untouched; then check the
  whole file (`tools/overlay_file_check.py`). Forty-odd candidates landed
  that way on 2026-10-09.

## What to hand back

As [WORKER.md](WORKER.md) says: `RESULT.md` holds exactly two lines, the
verdict and the candidate file; what you tried and what mattered goes in
`NOTES.md`. Put a short comment above the definition in the candidate:
what the function does and, if needed, why the C is shaped that way.

16. **Loop updates at the top, a `nop` where retail has work, a missing saved register** (func_L00_00207220,
    2026-10-12). Three things found with `TRY_CFLAGS='-dL'` (`src.c.loop`: each giv's benefit and why it was or
    was not reduced), `-dS`/`-dR` (`src.c.sched`, `src.c.sched2`: the per-block schedule) and
    `tools/regalloc.py`:
    - Retail kept `unk2238[i]` as base register + running offset (`addu $2,$s3,$s7`) instead of one pointer.
      The address giv is reduced when `benefit * lifetime * threshold >= loop insn count`; an explicit
      `off += 4` induction variable (not `i << 2`) drops its benefit to 0, so it stays unreduced.
    - Retail's biv updates sit at the loop bottom; ours were moved to the top by inter-block scheduling. A
      file whose retail object was built with `-fno-sched-interblock` needs that flag (config/file_cflags.txt):
      the function went from 88 bytes off to 6 off. Test a flag by appending a line there and running
      `try_func.py`; undo it if nothing improves.
    - The last two bytes were the bottom block's order: write `for (i = 7; i >= 0; p++, i--, off += 4)` with
      the stores in the arms and `continue`, and the order of the three updates decides which one reorg puts
      in the delay slot (the scheduler orders ties by insn number).

17. **A result that lands in the second operand's register** (func_L00_00211D28, 2026-10-12). Local allocation
    ties the result of `a - b` to a dying first operand, so `subu $v1,$v1,$a0` comes out where retail has
    `subu $v1,$a0,$v1`. Retail's form is a named temporary reused for the result, `r = l - r;` (declare `l`
    and `r` once, set both from the bit tests, then subtract into `r`). Written as one expression, or with a
    third temporary, it did not change. A two-function test file compiled by hand
    (`wine .../ee-gcc2953.exe -O2 -G2 -S`) shows this in seconds: a first operand that stays live afterwards
    ties the result to the second, which is how the rule was found.

18. **gcse hoists a plain `w = B + 12` to the top of the function** (func_L15_002A3B98, 2026-10-12). Expressions
    that every path computes (`high(sym)`, a stack address kept in a pointer variable) are inserted at the end of
    the first block by gcse ("PRE/HOIST" lines in `TRY_CFLAGS='-dG'`, file `src.c.gcse`), above a small loop that
    retail runs first; reload then keeps the stack address in `$a2` across it. Retail's address is computed after
    the loop. Two spellings stop it: write the address at its use (`qcopy(B + 12, pos)`, no pointer variable), and
    assign a loop-invariant base (`c = D_L15_00167300;`) inside the loop, where loop.c hoists it later and its live
    range shrinks enough to win a saved register over the table pointer (priorities 0.173 against 0.172 in
    `tools/regalloc.py`). Also: `qcopy()` has a memory clobber that makes gcc reload a value inside an inner loop
    that retail keeps in a register; `qcopy_nc()` does not.
