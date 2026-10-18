/* Checked against the US assembly: constants address corrected to 0x160760 (carried 0x1606A0 was wrong).
 * Carried from the PAL decompilation's nonmatching/game/func_001F9FC0.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* FUN_001f9df8: |x| through c0 + c1*a + c2*a^2 + c3*a^3 (the constants at gp-0x64A0 = 0x160760 (gp 0x166C00), read from
   the game's memory), times sqrt(1 - |x|) (VU0 vsqrt, which takes the magnitude), subtracted from pi/2
   and given the sign of x. From the earlier worker's C (build-sn/fill/FUN_001f9df8), which used
   literal constants and sqrtf. */
extern float openrac_sqrt(float);
extern float D_00160760_f[] __asm__("D_00160760");
float FUN_001f9df8(float x) {
    float sign = 1.0f;
    float ax;
    float t;
    float sq;
    float a2;
    float a3;
    float acc;
    float poly;
    union { unsigned int u; float f; } h;

    if (x < 0.0f) {
        sign = -1.0f;
        ax = -x;
    } else {
        ax = x;
    }
    t = 1.0f - ax;
    sq = openrac_sqrt(t < 0.0f ? -t : t);
    a2 = ax * ax;
    a3 = ax * a2;
    acc = D_00160760_f[0] + 0.0f;
    acc = acc + ax * D_00160760_f[1];
    acc = acc + a2 * D_00160760_f[2];
    poly = acc + a3 * D_00160760_f[3];
    h.u = 0x3FC90FDB;
    return sign * (h.f - poly * sq);
}
