# Near misses (`nonmatching/`)

A near miss is a candidate for a level function that compiles and comes
close to retail but is not EXACT yet. The best one per function is kept
in the repository, so everyone can see what is open and how close it is,
pick it up instead of starting over, and not redo it by accident.

## Where they live

`nonmatching/<source dir>/<func>.c`, one file per function, for example
`nonmatching/l17_fleet/func_L17_002EDE50.c`. Each file is:

- a header comment: the function's source file, the best verdict so far
  (`BYTES n/size` with the share of matching bytes, or `SIZE ours/retail`),
  the date it was checked, and the last notes from the workers who tried;
- the candidate, exactly as `tools/try_func.py` takes it (externs, then
  the function).

[`nonmatching/README.md`](../nonmatching/README.md) lists every one,
closest first. It is generated: never edit it by hand.

Nothing builds these files into the game, and none of the tools that read
`src/overlays/` see them. The function's retail assembly (`INCLUDE_ASM`)
stays in its source file until a candidate is EXACT; only then does the C
go into `src/overlays/`, and the staged file is removed.

## In the progress report

`tools/gen_progress_report.py` builds every staged file in its source
file and records its share of matching bytes as the function's
`fuzzy_match_percent` (a size mismatch scores 0: it moves every byte after
it). Matched code and matched functions still count EXACT functions only;
a staged one is capped below 100 and is never finished. decomp.dev shows
the fuzzy figure as partial progress.

## Working on one

- Start from the staged file: `wave.py` packets already do (`best.c` is the
  closest of the staged file and the run logs). By hand:
  `python tools/try_func.py func_X nonmatching/<dir>/func_X.c --diff`
  works directly; the header is a comment.
- Claim the function first (`tools/claims.py claim <you> func_X`), as for
  any other.
- Workers never edit `nonmatching/`. Their runs are logged under
  `build-sn/try/`; the lead re-stages what got closer.

## For the lead

- `python3 tools/wave.py stage [--level NN] [--max 0.15]` stages every
  overlay stub whose closest attempt is within 15% of its bytes (or of its
  size), unless the staged file is already as close. Run it after landing
  each batch. A candidate that no longer compiles against its file is
  repaired the way landing repairs one (an extern the file now declares
  another way is dropped, a typedef it now also defines is renamed); one
  that still fails is left out, and one that comes out EXACT is reported
  for `wave.py salvage` instead.
- `bash tools/docker/run.sh python tools/nonmatching.py check` re-checks
  every staged file against its source as it is now and rewrites the
  verdicts and the index: neighbours landing can change a near miss.
- Landing a function (`land`, `salvage`) removes its staged file.
- Commit `nonmatching/` with the batch and its report.
