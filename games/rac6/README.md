# Ratchet & Clank: Secret Agent Clank (2009)

The PlayStation 2 port of the 2008 PSP game.

| Version | Directory | Came from | Builds on |
|---|---|---|---|
| NTSC-U, `SCUS_976.23` (disc v1.00) | [ntsc/](ntsc/README.md) | [rac-sac-decomp](https://github.com/OpenRAC/rac-sac-decomp) | Python 3.12 for the tools; no qualified compiler yet |

The project is early. The boot executable is SCUS_976.23; the 31 level modules
are relocatable modules in the SNR2 `.REL` format.

Disc checksums are in [game.json](game.json) and the
[table in baserom/README.md](../../baserom/README.md#disc-images).

## What "matched" means here

There is no qualified compiler for the whole game yet, so the project counts
no game-wide percentage. Functions are accepted by wave at placement level.
OpenRAC's own tables leave this game's percentage out until the project has one.

## Working on it

Read [ntsc/README.md](ntsc/README.md), [ntsc/CONTRIBUTING.md](ntsc/CONTRIBUTING.md)
and [ntsc/LEGAL.md](ntsc/LEGAL.md). Its checks run without the game:

```sh
cd games/rac6/ntsc
python3 -m unittest discover -s tests -v
python3 tools/check_repository.py
```
