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

/* Where this version's decompilation declares a function otherwise: the type in its prototypes. */
/* ParseBin's result, the loaded level's entry. */
#define RAC1_TYPE_func_0012DA38_result gaddr
/* DrawTexturedQuad's colour and TEX0. */
#define RAC1_TYPE_func_001F5800_u64 unsigned long long
/* The text lookup's result, a string's address. */
#define RAC1_TYPE_func_001FE540_result gaddr

#endif /* OPENRAC_RAC1_VERSION_H */
