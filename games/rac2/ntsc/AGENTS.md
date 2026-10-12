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
| `progress/report.json`, the README table | progress | `tools/progress.py` only |
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
5. `python3 tools/progress.py` and commit (see Commits).

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
- `r1_...` names were carried from rac1-decomp and have no rac2 name yet: rename them to the
  `D_`/`func_` name of their address when you know it.

## Rules

- **Sources.** Use the retail programs of your own disc, SDK library *binaries*, open-source code
  under its licence, public decompilations and tools, and public hardware documentation. Never
  use Sony's SDK source, samples or headers, or leaked or NDA material of any kind (leaked
  builds, symbol dumps from them, leaked source), not even as a reference. Publicly distributed
  prototype discs may be cited, naming the build, never as the evidence for a match. OpenRAC's
  docs/policy/SOURCING.md is the full policy.
- **Plain C.** No inline assembly in a function, no register pins, no artificial barriers.
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
- **No campaign machinery**: no queues, registers, reservations, lots, "owner-local" publication
  routes, merge-queue scripts or PR-description validators.
- **No document per batch of work** ("Nth C lot", evidence notes, experiment registers). Put what
  was learned in a commit message, or in an existing doc if it is lasting knowledge.
- **No progress that counts a function more than once.** `tools/progress.py` counts each distinct
  function once.

## Commits

- Conventional commit subjects with a scope: `feat(src):`, `fix(tools):`, `build(compiler):`,
  `docs:`, `chore:`. The body says what changed and how it was checked: the commands and what
  they printed (e.g. "tools/build.py all: 28 programs OK; tools/progress.py: 1,028 -> 1,031").
- Small, coherent commits: one function or one small family per commit is fine.
- Commit `progress/report.json` and the README table only as `tools/progress.py` wrote them, after
  a full `tools/build.py all`.

## Pushing changes

1. Work on a branch from an up-to-date `master` (`git switch -c match/<what> origin/master`).
   Never commit to `master` directly and never push to it.
2. Before pushing, run all of these and keep their output for the PR:
   - `python3 tools/build.py all` (every program `OK`),
   - `python3 tools/progress.py`,
   - `python3 -m unittest discover -s tests`, `python3 tests/check_repository.py` and `python3 tools/progress.py --check`.
3. Push your branch and open a pull request to `master` with the template filled in: what you
   matched or changed, the commands you ran and what they printed, and what you did not run.
4. CI runs the tests and the repository check; it cannot build (it has no game data), so the
   build result in the PR is yours to report truthfully. A maintainer reviews and merges.
5. Never force-push a shared branch, never rewrite `master`, never merge your own PR unless you
   are a maintainer and the checks pass.

If several agents work at once, give each its own branch (or worktree) and its own functions;
they meet in pull requests, not in a shared queue or reservation system.

## If something seems to need a new mechanism

Stop and ask a human first. A new tool belongs in `tools/` with a test and a short docstring; a new
kind of committed data needs a reason it cannot be regenerated in `build/`. The repository check
(`tests/check_repository.py`, run by CI) rejects the old patterns: gzipped or oversized files,
generated sources, per-level copies, per-batch documents and extra files under `progress/`.
