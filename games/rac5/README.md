# Ratchet & Clank: Size Matters (2008)

The PlayStation 2 port of the 2007 PSP game.

| Version | Directory | Came from | Builds on |
|---|---|---|---|
| NTSC-U, `SCUS_976.15` (disc v1.00) | [ntsc/](ntsc/README.md) | [rac5-sm-decomp](https://github.com/OpenRAC/rac5-sm-decomp) | Python 3.12 for the tools; no qualified compiler yet |

The project is early. The boot executable is an ELF with 1,803 named symbols;
the frontend and 20 levels are relocatable modules in the SNR2 `.REL` format
([ntsc/docs/REL-FORMAT.md](ntsc/docs/REL-FORMAT.md),
[ntsc/docs/BOOT-SYMBOLS.md](ntsc/docs/BOOT-SYMBOLS.md)).

Disc checksums are in [game.json](game.json) and the
[table in baserom/README.md](../../baserom/README.md#disc-images).

## What "matched" means here

There is no qualified compiler for the whole game yet, so the project counts
no game-wide percentage. A function counts only after a private whole-program
gate rebuilds it and the complete load image compares byte for byte
([ntsc/docs/BOOT-TRIAL-GATE.md](ntsc/docs/BOOT-TRIAL-GATE.md),
[ntsc/docs/BOOT-MATCH-SCOPE.md](ntsc/docs/BOOT-MATCH-SCOPE.md)). The
[scoped progress snapshot](ntsc/progress/README.md) lists what is verified;
its ByteMatch figure divides those bytes by a provisional, optimistic code
size and is an estimate, not a match rate. OpenRAC's own tables leave this
game's percentage out until the project has one.

## Working on it

Read [ntsc/README.md](ntsc/README.md), [ntsc/CONTRIBUTING.md](ntsc/CONTRIBUTING.md)
and [ntsc/LEGAL.md](ntsc/LEGAL.md). Its checks run without the game:

```sh
cd games/rac5/ntsc
python3 -m unittest discover -s tests -v
python3 tools/check_repository.py
```
