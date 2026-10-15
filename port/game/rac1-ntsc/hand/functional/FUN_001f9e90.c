/* Carried from the PAL decompilation's nonmatching/game/func_001FA058.c (games/rac1) through the version
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
/*
 * FastArcTan(a, b) = atan2(b, a), on the VU0 polynomial. With the smaller and the
 * larger of |a|, |b|: t = (min - max) / (min + max), the angle is pi/4 + atan(t) with
 * atan(t) = t * P(t^2) (eight coefficients at D_001DE320), and the quadrant comes from
 * the sign bits of a and b and the sign of |a| - |b| (table D_001DE340, index
 * s3 + 2*sign(b) + 4*sign(a)): result = (pi/4 + atan(t)) * mul + add. Zero when both are 0.
 */
extern float D_001DE320[];
extern float D_001DE340[];

float FUN_001f9e90(float a, float b) {
    union { float f; unsigned int u; } ba, bb, bd;
    float fa, fb, mn, mx, t, u, v, w8;
    float p1x, p1y, p1z, p1w, p2x, p2y, p2z, p2w, acc, s, r;
    int s3, sa, sb, k;

    ba.f = a;
    bb.f = b;
    sa = ba.u >> 31;
    sb = bb.u >> 31;
    ba.u &= 0x7FFFFFFFu;
    bb.u &= 0x7FFFFFFFu;
    fa = ba.f;
    fb = bb.f;

    bd.f = fa - fb;
    s3 = bd.u >> 31;
    if (s3) {
        mn = fa;
        mx = fb;
    } else {
        mn = fb;
        mx = fa;
    }
    if (mx <= 0.0f) {
        return 0.0f;
    }

    t = (mn - mx) / (mn + mx);
    k = s3 + 2 * sb + 4 * sa;

    u = t * t;
    v = u * u;
    w8 = v * v;
    p1x = t;
    p1y = t * u;
    p1z = t * v;
    p1w = (t * v) * u;
    p2x = t * w8;
    p2y = (t * u) * w8;
    p2z = (t * v) * w8;
    p2w = ((t * v) * u) * w8;

    p1x = t * D_001DE320[0];
    p1y = p1y * D_001DE320[1];
    p1z = p1z * D_001DE320[2];
    p1w = p1w * D_001DE320[3];
    p2x = p2x * D_001DE320[4];
    p2y = p2y * D_001DE320[5];
    p2z = p2z * D_001DE320[6];
    p2w = p2w * D_001DE320[7];

    acc = p1x + p1y;
    acc = acc + p1z;
    acc = acc + p1w;
    acc = acc + p2x;
    acc = acc + p2y;
    acc = acc + p2z;
    s = acc + p2w;

    r = (0.78539819f + s) * D_001DE340[2 * k];
    return r + D_001DE340[2 * k + 1];
}
