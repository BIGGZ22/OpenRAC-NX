# Credits

OpenRAC is the work of the people behind the four projects it brings
together. The original repositories keep the full history of who wrote what
([docs/SOURCES.md](docs/SOURCES.md)). Names below are as they appear in each
project's commits up to the sync of 2026-10-07, most commits first.

## Ratchet & Clank, PAL: rac1-decomp (`games/rac1`)

- Kryštof "Lynder063" Malinda: creator and maintainer (commits as Lynder063,
  lynderjekokot and Kryštof Malinda)
- Flavius (Veradictus)

## Ratchet & Clank, NTSC-U: Lombyte (`games/rac1/ntsc`)

- Mateusz Kłysz: creator and maintainer
- Louis-Philippe Le Sieur
- Pezza
- Kryštof "Lynder063" Malinda
- Purab
- mechaivan

## Ratchet & Clank: Going Commando: rac2-decomp (`games/rac2/ntsc`)

- Louis-Philippe Le Sieur (llesieur99): creator and maintainer
- platypet2217-star: author of RAC2Decomp, the functional PAL port in
  `ports/pal-functional/`
- Kryštof "Lynder063" Malinda

## Ratchet & Clank: Up Your Arsenal: rac3-uya-decomp (`games/rac3/ntsc`)

- vetusmagnus: creator and maintainer
- Louis-Philippe Le Sieur

## Ratchet: Deadlocked: rac-deadlocked-decomp (`games/rac4/ntsc`)

- Kryštof "Lynder063" Malinda: creator and maintainer

## The top level

The consolidation into OpenRAC (the editor's move, `tools/`, `docs/`):
Flavius (Veradictus), with the level editor written in rac1-decomp.

Several projects work with AI assistants under human direction; their
commits say so where a project records it. A few rac1-decomp commits carry
an unconfigured git identity or an assistant's identity and are not listed
by name.

## Projects we build on

Credited where their work is used, with licenses, in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and each game's own notices:
ReRAC, Wrench, Replanetizer, OpenGOAL's jak-project (the model for the
editor's own-disc extraction), splat, spimdisasm, objdiff, asm-differ,
decomp-permuter, m2c, wibo, Himuro, the GNU toolchain, newlib, and the PS2
reverse-engineering and decompilation communities. The NTSC decomp of
Ratchet & Clank is credited by that name.

The community hub for the Ratchet & Clank decompilations is
[openrac.dev](https://openrac.dev/).
