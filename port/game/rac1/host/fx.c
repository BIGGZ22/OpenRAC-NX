/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: draw callbacks of level effects whose state only feeds their own drawing
 * (libraries.tsv rows: the source files declare them void (void)). They build quads for the
 * console's quad renderer (func_L00_001FD1D8), which the port does not run; what they keep (the
 * elements' positions, timers and colours in their own tables) is read by nothing else. Each is
 * where a native effect renderer takes over.
 */
#include "rac1.h"

/* func_L00_002D8898 (2052 bytes): the draw callback func_L00_002D83D8 registers for its effect
 * (moby p): steps and draws the effect's elements (tables D_L00_001CBAE0, D_L00_001D2B60,
 * D_L00_001D5A40 ... from p's start index). Not done: nothing is drawn or stepped. */
void func_L00_002D8898(gaddr p) {
    (void)p;
}
