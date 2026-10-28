// SPDX-License-Identifier: GPL-3.0-or-later
// Included inside the pinned lock-strafe module, after its memory helpers.
struct SwitchStrafeSmoothing {
    bool facing = false, moving = false, applied = false;
    float yaw = 0.0f, old_x = 0.0f, old_y = 0.0f, dt = 1.0f / 60.0f;
};
SwitchStrafeSmoothing s_switch_smoothing;

void switch_strafe_begin_tick() {
    rac1_strafe_input input;
    read_input(&input);
    const int kind = rac1_strafe_kind(s_mode, &input, &s_hold);
    auto& s = s_switch_smoothing;
    s.applied = false;
    if (kind == RAC1_STRAFE_NONE) {
        s = SwitchStrafeSmoothing{};
        return;
    }
    s.dt = openrac_strafe_dt(get_float(s_mod->relocate(s_disc->turn_step)));
    const float target = rac1_strafe_turn_target(kind, 0,
        get_float(hero() + k_hero_move_yaw), input.camera, &s_hold);
    if (!s.facing) s.yaw = input.facing;
    s.yaw = openrac_strafe_smooth_yaw(s.yaw, target, s.dt);
    s.facing = true;
    // Grounded run only: preserve jumping, knockback and special movement physics.
    s.moving = input.state == RAC1_STRAFE_STATE_RUN || input.state == RAC1_STRAFE_STATE_RUN_ALT;
    // Start from actual motion each tick, so wall collisions never accumulate momentum.
    s.old_x = get_float(hero() + 0xE0u);
    s.old_y = get_float(hero() + 0xE4u);
}

float switch_strafe_facing(float requested) {
    return s_switch_smoothing.facing ? s_switch_smoothing.yaw : requested;
}

void switch_strafe_finish_velocity() {
    auto& s = s_switch_smoothing;
    if (!s.moving || s.applied) return;
    rac1_strafe_input input;
    read_input(&input);
    if (rac1_strafe_kind(s_mode, &input, &s_hold) == RAC1_STRAFE_NONE
        || (input.state != RAC1_STRAFE_STATE_RUN && input.state != RAC1_STRAFE_STATE_RUN_ALT)) return;
    s.applied = true;
    float x, y;
    openrac_strafe_smooth_velocity(s.old_x, s.old_y,
        get_float(hero() + 0xE0u), get_float(hero() + 0xE4u), s.dt, &x, &y);
    put_float(hero() + 0xE0u, x);
    put_float(hero() + 0xE4u, y);
}
