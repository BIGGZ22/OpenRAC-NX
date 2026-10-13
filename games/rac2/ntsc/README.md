<p align="center">
  <img src="assets/rac2-logo.png" alt="RAC2 — Going Commando" width="560">
</p>
<p align="center"><strong>Ratchet &amp; Clank: Going Commando</strong></p>

<p align="center">
  <a href="progress/report.json"><img src="https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2Fllesieur99%2Frac2-decomp%2Fmaster%2Fprogress%2Freport.json&query=%24.percent&suffix=%25&label=C&style=flat-square&color=e89b35&labelColor=0d1117" alt="Progress"></a>
  <a href="docs/BUILDING.md"><img src="https://img.shields.io/badge/PS2-USA_v1.01-c3cbd8?style=flat-square&amp;labelColor=0d1117" alt="PlayStation 2 USA version 1.01"></a>
</p>

A matching decompilation of Ratchet & Clank: Going Commando for the PlayStation 2 (USA v1.01,
`SCUS_972.68`): C that compiles to the game's own bytes, on the way to a native PC port.

No game files, disc images, rebuilt executables or proprietary tools are distributed: bring
your own copy of the game.

## Progress

Every distinct function counts once: the boot executable's 3,390 and the levels' 5,711 that
the boot does not also have. A function many levels share is one function, however many
places it has. A function is C when its C links into a program that comes out equal to the
retail one ([`progress/report.json`](progress/report.json), `tools/progress.py`).

<!-- progress -->
| | Functions | Code |
| --- | ---: | ---: |
| Boot executable | 1,275 of 3,390 | 143,624 of 1,049,464 bytes |
| Levels | 1,075 of 5,711 | 96,780 of 4,488,592 bytes |
| **Total** | 2,350 of 9,101 | 240,404 of 5,538,056 bytes (4.34%) |

The 27 levels hold 106,337 function placements; C covers 31,004 of them.
84 of the matched functions are the original's own assembly (`ASM_FUNC`), kept as assembly.
<!-- /progress -->


## How it works

- `tools/levels.py dump` reads the boot executable and the 27 level programs from the disc.
- `config/boot/functions.tsv` and `config/levels/functions.tsv` list every function once,
  with its places ([docs/LEVELS.md](docs/LEVELS.md)).
- `src/` lists every function once, in address order, as its C or as an `INCLUDE_ASM` marker,
  as rac1-decomp does.
- `tools/build.py all` disassembles each program, links the C in place of its assembly, and
  compares every loaded byte with the retail program ([docs/BUILDING.md](docs/BUILDING.md)).

## Layout

| Path | What |
| --- | --- |
| `src/core/` | the boot's core code (SDK, libc), one file per original object |
| `src/game/` | the boot's own program |
| `src/levels/shared/`, `src/levels/NN_name/` | level functions in several levels, and in one |
| `nonmatching/` | near misses, never linked |
| `config/` | the function catalogues, per-file compiler flags, naming research |
| `tools/` | disc reader and catalogue, build, progress, the compiler recipes |
| `include/` | shared headers |
| `ports/pal-functional/` | functional C for the PAL release (not matching) |
| `docs/` | building, the level format, the compiler, game research |

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Related projects: the
[rac1-decomp](https://github.com/OpenRAC/rac1-decomp) and
[UYA](https://github.com/OpenRAC/rac3-uya-decomp) decompilations, and
[OpenRAC](https://github.com/OpenRAC/OpenRAC). Outside work used here is credited in
[CREDITS.md](CREDITS.md).

## Licence

GPL-3.0, see [LICENSE](LICENSE), like rac1-decomp and OpenRAC. Code released earlier under MIT
keeps its notice in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). The licence covers this
repository's own code, never the game, its assets or proprietary toolchains.
