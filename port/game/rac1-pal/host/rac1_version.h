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

/* Where libmc (func_001236F0..func_00124528, the port's lib/mc.c) keeps the state of a command, for
 * the port's library to keep it there too (boot.c, openrac_game_mc_state): D_00132EA8 the command that
 * runs (0: none; sceMcSync func_00123F30 returns -1 then, else hands it out and clears it once the
 * IOP has answered), D_0015B0C0 the library's RPC receive buffer, whose first word is the command's
 * result that sceMcSync hands out. A version without them keeps lib/mc.c's own. */
#define RAC1_MC_COMMAND 0x00132EA8u
#define RAC1_MC_RESULT 0x0015B0C0u

/* What this version leaves out of the shared C (rac1.h, RAC1_OMIT), and why. */
/* The pause menu mobys' update at 0x0023B578: the decompilation has C for it (func_0023B578) since the
 * remnant in front of it was split off. */
#define RAC1_OMIT_game_D_0023B578 1
/* shrub_class_init: the decompilation's candidate (nonmatching/functional,
 * register_shrub_render_class) is the function, the GS texture registers of the
 * class's packets included. */
#define RAC1_OMIT_func_00204340 1
/* The executable's sky tile cull (sky_bsphere_check, SkyBsphereCheck's twin): its candidate is the function. */
#define RAC1_OMIT_func_0022D7E0 1

/* Where this version's decompilation declares a function otherwise: the type in its prototypes. */
/* ParseBin's result, the loaded level's entry. */
#define RAC1_TYPE_func_0012DA38_result gaddr
/* DrawTexturedQuad's colour and TEX0. */
#define RAC1_TYPE_func_001F5800_u64 unsigned long long
/* The text lookup's result, a string's address. */
#define RAC1_TYPE_func_001FE540_result gaddr
/* The sprite's TEX0 (func_00200CA0). */
#define RAC1_TYPE_func_00200CA0_tex0 long long
/* UpdateWorldObjectAnimation's parameter (scene_actors.c): the chunk's slot, which it does not read. */
#define RAC1_TYPE_func_00204FC0_params gaddr chunk __attribute__((unused))

/* Where UpdateWorldObjectAnimation keeps the model record's word at +8 in the transfer state
 * D_0018CC20 (scene_actors.c). */
#define RAC1_TRANSFER_PAD08 0x46

#endif /* OPENRAC_RAC1_VERSION_H */
