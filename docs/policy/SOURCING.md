# Sourcing policy

This policy applies to everything in OpenRAC: every game, the editor, the tools
and the documentation. A game's own rules may be stricter; none may be looser.
It merges the rules the four sister projects already followed (for example
[rac1/pal's CONTRIBUTING.md](../../games/rac1/CONTRIBUTING.md) and
[LEGAL.md](../../games/rac1/LEGAL.md)).

## What goes into the repository

- Code you wrote: decompiled C reconstructed from a retail program's own
  machine code, tools, build scripts, tests.
- Facts about the retail programs: addresses, sizes, symbol names, section
  tables, checksums, descriptions of data formats. All four projects commit
  these (symbol maps, catalogues, progress reports); they hold no game bytes.
- Documentation of what was found and how.

## What never goes into the repository

- Disc images, executables, level programs, extracted assets, or anything
  carrying retail bytes: generated assembly (`asm/`), dumps, decompressed
  data, the Godot project the editor writes. `baserom/`, `assets/`, `build/`
  and each game's generated directories are ignored by git for this reason.
- Compilers, assemblers, linkers and SDK binaries. They are proprietary;
  `toolchains/` is ignored. Each game's docs say where its builders get them.
- Sony SDK source code, sample code or headers.
- Leaked or NDA material of any kind: leaked game source, leaked SDKs,
  internal documents, symbol dumps taken from leaked builds.

## What you may use as a reference

- The retail programs of a disc you own (assembly, Ghidra, m2c, emulators).
- SDK library *binaries* (`.a` archives: their code and symbol names), used
  the same way as the retail program.
- Open-source code under its own license, used as that license allows and
  credited: newlib, libgcc and GCC, and others.
- Public decompilations, ports and tools for these games (the sister
  projects, ReRAC, Wrench, Replanetizer and others), credited where used.
- Public hardware documentation.

**Never** use Sony's SDK source, samples or headers, or leaked or NDA
material, even when copies circulate in leaked releases of other games: not
as code, not as types or macros, and not as a reference to check a match
against. In rac1/pal, the movie code (Sony's ezmpeg sample as built into the
game) was reverted to assembly on 2026-09-30 for exactly this reason; it is
to be redone from the assembly alone.

When a function looks like SDK sample code, decode it from the assembly.
Reviewers look twice at a match that arrives suspiciously complete, with
SDK-style names or macros.

## Prerelease builds

Several projects cite publicly distributed demo, preview and prototype discs
as references (for example [rac1/pal's notes/beta_builds_research.md](../../games/rac1/notes/beta_builds_research.md),
[rac1/ntsc's docs/engine-source-layout.md](../../games/rac1/ntsc/docs/engine-source-layout.md)
and [rac2's docs/PROTOTYPE-BUILDS.md](../../games/rac2/ntsc/docs/PROTOTYPE-BUILDS.md)).
Until the group decides otherwise ([open questions](OPEN_QUESTIONS.md)):

- such a build may be cited as a reference, never as the evidence for a match;
- say which build a name or layout came from;
- nothing that came from leaked material, whatever the build.

## Removal requests

People have asked for every mention of them to be removed from these
projects, and OpenRAC honours those requests:

- Refer to the third-party NTSC decompilation of Ratchet & Clank only as
  "the NTSC decomp". Do not add its author's name, handle or address.
- Do not re-add material that the projects removed at someone's request. When
  you import or port from a sister project, check for it before committing,
  never after: git history keeps whatever is committed.
- Unsure whether something falls under a request? Ask the maintainers before
  adding it.

## Credit

Every reuse is written down: the project, the file or function, and its
license. A game's reuse goes in that game's `THIRD_PARTY_NOTICES.md` and in a
comment at the code; reuse in shared code goes in the top-level
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md). [CREDITS.md](../../CREDITS.md)
lists the people behind each imported project.
