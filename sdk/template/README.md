# My mod

A starting point for a mod of OpenRAC's native port. Copy this folder, then:

1. In `openrac-mod.json`, set the `id` (lower case and hyphens), the name,
   your name, a description, the games and discs it is for, its `settings`,
   and in `hooks` the hook points it takes.
2. In `src/mod.c`, the same id and version in `OPENRAC_MOD_DEFINE`, and your
   code in `openrac_mod_load`.
3. In `CMakeLists.txt`, the target's name (the library is named after it).

Build and package it as [the SDK's README](../README.md) says. The interface
is `openrac/openrac_mod.h`; the game's hook points are in
`openrac/games/rac1.h`.
