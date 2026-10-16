## Contributing

The full procedure is in [`docs/WORKFLOW.md`](docs/WORKFLOW.md). In short:

1. Pick a function. `python tools/rank_candidates.py` lists promising ones;
   `python tools/triage.py` sorts everything not yet exact into work queues
   (one-function tasks, per-file batches, and blocked).
   Before decompiling, check whether the function is library code with an
   open-source original (see Sources below).
2. Get a starting point with `python tools/m2c.py func_XXXXXXXX`. That runs
   [m2c](https://github.com/matt-kempster/m2c) with context from
   `sh tools/gen_ctx.sh` (or `python tools/gen_ctx.py` on Windows).
3. Iterate. `python tools/try_func.py func_XXXXXXXX c1.c c2.c ...`
   compiles each candidate through the real pipeline and says `EXACT` or
   how many bytes are off, in seconds, without touching `src/`.
   [`docs/LEVERS.md`](docs/LEVERS.md) is the one-page list of what makes
   a function match. For a side-by-side view, `sh tools/diff.sh
   func_XXXXXXXX` (or `tools\diff.bat` on Windows) runs
   [asm-differ](https://github.com/simonlindholm/asm-differ).
4. Verify from scratch with `bash tools/build_sn.sh` (or
   `python tools/build_sn.py` on Windows).
5. Regenerate the progress report with
   `python tools/gen_progress_report.py`, and commit it together with your
   change. CI fails if the report is out of date.

On macOS and Linux, prefix each of these with `bash tools/docker/run.sh`.
m2c and asm-differ are used from local clones (not vendored):

```
git clone https://github.com/matt-kempster/m2c tools/ext/m2c
git clone https://github.com/simonlindholm/asm-differ tools/ext/asm-differ
```

## Build fidelity

The build uses retail's own compilers and assembles what they emit as it is,
apart from a short, documented list of assembler and linker behaviours.
[docs/BUILD_FIDELITY.md](docs/BUILD_FIDELITY.md) has the list, the evidence
for each entry and how many matches depend on it. So:

- never add a step that changes the compiler's output to make a function
  match; a function that only matches that way is not matched;
- options apply to whole source files (`config/file_cflags.txt`), never to
  one function: GCC 2.95 takes them per translation unit;
- a new assembler or linker step needs evidence from the real tool or from
  retail first. `tools/check_build_fidelity.py` runs in
  `tools/gen_progress_report.py --check`, so CI enforces the list.

## Sources

C here is reconstructed from the retail executable's own code. Allowed
as references:

- the retail binary (our assembly, Ghidra, m2c), and SDK library
  *binaries* the toolchain ships (`.a` archives: their code and symbol
  names, used the same way as the retail binary);
- open-source code under its own license: newlib, libgcc/GCC, and
  anything else whose license permits it, credited in
  `THIRD_PARTY_NOTICES.md`;
- other public decompilations and ports of this game (Lombyte, the NTSC decomp,
  ReRAC: docs/SIBLING_DECOMPS.md), credited, and public hardware
  documentation.

**Never** use Sony's SDK source, sample code or headers, or any leaked or
NDA material, even when copies circulate inside leaked source releases
of other games. Not as code, not as types or macros, not as a reference
to check a match against. On 2026-09-30 the movie code (`movie/*`, Sony's
ezmpeg sample in retail) and one libmpeg helper were reverted to
assembly for exactly this: they had been compiled from that sample's
source. They are to be redone from the assembly alone.
The rules for doing that are in [`docs/MOVIE.md`](docs/MOVIE.md).

Known compiler behaviour, useful levers and measured dead ends are collected
in [`docs/DECOMP_PROGRESS.md`](docs/DECOMP_PROGRESS.md).

For LLM agents, [`docs/CONTAINERS.md`](docs/CONTAINERS.md) describes a
Ghidra container that serves decompilation and disassembly over MCP, and
`tools/export_dataset.py` exports every exact match with its retail
assembly as training pairs.

If you have questions, run into build issues, or want to collaborate with other contributors, feel free to drop by our [Discord](https://discord.gg/Sfd2B54PDG)!