/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank: whether a moby is in view, the test MobyProc makes before it draws one
 * (render_native.c, mark_drawn_mobys), apart from game memory so that a test can run it
 * (port/tests/moby_view_test.c).
 *
 * Its verdict is the moby's +0x31, "drawn this frame", which the game reads back: the moby loop
 * keeps a drawn moby active, and many mobys only make their effects while drawn. The nanotech
 * crates spawn their glowing orb, the crates and bolts their glows and sparkles, only then. The
 * game tests against its own 4:3 view, the horizontal tangent of half its field of view; the port's
 * picture shows that view in its middle and opens the tangent by as much as the picture is wider
 * (openrac_game_picture_wider). Tested against the 4:3 tangent, a moby between the 4:3 edge and
 * the picture's was drawn by the port's renderer but not "drawn" for the game, and its effects
 * stopped: the nanotech crates near the sides of a 16:9 picture lost their orbs. The tangent here
 * is opened by the same amount; at 4:3 nothing changes.
 */
#ifndef OPENRAC_RAC1_MOBY_VIEW_H
#define OPENRAC_RAC1_MOBY_VIEW_H

#include <math.h>

/*
 * `d` the sphere's centre less the camera's (world units), `rows` the camera's 3 x 4 rows (z, x,
 * y: forward, side, up), `r` the radius, `distance` the draw distance, `tx` and `ty` the tangents
 * of half the game's field of view, `wider` how many times wider than 4:3 the picture is (at
 * least 1). In view: within the draw distance, not wholly behind the near plane, not wholly
 * outside a side plane.
 */
static inline int moby_in_view(const float d[3], const float rows[12], float r, float distance, float tx,
                               float ty, float wider) {
    if (wider > 1.0f) {
        tx *= wider;
    }
    const float kx = sqrtf(1.0f + tx * tx), ky = sqrtf(1.0f + ty * ty);
    const float z = d[0] * rows[0] + d[1] * rows[1] + d[2] * rows[2];
    const float x = d[0] * rows[4] + d[1] * rows[5] + d[2] * rows[6];
    const float y = d[0] * rows[8] + d[1] * rows[9] + d[2] * rows[10];
    return z <= distance && z + r > 32.0f / 1024.0f && fabsf(x) - r * kx <= tx * z && fabsf(y) - r * ky <= ty * z;
}

#endif
