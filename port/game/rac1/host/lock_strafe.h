/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank: the lock-strafe decision (settings "lock_strafe", strafe.c), apart from game
 * memory so that a test can run it (port/tests/hero_strafe_test.c).
 *
 * How the game turns the stick into a heading. Each tick the hero's state code calls the stick
 * handler (Hero_SetHeadingFromStick, func_L00_00211F80), which sets the hero's move yaw (+0x180) to
 * the stick's angle plus the camera's yaw (D_L00_00166ED8): where the stick points on the screen.
 * The state then eases the hero's facing (rot.z, +0x98) toward the move yaw (Hero_TurnToHeading,
 * func_L00_002124E8), and the run builds his velocity along his facing (Hero_SetPlanarVelocity,
 * func_L00_00212790, given an angle above pi: "the hero's own"), so he turns first and runs where
 * he faces. Two places in the game do otherwise, and lock-strafe is built from them:
 *
 * - the run's start from a standstill with the stick pushed all the way (+0x3B8) asks for the
 *   velocity along the move yaw instead of the facing: he moves where the stick points before he
 *   has turned;
 * - in state 0x81, while L2 or R2 is held, the state puts the camera's yaw in the move yaw around
 *   its call of Hero_TurnToHeading and puts the stick's back afterwards: he faces the camera while
 *   the stick moves him (and in state 1, L1/L2's look, he turns to the camera standing); the
 *   follow camera's per-tick requests (func_L00_002E74B0) then ask for a strafe camera that
 *   slides along with him instead of swinging round to look where he goes.
 *
 * Lock-strafe does all three in the run (states 2 and 0x73) while the player fires the weapon in
 * hand: the facing eases toward the camera's yaw, the velocity follows the stick, the camera stays
 * behind him. Everything else is the game's own.
 */
#ifndef OPENRAC_RAC1_LOCK_STRAFE_H
#define OPENRAC_RAC1_LOCK_STRAFE_H

#include <stdint.h>

/* The hero's run states: running, and running with +0x20A9 set (the same movement code). */
#define RAC1_STRAFE_STATE_RUN 0x02
#define RAC1_STRAFE_STATE_RUN_ALT 0x73
/* Slot 0's item while the wrench is in hand (its button is Square): not a weapon to strafe with. */
#define RAC1_STRAFE_WRENCH 8
/* Slot 0 ready (HeroItemSlot.state). */
#define RAC1_STRAFE_SLOT_READY 2

/* What the decision reads of the hero and the pad. */
typedef struct rac1_strafe_input {
    int state;            /* hero +0x2084 */
    int frame_mode;       /* hero +0x20B3: 0 on level ground; others turn the hero's own frame */
    int slot_state;       /* hero +0x1090 + 0x24: slot 0's state */
    int slot_item;        /* hero +0x1090 + 0x28: slot 0's item */
    uint32_t slot_button; /* hero +0x1090 + 0x10: the buttons that use slot 0's item */
    uint32_t held;        /* the pad's buttons held (+0x1A0) */
} rac1_strafe_input;

/* Whether the hero strafes this tick: the setting on, running on level ground, a weapon (not the
 * wrench) ready in hand and its button held. */
static inline int rac1_strafe_active(int setting, const rac1_strafe_input* in) {
    if (!setting || in->frame_mode != 0) {
        return 0;
    }
    if (in->state != RAC1_STRAFE_STATE_RUN && in->state != RAC1_STRAFE_STATE_RUN_ALT) {
        return 0;
    }
    if (in->slot_state != RAC1_STRAFE_SLOT_READY || in->slot_item == RAC1_STRAFE_WRENCH || in->slot_button == 0) {
        return 0;
    }
    return (in->held & in->slot_button) != 0;
}

/* The yaw Hero_TurnToHeading eases the facing toward: the camera's while strafing (its first
 * argument 0, the turns that follow the move yaw), else the move yaw, as the game has it. */
static inline float rac1_strafe_turn_target(int active, int mode, float move_yaw, float camera_yaw) {
    return active && mode == 0 ? camera_yaw : move_yaw;
}

/* The angle Hero_SetPlanarVelocity builds the velocity along: while strafing, a request for the
 * facing (an angle above pi) is the move yaw instead; an angle of the caller's own is kept. */
static inline float rac1_strafe_planar_angle(int active, float requested, float move_yaw) {
    return active && !(requested <= 3.14159274f) ? move_yaw : requested;
}

#endif /* OPENRAC_RAC1_LOCK_STRAFE_H */
