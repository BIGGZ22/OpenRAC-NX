# Queue worker protocol

You match functions of Ratchet & Clank (PS2, PAL) byte for byte: C that
SN gcc 2.95.3 (`-O2 -G2`, EABI, MIPS R5900) compiles to retail's code. Your
prompt gives WAVE, ID, N and COUNT. This file is your whole instruction
set: do not read WORKER.md, LEVERS.md or other docs, and do not explore
the tree beyond what a packet names.

## Loop: claim COUNT functions at a time until N are handled

Run everything from the repository root, with Python 3.10 or newer: on
the Mac that is `/opt/homebrew/bin/python3` (plain `python3` is 3.9 and
fails on `wave.py`).

1. `python3 tools/wave.py claim <WAVE> <ID> --count <COUNT>` prints a
   packet per function: what it calls and uses, its assembly, and matched
   C to start from. `QUEUE EMPTY`: stop.
2. An attempt is ONE message with two tool calls, in this order:
   - Write `build-sn/try/<func>/pK.c` (K from the number the packet header gives, then up by one: a new file each time, never one an earlier round wrote);
   - Bash `bash tools/docker/run.sh python tools/try_func.py <func> build-sn/try/<func>/pK.c --diff`

   With several claimed functions, put their attempts in the same message.
3. Not `EXACT`: say in one line which instructions differ, then change one
   thing. `COMPILE`: read `build-sn/try/<func>/log.txt`, fix it.
4. A function is handled when try_func says `EXACT`, or when you stop:
   - its budget is spent (try_func refuses further runs);
   - three different wordings compile to the same bytes (an allocator or
     scheduler tie that rewording will not move);
   - it hits a wall below.

   When you stop, append two or three lines to `build-sn/try/<func>/NOTES.md`:
   what the function does, where the difference is, what would unblock it.
   Stopping is a normal outcome. Write no RESULT.md: the runs are logged.
5. Claim again until N functions are handled or the claim prints
   `QUEUE EMPTY`. Do not stop earlier: finishing one claim is not the end.
   If every try_func run for a function fails on a file other than your
   candidate (another stub's assembly), say so in NOTES.md and move on:
   that is for the lead to fix.
6. Your final message is one line and nothing else:
   `{"id": "<ID>", "exact": ["func_..."], "stopped": ["func_..."], "idiom": "<25 words at most, or empty>"}`

## Rules

- Write only in `build-sn/try/<func>/`, only files you create. Never edit
  `src/`, `include/`, `config/`, `tools/` or `docs/`; never delete
  anything; never commit, build the game, start sub-agents or use the web.
- Compile only through try_func. No compiler runs or harnesses of your own.
- Plain C: no register pins, no inline assembly in a function, no barriers
  (`__asm__("" : ...)`, `do { } while (0)`), no `volatile` added only to
  pin an order, no read of a local that was never assigned. A match that
  needs one is not a match: stop instead. A function that reads a register
  it never sets, or branches outside itself, is a fragment: stop at once.
- The build is fixed: no step may change what the compiler emitted, and
  flags apply only to whole files (GCC 2.95 has no per-function options).
  Never propose a new post-processing step or a per-function flag; a
  function that only matches that way is not a match
  (docs/BUILD_FIDELITY.md).
- Sources: work from the assembly and from what the packet gives you.
  Never use Sony SDK source, sample code or headers, or any leaked
  material, from memory either (CONTRIBUTING.md, "Sources"). If a function
  looks like SDK sample code, decode it from the assembly like any other.
- Matched C in a packet may call functions by readable names
  (`include/names.h` macros for the address names). Define and declare
  with the address name (`func_...`, `D_...`); in a body either works.
- A hardware register (0x10000000 and up, 0x70000000 scratchpad) is
  `*(volatile int *)0x10000000`. Never an `__asm__` alias whose name is
  a number.
- An `__asm__` alias names exactly one existing symbol:
  `__asm__("D_0013E633")`, never an expression such as
  `__asm__("D_0013E633 + 0xF1D")`. `land` refuses those.
- Never write a level address as a number. Use the symbol the assembly
  names (`D_L05_001B24D4`), declared in the candidate.

## Candidate file

Extern declarations first, then the one function under its own name, with
one comment line above it saying what it does. Declare what the packet
lists as "not declared anywhere yet"; copy exactly the declarations it
gives for the rest (a second declaration with another type fails to
compile). No string literals: `extern char D_xxx[];`.

## Lombyte ports

When the packet shows "Lombyte's matched C", Lombyte (the US build's
decompilation, MIT) has matched this function. Port it rather than start
over ([SIBLING_DECOMPS.md](SIBLING_DECOMPS.md), "Porting a function"):

- Keep its control flow, statement order and types; they are what
  matched. Its file (the path in the packet) has the structs and
  typedefs it uses: copy the ones you need into the candidate.
- Rename its symbols to ours. A function: `python3 tools/lombyte.py NAME`
  prints our name. A global (`D_xxxxxxxx`, `D_LNN_xxxxxxxx`, a named
  one) is a US address: take the symbol at the same place in our
  assembly (the n-th `%hi`/`%lo` or `$gp` access matches the n-th in
  theirs). Offsets inside a struct are the same in both builds.
- `s8`..`s64`, `u8`..`u64`, `f32`, `f64` exist in `include/common.h`;
  `u128` does not: declare it as under "Codegen" (128-bit copies).
- The comment above the function must end with
  `Adapted from Lombyte (MIT) for PAL: <its file under src/>, <its name>.`
  The lead lists every port in THIRD_PARTY_NOTICES.md.
- If it doesn't match with our compiler within the budget, stop as
  usual and say in NOTES.md how far it got: Lombyte builds some files
  with a patched EE-GCC that ours cannot reproduce.

## Near misses

When the packet shows "Best earlier attempt", an earlier worker came close
and stopped. The packet gives that candidate (`best.c`) and every
instruction that still differs. Your first attempt is `best.c` with one
change aimed at the first difference; never start over.

The repository keeps each function's closest attempt in
`nonmatching/<dir>/<func>.c` ([NONMATCHING.md](NONMATCHING.md)); `best.c`
already is the closer of that file and the run logs. Never edit
`nonmatching/`: your runs are logged, and the lead re-stages what got
closer.

- Same instructions, registers swapped: change the order locals are first
  assigned, or swap the operands of a `+`, `*`, `&`, `|` or `==`.
- Same instructions in another order: reorder the independent statements
  that produce them (see "Codegen": the source's last store tends to come
  out first).
- One extra or missing move: a local the compiler keeps or folds. Inline
  it, or give the value its own local.
- `lui` + `lw` against `$gp`, or the reverse: the symbol's declared size
  (`extern short` for `$gp`, `MACRO_ADDR` for `lui`).
- A difference only in a branch-likely (`beql`/`bnel`) or a delay slot:
  try the other form of the condition (`if (!x) ... else ...`).

Three changes that leave the same differences mean the tie won't move:
stop and say in NOTES.md which instructions are left.

## Reading the assembly

- Arguments `$a0`-`$a3`, `$t0`-`$t3`; floats `$f12`, `$f13`, `$f14`...;
  results `$v0`, `$f0`. `$s0`-`$s7` are saved, `$gp` is 0x166D00.
- `func_XXXXXXXX` and `D_XXXXXXXX` are the resident executable;
  `func_LNN_...` and `D_LNN_...` belong to the level. `lui` + `%lo` is an
  ordinary global. A `%gp_rel` access is a small one: at `-G2` only
  objects of two bytes or less go through `$gp`, so declare it
  `extern short D_x;` (or `char`) and read a word as `*(int *)&D_x`, a
  float as `*(float *)&D_x`. This is the project's convention
  (`include/common.h`), and for most functions it matches. Where it
  does not (a load retail has before a store through a pointer comes
  out after it, or is repeated after it), declare the global with its
  real type, `extern float D_x SDATA(D_x);`, and write the stores as
  struct members: a read through a cast is not a scalar to the
  compiler and waits behind every pointer store (`SDATA` in
  `include/common.h`; func_L17_002EEB08). A bare `$gp` offset (`addiu $2, $28, -0x7580`)
  is the address 0x166D00 + offset: at 0x15F000 or above it is level
  data, `D_LNN_<address>` (`D_L00_0015F780`), never `D_<address>`.
- A moby (game object) is a `char *`/struct pointer with fields at fixed
  offsets: state byte at 0x20, position vector at 0x10, its own data
  pointer at 0x78. Matched code in the packet shows the usual spellings.
- A packet that says "A joined function" lists several catalogue entries:
  they are one function, written under the first name. A branch to the
  next entry's name is a branch inside it. A `lui` in the delay slot of
  such a branch belongs to the load at the top of the next entry:
  `lui $3, 0x14` then `lw $2, 0x14DC($3)` reads `D_001414DC`.

## Codegen (verified on matched functions)

- `lq $2, 0(a)` then `sq $2, 0(b)`: `qcopy(b, a);` from `common.h`. Where retail keeps a value it read
  before the copy in its register and uses it after (ours reloads it), `qcopy_nc(b, a);`: the same copy
  without the "memory" clobber.
- `sq $zero, 0(a)`: `qzero(a);` from `common.h`.
- Any other `lq`/`sq` pair (another register, an offset, the `sq` in a
  delay slot) is a plain 128-bit copy:
  `typedef int u128 __attribute__((mode(TI)));` above the function, then
  `*(u128 *)(a + 0x30) = *(u128 *)(b + 0x10);`. A zero store through `por` is `*(u128 *)a = 0;`.
- The first temporary after a call is `$v0` when the callee returns a
  value and `$v1` when it does not: that decides a callee's return type.
  `sltiu` is an unsigned compare, `slti` a signed one. `lbu`/`lb`,
  `lhu`/`lh` give a field's signedness.
- Write stores in the target's order first. Where the compiler had a free
  choice, the source's last store tends to come out first: permute.
- A value loaded once and used twice is a local; a field loaded again at
  each use is re-read in the source (`moby[0x20]` written out twice).
- One shared `return` with the value set in each arm, or a `return` per
  arm: try the other when only the epilogue differs.
- A callee's argument register untouched since function entry is an
  argument passed straight on: keep the parameter order and types.
- Argument moves come out in the order the callee's PARAMETERS are declared.
  A float set up before or after an integer or pointer (`mov.s $f12` against
  `move $a0`) is the prototype, not a scheduler tie: declare the callee with
  its float where retail sets it up (floats and integers travel in separate
  registers, so the call is the same). func_L02_002D8B80 was 8 bytes off for
  three rounds over one such line; sixteen more closed the same way.
- With the result unused, a callee declared as returning a value gives `$v1`
  as the first temporary after the call and a `void` one gives `$v0`. Read
  the callee's epilogue: if it sets `$v0` or `$f0`, declare the return type
  (func_L00_00277A88, 3 bytes off until func_0022DD68 was `int`).
- An `addu` with the index first: `base + i * 4`. With the base first:
  index in its own local, or `base - (-(i * 4))`.
- `sltiu $2, $2, 3` after `addiu $2, $3, -5`: a range test, written
  `v >= 5 && v <= 7` (func_L00_0020DBB0).
- `movn`/`movz`: a default then one conditional assignment,
  `r = 0x7E; if (arg == 0) r = 0x68;` (func_L05_002559DC). Swap which
  value is the default to get the other instruction.
- A jump table in retail: a `switch` with one `case` label per value, even
  when several cases do the same thing; merged labels give a compare tree
  (func_L14_002F2880). Cases may skip a value.
- A global read as `lui` + `lw` in one register: declare it `MACRO_ADDR`
  (from `common.h`), under an alias if the file already declares the name:
  `extern int D_x_m __asm__("D_x") MACRO_ADDR;` (func_L01_00252E80).
- One global reached through `lui` in the body and through `$gp` in a
  branch delay slot (an indented `lw $x, -0x6CA8($28)`): not a wall.
  Declare it only `MACRO_ADDR`; the build turns a macro access the
  compiler puts in a delay slot into the `$gp` form
  (`tools/check_macro_slots.py`).
- Registers swapped inside one block while the rest matches: give that
  block its own block-scoped locals (its own base, offset, loop counter)
  instead of function-scope ones shared with other blocks, and declare
  them in the order that gives retail's registers (func_L17_002F3450,
  func_L17_002D8CC8, func_L08_002DD440).
- The level zero vector (`D_LNN_0015F660`, passed by address, often three
  times to func_L00_00265050) is `extern float D_LNN_0015F660[] MACRO_ADDR;`,
  passed as `D_LNN_0015F660`: as a plain `float` taken with `&` the call
  setup comes out in another order (func_L17_002F0678, func_L17_002F6ED0).
- A 128-bit copy between stack arrays is a plain `u128` assignment
  (`lq $v0,0x20($sp)`, the store free to move); through pointers it is
  `qcopy()` (func_L17_002F5388).
- A byte store of 0x80 and up (`0xFF`, `0xFA`) through a `char *` comes out
  as a negative constant: write it through `unsigned char *`
  (func_L13_002C4A68).
- A variant of matched C ("differs only in a number" in the packet): copy
  it and change the constant, offset or callee the assembly shows.
- A walk over an id list (`lhu`, `andi 0x7FFF`, `sll 8`, a class compare,
  `bgez` back to the top) where retail loads the class constant inside the
  loop: return early for an empty list, then `for (;;)` with every exit a
  `return` inside the body and nothing after the loop; an entry of the
  wrong class that retail sends back to the top is `continue`. A
  `do`/`while` with the test as its condition hoists the constant into a
  saved register (func_L16_002D0A40, func_L16_002D6E98).
- When one function of a family has matched, write its siblings in exactly
  its form first (func_L16_002D6F90 and func_L16_002D7178, each EXACT on
  the first run from func_L16_002D6E98).

## Walls: stop at once and name the wall in NOTES.md

- `sq $zero` at a non-zero offset (at offset 0 it is `qzero()`), `cfc2`/`ctc2`, `$at` used as an
  ordinary register, trapping `add`/`addi`: the original was assembly or
  has no C form.
- More saved registers or a bigger frame than retail with the instructions
  otherwise right, or a repeated `lui` for one symbol: a per-function
  compiler flag. Note it; do not chase it.
