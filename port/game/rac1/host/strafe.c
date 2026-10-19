/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the strafe, the player's option (settings "strafe": off by default, Hold L2-R2,
 * Lock with weapons). strafe.h says how the game turns the stick into a heading, what each mode
 * changes and why L2 and R2. The pad's report, the hero's motion input, two of his routines and
 * one of the follow camera's run under host functions (libraries.tsv, wrap), each calling the
 * game's own: with the option off they are the game's routines exactly; with it on, whenever the
 * hero is not strafing, but for L2 and R2, which the game does not see while he is on foot.
 */
#include "rac1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strafe.h"

#define HERO RAC1_ADDR(0x0013F450u)
#define PAD RAC1_ADDR(0x0013CA40u)
#define LEVEL_MODE OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0015F6A8u))

/* The PadState words with button bits (pad.h: held, pressed, released, raw_held, raw_pressed,
 * raw_released and the unmasked three). */
static const uint32_t k_pad_words[] = {0x1A0u, 0x1A4u, 0x1A8u, 0x1B0u, 0x1B4u, 0x1B8u, 0x1C0u, 0x1C4u, 0x1C8u};
#define PAD_WORDS (sizeof(k_pad_words) / sizeof(k_pad_words[0]))

static uint32_t s_buttons;    /* L2 and R2 as the player holds them, from the last report */
static rac1_strafe_hold s_hold;
static int s_swinging; /* the camera was asked last tick to swing behind Hold's yaw */

static float get_float(gaddr at) {
    float v;
    memcpy(&v, G(at), 4);
    return v;
}

static void put_float(gaddr at, float v) {
    memcpy(G(at), &v, 4);
}

static float camera_yaw(void) {
    return get_float(OPENRAC_LDATA(0, RAC1_LADDR(00, 0x00166ED8u)));
}

static void read_input(rac1_strafe_input* in) {
    in->playing = openrac_game_loaded_overlay() >= 0 && GREF(int32_t, LEVEL_MODE) == 0;
    in->state = GREF(int32_t, HERO + 0x2084u);
    in->frame_mode = GREF(uint8_t, HERO + 0x20B3u);
    in->vehicle_mode = GREF(uint8_t, HERO + 0x20A4u);
    in->slot_state = GREF(int32_t, HERO + 0x1090u + 0x24u);
    in->slot_item = GREF(int32_t, HERO + 0x1090u + 0x28u);
    in->held = (GREF(uint32_t, PAD + 0x1A0u) & ~RAC1_STRAFE_BUTTONS) | s_buttons;
    in->facing = get_float(HERO + 0x98u);
}

/* What the strafe does now (strafe.h). */
static int strafe_kind(void) {
    const int mode = openrac_game_strafe_mode();
    if (mode == RAC1_STRAFE_OFF) {
        return RAC1_STRAFE_NONE;
    }
    rac1_strafe_input in;
    read_input(&in);
    return rac1_strafe_kind(mode, &in, &s_hold);
}

/* OPENRAC_STRAFE_TRACE=1: a line each pad report (docs/port/SETTINGS.md section 7). */
static void trace(int mode) {
    static int on = -1;
    if (on < 0) {
        on = getenv("OPENRAC_STRAFE_TRACE") != NULL;
    }
    if (!on) {
        return;
    }
    rac1_strafe_input in;
    read_input(&in);
    fprintf(stderr,
            "[debug] strafe: mode %d kind %d playing %d state %#x vehicle %d item %#x/%d held %#x seen %#x latched %d/%.3f "
            "facing %.3f move %.3f camera %.3f pos %.3f %.3f\n",
            mode, rac1_strafe_kind(mode, &in, &s_hold), in.playing, (unsigned)in.state, in.vehicle_mode,
            (unsigned)in.slot_item, in.slot_state, (unsigned)in.held, GREF(uint32_t, PAD + 0x1A0u), s_hold.latched,
            s_hold.yaw, in.facing, get_float(HERO + 0x180u), camera_yaw(), get_float(HERO + 0x80u),
            get_float(HERO + 0x84u));
}

/* Hero_TurnToHeading under the camera's or Hold's yaw, as state 0x81 turns him while L2 or R2 is
 * held: that yaw in the move yaw for the call, the stick's put back after it. */
static void turn_toward(float yaw, int mode, float accel, float damping, float limit) {
    const gaddr move_yaw = HERO + 0x180u;
    const float stick = get_float(move_yaw);
    put_float(move_yaw, yaw);
    func_L00_002124E8__game(mode, accel, damping, limit);
    put_float(move_yaw, stick);
}

/* Pad_ProcessReport (func_002181F0): one report into the pad's state, each poll, before the tick
 * reads it. L2 and R2 are kept here as the player holds them; with the setting on, in play, on
 * foot, they are then taken out of the pad's button words (and this poll's history entry), the
 * way the game's own input locks (PadState.mode) take buttons out, so that the game does not look
 * or crouch on them (strafe.h). Hold's memory is updated here, once a tick. */
void func_002181F0(gaddr pad, gaddr report, int size) {
    func_002181F0__game(pad, report, size);
    if (pad != PAD) {
        return;
    }
    const int mode = openrac_game_strafe_mode();
    s_buttons = GREF(uint32_t, PAD + 0x1A0u) & RAC1_STRAFE_BUTTONS;
    rac1_strafe_input in;
    read_input(&in);
    rac1_strafe_hold_update(mode, &in, &s_hold);
    if (rac1_strafe_takes_buttons(mode, &in)) {
        for (unsigned i = 0; i < PAD_WORDS; ++i) {
            GREF(uint32_t, PAD + k_pad_words[i]) &= ~RAC1_STRAFE_BUTTONS;
        }
        const int last = (GREF(int16_t, PAD + 0x18Eu) + 29) % 30; /* hist_btn[hist_idx - 1] */
        GREF(uint32_t, PAD + 0x1E0u + 4u * (unsigned)last) &= ~RAC1_STRAFE_BUTTONS;
    }
    trace(mode);
}

/* Hero_UpdateMotionInput (func_L00_00211D28): the stick read into the hero, early in his update
 * each tick. Lock standing turns him toward the camera first, where the game's standing state does
 * not turn him: state 0x81's turn toward the camera with L2 or R2 held. */
void func_L00_00211D28(void) {
    if (openrac_game_strafe_mode() == RAC1_STRAFE_LOCK) {
        rac1_strafe_input in;
        read_input(&in);
        if (rac1_strafe_turns_standing(rac1_strafe_kind(RAC1_STRAFE_LOCK, &in, &s_hold), in.state)) {
            const float frame = get_float(OPENRAC_DATA(RAC1_ADDR(0x0015EE64u)));
            const float step = get_float(OPENRAC_DATA(RAC1_ADDR(0x0015EE6Cu)));
            turn_toward(camera_yaw(), 0, frame * 0.017f, frame * 0.3f, step * 4.712389f);
        }
    }
    func_L00_00211D28__game();
}

/* Hero_TurnToHeading: eases the facing toward the move yaw (+0x180). While strafing, toward the
 * camera's yaw (Lock) or the one Hold latched. */
void func_L00_002124E8(int mode, float accel, float damping, float limit) {
    const int kind = mode == 0 ? strafe_kind() : RAC1_STRAFE_NONE;
    if (kind == RAC1_STRAFE_NONE) {
        func_L00_002124E8__game(mode, accel, damping, limit);
        return;
    }
    turn_toward(rac1_strafe_turn_target(kind, mode, get_float(HERO + 0x180u), camera_yaw(), s_hold.yaw), mode, accel,
                damping, limit);
}

/* The follow camera's per-tick requests from the hero's state (func_L00_002E74B0, the camera moby's
 * update calls it before it places the eye). In state 0x81 with L2 or R2 held, the game's strafe,
 * it asks for its strafe camera and returns before its own panning and shake: the camera slides
 * along with the hero (+0x116), held behind him by its lateral spring at 0.04 / 0.2, and the pan
 * timers are cleared. While strafing the same is asked for: the game's routine runs, then what
 * its panning part changed is put as that branch leaves it (the pan weight D_L00_0015F040 as it
 * was, the timers 0, the spring and the slide asked for). Hold also swings the camera round behind
 * the latched facing, as the game's own requests to look along a heading do (func_L00_002E9DC8:
 * the turn the right stick gives, which wins whenever the stick is moved). */
void func_L00_002E74B0(gaddr camera) {
    const int kind = strafe_kind();
    const gaddr data = GREF(gaddr, camera + 0x70u);
    if (kind != RAC1_STRAFE_HELD && s_swinging) {
        /* The swing asked for last tick ends: its turn put back to rest (+0x1A8 + 0x1C). */
        s_swinging = 0;
        put_float(data + 0x1C4u, 0.0f);
    }
    if (kind == RAC1_STRAFE_NONE) {
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
    GREF(int16_t, data + 0x12u) = 0;
    GREF(int16_t, data + 0x14u) = 0;
    GREF(int16_t, data + 0x16u) = 0;
    if (kind == RAC1_STRAFE_HELD) {
        /* The heading as the game's camera volumes give theirs: (FastCos, FastSin, 0). */
        GFRAME(dir, 16);
        put_float(dir, func_001F9F90(s_hold.yaw));
        put_float(dir + 4u, func_001F9FA8(s_hold.yaw));
        put_float(dir + 8u, 0.0f);
        put_float(dir + 12u, 0.0f);
        s_swinging = 1;
        func_L00_002E9DC8(dir, 0.20943952f, 0.0f);
    }
}

/* Hero_SetPlanarVelocity: the velocity at the ground speed along an angle, the facing for one
 * above pi. While strafing, along the move yaw instead: where the stick points, as the game's own
 * run start from a standstill asks for. */
void func_L00_00212790(float angle) {
    if (!(angle <= 3.14159274f)) {
        angle = rac1_strafe_planar_angle(strafe_kind(), angle, get_float(HERO + 0x180u));
    }
    func_L00_00212790__game(angle);
}
