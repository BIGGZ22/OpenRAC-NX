/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * WadDecompress's two hand-written subroutines for its transfers into the scratchpad by DMA
 * (channel 9, toSPR). WadDecompress (func_0020C468) streams its input through the scratchpad a
 * 0x2000-byte window at a time: one subroutine starts the window's transfer, the other waits for
 * it. The port's WadDecompress (rac1/host/hand.c) decompresses straight from game memory and calls
 * neither, so nothing in the port reaches them; this is what they do, in the port's terms (PAL
 * only, libraries.tsv binds them). On the console the game writes the channel's registers and the
 * DMA controller copies; the port has no DMA controller, so the start writes the registers as
 * retail does and then does the transfer itself, leaving the registers as a finished transfer
 * leaves them.
 */
#include <string.h>

#include "rac1.h"

#define D9_CHCR 0x1000D400u
#define D9_MADR 0x1000D410u
#define D9_QWC 0x1000D420u
#define D9_SADR 0x1000D480u

/* A normal-mode toSPR transfer of `qwc` quadwords from main memory `madr` to scratchpad offset
 * `sadr` (its low 14 bits), done at once. */
static void spr_from_memory(uint32_t madr, uint32_t sadr, uint32_t qwc) {
    uint32_t from = madr & 0x01FFFFF0u;
    uint32_t to = sadr & 0x3FF0u;
    uint32_t bytes = qwc * 16u;

    if (to + bytes > GUEST_SCRATCHPAD_SIZE) {
        bytes = GUEST_SCRATCHPAD_SIZE - to;
    }
    memmove(G(GUEST_SCRATCHPAD + to), G(from), bytes);
    GREF(volatile uint32_t, D9_MADR) = madr + qwc * 16u;
    GREF(volatile uint32_t, D9_SADR) = sadr + qwc * 16u;
    GREF(volatile uint32_t, D9_QWC) = 0;
    GREF(volatile uint32_t, D9_CHCR) &= ~0x100u;
}

/*
 * func_0020C738 (the executable and every level's copy, 32 bytes, hand-written): WadDecompress's
 * start of the toSPR transfer of the stream's next 0x2000-byte window. Its inputs are registers no C
 * prototype names: $v1 the channel (0x1000D400, from its one caller), $t3 the window's place in the
 * stream (MADR), $t6 the scratchpad half it fills (SADR); it writes SADR, QWC 0x200, MADR and CHCR
 * 0x100 (start, normal mode) and gives back in $t6 the other half (SADR ^ 0x2000). Here MADR and SADR
 * are its arguments and the other half its result, the transfer done at once.
 */
unsigned int func_0020C738(unsigned int madr, unsigned int sadr) {
    GREF(volatile uint32_t, D9_SADR) = sadr;
    GREF(volatile uint32_t, D9_QWC) = 0x200u;
    GREF(volatile uint32_t, D9_MADR) = madr;
    GREF(volatile uint32_t, D9_CHCR) = 0x100u;
    spr_from_memory(madr, sadr, 0x200u);
    return sadr ^ 0x2000u;
}

/*
 * func_0020C758 (72 bytes, hand-written): WadDecompress's wait until the toSPR channel at $v1 has
 * finished (CHCR's STR, 0x100, clear), polled; behind its return two traps of WadDecompress's error
 * path. The port's transfers finish when they start (above), so there is nothing to wait for (as
 * VU0_loadMicroProgram's wait, rac1/host/hand.c).
 */
void func_0020C758(void) {}
