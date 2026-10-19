/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Whether a moby is "drawn" for the game (+0x31) in a picture wider than 4:3 (moby_view.h). */
#include <stdio.h>
#include <stdlib.h>
#include "../game/rac1/host/moby_view.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)

/* The camera looking along +z of its rows: forward z, side x, up y. */
static const float rows[12] = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0};

int main(void) {
    const float tx = 0.63f, ty = 0.63f * 0.75f;
    const float wide = (16.0f / 9.0f) / (4.0f / 3.0f);
    /* In the middle: in view at 4:3 and wider. */
    const float middle[3] = {0.0f, 0.0f, 10.0f};
    CHECK(moby_in_view(middle, rows, 0.5f, 64.0f, tx, ty, 1.0f));
    CHECK(moby_in_view(middle, rows, 0.5f, 64.0f, tx, ty, wide));
    /* Between the 4:3 edge (x = 6.3 at z = 10) and the 16:9 one (x = 8.4): the nanotech crate at
     * the side of the picture. Out at 4:3, in at 16:9, on both sides. */
    const float right[3] = {7.5f, 0.0f, 10.0f}, left[3] = {-7.5f, 0.0f, 10.0f};
    CHECK(!moby_in_view(right, rows, 0.5f, 64.0f, tx, ty, 1.0f));
    CHECK(moby_in_view(right, rows, 0.5f, 64.0f, tx, ty, wide));
    CHECK(moby_in_view(left, rows, 0.5f, 64.0f, tx, ty, wide));
    /* Beyond the 16:9 edge: out. */
    const float beyond[3] = {10.0f, 0.0f, 10.0f};
    CHECK(!moby_in_view(beyond, rows, 0.5f, 64.0f, tx, ty, wide));
    /* The vertical planes stay the game's. */
    const float above[3] = {0.0f, 6.0f, 10.0f};
    CHECK(!moby_in_view(above, rows, 0.5f, 64.0f, tx, ty, wide));
    /* A value below 1 never narrows the view. */
    CHECK(moby_in_view(right, rows, 0.5f, 64.0f, tx * 1.2f, ty, 0.5f));
    /* Draw distance and near plane as before. */
    const float far[3] = {0.0f, 0.0f, 70.0f}, behind[3] = {0.0f, 0.0f, -2.0f};
    CHECK(!moby_in_view(far, rows, 0.5f, 64.0f, tx, ty, wide));
    CHECK(!moby_in_view(behind, rows, 0.5f, 64.0f, tx, ty, wide));
    return 0;
}
