# Patterns from rac3-uya-decomp

[rac3-uya-decomp](https://github.com/OpenRAC/rac3-uya-decomp) matches R&C 3
with SN ee-gcc 2.95.3 v1.36, the same compiler family as our game code
(v1.14), and works directly with this project. Its
[Matching-Patterns wiki](https://github.com/OpenRAC/rac3-uya-decomp/wiki/Matching-Patterns)
records what made about 3,000 functions match. This page keeps the parts
that hold here and fit our rules ([LEVERS.md](LEVERS.md), rule 9; no
inline assembly in a function, no barriers, no `#define` in a candidate,
flags per whole file). The function names are rac3's examples; read them in
its tree (`~/Projects/rac3-decomp`, or OpenRAC's `games/rac3/ntsc`) when a
pattern fits.

Already in LEVERS.md and found by both projects: callee return types,
statement order and the scheduler's store rotation, `addu` operand order,
structs instead of byte offsets, `volatile` for delay slots, `long` being
64-bit (`long long` is 128-bit, `ULL` is rejected).

## Register allocation

- **The allocator's priority decides close ties**: `floor_log2(refs) * refs /
  live_length`, highest first, lower pseudo number on a tie. A parameter
  loses its incoming register to a higher-priority local that overlaps it.
  `tools/regalloc.py func_X CANDIDATE.c` prints the order, counts and
  registers (adapted from rac3's tool): change what outranks or overlaps
  the variable, not the declaration order.
- Splitting a statement raises a variable's references; a block-local
  temporary is a local pseudo and reuses the register of the input that
  dies into it (`func_003D2878`).
- One variable per loop counter, and one per unrelated value: a counter
  shared by a loop with calls and one without moves both into a saved
  register (`func_00384420`, `func_003A4A78`).
- Big switches: function-level temporaries reused across cases push values
  into saved registers; block-local per case fixes it (`func_00397490`). But
  where retail keeps one value in one saved register across cases, it was
  one variable (`func_003DBEC8`).
- A copy made right after a parameter's first use keeps the parameter in
  its incoming register (`d = x1 - x0; x = x0;`, `func_003D14D0`).

## Calls and arguments

- **Int/float order in a prototype.** EABI gives ints and floats registers
  independently, so reordering a prototype keeps the ABI but changes the
  order of the argument moves, and so which one lands in the `jal` delay
  slot. When only argument moves are out of order, sweep the interleavings
  of the callee's prototype, and of your own function's parameters
  (`func_003A0EB0`: one of 35 orders matched).
- **Arguments retail never sets**: the source passed the stale incoming
  parameter, or called through a zero-argument K&R declaration (an alias
  `extern void *f_X();` called as `f_X()` emits no argument moves,
  `func_003E11D0`).
- **Arguments retail sets for no visible reason**: the callee takes more
  parameters than the call seems to need (`func_003E8FB0`), or the function
  passes its own parameters straight on (`f(p, b, c)` calls `g(p, b, c)`).
- A callee defined later in a file is an implicit `int f()` to the code
  before it: float arguments go as doubles and `$v0` stays busy. Declare
  every callee.

## Constants, loops and scheduling

- **gcse copies a float constant into every block where its set is
  available**, so `1.0f` gets rebuilt at each call where retail keeps it in
  a register: assign it inside the loop, in the block of its use
  (`func_003AF2C0`).
- A constant retail rebuilds in the loop body has two sets in the source,
  so loop.c won't hoist it: `{ s32 lim = -1; if (i <= lim) i = 3; lim = 0; }`
  (`func_003B23F8`). A constant retail keeps in a saved register is a local
  declared before the loop (`s32 one = 1;`, `func_003947B0`).
- Separate variables for identical constant arguments (`k1 = 0x98; k2 =
  0x98;`): each `li` keeps one dependent and stops outranking the rest
  (`func_003D1D40`).
- Reusing one variable sets anti-dependences that keep stores in order:
  `t = G; H = t; t += 0x10; G = t;` (`func_00384B68`).
- sched1's tie-breakers: the instruction that ends a register's life first,
  then the one with more dependents (`func_003E89F0`).
- A new variable that does not cross a call keeps its computation after the
  call; sets of pseudos that cross calls get hoisted (`func_003869E8`).
- Hide a value from combine: in `e = A; for (i = 0; ...)` combine knows `e`'s
  sign and turns `blez` into `beqz`; `i = 0; e = A;` hides it (`func_003B5D10`).
- `k = C; k *= t;` ties the output to the constant's register where
  `k = C * t` doesn't (`func_003A73A0`).
- Loop shapes: `for (i = lo; i < hi; i++) dst[i].x = ...;` rather than a
  walking pointer (`func_0039B0F8`); a count-up loop gcc would reverse as
  `i = 0; if (n > i) { do { ... } while (...); }` (`func_0038E030`);
  derived pointers as extra loop increments, `for (...; i++, q += 0x20)`
  (`func_003C9078`); separate block-local pointers per loop.

## Branches and switches

- **Branch or conditional move**: gcc only makes `if (a < b) a = b;` a
  conditional move when the source value is already in a register; a memory
  source keeps the branch. `if (x) c = f(A); else c = f(B);` branches where
  a selected-id form gives `movz` (`func_003B2AF8`).
- An `s32` selector puts the then-assignment in a `beq` delay slot; a `u8`
  selector gives `bne`/`b` (`func_003B8440`).
- A boolean from a compare: `if (r < 0) return 0; return 1;` gives `slti` +
  `xori`, where `return r >= 0;` gives `nor`/`srl` (`func_003AAAC8`).
  `(x ^ 1) == 0` gives an `xori` test where `x == 1` gives `li`/`bne`.
- ANDs of call results: `t = f() != 0; ok = ok & t;` gives `sltu` + `and`;
  `ok &= f() != 0` gives `movz` (`func_003E3A80`).
- **Cross-jumping keeps the later copy**: when retail jumps forward into
  another case's code, both tails must be byte-identical. To stop a merge,
  use block-local temporaries per case and a `goto` into the shared tail
  (`func_00391B70`); to get a merge retail has, write the shared body in
  each case.
- Switch tables: `case 0:` sharing the `default:` body keeps a table that
  starts at 0 (`func_003AE368`); gcc builds a table only with at least five
  case nodes and drops cases that go where `default` goes, so add separate
  `case N: break;` arms to keep retail's length. `case 2: default:` or an
  extra `case 3: break;` picks a different root for a compare tree
  (`func_003813E0`).

## Memory and declarations

- **Stack slot order follows when a variable becomes addressable**, not its
  declaration: a scalar moves to the stack at its first `&x`, an array gets
  its slot when declared. One `u8 cc[2]` instead of two `u8` scalars put the
  bytes at retail's offsets (`func_003C0188`). Block-local arrays get
  separate slots; retail shares slots, so declare stack buffers once at the
  top (`func_003DCD08`).
- A `static __inline__` helper's parameter order sets the schedule of the
  copies integrate.c makes of its arguments (`func_003EB728`).
- A load through a cast pointer is untyped memory, so the scheduler keeps
  stores to plain globals after it; a struct member read does not conflict
  with them (`func_003B6528`).
- An indexed struct member written out at each use (`D.row[a].f`) keeps
  retail's addressing where a row pointer folds the member offset into the
  base register (`func_00395AC8`); the reverse when retail re-reads a field
  chain (`func_003C0B10`).
- A hardware register with its full address in a register:
  `*(volatile u32 *)0x10000800`; without `volatile` gcc folds the low half
  into the load offset.
- Float constants: write the exact decimal of retail's bits, truncation
  included (`0.0749999955f` for `0x3D999999`, `func_003A0EB0`).
- Chained assignment `a = b = f()` gives retail's store order
  (`func_003BA490`).

## What does not carry over

- **Its VU0 forms** (inline asm in functions, the `j`-constraint form) and
  the inline-asm `nop`, `sq $zero` and `plzcw` forms: inline assembly in a
  function is not allowed here; `qcopy`, `qcopy_nc` and `qzero` in
  `include/common.h` cover retail's quadword copies and zero stores.
- **`do { } while (0)` as a lever** (reference weight, a scheduling barrier
  through its loop notes): upstream bans it as a barrier, and
  `tools/integrate.py` refuses it.
- **Per-function flag overrides** (`-fno-schedule-insns`,
  `-fno-rerun-loop-opt`, dropping `-fforce-mem`): flags apply to whole files
  here (docs/BUILD_FIDELITY.md, "Flags"). rac3's own data agrees with that
  rule: `-fno-force-mem` holds for all 88 C functions of one of its files.
- **`-mvu0-use-vf0-vf2`**, which rac3 builds everything with because it
  changes ordinary loop code: measured here on 2026-10-07, it breaks 19 of
  the 2,056 matched level functions and turns none of 410 near misses
  exact, so retail RAC1 was not built with it.
- **Its `sq $ra` rewrite and `div.s` padding table**: steps that change
  compiler output; RAC1 has no padded divides, and v1.14 emits retail's
  `sq` saves itself.
- **Its `.extern SYM, SIZE` hints** matter only when SN's real assembler
  runs; see docs/BUILD_FIDELITY.md for where that stands here.
