/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * RAC1 body and equipped mobys for the separate hero shadow map.
 */
#include "hero_attachments.h"

#include <string.h>

/* Reads a game pointer without alignment assumptions; callers validate the complete hero state. */
static uint32_t address_at(const uint8_t* ram, size_t offset) {
    uint32_t address;
    memcpy(&address, ram + offset, sizeof(address));
    return address;
}

/* Appends a unique, complete moby record while retaining body-first order. */
static unsigned append(
    uint32_t* addresses, unsigned count, unsigned capacity, uint32_t address, size_t size
) {
    /* A moby record occupies 0x100 bytes (seen in game code: HeroAttachMoby). */
    if (!address || address > size || size - address < 0x100 || count >= capacity) {
        return count;
    }

    for (unsigned i = 0; i < count; ++i) {
        /* Reused slot pointers must not submit a caster twice. */
        if (addresses[i] == address) {
            return count;
        }
    }

    addresses[count] = address;
    return count + 1;
}

unsigned openrac_rac1_hero_attachments(
    const uint8_t* ram, size_t size, uint32_t hero, uint32_t* addresses, unsigned capacity
) {
    /* HeroAttachState ends its body pointer at 0x2084 (seen in game code: level_hero_attach.c). */
    if (!ram || !addresses || !capacity || !hero || hero > size || size - hero < 0x2084) {
        return 0;
    }

    unsigned count = append(addresses, 0, capacity, address_at(ram, hero + 0x2080), size);
    /* Equipment without a live hero must not move the shadow projection to an unrelated object. */
    if (!count) {
        return 0;
    }

    /* Seven 0x50-byte slots start at 0x1070; +0x3A marks detached equipment (seen in game code). */
    for (unsigned slot = 0; slot < 7; ++slot) {
        const size_t at = hero + 0x1070 + slot * 0x50;
        /* Thrown equipment resumes independent world casting until attached again. */
        if (ram[at + 0x3A]) {
            continue;
        }

        /* Primary, secondary and auxiliary are at +0x20, +0x24 and +0x2C (seen in game code). */
        const unsigned offsets[] = {0x20, 0x24, 0x2C};
        for (unsigned i = 0; i < 3; ++i) {
            count = append(addresses, count, capacity, address_at(ram, at + offsets[i]), size);
        }
    }

    return count;
}
