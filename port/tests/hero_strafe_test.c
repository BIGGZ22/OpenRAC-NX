/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Lock-strafe's decision (port/game/rac1/host/lock_strafe.h): when the hero strafes, where his
 * facing turns and which way his velocity goes; with the setting off, always the game's own. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../game/rac1/host/lock_strafe.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"%d: %s\n",__LINE__,#c); exit(1); } } while(0)

/* The hero running with the bomb glove ready and Circle (its button) held. */
static rac1_strafe_input firing(void) {
    rac1_strafe_input in = {RAC1_STRAFE_STATE_RUN, 0, RAC1_STRAFE_SLOT_READY, 10, 0x20, 0x20};
    return in;
}

/* One tick as the run does it, with the decision in the two places strafe.c changes: the facing
 * eased toward its target by a fraction (the game springs it, the same direction), the velocity
 * along the planar angle (the facing for 99999, the run's request). */
static void tick(int active, float camera, float stick, float* facing, float v[2]) {
    /* The stick handler adds the rotations as the game's FastAddRots does: within -pi..pi. */
    float move_yaw = camera + stick;
    if (move_yaw > 3.1415927f) {
        move_yaw -= 6.2831855f;
    } else if (move_yaw < -3.1415927f) {
        move_yaw += 6.2831855f;
    }
    const float target = rac1_strafe_turn_target(active, 0, move_yaw, camera);
    *facing += (target - *facing) * 0.25f;
    float angle = rac1_strafe_planar_angle(active, 99999.0f, move_yaw);
    if (!(angle <= 3.14159274f)) {
        angle = *facing;
    }
    v[0] = cosf(angle);
    v[1] = sinf(angle);
}

int main(void) {
    rac1_strafe_input in = firing();
    CHECK(rac1_strafe_active(1, &in));
    CHECK(!rac1_strafe_active(0, &in)); /* the setting off */
    in.state = RAC1_STRAFE_STATE_RUN_ALT;
    CHECK(rac1_strafe_active(1, &in));
    /* Every gate on its own stops it. */
    for (int gate = 0; gate < 7; ++gate) {
        in = firing();
        switch (gate) {
            case 0: in.state = 0; break;                     /* standing */
            case 1: in.state = 0x81; break;                  /* a state with a strafe of its own */
            case 2: in.frame_mode = 1; break;                /* walking on a wall's own frame */
            case 3: in.slot_state = 3; break;                /* the item released */
            case 4: in.slot_item = RAC1_STRAFE_WRENCH; in.slot_button = in.held = 0x80; break;
            case 5: in.held = 0x40; break;                   /* jumping, not firing */
            case 6: in.slot_button = 0; break;               /* nothing in hand */
        }
        CHECK(!rac1_strafe_active(1, &in));
    }
    /* Off, the game's own values pass through untouched. */
    CHECK(rac1_strafe_turn_target(0, 0, 1.25f, -2.0f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(0, 99999.0f, 1.25f) == 99999.0f);
    CHECK(rac1_strafe_planar_angle(0, 0.5f, 1.25f) == 0.5f);
    /* On: the facing's target is the camera's (turns of mode 0 only), the facing request the move
     * yaw, an angle the caller chose itself kept. */
    CHECK(rac1_strafe_turn_target(1, 0, 1.25f, -2.0f) == -2.0f);
    CHECK(rac1_strafe_turn_target(1, 1, 1.25f, -2.0f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(1, 99999.0f, 1.25f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(1, 3.14159274f, 1.25f) == 3.14159274f);
    CHECK(rac1_strafe_planar_angle(1, 0.5f, 1.25f) == 0.5f);
    /* Stick right (a quarter turn clockwise of the camera's forward) for a while: strafing, he
     * ends facing the camera's way and moving at right angles to it; the game's way, he turns
     * round to face where he goes. */
    const float camera = 0.5f;
    const float right = -1.5707964f;
    float facing = camera, v[2];
    for (int i = 0; i < 60; ++i) {
        tick(1, camera, right, &facing, v);
    }
    CHECK(fabsf(facing - camera) < 1e-4f);
    CHECK(fabsf(v[0] * cosf(camera) + v[1] * sinf(camera)) < 1e-4f); /* no forward part */
    CHECK(fabsf(v[0] - cosf(camera + right)) < 1e-4f && fabsf(v[1] - sinf(camera + right)) < 1e-4f);
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(0, camera, right, &facing, v);
    }
    CHECK(fabsf(facing - (camera + right)) < 1e-4f);
    /* Back: still facing the camera's way, moving toward it. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(1, camera, 3.1415927f, &facing, v);
    }
    CHECK(fabsf(facing - camera) < 1e-4f && v[0] * cosf(camera) + v[1] * sinf(camera) < -0.999f);
    puts("hero strafe: gates, pass-through with the option off, facing and velocity passed");
    return 0;
}
