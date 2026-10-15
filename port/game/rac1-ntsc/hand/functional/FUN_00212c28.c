/* Carried from the PAL decompilation's nonmatching/game/func_00213A78.c (games/rac1) through the version
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
/* The camera-environment region a point is in (effects.c's caller reads D_0019ADC0[*index]): walks the
   region spheres D_0019ABC0 (xyz centre, w = radius squared), entries 0..D_0015FB44 inclusive (the
   hand-written loop tests the entry at the count too), for the first whose (dx*dx + dy*dy) - w is
   negative (x and y only). The point (x, y, z, 1) goes through that region's 4x4 matrix
   (D_0019ADC0 + index * 0x80: rows r0..r3, p = r0 x + r1 y + r2 z + r3); unless vclipw finds any of
   p.xyz outside [-1, 1], *index = the region, *blend = 0.5 * (p.x + 1) and the result is 1; else 0. */
extern float D_0019ACC0_f[] __asm__("D_0019ABC0");
extern float D_0019AEC0_f[] __asm__("D_0019ADC0");
extern int D_0015FC04_i[] __asm__("D_0015FB44");

int FUN_00212c28(float *pos, float *blend, int *index) {
    float *s = D_0019ACC0_f;
    float *end = D_0019ACC0_f + D_0015FC04_i[0] * 4;
    float *m;
    float p[3];
    float dx, dy;
    int i = -1;
    int k;

    for (;;) {
        dx = pos[0] - s[0];
        dy = pos[1] - s[1];
        i++;
        if ((dx * dx + dy * dy) - s[3] < 0.0f) {
            break;
        }
        if (s == end) {
            return 0;
        }
        s += 4;
    }
    m = D_0019AEC0_f + i * 32;
    for (k = 0; k < 3; k++) {
        p[k] = ((m[k] * pos[0] + m[4 + k] * pos[1]) + m[8 + k] * pos[2]) + m[12 + k];
        if (p[k] > 1.0f || p[k] < -1.0f) {
            return 0;
        }
    }
    *index = i;
    *blend = 0.5f * (p[0] + 1.0f);
    return 1;
}
