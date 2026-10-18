# Knowledge base

Developer guide to the game code of Ratchet & Clank (PS2, PAL, SCES-50916), for contributors
and for the native PC port that imports this C.

## The one rule

**Byte-exact matching decompilation**: every C function compiles with the original SN Systems
GCC 2.95 toolchain to the retail bytes ([BUILD_FIDELITY.md](../BUILD_FIDELITY.md)), so it
behaves 1:1 with the PS2 game, bugs and quirks included (speedrun routes depend on them).
Edits to matched code are type, name and comment changes that keep identical bytes
([verify](conventions.md#6-verifying-a-change)). When a comment and the code disagree, the
code is what retail does. A port changes mechanisms (endianness, pointer width, hardware
access), not observable results, outside the matched C where possible.

## Pages

| Page | What it covers |
|---|---|
| [architecture.md](architecture.md) | Segments, source layout, the 19 level overlays, dispatch tables (which function updates moby class N), boot, moby loop, profiler stages, progress, "where do I look for X" |
| [conventions.md](conventions.md) | Symbol names, `names.tsv`/`names.h`, aliases and `MACRO_ADDR`/`NOT_SDA`/`SDATA`, shared headers, private views, provenance, verification commands |
| [data-structures.md](data-structures.md) | Field tables for `Moby`, `MobyClass`, `PadState`, hero records; vectors; pad bits and input; globals |
| [mobys.md](mobys.md) | Moby lifecycle, save ids, pvars, animation fields, class-to-update tables, class ids named by debug strings |
| [datamining.md](datamining.md) | Retail leftovers: debug menu, profiler, dev paths, NPC names, designer errors, button-code decoder, memory budget, SDK versions |

Older, more detailed documents:
[OVERLAYS.md](../OVERLAYS.md), [ASSETS.md](../ASSETS.md), [NAMES.md](../NAMES.md),
[TOOLCHAIN.md](../TOOLCHAIN.md), [BUILD_FIDELITY.md](../BUILD_FIDELITY.md),
[LEVERS.md](../LEVERS.md), [PVARS.md](../PVARS.md),
[DATAMINED_REVERSE_ENGINEERING.md](../DATAMINED_REVERSE_ENGINEERING.md).

## Known inconsistencies

Not fixed yet:

- `docs/PVARS.md` and `MobyInstance` in `include/structs.h` put the moby's update pointer,
  pvars and oClass at 0xA8 / 0xAC / 0xBC. The matched code and `include/moby.h` use
  0x74 / 0x78 / 0xA6. Use `moby.h`. PVARS.md's level numbers are also off (Pokitaru is 11;
  see [OVERLAYS.md](../OVERLAYS.md#levels)).
- Some header comments carry US addresses: `include/hero.h` names the hero `D_0013F350`
  (PAL: `D_0013F450`), and `include/pad.h` names the mirror flag `D_0015EDB4`
  (PAL: `D_0015EEB4`).
- Fixed: `func_001E9808` was named `showDebugFont` from the NTSC decomp, but its body starts a
  level; it is now `StartLevel` via `config/names_local.tsv`.
- `docs/ASM_CLASSIFICATION.md` counts 212 handwritten functions; `config/handwritten_asm.txt`
  lists 245 (the 33 VU0 helpers added on 2026-10-17 are not counted).
- `docs/DATAMINED_REVERSE_ENGINEERING.md` calls the camera unit `camera.c` (no such string
  exists; the header of `src/game/camera.c` says the original was `camera.cpp`), its
  `RaC1GadgetId` enum does not match retail item ids, and its `MobyInstance` layout cannot come
  from debug symbols: this ELF has no debug sections. See [datamining.md](datamining.md) §12.
- String addresses in `config/strings.tsv` for the literal/data ranges are ELF-image
  addresses; each level lays those records out differently, so the xref columns there are
  unreliable.

## Sourcing

Everything comes from this repository (matched C, headers, `config/`, generated asm,
`baserom/`); verified vs inferred is marked where it matters. The third-party US decompilation is "the NTSC
decomp". Provenance notes ("Adapted from ... (MIT)") are licence credit and stay.
