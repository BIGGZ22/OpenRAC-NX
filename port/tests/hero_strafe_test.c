/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The strafe's decision (port/game/rac1/host/strafe.h): when the hero strafes and how, which ticks
 * the hero's update does not see L2 and R2, where his facing turns and which way his velocity
 * goes; with the setting off, always the game's own. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../game/rac1/host/strafe.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"%d: %s\n",__LINE__,#c); exit(1); } } while(0)

#define BLASTER 0x0F
#define WRENCH 0x08
#define SWINGSHOT 0x0C
#define CIRCLE 0x20u
#define L1 0x4u
#define R1 0x8u

/* The hero running on level ground on foot, the blaster ready, nothing held. */
static rac1_strafe_input running(void) {
    rac1_strafe_input in = {1, RAC1_STRAFE_STATE_RUN, 0, 0, RAC1_STRAFE_SLOT_READY, BLASTER, 0, 0.75f};
    return in;
}

/* One tick as the run does it, with the decision in the two places strafe.c changes: the facing
 * eased toward its target by a fraction (the game springs it, the same direction), the velocity
 * along the planar angle (the facing for 99999, the run's request). */
static void tick(int kind, float camera, float stick, float hold_yaw, float* facing, float v[2]) {
    /* The stick handler adds the rotations as the game's FastAddRots does: within -pi..pi. */
    float move_yaw = camera + stick;
    if (move_yaw > 3.1415927f) {
        move_yaw -= 6.2831855f;
    } else if (move_yaw < -3.1415927f) {
        move_yaw += 6.2831855f;
    }
    const float target = rac1_strafe_turn_target(kind, 0, move_yaw, camera, hold_yaw);
    *facing += (target - *facing) * 0.25f;
    float angle = rac1_strafe_planar_angle(kind, 99999.0f, move_yaw);
    if (!(angle <= 3.14159274f)) {
        angle = *facing;
    }
    v[0] = cosf(angle);
    v[1] = sinf(angle);
}

static void items(void) {
    const int aimable[] = {0x09, 0x0A, 0x0B, 0x0F, 0x10, 0x13, 0x15, 0x17};
    const int not_aimable[] = {WRENCH, SWINGSHOT, 0x0D, 0x0E, 0x11, 0x12, 0x14, 0x16, 0x18, 0x19, 0x1A, 0x1B, 0x1F, 0x20, 0, 0x26};
    for (unsigned i = 0; i < sizeof aimable / sizeof aimable[0]; ++i) {
        CHECK(rac1_strafe_aimable(aimable[i]));
    }
    for (unsigned i = 0; i < sizeof not_aimable / sizeof not_aimable[0]; ++i) {
        CHECK(!rac1_strafe_aimable(not_aimable[i]));
    }
}

/* Off: nothing strafes, L2 and R2 reach the game, nothing is latched. */
static void off(void) {
    rac1_strafe_hold hold = {0, 0.0f};
    rac1_strafe_input in = running();
    in.held = RAC1_PAD_R2 | CIRCLE;
    rac1_strafe_hold_update(RAC1_STRAFE_OFF, &in, &hold);
    CHECK(!hold.latched);
    CHECK(!rac1_strafe_takes_buttons(RAC1_STRAFE_OFF, &in));
    CHECK(rac1_strafe_kind(RAC1_STRAFE_OFF, &in, &hold) == RAC1_STRAFE_NONE);
    hold.latched = 1; /* even a latch left over gives nothing */
    CHECK(rac1_strafe_kind(RAC1_STRAFE_OFF, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_NONE, RAC1_STRAFE_STATE_STAND));
    /* The game's own values pass through untouched. */
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_NONE, 0, 1.25f, -2.0f, 0.5f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_NONE, 99999.0f, 1.25f) == 99999.0f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_NONE, 0.5f, 1.25f) == 0.5f);
}

/* Hold: L2 or R2 going down latches the facing; held, he strafes in the run, the jump and standing;
 * let go, the game's own. */
static void hold_mode(void) {
    const int mode = RAC1_STRAFE_HOLD;
    rac1_strafe_hold hold = {0, 0.0f};
    rac1_strafe_input in = running();
    in.slot_item = WRENCH;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched && rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(rac1_strafe_takes_buttons(mode, &in)); /* on foot: L2 and R2 are the strafe's */
    in.held = RAC1_PAD_R2;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.latched && hold.yaw == 0.75f);
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_HELD);
    /* Held: the facing he turns to later is not latched again. */
    in.facing = -1.0f;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.yaw == 0.75f);
    /* L2 does it too, and with both the latch stays. */
    in.held = RAC1_PAD_L2 | RAC1_PAD_R2;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.latched && hold.yaw == 0.75f);
    /* The jump, the alternate run and standing strafe too; other states do not. */
    const int strafing_states[] = {RAC1_STRAFE_STATE_RUN, RAC1_STRAFE_STATE_RUN_ALT, RAC1_STRAFE_STATE_JUMP, RAC1_STRAFE_STATE_STAND};
    for (unsigned i = 0; i < 4; ++i) {
        in.state = strafing_states[i];
        CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_HELD);
    }
    const int other_states[] = {0x01, 0x35, 0x0B, 0x81, 0x13, 0x77};
    for (unsigned i = 0; i < 6; ++i) {
        in.state = other_states[i];
        CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    }
    in.state = RAC1_STRAFE_STATE_RUN;
    /* A wall's own frame (the Magneboots) and a vehicle: not. */
    in.frame_mode = 1;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    in.frame_mode = 0;
    in.vehicle_mode = 1;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(!rac1_strafe_takes_buttons(mode, &in));
    in.vehicle_mode = 0;
    /* A scene, a menu, a vendor (not game mode 0), or no level: the game keeps L2 and R2 and
     * nothing strafes. */
    in.playing = 0;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(!rac1_strafe_takes_buttons(mode, &in));
    in.playing = 1;
    /* L1, R1 and Circle never strafe and never latch. */
    hold.latched = 0;
    in.held = L1 | R1 | CIRCLE;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched && rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    /* Let go: forgotten. */
    in.held = RAC1_PAD_L2;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.latched);
    in.held = 0;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched && rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    /* Pressed in a vehicle (or in the game's own strafe state): latched once he is on foot, with the
     * facing then. */
    in.vehicle_mode = 2;
    in.held = RAC1_PAD_R2;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched);
    in.vehicle_mode = 0;
    in.playing = 0; /* held through a scene: not latched either */
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched);
    in.playing = 1;
    in.state = RAC1_STRAFE_STATE_GAME;
    in.vehicle_mode = 0;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched && !rac1_strafe_takes_buttons(mode, &in));
    in.state = RAC1_STRAFE_STATE_RUN;
    in.facing = 2.0f;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.latched && hold.yaw == 2.0f);
    /* The aimable weapon makes no difference to Hold. */
    in.slot_item = BLASTER;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_HELD);
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_HELD, RAC1_STRAFE_STATE_STAND));
    /* Targets: the latched yaw for turns of mode 0; the move yaw for the velocity. */
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_HELD, 0, 1.25f, -2.0f, 0.5f) == 0.5f);
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_HELD, 1, 1.25f, -2.0f, 0.5f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_HELD, 99999.0f, 1.25f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_HELD, 0.5f, 1.25f) == 0.5f);
}

/* Lock: a forward-firing weapon ready makes him face the camera without a button; anything else in
 * hand is Hold. */
static void lock_mode(void) {
    const int mode = RAC1_STRAFE_LOCK;
    rac1_strafe_hold hold = {0, 0.0f};
    rac1_strafe_input in = running();
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_LOCKED);
    CHECK(rac1_strafe_takes_buttons(mode, &in));
    in.state = RAC1_STRAFE_STATE_STAND;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_LOCKED);
    CHECK(rac1_strafe_turns_standing(RAC1_STRAFE_LOCKED, in.state));
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_LOCKED, RAC1_STRAFE_STATE_RUN));
    in.state = RAC1_STRAFE_STATE_RUN;
    /* L2 held with the weapon: still the camera's. */
    in.held = RAC1_PAD_L2;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_LOCKED);
    /* The weapon not ready (being put away, coming out): Hold's, while L2 is held. */
    in.slot_state = 3;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_HELD);
    in.held = 0;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    in.slot_state = RAC1_STRAFE_SLOT_READY;
    /* The wrench and the gadgets: Hold. */
    const int others[] = {WRENCH, SWINGSHOT, 0x16, 0x12};
    for (unsigned i = 0; i < 4; ++i) {
        in.slot_item = others[i];
        in.held = 0;
        rac1_strafe_hold_update(mode, &in, &hold);
        CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
        in.held = RAC1_PAD_R2;
        rac1_strafe_hold_update(mode, &in, &hold);
        CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_HELD);
    }
    /* Vehicles, walls, other states: never. */
    in.slot_item = BLASTER;
    in.vehicle_mode = 1;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    in.vehicle_mode = 0;
    in.frame_mode = 2;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    in.frame_mode = 0;
    in.state = 0x35; /* crouching */
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_LOCKED, 0, 1.25f, -2.0f, 0.5f) == -2.0f);
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_LOCKED, 1, 1.25f, -2.0f, 0.5f) == 1.25f);
}

/* Moving: stick right (a quarter turn clockwise of the camera's forward) for a while. */
static void movement(void) {
    const float camera = 0.5f;
    const float right = -1.5707964f;
    float facing, v[2];
    /* Lock: he ends facing the camera's way and moving at right angles to it. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_LOCKED, camera, right, 0.0f, &facing, v);
    }
    CHECK(fabsf(facing - camera) < 1e-4f);
    CHECK(fabsf(v[0] * cosf(camera) + v[1] * sinf(camera)) < 1e-4f); /* no forward part */
    CHECK(fabsf(v[0] - cosf(camera + right)) < 1e-4f && fabsf(v[1] - sinf(camera + right)) < 1e-4f);
    /* Hold, latched at 1.2 with the camera elsewhere: he keeps 1.2 and moves where the stick says. */
    facing = 1.2f;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_HELD, camera, right, 1.2f, &facing, v);
    }
    CHECK(fabsf(facing - 1.2f) < 1e-4f);
    CHECK(fabsf(v[0] - cosf(camera + right)) < 1e-4f && fabsf(v[1] - sinf(camera + right)) < 1e-4f);
    /* The game's way: he turns round to face where he goes. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_NONE, camera, right, 0.0f, &facing, v);
    }
    CHECK(fabsf(facing - (camera + right)) < 1e-4f);
    /* Back: still facing the camera's way, moving toward it. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_LOCKED, camera, 3.1415927f, 0.0f, &facing, v);
    }
    CHECK(fabsf(facing - camera) < 1e-4f && v[0] * cosf(camera) + v[1] * sinf(camera) < -0.999f);
}

int main(void) {
    items();
    off();
    hold_mode();
    lock_mode();
    movement();
    puts("hero strafe: weapons, off, hold, lock, the buttons taken, facing and velocity passed");
    return 0;
}
