# Worker guide

For an agent working on one function. `tools/wave.py` gives you the
function's name, your role and your budget. Everything else is here, in
`build-sn/try/<func>/CONTEXT.md` and in [LEVERS.md](LEVERS.md).

## Rules for every role

- Do the work yourself. Don't start sub-agents, search the web or install
  software.
- The build is fixed: no step may change what the compiler emitted, and flags apply only to whole files (GCC 2.95 has no per-function options). Never propose a new post-processing step or a per-function flag; a function that only matches that way is not a match (docs/BUILD_FIDELITY.md).
- Plain C only, as upstream requires
  ([LLM_DECOMP_INSTRUCTIONS.md](LLM_DECOMP_INSTRUCTIONS.md)): no register
  pins, no inline assembly inside a function, no artificial barriers
  (`__asm__("" : ...)`, `do { } while (0)`). A match that needs one isn't a
  match: report the best plain-C candidate instead. Prefer real structs to
  raw offset arithmetic. Retail's 16-byte vector copies (`lq $2,0(a)` then
  `sq $2,0(b)`) are `qcopy(dst, src)` from `include/common.h`: call it
  (`qcopy_nc` where retail keeps a value live across the copy; `qzero(p)` for `sq $zero,0(p)`).
- Write only inside `build-sn/try/<func>/`. Never edit `src/`, `include/`,
  `config/`, `tools/` or `docs/`, never run the full build, never commit.
- Every `try_func` run counts against your budget, `--diff` reruns
  included. It prints `run k of N` and refuses once the budget is spent.
- Compile only through `try_func`: no direct compiler runs, RTL dumps or
  harnesses of your own. In wave 2 every match came within 6 runs, while
  the workers who dumped RTL cost twice as much and matched nothing.
- Finish by writing two files in `build-sn/try/<func>/`:
  - `RESULT.md`, exactly two lines: the verdict (`EXACT`, `BYTES n/size`,
    `SIZE ours X / retail Y` or `COMPILE`), then the best candidate's path.
  - `NOTES.md`: add a section for this round below any earlier ones, never
    deleting them. Say what the function does, what you tried, what
    mattered and where any remaining difference is.
- End with a one-line reply: `<verdict> | <candidate path> | <runs used>`.

## Matching

1. Read `CONTEXT.md`. It gives the function's source file, compiler, the
   declarations of everything it calls and uses, its callers' prototypes,
   matched functions of a similar shape in the same file, and earlier
   attempts. If there were earlier attempts, read their notes and start
   from the best candidate; don't repeat what failed. Check the source
   file too: declarations added since the dossier was written win.
2. Read [LEVERS.md](LEVERS.md), and do its "Start here" steps before
   writing C:
   - `python3 tools/lombyte.py <func>`: if Lombyte has matched the same
     function, port its C (see [SIBLING_DECOMPS.md](SIBLING_DECOMPS.md)).
     That usually takes one or two runs.
   - List every global and how retail reaches it; that decides the
     declarations.
   - Compare the assembly with "Known walls". If it hits one, say which in
     NOTES.md and stop.
3. Start from, in this order: Lombyte's C, the best earlier candidate, the
   open-source original when `CONTEXT.md` names one, `m2c.c`, or
   `bash tools/docker/run.sh python tools/m2c.py <func>`.
   Never use Sony SDK source, samples or headers, or any leaked
   material (CONTRIBUTING.md, "Sources"); if a function looks like SDK
   sample code, decode it from the assembly like any other.
4. Write each candidate as `build-sn/try/<func>/pN.c`, taking the next free
   number: the function plus only the externs it needs. Copy declarations
   from `CONTEXT.md` or the file exactly; a second declaration with
   another type fails. Declare and define with address names
   (`func_X`, `D_X`); inside the body, call or use a symbol by the name
   `CONTEXT.md` gives it (`include/names.h`, docs/NAMES.md) when it is
   not a candidate. Both spellings compile to the same code.
5. Test with
   `bash tools/docker/run.sh python tools/try_func.py <func> build-sn/try/<func>/pN.c`,
   adding `--diff` to see which instructions differ. Change one thing at a
   time and keep only what improves the verdict.
6. When the instructions are right and only `%hi` handling differs (more
   saved registers or a bigger frame than retail, or a `lui` retail
   repeats), spend one run on `-mno-split-addresses` (LEVERS.md lever 10)
   and report what it did.
7. When only registers differ, run
   `bash tools/docker/run.sh python tools/regalloc.py <func> build-sn/try/<func>/pN.c`
   once (it does not count against the budget): it prints the order the
   allocator took the variables in and their priorities, so change what
   outranks or overlaps the one in the wrong register (LEVERS.md lever 13).
8. Stop at `EXACT`, when the budget runs out, or when three variants in a
   row compile to the same bytes: that is an allocator or scheduler tie
   that rewording won't move. Note where it is and stop. Matches come
   early: in the waves so far, every function under 600 bytes that
   matched did so within 9 runs.

## First compile

Only make the m2c sketch compile; don't try to match.

1. Read `CONTEXT.md`.
2. Copy `m2c.c` to `p0.c` and fix it until `try_func` gives any verdict
   other than `COMPILE`:
   - replace m2c's `?` and unknown types with `int`, `float` or `void *`,
     as the assembly uses them;
   - delete m2c's own `extern` lines and use the declarations in
     `CONTEXT.md` instead;
   - keep m2c's structure; don't rewrite the logic.
3. `RESULT.md`: the verdict and `p0.c`. `NOTES.md`: one line on what you
   changed.
