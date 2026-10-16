# decomp-permuter

[decomp-permuter](https://github.com/simonlindholm/decomp-permuter) randomly
rewrites one C function -- reorders statements, adds temporaries, retypes
things, swaps branches -- and keeps whatever compiles closer to retail. It
is the tool for a register-allocation or scheduling tie that no hand
rewrite moves: the "known walls" in `LEVERS.md`, where three different
wordings all land on the same wrong bytes.

`tools/permuter_setup.py` wires it up against our own compiler, inside the
container, so a score of 0 there means the same thing `tools/try_func.py`
calls `EXACT`. This is new (2026-09-28); `WORKFLOW.md`'s decomp.wiki table
used to say we only had `tools/permute.py`.

## vs. `tools/permute.py`

Both start from a `try_func.py`-style candidate. They solve different
problems:

| | `tools/permute.py` | `tools/permuter_setup.py` (decomp-permuter) |
|---|---|---|
| What moves | statements **you** mark with `/*P*/` | anything decomp-permuter's randomizer can touch: statement order, temporaries, types, branch shape |
| Coverage | every ordering of the marked lines, exhaustively | a random sample; no guarantee it ever finds the answer |
| Ceiling | ~8 marked lines (40320 permutations) | no hard ceiling, but the search gets less effective as the function grows |
| Good for | "these N stores are in some wrong order" -- func_0012C4C0-shaped near-misses | register-allocation ties, scheduling near-misses, anything permute.py's exhaustive search doesn't cover (retyping, branch rewrites, ...) |
| Confidence in a hit | proof (every ordering was tried) | a lead: still just one candidate that happened to compile closer |

Try `permute.py` first when the near-miss really is "these lines, some
order" -- it's exact and near-instant. Reach for the permuter when that
doesn't apply, or comes up empty.

## Setup (once)

```
bash tools/docker/run.sh bash tools/permuter_bootstrap.sh
```

Clones decomp-permuter and fetches the pieces our build container doesn't
already carry (`toml`, and an objdump that can decode plain MIPS) into
`tools/ext/`, which is gitignored but lives inside the repo -- so, unlike
anything installed straight into the container, it survives past that
container's `--rm` (see the script's own comment for why). Re-run it if
`tools/ext/` is ever deleted.

**Everything below runs inside the container**
(`bash tools/docker/run.sh ...`). The permuter's own C-file preprocessing
is plain macro expansion and works fine on the host, but `target.o` and
every candidate it compiles need Wine and the real SN compiler, which only
the container has -- there's no benefit to splitting the two steps.

## One function

```
bash tools/docker/run.sh python3 tools/permuter_setup.py func_L00_00235608 build-sn/try/func_L00_00235608/cand3.c
bash tools/docker/run.sh bash tools/permuter_run.sh build-sn/permuter/func_L00_00235608 -j$(nproc) --stop-on-zero
```

`func_X` is any function still `INCLUDE_ASM`'d in its real source file
(this needs the retail stub to build `target.o` from -- a near-miss
already written to C isn't a stub anymore, see "What it doesn't do"
below). The candidate `.c` is the same shape `try_func.py` takes: the
function, plus whatever `extern` declarations it needs beyond what the
real file already has above it. Use your best `try_func.py` attempt so
far, not a fresh stub -- the permuter refines a near-miss, it doesn't
write one from nothing.

`permuter_setup.py` writes `build-sn/permuter/<func>/` (gitignored,
`build-sn/` in `.gitignore`): `base.c` (what the randomizer mutates),
`target.o` (retail, built through the exact same pipeline), `compile.sh`
and `settings.toml`. `permuter_run.sh` sets `PATH`/`PYTHONPATH` for the
vendored pieces from bootstrap and runs `permuter.py` itself. `-j$(nproc)`
uses every core in the container; `--stop-on-zero` exits as soon as it
finds an exact match instead of running until you kill it.

## Several functions overnight

Point it at every near-miss with notes under `build-sn/try/` (the "best
candidate" line in each function's `NOTES.md` says which file to use), and
give each one a time budget instead of running forever:

```bash
for pair in \
    "func_L00_002352D0:cand4.c" \
    "func_L00_00233B08:cand4.c" \
    "func_L00_0023D750:cand2.c" \
    "func_L00_0023BAB8:cand.c" \
    "func_L00_0023B610:cand3.c" \
    "func_L00_00236DE8:cand3.c" \
    "func_L00_00235CA0:cand.c" \
    ; do
  name="${pair%%:*}"; cand="${pair##*:}"
  bash tools/docker/run.sh python3 tools/permuter_setup.py "$name" "build-sn/try/$name/$cand"
done

for d in build-sn/permuter/*/; do
  timeout 1800 bash tools/docker/run.sh bash tools/permuter_run.sh "$d" -j"$(nproc)" --stop-on-zero
done
```

Results land under each `build-sn/permuter/<func>/output-<seed>-<n>/` (a
`source.c` and a `diff.txt` per improvement found; `output-0-1` is the
first, and usually the best). `--stop-on-zero` still applies per function,
so a run that finds an exact match early moves on to `output written`
rather than burning the rest of its 30-minute slot; one that doesn't just
runs until the `timeout` kills it. Raise `-j` cautiously: each worker is
its own Wine process, and the container is already running one per core
by default.

## Checking a result

A score of 0 is decomp-permuter's own signal, computed by its own
objdump-based diff -- not proof. Always re-check with `try_func.py`,
exactly as `LEVERS.md` says to for anything else:

```
bash tools/docker/run.sh python tools/try_func.py func_X candidate.c --no-budget
```

Where `candidate.c` is the permuter's `decls.c` (in the same
`build-sn/permuter/<func>/` directory, unchanged from setup) plus the
`output-*/source.c` function body, reassembled by hand or with:

```bash
python3 - <<'EOF'
import re, sys
d, name = "build-sn/permuter/func_L00_00235608", "func_L00_00235608"
text = open(f"{d}/output-0-1/source.c").read()
m = re.search(r"^(?!extern\b)[A-Za-z_].*?\b" + re.escape(name) + r"\s*\(", text, re.M)
lines, start = text.splitlines(), text[:m.start()].count("\n")
depth = seen = 0
for j in range(start, len(lines)):
    depth += lines[j].count("{") - lines[j].count("}")
    seen = seen or "{" in lines[j]
    if seen and depth == 0:
        func = "\n".join(lines[start:j + 1]); break
open(f"{d}/verify.c", "w").write(open(f"{d}/decls.c").read() + func + "\n")
EOF
```

`EXACT` there means the same thing it always does -- ready to hand-rewrite
into clean C for `src/`, never pasted in verbatim (see "What it doesn't
do").

## Candidates defined through an alias

When the source file already declares the function with another prototype, a
candidate defines it as `void impl(char *m) __asm__("func_X");` (see
LEVERS.md). `permuter_setup.py` accepts that: base.c calls the function
`func_X` (pycparser and the scorer need the real symbol), and `compile.sh`
turns the name back into `impl` before compiling (`permuter_compile.py
--defname`). Declarations the candidate needs (including the alias line) are
the ones above the definition, as before.

## What it doesn't do

- **Only works on a live `INCLUDE_ASM` stub.** `target.o` is built from
  the stub's own embedded retail assembly; a function already rewritten
  to near-miss C in `src/` has no stub left, so `permuter_setup.py`
  refuses it. Keep refining that one with `try_func.py` by hand, or with
  `tools/permute.py` if the near-miss is a pure reordering.
- **Never touches `src/`.** `permuter_setup.py` only reads the real
  source tree (to splice a candidate in, the same way `try_func.py`
  does); the permuter itself writes only under `build-sn/permuter/`.
  Nothing here lands a match -- that's still a human, `try_func.py`, and
  a normal commit.
- **Strips GCC attributes to parse the function.** `base.c` is
  preprocessed with `-D'__attribute__(x)='` so decomp-permuter's
  pycparser-based AST tool can read it (its own recipe, upstream
  `USAGE.md`). A local like `float m[16] __attribute__((aligned(16)))`
  loses that attribute in every permuter candidate -- harmless for
  scoring here (nothing in these five functions' results depended on it),
  but the attribute has to be added back by hand if a permuter result
  gets rewritten into `src/`.
- **`compile.sh` recompiles the function's whole containing source file**
  (`tools/try_func.py`'s own `build()`, which every candidate goes
  through unchanged) **on every iteration**, not just the one function.
  That keeps the pipeline byte-for-byte identical to `try_func.py` --
  same `file_cflags.py` flags and `fix_orphan_hi.py`/`ps2eeas_nops.py` passes -- but
  it means each iteration costs roughly what one `try_func.py` run costs,
  and the file's other, already-decided code shows up as a constant
  (always-matching, zero-penalty) prefix/suffix in every score. Iteration
  throughput is bottlenecked by Wine process startup, same as everywhere
  else in this build.

## What we found running it

Verified 2026-09-28 against five real near-misses (`build-sn/try/func_L00_*/NOTES.md`):
base scores were nonzero and tracked `try_func.py`'s verdict in every
case (`func_L00_00235608`, `BYTES 4/200`, scored 20 -- all register-diff
penalties; `func_L00_0023BAB8`, the one actual `SIZE` mismatch, scored
highest at 665, with insertion/deletion penalties `try_func.py`'s masked
byte count can't show). A `-j8 --stop-on-zero` run on
`func_L00_00235608` found a score-0 candidate at iteration 105 (under two
minutes): swapping two of the four tail stores' source order --
`+0x68, +0x54, +0x6C, +0x50` instead of `+0x54, +0x68, +0x6C, +0x50` --
compiles to a byte-identical match. `try_func.py --no-budget` on the
reassembled candidate confirms `EXACT`. It was then landed
by hand as a normal commit (`feat(overlays): func_L00_00235608 exact
match`), so that function is no longer a stub the permuter can take.

That result is exactly the shape the permuter is good at: a pure
store-order tie between a handful of independent stores, the same family
`tools/permute.py` targets, just past its practical size (or here, simply
found faster by random search than by hand). Expect similar luck on
**store-order and small register-allocation near-misses** generally --
the FAQ upstream is blunt about this ("best towards the end, when mostly
regalloc changes remain"). Less promising: the loop-reversal and
larger-scale control-flow shapes in `LEVERS.md`'s levers list (branch
inversion, loop-invariant placement, branch-invariant duplication) are
mostly outside what the randomizer's passes touch -- those still want a
hand rewrite first, with the permuter only as a follow-up polish once the
control flow already matches and what's left is regalloc.
