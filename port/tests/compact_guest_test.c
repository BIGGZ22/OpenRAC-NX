/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "openrac/guest.h"
#undef NDEBUG
#include <assert.h>
#include <stdlib.h>

uint8_t* openrac_guest_base;
int main(void) {
    openrac_guest_base = calloc(1, OPENRAC_COMPACT_SIZE);
    assert(openrac_guest_base);
    assert(OPENRAC_COMPACT_SIZE < 50u * 1024u * 1024u);
    GREF(uint32_t, 0x1234) = 0x12345678;
    assert(GREF(uint32_t, GUEST_RAM_UNCACHED + 0x1234) == 0x12345678);
    GREF(uint32_t, GUEST_RAM_UNCACHED_ACCEL + 0x1234) = 0xABCDEF01;
    assert(GREF(uint32_t, 0x1234) == 0xABCDEF01);
    const gaddr regions[] = {GUEST_RAM, GUEST_EE_REGS, GUEST_VU_MEM, GUEST_GS_REGS,
                            GUEST_SCRATCHPAD, GUEST_PORT};
    const uint32_t sizes[] = {GUEST_RAM_SIZE, GUEST_EE_REGS_SIZE, GUEST_VU_MEM_SIZE,
                             GUEST_GS_REGS_SIZE, GUEST_SCRATCHPAD_SIZE, GUEST_PORT_SIZE};
    for (unsigned i = 0; i < sizeof(regions) / sizeof(regions[0]); ++i) {
        const gaddr first = regions[i], last = first + sizes[i] - 1;
        assert(GADDR(G(first)) == first && GADDR(G(last)) == last);
        GREF(uint8_t, last) = (uint8_t)(10 + i);
    }
    for (unsigned i = 0; i < sizeof(regions) / sizeof(regions[0]); ++i)
        assert(GREF(uint8_t, regions[i] + sizes[i] - 1) == 10 + i);
    free(openrac_guest_base);
    return 0;
}
