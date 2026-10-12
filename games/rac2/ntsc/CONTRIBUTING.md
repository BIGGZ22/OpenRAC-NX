# Contributing

## Matching a function

1. Set up the build once ([docs/BUILDING.md](docs/BUILDING.md)) and run `tools/build.py all`.
2. Pick a function from `config/boot/functions.tsv` or `config/levels/functions.tsv`. A level
   function's row lists all its places; one definition serves them all.
3. Find its `INCLUDE_ASM` line in `src/` (`grep -rn func_XXXXXXXX src`) and replace it with
   its C, in place. Name what it uses by what it is: other functions by their catalogue names,
   the boot's resident data `D_XXXXXXXX`, a level's data `D_LNN_XXXXXXXX` (its address in level
   NN). Never write a level address as a number: a level function is linked into every level
   that has it, each with its own addresses.
4. `tools/check.py func_XXXXXXXX` compiles its file and tries it at each of its places in a few
   seconds; `--diff` shows the instructions beside the retail ones.
5. `tools/build.py` the programs that have it (`boot`, `level 4`, or `all`). It places the
   function everywhere the catalogue says and prints `OK` per program, or why a place does
   not match. A function counts once its C links into a program that equals the retail one.
6. Leave `progress/report.json` and the README table alone: a maintainer regenerates them on
   `master` after merging, so pull requests do not conflict on them (AGENTS.md, "The progress report").

Close but not matching: put it in `nonmatching/` with a line in its README.

## Sending your work

Branch from `master`, commit, push the branch and open a pull request to `master`. In the pull
request, give the commands you ran and what they printed (`tools/build.py all`,
`tools/progress.py`, the tests). CI runs the tests and the repository check; a maintainer merges.
Never push to `master` and never force-push a shared branch. AGENTS.md has the full rules and docs/AGENT_WORKFLOW.md says which model takes which functions, for
people as much as for agents.

## Rules

- **Sources.** Use the retail programs of your own disc, SDK library *binaries*, open-source code
  under its licence, public decompilations and tools, and public hardware documentation. Never
  use Sony's SDK source, samples or headers, or leaked or NDA material of any kind (leaked
  builds, symbol dumps from them, leaked source), not even as a reference. Publicly distributed
  prototype discs may be cited, naming the build, never as the evidence for a match. OpenRAC's
  docs/policy/SOURCING.md is the full policy.
- Plain C. No inline assembly in a function, no register pins, no tricks to defeat the
  compiler. Flags are per file (`config/cflags.txt`), never per function.
- The build only does what the retail assembler and linker did; nothing edits compiler
  output.
- Never commit game data, extracted assembly, disc images or proprietary tools: `baserom/`,
  `toolchain/` and `build/` stay local.
- Conventional commit subjects (`feat(src):`, `fix(tools):`, `docs:` ...), with a body that
  says what changed and how it was checked.
- `python -m unittest discover -s tests`, `python tests/check_repository.py` and
  `python tools/progress.py --check` pass (CI runs
  them; it cannot build, having no game data).
