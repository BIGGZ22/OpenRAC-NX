/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank's host C, one copy for every version of the game (port/game/rac1/README.md).
 *
 * The files here are compiled into each version's program (hostgen.json, "host_shared") with that
 * version's translation (game_protos.h) and its table of names and addresses (rac1_version.h in
 * port/game/rac1-<version>/host). They are written with games/rac1's names for the game's functions
 * (func_001F8B6C, func_L00_002A8A20, func_0022C9A8__game) and its addresses inside RAC1_ADDR():
 * each version's table turns them into its own, so a fix written here holds for every disc.
 */
#ifndef OPENRAC_RAC1_H
#define OPENRAC_RAC1_H

#include "game_protos.h"
#include "openrac/game_host.h"

/* After the translation's prototypes: a version's table renames games/rac1's names to its own. */
#include "rac1_version.h"

#ifndef RAC1_ADDR
#error "rac1_version.h must define RAC1_ADDR"
#endif

/* Whether this version leaves out a function of the shared C: `#if !RAC1_OMIT(func_...)` around it,
 * and RAC1_OMIT_func_... 1 in the version's table with the reason (its decompilation has C of its
 * own for the function, or its catalogue places something else there, or its game never calls it). */
#define RAC1_OMIT(f) RAC1_OMIT_##f

/* A function's name in this version, as a string (for logs and switches that name functions). */
#define RAC1_NAME_(f) #f
#define RAC1_NAME(f) RAC1_NAME_(f)

/* hand.c: the game's printf conversions from host arguments into `out` (at most size - 1
 * characters, terminated); the count written. */
#include <stdarg.h>
#include <stddef.h>
int rac1_vformat(char *out, size_t size, const char *format, va_list args);

/* boot.c: a new game straight in `level` from the title, as --level N with OPENRAC_DIRECT. */
void rac1_start_level(int level);

/* options.c: once a frame, the game's options the port remembers between runs. */
void rac1_options_frame(void);

/* moby_pool.c: a WAD decompressed to `to` (hand.c's WadDecompress, after it wrote it): the gameplay
 * header's spare moby count raised while the level loader runs. */
void rac1_wad_decompressed(gaddr to);

/* front_end.c: what the window's menus need of the game (game_host.h). */
extern const openrac_front_end rac1_front_end;

#endif /* OPENRAC_RAC1_H */
