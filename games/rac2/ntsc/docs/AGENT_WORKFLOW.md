# Matching with agents

How to spread matching work over AI agents without wasting the expensive ones. AGENTS.md has the
rules every agent follows; this is about who does what. The same tiers are used in rac1-decomp.

## Which model for which function

Size is the first sorting key because it predicts difficulty well: a small function has few
ways to be written, a large one has many and needs a model that can hold the whole of it.

| Tier | Model | Functions | At once |
| --- | --- | --- | --- |
| Small | Haiku 5.5 | up to 400 bytes | 8 to 10 workers |
| Medium | Sonnet 5.5 | 401 to 1,000 bytes | 3 to 5 workers |
| Large or hard | Opus 5.5 | over 1,000 bytes; anything a lower tier gave up on; near misses in `nonmatching/` | 2 to 4 workers |
| Lead | Opus 5.5 | hands out work, runs the full build, lands, writes the report | one |

With another vendor's models, use its smallest, middle and largest in the same places.

Opus is only for what needs it. Do not give it a 60-byte accessor, and do not give Haiku a
2 KB state machine: the first burns money, the second burns time and returns nothing.

`python3 tools/todo.py` lists what is left by tier (it needs no game data):

```sh
python3 tools/todo.py                       # how much each tier has
python3 tools/todo.py --max 400             # small: for Haiku
python3 tools/todo.py --min 401 --max 1000  # medium: for Sonnet
python3 tools/todo.py --min 1001            # large: for Opus
```

It puts the functions with the most places first: a level function shared by all 27 levels
counts once in the progress, but one match removes it from 27 programs.

## Before any model: tools

Much of the easy work needs no model at all. Before a wave, check whether a function is
byte-identical to one already matched in rac1-decomp (docs/RAC1-TO-RAC2.md) or to another
function already matched here; those are carried over, not rewritten.

## Escalation

1. A Haiku worker gets a handful of small functions. For each it either matches (`tools/check.py`
   says so at every place) or stops after a bounded effort, about ten compiles without progress,
   and leaves its best attempt.
2. What Haiku left goes to Sonnet, with the attempt. Starting from a near miss is cheaper than
   starting from the assembly.
3. What Sonnet left, or anything within a few bytes after real effort, goes to Opus. So do
   functions with the known hard patterns: register allocation that will not settle, switch
   tables, long float expressions, delay-slot ordering.
4. What Opus leaves stays in `nonmatching/` with a line in its README saying how many bytes
   differ and what was tried. Do not loop on it; come back when a similar function is solved.

A worker never moves up a tier by itself and never keeps going past its bound. Giving up early
and cleanly is the job.

## What a worker is told

Give each worker a brief with exactly this, nothing else to look up:

- its functions (name, size, file), from `tools/todo.py`;
- to read AGENTS.md first;
- the loop: replace the `INCLUDE_ASM` marker in place, `python3 tools/check.py <func> --diff`,
  repeat;
- its bound, and where to leave what it could not finish (`nonmatching/<area>/<func>.c`, marker
  put back);
- what it must not do: no commits to `master`, no push, no deleting what it did not create, no
  edits to the catalogues, `progress/` or other workers' files, no new tools or documents;
- what to report: for each function, matched or not, the last `tools/check.py` line, and anything
  it learned about the compiler that the next worker should know.

Three things every brief should say, because the first trial (16 Haiku workers, 128 small level
functions) lost a third of its misses to them:

- `tools/check.py` only finds a function that has a C definition, so put a stub in place of the
  marker first; `--diff` then shows retail's instructions.
- `$gp`-relative access needs a `-g8.c` sibling file, and a call whose target differs per level
  needs a `ref_LNN_XXXXXXXX_kk` name (AGENTS.md, "Naming in C").
- A catalogue row can be wrong: bytes with no `jr ra`, or a body that starts mid-function, are
  not a function. Report it and move on; that is a fix in `tools/levels.py`, not in C.

A worker's claim is not a match. Only the lead's `tools/build.py` is.

## Keeping workers apart

Each worker gets its own git worktree and branch (`git worktree add ../rac2-w3 -b match/w3
origin/master`) and functions nobody else has. Hand out whole files where you can: two workers
editing one file is the usual source of conflicts. There is no queue, claim file or reservation
system to maintain; the lead's list of who has what, in its own notes, is enough.

Workers need the build environment (docs/BUILDING.md) but not a full build: `tools/check.py`
compiles one file and compares one function in seconds.

## Landing

The lead, for each finished worker:

1. takes the worker's branch into its own (`git merge` or cherry-pick), one worker at a time;
2. runs `python3 tools/build.py all` (it builds the programs in parallel; give it the cores):
   every program must print `OK`. A function that passed
   `tools/check.py` but breaks a program goes back to `nonmatching/`, it is not fixed up by hand
   in the build;
3. runs `python3 tools/progress.py`, the tests, `tests/check_repository.py` and
   `tools/progress.py --check`;
4. commits (one function or one family per commit, the report in its own commit) and opens the
   pull request as AGENTS.md describes, with the build output in it.

Lasting knowledge a worker found (a compiler habit, a pattern that keeps coming back) goes into
docs/COMPILER-NOTES.md. Nothing else is written down per wave: no wave logs, lot documents or
registers.

## Cost sense

- Start a wave only when a human asked for one.
- Prefer many small Haiku briefs over a few long ones: a worker that has drifted costs more than
  a fresh one.
- Stop a tier when its yield drops: if a Haiku wave matches under one function in five, the easy
  ones are gone and the rest belong one tier up.
- Never run Opus on the small tier "to be safe". If the small ones resist Haiku, there is
  usually a shared cause (flags, a header, a compiler habit) worth one Opus look, not a hundred.
