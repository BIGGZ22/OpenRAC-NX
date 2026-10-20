/* SPDX-License-Identifier: GPL-3.0-or-later */
/* When the bounding sphere test sees the picture's sides, and what it then sees (wide_view.h). */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../game/rac1/host/wide_view.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)

/* FastBSphereCheck's side test (wide_view.c): culled when |x| - sec * r > tan * z. */
static int culled(float x, float z, float r, float tan_x, float sec_x) {
    return fabsf(x) - sec_x * r > tan_x * z;
}

int main(void) {
    const float wide = (16.0f / 9.0f) / (4.0f / 3.0f);
    /* Outside the lists and the drawing updates (the game's logic) the view is the game's. */
    CHECK(wide_sphere_wider(0, 1.0f, 0, 1.0f) == 0.0f);
    CHECK(wide_sphere_wider(0, wide, 0, wide) == 0.0f);
    /* In the nanotech cluster's update, at 16:9: opened; at 4:3: not. */
    CHECK(wide_sphere_wider(0, 1.0f, 1, wide) == wide);
    CHECK(wide_sphere_wider(0, 1.0f, 1, 1.0f) == 0.0f);
    /* In a draw callback list, the list's. */
    CHECK(wide_sphere_wider(1, wide, 0, 1.0f) == wide);
    CHECK(wide_sphere_wider(1, wide, 1, 2.0f) == wide);
    /* A value below 1 never narrows the view. */
    CHECK(wide_sphere_wider(0, 1.0f, 1, 0.5f) == 0.0f);

    /* The orb between the 4:3 edge (x = 6.3 at z = 10) and the 16:9 one (x = 8.4): culled against
     * the game's constants, in view against the opened ones, on both sides; beyond 16:9 still out. */
    const float tan_x = 0.63f, sec_x = sqrtf(1.0f + tan_x * tan_x), r = 0.5f;
    float t, s;
    wide_clip(tan_x, wide, &t, &s);
    CHECK(fabsf(t - tan_x * wide) < 1e-6f && fabsf(s - sqrtf(1.0f + t * t)) < 1e-6f);
    CHECK(culled(7.8f, 10.0f, r, tan_x, sec_x));
    CHECK(!culled(7.8f, 10.0f, r, t, s));
    CHECK(!culled(-7.8f, 10.0f, r, t, s));
    CHECK(culled(10.0f, 10.0f, r, t, s));
    return 0;
}
