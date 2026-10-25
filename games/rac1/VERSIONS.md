# Ratchet & Clank (2002)

`games/rac1/` is becoming one game for every version: one source tree, with
the differences between versions kept as data (each version's address map,
disc checksums) and, where code really differs, version switches in the
source. The tree here is the PAL decompilation (from rac1-decomp, still
synced as it progresses); NTSC-U is built from it by placing its functions at
NTSC-U's addresses (`tools/versions.py build rac1/pal rac1/ntsc`, which
reproduces all 20 NTSC-U programs; docs/engine/SHARED_CODE.md). The NTSC
decomp's own tree stays in [ntsc/](ntsc/README.md) as the source its progress
is ported from, until both projects reach 100%. The launcher picks the
version from your disc.

Both decompilations cover the boot executable and the 19 level programs
(code the game loads with each planet).

| Version | Directory | Came from | Builds on |
|---|---|---|---|
| PAL, `SCES_509.16` (disc v2.00) | [`games/rac1/`](README.md) | [rac1-decomp](https://github.com/OpenRAC/rac1-decomp) | macOS and Linux through Docker; Windows natively |
| NTSC-U, `SCUS_971.99` (disc v1.00) | [ntsc/](ntsc/README.md) | [Lombyte](https://github.com/lombyte-project/Lombyte) | Linux or WSL; macOS through `./setup.sh --docker` |

Disc checksums are in [baserom/README.md](../../baserom/README.md); progress
in [progress/](../../progress/README.md).

## How the two relate

- **Shared work.** The projects have ported functions to each other for a
  while: Lombyte credits code ported from the PAL project throughout its
  source and in its [THIRD_PARTY_NOTICES.md](ntsc/THIRD_PARTY_NOTICES.md), and
  the PAL project credits Lombyte in [its own](THIRD_PARTY_NOTICES.md).
- **A map between them.** [config/overlays/us_map.tsv](config/overlays/us_map.tsv)
  pairs 99.7% of the US code with its PAL counterpart, from the code alone
  ([docs/OVERLAYS.md](docs/OVERLAYS.md)); `tools/lombyte.py`
  uses it to find functions matched in one project and not the other; inside
  OpenRAC it reads `games/rac1/ntsc` and the US report kept in `progress/sources/`.
- **Different names.** PAL names functions by address (`func_0012D8F8`,
  `func_L05_002D48B8` in level code) with readable names in
  [include/names.h](include/names.h); the US project keeps one file per
  function under a subsystem path, with recovered names in
  [ntsc/config/us/recovered_names.json](ntsc/config/us/recovered_names.json).
- **Different compilers.** The PAL project matches the game code with SN GCC
  2.95.3 and Sony's SDK objects and libgcc with EE-GCC 2.9-ee-991111; the US
  project matches everything with a patched GNU EE-GCC 2.9. The PAL build
  runs no step that edits compiler output and gives flags only per file
  ([docs/BUILD_FIDELITY.md](docs/BUILD_FIDELITY.md)). Settling which
  compiler built the game comes before merging the two into one tree
  ([docs/decomp/TOOLCHAINS.md](../../docs/decomp/TOOLCHAINS.md),
  [open questions](../../docs/policy/OPEN_QUESTIONS.md#4-one-ratchet--clank-tree-for-both-regions)).
- **Different licenses.** The PAL project is GPL v3 since 2026-10-07 and the
  US project MIT, so US code can move into the PAL tree, and PAL code written
  since then cannot move back without its authors' permission
  ([LICENSE.md](../../LICENSE.md)).

## Getting started

```sh
python3 tools/openrac.py setup rac1/pal rac1/ntsc   # from the top of OpenRAC
```

Then follow [README.md](README.md) or [ntsc/README.md](ntsc/README.md).
On an Apple Silicon Mac, build `ntsc/` through [ntsc/host/](ntsc/host/README.md):
its container's own Wine cannot run the project's Windows tools there, and
that folder runs them under rac1/pal's Wine instead. Both of its gates pass
that way, the boot ELF and all 1,540 overlay functions.
The level editor ([editor/](../../editor/README.md)) reads the PAL disc.

## Where versions differ: variants

When a function's code differs in NTSC-U, its NTSC-U version is a variant:
`versions/ntsc/src/<path>` holds NTSC-U's copy of the PAL file `src/<path>`,
with only the differing functions changed. Only NTSC-U's build compiles it,
so the PAL tree stays exactly as rac1-decomp has it and its syncs keep
applying. The copy is the whole file, so it compiles as the same translation
unit PAL's does (`tools/versions.py` builds it with PAL's own `Makefile.sn`
in a shadow directory: same compiler, per-file flags and assembler steps),
and NTSC-U's link takes from it only the functions PAL's own objects could
not place.

For a function whose code differs only in immediates (struct offsets,
constants), `python3 tools/versions.py variant rac1/pal rac1/ntsc NAME ...`
writes the variant from the bytes NTSC-U has: each immediate that differs is
rewritten in the function's C, and a struct the function uses gets the
padding before a moved field resized. Rerun it when the PAL file changes;
`python3 tools/versions.py check rac1/pal rac1/ntsc` says whether each
variant reproduces NTSC-U.

The first variants: the record at `D_0013D390` is 8 bytes shorter in NTSC-U
from before `+0xC0` on (`menu.c`, `bmain.c` and `space.c` read fields past
that point), and `missionfunc.c` uses another id (`0x523E` for PAL's
`0x5243`). With them, 14 more functions reproduce NTSC-U, and all 20 NTSC-U
programs still link identical to the disc.

Level code works the same way: a variant of a file under `src/overlays/` is
built with the pipeline PAL's progress report builds level files with, and
placed in every level where PAL's version could not go. A shared level
function sits in several levels, and NTSC-U's levels do not always agree on
it, so a variant may place in some of its levels and not others.

The generator rewrites hex literals, word indices (`[0x3F]`, `[63]`,
`[0xFC/4]`), address literals whose low half moved, and the padding before a
moved field in the structs a function uses (one-line typedefs included).
Variants it cannot write are written by hand and say so in their header:
`pause.c` and its level copy `pause_00277208.c` hold a function that does the
same in both versions but whose NTSC-U code was compiled from C shaped
differently (the store falls through to the common `return 0`). The level
functions still differing either have no C of their own (pieces of joined
functions, handwritten assembly) or call a different routine in some levels
(the catalogue's false folds), which is a catalogue fix, not a variant.

## NTSC-U's own code where the merged tree has none

What the merged tree's C (PAL's functions and the variants) does not place
comes from NTSC-U's own disassembly, not its retail bytes: splat splits
`SCUS_971.99` with `versions/ntsc/splat.yaml` (PAL's config with NTSC-U's
offsets) and PAL's own fix-ups of splat's output, its globals get an `n`
in front (`nfunc_`, `nD_`, `njtbl_`, so they cannot collide with PAL's
address names), and each segment is assembled the way PAL builds its
unmatched functions, as `INCLUDE_ASM` lines in a generated C file. Each
function goes in at the address its name says. `tools/versions.py build`
does all of it; the disassembly goes to the ignored `versions/ntsc/asm/`.

For the boot executable, 2026-10-15: 922 functions from the merged tree's C
and 746 from NTSC-U's own assembly. 96% of `core.text` and 95% of `.text`
come from code that is built; the rest, linker fill between objects and 34
functions the check cannot place yet, stays retail, as does the data. All
20 programs still link identical to the disc. Level programs are linked
from the merged tree's C with NTSC-U's bytes between, as before.
