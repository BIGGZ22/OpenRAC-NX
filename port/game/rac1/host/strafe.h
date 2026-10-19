/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank: the strafe decision (settings "strafe", strafe.c), apart from game memory so
 * that a test can run it (port/tests/hero_strafe_test.c).
 *
 * How the game turns the stick into a heading. Each tick the hero's state code calls the stick
 * handler (Hero_SetHeadingFromStick, func_L00_00211F80), which sets the hero's move yaw (+0x180) to
 * the stick's angle plus the camera's yaw (D_L00_00166ED8): where the stick points on the screen.
 * The state then eases the hero's facing (rot.z, +0x98) toward the move yaw (Hero_TurnToHeading,
 * func_L00_002124E8), and the run builds his velocity along his facing (Hero_SetPlanarVelocity,
 * func_L00_00212790, given an angle above pi: "the hero's own"), so he turns first and runs where
 * he faces. Two places in the game do otherwise, and the strafe is built from them:
 *
 * - the run's start from a standstill with the stick pushed all the way (+0x3B8) asks for the
 *   velocity along the move yaw instead of the facing: he moves where the stick points before he
 *   has turned;
 * - in state 0x81, while L2 or R2 is held, the state puts the camera's yaw in the move yaw around
 *   its call of Hero_TurnToHeading and puts the stick's back afterwards: he faces the camera while
 *   the stick moves him; the follow camera's per-tick requests (func_L00_002E74B0) then ask for a
 *   strafe camera that slides along with him instead of swinging round to look where he goes.
 *
 * On foot the game gives the shoulder buttons in pairs: L1 or L2 looks in first person (state 1,
 * from standing: held & 5), R1 or R2 crouches (held & 0xA: the crouch, the crouch-jumps, the
 * slide from a run). With the setting on, in play on foot, the game sees L2 and R2 as not held
 * (strafe.c takes them out of the pad's state at each report, as the game's own input locks take
 * buttons out), so L1 still looks and R1 still crouches, and L2 and R2 strafe. State 0x81 keeps
 * them: it has its strafe of its own on them. Scenes, menus, vendors and vehicles keep them too.
 *
 * Two ways to strafe, the later games':
 *
 * - Hold (Going Commando's): when L2 or R2 goes down the camera swings round, once, behind the
 *   facing he has then, and he keeps that facing while it swings. From then on, as long as it is
 *   held, he faces the camera's yaw as Lock does: the right stick (the game's own camera control)
 *   turns the camera and him with it, and the stick moves him sideways, back and forward along the
 *   move yaw. The swing ends when the camera is behind him, when the right stick is pushed
 *   sideways, or after a while (a wall or a camera volume can keep it from getting there); nothing
 *   pulls the camera back to the facing he had at the press after that.
 * - Lock (Up Your Arsenal's lock-strafe): with a forward-firing weapon ready in hand
 *   (rac1_strafe_aimable) he always faces the camera's yaw, which the right stick turns (the game's
 *   own camera control), and the stick moves him; the camera does not swing to follow where he
 *   goes. With anything else in hand, Hold.
 *
 * Either is done in the run (states 2 and 0x73) and the jump (7), on level ground; both also turn
 * him standing (state 0) with the camera (Hold once its swing is done). Everything else is the game's own.
 */
#ifndef OPENRAC_RAC1_STRAFE_H
#define OPENRAC_RAC1_STRAFE_H

#include <stdint.h>

/* The setting (openrac_game_strafe_mode). */
#define RAC1_STRAFE_OFF 0
#define RAC1_STRAFE_HOLD 1
#define RAC1_STRAFE_LOCK 2

/* The pad's button bits as the game keeps them (PadState.held): L2 and R2, the strafe's. */
#define RAC1_PAD_L2 0x1u
#define RAC1_PAD_R2 0x2u
#define RAC1_STRAFE_BUTTONS (RAC1_PAD_L2 | RAC1_PAD_R2)

/* The hero's states the strafe works in: standing, running, running with +0x20A9 set (the same
 * movement code), jumping. */
#define RAC1_STRAFE_STATE_STAND 0x00
#define RAC1_STRAFE_STATE_RUN 0x02
#define RAC1_STRAFE_STATE_RUN_ALT 0x73
#define RAC1_STRAFE_STATE_JUMP 0x07
/* The state with a strafe of its own on L2 and R2: left as the game has it. */
#define RAC1_STRAFE_STATE_GAME 0x81
/* Slot 0 ready (HeroItemSlot.state). */
#define RAC1_STRAFE_SLOT_READY 2

/* What a strafe does this tick. */
#define RAC1_STRAFE_NONE 0
#define RAC1_STRAFE_HELD 1   /* L2 or R2 held: the swing behind his facing, then the camera's yaw */
#define RAC1_STRAFE_LOCKED 2 /* facing the camera's yaw */

/* Hold's swing ends with the camera's yaw this near the latched facing (radians), the right stick
 * pushed this far sideways (the game's own test for the stick moved, CameraType0_HeroTweaks), or
 * after this many ticks. */
#define RAC1_STRAFE_SWING_DONE 0.05f
#define RAC1_STRAFE_SWING_STICK 0.3f
#define RAC1_STRAFE_SWING_TICKS 120

/* What the decision reads of the hero and the pad. */
typedef struct rac1_strafe_input {
    int playing;      /* a level loaded and its game mode 0 (D_L00_0015F6A8: not a scene, a menu, a vendor) */
    int state;        /* hero +0x2084 */
    int frame_mode;   /* hero +0x20B3: 0 on level ground; others turn the hero's own frame */
    int vehicle_mode; /* hero +0x20A4: 0 on foot */
    int slot_state;   /* hero +0x1090 + 0x24: slot 0's state */
    int slot_item;    /* hero +0x1090 + 0x28: slot 0's item */
    uint32_t held;    /* the pad's buttons as the player holds them (+0x1A0, L2 and R2 from before strafe.c takes them) */
    float facing;     /* hero +0x98 */
    float camera;     /* the camera's yaw (D_L00_00166ED8) */
    float look_x;     /* the right stick sideways (PadState.analog[0], +0x100) */
} rac1_strafe_input;

/* What Hold remembers from tick to tick. */
typedef struct rac1_strafe_hold {
    int latched;  /* L2 or R2 held since it went down on foot */
    int swinging; /* the camera still swinging round behind yaw */
    int ticks;    /* ticks of the swing so far */
    float yaw;    /* the facing when it went down */
} rac1_strafe_hold;

/* b - a, within -pi..pi. */
static inline float rac1_strafe_angle_diff(float a, float b) {
    float d = b - a;
    while (d > 3.14159274f) {
        d -= 6.28318548f;
    }
    while (d < -3.14159274f) {
        d += 6.28318548f;
    }
    return d;
}

/* The items that fire forward or at what the hero faces, Lock's weapons (slot 0's item ids):
 * the Suck Cannon, the Bomb Glove, the Devastator, the Blaster, the Pyrocitor, the Tesla Claw, the
 * Morph-o-Ray and the R.Y.N.O. Not the wrench (8), nor what does not fire ahead: the Swingshot,
 * the Visibomb (a guided rocket seen from it), the Taunter, the Mine Glove, the Walloper (a
 * punch), the Glove of Doom (its agents find their own way), the Hydrodisplacer, the Drone Device,
 * the Decoy Glove and the gadgets. */
static inline int rac1_strafe_aimable(int item) {
    switch (item) {
        case 0x09: /* Suck Cannon */
        case 0x0A: /* Bomb Glove */
        case 0x0B: /* Devastator */
        case 0x0F: /* Blaster */
        case 0x10: /* Pyrocitor */
        case 0x13: /* Tesla Claw */
        case 0x15: /* Morph-o-Ray */
        case 0x17: /* R.Y.N.O. */
            return 1;
        default:
            return 0;
    }
}

/* In play on foot, in a state the setting changes L2 and R2 in. */
static inline int rac1_strafe_on_foot(const rac1_strafe_input* in) {
    return in->playing && in->vehicle_mode == 0 && in->state != RAC1_STRAFE_STATE_GAME;
}

/* Whether the game sees L2 and R2 as not held this tick: the setting on, in play on foot. */
static inline int rac1_strafe_takes_buttons(int mode, const rac1_strafe_input* in) {
    return mode != RAC1_STRAFE_OFF && rac1_strafe_on_foot(in);
}

/* Hold's memory, once a tick at the pad's report: L2 or R2 going down on foot latches the
 * facing and starts the swing; the swing ends with the camera behind that facing, the right stick
 * pushed sideways or the ticks run out; letting both go, or the setting off, forgets it all. */
static inline void rac1_strafe_hold_update(int mode, const rac1_strafe_input* in, rac1_strafe_hold* hold) {
    if (mode == RAC1_STRAFE_OFF || (in->held & RAC1_STRAFE_BUTTONS) == 0) {
        hold->latched = 0;
        hold->swinging = 0;
        return;
    }
    if (!hold->latched) {
        if (rac1_strafe_on_foot(in)) {
            hold->latched = 1;
            hold->swinging = 1;
            hold->ticks = 0;
            hold->yaw = in->facing;
        }
        return;
    }
    if (hold->swinging) {
        const float d = rac1_strafe_angle_diff(in->camera, hold->yaw);
        const float x = in->look_x < 0.0f ? -in->look_x : in->look_x;
        if ((d < RAC1_STRAFE_SWING_DONE && d > -RAC1_STRAFE_SWING_DONE) || x >= RAC1_STRAFE_SWING_STICK
            || ++hold->ticks >= RAC1_STRAFE_SWING_TICKS) {
            hold->swinging = 0;
        }
    }
}

/* What the strafe does this tick (RAC1_STRAFE_NONE, _HELD, _LOCKED). */
static inline int rac1_strafe_kind(int mode, const rac1_strafe_input* in, const rac1_strafe_hold* hold) {
    if (mode == RAC1_STRAFE_OFF || !in->playing || in->vehicle_mode != 0 || in->frame_mode != 0) {
        return RAC1_STRAFE_NONE;
    }
    if (in->state != RAC1_STRAFE_STATE_STAND && in->state != RAC1_STRAFE_STATE_RUN && in->state != RAC1_STRAFE_STATE_RUN_ALT
        && in->state != RAC1_STRAFE_STATE_JUMP) {
        return RAC1_STRAFE_NONE;
    }
    if (mode == RAC1_STRAFE_LOCK && in->slot_state == RAC1_STRAFE_SLOT_READY && rac1_strafe_aimable(in->slot_item)) {
        return RAC1_STRAFE_LOCKED;
    }
    return hold->latched ? RAC1_STRAFE_HELD : RAC1_STRAFE_NONE;
}

/* Whether the camera is asked this tick to swing round behind Hold's latched facing. */
static inline int rac1_strafe_swings_camera(int kind, const rac1_strafe_hold* hold) {
    return kind == RAC1_STRAFE_HELD && hold->swinging;
}

/* The yaw Hero_TurnToHeading eases the facing toward: while strafing (its first argument 0, the
 * turns that follow the move yaw) the latched yaw while Hold's swing lasts, the camera's after it
 * and in Lock; else the move yaw, as the game has it. */
static inline float rac1_strafe_turn_target(int kind, int mode, float move_yaw, float camera_yaw,
                                            const rac1_strafe_hold* hold) {
    if (mode != 0 || kind == RAC1_STRAFE_NONE) {
        return move_yaw;
    }
    return rac1_strafe_swings_camera(kind, hold) ? hold->yaw : camera_yaw;
}

/* The angle Hero_SetPlanarVelocity builds the velocity along: while strafing, a request for the
 * facing (an angle above pi) is the move yaw instead; an angle of the caller's own is kept. */
static inline float rac1_strafe_planar_angle(int kind, float requested, float move_yaw) {
    return kind != RAC1_STRAFE_NONE && !(requested <= 3.14159274f) ? move_yaw : requested;
}

/* Whether strafe.c turns him toward the camera itself early in the hero's update: standing, where
 * the game's standing state does not turn him, in Lock and in Hold once its swing is done. */
static inline int rac1_strafe_turns_standing(int kind, int state, const rac1_strafe_hold* hold) {
    return state == RAC1_STRAFE_STATE_STAND
           && (kind == RAC1_STRAFE_LOCKED || (kind == RAC1_STRAFE_HELD && !hold->swinging));
}

#endif /* OPENRAC_RAC1_STRAFE_H */
