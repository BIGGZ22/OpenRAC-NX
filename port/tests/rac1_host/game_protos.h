/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A stand-in for hostgen's game_protos.h (PAL's names), for unit tests that compile a piece of the
 * shared rac1 host C (port/game/rac1/host) without a disc: only what those pieces call, each
 * defined by the test. */
#ifndef OPENRAC_GAME_PROTOS_H
#define OPENRAC_GAME_PROTOS_H
#include "openrac/guest.h"

/* scene_actors.c */
void func_00118D80(int a0);
int func_0020C468(int a0, int a1);
gaddr func_0020D348(int a0);
void func_00204FC0(gaddr a0);

#endif
