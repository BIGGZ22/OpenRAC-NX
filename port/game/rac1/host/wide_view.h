/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank: the arithmetic of wide_view.c, apart from game memory so that a test can run it
 * (port/tests/wide_view_test.c): when the bounding sphere test (FastBSphereCheck) sees a view
 * opened to the picture's sides, and the clip constants it then sees.
 */
#ifndef OPENRAC_RAC1_WIDE_VIEW_H
#define OPENRAC_RAC1_WIDE_VIEW_H

#include <math.h>

/*
 * How much wider the sphere test's view is opened: `lists` how many draw callback lists run (their
 * `list_wider`), `effects` how many updates run whose test stands for what the player sees
 * (their `effect_wider`: the nanotech cluster's, Kerwan's air traffic's). 0 where the test sees the game's own 4:3 view: outside
 * both, where it serves the game's logic too, and in a picture no wider than 4:3.
 */
static inline float wide_sphere_wider(int lists, float list_wider, int effects, float effect_wider) {
    const float wider = lists > 0 ? list_wider : effects > 0 ? effect_wider : 0.0f;
    return wider > 1.0f ? wider : 0.0f;
}

/* The clip constants' horizontal tangent `tan_x` opened by `wider`, and its secant. */
static inline void wide_clip(float tan_x, float wider, float* tan_out, float* sec_out) {
    *tan_out = tan_x * wider;
    *sec_out = sqrtf(1.0f + *tan_out * *tan_out);
}

#endif
