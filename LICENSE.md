# Licensing

OpenRAC holds several projects, and each keeps its own license. A file is
covered by the license of the closest directory below that has one.

| Directory | License |
|---|---|
| [`games/rac1`](games/rac1/LICENSE) | GNU General Public License v3 (since 2026-10-07; MIT, Kryštof "Lynder063" Malinda, before). The libgcc sources in `src/libgcc/` keep the GPL with the runtime exception their headers state, `include/moby_pvars.h` is adapted from Wrench (GPL-3.0-or-later), and the functions carried over from Lombyte keep its MIT notice; see its [THIRD_PARTY_NOTICES.md](games/rac1/THIRD_PARTY_NOTICES.md) and [LEGAL.md](games/rac1/LEGAL.md). |
| [`games/rac1/ntsc`](games/rac1/ntsc/LICENSE) | MIT, Copyright (c) 2026 Mateusz Kłysz. Code reconstructed from libgcc and the EE-GCC patches is GPL-2.0 ([licenses/GPL-2.0.txt](games/rac1/ntsc/licenses/GPL-2.0.txt)); newlib code keeps newlib's licenses ([licenses/COPYING.NEWLIB.txt](games/rac1/ntsc/licenses/COPYING.NEWLIB.txt)); see its [THIRD_PARTY_NOTICES.md](games/rac1/ntsc/THIRD_PARTY_NOTICES.md). |
| [`games/rac2/ntsc`](games/rac2/ntsc/LICENSE) | MIT, Copyright (c) 2026 llesieur99. `ports/pal-functional/` is platypet2217-star's RAC2Decomp, MIT under its own [LICENSE](games/rac2/ntsc/ports/pal-functional/LICENSE). |
| [`games/rac3/ntsc`](games/rac3/ntsc/LICENSE) | GNU General Public License v3 (since 2026-10-04). |
| [`games/rac4/ntsc`](games/rac4/ntsc/LICENSE) | MIT, Copyright (c) 2026 Kryštof "Lynder063" Malinda. `src/libgcc/` keeps the GPL with the runtime exception, and `src/libm/` newlib's fdlibm notice ([THIRD_PARTY_NOTICES.md](games/rac4/ntsc/THIRD_PARTY_NOTICES.md)). |
| [`sdk`](sdk/LICENSE) | MIT, Copyright (c) 2026 the OpenRAC contributors (chosen 2026-10-21), so that mods built with the SDK can be under any license; the port they run in stays GPL. |
| [`editor`](editor/LICENSE) | MIT ([editor/LICENSE](editor/LICENSE)): it was written in rac1-decomp and moved here while rac1-decomp was MIT. |
| Everything else (`launcher/`, `tools/`, `docs/` and the top-level files) | GNU General Public License v3 or later ([COPYING](COPYING)), Copyright (c) 2026 the OpenRAC contributors (chosen 2026-10-08). Code adapted from another project keeps that project's notice, listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). |

Code from a GPL directory (rac1/pal, rac3, the top level) can only move
into another directory under the GPL, or with its authors' permission; MIT
code can move into a GPL directory. How code moves between games whose
licenses differ is still an
[open question](docs/policy/OPEN_QUESTIONS.md#1-licensing).

None of these licenses cover the games. OpenRAC holds no game code or assets:
you supply your own discs ([docs/policy/SOURCING.md](docs/policy/SOURCING.md)).
Third-party code and data used in OpenRAC are listed, with their licenses, in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and in each game's own notices.
