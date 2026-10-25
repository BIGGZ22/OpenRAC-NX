# Ratchet: Deadlocked (2005)

Released in Europe as *Ratchet: Gladiator*.

| Version | Directory | Came from | Builds on |
|---|---|---|---|
| NTSC-U, `SCUS_974.65` (disc v1.00) | [ntsc/](ntsc/README.md) | [rac-deadlocked-decomp](https://github.com/OpenRAC/rac-deadlocked-decomp) | macOS and Linux through Docker (rac1/pal's image) |

The project began on 2026-10-04 and is early. The boot executable is a small
loader around one compressed game image; the project unpacks that image into
its 17 sections, disassembles the core, network and level code (8,027
functions), and does the same for the 47 level overlays on the disc (24
campaign, 23 multiplayer). The runtime libraries are rebuilt from their open
sources: libgcc from GCC, libm from newlib
([ntsc/docs/RESEARCH.md](ntsc/docs/RESEARCH.md),
[ntsc/docs/OVERLAYS.md](ntsc/docs/OVERLAYS.md)).

It is built like rac1/pal, by the same author, and compiles like rac3: SN GCC
2.95.3 v1.36 with `-O2 -G8 -fopt-stack -mno-check-zero-division`
([toolchains/README.md](../../toolchains/README.md)).

Disc checksums are in [baserom/README.md](../../baserom/README.md); progress
in [progress/](../../progress/README.md).

## What "matched" means here

There is no linked image yet, so each compiled function is compared with the
retail bytes at its address on its own, with relocatable fields masked
(`ntsc/tools/retail.py`). The mask covers jump targets, every `lui` immediate,
and the 16-bit immediate of loads, stores and immediate arithmetic unless the
base register is `$sp`. A function that reads the wrong symbol, uses a wrong
struct offset or a wrong small constant can therefore still count. The other
games prove more: a byte-identical build, or a strict per-function check at
link addresses. Read this game's percentage with that in mind until it has a
link-time comparison (the first of its own
[open questions](ntsc/docs/RESEARCH.md#open-questions)).

## What it shares with the other games

The function map across the games ([docs/engine/SHARED_CODE.md](../../docs/engine/SHARED_CODE.md))
finds 567 KB of its code identical to RAC3's, and 282 functions (29,872
bytes) that another project has already matched: 161 from RAC3, about 130
from each RAC1 project. The lists are in
[shared/xmap/ports/](../../shared/xmap/ports).

## Getting started

From the top of OpenRAC, with the two toolchain mirrors in `toolchains/`
([toolchains/README.md](../../toolchains/README.md)):

```sh
python3 tools/openrac.py setup rac4/ntsc      # the boot executable from your disc, and the toolchain link
cd games/rac4/ntsc
bash tools/setup_asm.sh                       # unpack the image, split it, disassemble into asm/
bash tools/docker/run.sh bash tools/build.sh  # compile every src/**/*.c
bash tools/docker/run.sh bash tools/build_libgcc.sh
bash tools/get_newlib.sh && bash tools/docker/run.sh bash tools/build_libm.sh
venv/bin/python tools/audit_matches.py        # compare with retail
python3 tools/gen_progress_report.py --check
```

The level overlays come from a Wrench unpack of your disc
(`wrenchbuild unpack baserom/SCUS_974.65.iso -o DIR -g dl -r us`, about 5 GB;
on a Mac, run Wrench's Linux build in an amd64 container as
[rac3's page](../rac3/README.md#getting-started) shows). Pass the folder as
`OVERLAYS=DIR` to `setup_asm.sh` and `audit_matches.py`
([ntsc/docs/OVERLAYS.md](ntsc/docs/OVERLAYS.md)).

Then follow [ntsc/CONTRIBUTING.md](ntsc/CONTRIBUTING.md). Its naming rule is
strict, and stricter than any other game's: every name comes from your own
work on the retail binary, `func_<address>` and generated `TypeN` and `fNN`
until the code is understood, and nothing is copied from another source
([ntsc/LEGAL.md](ntsc/LEGAL.md)). For this game that rule matters most:
nothing taken from leaked material is used, whatever circulates about it
([sourcing policy](../../docs/policy/SOURCING.md)).
