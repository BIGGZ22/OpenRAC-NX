/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the draw callback lists' "register" routines that only the levels'
 * programs carry. The engine keeps four lists of (function, argument) pairs, each with its own
 * count (src/game/draw.c); the executable has the routines that register into lists 1 and 2
 * (func_001F49B0, func_001F4B68) and run each list, and a level's copy of the engine also has the
 * ones for lists 3 and 4. All of them are the same code with other addresses, so the catalogue
 * folds every place into func_001F49B0 (and the runners into func_001F4A00); split_places.tsv
 * gives each place to the routine it is. These two are func_001F49B0 on list 3's and list 4's
 * addresses, the ones the runners func_001F4A78 and func_001F4AF0 read.
 */
#include "rac1.h"

static void register_callback(gaddr count_at, gaddr fns, gaddr args, gaddr fn, gaddr arg) {
    const int count = GREF(int, count_at);
    if (count < 0x40) {
        GREF(gaddr, fns + (gaddr)count * 4u) = fn;
        GREF(gaddr, args + (gaddr)count * 4u) = arg;
        GREF(int, count_at) = count + 1;
    }
}

/* List 3: count D_0015F56C, pairs in D_0018E040 / D_0018E140. */
void func_L02_0020A690(gaddr fn, gaddr arg) {
    register_callback(OPENRAC_DATA(RAC1_ADDR(0x0015F56Cu)), OPENRAC_DATA(RAC1_ADDR(0x0018E040u)), OPENRAC_DATA(RAC1_ADDR(0x0018E140u)), fn, arg);
}

/* List 4: count D_0015F570, pairs in D_0018E240 / D_0018E340. */
void func_L14_0020CAE0(gaddr fn, gaddr arg) {
    register_callback(OPENRAC_DATA(RAC1_ADDR(0x0015F570u)), OPENRAC_DATA(RAC1_ADDR(0x0018E240u)), OPENRAC_DATA(RAC1_ADDR(0x0018E340u)), fn, arg);
}
