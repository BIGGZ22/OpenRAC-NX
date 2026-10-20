# Instructions for agents

This file is for AI agents (Claude, Codex, Qwen or others) working in this repository. Read it
completely before changing anything; then read README.md, CONTRIBUTING.md, docs/BUILDING.md and
docs/LEVELS.md. The repository follows the same structure as rac1-decomp and the other OpenRAC
decompilations: keep it that way.

## What this repository is

A matching decompilation of Ratchet & Clank: Going Commando (PS2, USA v1.01, `SCUS_972.68`):
the boot executable and 27 level programs, rebuilt from C and assembly until every loaded byte
equals the retail programs.

| Path | What it is | Who writes it |
| --- | --- | --- |
| `src/` | every function once, as C or as an `INCLUDE_ASM` marker | you (C replacing markers) |
| `config/boot/functions.tsv`, `config/levels/functions.tsv` | the function catalogues: names, sizes, places | `tools/levels.py catalogue` only |
| `config/levels/boot_pairs.tsv` | two measured pairings the catalogue cannot infer | by hand, rarely |
| `config/cflags.txt` | compiler flags per file | by hand, when a whole file needs other flags |
| `include/` | headers shared with rac1-decomp | by hand |
| `nonmatching/` | near misses, never linked | you |
| `progress/report.json`, the README table | progress | `tools/progress.py` only, on `master` by a maintainer |
| `tools/` | disc reader, build, check, progress, compiler recipes | by hand, with tests |
| `docs/` | how to build, the level format, the compiler, game research | by hand |
| `baserom/`, `build/`, `toolchain/`, `work/` | game data, build output, proprietary tools, scratch | never committed |

## How a function is matched

1. Pick a function with an `INCLUDE_ASM` marker. Its catalogue row tells you its size and every
   place it has (a level function may be in all 27 levels: it is still one function).
2. Replace the marker with C, in place, in the same file. One definition per function, never a
   copy per level.
3. `python3 tools/check.py func_XXXXXXXX --diff`: compiles the file and tries the function at
   each of its places in seconds.
4. `python3 tools/build.py boot` / `level N` / `all`: the real check. A function counts only when
   it links into a program that equals retail byte for byte; the build prints `OK` per program.
5. Commit (see Commits). Do not regenerate or commit the progress report: see "The progress
   report" below.

If you get close but not exact, move your C to `nonmatching/<area>/func_XXXXXXXX.c` with a line in
`nonmatching/README.md`, and put the `INCLUDE_ASM` marker back.

## Naming in C

- Functions: their catalogue name (`func_XXXXXXXX` for the boot, `func_LNN_XXXXXXXX` for level
  functions). Readable names go in `include/names.h` as `#define`s, never by renaming the function.
- Data: `D_XXXXXXXX` for the boot's resident data (below `0x1A7C80`, the same in every program),
  `D_LNN_XXXXXXXX` for a level's data (its address in level NN).
- **Never write a level address as a number.** A level function is linked into every level that
  has it, each with its own addresses; the build reads each symbol's value per program from the
  retail bytes. A hard-coded address only works in one level.
- **A call or symbol whose target differs from level to level** gets no address in its name:
  `ref_LNN_XXXXXXXX_kk`, the kk-th such external of function `func_LNN_XXXXXXXX` (declare it
  `extern`). `tools/check.py` tells you when you need one: the function is `OK` in some levels
  and `<name> would be at 0x...` in others.
- **Small data reached through `$gp`** (`lw v0,0xBD0($gp)`, `addiu v0,$gp,-0x1388`) only comes
  out of a file built with `-G8`. Put such a function in a sibling file named `<file>-g8.c`
  (it gets `-G8` by its name; see `config/cflags.txt`) and take its marker out of the original.
  If its size is one or two instructions over retail and the difference is a `lui`, this is why.
- `r1_...` names were carried from rac1-decomp and have no rac2 name yet: rename them to the
  `D_`/`func_` name of their address when you know it.

## Handwritten assembly

Some of the game was written in assembly: VU0 vector routines, the renderer's inner loops,
a few wait loops. No C compiles to them, so they stay assembly and are marked
`ASM_FUNC("asm/...", name);` in place of `INCLUDE_ASM`. `tools/progress.py` counts an `ASM_FUNC`
function as finished, as rac1-decomp does.

Mark a function `ASM_FUNC` only on evidence, never because it resisted matching:

- it is byte-identical (apart from addresses) to a routine rac1-decomp classifies as
  handwritten, and longer than 16 bytes, so the identity is not a coincidence; or
- it uses instructions the compiler never emits (COP2/VU0 macro instructions, `$at` as a
  working register, a return with a live instruction after its delay slot) and a maintainer
  agrees in review.

Say which in the commit. A function that is merely hard is a near miss, not handwritten.

## When to stop on a function

Some functions cannot be matched from C, and some have cost enough. Know which before another
attempt; OpenRAC's `docs/workflow/BACKFILL.md` has the measurements and the full procedure.

- **Not a function.** It reads a register it never sets, branches outside its own bytes, or has
  no `jr $ra`: a piece of its neighbour that the catalogue split off. Do not write C; report it,
  it is a fix in `tools/levels.py`.
- **Handwritten.** COP2/VU0 instructions, `$at` as a working register, a live instruction after
  the return's delay slot, a trapping `addi`: see "Handwritten assembly" above.
- **A tie.** Three different wordings that compile to the same bytes: a fourth will too. So will
  another compiler or another flag: on rac1, three other compilers fixed none of 492 near misses.
- **Out of budget.** About ten compiles without getting closer.

Then stop: put the marker back, leave the attempt in `nonmatching/<area>/` with what differs and
what would unblock it, and let it go up one model tier (docs/AGENT_WORKFLOW.md). Stopping is a
normal result. C that behaves like the function without matching it is useful to the native port,
never to `src/` or the progress report.

## Use the machine

Builds and checks are the slow part of this work, and most of them are independent. Run them
side by side instead of one after another:

- `python3 tools/build.py all` builds the 28 programs in parallel by itself (`--jobs N`). Never
  build them one at a time in a loop.
- `tools/check.py` runs for different functions can overlap; so can the tests, the repository
  check and a build.
- When verifying many functions (after a wave, after a merge), fan the checks out, eight or so at
  a time, and collect the verdicts.
- Start long steps in the background and keep working; do not sit on a build.

The exception: never run two builds of the same program in the same checkout at once. Workers
each have their own worktree for that reason (docs/AGENT_WORKFLOW.md).

## Rules

- **Sources.** Use the retail programs of your own disc, SDK library *binaries*, open-source code
  under its licence, public decompilations and tools, and public hardware documentation. Never
  use Sony's SDK source, samples or headers, or leaked or NDA material of any kind (leaked
  builds, symbol dumps from them, leaked source), not even as a reference. Publicly distributed
  prototype discs may be cited, naming the build, never as the evidence for a match. OpenRAC's
  docs/policy/SOURCING.md is the full policy.
- **Plain C.** No inline assembly in a function, no register pins, no artificial barriers.
- **English.** Everything that lands here is in English: code, comments, commit messages,
  issues, pull requests, reviews, and the notes kept in `nonmatching/`. Translate before it is
  published.
- **Flags per file, never per function** (`config/cflags.txt`). If one function needs other flags,
  it belongs in a file of its own with those flags.
- **Nothing edits compiler output.** The build only does what the retail assembler and linker did.
- **A match is only `tools/build.py` saying OK.** Not a compiled object, not a masked comparison,
  not a model's claim. Report exactly what you ran and what it printed.
- **Never commit game data or proprietary tools**: disc images, extracted programs, disassembly,
  SN ProDG binaries. `baserom/`, `build/` and `toolchain/` are ignored; keep it so.
- **Keep the catalogue generated.** Do not edit `config/*/functions.tsv` by hand; change
  `tools/levels.py` and rerun `catalogue`.
- Before a commit: `python3 -m unittest discover -s tests`, `python3 tests/check_repository.py` and `python3 tools/progress.py --check`.

## Do not bring back what was removed

An earlier version of this repository grew a parallel bureaucracy that made it 1.5 GB and hard to
work in. It was removed on purpose. Do not recreate any of it:

- **No per-level copies of a function** (`LVL_<n>_<LEVEL>_FUN_...` names, one C copy per level).
  One definition, placed everywhere by the catalogue.
- **No generated sources in git** (`candidates/`, assembled `.c` units, `.cfrag` fragments with
  placeholders and recipes). The C in `src/` is the source.
- **No committed generated data**, above all no gzipped files: no per-level catalogues, no
  evidence or binding archives, no code-reuse reports. Anything regenerated belongs in `build/`.
- **No proofs, receipts, attestations or hash ledgers** for matches. The build is the proof and
  anyone can rerun it.
- **No in-repository campaign machinery**: no committed queues, registers, reservation databases,
  lots, "owner-local" publication routes, merge-queue scripts or PR-description validators.
  Contributor scope coordination through issue #152 is separate from this rule and must not be
  represented by committed reservation files or databases.
- **No document per batch of work** ("Nth C lot", evidence notes, experiment registers). Put what
  was learned in a commit message, or in an existing doc if it is lasting knowledge.
- **No progress that counts a function more than once.** `tools/progress.py` counts each distinct
  function once.

## Commits

- Conventional commit subjects with a scope: `feat(src):`, `fix(tools):`, `build(compiler):`,
  `docs:`, `chore:`. The body says what changed and how it was checked: the commands and what
  they printed (e.g. "tools/build.py all: 28 programs OK; 3 functions added").
- Small, coherent commits: one function or one small family per commit is fine.
- Leave `progress/report.json` and the README table out of your commits (see "The progress
  report").

## The progress report

`progress/report.json` and the README table are generated, and every pull request that
regenerates them conflicts with every other one. So pull requests do not touch them:

- In a pull request, say what `tools/build.py all` printed and how many functions you added.
  `tools/progress.py --check` passes with the report behind; it only fails if the report counts
  something that is no longer C.
- After merging, a maintainer runs `tools/build.py all` and `tools/progress.py` on `master` and
  commits the two files alone (`chore(progress): N of 9,101 functions`). Several merged pull
  requests can share one regeneration.
- If your branch already has the two files changed, restore them before pushing:
  `git checkout origin/master -- progress/report.json README.md`.

## Pushing changes

1. Work on a branch from an up-to-date `master` (`git switch -c match/<what> origin/master`).
   Never commit to `master` directly and never push to it.
2. Before pushing, run all of these and keep their output for the PR:
   - `python3 tools/build.py all` (every program `OK`),
   - `python3 -m unittest discover -s tests`, `python3 tests/check_repository.py` and `python3 tools/progress.py --check`.
3. Push your branch and open a pull request to `master` with the template filled in: what you
   matched or changed, the commands you ran and what they printed, and what you did not run.
4. CI runs the tests and the repository check; it cannot build (it has no game data), so the
   build result in the PR is yours to report truthfully. A maintainer reviews and merges.
5. Never force-push a shared branch, never rewrite `master`, never merge your own PR unless you
   are a maintainer and the checks pass.

If several agents work at once, give each its own branch (or worktree) and its own functions.
Coordinate contributor scopes through issue #152; agents within the same contributor's claimed
scope may divide work locally. Their code meets in pull requests, not through committed queues or
reservation files.

## Which model does what

docs/AGENT_WORKFLOW.md is the whole procedure. In short, sort by size with `python3 tools/todo.py`:

- **Haiku 5.5**: small functions, up to 400 bytes, many workers at once.
- **Sonnet 5.5**: 401 to 1,000 bytes, and what Haiku gave up on.
- **Opus 5.5**: only functions over 1,000 bytes, complex ones, near misses in `nonmatching/`, and
  leading (the full build, landing, the report).

A worker stops at its bound and hands its attempt up a tier; it never keeps grinding.

## Coordinating work between contributors

All contributors actively matching functions must coordinate through the shared GitHub issue:

https://github.com/OpenRAC/rac2-gc-decomp/issues/152

The purpose of this issue is to prevent contributors and their AI agents from duplicating work.

This is a lightweight coordination noticeboard only. It is **not** a task queue, campaign
register, reservation database, match proof or source of truth for completed work. The repository
and `master` remain authoritative.

### Every active contributor must have one comment

Each contributor working on the project must have exactly **one persistent coordination comment**
in issue #152.

If you are beginning work and do not already have your own comment in that issue, you are
**required to create one before starting matching work**.

Each contributor edits only their own comment. Do not modify another contributor's comment.

Do not create a new comment every time work changes. Update your existing comment in place.

AI agents acting on behalf of a contributor must verify that their contributor has a coordination
comment before beginning or assigning matching work. If no comment exists, create one before
continuing.

### Describe only your own reservations

List only your own current reservations in your comment. Do not copy other contributors' claims,
add lists of scopes you promise not to touch, or state that another contributor still reserves a
scope. Those copies become stale when its owner finishes or releases the work.

Before choosing work, check the other contributors' current comments for overlap. Each
contributor's own comment is the reference for that contributor's active reservations; another
person's comment does not reserve work on their behalf. Update your own comment when you finish,
release or replace your scope.

### Scope selection and reservation limit

Reserve a manageable amount of work that you intend to attempt next. Prefer small,
clearly defined scopes rather than entire directories.

Each contributor may reserve a maximum of **500 unfinished functions at one time**,
counted across all their active scopes, branches, worktrees and AI agents combined.
This is a per-contributor limit, not a per-agent or per-comment limit.

Count each distinct catalogue function once, regardless of how many program placements
it has. Functions already implemented on current `origin/master` do not count toward
the limit.

A scope may be a source file, a related group of files, a level, or an explicit list
of functions, provided its total does not exceed the contributor's remaining allowance.
If a directory or file contains too many unfinished functions, reserve a smaller
sub-scope or list the specific functions being reserved.

Do not reserve additional functions indirectly through broad directory claims,
multiple comments, alternate branches or additional AI agents.

### Bringing existing reservations within the limit

Contributors currently reserving more than 500 unfinished functions must update their
existing coordination comment to retain no more than 500 and explicitly release the
remainder. This requirement applies to existing reservations as well as new ones.

Until an oversized reservation is narrowed, other contributors must request
coordination rather than assume which functions have been released. The contributor
holding the oversized reservation must identify the retained scope clearly; a
maintainer should resolve any dispute or lack of response.

### Updating a reservation

Contributors may request additional functions at any time, provided the resulting
active reservation never exceeds 500 unfinished functions.

Before adding or replacing reserved functions:

1. Fetch `origin/master` and check which functions remain unfinished.
2. Read the current comments in issue #152 and check for overlapping reservations.
3. Release enough existing functions if necessary to remain within the limit.
4. Update your one persistent coordination comment with the retained, released and
   newly reserved scopes.
5. Re-check for overlap before starting work.

A function remains reserved until its implementation reaches `origin/master` or the
contributor explicitly releases it. Local matches, local commits and open pull
requests still count toward the limit while the function remains unfinished upstream.

### Active scopes are exclusive

Do not work inside another contributor's active scope without their explicit
agreement. Directory and file claims include their unfinished functions, subject
to the 500-function limit.

An individual-function reservation does not reserve the surrounding source file.
Coordinate edits when contributors work on different functions in the same file.

AI agents must verify their contributor's current scope and combined reservation
count before beginning or assigning matching work. Agents must not expand a
reservation beyond 500 functions or modify another contributor's comment.

Contributors with oversized reservations must narrow them before starting or assigning further matching attempts. 
They may finish attempts already in progress, but must not start new ones until their reservation complies with the limit.

These rules apply to all matching work, including AI-assisted work and matches
carried from another OpenRAC decompilation.

## If something seems to need a new mechanism

Stop and ask a human first. A new tool belongs in `tools/` with a test and a short docstring; a new
kind of committed data needs a reason it cannot be regenerated in `build/`. The repository check
(`tests/check_repository.py`, run by CI) rejects the old patterns: gzipped or oversized files,
generated sources, per-level copies, per-batch documents and extra files under `progress/`.
