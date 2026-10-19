/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Corrected from nonmatching/functional/func_001F9E58.c: retail returns an int (1 when it shortened a, 0 when it did not) and writes out only when it shortens it; the old copy returned void and always wrote out. A zero-length a returns 0 as retail does. Same result for the in-place call in sound.c. */
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): two of the game's small number routines that only its sound
 * code reaches (src/game/sound.c: where a sound is panned to, and whether a wall is
 * between it and the listener), still assembly in the decompilation. Not a PS2 match.
 *
 * Unlike game_fastfunc.c's, these were not written from the routines' instructions:
 * what each does is taken from how the decompilation's matched callers use it
 * (func_0022DB48, func_0022D970) and from ReRAC's port of the same two callers of the
 * US release.
 *
 * Adapted from ReRAC (https://github.com/re-rac/rerac),
 * crates/rc-game/src/audio/voices.rs (pan_degrees, occluded): ISC License, Copyright
 * (c) 2026 ReRAC contributors. NTSC-U 0x22c830 and 0x22c658 are PAL's func_0022DB48
 * and func_0022D970.
 */
extern float openrac_sqrt(float x);

/* out = a, shortened to the length `longest` when it is at least that long, in x, y and z; w is
 * a's. Returns 1 when it shortened a (and stored out), 0 otherwise (nothing stored). The
 * occlusion test's line ends at most 64 units from the listener. */
int func_001F9E58(void *out_, void *a_, float longest) {
    float *out = (float *)out_;
    float *a = (float *)a_;
    float x = a[0];
    float y = a[1];
    float z = a[2];
    unsigned int w = ((unsigned int *)a)[3];
    float length = openrac_sqrt(x * x + y * y + z * z);

    if (length == 0.0f || length < longest) {
        return 0;
    }
    {
        float scale = longest / length;

        out[0] = x * scale;
        out[1] = y * scale;
        out[2] = z * scale;
        ((unsigned int *)out)[3] = w;
    }
    return 1;
}
