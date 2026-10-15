# Legal scope of this project

This repository is a **matching decompilation** of *Ratchet & Clank* (2002,
PS2, SCUS-97113 / SCES-50916 / SCPS-15006 depending on region) — the
approach used by projects like sm64/papermario/mm-decomp/Klonoa-decomp:
produce C source that, when compiled, reassembles byte-for-byte (or as
close as practically achievable) to the original retail executable.

This is a **different legal posture** than a clean-room reimplementation.
Be deliberate about it:

## What this repo does and does not contain

- **Never** the original disc image, `SLUS`/`SCUS`/`SLES` executable, ELF,
  or any file extracted directly from a copyrighted disc/PSN copy.
- **Never** any asset binary (textures, models, audio, level data) extracted
  from the original game, and never the ROM/ISO/executable itself.
- Build **scripts and config** (`splat.yaml`, symbol maps, linker scripts,
  Makefiles) that describe *how* to reconstruct the original layout from a
  baserom the user supplies themselves.
- **Not** `asm/`. The MIPS disassembly `splat` produces carries the
  retail executable's instruction bytes, so it is generated locally by
  each contributor from their own baserom (`bash tools/setup_asm.sh`) and
  is gitignored. That keeps this file consistent with rule 1 below.
  (It was committed earlier in the project and removed from the tree on
  2026-09-16. The history has since been rewritten: no commit in this
  repository contains `asm/` or any `.s` file.) The setup
  is pinned (`requirements.txt`, `config/splat.yaml`, the baserom sha1),
  and a regenerated `asm/` rebuilds the image byte-identically (checked
  on 2026-09-23 on a second machine, see `docs/TOOLCHAIN.md`).
- Progressively, hand-written C source in `src/` that a contributor has
  decompiled from the (locally generated) disassembly in `asm/` and cleaned up to compile back
  to matching (or `INCLUDE_ASM`-stubbed, not-yet-matching) object code.
- Non-matching helper tooling under `tools/`.

## What a contributor/user must supply themselves

- Their own legally owned copy of the game (disc or legally obtained PSN
  ISO), dumped by them.
- The retail executable itself, `SCES_509.16` (PAL v2.00, SHA-1
  `79956931bd62fafd8d20fa2eae796dbaf2e15e83`), copied from their own disc
  into `baserom/`, which is outside version control (see `.gitignore`).
  It is used locally to run `splat` and to check the build against
  retail, and is never committed or redistributed through this repo.

## Why this is a bigger legal step than an engine-port project

Recreating the original binary's *exact* code shape (not just its
behavior, rewritten independently) sits closer to the disputed edge of
fair use / interoperability doctrine than either format-reverse-engineering
or "observe behavior, reimplement independently" does. The established
decomp-scene norm — and the one this repo follows — is:

1. Never distribute the original ROM/ISO/executable or anything extracted
   directly from it (textures, models, audio, raw disassembly).
2. Only distribute the *tooling and source* needed to rebuild a matching
   executable from a copy the end user already owns and supplies locally.
3. Treat the resulting repo as source-available for research/preservation/
   modding purposes, not as a way to distribute the game or its assets.

This mirrors the norm under which sm64/oot/papermario/many other
matching-decomp projects have operated for years without takedown, but it
is not risk-free, and it is a deliberate choice specific to this project —
not one to extend further (e.g. to hosting extracted assets, or a prebuilt
ROM) without thinking it through again.

## Sources used for the code

The C is reconstructed from the retail executable, with open-source
libraries (newlib, libgcc) and public projects about this game as
references. Sony's SDK source, samples and headers, and leaked or NDA
material of any kind, are not used (CONTRIBUTING.md, "Sources"). Code
found to have been derived from such material is reverted to assembly:
on 2026-09-30, the 81 functions of `movie/` (Sony's ezmpeg MPEG
streaming sample, as built into the game) and libmpeg's
`_initRefImages`, together with `include/ezmpeg.h`.

## Practical implication

This repo will not build into a runnable game for anyone who doesn't
already own a copy and dump it themselves. Cloning it gets you build
scripts and (as decompilation progresses) increasingly complete C source —
never assets, never the executable.
