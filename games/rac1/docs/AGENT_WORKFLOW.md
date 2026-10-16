# Agent workflow

How matching runs when models do it: one lead model plans waves, launches
cheap workers, reviews what they match and lands it, and a strict tool
decides what counts as a match. The goal is the most matched bytes per
token. The setup follows
[Thief3-Decomp's tiered workflow](https://github.com/Veradictus/Thief3-Decomp)
and every setting below was measured here (2026-09-30; the model tiers
again on 2026-10-07).

[WORKFLOW.md](WORKFLOW.md) covers the build and the manual loop,
[QUEUE.md](QUEUE.md) is the workers' protocol, [OVERLAYS.md](OVERLAYS.md)
the level-code catalogue the waves draw from.

## At a glance

| Role | Model | Does |
|---|---|---|
| Lead | Opus 5.5 | plans, launches, refills, reviews, lands, commits; matches nothing itself |
| Tools first | no model | variants (`clone`), EXACT runs that never landed (`salvage`), Lombyte ports, linker remnants and joins: whatever a script can close never reaches a model |
| Swarm worker | Haiku 5.5 (`model: haiku`), 8 to 10 at once | small level functions, about 400 bytes and under, whole functions only; a first pass that matches about a third and leaves notes and a staged candidate for the rest |
| Escalation worker | Opus 5.5 (`model: opus`), 2 to 4 at once | what the swarm left close (right size, a few bytes off) and every function of 1 KB or more: one function per agent, 20 to 30 runs ([LONG_FUNCTIONS.md](LONG_FUNCTIONS.md)) |
| (rarely) | Sonnet 5.5 | only library code with an answer key (a reference source); on game code it neither matches what Haiku can't nor closes what Opus does: see [Model tiers](#model-tiers) |

- The standing fleet is 8 to 10 Haiku swarm workers and 2 to 4 Opus
  escalation workers, about 12 agents. Work flows up: tools, then Haiku on
  the small functions, then Opus on Haiku's near misses and on the long
  functions, each starting from the notes and best candidate of the tier
  below ([Model tiers](#model-tiers)). 80% of the level code still
  unmatched is in functions over 1 KB ([Long functions](#long-functions-opus)),
  so the Opus tier never runs dry; the swarm keeps the small functions,
  which are many, off it.
- The claims and the landing lock
  ([Several agents at once](#several-agents-at-once)) are what make a
  dozen agents in one checkout safe.
- A worker's whole prompt is one line:

  ```
  Read docs/QUEUE.md and follow it exactly. WAVE=q5 ID=k03 N=8 COUNT=2. Keep claiming until you have handled 8 functions or the claim prints QUEUE EMPTY; do not stop after your first claim.
  ```

  `WAVE` is the wave's name, `ID` the worker's id (never reused within a
  wave), `N` how many functions it handles, `COUNT` how many it claims at
  a time.

## Long functions (Opus)

Picking a function for an Opus worker:

1. Over 1 KB, untried, and not blocked by `tools/rank_candidates.py`
   (`classify` must not say `blocked`: a `sq $zero` 128-bit zero store is a
   real wall, our gcc writes `por` then `sq`). Two trial picks skipped this
   and their agents stopped at once.
2. Most of its callees already matched, so their prototypes, struct
   offsets and globals are settled: rank by the share of matched callees.
3. Not in a file another worker holds a function of.

`python3 tools/wave.py long NAME func_X ...` sets each one up (dossier,
m2c sketch, `PACKET.md`, a 30-run budget in `build-sn/try/func_X/opus/`),
refuses a blocked one, and prints each worker's one-line prompt:
`Read docs/LONG_FUNCTIONS.md and follow it exactly. FUNC=func_X ARM=opus.`
Its runs land through `wave.py salvage`, and a near miss goes to the next
near wave; both read the arm's run log.

Trial (2026-09-30, both models with LONG_FUNCTIONS.md, 20 runs each):

| Function | Opus 5.5 | Sonnet 5.5 |
|---|---|---|
| func_L00_00210558, 2.2 KB | EXACT in 16 runs, 221K tokens | never the right size, 19 runs, 322K |
| func_L02_002D8B80, 3.7 KB | 8 bytes off in 14 runs, 213K | 8 bytes off in 20 runs, 276K (the same two instructions) |
| func_L18_002F86E8, 1 KB | 47 bytes off in 20 runs, 195K | 4 bytes too big in 13 runs |

Opus closes in run after run (176, then 150, then 13 bytes off, then
EXACT); Sonnet stalls at the size. Neither moves a scheduler tie: the last
few bytes of a near miss are the compiler's choice, not a reasoning
problem.

## The pieces

| Piece | What it is |
|---|---|
| `tools/wave.py plan NAME --queue --overlay --family` | picks the wave's functions and writes each one's dossier and run budget |
| `tools/wave.py claim NAME ID` | a worker's call: locks the next functions and prints their packets |
| `tools/try_func.py` | compiles one candidate in a scratch copy of its file and compares it with retail |
| `tools/wave.py status NAME`, `tokens NAME` | verdicts from the run logs; tokens per worker and per match |
| `tools/wave.py land NAME --batch` | applies every exact candidate and re-checks it in its file |
| `tools/wave.py salvage [--ports] [--level NN]` | lands every stub that already has an EXACT run logged; Lombyte ports only with `--ports` |
| `tools/wave.py stage [--level NN]` | shares near misses: each function's closest attempt (within 15%) goes to `nonmatching/` ([NONMATCHING.md](NONMATCHING.md)) |
| `tools/wave.py long NAME func_X ...` | sets up Opus long-function workers: dossier, m2c sketch, packet, per-arm budget |
| `tools/wave.py plan NAME --queue --overlay --near` | a near wave: earlier attempts within 10% of retail, each packet with its best candidate and what still differs (`tools/near_diffs.py`) |
| `tools/overlay_variants.py clone` | matches variants of matched functions with no model |
| `.claude/agents/match-worker.md` | the sub-agent type: Read, Write, Edit, Grep, Glob, Bash; Sonnet by default |
| `docs/QUEUE.md` | the workers' whole instruction set, under 2K tokens |
| `tools/claims.py` | shared claims and the landing lock, for several agents in one checkout |

The gate is `try_func`. For level code its `EXACT` is strict: the
candidate is linked at the function's address in its level and every byte
and relocation is compared ([OVERLAYS.md](OVERLAYS.md)). Nothing a worker
says counts; only what `try_func` logged.

## The lead's loop

1. **Prepare.** From a clean tree:

   ```
   bash tools/docker/run.sh python tools/overlay_variants.py clone
   python3 tools/wave.py plan q6 --queue --overlay --family --count 64 --min-size 32 --max-size 600 --budget 6
   ```

   `clone` first, so variants of already matched functions never reach a
   worker. Then `python3 tools/wave.py salvage` (and `salvage --ports`,
   committed apart): a wave stopped before landing, or a file that
   clashed, leaves EXACT runs behind as stubs. `plan` skips functions that are matched, were tried by an
   earlier wave, are fragments, or hit a known wall
   (`tools/rank_candidates.py`).
2. **Launch** the fleet with the Agent tool (`subagent_type:
   match-worker`, in the background), all in one message:
   - 8 to 10 Haiku swarm workers (`model: haiku`), each with its own `ID`,
     on functions of about 400 bytes and under;
   - 2 to 4 Opus escalation workers (`model: opus`), one function each:
     the swarm's closest near misses first (right size, a few bytes off,
     started from the staged candidate and its notes), then long functions
     ([Long functions](#long-functions-opus)).

   Pick the swarm's pool with `plan`, never by hand: it skips entries that
   branch outside themselves. Of a hand-picked batch of 8, 5 were pieces of
   a function the catalogue had cut apart. A one-function `plan` (without
   `--queue`) does not claim: claim each function for the wave with
   `tools/claims.py` so another agent does not take it.
3. **Refill.** A notification arrives when a worker ends. If
   `wave.py status` still shows unclaimed functions, start another worker
   with a new `ID`. Workers sometimes stop after one claim, whatever the
   protocol says; the prompt's last sentence and the refill cover it.
4. **Land** a wave once its own workers are done; the rest of the fleet
   keeps running:

   ```
   python3 tools/wave.py land q6 --batch [--reject func_X ...]
   ```

   It refuses candidates with banned constructs, applies each exact one,
   and re-checks it in its real file.

   **To build before the first mixed round:** landing must skip a file
   while a running worker holds a claim on one of its unmatched functions,
   and land it on a later pass. try_func builds a scratch copy of the whole
   file, so a landing mid-edit breaks that worker's builds: in the trial a
   salvage landing cost a Sonnet worker most of its runs. Only claims on
   functions still `INCLUDE_ASM` count: claims stay in place after a match.
5. **Review** the diff before committing. Reject, and revert to the stub:
   - a read of a local that was never assigned (it reproduces a register
     left by earlier code: the entry is a fragment);
   - register pins, inline assembly, barriers, `volatile` added to pin an
     order (`tools/integrate.py` refuses the first three);
   - anything that looks taken from Sony SDK source or samples
     (CONTRIBUTING.md, "Sources");
   - any change to the build itself that edits compiler output, or a flag
     for one function: options go per file (`config/file_cflags.txt`), and
     a new assembler or linker step needs evidence and a section in
     [BUILD_FIDELITY.md](BUILD_FIDELITY.md) first.
     `tools/check_build_fidelity.py` runs in the report's `--check`.

   The `extern short D_x;` read as `*(int *)&D_x` is not a hack: it is
   this project's way to get a `$gp`-relative access at `-G2`
   (`include/common.h`).
6. **Clone and name.**

   ```
   bash tools/docker/run.sh python tools/overlay_variants.py clone
   python3 tools/names.py apply
   ```

   Every new match can bring its variants; `names.py apply` writes the
   readable names into the new bodies ([NAMES.md](NAMES.md)). Then
   `python3 tools/wave.py stage`: every near miss the batch left goes to
   `nonmatching/` ([NONMATCHING.md](NONMATCHING.md)), so nobody redoes
   it and anyone can pick it up. Landing a function removes its file.
7. **Report and commit.** `bash tools/docker/run.sh python
   tools/gen_progress_report.py --no-build` regenerates the report (about
   2 minutes: it rebuilds and re-checks every file with C, one file per
   CPU), then `python3 tools/gen_progress_report.py --check`. Before it,
   `tools/overlay_file_check.py` (in the container) lists every changed
   file that fails to build or holds a C function that is no longer
   EXACT. The report also scores every staged near miss as
   `fuzzy_match_percent` (never as matched). One commit per batch of
   waves, with `nonmatching/` and the report; code ported from another
   project goes in a commit of its own that credits it. Commit only; the maintainer
   pushes.
8. **Feed back.** Add an idiom to QUEUE.md only when a landed function
   shows it. Fix what the wave tripped over (see
   [What went wrong](#what-went-wrong-and-what-fixed-it)) before the next
   one.

## What a worker does

QUEUE.md in short:

1. `wave.py claim` prints a packet per function: what it calls and uses,
   its assembly, and matched C to start from (the function it is a variant
   of, its nearest relative, short matched functions of its file).
2. One attempt is one message: write `build-sn/try/<func>/pK.c`, run
   `try_func ... --diff`. Attempts for both claimed functions go in the
   same message.
3. It stops a function at `EXACT`, when the budget is spent, when three
   wordings compile to the same bytes, or on a wall, and then writes two or
   three lines in `NOTES.md`.
4. It claims again until `N` functions are handled.
5. Its final message is one JSON line.

Why it is shaped this way:

- **Several functions per worker.** Each worker pays its startup (system
  prompt, protocol) once. The harness counted about 81K tokens for a
  worker handling six small functions, where a one-function worker of the
  earlier waves used about 112K for one.
- **One short protocol, read once.** It replaces WORKER.md and LEVERS.md
  for workers. The lead's output is the most expensive text in the system,
  so the prompt is a line of parameters.
- **The packet comes with the claim.** No searching the tree, no separate
  reads of the dossier and the assembly.
- **No RESULT.md.** `try_func` logs every run; `status`, `tokens` and
  `land` read that log.
- **A run budget of 6**, enforced by `try_func`: matches come early, and a
  miss otherwise spends tokens to the end. In waves q6-q26 a run matched
  about 12% of the time for runs 1-5, 8% for runs 6-10 and 3-4% after, so a
  run on a fresh function is worth about two late ones. What a function
  has left after 6 runs goes to a near wave (`plan --overlay --near`), whose
  worker starts from the best attempt and its remaining differences.

## Picking functions: family order

`--family` orders the pool for reuse ([OVERLAYS.md](OVERLAYS.md),
"Relatives" and "Variants"):

1. functions with a matched relative or variant parent, most similar
   first: their packet carries that C, and the work is a port;
2. one function, the smallest, of each family nobody has matched, largest
   family first: each match opens the most ports;
3. the rest, common code in all 19 levels first, smaller first.

| Wave | Pool | Functions | Exact | Input tokens per match |
|---|---|---|---|---|
| q2 | leftover common code, 97-300 bytes, no family order | 12 | 3 (336 bytes) | 1.61M |
| q3 | `--family`, 64-500 bytes | 12 | 9 landed (1,236 bytes), 1 rejected | 157K |
| q4 | `--family`, 32-600 bytes | 56 | 34 (11,128 bytes) | 1.15M |
| q5 | `--family`, 32-600 bytes, right after q4 | 64 | 44 (13,316 bytes) | 325K |

Input tokens include cache reads, as `wave.py tokens` counts them. q4's
functions average 327 bytes against q3's 137, and a long function means a
long conversation re-read at every run, so tokens per match grow faster
than size: q4 cost about 3,500 input tokens per matched byte.

q5 shows what the order is for. Its functions were as large as q4's, but
q4's matches were their relatives, so most packets carried C to port: the
workers needed about half the runs, and a matched byte cost about 1,100
input tokens, a third of q4's. Run waves back to back, each one planned
after the last has reported.

Matching `func_L01_00252E80` in q3 brought 17 variants with it through
`clone`. Across the first day the clone tool matched 37 functions with no
model.

## Model tiers

**Haiku 5.5, 2026-10-07: the swarm tier.** 18 one-function workers
(WORKER.md, budget 15 runs) on small level functions, in two batches.

| Batch | Functions | Exact | Runs per worker | Tokens (per match) |
|---|---|---|---|---|
| never tried, 104-192 bytes | 8 | 4, each after a join the worker named | 2 to 9 | 0.91M (229K) |
| tried before (1-6 runs each), 132-396 bytes, 7 with a staged near miss | 10 | 2 | 3 to 10 | 1.03M (514K) |

Tokens are the harness's per-agent totals.

- **What it does well.** It recognises a function the catalogue cut apart
  (five times, naming the missing piece and its address each time) and an
  already matched twin to reuse (func_L00_001F75F8 is the executable's
  func_001F2A38). It follows the rules and reports walls plainly: an order
  only `volatile` gives, a 16-byte argument no declaration of the callee
  passes the way retail does, a file's `short` declaration of a global
  that forces `$gp` where the function needs `lui`.
- **Where it stops.** Register-allocation and scheduling ties: it ends a
  function after three wordings with the same bytes, which is where Opus
  usually still finds the form. Four of batch two's near misses ended
  that way at the right size. It wasted a few runs (a compile error, a
  flag the compiler does not have), and one worker wrote to `/tmp` against
  its brief.
- **So:** run it wide on the small functions, then send what it left
  close to Opus with its notes. A start from a good candidate is cheap for
  Opus: on the same day Opus finished other workers' 2-byte residue in 2
  runs and a right-size 197-byte one in 9.
- **Still to measure:** Haiku as queue workers (QUEUE.md, several
  functions each) instead of one-function workers; queue workers paid
  their startup once, about 81K tokens for six small functions against
  112K for one in earlier waves. And Haiku's cost per match against
  Opus's on the same pool at current prices: that decides whether the
  swarm pays for its matches or mainly prepares Opus's.

**Sonnet 5.5.** It matched library code well where a reference source
was at hand (60K to 100K tokens per function), but 0 of 15 game
functions in its 2026-09-24 batches and 1 of 10 in a one-function pilot
(2.1M tokens), and it stalls at the size of long functions where Opus
closes them ([Long functions](#long-functions-opus)). Haiku's leftovers
are exactly that kind of residue, so the ladder goes from Haiku straight
to Opus.

**Haiku 4.5, 2026-09-30, for the record.** Two trials, each one queue
with Haiku and Sonnet workers claiming from it at the same time.

| Trial | Model | Handled | Exact | Runs per function | Input tokens per match |
|---|---|---|---|---|---|
| q6: all 85 functions of 8-31 bytes, 5 runs each | Haiku 4.5 | 39 | 15 (38%) | 2.6 | 887K |
| | Sonnet 5.5 | 46 | 18 (39%) | 0.9 | 129K |
| q1: 8-92 bytes, before the fragment filter | Haiku 4.5 | 15 | 4 | | 1.02M |
| | Sonnet 5.5 | 17 | 4 | | 214K |

Half of that 8-31 byte band was tails of larger functions, which neither
model can match on their own; joins (`config/overlays/joined.tsv`) and
the linker-remnant list have since taken many of them out of the pool.

## When to stop, and the backfill

A function that stopped is not owed another round at the same tier. OpenRAC's
`docs/workflow/BACKFILL.md` has the whole procedure; what it says for this project:

- **Not a function**: it reads a register it never sets, branches outside its own bytes, or has
  no `jr $31`. A piece of its neighbour: join it (`config/overlays/joined.tsv`), write no C. 120
  of the 241 small entries left on 2026-10-12 were this.
- **Handwritten** (COP2/VU0 instructions, `$at` as a working register, a trapping `addi`): it
  stays `ASM_FUNC`.
- **A tie**: three wordings, the same bytes. Another compiler does not help either: SN 2.95.2,
  SN 2.95.3 v1.36 and the NTSC decompilation's patched 2.9-ee fixed none of 492 near misses
  (`build-sn/cctest/`, 2026-10-12).
- **Out of budget**: stop, write the note, stage the near miss, and it goes up one tier.

What the native port needs and no tier has matched is written as functional C: the same
behaviour, not the same bytes, in `nonmatching/functional/` with a row in
`nonmatching/functional.tsv`. It is never in `src/` and never in the report
(docs/NONMATCHING.md). CI uploads the list beside the report as `functional.json`
(`tools/gen_functional_list.py`, which reads each file's `functional:` line for its
confidence); ByteMatch shows it on its own line under the Code bar and never as progress.

## Matches without a model

The cheapest match is the one no worker makes.

- **Variants** ([OVERLAYS.md](OVERLAYS.md#variants)): 137 catalogued
  functions differ from another only in a constant, a float or a struct
  offset. `overlay_variants.py clone` takes the parent's C, renames the
  function and the symbols its assembly names differently, replaces the
  numbers that differ, and keeps the result when `try_func` says `EXACT`.
- **Identical copies** need nothing: the catalogue gives every copy of a
  function across the levels one name, so one definition covers them all.

## Trust the records, not the reports

- A worker's final message is a claim. `build-sn/try/<func>/runs.log` is
  the record, and `land` re-checks each candidate in its file.
- `tokens` and `status` read a wave's own log even after a later wave
  retried the function.
- Workers' `idiom` notes are leads. They enter QUEUE.md only with a landed
  function that shows them.
- A worker's blocker note can be wrong. The ones that were right named a
  file that failed to assemble, or a fragment.

## What went wrong, and what fixed it

| Problem | Fix |
|---|---|
| Workers stopped after one claim | the prompt's last sentence; the lead refills with a new `ID` |
| Small catalogue entries were fragments of larger functions | `plan` skips entries that branch outside themselves (245); tails that read registers they never set still get through (about half of the 8-31 byte band), and merging fragments back in the catalogue is still open |
| A match read an unassigned local to reproduce a leftover register | rejected at review; QUEUE.md forbids it |
| A stub branched into a function in another file, so nothing in its file assembled and four functions were lost | `overlay_asm.py --fix-branches` writes such branches as words |
| The catalogue merged functions that differ in a constant, so matched C called the wrong copy | the catalogue compares constants now (`identity()` in `tools/overlays.py`) |
| Matches failed to land: a candidate redeclared a function its file now defines, with the prototype its author guessed | `land` drops the clashing `extern` and re-checks; the file's declaration wins |
| Workers matched variants the clone tool would have matched for free | run `clone` before `plan`, and after every landing |
| 16 functions (7.2 KB) end on a jump whose delay slot the catalogue gave to a 4-byte "function" copied from the executable, so they cannot reach retail's size | open: the catalogue's split must not cut inside a delay slot (then regenerate asm/overlays) |
| A later wave's match was counted for an earlier worker | `wave.py` reads each wave's own run log |
| Two waves queued the same functions: `plan` skipped only claimed ones, so a function another wave had queued but nobody had taken yet went into both | `plan` skips every function already in any wave's queue |
| Waiting agents starved on the landing lock: `land` released it and took it straight back | `land` pauses after each release; waiters poll every 0.2 s for up to 30 minutes |
| The strict check mis-paired `%hi`/`%lo` when retail interleaves the loads of two function pointers | pair in relocation-table order; an orphan `%lo` falls back to address order |
| A hand-landed match was 20 bytes long in its file: a callee it uses was declared only further down, so it was implicitly `int` | re-check every landing in its file, not only the candidate on its own |
| A landed function stopped matching when a neighbour landed later declared the same symbol through a `MACRO_ADDR` alias (the assembler keeps one form per symbol per file) | `land` builds the whole file after each landing (`tools/overlay_file_check.py`) and undoes the landing if any C function in it broke |
| Landing ports one at a time under one lock took a minute each | `land --batch` applies a whole wave at once, re-checks every touched file in one parallel run, and lands only the files with a problem one by one |
| A candidate kept a test `#define` that renamed a declaration its neighbour links against, and the executable failed to link | `integrate.py` refuses a candidate with a `#define`; such a candidate is landed by hand after review |
| A candidate defined under an `__asm__` alias was reported landed though its stub stayed | `land` checks the stub is gone before counting a function as landed |
| A tool-testing agent deleted match history in `build-sn/try/` | workers write only files they create; nothing under `build-sn/try/` or `build-sn/waves/` is scratch |
| A hand-picked swarm batch was mostly pieces of split functions (5 of 8) | pick through `plan`, which skips entries that branch outside themselves; when a worker names the missing piece, add the join and re-check its candidate |
| A joined function's pieces were three nops apart (the assembler pads a short loop), so the join check refused it | `overlay_check.joined_size` allows pieces up to four nops apart and compares the nops with the rest |

## Several agents at once

Several agents can work in one checkout (Claude waves, a GPT agent, a
person) through `tools/claims.py`:

```
python3 tools/claims.py claim gpt func_L05_002559DC     # "claimed", or "taken by <owner>" (exit 1)
python3 tools/claims.py release gpt func_L05_002559DC   # giving up on it
python3 tools/claims.py who func_L05_002559DC
python3 tools/claims.py list [OWNER]
python3 tools/claims.py lock gpt      # before writing any file in src/
python3 tools/claims.py unlock gpt    # right after
```

- **Claim before working on a function.** A claim is the file
  `build-sn/claims/<func>`; creating it is atomic, so only one agent gets
  a function. Release it when you give up on it; leave it once matched.
  `wave.py claim` and `plan` use the same claims: waves never take
  another agent's function, and `land` frees a wave's unmatched ones.
- **Take the landing lock around every write to `src/`.**
  `tools/integrate.py --apply` and `wave.py land` take it themselves; an
  agent that edits source files by hand runs `lock` and `unlock` around
  the edit. A lock older than 15 minutes counts as abandoned.
- **Never restore a file wholesale.** `land` puts a file back after a
  failed re-check only if nobody else changed it meanwhile.
- **One owner for the report and the commits.** Only the lead
  regenerates `progress/report.json`, once, when every agent has finished;
  the user decides what is committed. Agents leave their matches
  uncommitted in the tree.
- Matching itself never collides: `try_func` works in a scratch copy in
  `build-sn/try/<func>/`.

## Rules every brief carries

They are in QUEUE.md, so the one-line prompt is enough:

- plain C: no register pins, inline assembly, barriers
  ([LLM_DECOMP_INSTRUCTIONS.md](LLM_DECOMP_INSTRUCTIONS.md));
- write only in `build-sn/try/<func>/`, never delete, never commit, no
  sub-agents, no web;
- compile only through `try_func`;
- never Sony SDK source, samples or headers, or leaked material
  (CONTRIBUTING.md, "Sources");
- never a level address as a number: the symbol the assembly names;
- the build is fixed: no new step that edits compiler output, no
  per-function flags ([BUILD_FIDELITY.md](BUILD_FIDELITY.md)).

## Running it on a smaller plan

With room for 2 or 3 workers the loop is unchanged: plan a wave of 16 to
24 functions, keep 2 or 3 workers busy, refill as they end, land once.
Keep `N=8`, `COUNT=2`. The wave takes longer; the cost per match is the
same.

## Executable functions

The queue tools accept the executable's functions too (`plan` without
`--overlay`), but no queue wave has been run on them yet. Two things
differ there: `try_func` resolves relocations but links nothing, so an
`EXACT` still has to pass the full build, and `land` (without `--batch`) runs that build per
function. What is left of the executable is mostly large functions
without relatives, so level code is where the waves go.
