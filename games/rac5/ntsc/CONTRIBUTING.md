# Contributing

Start with the complete [README contribution walkthrough](README.md#start-contributing)
and [LEGAL.md](LEGAL.md). The pinned USA input and SNR2/boot metadata are
verified, with 31 narrowly accepted boot placements (1,064 bytes) and two
shared level functions. The level compiler
profiles, complete function catalogue and game-wide progress build remain
open. Run the synthetic tests first. For a game-derived
finding, verify your own lawful USA disc against `config/target.json` and
identify the exact program, address, method and uncertainty. Share a measured
hypothesis in an issue or a focused change to the tools or documentation.
Check open issues and PRs before choosing a scope; no reservation queue exists
yet. Do not add guessed function matches or claim progress from symbol names.

Once the applicable compiler, program format and function boundary are
qualified, write matching C or C++ in `src/`, with one definition per function.
A function is accepted only after a byte-identical complete-program gate for
every applicable placement. The boot-only scope and its remaining caveats are
documented in `docs/BOOT-MATCH-SCOPE.md`. Put unaccepted but useful bodies in
`nonmatching/` and explain their remaining differences.

The same source rule applies to accepted C and provisional witnesses: no inline or
global assembly, fixed registers, raw instruction words or patched compiler output
to force a match. Hybrid experiments stay private, and their original instructions
remain in the assembly baseline with no C credit. A declaration may use an
assembler-name label for a retail ELF symbol (including C++ names); this names a
symbol and does not emit instructions. The repository and boot compiler gates
enforce this distinction, including a check of preprocessed boot input.

Use short, focused commits. Before a PR, run
`python -m unittest discover -s tests -v` and
`python tools/check_repository.py`, then review the staged diff. Include your
input version, evidence, commands, results and remaining limitations in the
PR description. Never include game-derived bytes, disassembly listings,
memory dumps, personal disc paths, secrets or proprietary binaries. The
maintainer merges accepted PRs manually.

## Review and promotion

For boot candidates, run the [promotion audit](docs/BOOT-PROMOTION.md) after the
complete private gate. It explains each owner's mechanical blockers and creates
a shortlist for semantic review; it never accepts a function automatically.

Use the seven rules in `AGENTS.md` throughout a small, coherent change:

- Identify the program, reference symbol and owned byte range before editing.
  Account for padding, aliases, callees and any bytes following a compiled body.
  Keep uncertain boundaries provisional; neither a terminal call nor a matching
  prefix proves a complete retail function.
- Promotion is a separate reviewed step. Show that the accepted source supplies
  the claimed range, not the retained assembly baseline. Recompile with the pinned
  profile and run the complete-program gate at every applicable placement after
  integration. Record the tested commit and real output. If inputs change, repeat
  the affected gates; a failed run must not preserve a previous passing claim.
- Justify every compiler/profile change using a classified mismatch and source
  provenance. Keep source and object controls from the previous qualified profile;
  check that wrong tools, flags or inputs are rejected. Recompile all affected
  accepted bodies before changing qualification. A patch to compiler source is
  different from inserting dead C operations or editing emitted instructions.
- Preserve the reference linker symbol when giving C a descriptive alias. Recover
  prototypes from both callers and callees; verify layout, access width and sign
  across users before introducing a shared struct. Use target-compatible size and
  offset assertions where practical. Meaningful names and semantic correctness
  are review obligations, not facts inferred from a passing linter.
- Compare one hypothesis at a time and keep the best source. After three unchanged
  mismatch classifications, revisit types/ABI/context, choose another approach,
  or park the attempt. Record the first difference, profile, attempted shapes and
  a reopening trigger in its existing near-miss entry. Keep raw logs privately;
  no committed campaign queue or experiment ledger is needed.
- Cleanup must preserve qualified output and original behavior. Recheck affected
  callers after type/header changes. Do not fix retail bugs, add artificial dead
  operations, or introduce PC behavior into a matching change. Platform ports and
  deliberate behavior fixes need separate scope and validation.
- Keep the public change focused: named source, useful explanations, maintained
  tools and their tests. `scratch/`, `_scratch*/`, private paths, generated screening
  output and toolchain experiments stay out of Git. Preserve useful variants under
  `nonmatching/` with provenance, without turning them into matching credit.

The repository, compiler and program gates cover machine-checkable invariants.
The PR review must cover the remaining boundary, naming, ABI and behavioral evidence;
CI without game inputs is not a full-image validation.
