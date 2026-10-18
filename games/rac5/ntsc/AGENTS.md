# Instructions for contributors and AI agents

Read `README.md`, `LEGAL.md` and `docs/ROADMAP.md` before changing this project.
This repository adopts RAC2's **evidence standard and source layout**, not its
compiler settings or level-format assumptions.

- Use only a legally obtained disc, public documentation, licensed code, and
  your own reverse engineering. Never use leaked or NDA source, Sony SDK
  source, samples or headers.
- A compiler from an SDK may be used as a private build tool; that does not
  authorize publishing source from the SDK. For reused public code, identify
  the public upstream version and file, preserve its copyright, license and
  exception notices, and describe local adaptations. A path into a local SDK
  source tree is not public provenance. Verify any SDK-bundled GNU routine
  against a public GNU release before importing it.
- Never commit disc images, retail executables, extracted levels, disassembly,
  assets, proprietary tools, build output or secrets. This includes instruction
  listings inside C comments. Keep them under the ignored local directories.
- Keep one authored definition per function in `src/`, using C or C++ only
  after the corresponding compiler profile is measured. Put a useful but
  nonmatching attempt in `nonmatching/`; do not count it as a match.
- C/C++ evidence must come from compiler-generated instructions: no inline or
  global assembly, fixed-register bindings, raw instruction words, artificial
  assembly barriers, or edits to compiler output. This also applies to provisional
  witnesses and public `nonmatching/` attempts. Preserve hybrid experiments privately;
  leave their retail intervals in the assembly baseline without C credit. Real
  handwritten assembly requires separate evidence and maintainer classification,
  never automatic C acceptance. File-scope assembler-name labels on declarations
  are permitted solely to name retail ELF symbols; they must emit no instructions.
- Do not infer this game's compiler, flags, function boundaries or level layout
  from RAC2. Record the evidence for each before changing the build.
- A match requires the qualified toolchain to reproduce the complete retail
  program bytes at every applicable placement. A source file compiling, a
  similar disassembly, or a masked comparison is not sufficient.
- Regenerate catalogues and progress with their tools. `tools/rel.py` and
  `tools/boot_symbols.py` generate symbol metadata, not function or progress
  claims. Never hand-edit generated claims.
- Run the relevant tests and `python tools/check_repository.py` before a
  commit. Describe what was verified and what remains provisional.
- Work on a branch for contributions. Keep game data and private paths out of
  commits and pull requests. The owner merges manually.

## Seven working rules

1. **Bounds and ownership.** Each replacement range has one owner, no overlaps
   or duplicate names. A compiled prefix or next-label interval is not proof of
   a complete function. Unknown extents remain provisional and uncounted.
2. **Explicit promotion.** Count C only when the claimed body is actually linked
   at every reviewed placement and the complete-program gates pass. Assembly
   fallback preserves the baseline but earns no C credit. One proven zero word
   of linker alignment may follow a complete body, but count only compiled
   body bytes after boundary and semantic review; promote separately.
3. **Evidence-based profiles.** Compiler routes, options and patches require a
   measured cause, known provenance and regression controls. Do not select a
   compiler just because one sample matches. Requalify affected accepted bodies.
4. **Proven names and types.** Keep reference ELF symbols stable. Descriptive
   aliases, prototypes, field sizes, offsets and signedness need evidence from
   symbols, code, callers or data. Share definitions rather than inventing layouts.
5. **Bounded experiments.** Change one hypothesis at a time. After three attempts
   with the same mismatch class, recover context, change approach or park the
   best source with its first difference and a concrete reopening condition.
6. **Original behavior.** Preserve original behavior, including bugs, in matching
   work. Do not add purposeless C tricks to steer code generation. Bug fixes and
   PC platform adaptations belong in explicitly separate work and earn no PS2
   matching credit. This does not relax the strict no-instruction-assembly rule.
7. **Public hygiene.** Keep scratch directories, probes, generated lists and raw
   reports private. Index useful alternatives by function in `nonmatching/`.
   Reusable tools belong in `tools/` with tests; public text is English.

See [CONTRIBUTING.md](CONTRIBUTING.md#review-and-promotion) for the review steps.
Checks enforce structural invariants, not semantic truth: names/types, patch
justification, intent and original behavior still require evidence and review.
