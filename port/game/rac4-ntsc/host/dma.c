/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native boundary for Deadlocked's VIF0 DMA-chain submission.
 * Identified from the retail routine's channel-0 CHCR/QWC/TADR accesses.
 * It caches the unmasked chain address and waits for synchronous completion.
 */
#include <stdint.h>
#include <string.h>
#include "openrac/guest.h"
#include "openrac/game_lib.h"

int func_004E7450(int address) {
    static int submitted_to_native_renderer;
    const uint32_t chain = (uint32_t)address;
    uint32_t previous;
    memcpy(&previous, G(0x002206ECu), sizeof previous);
    if (submitted_to_native_renderer && chain == previous) {
        return (int)previous;
    }
    openrac_lib_sceDmaSend(0x10008000u, chain & 0x0FFFFFFFu);
    /* The retail VIF0 path has no game-registered DMAC callback. Its native
     * renderer hand-off is synchronous, so retire the corresponding bit in
     * Deadlocked's DMA wait mask here. */
    uint32_t pending;
    memcpy(&pending, G(0x002206E8u), sizeof pending);
    pending &= ~1u;
    memcpy(G(0x002206E8u), &pending, sizeof pending);
    memcpy(G(0x002206ECu), &chain, sizeof chain);
    submitted_to_native_renderer = 1;
    return 0;
}

/* Deadlocked submits its title-frame VIF0 work before this VIF1 chain, then
 * waits on bit 0 of 0x2206E8. Native sends are synchronous; after the retail
 * VIF1 interrupt handler has run, retire the paired VIF0 wait as well. */
int func_00139DA0(gaddr channel, gaddr chain) {
    openrac_lib_sceDmaSend(channel, chain);
    if (channel == 0x10009000u) {
        uint32_t pending;
        memcpy(&pending, G(0x002206E8u), sizeof pending);
        pending &= ~1u;
        memcpy(G(0x002206E8u), &pending, sizeof pending);
    }
    return 0;
}
