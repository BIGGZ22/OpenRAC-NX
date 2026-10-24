/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * RAC1 shadow-rig extraction, shared by the host callback and synthetic tests.
 * Layout comes from level_hero_attach.c, HeroAttachState and HeroAttachSlot (seen in game code).
 */
#ifndef OPENRAC_RAC1_HERO_ATTACHMENTS_H
#define OPENRAC_RAC1_HERO_ATTACHMENTS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Writes the body first, then unique attached equipment addresses. Zero means invalid state.
 * `ram` is a stable frame snapshot; `hero` is the version's hero-state address within it.
 * Detached slots and out-of-range mobys are excluded. The result never exceeds capacity. */
unsigned openrac_rac1_hero_attachments(
    const uint8_t* ram, size_t size, uint32_t hero, uint32_t* addresses, unsigned capacity
);

#ifdef __cplusplus
}
#endif
#endif
