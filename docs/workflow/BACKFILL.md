# When to stop matching, and backfill in C

For anyone, person or agent, matching functions in one of OpenRAC's
decompilations. It answers two questions: **when is a function not worth
another attempt**, and **what do you write instead**.

The short version: some functions cannot be matched from C at all, and some
can but have already cost more than they return. Neither kind blocks the
native port, because the port only needs C that *does what the function
does*. That C is the backfill. It is kept apart from the matching source and
never counted as a match.

## Two kinds of C

| | Matching C | Functional C (the backfill) |
|---|---|---|
| Compiles to | the retail bytes, exactly | anything that behaves the same |
| Proven by | the build: the linked program equals retail | fcheck: the same result and memory as retail on real calls (the gate, below), or nothing yet |
| Lives in | `src/` | `nonmatching/functional/`, listed in `nonmatching/functional.tsv` |
| Counted in the progress report | yes | never |
| Read by | the matching build and the port | the port only |

`games/rac1` has this in place (321 functions listed on 2026-10-13); the
port reads the list through `port/game/rac1-pal/hostgen.json` ("candidates").
A decompilation without the list yet should add the same two paths, not a new
scheme.

## Functions that C cannot match

Recognise these before spending an attempt. Each was measured, mostly on
Ratchet & Clank (PAL), and the same signs hold in the other games.

**1. It is not a function.** The catalogue ends a function wherever the next
address looks like a start, so one real function can show as several entries.
Of the 241 unmatched entries of 400 bytes or less that Ratchet & Clank had
left on 2026-10-12, **120 were pieces of a neighbour**. The signs:

- it reads a register it never sets (and that register is not an argument);
- it branches to an address outside its own bytes, or a neighbour branches
  into it;
- it has no `jr $ra`, or starts with instructions that only make sense after
  a prologue someone else ran;
- it is one or two words on an 8-byte boundary after a function that already
  returned (what the linker left of a function it stripped).

What to do: do not write C. Join it to its owner
(`config/overlays/joined.tsv` in rac1; fix the catalogue in `tools/levels.py`
in rac2) or list it as a linker remnant. The owner's C then covers it.

**2. It was written in assembly.** No compiler produced it, so no C
reproduces it. The signs:

- COP2 / VU0 macro instructions (`lqc2`, `sqc2`, `vmul`, `vadd`, `vdiv`,
  `qmfc2`, `cfc2`);
- `$at` used as a working register;
- a return with a live instruction after its delay slot (`jr $ra; lui $ra, …`);
- an `addi` (which traps on overflow) where the compiler always writes
  `addiu`; a trailing `sync`;
- multimedia instructions in a two-instruction body.

What to do: mark it `ASM_FUNC` in place of `INCLUDE_ASM`. It stays assembly
and counts as finished, because the original was assembly too. Mark it only
on this evidence (or because it is byte-identical, and longer than 16 bytes,
to a routine another decompilation already classifies this way), never
because it resisted matching. For the port, write functional C: a VU0 dot
product is three multiplies and two adds in plain floats.

**3. The build cannot express what it needs.** The builds reproduce the retail
compilers and change nothing they emit; flags apply to whole files. A function
that would only match with a step that edits compiler output, or with flags of
its own inside a file that needs others, is not matched by doing that. One
real case: two functions in one file that each need the same global declared
at a different size. Record what it needs in its near-miss note; it may become
possible when the file is split the way retail's was.

## Functions that C can match, but not today

Everything else is matchable in principle. What stops an attempt is cost.
These are the measured walls:

- **The compiler is not the problem.** On 2026-10-12 Ratchet & Clank's level
  code was compiled with three other compilers (SN 2.95.2, SN 2.95.3 v1.36,
  and the patched 2.9-ee the NTSC decompilation uses). None turned a single
  one of 492 near misses into a match. Do not go looking for another compiler
  or another flag for one function.
- **Ties.** Register allocation and instruction scheduling have ties that
  wording does not move: the loop counter in `$s1` where retail has `$s0`, a
  store that lands in a delay slot. When three different wordings compile to
  the same bytes, a fourth will too.
- **Size.** Small models stop being productive above about 1.5 KB: two waves
  of the smallest tier on 1.7 to 3 KB functions matched 0 of 88.

So every attempt has a bound, set before it starts:

| Stop when | |
|---|---|
| the run budget is spent | 8 to 15 compiles per function, by tier |
| three different wordings give identical bytes | a tie; rewording will not move it |
| about ten compiles pass without getting closer | you are circling |
| it shows a sign from the list above | it was never going to match |

Stopping is a normal result. Leave the best attempt as a near miss
(`nonmatching/<area>/<function>.c`) with two or three lines: what the function
does, where it differs, what would unblock it. Then it goes **up one tier**
(small model, then mid, then large), each starting from that note. A function
does not go round the same tier twice.

## Before any model

Much of what looks like matching work needs no attempt at all. In this order:

1. Is it a piece or a remnant? (above)
2. Is it handwritten? (above)
3. Is the same function already matched somewhere? Another version of the
   same game, or another game: `tools/xmap.py ports` lists them and
   `tools/port.py` carries the C across and checks it. Identical bytes with a
   compiler of the same family is nearly always a match; across compiler
   families, about a third pass as they are.
4. Does an exact attempt exist that never landed? (`wave.py salvage` in rac1.)
5. Is it a variant of a matched function, the same instructions with another
   constant? (`overlay_variants.py clone` in rac1.)

Only what is left is worth a model's time.

## Writing the backfill

When a function has stopped and the port needs it, write functional C. A near
miss that is 95% byte-identical is usually already that; start from it.

- **Same behaviour, not same bytes.** The same reads, writes and calls, with
  the same arguments, in the same order; the same return value. Locals,
  helpers and clearer control flow are fine. Keep the function's name and
  signature.
- **Write it for the host too.** These broke the port in practice:
  - PlayStation 2 floats never produce NaN or infinity. Take `sqrt` of an
    absolute value where retail could go slightly negative, and clamp.
  - A call written `f()` that relied on a register still holding an argument
    must pass that argument.
  - `long` is 64 bits and `long long` 128 in the original compilers.
  - A pointer is a 32-bit game address, not a host pointer.
- **Say how sure you are.** The file's first line is a comment:
  `/* functional: high|medium|low. <what differs from retail, and why that
  does not change behaviour> */`. Say `low` when any part is a guess.
- **Check what can be checked.** It must compile in place of its stub. Byte
  similarity is evidence, not proof: a one-instruction difference can be a
  wrong constant. Reading it against the assembly is not proof either: a
  wrong constants address and a literal read from the wrong copy both shipped
  in C that read right. The proof is fcheck (the gate, below).
- **Keep it out of `src/`.** It goes in `nonmatching/functional/` with a row
  in `functional.tsv`. The progress report must not change.

What it is not: a way to raise the percentage, or a place for a match that
only works with a trick. If a later attempt matches the function, the match
goes in `src/` and the functional file and its row are removed.

## The gate: done means fcheck says SAME

A backfilled function is **done only when fcheck says SAME on captured calls
of the retail game**. Anything fcheck cannot reach is **unchecked**, which is
not done; it stays on the list with its reason. Reading the C, a high
confidence line and a clean build are not a verdict.

fcheck (rac1-decomp `tools/fcheck.py`, test-only) plays the PAL disc headless
into level 0, captures the machine state at real calls of a function, runs the
retail code from each state in OpenRAC's interpreter build, then runs the
candidate from the same state and compares memory (but the dead stack and the
scratchpad), the result by the return type, the registers a function keeps,
and the order of device accesses and system calls. It is an offline check: it
is never shipped and never used to play.

The verdicts are kept in files OpenRAC owns, one per backfill table and keyed
by function name, so the tables themselves (`functional.tsv` is synced from
rac1-decomp, both are read by hostgen) are never changed:
`port/game/rac1-pal/fcheck.tsv` for `nonmatching/functional.tsv` and
`port/game/rac1-ntsc/hand/fcheck.tsv` for `hand/hand.tsv`. Columns, after
`name`:

| Column | Meaning |
|---|---|
| `status` | `same`: compared real calls, all equal. `differs`: a difference (the note says where; fix the C, not the verdict). `unchecked`: fcheck could not compare (the C did not build or place, every state was skipped, no oracle for that version); the note says why. `no-states`: the capture run (title, menu, level 0) never calls it. `not-executable`: not a boot-executable `func_XXXXXXXX` (level and overlay code; fcheck runs those only for now) |
| `date` | the day it was run |
| `fcheck` | the rac1-decomp commit fcheck ran from (`+dirty` when `tools/fcheck.py` had local edits) |
| `csum` | the first 12 hex digits of the sha1 of the C file; an edit after the verdict makes it stale |
| `note` | the call count, the differing lines, or the reason for `unchecked` |

Only `same` counts as done. `differs` is a bug to fix. The rest are open: say
so when you hand the function on, and never call it verified. A backfill row with no
verdict has not been through the gate.

Run it (OpenRAC built so that `build/runtime/openrac-fcheck` and
`openrac-boot` exist; the PAL disc in place; a container runtime for the
retail compiler; headless, it opens no window):

```sh
python3 tools/fcheck_gate.py run                       # every PAL row: capture what is missing, check in parallel, write fcheck.tsv
python3 tools/fcheck_gate.py run --only func_001F7E98  # one or a few rows after you changed their C
python3 tools/fcheck_gate.py report [--strict]         # joins each backfill table with its verdicts: counts, differing, stale and unverdicted rows; --strict exits 1 on any
python3 tools/fcheck_gate.py us                        # the US table: unchecked, no US oracle
python3 -m unittest tools/test_fcheck_gate.py
```

`OPENRAC_ROOT` names the OpenRAC checkout with the runtime build and
`OPENRAC_DECOMP` the rac1-decomp checkout (defaults: `~/Projects/RAC/OpenRAC`
and `~/Projects/RAC/rac1-decomp`). Each log is kept in rac1-decomp's
`build-sn/fcheck/gate/logs/`. The gate copies each C file into
`build-sn/fcheck/gate/`, which is ignored; it edits nothing else there.

**How a worker reports.** A backfill worker hands a function on as one of:

- `SAME`, with the call count from `tools/fcheck.py FUNC FILE` (or the row the
  gate wrote). Then it is done.
- `unchecked`, with the reason in one line: no states (never called in the
  capture run), not executable (a level function), the candidate did not
  build, every state skipped, or no oracle for the version. It is delivered
  as unchecked and listed as such in the report, never as done or verified.
- `differs`: not delivered. Fix the C and run fcheck again, or hand the
  function back with the difference as it printed.

A report line that says a backfill is finished without one of those is
incomplete. The lead runs `fcheck_gate.py run --only ...` over the batch
before landing and commits `fcheck.tsv` with the files.

**The US table has no oracle.** `port/game/rac1-ntsc/hand/hand.tsv` rows are
all `unchecked (no US oracle)` in `hand/fcheck.tsv`: fcheck's interpreter build, its hooks file and
its capture list know the PAL disc and the PAL executable only. A US oracle
would need: an interpreter build that boots the NTSC-U disc (its executable
SCUS_971.99, its own hooks file, the button presses for its menus and a level
start); a capture list keyed by the US executable's `FUN_` names, addresses
and code bytes (fcheck reads function bytes from the PAL assembly and
executable, and only takes `func_XXXXXXXX`); a way to compile a candidate in
place of its stub with the NTSC decomp's compiler and place it with US
addresses (the sections from `core.lit` on sit 0x100 lower than PAL's); and a
check of what the PAL oracle cannot say. Until then a US function whose PAL
counterpart the version map calls `same` has the PAL verdict as evidence (not
a verdict of its own), and the rest stay unchecked.

## Picking what to backfill first

The port does not need every function at once: it needs the ones gameplay goes
through. `port/tools/hostgen/priority.py` ranks the weak ones by static
analysis, from what hostgen already wrote, without running the game:

```sh
cmake --build --preset release            # in port/: hostgen writes gen/report.json
python3 port/tools/hostgen/priority.py --game port/game/rac1-pal/hostgen.json \
    --gen port/build/release/games/rac1-pal/gen \
    --dump games/rac1/baserom/overlays --exe games/rac1/baserom/SCES_509.16 \
    --out build/priority/rac1-pal.md
python3 -m unittest port/tools/hostgen/test_priority.py   # its test (no Clang)
```

A weak function is one with no C, C hostgen cannot translate, a call that
leaves an argument out (filled with 0), C that is not matched (a candidate or
the port's hand C, weighted by its confidence line: low, medium, then high;
VU0 microcode first of all), a candidate whose declarations hostgen's prep
renamed wrongly (in `extern T a(..), b(..);` the calls to `b` reach `a`), or
matched C its callers declare differently (hostgen's `call_site_mismatches`:
more or fewer integer or float arguments than it takes, or a result it does
not return; extra arguments it never reads are left out). For each level program it follows hostgen's `call_graph`, the
retail code's own calls where only the level knows the target (the level
hooks, functions with no C yet), and every function the level's `vtbl`,
`camvtbl`, `sndvtbl` and data records point at (the moby class tables). The
report ranks what every level reaches, or what sits on a hot path (hero,
camera, collision, HUD, 2D draw, moby update and draw, sound, level load),
first; then by the number of levels; then by how weak; and suggests a tier
(Haiku for small plain functions, Sonnet up to 1 KB or for one argument read
from the assembly, Opus for large or hand-written ones).

What it cannot see: calls through pointers computed at run time and kept
outside those records (a pointer built in the heap, a table in the
executable's own data), so "no level reaches it" means nothing it follows
does. Hot paths come from source units, readable names and table roles,
carried two calls down; read them as a hint. Without `--dump` it uses the
roles in `config/overlays/names.tsv` instead of the records.

## Deciding, in one pass

```
is it a piece of a neighbour, or a linker remnant?  -> join or list it; no C
is it handwritten assembly?                         -> ASM_FUNC; functional C for the port
is the same code matched elsewhere?                 -> port it mechanically
try to match, within the bound for its tier
  matched                                           -> src/, the build proves it
  stopped                                           -> near miss with a note; next tier up
no tier left, or the port needs it now              -> functional C, with its confidence
  fcheck SAME on captured calls                     -> done
  unchecked (with the reason) or differs            -> not done
```

## Where the pieces are

| | Ratchet & Clank (PAL) | Going Commando |
|---|---|---|
| Near misses | `nonmatching/<area>/` | `nonmatching/<area>/` |
| Functional C | `nonmatching/functional/`, `functional.tsv`; verdicts in OpenRAC's `port/game/rac1-pal/fcheck.tsv` | not set up yet: add the same two |
| Pieces of one function | `config/overlays/joined.tsv` | the catalogue, `tools/levels.py` |
| Handwritten | `ASM_FUNC`, counted as finished | `ASM_FUNC`, counted as finished |
| Worker bound and protocol | `docs/QUEUE.md`, `docs/AGENT_WORKFLOW.md` | `docs/AGENT_WORKFLOW.md` |

The measurements behind this page are in the commit history of 2026-10-12
(the compiler comparison, the small-function sweep and its joins) and in
[games/rac1/docs/NONMATCHING.md](../../games/rac1/docs/NONMATCHING.md).
