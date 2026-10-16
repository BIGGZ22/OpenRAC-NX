# Ratchet & Clank: the port's host C, one copy for every disc

The port's own host C for Ratchet & Clank, compiled into each version's
program (`openrac-rac1-pal`, `openrac-rac1-ntsc`): the overlay hook and the
direct start, the native 2D and world hand-off, the collision kernels, the
hardware routines, the variants of folded functions. A fix written here
holds for every disc, so the player's disc decides nothing but its own
addresses.

```
port/game/rac1/host/*.c          the shared C, with games/rac1's names
port/game/rac1-pal/host/         rac1_version.h: PAL's names and addresses (its own: nothing renamed)
port/game/rac1-ntsc/host/        rac1_version.h: NTSC-U's names and addresses; C only NTSC-U needs
```

A version takes the shared C with `"host_shared": ["../rac1/host"]` in its
`hostgen.json` ([cmake/Games.cmake](../../cmake/Games.cmake)). Each file
includes [rac1.h](host/rac1.h), which reads the version's translation
(`game_protos.h`) and then its table.

## Writing shared C

- **Functions by games/rac1's names**: `func_001F8B6C`, `func_L00_002A8A20`,
  and `func_0022C9A8__game` for the decompilation's C under a `wrap`. A
  version whose decompilation names them otherwise renames them in its
  table (`#define func_001F8B6C game_FUN_001f89a4`).
- **Addresses in `RAC1_ADDR()`**, written `0x%08Xu`, one literal each:
  `OPENRAC_DATA(RAC1_ADDR(0x0018A3D0u) + i * 4)`. A level program's address
  is `RAC1_LADDR(NN, ...)`: `OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0015F6A8u))`.
  A version without an entry for an address does not compile.
- **What a version leaves out**: `#if !RAC1_OMIT(func_...)` around a
  function, and `RAC1_OMIT_func_... 1` in that version's table with the
  reason (its decompilation has C of its own for it, its catalogue places
  something else there, its game never calls it).
- **Where the decompilations declare a function otherwise**: the type is
  `RAC1_TYPE_<name>_<what>` in each table.
- **Names in strings**: `RAC1_NAME(func_...)` is the version's name.

## What stays per version

A version's own folder keeps what truly differs: its `hostgen.json`,
`libraries.tsv`, `split_places.tsv`, `hand/` (C for functions its
decompilation has only as assembly), `drafts.tsv`, its table, and host C
for a routine only it has. That C is written in the version's own names and
includes `game_protos.h`, not `rac1.h`.
