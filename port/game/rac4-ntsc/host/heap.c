/* SPDX-License-Identifier: GPL-3.0-or-later
 * NTSC-U's newlib program break, without the console's interrupt masking.
 * Startup 0x001576B0..0x001576C4 configures SetupHeap(0x01F38000, 0xC6500).
 * sbrk's binary identification was checked against libkernl.a/glue.o.
 */
#include <stdint.h>
#include "openrac/guest.h"

#define RAC4_HEAP_END (0x01F38000u + 0x000C6500u)

gaddr func_00126B80(void) {
    return RAC4_HEAP_END;
}

gaddr func_00127298(int increment) {
    const uint32_t previous = GREF(uint32_t, 0x0016468Cu);
    const uint32_t next = previous + (uint32_t)increment;
    if (next > RAC4_HEAP_END) {
        const gaddr reent = GREF(gaddr, 0x00163E74u);
        GREF(int, reent) = 12; /* The game's ENOMEM. */
        return UINT32_MAX;
    }
    GREF(uint32_t, 0x0016468Cu) = next;
    return previous;
}
