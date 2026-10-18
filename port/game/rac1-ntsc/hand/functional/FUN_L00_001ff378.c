/* Carried from the PAL decompilation's nonmatching/game/func_L00_001FF328.c (games/rac1) through the version
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
/* functional: high. Checked against the US assembly (config/us/overlays/asm/FUN_L00_001ff378.s): vdiv Q = 1 / s,
 * vmulq.xyz, w copied; the zero case and the overflow case are the VU's clamp to the largest float with the sign
 * of s, so the result is never infinity. Not run on hardware.
 * out.xyz = a.xyz * (1 / s) (VU0 vdiv Q = vf0.w / s, then vmulq.xyz), out.w = a.w; a zero s is
   the VU's division by zero, the largest float. */
void FUN_L00_001ff378(float *out, float *a, float s) {
    union { unsigned int u; float f; } q;
    float x, y, z, w;

    if (s != 0.0f) {
        q.f = 1.0f / s;
        if ((q.u & 0x7F800000) == 0x7F800000) {
            q.u = (q.u & 0x80000000) | 0x7F7FFFFF;
        }
    } else {
        q.u = (*(unsigned int *)&s & 0x80000000) | 0x7F7FFFFF;
    }
    x = a[0] * q.f;
    y = a[1] * q.f;
    z = a[2] * q.f;
    w = a[3];
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}
