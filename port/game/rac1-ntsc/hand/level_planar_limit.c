/* Carried from port/game/rac1-pal/hand/level_planar_limit.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native replacement for handwritten PAL VU routine 001FF5B0 (0x58).
 * The branch tail returns zero without a store; equality scales and
 * returns one. Preserve the integer test of the float difference bits,
 * the separate sqrt/divide operations, and the unmodified z/w lanes.
 * This is a native implementation, not compiler-matching PS2 C. */
extern float openrac_sqrt(float);
int FUN_L00_001ff600(float limit, float *out, float *input) {
    float x = input[0], y = input[1];
    unsigned z = ((unsigned *)input)[2], w = ((unsigned *)input)[3];
    float xx = x * x, yy = y * y;
    float magnitude = 0.0f + openrac_sqrt(xx + yy);
    float difference = limit - magnitude;
    /* qmfc2 reads both x and y into the low 64 bits for the zero test;
     * dsll32 then isolates the difference's x bits for signed bgtz. */
    if ((*(unsigned *)&magnitude == 0 && *(unsigned *)&yy == 0)
        || *(int *)&difference > 0)
        return 0;
    float scale = limit / magnitude;
    out[0] = x * scale;
    out[1] = y * scale;
    ((unsigned *)out)[2] = z;
    ((unsigned *)out)[3] = w;
    return 1;
}
