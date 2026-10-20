/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank (PAL, SCES-50916): the names and addresses of the shared host C
 * (port/game/rac1/host, rac1.h) in this version. The shared C is written with games/rac1's names
 * and addresses, which are this version's own: nothing is renamed.
 */
#ifndef OPENRAC_RAC1_VERSION_H
#define OPENRAC_RAC1_VERSION_H

#define RAC1_ADDR(a) a
#define RAC1_LADDR(n, a) a

/* gHeroPos (D_0013F4D0), for crash reports (boot.c); a version without it reports no position. */
#define RAC1_HERO_POS 0x0013F4D0u

/* What this version leaves out of the shared C (rac1.h, RAC1_OMIT), and why. */
/* The pause menu mobys' update at 0x0023B578: the decompilation has C for it (func_0023B578) since the
 * remnant in front of it was split off. */
#define RAC1_OMIT_game_D_0023B578 1

/* Where this version's decompilation declares a function otherwise: the type in its prototypes. */
/* ParseBin's result, the loaded level's entry. */
#define RAC1_TYPE_func_0012DA38_result gaddr
/* DrawTexturedQuad's colour and TEX0. */
#define RAC1_TYPE_func_001F5800_u64 unsigned long long
/* The text lookup's result, a string's address. */
#define RAC1_TYPE_func_001FE540_result gaddr
/* The sprite's TEX0 (func_00200CA0). */
#define RAC1_TYPE_func_00200CA0_tex0 long long

#endif /* OPENRAC_RAC1_VERSION_H */
