#ifndef PAD_H
#define PAD_H

/*
 * The controller state. One definition for every source file, in place of
 * the private copies candidates carried to land without clashing with
 * their files.
 *
 * Adapted from Lombyte (GPL-3.0), the US decompilation:
 * include/rnc/input/pad_state.h at 42adb529, refreshed from the NTSC decomp
 * (the same project, GPL-3.0) at 4e1f255e. Function and data names in
 * the comments are Lombyte's, from the US build. See THIRD_PARTY_NOTICES.md.
 */

#include "common.h"

/*
 * Controller state D_0013CA40 (US D_0013C940), polled each frame by poll_pad_device_state
 * (state machine on profile_state) and filled by process_pad_input;
 * socket set up by init_pads.
 *
 * Button masks: process_pad_input ORs left-stick directions into `held`
 * (0x8000/0x2000 x, 0x1000/0x4000 y) and swaps 0x8000/0x2000 when the
 * mirror flag D_0015EDB4 (US) is set. The *_unmasked copies are taken before the
 * `mode` 1/2 masks and have that swap undone; menus read them.
 */
/* Button bits of held / pressed (DualShock order). */
#define PAD_L2       0x0001
#define PAD_R2       0x0002
#define PAD_L1       0x0004
#define PAD_R1       0x0008
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE   0x0020
#define PAD_CROSS    0x0040
#define PAD_SQUARE   0x0080
#define PAD_SELECT   0x0100
#define PAD_L3       0x0200
#define PAD_R3       0x0400
#define PAD_START    0x0800
#define PAD_UP       0x1000
#define PAD_RIGHT    0x2000
#define PAD_DOWN     0x4000
#define PAD_LEFT     0x8000

struct PadState {
    u8 pad_0[0x100] __attribute__((aligned(16)));
    f32 analog[16];                     /* 0x100: [0..3] stick axes (-1..1), [4..15] pressure (0..1) */
    f32 analog_prev[16];                /* 0x140: copy of analog */
    u8 profile[4];                      /* 0x180: button profile block UpdatePad reads (func_00124DF0) */
    u8 act[4];                          /* 0x184: actuator block UpdatePad reads (func_00125218) */
    u8 unk188;                          /* 0x188 */
    u8 unk189;                          /* 0x189: copied by metal_detector_update on spawn */
    u8 pad_18A[0x2];
    s16 unk18C;                         /* 0x18C: counted down by UpdatePad; on pad 0, nonzero suppresses pressed/released (PAL) */
    s16 hist_idx;                       /* 0x18E: next slot of the 30-poll history; cleared with hist_count by ResetGlobalStateFields */
    s32 hist_count;                     /* 0x190: history entries filled, up to 30 */
    s32 socket;                         /* 0x194: scePad2CreateSocket(&param, &D_0013C940) result, init_pads */
    s32 profile_state;                  /* 0x198: 0 query profile, 1 read pad, 2 profile too long */
    s32 device_state;                   /* 0x19C: scePad2GetState result */
    s32 held;                           /* 0x1A0: buttons down, plus stick directions */
    s32 pressed;                        /* 0x1A4: newly down this frame (~prev_held & held) */
    s32 released;                       /* 0x1A8: ~held & prev_held */
    s32 prev_held;                      /* 0x1AC */
    s32 raw_held;                       /* 0x1B0: buttons only, no stick directions */
    s32 raw_pressed;                    /* 0x1B4: ~prev_held & raw_held; saving_data_menu pad_buttons source */
    s32 raw_released;                   /* 0x1B8: ~held & prev_raw_held; weapon swap tests 0x10 */
    s32 prev_raw_held;                  /* 0x1BC: raw_held of the previous poll */
    u32 held_unmasked;                  /* 0x1C0 */
    u32 pressed_unmasked;               /* 0x1C4: menu handlers test 0xD00 (accept) and 0x10 each frame */
    u32 released_unmasked;              /* 0x1C8 */
    s32 mode;                           /* 0x1CC: 1 masks off 0x5030 and the left stick, 2 keeps only 0x900; reset to 0 */
    s32 no_buttons;                     /* 0x1D0: held == 0 */
    s32 no_direction;                   /* 0x1D4: (held & 0xF000) == 0 */
    s32 stick_moved;                    /* 0x1D8: analog[2] or analog[3] nonzero */
    s32 unk1DC;                         /* 0x1DC: 0x79 if the button profile is all ones, else 0; l01 picks pressure input on 0x79 */
    s32 hist_btn[30];                   /* 0x1E0: pressed, per poll, for the stick flick detector */
    f32 hist_ang[30];                   /* 0x258: stick angle, per poll */
    f32 hist_mag[30];                   /* 0x2D0: stick magnitude, per poll */
    s32 unk348;                         /* 0x348: held, latched by UpdatePad */
    u8 pad_34C;
    u8 rep_delay;                       /* 0x34D: key repeat: frames before the first repeat */
    u8 rep_rate;                        /* 0x34E: frames between repeats */
    u8 rep_timer;                       /* 0x34F */
};

typedef struct PadState PadState;

/*
 * The two button pairs as doublewords: retail reads held/pressed (0x1A0) and
 * held_unmasked/pressed_unmasked (0x1C0) with a single ld in
 * update_debug_camera and func_L02_002D9F08, and the pair comes out as one
 * long. Only a union member of a struct the pad is cast to gives GCC 2.95
 * that load with retail's schedule (a cast of the field's address does not),
 * so those two readers cast the pad to this view.
 */
typedef struct {
    u8 pad_0[0x1A0];
    union { s64 l; s32 i; struct { s32 lo; s32 hi; } w; } held_pressed;   /* 0x1A0: held (lo), pressed (hi) */
    u8 pad_1A8[0x18];
    union { s64 l; s32 i; struct { s32 lo; s32 hi; } w; } unmasked;       /* 0x1C0: held_unmasked (lo), pressed_unmasked (hi) */
} PadDwords;

#endif /* PAD_H */
