# Long functions (trial protocol)

You match one long level function (about 1 to 4 KB) byte for byte: C that
SN gcc 2.95.3 (`-O2 -G2`, EABI, MIPS R5900) compiles to retail's code.
Your prompt gives FUNC and ARM. Read [QUEUE.md](QUEUE.md) for its "Rules",
"Candidate file", "Reading the assembly", "Codegen" and "Walls" sections:
they all apply. Its loop does not; follow the one below.

## Files

- `build-sn/try/<FUNC>/PACKET.md`: what the function calls and uses, with
  the declarations to copy, its assembly, and matched C from its file.
- `build-sn/try/<FUNC>/m2c.c`: a machine sketch of the whole function. It
  never matches as written, and its types are guesses (`?`, `s32`), but
  its control flow and expressions are a fast start.
- `nonmatching/<dir>/<FUNC>.c`, if there is one: the closest attempt so
  far, with what the last workers found in its header
  ([NONMATCHING.md](NONMATCHING.md)). Start from it rather than from
  `m2c.c`; never edit it.
- Write only in `build-sn/try/<FUNC>/<ARM>/`, only files you create. Do
  not read the other folders under `build-sn/try/<FUNC>/`.

An attempt is ONE message with two tool calls:

- Write `build-sn/try/<FUNC>/<ARM>/pK.c` (K = 0, 1, ...; a new file each time);
- Bash `bash tools/docker/run.sh python tools/try_func.py <FUNC> build-sn/try/<FUNC>/<ARM>/pK.c --diff --arm=<ARM>`

Your budget is the number in `<ARM>/BUDGET` (30 runs for a first pass); try_func refuses more.

## Method

1. **Map it before writing.** Read the assembly once, whole. Note the
   frame (stack size, which `$s` registers are saved: that many values
   live across calls), the loops, the calls in order, and the branches
   that skip to the epilogue. Put this outline, ten lines at most, in
   `<ARM>/NOTES.md`.
2. **First candidate: complete, compiling, right shape.** Start from
   `m2c.c`: give every callee and global the declaration PACKET.md lists,
   replace `?` types, turn `goto` ladders back into `if`/`while`/`for`.
   The first goal is a candidate that compiles and comes out the right
   size.
3. **SIZE wrong:** `--diff` aligns our instructions against retail's and
   prints the stretches that differ. Fix the first stretch: a missing
   branch or call, a loop written as the wrong kind, a value kept in a
   local versus recomputed.
4. **BYTES n:** same size, n bytes differ, listed by offset. Work top to
   bottom: fix the first differing region with one or two changes, keep
   everything else identical. Never rewrite a part that already matches.
5. **After every run**, add one line to `<ARM>/NOTES.md`: run number,
   verdict, what you changed.
6. **Registers only:** `bash tools/docker/run.sh python tools/regalloc.py <FUNC> <ARM>/pK.c`
   prints the allocator's order and priorities (no budget run); change
   what outranks or overlaps the variable (docs/RAC3_PATTERNS.md).
7. **Stop** at `EXACT`, when the budget is spent, on a wall, or when three
   changes in a row leave the same differences.

## Final message

One line and nothing else:
`{"func": "<FUNC>", "arm": "<ARM>", "exact": true|false, "best": "<best verdict>", "best_file": "<path>", "runs": <n>, "left": "<what still differs, 25 words at most>"}`
