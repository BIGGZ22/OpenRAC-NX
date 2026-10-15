# Workflow

How a function goes from `INCLUDE_ASM` to matched C, in order. Every step
reflects a real incident. `docs/DECOMP_PROGRESS.md` covers the levers and
the dead ends; this file is the procedure.

Revised 2026-09-16 after checking our process against
[decomp.wiki](https://decomp.wiki). The "Checked against decomp.wiki"
section at the end records what was adopted and what was ruled out.

## Setup (once, and after changing config/splat.yaml)

`asm/` is not in git. It is the disassembly of the retail executable.
Generate it from your own baserom:

```
pip install -r requirements.txt
bash tools/setup_asm.sh
```

The script checks the baserom sha1 and the pinned splat/spimdisasm
versions, then applies the same post-processing the build expects
(`fix_vu0_macro.py`, `sn_regnames.py`, `fix_denormal_floats.py`). It leaves `src/` and `include/`
alone. The output was verified byte-identical, so every contributor diffs
against the same thing.

## 0. Before decompiling anything: is it even game code?

**Check whether a real source exists first.** Libraries were not written
for the game, and compiling their real source is cheaper and more faithful
than decoding them. It can also reveal a different compiler.

- libgcc: `src/libgcc/` (fp-bit, built by gcc 2.9-ee) is the precedent.
  It settled three near-misses that had been written off as unsteerable.
- Signs that a function is library code:
  - it has no callers from game code but calls into a cluster;
  - it matches a textbook libgcc or libc shape;
  - a whole family shares the same "unexplained" residual;
  - `sd` spills where the rest of its segment uses `sq`.
- Try every EE `cc1` in the toolchain mirrors on the verbatim source. See
  `docs/DECOMP_PROGRESS.md`, "libgcc is 2.9-ee".

**A residual shared across a whole family is a signal, not a verdict.**
Before recording "treat the rest of this family as the same known
residual", compile one member with the other compilers.

## 1. Get a sketch

```
sh tools/gen_ctx.sh                  # ctx.c from include/ (once per header change)
python tools/m2c.py func_XXXXXXXX    # m2c sketch, with context
```

m2c output is a reference. It is never matching as emitted, and it
mis-decodes branch-likely (`bnel`) conditions. Ghidra output (from the
MCP container, `docs/CONTAINERS.md`) is reference only too, and is never
pasted into `src/`.

Look for **family siblings** before writing anything: grep
`asm/nonmatchings/` for the distinguishing call or constant. The family
method has paid in every round since it was introduced.

## 2. Iterate with the differ

For quick tries, `tools/try_func.py` compiles one candidate function in a
scratch copy of its source file (the same per-segment pipeline) and
compares it with retail in seconds, without touching `src/` or linking:

```
python tools/try_func.py func_XXXXXXXX candidate.c --diff
python tools/try_func.py func_XXXXXXXX c1.c c2.c c3.c      # one verdict each
```

It also takes a function that is already C: the candidate then replaces
its definition, which is how near-misses get refined. It masks relocated
fields, so a pass there is a filter, not a match: the function still has
to pass the full build (step 3). On macOS/Linux run it through
`bash tools/docker/run.sh python tools/try_func.py ...`. For a whole-image view
and asm-differ's side-by-side, use:

```
sh tools/diff.sh func_XXXXXXXX
```

It builds, links, regenerates the images and runs asm-differ. It
**refuses to diff after a failed make**: a failed compile leaves the
previous `.o`, whose `INCLUDE_ASM` stubs still hold retail's bytes. That
would show a fictional match, and it has happened 15+ times.

A batch of candidates (one `func_X path/to/candidate.c` line each in a
manifest) goes in with `python tools/integrate.py MANIFEST --apply`: it
re-checks each one with `try_func.py` and applies the exact ones. Build
and audit afterwards as always.

In `core_text`, first ask which compiler built the object: Sony SDK code
(the C library, the memory card library, libmpeg, ...) was built with
the SDK's 2.9-ee, the rest with 2.95.3. `tools/compiler_sweep.py
src/core/X.c` compiles a file whole under both and lists each function's
verdict. A near-miss that only one compiler reaches says which one it is;
the objects marked `ee29` in `config/core_text.objects` build with 2.9-ee.

Work through the levers in `docs/DECOMP_PROGRESS.md` in rough order of
cost:

1. Types and struct shape: `long` is 64-bit, `long long` is 128-bit;
   `char` vs `unsigned char`; type the base as a struct.
2. Control-flow spelling: invert the test (exit cross-jumping, arm
   order, branch-likely); use one definition per arm at a join.
3. Addressing form: base pointers, and two C names on one symbol.
4. **Branch-invariant duplication** (decomp.wiki, GCC 2.9 991111 on PS2).
   If an allocator tie survives, try duplicating the code shared after an
   if/else into both arms. GCC hoists it back out, but the register
   choice follows the original shape. This is untested here, and it is
   the first thing to try on allocator-tie near-misses, which we had
   recorded as not source-steerable.
5. **Irregular switches** (decomp.wiki). An if/else chain from m2c, or a
   compare whose result nothing uses, can be a small `switch`. That
   includes an explicit case that duplicates `default`.

A size mismatch is always reverted or stubbed, because one short function
shifts everything after it. A same-size near-miss may be kept, with the
recovered source and every spelling tried (with byte counts) in a comment.

## 3. Verify, from scratch

```
bash tools/build_sn.sh
```

The script deletes the objects first, stops on make's own exit status, then
links, and runs `sweep_matches.py`, `check_layout.py` and `check_image.py`. If you run the
steps by hand, check make's exit status yourself. `$?` after a pipe is the
status of the last command in the pipe, not make's. When a tool itself changes (the sweep, a rewriter,
the report generator), also cross-check with an independent whole-image
byte comparison. A tool that was just modified is not evidence for its
own correctness.

## 4. Publish

```
python tools/gen_progress_report.py      # does its own from-scratch build
python tools/gen_progress_report.py --check
```

Commit the regenerated `progress/report.json` **together with** the source
or original-assembly classification change. CI (`Progress report`) fails a
push whose report is out of date with `src/` or the tracked classification
lists, and decomp.dev publishes whatever the report says. The build audit
counts exact C separately from the report's finished original assembly.

- Stage files by name, never with `git add -A`. Stray tool output has
  reached the public repo that way before.
- Commit messages end with a single `Co-Authored-By` trailer naming the
  model that did the work, for example
  `Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>`.
  They carry no session link.
- Never commit `baserom/`, `toolchain/`, `build-sn/`, `asm/`, `tools/ext/`,
  extracted assets, or retail bytes of any kind (that includes "target"
  objects).

If a new library module is added, update `tools/libgcc_units.py`,
`Makefile.sn` and the aliases in `rac1.ld.sh` together.

## Checked against decomp.wiki (2026-09-16)

| wiki item | status here |
|---|---|
| Real compiler per component, e.g. libraries built separately | **adopted**: libgcc is 2.9-ee (+9 exact) |
| GCC 2.9 991111 used by PS2 games (Fatal Frame, PaRappa 2, TM:Black) | **partly true here**: libgcc yes. The game's `text` segment spills s-regs with `sq`, which 2.9-ee does not emit, so it is not 2.9-ee as-is |
| C++ codegen differs from C (e.g. `bool` changes load/store order) | **game is C++**: retail contains `hud.cpp`, `loaders.cpp`, `map.cpp` (and `snd.c`). **Measured:** compiling `text` as C++ (cc1plus 2.95.3, `extern "C"`) gives 239 exact vs 240 as C, and all 25 near-misses are unchanged. The language switch is not the lever. **Untested:** C++ *features* such as `bool` fields and member functions. Try `bool` on store-order near-misses before calling them unsteerable |
| NOPs from floating-point literals in `.lit4` | **ruled out**: retail loads no float literal via `$gp` from `.lit`. It has 63 gp-relative `lwc1`, all globals, and float constants are inline (858 `mtc1`) |
| Mixed `$gp` and `lui` access to one variable within a file suggests a TU boundary | **relevant**: our `NOT_SDA` / `extern short` workarounds may be compensating for compiling two giant files where retail had many TUs. Use it as a split hint when a variable is addressed both ways |
| objdiff units = real TUs | **not yet**: units are our files, not retail's TUs. Refine as splits become known (the `.cpp` names above are the first evidence) |
| The NTSC decomp's flags `-G8 -O2 -ffast-math -fno-exceptions` (GCC 2.95.2) | **`-ffast-math` measured and rejected**: a whole build with it gives 363 exact vs 364. It breaks `func_0022DB48` and improves nothing. `-G8` was already ruled out (float constants would pool into `.lit4`, which retail never does). `-fno-exceptions` only matters for C++. NTSC decomp decompiles almost nothing, so its flags were never verified against matches |
| decomp-permuter / decomp.me | **adopted, locally**: `tools/permuter_setup.py` runs the real decomp-permuter against our own compiler, inside the container (docs/PERMUTER.md). decomp.me itself still has no SN ProDG compiler preset for this game, so it cannot be used for collaboration on it as-is |

## Agent waves

When agents do the matching, one orchestrator plans waves of workers
and reviews what comes back. The workers follow [WORKER.md](WORKER.md).

```
python3 tools/lombyte.py todo                # matched in Lombyte, not here
python3 tools/triage.py                      # what is left, by route
python3 tools/wave.py plan w7 --budget 10 func_X ...   # or --near / --fresh / --overlay
python3 tools/wave.py status w7              # verdicts as workers finish
python3 tools/wave.py land w7                # one commit per EXACT, full build each
```

- `plan` writes each function's `CONTEXT.md` (`tools/dossier.py`): the
  declarations of everything it calls and uses, its callers' prototypes,
  similar matched functions and earlier attempts. Workers start from that
  instead of searching the tree.
- `plan` also writes a `BUDGET` of `try_func` runs, which `try_func`
  enforces.
- Roles:
  - `match`: the matching itself.
  - `compile`: turns the m2c sketch into a candidate that compiles, for a
    cheaper model to do before matching starts.

### Queue waves

The full description, with the lead's loop, the measurements and what
went wrong, is in [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md). In short:

Modelled on Thief3-Decomp's tiered workflow: the aim is the most matches
per token.

```
python3 tools/wave.py plan q1 --queue --overlay --count 40 --max-size 300 --budget 8
python3 tools/wave.py tokens q1      # tokens per worker and per match
```

- Each worker takes N functions from the wave, COUNT at a time
  (`wave.py claim`), so the startup cost (system prompt, protocol) is paid
  once per worker instead of once per function. Its whole prompt is one
  line: `Read docs/QUEUE.md and follow it exactly. WAVE=q1 ID=s01 N=6 COUNT=2`.
  IDs are never reused within a wave.
- [QUEUE.md](QUEUE.md) is the workers' whole instruction set (about 1.3K
  tokens), in place of WORKER.md and LEVERS.md. Add an idiom to it only
  when a landed function shows it.
- A claim prints the function's packet: dossier, assembly, and matched C
  to start from (the function it is a variant of, its nearest relative,
  short matched functions of its file).
- Verdicts come from `runs.log`, which try_func writes: workers write no
  RESULT.md, and their final message is one JSON line. Trust the log, not
  the message.
- `status` and `land` work as for any wave.
- Before a queue wave, run `tools/overlay_variants.py clone`
  ([OVERLAYS.md](OVERLAYS.md#variants)): it matches variants of matched
  functions with no model.

Trials of 2026-09-30 (input tokens include cache reads, as
`wave.py tokens` counts them):

| Wave | Model | Functions | Exact | Input tokens per match |
|---|---|---|---|---|
| q2: common code, 97-300 bytes | Sonnet | 12 | 3 (336 bytes) | 1.61M |
| q1: 8-92 bytes | Sonnet | 17 | 4 (124 bytes) | 214K |
| q1: 8-92 bytes | Haiku | 15 | 4 (100 bytes) | 1.02M |
| q3: `--family`, 64-500 bytes | Sonnet | 12 | 9 landed (1,236 bytes), 1 rejected | 157K |
| q4: `--family`, 32-600 bytes | Sonnet | 56 | 34 (11,128 bytes) | 1.15M |
| q5: `--family`, 32-600 bytes, after q4 | Sonnet | 64 | 44 (13,316 bytes) | 325K |

- **A queue worker is cheap per function.** The harness counted about 81K
  tokens for each q2 worker, six functions each, where a one-function
  worker of the earlier waves used about 112K for one.
- **No Haiku tier here.** On the same queue Haiku used almost five times
  Sonnet's tokens per match, more than its lower price makes up for.
  Sonnet recognised an unmatchable entry and stopped without a run;
  Haiku spent its runs on it.
- **Small catalogue entries are often fragments**, not functions: a piece
  the splitter cut at a call target or after a return, which branches out
  of itself or reads registers it never sets. About half of q1 was that.
  They need merging back into their function in the catalogue before
  they can match; until then keep `--min-size` at 32 or more and expect
  workers to stop on them.
- What is left of the common code under 300 bytes matched at 25%: the
  earlier waves and the variants took the easy part.
- **Family order pays best.** With `--family`, nine of q3's twelve
  functions came with a matched relative's C in their packet, and half of
  the matches took one run. One match was rejected at review (it read an
  unassigned local). Matching func_L01_00252E80 brought 17 variants with
  it through `overlay_variants.py clone`, with no model. Plan waves this
  way by default, and run `clone` after landing.
- Workers sometimes stop after one claim; the lead refills the queue with
  a new worker (a new ID) until `status` shows nothing pending.

### Picking a wave

Measured on 2026-09-26/27 (Sonnet workers, one function each):

| Pool | Workers | Exact | Code per 10 workers | Tokens per 0.1% of code |
|---|---|---|---|---|
| Near-misses, retries and some large new functions (waves 2-4) | 84 | 14 | +0.11% | 1.2M |
| Everything else up to 1000 bytes (wave 5) | 51 | 3 | +0.04% | 5.0M |
| Freshly unblocked functions (wave 6) | 25 | 7 | +0.13% | 1.0M |

"Code" there is the executable's (466 KB). Level code, measured
2026-09-28, is counted against all of the game's code (3.69 MB):

| Pool | Workers | Exact | Bytes matched | Bytes per 1M tokens |
|---|---|---|---|---|
| Common level code in all 19 levels, 156-440 bytes (ov1, ov2) | 24 | 11 | 2,284 | about 850 |
| For comparison: wave 6 above | 25 | 7 | about 2,300 | about 470 |

- **Level code matches well.** Many level functions are copies or
  variants of executable functions that already have C (the sound bank,
  interpolation and moby helpers), and workers found them in `src/game/`.
  The ones that stopped short stopped on the usual ties (register order,
  branch-likely, store order).

- **Ports first.** Functions Lombyte has matched (`tools/lombyte.py todo`,
  66 on 2026-09-27, 6.8% of code) should match in a run or two once
  renamed. See [SIBLING_DECOMPS.md](SIBLING_DECOMPS.md).
- **Fresh functions pay best.** A pool that a tool fix just unblocked
  (the `$fp` save rule, `qcopy()`) matched at 28%. Leftovers that earlier
  waves stopped on matched at 6%.
- **Retries with notes pay moderately**: a second round starting from
  good notes found most of waves 3-4's matches.
- **Small first, budget 10.** Every match under 600 bytes came within 9
  runs; misses spent the rest of the budget for nothing. Give 20 only
  above 600 bytes, where first attempts cost 200-330K tokens and rarely
  match exactly.
- At most 20 workers run at once: queue the rest.

### Landing

- `land` compiles each candidate again and runs the full build per
  function. `try_func` resolves relocations to retail's addresses but
  needs no link, so an `EXACT` can still fail there (an unpaired `%hi`,
  func_001E9808). When it does, change that function's
  `RESULT.md` from `EXACT`, or every later `land` rebuilds it again.
- The candidate's comments go into `src/` as they are. Rewrite worker
  notes into what the function does plus the one fact that is
  load-bearing, before starting `land`: it reads the candidates when it
  starts.
- A candidate defined under an alias because the file declares it with
  another type: fix the file's prototype in its own commit (full build),
  then land a plain definition.
- `tools/integrate.py` refuses pins, barriers and inline asm. A `while (0)`
  inside a macro taken from the original source (newlib's `MALLOC_ZERO`)
  is not a barrier: review it and land it by hand.

### Overlay pool

Overlay functions (`func_LNN_XXXXXXXX`, docs/OVERLAYS.md) are their own
pool: name them on the `plan` line, or let `--overlay` pick shared code in
all 19 levels first, smaller first, after the usual triage of
`asm/overlays/<name>.s`. A wave is all overlay or all executable. There is
no m2c sketch for them, so no `--role compile`.

`land` checks an overlay match by re-running try_func on the landed file
(the strict check, docs/OVERLAYS.md) instead of the full build, which
doesn't include `src/overlays/`. It regenerates the progress report when
`build-sn/rac1.elf` exists, and commits `feat(overlays): <name> exact
match`.

### Open work that would help the next waves

- **Map per-file flags.** Run every near-miss candidate with
  `TRY_CFLAGS=-mno-split-addresses` (and `-fopt-stack`), record which files
  match better, and give those files their flags in `Makefile.sn`, the way
  UYA's `tools/text_parts.txt` does.
- ~~Resolve relocations in `try_func`~~: done 2026-10-07, after
  rac3-uya-decomp's `try_func.py`. Every executable C function still
  comes out `EXACT` under it; 24 keep a few masked bytes (references into
  their own `.rodata`).
- **Stage near-misses in `src/`.** Candidates and notes live only in
  `build-sn/try/`, which is not tracked. Lombyte keeps unfinished C under
  `#else` of a `NON_MATCHING` guard, beside the retail assembly, so the
  work is shared and the build stays exact.
