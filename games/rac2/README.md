# Ratchet & Clank: Going Commando (2003)

Released in Europe as *Ratchet & Clank 2: Locked and Loaded*.

| Version | Directory | Came from | Builds on |
|---|---|---|---|
| NTSC-U, `SCUS_972.68` (disc v1.01) | [ntsc/](ntsc/README.md) | [rac2-gc-decomp](https://github.com/OpenRAC/rac2-gc-decomp) | macOS, Linux and Windows through Docker; Linux with Wine natively |

Since 2026-10-11 the project has rac1/pal's structure:

- **Catalogues.** `tools/levels.py` reads the boot executable and the 27 level programs from
  the disc and lists every distinct function once, with all its places
  ([ntsc/docs/LEVELS.md](ntsc/docs/LEVELS.md)).
- **Source.** `src/` holds every function in address order, as C or as an `INCLUDE_ASM`
  marker; a function shared by many levels is defined once.
- **Build.** `tools/build.py` links the C in place of splat's assembly and compares every
  loaded byte of all 28 programs with retail ([ntsc/docs/BUILDING.md](ntsc/docs/BUILDING.md)).
  Both compilers rebuild from public source (`tools/compiler/`).
- **Progress.** `progress/report.json` is objdiff's format and counts each distinct function
  once.

Some of its C was carried over from rac1/pal, where the two games share byte-identical
functions (the SDK and library code of the boot).

[ntsc/ports/pal-functional/](ntsc/ports/pal-functional/README.md) is a separate project inside
it: platypet2217-star's RAC2Decomp (MIT), a functional, not matching, reconstruction of the PAL
release (`SCES_516.07`) as C with SDL2 and OpenGL. It builds with CMake on its own and is not
part of the matching sources or the progress.

Only v1.01 is built; the Greatest Hits release (v2.00) and the PAL release are identified in
[ntsc/docs/REGIONS.md](ntsc/docs/REGIONS.md). Disc checksums are in
[baserom/README.md](../../baserom/README.md); progress in [progress/](../../progress/README.md).

## Getting started

```sh
cd games/rac2/ntsc
python3 -m unittest discover -s tests       # tool tests, no game data needed
python3 tests/check_repository.py           # the catalogues and the layout
python3 tools/progress.py --check           # the report matches src/
```

To build, put the disc in `games/rac2/ntsc/baserom/SCUS_972.68.iso` and follow
[ntsc/docs/BUILDING.md](ntsc/docs/BUILDING.md); `bash tools/docker/run.sh <command>` runs any step
in the project's build image.
