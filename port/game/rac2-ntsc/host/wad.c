/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Going Commando's WAD decompressor, 0x0029D4D0 (hand-written assembly).
 *
 * The retail routine checks the "WAD" header at its first argument, streams
 * the compressed data through the scratchpad by DMA (channel 0x1000D400) and
 * writes the output at its second argument; it returns the number of bytes
 * written (its output pointer less the second argument) and traps on a bad
 * header. The port's routine every game shares does the same in memory
 * (common/lib/wad_routine.h).
 */
#include "game/common/lib/wad_routine.h"

int func_0029D4D0(int source, int destination) {
    return openrac_wad_routine(source, destination, "Going Commando");
}
