/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: lock-strafe, the player's option (settings "lock_strafe", off by default).
 * lock_strafe.h says how the game turns the stick into a heading and what is changed. Two of the
 * hero's routines and one of the follow camera's run under host functions (libraries.tsv, wrap),
 * each calling the game's own: with the option off, or whenever the hero is not strafing, they are
 * the game's routines exactly.
 */
#include "rac1.h"

#include <string.h>

#include "lock_strafe.h"

#define HERO RAC1_ADDR(0x0013F450u)
#define PAD RAC1_ADDR(0x0013CA40u)

static float get_float(gaddr at) {
    float v;
    memcpy(&v, G(at), 4);
    return v;
}

static void put_float(gaddr at, float v) {
    memcpy(G(at), &v, 4);
}

/* Whether the hero strafes now (lock_strafe.h). */
static int strafing(void) {
    if (!openrac_game_lock_strafe()) {
        return 0;
    }
    rac1_strafe_input in;
    in.state = GREF(int32_t, HERO + 0x2084u);
    in.frame_mode = GREF(uint8_t, HERO + 0x20B3u);
    in.slot_state = GREF(int32_t, HERO + 0x1090u + 0x24u);
    in.slot_item = GREF(int32_t, HERO + 0x1090u + 0x28u);
    in.slot_button = GREF(uint32_t, HERO + 0x1090u + 0x10u);
    in.held = GREF(uint32_t, PAD + 0x1A0u);
    return rac1_strafe_active(1, &in);
}

/* Hero_TurnToHeading: eases the facing toward the move yaw (+0x180). While strafing, toward the
 * camera's yaw, which the state code's stick handler adds to the stick's angle (D_L00_00166ED8),
 * the way the game's own state 0x81 turns the hero while L2 or R2 is held: the camera's yaw in the
 * move yaw for the call, the stick's put back after it. */
void func_L00_002124E8(int mode, float accel, float damping, float limit) {
    if (mode != 0 || !strafing()) {
        func_L00_002124E8__game(mode, accel, damping, limit);
        return;
    }
    const gaddr move_yaw = HERO + 0x180u;
    const float stick = get_float(move_yaw);
    const float camera = get_float(OPENRAC_LDATA(0, RAC1_LADDR(00, 0x00166ED8u)));
    put_float(move_yaw, rac1_strafe_turn_target(1, mode, stick, camera));
    func_L00_002124E8__game(mode, accel, damping, limit);
    put_float(move_yaw, stick);
}

/* The follow camera's per-tick requests from the hero's state (func_L00_002E74B0, the camera moby's
 * update calls it before it places the eye). In state 0x81 with L2 or R2 held, the game's strafe,
 * it asks for its strafe camera and returns before its own panning and shake: the camera slides
 * along with the hero (+0x116), held behind him by its lateral spring at 0.04 / 0.2, and the pan
 * timers are cleared. While lock-strafing the same is asked for: the game's routine runs, then what
 * its panning part changed is put as that branch leaves it (the pan weight D_L00_0015F040 as it
 * was, the timers 0, the spring and the slide asked for). */
void func_L00_002E74B0(gaddr camera) {
    if (!strafing()) {
        func_L00_002E74B0__game(camera);
        return;
    }
    const gaddr pan = OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0015F040u));
    const float pan_before = get_float(pan);
    func_L00_002E74B0__game(camera);
    put_float(pan, pan_before);
    func_L00_002E9A18(0);
    func_L00_002E9AF8();
    func_L00_002E9A40(0.04f, 0.2f);
    const gaddr data = GREF(gaddr, camera + 0x70u);
    GREF(int16_t, data + 0x12u) = 0;
    GREF(int16_t, data + 0x14u) = 0;
    GREF(int16_t, data + 0x16u) = 0;
}

/* Hero_SetPlanarVelocity:the velocity at the ground speed along an angle, the facing for one
 * above pi. While strafing, along the move yaw instead: where the stick points, as the game's own
 * run start from a standstill asks for. */
void func_L00_00212790(float angle) {
    if (!(angle <= 3.14159274f) && strafing()) {
        angle = rac1_strafe_planar_angle(1, angle, get_float(HERO + 0x180u));
    }
    func_L00_00212790__game(angle);
}
