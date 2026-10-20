# To do

Larger pieces of work that are scoped and decided but not started. Each
section gives the goal, what is already known, how to do it, how to check it,
and where to begin. If you take one on, say so in its section (name and date),
and remove the section when it lands.

Known bugs that are guarded but not properly fixed are in
[port/KNOWN_ISSUES.md](port/KNOWN_ISSUES.md).

## 1. A bigger moby pool for Ratchet & Clank (port)

**Goal.** No level runs out of room for dynamic mobys (objects).

**Why.** Every level gets 255 usable spawn slots. Busy effects can fill them,
e.g. Gaspar's lava sparks: fcheck shows the original game reaching 254 of 255
there. CreateMoby then returns 0, and any caller that doesn't check reads
through a null moby and crashes the port. Gaspar's Pilot's Helmet cutscene was
one such caller. 0.8.4 guards that one call site
(`port/game/rac1/host/scene_actors.c`), but any other site can crash the same
way. The details and evidence are in
[port/KNOWN_ISSUES.md](port/KNOWN_ISSUES.md).

**What is known** (from reading the code, 2026-10-20):

- The pool's size is data. Each level's gameplay file gives a spare-slot
  count at the moby section: header +0x44, second word (`n2`).
- The level loader, `LoadLevelGameplay` (PAL `func_L00_002422D8` in
  `games/rac1/src/overlays/shared/loaders_00240398.c`; US
  `games/rac1/ntsc/src/overlays/shared/world/loaders/load_level_gameplay.c`),
  sizes everything from it:
  - the array, `(placed + n2) << 8` bytes;
  - its pvars, 0x80 bytes per spare slot;
  - the pool end, `base + (placed + n2 - 1) * 0x100`. The last slot holds the
    free mark.
- Everything after the array is allocated from where the array ends, so it
  moves with `n2`.
- Every level ships 256 spare slots except level 17, which ships 400. So a
  larger count already works in retail data.

**How.**

1. Wrap the load step with the port's wrap mechanism (`libraries.tsv`
   `wrap`, with the decomp's own function kept as `__game`, as
   `port/game/rac1/host/wide_view.c` and `crash_state.c` do).
2. After the header is in memory and before the loader reads it, raise `n2`
   to `max(n2, N)`, with N around 512 or 768. Do this on both discs (PAL and
   US).
3. Make it a setting if needed, but the default should be the larger pool.
4. Add an entry to `port/game/rac1/differences.json`: a `port` difference
   ("more room for effects than on a PS2").

**Check before choosing N:**

- the level heap's headroom on the levels with the most placed mobys: 16
  (1,569), 5 (1,431) and 3 (1,029);
- the moby grid's 1,536 shared blocks of 16 ids
  (`port/game/rac1-pal/hand/game_moby_grid.c`; it traps when they run out);
- the 512-entry target list in the scratchpad (0x70000800–0x70000C00);
- any fixed cap in the renderer or the VU packet buffers on how many mobys
  are drawn (not checked yet).

**Rejected approaches:** moving the array after load, because placed mobys,
groups and the grid hold its addresses and indices; and a second overflow
pool, because `(m - base) >> 8` indices break outside the array.

**Verify.** Build both discs, run ctest, then a hidden Gaspar run with the
moby census:
- a temporary log of the live slots between the pool's two bounds, by class;
- the pool should never reach its new end;
- the guard's "scene actor skipped" line should not appear.

**Effort:** about half a day to a day. Keep the 0.8.4 guard as the stopgap.

## 2. Rearrange rac3-uya-decomp like rac1-decomp and rac2-decomp

**Goal.** The Ratchet & Clank 3 decompilation
([OpenRAC/rac3-uya-decomp](https://github.com/OpenRAC/rac3-uya-decomp),
synced into `games/rac3/ntsc`) uses the same layout, entry points and
matching rules as the other two. That way contributors and tools move between
them freely.

**Decided** (by the project owner, 2026-10-20):

- **Upstream, in steps.** A series of small PRs to rac3-uya-decomp, each
  keeping all three programs byte-identical:
  - frontbin SHA-1 `3bc94ee8…`;
  - boot_elf `48797530…`;
  - i5bootn `71f3ecfc…`.
  Use `git mv`, with no history rewrite. Its main contributor (vetusmagnus)
  commits straight to `main`, so tell them before the first move lands.
- **rac2-style entry points; the old names keep working.**
  - Add `tools/check.py`, `tools/progress.py`, `tools/todo.py` and
    `tests/check_repository.py` as the main entry points.
  - Keep `pr_check.py`, `try_func.py`, `triage.py` and the Windows `Makefile`
    working as thin aliases.
- **Honest per-file compiler flags.** rac1 and rac2 allow one flag set per
  real source file. A function that matches only with flags of its own is not
  matched (rac1-decomp's `docs/BUILD_FIDELITY.md`).

**What is known.**

- rac3 sets flags by address range (`tools/text_parts.txt` and
  `targets/<program>/text_parts.txt`). It compiles each run of equal flags as
  its own slice. The flag lines and single-function overrides per program:
  - frontbin: 748 flag lines, 451 overrides;
  - boot_elf: 978 flag lines, 441 overrides, plus 166 Sony 2.9 (`@ee29`)
    ranges;
  - i5bootn: 16 flag lines, 1 override, plus 9 `@ee29` ranges.
- About 500 lines also pick a different assembler for one function
  (`@ps2as`).

**How**, in order:

1. **Docs** (mechanical): add `AGENTS.md`, a `CLAUDE.md` pointer, `LEGAL.md`,
   `THIRD_PARTY_NOTICES.md` and `nonmatching/README.md`.
2. **Tests** (mechanical): add `tests/check_repository.py` and
   `tests/test_tools.py`, wrapping `pr_check.py` and checking that no game data
   is committed. Add a checks workflow.
3. **`config/`** (careful): move each program's data files (symbol_addrs, the
   flag tables, src_files, divs_nops, sq_ra_funcs) to `config/<program>/` and
   the splat yamls to `config/splat/`. Change `tools/targets.py` first, since
   every tool reads its paths there. Then update the Makefile, `build.py`,
   `setup_asm.py` and the yamls.
4. **Move:** the three ELFs to `baserom/`, `localdecomp/` to
   `tools/localdecomp/`, and the objdiff generators under `tools/`.
5. **Progress and links:** add the entry-point aliases, move
   `progress_report.json` to `progress/report.json` (its format already
   matches), and update the CI workflow, README and wiki links.
6. **Flags:**
   - Library code (`@ee29`, about 175 ranges) becomes its own files, since
     Sony compiled it separately.
   - For every other override, try to make the function match under its
     file's own flags.
   - Functions that can't go back to `INCLUDE_ASM`, with their drafts kept in
     `nonmatching/`.
   - This is about 900 functions of bounded work each, which suits a swarm of
     small workers, one function per job, with a check budget.
   - Expect rac3's count to drop at first and then recover.

**Verify:** after every step, `python3 tools/build.py --target all` prints
three `MATCH` lines with the SHA-1s above.

**Setup notes:**
- It builds on Linux and macOS through `tools/build.py` with
  [wibo](https://github.com/decompals/wibo) 1.2.0 and SN ee-gcc 2.95.3 v1.36.
- The `@ee29` ranges need Sony's 2.9-ee-991111 in its Windows layout
  (`lib/gcc-lib/ee/2.9-ee-991111/`). The `sn-prodg-24` tree's
  `local/sce/ee/gcc` has it; see [toolchains/README.md](../toolchains/README.md).

## 3. A refactor pilot for rac1-decomp

**Goal.** Readable C with real names and types, while every build stays
byte-identical. Start with a pilot on one subsystem (the camera is a good
first one), to measure how well the method works before scaling it up.

**The method:** design centrally, edit in parallel.

1. **A designer.** One strong model or person writes the subsystem's shared
   types (structs with field names) and a name registry for its functions,
   globals and fields. The names come from evidence: call sites, strings, and
   what the rac2 and rac3 decompilations call the same things.
2. **Workers on bounded jobs.** Each takes one file and applies the registry:
   use the structs, the registry's names and descriptive locals. Workers never
   invent shared names; a name that is missing goes back to the designer.
3. **Automatic gates.** Each file builds byte-identical (rac1-decomp's
   `tools/overlay_file_check.py` and `tools/build_sn.sh`). Nothing new appears
   outside the registry. Each batch gets a full build.

**Why this shape.** A byte-identical build can't tell a wrong name from a
right one, so names must be reviewed, and wrong ones are worse than
`field_1F8`. Shared types touched by many workers at once also cause merge
conflicts and duplicate declarations.

**Verify and review:**
- the gates above;
- a human review of the registry before the workers start, and a sample of
  their files afterwards;
- measure the hit rate (files that pass on the first try) and the
  review-correction rate, to decide whether to scale up.

**Where:** changes go to rac1-decomp through its usual PRs, then sync into
`games/rac1`.
