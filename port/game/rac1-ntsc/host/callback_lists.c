/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): the draw callback lists' "register" routines that only the levels'
 * programs carry. The engine keeps four lists of (function, argument) pairs, each with its own
 * count (src/game/draw.c); the executable has the routines that register into lists 1 and 2
 * (game_FUN_001f4600, game_FUN_001f47b8) and run each list, and a level's copy of the engine also has the
 * ones for lists 3 and 4. All of them are the same code with other addresses, so the catalogue
 * folds every place into game_FUN_001f4600 (and the runners into game_FUN_001f4650); split_places.tsv
 * gives each place to the routine it is. These two are game_FUN_001f4600 on list 3's and list 4's
 * addresses, the ones the runners game_FUN_001f46c8 and game_FUN_001f4740 read.
 */
/* Carried from port/game/rac1-pal/host/callback_lists.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

static void register_callback(gaddr count_at, gaddr fns, gaddr args, gaddr fn, gaddr arg) {
    const int count = GREF(int, count_at);
    if (count < 0x40) {
        GREF(gaddr, fns + (gaddr)count * 4u) = fn;
        GREF(gaddr, args + (gaddr)count * 4u) = arg;
        GREF(int, count_at) = count + 1;
    }
}

/* List 3: count D_0015F46C, pairs in D_0018DF40 / D_0018E040. */
void func_L02_0020A390(gaddr fn, gaddr arg) {
    register_callback(OPENRAC_DATA(0x0015F46Cu), OPENRAC_DATA(0x0018DF40u), OPENRAC_DATA(0x0018E040u), fn, arg);
}

/* List 4: count D_0015F470, pairs in D_0018E140 / D_0018E240. */
void func_L14_0020C760(gaddr fn, gaddr arg) {
    register_callback(OPENRAC_DATA(0x0015F470u), OPENRAC_DATA(0x0018E140u), OPENRAC_DATA(0x0018E240u), fn, arg);
}
