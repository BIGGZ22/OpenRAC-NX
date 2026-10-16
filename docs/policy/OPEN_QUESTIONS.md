# Open questions

Decisions the group still has to make. Each says where things stand today and
what is affected. Settle one in a pull request that updates this file and the
docs it names.

## 1. Licensing

| Directory | License today |
|---|---|
| `games/rac1` | GNU GPL v3 ([LICENSE](../../games/rac1/LICENSE)), chosen by the project on 2026-10-07 (MIT before); libgcc sources under the GPL with the runtime exception |
| `games/rac1/ntsc` | MIT, Mateusz Kłysz ([LICENSE](../../games/rac1/ntsc/LICENSE)); GPL-2.0 libgcc and EE-GCC patches, newlib's license ([licenses/](../../games/rac1/ntsc/licenses)) |
| `games/rac2/ntsc` | MIT, llesieur99 ([LICENSE](../../games/rac2/ntsc/LICENSE)) |
| `games/rac3/ntsc` | GNU GPL v3 ([LICENSE](../../games/rac3/ntsc/LICENSE)), chosen by the project on 2026-10-04 |
| `games/rac4/ntsc` | MIT, Kryštof "Lynder063" Malinda ([LICENSE](../../games/rac4/ntsc/LICENSE)); GPL libgcc and newlib libm sources |
| `editor/` | MIT ([editor/LICENSE](../../editor/LICENSE)), moved here from rac1-decomp while that was MIT |
| rest of the top level (`tools/`, `docs/`, the launcher, the native port and its renderer) | GNU GPL v3 or later ([COPYING](../../COPYING)), chosen on 2026-10-08 |

The top level's license is settled. Still to decide: how code moves between
games now that the projects differ. rac1/pal, rac3 and the top level are GPL
v3 and the others MIT, so their code cannot be copied into an MIT directory without its
authors' permission, while MIT code can go the other way. For
[tools/port.py](../../tools/port.py) that means Lombyte to rac1/pal is fine
and rac1/pal to Lombyte is not, for code written after rac1-decomp's move
to the GPL; Lombyte's [THIRD_PARTY_NOTICES.md](../../games/rac1/ntsc/THIRD_PARTY_NOTICES.md)
credits what it took from rac1-decomp while that was MIT. That bears on
shared code (question 6).

## 2. Credit lines for AI assistants in commits

The projects disagree:

- rac1/pal keeps `Co-Authored-By:` trailers for AI assistants.
- rac1/ntsc's optional commit hook ([scripts/commit-msg.py](../../games/rac1/ntsc/scripts/commit-msg.py))
  strips co-author and session trailers.
- rac3's pull-request rules ([docs/wiki/Pull-Requests.md](../../games/rac3/ntsc/docs/wiki/Pull-Requests.md))
  ask contributors to strip AI co-author lines.

OpenRAC's own commits so far keep the trailer. To decide: one rule for the
repository, written into [CONTRIBUTING.md](../../CONTRIBUTING.md).

## 3. Names that come from prerelease builds

Some committed names and layouts come from demo, preview or prototype discs
rather than retail ([SOURCING.md](SOURCING.md#prerelease-builds)):

- rac1/pal [include/structs.h](../../games/rac1/include/structs.h): the
  IOP stash structures are attributed to debug symbols in a June 2002
  prototype, and are used by `src/game/stash.c` and
  `src/overlays/shared/stash_00295010.c`. The `BSphere`, `vec4` and
  `MobyInstance` blocks are attributed to STABS debug information without
  naming the build; no C file uses them. The `MobyInstance` field list is
  the same as the one left out of rac2 ([SOURCES.md](../SOURCES.md)),
  whose copy said it came from a leaked build. It also disagrees with
  matched retail code: `InitMobyInstance` (`func_0020D440`,
  [src/game/mobyfunc.c](../../games/rac1/src/game/mobyfunc.c)) stores a
  float at +0x2C and a short at +0xA6, where the header has bytes, so it may
  describe another build of the engine.
- rac1/ntsc [docs/engine-source-layout.md](../../games/rac1/ntsc/docs/engine-source-layout.md):
  original source file names and structures from 2002 development and demo builds.
- rac2 [include/flags.h](../../games/rac2/ntsc/include/flags.h) and
  [include/gadgets.h](../../games/rac2/ntsc/include/gadgets.h) (prototype strings),
  and the class names in [docs/moby-dispatch.tsv](../../games/rac2/ntsc/docs/moby-dispatch.tsv)
  (a prototype's extracted tables). No C file includes rac2's headers.

- rac4 forbids any name not derived from the retail binary by the contributor
  ([LEGAL.md](../../games/rac4/ntsc/LEGAL.md)). One of its files carried four of
  the `MobyInstance` member names above; they are named by offset here
  ([SOURCES.md](../SOURCES.md)).

To decide: whether names from prerelease builds may stay in code, or should be
replaced by names derived from retail alone. rac4's rule is the strictest
answer already in the repository.

## 4. One Ratchet & Clank tree for both regions

rac1/pal and rac1/ntsc decompile the same game, and have long shared and
ported functions ([games/rac1/docs/SIBLING_DECOMPS.md](../../games/rac1/docs/SIBLING_DECOMPS.md),
[config/overlays/us_map.tsv](../../games/rac1/config/overlays/us_map.tsv)
maps 99.7% of the US code to PAL). Merging them into one tree that builds
both regions needs one answer first: which compiler built the game code. The
PAL project matches it with SN GCC 2.95.3; the US project with a patched GNU
EE-GCC 2.9 ([docs/toolchains](../toolchains/README.md)).

## 5. Continuous integration and decomp.dev

Each project had its own workflows; they are kept, inert, under
`games/*/*/.github/` (GitHub only runs workflows at the repository root).
To decide: which checks run for OpenRAC, how each game's report reaches
decomp.dev from one repository, and what happens to Lombyte's progress
branch and its private scoring repository.

## 6. Code shared between the games

The games share engine code and Sony's libraries, now measured
([docs/engine/SHARED_CODE.md](../engine/SHARED_CODE.md)): the core carries over
between games, 319 functions (38 KB) are in every version, and each project has
hundreds of functions open that another has matched in identical code. To
decide: whether identical functions live once in a shared tree that several
game builds compile, or stay copied per game with their provenance noted. Two
things bear on it: the projects' compilers differ, so a shared file must pass
each game's own check, and the GPL v3 of rac1/pal and rac3 limits where their code may go
(question 1).

Also to decide, before code is ported into Deadlocked by machine: how names
travel. [tools/port.py](../../tools/port.py) renames every symbol to the
target's own address-based name, but the C it carries keeps the source
project's local, type and member names. rac4's rule
([LEGAL.md](../../games/rac4/ntsc/LEGAL.md), "Naming") allows code from other
public decompilations and forbids identifiers copied from any other source, so
a port into rac4 needs either its author's word that names from rac1/pal (his
own project) may come along, or a pass that regenerates them (`Type1`, `f20`).
130 functions (15,928 bytes) rac1/pal has matched are identical in Deadlocked
and wait on this.


## 7. Build hosts

- rac1/pal builds on macOS and Linux through Docker, and natively on Windows.
- rac1/ntsc builds on Linux or WSL, and on macOS through `./setup.sh --docker`.
  On Apple Silicon that container is emulated linux/amd64, and its own Wine 9
  cannot start the project's 32-bit Windows tools there (status c0000018,
  under Rosetta and under QEMU). With the 32-bit Wine 8 of rac1/pal's image
  in its place, both of the project's gates pass on an Apple Silicon Mac with
  Rosetta on: the boot ELF byte for byte, and all 1,540 overlay functions
  (2026-10-04; [games/rac1/ntsc/host](../../games/rac1/ntsc/host/README.md)).
- rac4 builds like rac1/pal, in the same container image.
- rac2 builds on Windows with WSL only (`scripts/wsl_chain.py` has
  machine-specific defaults).
- rac3 builds on Windows, or on Linux and macOS with wibo; several tools
  default to `C:\tools\...` paths.

To decide: whether OpenRAC aims for one container image that builds every
game on any host.

## 8. Project names and branding

The imported directories keep their projects' names and artwork (Lombyte's
logo, the rac2 logo). To decide: whether they stay as they are inside OpenRAC.
