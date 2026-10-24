# Source and distribution scope

This repository contains authored tools, documentation and reconstructed C/C++,
including identified portions adapted from compatible public sources. It does
**not** distribute Ratchet & Clank: Secret Agent Clank,
disc images, executables, extracted assets, assembly dumps or proprietary
toolchains. Contributors supply their own lawful game copy for local
research.

The repository's authored material is licensed under GPL-3.0 (`LICENSE`).
Facts such as addresses, sizes, hashes and function boundaries may be recorded
without committing game bytes. Credit and preserve the licenses of any
third-party code before importing it.

## What stays private, always

- the game image, the retail executables and the extracted modules;
- the retail bytes themselves, instruction listings and disassemblies;
- proprietary toolchains, SDK sources, headers and samples;
- leaked or NDA source, in any form;
- personal paths, credentials and secrets.

`AGENTS.md` states the same rules for contributors and agents, and
`.gitignore` covers the usual local locations. The guard is structural: read
your own diff before pushing.

## Reuse from the sibling projects

The method, the tools and parts of the written source of
`OpenRAC/rac1-decomp`, `OpenRAC/rac2-gc-decomp` and
`OpenRAC/rac5-sm-decomp` may be reused here. They are GPL-3.0: keep the
copyright, the license and the exception notices, and describe the local
adaptation. A path into a local checkout of another project is not public
provenance — cite the upstream project, file and version.
