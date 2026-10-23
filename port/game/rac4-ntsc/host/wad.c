/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Native boundary for the NTSC-U WAD decoder at 0x00469C40.
 * The retail routine checks the WAD header, feeds compressed data through
 * scratchpad DMA, and returns the number of decompressed bytes.
 * The port's routine every game shares does the same in memory
 * (common/lib/wad_routine.h).
 */
#include "game/common/lib/wad_routine.h"

int func_00469C40(int source, int destination) {
    return openrac_wad_routine(source, destination, "Deadlocked");
}
