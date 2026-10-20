/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's DMA hand-offs.
 *
 * The native renderer consumes a chain when it is sent, so every send here
 * completes at once. What the game's own completion handlers then read must
 * be what the DMAC leaves after the whole chain.
 */
#include <stdint.h>
#include <string.h>

#include "openrac/guest.h"
#include "openrac/game_host.h"
#include "openrac/game_lib.h"

static uint32_t reg(gaddr at) {
    uint32_t v;
    memcpy(&v, G(at), sizeof v);
    return v;
}

static void set_reg(gaddr at, uint32_t v) {
    memcpy(G(at), &v, sizeof v);
}

/* sceDmaSend (0x00127220, the same instructions as rac1-pal's): as the shared
 * replacement, but the channel's CHCR is left as the DMAC leaves it after the
 * chain's end tag: tag ID 7 (end) in its upper half, the start bit clear.
 * Going Commando's VIF1 handler (0x002FCE18, installed by 0x002FCD18) reads
 * that tag field: an end tag retires every stage of the frame's wait mask
 * (0x001AD520, waited on by 0x002FC3C0); another value retires one stage and
 * restarts the channel, which a chain already drawn cannot do. */
void func_00127220(gaddr channel, gaddr tag) {
    if ((tag >> 28) == 7u) {
        tag = (tag & 0x0FFFFFFFu) | 0x80000000u; /* libdma's scratchpad form, as the shared one */
    }
    if (reg(channel + 0x30u) != 0xFFFFFFFFu) {
        set_reg(channel + 0x30u, tag);
    }
    set_reg(channel + 0x20u, 0);
    set_reg(channel, (reg(channel) & ~0xCu) | 0x105u);

    openrac_game_dma_send(channel, tag);

    const uint32_t done = (reg(channel) & ~0x100u & 0x0000FFFFu) | 0x70000000u;
    set_reg(channel, done);
    openrac_game_run_dmac_handlers((int)((channel >> 12) & 0xF) - 8);
}

/* KickVif0Chain (0x002FC070, matched C in the decompilation) waits until
 * VIF0's channel is idle, writes QWC 0 and TADR, starts the channel in
 * source-chain mode (CHCR 0x145) and waits again. With the synchronous
 * hand-off neither wait has anything to wait for. */
void func_002FC070(int chain) {
    func_00127220(0x10008000u, (gaddr)chain & 0x0FFFFFFFu);
}
