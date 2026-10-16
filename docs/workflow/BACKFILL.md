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
| Proven by | the build: the linked program equals retail | nothing; it is believed, then exercised |
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
  wrong constant.
- **Keep it out of `src/`.** It goes in `nonmatching/functional/` with a row
  in `functional.tsv`. The progress report must not change.

What it is not: a way to raise the percentage, or a place for a match that
only works with a trick. If a later attempt matches the function, the match
goes in `src/` and the functional file and its row are removed.

## Deciding, in one pass

```
is it a piece of a neighbour, or a linker remnant?  -> join or list it; no C
is it handwritten assembly?                         -> ASM_FUNC; functional C for the port
is the same code matched elsewhere?                 -> port it mechanically
try to match, within the bound for its tier
  matched                                           -> src/, the build proves it
  stopped                                           -> near miss with a note; next tier up
no tier left, or the port needs it now              -> functional C, with its confidence
```

## Where the pieces are

| | Ratchet & Clank (PAL) | Going Commando |
|---|---|---|
| Near misses | `nonmatching/<area>/` | `nonmatching/<area>/` |
| Functional C | `nonmatching/functional/`, `functional.tsv` | not set up yet: add the same two |
| Pieces of one function | `config/overlays/joined.tsv` | the catalogue, `tools/levels.py` |
| Handwritten | `ASM_FUNC`, counted as finished | `ASM_FUNC`, counted as finished |
| Worker bound and protocol | `docs/QUEUE.md`, `docs/AGENT_WORKFLOW.md` | `docs/AGENT_WORKFLOW.md` |

The measurements behind this page are in the commit history of 2026-10-12
(the compiler comparison, the small-function sweep and its joins) and in
[games/rac1/docs/NONMATCHING.md](../../games/rac1/docs/NONMATCHING.md).
