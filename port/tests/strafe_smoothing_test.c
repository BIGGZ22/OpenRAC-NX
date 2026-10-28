/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "platform/switch/strafe_smoothing.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(void) {
    float x = 1, y = 0;
    for (int i = 0; i < 60; ++i) {
        float previous = x;
        openrac_strafe_smooth_velocity(x, y, -1, 0, 1.0f / 60, &x, &y);
        CHECK(isfinite(x) && x >= -1 && x <= previous && y == 0);
    }
    CHECK(x < -0.999f);
    openrac_strafe_smooth_velocity(x, 0.5f, 0, 0, 1.0f / 60, &x, &y);
    CHECK(x == 0 && y == 0);
    float from = 3.12413936f, to = -3.12413936f;
    float yaw = openrac_strafe_smooth_yaw(from, to, 1.0f / 60);
    CHECK(fabsf(remainderf(yaw - from, 6.2831853f)) < 0.035f);
    float results[3];
    const int rates[3] = {30, 60, 120};
    for (int i = 0; i < 3; ++i) {
        yaw = 0;
        for (int j = 0; j < rates[i] / 2; ++j)
            yaw = openrac_strafe_smooth_yaw(yaw, 1, 1.0f / rates[i]);
        results[i] = yaw;
    }
    CHECK(fabsf(results[0] - results[1]) < 0.00001f && fabsf(results[1] - results[2]) < 0.00001f);
    x = y = 0;
    for (int i = 0; i < 60; ++i) openrac_strafe_smooth_velocity(x, y, 1, 1, 1.0f / 60, &x, &y);
    CHECK(x > 0.999f && y > 0.999f); /* Full diagonal speed is retained. */
    CHECK(isfinite(openrac_strafe_smooth_yaw(0, 1, NAN)));
    return 0;
}
