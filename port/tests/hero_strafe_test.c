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
    rac1_strafe_input in = {1, RAC1_STRAFE_STATE_RUN, 0, 0, RAC1_STRAFE_SLOT_READY, BLASTER, 0, 0.75f, 0.0f, 0.0f};
    return in;
}

/* One tick as the run does it, with the decision in the two places strafe.c changes: the facing
 * eased toward its target by a fraction (the game springs it, the same direction), the velocity
 * along the planar angle (the facing for 99999, the run's request). */
static void tick(int kind, float camera, float stick, const rac1_strafe_hold* hold, float* facing, float v[2]) {
    /* The stick handler adds the rotations as the game's FastAddRots does: within -pi..pi. */
    float move_yaw = camera + stick;
    if (move_yaw > 3.1415927f) {
        move_yaw -= 6.2831855f;
    } else if (move_yaw < -3.1415927f) {
        move_yaw += 6.2831855f;
    }
    const float target = rac1_strafe_turn_target(kind, 0, move_yaw, camera, hold);
    *facing += rac1_strafe_angle_diff(*facing, target) * 0.25f;
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
    rac1_strafe_hold hold = {0, 0, 0, 0.0f};
    rac1_strafe_input in = running();
    in.held = RAC1_PAD_R2 | CIRCLE;
    rac1_strafe_hold_update(RAC1_STRAFE_OFF, &in, &hold);
    CHECK(!hold.latched);
    CHECK(!rac1_strafe_takes_buttons(RAC1_STRAFE_OFF, &in));
    CHECK(rac1_strafe_kind(RAC1_STRAFE_OFF, &in, &hold) == RAC1_STRAFE_NONE);
    hold.latched = 1; /* even a latch left over gives nothing */
    CHECK(rac1_strafe_kind(RAC1_STRAFE_OFF, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_NONE, RAC1_STRAFE_STATE_STAND, &hold));
    CHECK(!rac1_strafe_swings_camera(RAC1_STRAFE_NONE, &hold));
    /* The game's own values pass through untouched. */
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_NONE, 0, 1.25f, -2.0f, &hold) == 1.25f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_NONE, 99999.0f, 1.25f) == 99999.0f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_NONE, 0.5f, 1.25f) == 0.5f);
}

/* Hold: L2 or R2 going down latches the facing; held, he strafes in the run, the jump and standing;
 * let go, the game's own. */
static void hold_mode(void) {
    const int mode = RAC1_STRAFE_HOLD;
    rac1_strafe_hold hold = {0, 0, 0, 0.0f};
    rac1_strafe_input in = running();
    in.slot_item = WRENCH;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(!hold.latched && rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(rac1_strafe_takes_buttons(mode, &in)); /* on foot: L2 and R2 are the strafe's */
    in.held = RAC1_PAD_R2;
    in.camera = -2.0f; /* the camera elsewhere: the swing starts */
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.latched && hold.swinging && hold.yaw == 0.75f);
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_HELD);
    /* Held: the facing he turns to later is not latched again. */
    in.facing = -1.0f;
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(hold.yaw == 0.75f && hold.swinging);
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
    CHECK(hold.swinging && rac1_strafe_swings_camera(RAC1_STRAFE_HELD, &hold));
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_HELD, RAC1_STRAFE_STATE_STAND, &hold));
    /* Targets: while the swing lasts the latched yaw for turns of mode 0; the move yaw for the
     * velocity. */
    hold.yaw = 0.5f;
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_HELD, 0, 1.25f, -2.0f, &hold) == 0.5f);
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_HELD, 1, 1.25f, -2.0f, &hold) == 1.25f);
    /* The swing done: the camera's, standing too, and no more swing. */
    hold.swinging = 0;
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_HELD, 0, 1.25f, -2.0f, &hold) == -2.0f);
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_HELD, 1, 1.25f, -2.0f, &hold) == 1.25f);
    CHECK(rac1_strafe_turns_standing(RAC1_STRAFE_HELD, RAC1_STRAFE_STATE_STAND, &hold));
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_HELD, RAC1_STRAFE_STATE_RUN, &hold));
    CHECK(!rac1_strafe_swings_camera(RAC1_STRAFE_HELD, &hold));
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_HELD, 99999.0f, 1.25f) == 1.25f);
    CHECK(rac1_strafe_planar_angle(RAC1_STRAFE_HELD, 0.5f, 1.25f) == 0.5f);
}

/* Lock: a forward-firing weapon ready makes him face the camera without a button; anything else in
 * hand is Hold. */
static void lock_mode(void) {
    const int mode = RAC1_STRAFE_LOCK;
    rac1_strafe_hold hold = {0, 0, 0, 0.0f};
    rac1_strafe_input in = running();
    rac1_strafe_hold_update(mode, &in, &hold);
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_LOCKED);
    CHECK(rac1_strafe_takes_buttons(mode, &in));
    in.state = RAC1_STRAFE_STATE_STAND;
    CHECK(rac1_strafe_kind(mode, &in, &hold) == RAC1_STRAFE_LOCKED);
    CHECK(rac1_strafe_turns_standing(RAC1_STRAFE_LOCKED, in.state, &hold));
    CHECK(!rac1_strafe_turns_standing(RAC1_STRAFE_LOCKED, RAC1_STRAFE_STATE_RUN, &hold));
    CHECK(!rac1_strafe_swings_camera(RAC1_STRAFE_LOCKED, &hold));
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
    hold.swinging = 1; /* a swing never matters to Lock */
    hold.yaw = 0.5f;
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_LOCKED, 0, 1.25f, -2.0f, &hold) == -2.0f);
    CHECK(rac1_strafe_turn_target(RAC1_STRAFE_LOCKED, 1, 1.25f, -2.0f, &hold) == 1.25f);
    CHECK(!rac1_strafe_swings_camera(RAC1_STRAFE_LOCKED, &hold));
}

/* Moving: stick right (a quarter turn clockwise of the camera's forward) for a while. */
static void movement(void) {
    const float camera = 0.5f;
    const float right = -1.5707964f;
    const rac1_strafe_hold none = {0, 0, 0, 0.0f};
    const rac1_strafe_hold latched = {1, 1, 0, 1.2f}; /* swinging behind 1.2 */
    float facing, v[2];
    /* Lock: he ends facing the camera's way and moving at right angles to it. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_LOCKED, camera, right, &none, &facing, v);
    }
    CHECK(fabsf(facing - camera) < 1e-4f);
    CHECK(fabsf(v[0] * cosf(camera) + v[1] * sinf(camera)) < 1e-4f); /* no forward part */
    CHECK(fabsf(v[0] - cosf(camera + right)) < 1e-4f && fabsf(v[1] - sinf(camera + right)) < 1e-4f);
    /* Hold, swinging behind 1.2 with the camera still elsewhere: he keeps 1.2 and moves where the
     * stick says. */
    facing = 1.2f;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_HELD, camera, right, &latched, &facing, v);
    }
    CHECK(fabsf(facing - 1.2f) < 1e-4f);
    CHECK(fabsf(v[0] - cosf(camera + right)) < 1e-4f && fabsf(v[1] - sinf(camera + right)) < 1e-4f);
    /* The game's way: he turns round to face where he goes. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_NONE, camera, right, &none, &facing, v);
    }
    CHECK(fabsf(facing - (camera + right)) < 1e-4f);
    /* Back: still facing the camera's way, moving toward it. */
    facing = camera;
    for (int i = 0; i < 60; ++i) {
        tick(RAC1_STRAFE_LOCKED, camera, 3.1415927f, &none, &facing, v);
    }
    CHECK(fabsf(facing - camera) < 1e-4f && v[0] * cosf(camera) + v[1] * sinf(camera) < -0.999f);
}

/* The camera's yaw over a tick as the follow camera turns it: the right stick's turn when it is
 * pushed (the game's), else Hold's swing toward the latched facing while strafe.c asks for it,
 * else left where it is (the strafe camera slides along with him, it does not turn). */
static float camera_tick(float camera, float look_x, int kind, const rac1_strafe_hold* hold) {
    if (look_x != 0.0f) {
        camera += look_x * 0.05f;
    } else if (rac1_strafe_swings_camera(kind, hold)) {
        camera += rac1_strafe_angle_diff(camera, hold->yaw) * 0.15f;
    }
    return rac1_strafe_angle_diff(0.0f, camera);
}

/* The whole of Hold, a tick at a time: the pad's report (Hold's memory), the camera, the run. */
typedef struct hold_play {
    rac1_strafe_hold hold;
    rac1_strafe_input in;
    float v[2];
    int kind;
} hold_play;

static void hold_tick(hold_play* p, uint32_t held, float look_x, float stick) {
    p->in.held = held;
    p->in.look_x = look_x;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &p->in, &p->hold);
    p->kind = rac1_strafe_kind(RAC1_STRAFE_HOLD, &p->in, &p->hold);
    p->in.camera = camera_tick(p->in.camera, look_x, p->kind, &p->hold);
    tick(p->kind, p->in.camera, stick, &p->hold, &p->in.facing, p->v);
}

/* Hold as it plays: the swing once, then the right stick turns the camera and him with it, nothing
 * pulls the camera back, and letting go leaves the camera where it is. */
static void hold_play_through(void) {
    const float right = -1.5707964f;
    hold_play p = {{0, 0, 0, 0.0f}, running(), {0.0f, 0.0f}, 0};
    p.in.slot_item = WRENCH;
    p.in.facing = 2.0f;
    p.in.camera = -0.5f; /* the camera well off to his side */
    /* Pressed: the camera swings round behind his facing (2.0), which he keeps meanwhile. */
    int ticks = 0;
    hold_tick(&p, RAC1_PAD_R2, 0.0f, right);
    CHECK(p.kind == RAC1_STRAFE_HELD && p.hold.swinging && p.hold.yaw == 2.0f);
    while (p.hold.swinging && ticks < 200) {
        hold_tick(&p, RAC1_PAD_R2, 0.0f, right);
        CHECK(!p.hold.swinging || fabsf(p.in.facing - 2.0f) < 1e-4f);
        ++ticks;
    }
    CHECK(!p.hold.swinging && ticks < RAC1_STRAFE_SWING_TICKS); /* ended by getting there */
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.camera, 2.0f)) < RAC1_STRAFE_SWING_DONE);
    /* Then he faces the camera's yaw, tick by tick. */
    for (int i = 0; i < 30; ++i) {
        hold_tick(&p, RAC1_PAD_R2, 0.0f, right);
    }
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.facing, p.in.camera)) < 1e-4f);
    /* The right stick turns the camera, and his facing follows it. */
    const float before = p.in.camera;
    for (int i = 0; i < 20; ++i) {
        hold_tick(&p, RAC1_PAD_R2, 1.0f, right);
    }
    const float turned = p.in.camera;
    CHECK(fabsf(rac1_strafe_angle_diff(before, turned) - 1.0f) < 1e-4f);
    for (int i = 0; i < 30; ++i) {
        hold_tick(&p, RAC1_PAD_R2, 0.0f, right);
        /* No snap back: the stick let go, the camera stays where it was turned, no swing again. */
        CHECK(p.in.camera == turned && !p.hold.swinging);
    }
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.facing, turned)) < 1e-4f);
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.facing, 2.0f)) > 0.9f); /* not the press-time facing */
    /* The left stick moves him relative to it: right is at right angles, no forward part. */
    CHECK(fabsf(p.v[0] * cosf(turned) + p.v[1] * sinf(turned)) < 1e-4f);
    CHECK(fabsf(p.v[0] - cosf(turned + right)) < 1e-4f && fabsf(p.v[1] - sinf(turned + right)) < 1e-4f);
    /* Back: still facing it, moving toward the camera. */
    for (int i = 0; i < 5; ++i) {
        hold_tick(&p, RAC1_PAD_R2, 0.0f, 3.1415927f);
    }
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.facing, turned)) < 1e-4f);
    CHECK(p.v[0] * cosf(turned) + p.v[1] * sinf(turned) < -0.999f);
    /* Let go: the game's own, and the camera does not move on its account. */
    const float facing = p.in.facing;
    hold_tick(&p, 0, 0.0f, 0.0f);
    CHECK(p.kind == RAC1_STRAFE_NONE && !p.hold.latched && !p.hold.swinging);
    CHECK(p.in.camera == turned);
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.facing, facing)) < 1e-4f); /* stick ahead: he stays */

    /* Pressed again, the right stick pushed during the swing: the swing ends there and then, and
     * he turns with the camera from where the player takes it. */
    p.in.facing = 0.0f;
    p.in.camera = 3.0f;
    hold_tick(&p, RAC1_PAD_L2, 0.0f, 0.0f);
    CHECK(p.hold.swinging);
    hold_tick(&p, RAC1_PAD_L2, 0.5f, 0.0f);
    CHECK(!p.hold.swinging);
    const float taken = p.in.camera;
    for (int i = 0; i < 60; ++i) {
        hold_tick(&p, RAC1_PAD_L2, 0.0f, 0.0f);
        CHECK(p.in.camera == taken);
    }
    CHECK(fabsf(rac1_strafe_angle_diff(p.in.facing, taken)) < 1e-4f);

    /* A camera that cannot get there (a wall): the swing gives up after its ticks. */
    p.in.held = 0;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &p.in, &p.hold);
    p.in.facing = 1.0f;
    p.in.camera = -1.0f;
    p.in.held = RAC1_PAD_R2;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &p.in, &p.hold);
    CHECK(p.hold.swinging);
    for (int i = 0; i < RAC1_STRAFE_SWING_TICKS; ++i) {
        rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &p.in, &p.hold);
    }
    CHECK(!p.hold.swinging && p.hold.latched);
}

/* Lock as it plays: the right stick's turn stays, nothing swings the camera back. */
static void lock_play_through(void) {
    rac1_strafe_hold hold = {0, 0, 0, 0.0f};
    rac1_strafe_input in = running();
    in.camera = 0.3f;
    float facing = 0.3f, v[2];
    for (int i = 0; i < 20; ++i) {
        rac1_strafe_hold_update(RAC1_STRAFE_LOCK, &in, &hold);
        const int kind = rac1_strafe_kind(RAC1_STRAFE_LOCK, &in, &hold);
        CHECK(kind == RAC1_STRAFE_LOCKED);
        in.camera = camera_tick(in.camera, -1.0f, kind, &hold);
        tick(kind, in.camera, 0.0f, &hold, &facing, v);
    }
    const float turned = in.camera;
    for (int i = 0; i < 40; ++i) {
        rac1_strafe_hold_update(RAC1_STRAFE_LOCK, &in, &hold);
        const int kind = rac1_strafe_kind(RAC1_STRAFE_LOCK, &in, &hold);
        in.camera = camera_tick(in.camera, 0.0f, kind, &hold);
        tick(kind, in.camera, 0.0f, &hold, &facing, v);
        CHECK(in.camera == turned);
    }
    CHECK(fabsf(rac1_strafe_angle_diff(facing, turned)) < 1e-4f);
}

/* The backpedal: when it plays the walk or the run backwards, and the step back itself. */
static void backpedal(void) {
    const float deg = 0.017453292f;
    const int run = RAC1_STRAFE_STATE_RUN;
    /* Strafing in the run: straight back and up to 110 degrees off, the edges with their hysteresis. */
    CHECK(rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 0.5f, 0.5f + 3.1415927f, 0));
    CHECK(rac1_strafe_backpedal(RAC1_STRAFE_LOCKED, RAC1_STRAFE_STATE_RUN_ALT, 0.5f, 0.5f - 120.0f * deg, 0));
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 0.5f, 0.5f + 105.0f * deg, 0)); /* not yet */
    CHECK(rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 0.5f, 0.5f + 105.0f * deg, 1));  /* kept */
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 0.5f, 0.5f + 95.0f * deg, 1));  /* left */
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 0.5f, 0.5f - 90.0f * deg, 0));  /* sideways */
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 0.5f, 0.5f, 1));                /* forward */
    /* Across the wrap of the angles: facing 170, moving at -30 (160 off). */
    CHECK(rac1_strafe_backpedal(RAC1_STRAFE_HELD, run, 170.0f * deg, -30.0f * deg, 0));
    /* Not strafing, or not in the run (standing, the jump): the game's. */
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_NONE, run, 0.5f, 0.5f + 3.1415927f, 1));
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_HELD, RAC1_STRAFE_STATE_JUMP, 0.5f, 0.5f + 3.1415927f, 0));
    CHECK(!rac1_strafe_backpedal(RAC1_STRAFE_HELD, RAC1_STRAFE_STATE_STAND, 0.5f, 0.5f + 3.1415927f, 0));

    /* The step back, a sequence of 4 keys at rates 0.5, 0.25, 0.5, 1. */
    const float rates[4] = {0.5f, 0.25f, 0.5f, 1.0f};
    rac1_strafe_anim a = {2, 3, 0.6f, 0.5f};
    rac1_strafe_anim_back(&a, 0.25f, 4, rates); /* within the interval */
    CHECK(a.key == 2 && a.next_key == 3 && fabsf(a.fraction - 0.35f) < 1e-6f && a.rate == 0.5f);
    rac1_strafe_anim_back(&a, 0.45f, 4, rates); /* 0.1 below: 0.2 ticks into key 1's interval */
    CHECK(a.key == 1 && a.next_key == 2 && a.rate == 0.25f && fabsf(a.fraction - (1.0f - 0.2f * 0.25f)) < 1e-6f);
    /* Back past key 0: the last key and key 0, its rate. */
    a = (rac1_strafe_anim){0, 1, 0.1f, 0.5f};
    rac1_strafe_anim_back(&a, 0.2f, 4, rates);
    CHECK(a.key == 3 && a.next_key == 0 && a.rate == 1.0f && fabsf(a.fraction - (1.0f - 0.2f)) < 1e-6f);
    /* Several keys in one tick. */
    a = (rac1_strafe_anim){3, 0, 0.5f, 1.0f};
    rac1_strafe_anim_back(&a, 1.0f, 4, rates); /* 0.5 ticks over: key 2 (rate 0.5) takes 0.25 of it */
    CHECK(a.key == 2 && a.next_key == 3 && fabsf(a.fraction - 0.75f) < 1e-6f);
    /* Never left where the advance would round it up to the next key. */
    a = (rac1_strafe_anim){2, 3, 0.001f, 0.5f};
    rac1_strafe_anim_back(&a, 0.002f, 4, rates);
    CHECK(a.key == 1 && a.fraction <= RAC1_STRAFE_BACK_TOP);
    /* A whole cycle backwards visits every key in reverse and comes back, and a key of no rate or a
     * huge step stops the loop. */
    a = (rac1_strafe_anim){0, 1, 0.5f, 0.5f};
    int seen = 0;
    for (int i = 0; i < 200 && seen < 5; ++i) {
        const int before = a.key;
        rac1_strafe_anim_back(&a, 0.1f, 4, rates);
        if (a.key != before) {
            CHECK(a.key == (before + 3) % 4 && a.next_key == before);
            ++seen;
        }
    }
    CHECK(seen == 5);
    const float stuck[4] = {0.5f, 0.0f, 0.5f, 0.5f};
    a = (rac1_strafe_anim){2, 3, 0.1f, 0.5f};
    rac1_strafe_anim_back(&a, 0.5f, 4, stuck);
    CHECK(a.key == 1 && a.fraction == RAC1_STRAFE_BACK_TOP); /* a key of no rate holds, as forward */
    a = (rac1_strafe_anim){2, 3, 0.1f, 0.5f};
    rac1_strafe_anim_back(&a, 1e30f, 4, rates);
    CHECK(a.fraction >= 0.0f && a.fraction <= RAC1_STRAFE_BACK_TOP);
}

int main(void) {
    items();
    off();
    hold_mode();
    lock_mode();
    movement();
    hold_play_through();
    lock_play_through();
    backpedal();
    puts("hero strafe: weapons, off, hold, lock, the buttons taken, facing, velocity, swing, right stick and backpedal passed");
    return 0;
}
