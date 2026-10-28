/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef OPENRAC_SWITCH_STRAFE_SMOOTHING_H
#define OPENRAC_SWITCH_STRAFE_SMOOTHING_H
#include <math.h>

/* Seconds, independent of the game's frame rate. Keep aiming responsive. */
#define OPENRAC_STRAFE_FACING_TIME 0.040f
#define OPENRAC_STRAFE_MOVE_TIME 0.030f
#define OPENRAC_STRAFE_REVERSE_TIME 0.045f

static inline float openrac_strafe_dt(float dt) {
    if (!isfinite(dt) || dt <= 0.0f) return 1.0f / 60.0f;
    return fminf(dt, 0.05f);
}
static inline float openrac_strafe_blend(float dt, float time) {
    return -expm1f(-openrac_strafe_dt(dt) / time);
}
static inline float openrac_strafe_smooth_yaw(float from, float to, float dt) {
    const float pi = 3.14159265358979323846f;
    if (!isfinite(from)) return to;
    if (!isfinite(to)) return from;
    const float difference = remainderf(to - from, 2.0f * pi);
    return remainderf(from + difference * openrac_strafe_blend(dt, OPENRAC_STRAFE_FACING_TIME), 2.0f * pi);
}
static inline void openrac_strafe_smooth_velocity(float old_x, float old_y,
    float target_x, float target_y, float dt, float* x, float* y) {
    /* Stopping stays exact: no extra slide or creeping across a ledge. */
    if ((target_x == 0.0f && target_y == 0.0f) || !isfinite(old_x) || !isfinite(old_y)) {
        *x = target_x; *y = target_y; return;
    }
    const float time = old_x * target_x + old_y * target_y < 0.0f
        ? OPENRAC_STRAFE_REVERSE_TIME : OPENRAC_STRAFE_MOVE_TIME;
    const float blend = openrac_strafe_blend(dt, time);
    *x = old_x + (target_x - old_x) * blend;
    *y = old_y + (target_y - old_y) * blend;
}
#endif
