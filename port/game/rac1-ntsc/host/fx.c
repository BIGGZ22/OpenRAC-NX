/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): draw callbacks of level effects whose state only feeds their own drawing
 * (libraries.tsv rows: the source files declare them void (void)). They build quads for the
 * console's quad renderer (func_L00_001FD228), which the port does not run; what they keep (the
 * elements' positions, timers and colours in their own tables) is read by nothing else. Each is
 * where a native effect renderer takes over.
 */
/* Carried from port/game/rac1-pal/host/fx.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

/* game_FUN_L00_002d73e8 (2052 bytes): the draw callback game_FUN_L00_002d6f28 registers for its effect
 * (moby p): steps and draws the effect's elements (tables D_L00_001CB7E0, D_L00_001D2860,
 * D_L00_001D5740 ... from p's start index). Not done: nothing is drawn or stepped. */
void game_FUN_L00_002d73e8(gaddr p) {
    (void)p;
}
