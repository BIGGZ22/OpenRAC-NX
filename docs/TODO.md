# To do

Larger pieces of work that are scoped and decided but not started. Each
section gives the goal, what is already known, how to do it, how to check it,
and where to begin. If you take one on, say so in its section (name and date),
and remove the section when it lands.

Known bugs that are guarded but not properly fixed are in
[port/KNOWN_ISSUES.md](port/KNOWN_ISSUES.md).

## 1. The rest of the bigger moby pool for Ratchet & Clank (port)

**Landed** (2026-10-22): the port gives every level at least 768 spare moby
slots (767 usable, against the disc's 255) except Rilgar (5), Blarg Station
(6) and Quartu (15), and caps the tick's target list at 511
(`port/game/rac1/host/moby_pool.c`; the limits checked are in
[port/KNOWN_ISSUES.md](port/KNOWN_ISSUES.md)). Left:

- **Check it in game.** A hidden Gaspar run with a temporary census of the
  live slots between the pool's two bounds, by class. The pool should stay
  well short of its new end, and neither "scene actor skipped" nor "moby
  target list full" should appear. Then a level with many placed mobys
  (Kalebo III) with the difficulty mod's enemy count at 2 or more.
- **Room on Rilgar, Blarg Station and Quartu.** Their level heap ends within
  0 to 65 KiB of 0x1FF8000, so they keep 255. Room would have to come from
  shrinking the per-level VU chain area (f4 to fC of the load state, 0x160000
  to 0x1C0000 twice; the port's high-water mark there is not measured, and f4
  also holds the decompressed gameplay file) or from guest memory past
  32 MiB (several port paths mask addresses with `& 0x01FFFFFF`).
- **The moby grid's per-cell count.** Only mobys with collision enter the
  grid (lava sparks don't). A cell's count is a byte: the collision queries
  read it signed, so a cell with 128 or more colliding mobys drops out of
  collision, and at 256 it wraps and the next removal traps
  (`port/game/rac1-pal/hand/game_moby_grid.c`). Kalebo III's densest cell
  already holds about 117. Enemy copies piled into one 16-unit cell could
  reach it; reading the count unsigned and refusing an insert into a full
  cell would make it degrade instead.

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
