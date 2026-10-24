# Contributing

Read `AGENTS.md` first: it holds the rules that matter. This page is the
review procedure.

## Before you start

1. Read `PROMPT.md`, `docs/METHOD.md` and `docs/EVIDENCE.md`.
2. Check `docs/STATE.md` for the current state and the next open task.
3. Check the sibling projects (see `docs/REUSE.md`) for an authentic source
   form before reconstructing a routine by hand.

## Where to work

- The **root** is the only writer in the clone. It integrates, gates,
   catalogs, documents and opens pull requests.
- An **agent** works one lot, in `work/<LOT>/` only, and hands over its
   results. It never edits `src/`, `config/` or `tools/`.

## What a pull request contains

Use the pull request template: **What** (the scope, the symbols or programs
involved, and whether this promotes C, retains an attempt, changes a compiler
profile, or changes behavior separately), **Checked** (the tested commit, the
exact commands, the actual results, and the complete-program gates for the
affected placements), **Remaining limits** (the unverified scope, the uncertain
bounds, the parked attempts with their first difference and their reopening
condition).

## Promotion

A body is promoted when:

- it is linked at **every** reviewed placement of its extent;
- the complete-program gate rebuilds the affected programs and their hashes
  equal the pinned retail hashes;
- the previously measured load images still match;
- the source is authored C/C++ with no injected instructions, and its name,
  type and layout are evidenced.

Assembly fallbacks preserve the baseline and earn no C credit. A body that
does not match stays in `nonmatching/`, indexed by function, with its first
difference.

## Hygiene

Review your own diff before pushing: no game data, no disassembly, no build
output, no proprietary tools, no personal paths. The repository guard and the
unit tests must pass. Describe what was verified and what remains
provisional.
