# Matching workflows

This page takes you from "I want to decompile a function" to "my change is
proven and committed" in any of OpenRAC's games. It also compares how the four
projects work, so that what one does well can spread to the others.

Each game's own docs decide how a match is proven. This page links to them and
copies their commands as they stood when the projects were imported
(2026-10-03). If a command here disagrees with a game's docs, the game's docs
win. Run game commands from that game's directory (for example
`games/rac1/pal/`). Run [`tools/openrac.py`](../../tools/openrac.py) from the
OpenRAC root.

## 1. The common loop

All four projects follow the same seven steps. They differ in their tools and
in how strict the final proof is.

| Step | What it means | Where the projects differ |
|---|---|---|
| 1. Pick | A function nobody has matched yet: preferably a small one, or a relative of one already matched | rac1/pal and rac3 have triage tools, rac1/ntsc lists pending units, and rac2 has no function catalogue yet |
| 2. Read | The retail assembly (generated locally from your own disc, never committed), plus the function's callers, callees and globals | In all four, whether a global is reached through `$gp` or through `lui` decides how it must be declared |
| 3. Write C | Plain C in the project's types and naming | Rules on inline assembly differ (section 3) |
| 4. Check | Compile the candidate on its own and compare it with retail | Relocations are masked, resolved, or linked for real |
| 5. Prove | Rebuild in context and compare with retail | Whole image, whole program, or one symbol plus every loaded byte |
| 6. Record progress | Update the game's progress report, then OpenRAC's | rac1/pal commits its report by hand, rac1/ntsc and rac3 have CI derive it, and rac2 exports it from committed proofs |
| 7. Commit | One granular commit per change | Same everywhere in OpenRAC (section 2) |

At no step does retail-derived material enter the repository: no disc images,
executables, level programs, generated `asm/`, objects made from retail, or
build output ([docs/policy/SOURCING.md](../policy/SOURCING.md)).

## 2. OpenRAC-wide rules

The full rules are in [CONTRIBUTING.md](../../CONTRIBUTING.md#commits) and, for
agents, [AGENTS.md](../../AGENTS.md). In short:

**Commits** follow Conventional Commits, `type(scope): summary`.

- Types: `feat`, `fix`, `docs`, `refactor`, `perf`, `test`, `build`, `ci`,
  `chore`, `style` (CONTRIBUTING.md also lists `revert`, and says when to use
  each type).
- Scope: the game version (`rac1/pal`, `rac1/ntsc`, `rac2`, `rac3`, `rac4`)
  or a component (`editor`, `tools`, `docs`, `progress`, and the others listed
  there).
- Keep commits granular: each one builds, and the game's own gate still passes.
- Inside OpenRAC this replaces the per-project styles: Lombyte's
  [docs/commit-messages.md](../../games/rac1/ntsc/docs/commit-messages.md),
  rac2's [AGENTS.md](../../games/rac2/ntsc/AGENTS.md) and
  [.gitmessage](../../games/rac2/ntsc/.gitmessage), and the format in rac1/pal's
  [LLM_DECOMP_INSTRUCTIONS.md](../../games/rac1/pal/docs/LLM_DECOMP_INSTRUCTIONS.md).
  The projects' own examples carry over directly: Lombyte's
  `decomp: promote fun_002212b8 (424 B)` becomes
  `feat(rac1/ntsc): promote fun_002212b8 (424 B)`, and rac1/pal's
  `feat(overlays): func_L00_00235608 exact match` becomes
  `feat(rac1/pal): func_L00_00235608 exact match`. rac2's guidance for
  message bodies (the change, its reason, its measured scope, and the checks
  actually run) still fits.

Some imported tools and docs still write or enforce the old styles:

| Tool or doc | What it does | What to do in OpenRAC |
|---|---|---|
| rac1/ntsc [scripts/commit-msg.py](../../games/rac1/ntsc/scripts/commit-msg.py) | An optional `commit-msg` hook. It accepts only `<type>: <summary>` with Lombyte's six types, removes `Co-Authored-By:` trailers, and rejects anthropic.com and openai.com identities | Do not install it: it rejects every `type(scope):` subject and strips the trailer CONTRIBUTING.md asks for. Its install line (`ln -sf ../../scripts/commit-msg.py .git/hooks/commit-msg`) also assumes `games/rac1/ntsc` is a repository of its own |
| rac1/pal `tools/wave.py land` without `--batch` | Commits each match itself; inside OpenRAC as `feat(rac1/pal/<unit>): ...` or `feat(rac1/pal/overlays): ...` | Fine for single matches; use `land --batch` and commit yourself for a batch |
| rac3 [localdecomp/server.py](../../games/rac3/ntsc/localdecomp/server.py) | On Save, commits every perfect match as `localdecomp: match func_X`, and offers a Push button, when its project is the top of its own git work tree | Nothing: inside OpenRAC both stay off. `--no-git-sync` turns them off in a standalone checkout |
| rac1/pal LLM_DECOMP_INSTRUCTIONS.md, step 8 | One function per commit as `<unit>: <FunctionName> (func_XXXXXXXX) exact match`, then `git push origin main` | Its step 8 now carries a note: use the OpenRAC subject, keep the report in the commit, push only when asked ([AGENTS.md](../../AGENTS.md), rule 7) |

**Progress.** Each game keeps its own report (section 3). The consolidated
[progress/](../../progress/README.md) is regenerated, never edited, with
`python3 tools/openrac.py progress [--fetch]` (`--fetch` first copies
Lombyte's published report). Large regenerated output goes in its own
`chore(progress)` commit, except rac1/pal's `progress/report.json`, which goes
in the same commit as the functions it counts: OpenRAC's CI
([checks.yml](../../.github/workflows/checks.yml)) runs its `--check` on every
push, as rac1-decomp's own CI did ([CONTRIBUTING.md](../../CONTRIBUTING.md#commits)).

**Matching rules stay per project.** Section 3 lists what each project accepts
as proof. A match in one game proves nothing in another: rac2's
[RAC1-TO-RAC2.md](../../games/rac2/ntsc/docs/RAC1-TO-RAC2.md) does not accept
RAC1's compiler profile for RAC2 until it has been measured there.

**Sourcing** ([docs/policy/SOURCING.md](../policy/SOURCING.md)). Never use
Sony SDK source, samples or headers, or leaked or NDA material, not even to
check a match; rac1/pal reverted its movie code to assembly on 2026-09-30 for
exactly this. Write every reuse down (section 6). Refer to the third-party
NTSC decompilation of Ratchet & Clank only as "the NTSC decomp"
([Removal requests](../policy/SOURCING.md#removal-requests)).

## 3. Quick reference

| | rac1/pal | rac1/ntsc | rac2 | rac3 |
|---|---|---|---|---|
| Target | `SCES_509.16` and its 19 level programs | `SCUS_971.99` and its 19 level programs | `SCUS_972.68` and its 27 level overlays | `frontbin.elf`; level programs and other executables are counted, and level C is opt-in |
| Unit of work | One function: `func_X` in the executable, `func_LNN_X` in level code | One unit file per function under `src/` | A catalogued symbol in `candidates/boot.c`, or a level's own source | One function block in its `src/frontbin/` file |
| Game-code compiler | SN GCC 2.95.3, `-O2 -G2`; Sony's 2.9-ee for SDK code and libgcc | EE-GCC `2.9-ee-991111b` built from source with the project's patch stack, assembled by `Ps2EeAs`; SDK compiler `2.9-ee-991111-01` | SN ProDG 3.01 `ee-gcc2953`, `-O2 -G0 -ffunction-sections`, qualified per function | SN ee-gcc 2.95.3 v1.36, `-O2 -G8 -fopt-stack -mno-check-zero-division`, flags per address range |
| Inner-loop check | `try_func.py`: relocations masked for executable functions; linked at the real address for level functions | `check-unit.py`: objdiff, which ignores relocations, plus a strict pass over non-text sections | `check_candidates.py`: standalone link, complete symbol | `try_func.py`: relocations resolved to real addresses |
| Final proof | Full-build audit (size and bytes of each function at its retail address); for level code, the strict check plus the whole file still exact | `make elf`: SHA-256 of the whole boot ELF | Every loaded byte of both PT_LOAD segments, for the boot and each overlay touched | `make` prints `MATCH` (frontbin SHA-1), and `pr_check.py` prints `OK` |
| Inline asm in a match | Refused, apart from `qcopy()` and file-scope aliases | Only as a name label or the `qcopy.h`/`qzero.h` idiom; any other asm keeps the unit pending | Refused by the checker | Accepted for VU0/MMI leaves, called out in the PR |
| Near misses kept | `nonmatching/` (level functions), scored as fuzzy; same-size executable near misses may stay in `src/` with notes | C under `#else` of `NON_MATCHING`, scored as C_FUZZY | Experiment register; the evidence stays local | localdecomp drafts, local only |
| Progress report | `progress/report.json`, committed | None committed: CI publishes it | `progress/*.json` proofs, exported by `decomp_report.py` | CI artifact; `progress_report.json` is also committed |
| Build host | Docker on macOS and Linux, or native Windows | Linux or WSL; macOS through `--docker` | Windows with WSL | Windows; Linux and macOS through wibo |

Disc inputs come from your own images in `baserom/`:
`python3 tools/openrac.py discs` identifies them, and
`python3 tools/openrac.py setup [GAME/VERSION ...]` places each game's inputs.
Each `games/*/game.json` has a `setup` line with the remaining steps.
[toolchains/README.md](../../toolchains/README.md) says where to get the
compilers, and [docs/toolchains](../toolchains/README.md) compares them.

### rac1/pal: `games/rac1/pal`

On macOS and Linux, run the `python`, `sh` and `bash` commands through
`bash tools/docker/run.sh`, as the setup row shows
([CONTRIBUTING.md](../../games/rac1/pal/CONTRIBUTING.md)). Commands written
with `python3` run on the host.

| Step | Command |
|---|---|
| Set up | `bash tools/docker/run.sh bash tools/setup_asm.sh`, then `bash tools/docker/run.sh bash tools/build_sn.sh` |
| List | `python tools/triage.py`, `python tools/rank_candidates.py`, `python3 tools/lombyte.py todo`; near misses in [nonmatching/README.md](../../games/rac1/pal/nonmatching/README.md) |
| Sketch | `sh tools/gen_ctx.sh`, then `python tools/m2c.py func_XXXXXXXX` |
| Try | `python tools/try_func.py func_XXXXXXXX candidate.c --diff` |
| In context | Executable: `bash tools/build_sn.sh`. Level code: `try_func` is already strict; `python tools/overlay_file_check.py [FILE ...]` re-checks whole files |
| Land | `python tools/apply_candidate.py func_X cand.c`, or `python tools/integrate.py MANIFEST --apply`; then `bash tools/build_sn.sh` |
| Progress | `python tools/gen_progress_report.py`, then `python3 tools/gen_progress_report.py --check` |

### rac1/ntsc: `games/rac1/ntsc`

| Step | Command |
|---|---|
| Set up | `./setup.sh --elf config/us/SCUS_971.99` (add `--docker` on macOS), then `make elf` |
| List | `python3 scripts/list-functions.py --score` |
| Try | `python3 scripts/check-unit.py assembly/textbin/runtime/memory/clear_u64_value`, until it prints `Object matches (promotable).` |
| In context | `make elf`, which must end with `PASS: reconstructed boot ELF matches retail`; the acceptance gate is `./verify-baseline.sh` |
| Land | Delete the oracle block, `git mv src/assembly/<path>.c src/<path>.c`, change the owner in `config/us/rnc1.us.yaml`, then `make elf` |
| Progress | Nothing to commit: CI rebuilds the report. `make check` and `make progress` preview it locally |

### rac2: `games/rac2/ntsc`

| Step | Command |
|---|---|
| Set up | `python3 tools/levels.py dump` (disc in `baserom/SCUS_972.68.iso`), `tools/compiler/build.sh`, `python3 tools/compiler/sdk/build.py` |
| List | `grep -rn INCLUDE_ASM src/` (every function without C, in address order); the catalogues are `config/boot/functions.tsv` and `config/levels/functions.tsv` |
| Try | `python3 tools/check.py func_XXXXXXXX --diff` (one function at all its places) |
| In context | `python3 tools/build.py all` (or `boot`, `level N`): all 28 programs, every loaded byte compared |
| Land | Replace the function's `INCLUDE_ASM` line with its C, in place; then `python3 tools/progress.py` |
| Test | `python3 -m unittest discover -s tests`, `python3 tests/check_repository.py`, `python3 tools/progress.py --check` |
| Progress | `python3 tools/progress.py` (objdiff's report format) |

On hosts without the tools, prefix any step with `bash tools/docker/run.sh`.

### rac3: `games/rac3/ntsc`

| Step | Command |
|---|---|
| Set up | `python tools/setup_asm.py --target all`, then `& "C:\tools\eegcc_2.95.3_sn_v1.36\bin\make.exe"`; on Linux and macOS, `python3 tools/setup_asm.py --target all`, then `python3 tools/build.py --target all --toolchain <dir> --runner <wibo>` with `UYA_EE29` set |
| List | `python tools/triage.py --tsv remaining.tsv` (pick from `plain`, smallest first) |
| Try | `python tools/try_func.py scratch/func_0039BEC0.c` (`--all-modes` tries every address mode and assembler), or `python localdecomp/server.py --project . --no-git-sync` |
| In context | `python tools/try_in_context.py scratch/func_003AED08.c` |
| Land | Replace the `INCLUDE_ASM` line with the block inside `localdecomp:start`/`end` markers, then `python tools/split_text.py --refresh` |
| Prove | `python tools/pr_check.py --target all` (prints `OK`), then the full build (prints `MATCH` per target) |
| Progress | CI on the maintainer's runner after merge; `make.exe objdiff` or localdecomp's Full check reproduce its numbers |

## 4. Per game

### rac1/pal: Ratchet & Clank PAL (from rac1-decomp)

- **Setup** ([README.md](../../games/rac1/pal/README.md)). The SN toolchain
  comes from two community mirrors. The build runs in a linux/386 container
  with Wine, which mounts all of OpenRAC. Host scripts such as `wave.py` need
  Python 3.10 or newer ([QUEUE.md](../../games/rac1/pal/docs/QUEUE.md)).
- **Procedure.** [docs/WORKFLOW.md](../../games/rac1/pal/docs/WORKFLOW.md) is
  the manual loop. Its step 0 asks whether the function is library code with
  an open-source original. [LEVERS.md](../../games/rac1/pal/docs/LEVERS.md) is
  the one-page cheat sheet: try `tools/lombyte.py` first, list every global
  and how retail reaches it, and check the known walls.
- **Two kinds of function.** For an executable function, `try_func` masks
  relocations, so its `EXACT` is only a filter and the full build's audit
  decides. For a level function (`func_LNN_X`,
  [OVERLAYS.md](../../games/rac1/pal/docs/OVERLAYS.md)), `try_func` links the
  file so the function sits at its address in its level and compares every
  byte and relocation. After a landing, `overlay_file_check.py` catches
  neighbours that stopped matching.
- **Rules.** A size mismatch is always reverted. `integrate.py` refuses
  register pins, empty-asm barriers, inline asm, `while (0)`, expression
  aliases and a `#define` in a candidate. Write a level address as the symbol
  the assembly names, never as a number.
- **Build fidelity** ([BUILD_FIDELITY.md](../../games/rac1/pal/docs/BUILD_FIDELITY.md)). No build step may change what the compiler
  emitted, and options apply to whole files (`config/file_cflags.txt`); a
  function that needs an option its neighbours do not goes in a file of its
  own. A function that only matches through a new build step or a flag of
  its own is a near miss, not a match. `tools/check_build_fidelity.py` runs
  in `gen_progress_report.py --check`, so CI enforces it.
- **What counts as finished without C.** Handwritten functions and linker
  remnants are retail assembly, counted as finished
  ([ASM_CLASSIFICATION.md](../../games/rac1/pal/docs/ASM_CLASSIFICATION.md)); a
  run of remnants qualifies under one rule (`tools/overlay_remnants.py`). A
  level function the catalogue cut into pieces is written whole and listed in
  `config/overlays/joined.tsv`.
- **Near misses** ([NONMATCHING.md](../../games/rac1/pal/docs/NONMATCHING.md)).
  The best attempt at each level function is kept as
  `nonmatching/<dir>/<func>.c`, with its verdict and the last notes in a
  header. Nothing builds these files, and the report scores them as
  `fuzzy_match_percent`, never as matched. Start from the staged file:
  `python tools/try_func.py func_X nonmatching/<dir>/func_X.c --diff`.
  `python3 tools/wave.py stage` re-stages near misses after each batch.
- **Before committing.** Run the full build, stage files by name (never
  `git add -A`), and put code ported from another project in a commit of its
  own that credits it
  ([AGENT_WORKFLOW.md](../../games/rac1/pal/docs/AGENT_WORKFLOW.md)). There is
  no pull-request template. See also [NAMES.md](../../games/rac1/pal/docs/NAMES.md)
  and [CONTAINERS.md](../../games/rac1/pal/docs/CONTAINERS.md) (Ghidra over MCP).

### rac1/ntsc: Ratchet & Clank NTSC-U (from Lombyte)

- **Setup** ([docs/building.md](../../games/rac1/ntsc/docs/building.md)).
  `./setup.sh` downloads and hash-checks the public toolchains, builds the
  game compiler from source, and runs the first `make elf`. Units that need
  the optional [patched EE-GCC profile](../../games/rac1/ntsc/docs/patched-toolchain.md)
  build from the retail oracle without it.
- **Oracles** ([CONTRIBUTING.md](../../games/rac1/ntsc/CONTRIBUTING.md)). A
  pending unit keeps the retail assembly under `#ifndef NON_MATCHING` and the
  C body you edit under `#else`. Do not remove the oracle while you work: only
  `check-unit` compiles the body. Units classified as intentional assembly
  (SIMD, VU0/MMI and COP2 helpers) are outside the C goal and have no body.
- **Proof.** objdiff ignores relocations, so 100 % is not exact
  ([progress-metrics.md](../../games/rac1/ntsc/docs/progress-metrics.md)):
  `make elf` and `./verify-baseline.sh` are the gate.
  [decompilation-tips.md](../../games/rac1/ntsc/docs/decompilation-tips.md)
  holds the acceptance discipline and the recovery ladder.
- **Level code.** `make overlays` compiles `src/overlays/` without linking.
  The overlay byte proof (`scripts/decomp try`) and `promote-unit.py` live in
  the maintainers' private tooling repository, not in this tree.
- **Not exact yet?** Leave the improved body under `#else`, keep `make elf`
  passing, and open a work-in-progress pull request. To claim a unit, open an
  issue or comment on one.
- **PR checklist** ([template](../../games/rac1/ntsc/.github/pull_request_template.md)):
  the `make elf` result with its `PASS` line, the `check-unit` score, and for a
  promotion the symbol, bytes, measures and compiler flags. Run
  `python3 scripts/test_public_tools.py` before changing `scripts/`. Names
  follow [recovered-names.md](../../games/rac1/ntsc/docs/recovered-names.md).

### rac2: Going Commando NTSC-U v1.01 (from rac2-decomp)

Since 2026-10-11 the project follows rac1/pal's structure ([AGENTS.md](../../games/rac2/ntsc/AGENTS.md),
[docs/BUILDING.md](../../games/rac2/ntsc/docs/BUILDING.md), [docs/LEVELS.md](../../games/rac2/ntsc/docs/LEVELS.md)).

- **Setup.** The disc, SN ProDG 2.0's assembler and linker you supply, and the two compilers
  `tools/compiler/` rebuilds from public source; or the project's Docker image.
- **What a match is.** Only what `tools/build.py` links into a program that equals retail byte
  for byte. A level function is defined once and linked at every place the catalogue gives it;
  the symbol values each program needs are read from its retail bytes.
- **Where C goes.** In place of the function's `INCLUDE_ASM` line; never a copy per level, and
  never a level address written as a number.
- **Sending work.** A branch and a pull request with the build, progress and test output; CI
  checks the catalogues, the layout and that the report is current.

### rac3: Up Your Arsenal NTSC-U (from rac3-uya-decomp)

- **Setup** ([docs/wiki/Setup.md](../../games/rac3/ntsc/docs/wiki/Setup.md),
  [targets.md](../../games/rac3/ntsc/docs/targets.md)). It builds three
  programs from your own disc: `frontbin.elf` (extracted with Wrench),
  `boot_elf.elf` (the main executable) and `i5bootn.elf` (the launcher). It
  needs exactly SN ee-gcc 2.95.3 v1.36, and for the library ranges marked
  `@ee29` also Sony's 2.9-ee-991111 (`UYA_EE29`, the SDK 2.4 mirror's
  `ee/gcc` folder). Tools take `--target frontbin|boot_elf|i5bootn|all`.
- **Loop** ([Workflow.md](../../games/rac3/ntsc/docs/wiki/Workflow.md),
  [Tools.md](../../games/rac3/ntsc/docs/wiki/Tools.md)). A score of 0 or a
  `MATCH` from `try_func.py` is necessary, not sufficient. localdecomp's Save
  writes into the source file, so save only at score 0.
- **Flags.** [tools/text_parts.txt](../../games/rac3/ntsc/tools/text_parts.txt)
  sets flags per address range. A function that needs other flags gets a
  two-line single-function override, a whole source file can take a flag
  (`3958F0.c` takes `-fno-force-mem`), `@ps2as` selects SN's assembler and
  `@ee29` Sony's library compiler
  ([Toolchain-and-Build.md](../../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md),
  [ee29.py](../../games/rac3/ntsc/tools/ee29.py)).
- **Not C.** `ASM_FUNC(...)` (handwritten) and `LINKER_REMNANT(...)` entries
  count as finished; report `odd` entries instead of writing C for them.
  Trailing padding is part of the layout
  ([Matching-Patterns.md](../../games/rac3/ntsc/docs/wiki/Matching-Patterns.md#not-everything-is-c)).
- **PR rules** ([Pull-Requests.md](../../games/rac3/ntsc/docs/wiki/Pull-Requests.md),
  [template](../../games/rac3/ntsc/.github/pull_request_template.md)). Every
  global stays `extern`, new aliases go in `symbol_addrs_resolved.txt`, inline
  asm and `$gp` hacks are called out, and a new pattern goes into
  Matching-Patterns.md in the same pull request. CI does not build pull
  requests, so reviewers build them locally.
- **Level code** starts in `src/levels/common/`, behind the opt-in
  `tools/build_common_c.py` and its own strict gates
  ([common_level_c.md](../../games/rac3/ntsc/docs/common_level_c.md)).

### rac4: Deadlocked NTSC-U (from rac-deadlocked-decomp)

Run from `games/rac4/ntsc`. Begun 2026-10-04 by rac1/pal's author, with a
smaller version of the same tooling ([CONTRIBUTING.md](../../games/rac4/ntsc/CONTRIBUTING.md)).

| Step | Command |
|---|---|
| Set up | `python3 tools/openrac.py setup rac4/ntsc` (OpenRAC root), then `bash tools/setup_asm.sh` |
| Compile everything | `bash tools/docker/run.sh bash tools/build.sh` |
| Compile one file | `bash tools/docker/run.sh bash tools/cc.sh SRC.c OUT.o` |
| Compare with retail | `venv/bin/python tools/audit_matches.py`; `venv/bin/python tools/diff_func.py func_00473A88` shows one function word by word |
| Try several forms of a function | `venv/bin/python tools/try_variants.py FILE.c name=func_00497438 ...` |
| Regenerate and check progress | `python3 tools/gen_progress_report.py`, then `--check` |

- **Proof.** Per function, with relocatable fields masked
  ([retail.py](../../games/rac4/ntsc/tools/retail.py)); nothing is linked. The mask also hides
  struct offsets and small constants, so this is looser than the other games'
  proofs ([games/rac4/README.md](../../games/rac4/README.md#what-matched-means-here)).
- **Source.** One file per function, `src/<core|net|game>/<ADDR>.c` and
  `src/overlays/L<nn>/`, with per-file flags in a `/* cflags: ... */` comment.
  `tools/auto_structs.py` drafts small functions from m2c output and keeps the
  ones that match.
- **Names.** `func_<address>`, `D_<address>`, generated `TypeN` and `fNN`
  until the code is understood; nothing copied from another source
  ([LEGAL.md](../../games/rac4/ntsc/LEGAL.md)).
- **Near misses.** `nonmatching/` holds drafts that are not built or scored.
- **Report.** `progress/report.json` is committed with the source; OpenRAC's
  CI runs its `--check`.

## 5. Working with AI agents

**rac1/pal's agent waves** ([AGENT_WORKFLOW.md](../../games/rac1/pal/docs/AGENT_WORKFLOW.md)).
A lead model plans waves, launches workers, reviews their matches and lands
them; it matches nothing itself. Work flows up the tiers measured on
2026-10-07 ([AGENT_WORKFLOW.md](../../games/rac1/pal/docs/AGENT_WORKFLOW.md), "Model tiers"): tools that need no model first
(`tools/port.py`, `overlay_variants.py`), then a swarm of small, cheap workers
(Haiku) on level functions of about 400 bytes and under, then Opus on their
near misses and on functions over 1 KB
([LONG_FUNCTIONS.md](../../games/rac1/pal/docs/LONG_FUNCTIONS.md)), each tier
starting from the notes and best candidate of the one below. Sonnet is used
only for library code with an answer key. Queue workers follow
[QUEUE.md](../../games/rac1/pal/docs/QUEUE.md); the subagent type is
[.claude/agents/match-worker.md](../../games/rac1/pal/.claude/agents/match-worker.md).

```
python3 tools/wave.py plan q6 --queue --overlay --family --count 64 --min-size 32 --max-size 600 --budget 6
python3 tools/wave.py status NAME            # verdicts from the run logs
python3 tools/wave.py land q6 --batch [--reject func_X ...]
bash tools/docker/run.sh python tools/overlay_variants.py clone   # variants, no model
```

- Workers write only in `build-sn/try/<func>/` and compile only through
  `try_func`, which enforces a run budget and logs every run. They never
  delete anything or commit, and use no subagents and no web. The lead trusts
  `runs.log`, not a worker's final message.
- At review, the lead rejects reads of unassigned locals, register pins,
  inline asm, barriers, `volatile` added to force an order, and anything that
  looks taken from Sony SDK source, and any new build step or per-function
  flag (the build-fidelity rules above). The measured cost per match, by
  model and pool, is what sets the tiers.

**Several agents in one checkout.** rac1/pal's `tools/claims.py` lets agents
and people share a working tree
([Several agents at once](../../games/rac1/pal/docs/AGENT_WORKFLOW.md#several-agents-at-once)):

```
python3 tools/claims.py claim gpt func_L05_002559DC     # "claimed", or "taken by <owner>" (exit 1)
python3 tools/claims.py release gpt func_L05_002559DC   # giving up on it
python3 tools/claims.py lock gpt      # before writing any file in src/
python3 tools/claims.py unlock gpt    # right after
```

Claims are files under `build-sn/claims/`, so they cover only one checkout of
rac1/pal. The other projects coordinate in public: rac2 and rac3 ask for an
early draft pull request, and Lombyte for an issue or comment.

**The other projects.** Lombyte's README calls it AI-driven, with every pull
request reviewed by hand. rac2's README says "an AI-generated answer alone is
not evidence of correctness". rac3's roadmap plans agent batches, but its docs
set no rules for agents beyond the pull-request rules.

**AI attribution trailers: an open question.** The projects disagree
([open question 2](../policy/OPEN_QUESTIONS.md#2-credit-lines-for-ai-assistants-in-commits)):

| Project | Rule in its own docs |
|---|---|
| rac1/pal | Commit messages end with one `Co-Authored-By` trailer naming the model ([WORKFLOW.md](../../games/rac1/pal/docs/WORKFLOW.md), "Publish") |
| rac1/ntsc | No agent trailers or session links. Its optional hook strips them and rejects AI identities as author or committer ([commit-messages.md](../../games/rac1/ntsc/docs/commit-messages.md)) |
| rac2 | Not addressed |
| rac3 | Contributors remove AI `Co-authored-by:` lines before pushing, and maintainers delete any left when squash-merging ([Pull-Requests.md](../../games/rac3/ntsc/docs/wiki/Pull-Requests.md)) |

For now, OpenRAC's [CONTRIBUTING.md](../../CONTRIBUTING.md#commits) asks for a
`Co-Authored-By:` trailer naming the assistant, as OpenRAC's commits have
carried so far, and leaves the rule open for the group to decide.

**rac2's tiers** ([AGENT_WORKFLOW.md](../../games/rac2/ntsc/docs/AGENT_WORKFLOW.md)).
The same idea sorted by size with `tools/todo.py`: Haiku up to 400 bytes, Sonnet
from 401 to 1,000, Opus only above that and on near misses. Each worker has its
own worktree and branch; there is no queue or claim file.

## 6. Porting between versions and games

### US and PAL (rac1/ntsc and rac1/pal)

Both projects decompile the same game, so a function matched in one is the
best starting point for the other.

- **The map.** `python3 tools/overlays.py us-map` (rac1/pal) pairs every US
  function with its PAL counterpart from the code alone, using ReRAC's
  extraction of the US disc. It writes
  [config/overlays/us_map.tsv](../../games/rac1/pal/config/overlays/us_map.tsv)
  ([OVERLAYS.md, "US map"](../../games/rac1/pal/docs/OVERLAYS.md#us-map)).
- **Pairing.** `python3 tools/lombyte.py func_X` gives the counterpart, its
  status and its C file, and `todo` lists what Lombyte matched and PAL has not
  ([SIBLING_DECOMPS.md](../../games/rac1/pal/docs/SIBLING_DECOMPS.md)). It
  looks in `$LOMBYTE`, else `games/rac1/ntsc` inside OpenRAC (else
  `~/Projects/Lombyte`), and reads Lombyte's report from its tree
  (`build/progress/report.json`, which `make check` writes) or from OpenRAC's
  `progress/sources/rac1-ntsc.json` (`openrac.py progress --fetch`).
- **Porting a function** ([SIBLING_DECOMPS.md](../../games/rac1/pal/docs/SIBLING_DECOMPS.md#porting-a-function),
  [QUEUE.md](../../games/rac1/pal/docs/QUEUE.md#lombyte-ports)). Keep the
  control flow, statement order and types, and rename functions through
  `lombyte.py`. Map globals by position: the n-th `%hi`/`%lo` or `$gp` access
  in one build is the n-th in the other. The level boundary and `$gp` differ
  by 0x100: they are 0x15EF00 and 0x166C00 in the US build, and 0x15F000 and
  0x166D00 in PAL ([rac1/ntsc overlays.md](../../games/rac1/ntsc/docs/overlays.md)).
  `python3 tools/wave.py salvage --ports` lands logged port attempts apart
  from other matches, so they go in a commit of their own.
- **The compilers differ.** PAL matches game code with SN GCC 2.95.3, and
  Lombyte uses a patched EE-GCC 2.9, so a function that needed the patched
  profile may not match on PAL. rac1/pal measured the NTSC decomp's per-file
  flags and found that they do not carry over: treat them as hints to test on
  one function at a time.
- **Provenance.** On PAL, the comment above a port ends with
  `Adapted from Lombyte (MIT) for PAL: <its file under src/>, <its name>.`,
  and [THIRD_PARTY_NOTICES.md](../../games/rac1/pal/THIRD_PARTY_NOTICES.md)
  lists each port. On Lombyte, a ported file's first line reads
  `Ported from rac1-decomp, the PAL decompilation (<file>, <function>)`
  ([THIRD_PARTY_NOTICES.md](../../games/rac1/ntsc/THIRD_PARTY_NOTICES.md));
  its public scripts include no porting tool.

### RAC1 bodies in RAC2

rac2 reuses a RAC1 body only when its bytes are identical in RAC2. It locates
the body by exact byte search and re-verifies it with RAC2's own tools; no
RAC1 name, address or compiler profile is carried over by analogy
([SECOND-C-LOT.md](../../games/rac2/ntsc/docs/SECOND-C-LOT.md)). Each lot
gives every function's RAC1 origin, symbol and file, in a table
([THIRD-C-LOT.md](../../games/rac2/ntsc/docs/THIRD-C-LOT.md)). Material that
is not a byte proof goes in
[COMMUNITY-ENGINE-REFERENCE.md](../../games/rac2/ntsc/docs/COMMUNITY-ENGINE-REFERENCE.md),
marked as reference, never evidence.

### RAC3 cross-references

[Cross-Repository-Resources.md](../../games/rac3/ntsc/docs/wiki/Cross-Repository-Resources.md)
pins each sister project at an exact commit and keeps three things apart:
algorithmic similarity, C that already exists, and verified matches. It also
sets out a relocation workflow (pin identities, select candidates by size and
hash, resolve every relocation, compare linked bytes) and notes each
project's license; rac3 itself has none. Within rac3, level code is adapted
from frontbin "donor" functions with the donor's flags
([common_level_c.md](../../games/rac3/ntsc/docs/common_level_c.md)).

### Where provenance is recorded

| Project | At the code | In a list |
|---|---|---|
| rac1/pal | Comment above the function | `THIRD_PARTY_NOTICES.md`; ports committed separately, with credit |
| rac1/ntsc | First line of the unit file | `THIRD_PARTY_NOTICES.md` |
| rac2 | Not specified | Origin tables in `docs/*-C-LOT.md`; the pull request names the project, its author and the agreement |
| rac3 | Not specified | Cross-Repository-Resources.md, with pinned commits |

The OpenRAC rule is in [SOURCING.md, "Credit"](../policy/SOURCING.md#credit).
Where each game's code was imported from, and how later work is brought in, is
in [docs/SOURCES.md](../SOURCES.md#bringing-in-later-work).

## 7. Practices worth sharing

These are suggestions for the group, each based on something one project
already does.

- **Keep near misses in the repository.** rac1/pal's `nonmatching/` and
  Lombyte's `#else` bodies make unfinished work visible and earn partial
  credit as a fuzzy score. rac3's drafts stay in localdecomp's local state,
  and rac2's trial evidence stays on its author's disk. A staged-attempts
  directory with a generated index
  ([NONMATCHING.md](../../games/rac1/pal/docs/NONMATCHING.md)) would work for
  both.
- **Make the inner loop relocation-aware.** rac3's `try_func.py` resolves
  relocations, and rac1/pal's level check links for real. rac1/pal's
  executable path and Lombyte's `check-unit` still let swapped stores to
  different globals pass until the full build. rac1/pal's WORKFLOW.md already
  lists "Resolve relocations in `try_func`" as open work.
- **Re-check the whole file after landing.** A function can stop matching when
  a neighbour lands. rac1/pal's `overlay_file_check.py` and rac3's
  `try_in_context.py` catch this before the full build.
- **Run a pre-PR checker.** rac3's `pr_check.py` names the line that will
  break the build, and rac2's `doctor.py` tells a newcomer what their machine
  can do. rac1/pal's `gen_progress_report.py --check` and Lombyte's
  `make check` need no game data.
- **Share permuter setups.** rac1/pal ([PERMUTER.md](../../games/rac1/pal/docs/PERMUTER.md),
  plus `tools/permute.py` for exhaustive reordering) and rac3
  ([permuter.md](../../games/rac3/ntsc/docs/permuter.md), plus `regalloc.py`
  for allocation ties) both wire decomp-permuter to their real compile
  pipelines. Lombyte lists "guided C mutation search" in its recovery ladder
  but has no public tool for it.
- **Make flags and padding explicit.** rac3's `text_parts.txt` keeps every
  flag override single-function, Lombyte's `ROUTE_EXCEPTIONS` may only shrink,
  and rac1/pal has not mapped its per-file flags yet. rac3 also adds retail's
  trailing padding with `TEXT_PADDING(N)` before anyone converts a function
  ([trailing_padding.md](../../games/rac3/ntsc/docs/trailing_padding.md)),
  which rac1/pal's SIBLING_DECOMPS.md calls worth copying.
- **Compare percentages with care.** Handwritten code counts as finished in
  rac3 and rac1/pal but is left out of Lombyte's denominator.
- **Reuse before writing.** rac1/pal's `overlay_variants.py clone` matches
  variants with no model, and family order paid best in its waves. rac3's
  Matching-Patterns lists
  [families](../../games/rac3/ntsc/docs/wiki/Matching-Patterns.md#families),
  and rac2 reuses byte-identical RAC1 bodies.
- **Write down every trial and every pattern.** rac2's experiment register and
  rac1/pal's `runs.log` and `NOTES.md` stop people from repeating failed
  attempts, and Lombyte asks for the same evidence discipline. rac3 adds a new
  pattern to Matching-Patterns in the pull request that found it, and rac1/pal's
  QUEUE.md takes an idiom only with a landed function that shows it.
