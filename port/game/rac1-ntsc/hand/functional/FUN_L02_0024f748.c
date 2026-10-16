/* Carried from the PAL decompilation's nonmatching/functional/func_L02_00250720.c (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* functional: medium. Tile visibility grid: for each cell of a w x h grid (cell step 7 units, data in P) with a nonzero flag byte, tests the cell centre against the camera distance (d^2 < P[3]) and the 4 corners against two view frusta (matrices at D_L02_00167280+0xC0 / +0x100, camera at +0x140); writes 0 (culled), 1 (inside) or 2 (needs clipping) per cell to the scratchpad 0x70002000. VU0 clip flag and sign bit modelled by hand; cannot be run. */
extern float D_L02_00167300_50720[] __asm__("D_L02_00167280");



void FUN_L02_0024f748(char *p) {
    float *T = D_L02_00167300_50720;
    float qx = *(float *)(p + 0x0), qy = *(float *)(p + 0x4), qz = *(float *)(p + 0x8), r2 = *(float *)(p + 0xC);
    float sx = *(float *)(p + 0x10) * 7.0f;
    float sy = *(float *)(p + 0x14) * 7.0f;
    float hx = sx * 0.5f;
    float hy = sy * 0.5f;
    float x25 = qx, x26 = qx + sx, x27 = qx, x28 = qx + sx, x29 = qx + hx;
    float y25 = qy, y26 = qy, y27 = qy + sy, y28 = qy + sy, y29 = qy + hy;
    signed char *flags = *(signed char **)(p + 0x38);
    int cntY = *(unsigned short *)(p + 0x1A);
    int cntX;
    unsigned char *out = (unsigned char *)0x70002000;
    unsigned int c11, c12;
    int res, k, m;
    float dx, dy, dz, d2;
    float cx[4], cy[4], vx, vy, vz, tx, ty, tz, tw, aw;
    float *M;
    unsigned int bits;

    do {
        x25 = qx; x26 = qx + sx; x27 = qx; x28 = qx + sx; x29 = qx + hx;
        cntX = *(unsigned short *)(p + 0x18);
        do {
            signed char f = *flags;
            flags++;
            res = 0;
            if (f != 0) {
                dx = x29 - T[80];
                dy = y29 - T[81];
                dz = qz - T[82];
                d2 = (dx * dx + dy * dy) + dz * dz;
                if (d2 - r2 < 0.0f) {
                    c11 = 0;
                    c12 = 0;
                    cx[0] = x25; cy[0] = y25;
                    cx[1] = x26; cy[1] = y26;
                    cx[2] = x27; cy[2] = y27;
                    cx[3] = x28; cy[3] = y28;
                    for (k = 0; k < 4; k++) {
                        vx = (cx[k] - T[80]) * 1024.0f;
                        vy = (cy[k] - T[81]) * 1024.0f;
                        vz = (qz - T[82]) * 1024.0f;
                        for (m = 0; m < 2; m++) {
                            M = T + (m == 0 ? 48 : 64);
                            tx = ((M[0] * vx + M[4] * vy) + M[8] * vz) + M[12];
                            ty = ((M[1] * vx + M[5] * vy) + M[9] * vz) + M[13];
                            tz = ((M[2] * vx + M[6] * vy) + M[10] * vz) + M[14];
                            tw = ((M[3] * vx + M[7] * vy) + M[11] * vz) + M[15];
                            aw = tw < 0.0f ? -tw : tw;
                            bits = 0;
                            if (tx > aw) bits |= 0x01;
                            if (tx < -aw) bits |= 0x02;
                            if (ty > aw) bits |= 0x04;
                            if (ty < -aw) bits |= 0x08;
                            if (tz > aw) bits |= 0x10;
                            if (tz < -aw) bits |= 0x20;
                            if (m == 0) {
                                c12 = (c12 << 8) | bits;
                            } else {
                                c11 = (c11 << 8) | bits;
                            }
                        }
                    }
                    res = ((c12 & 0x2F2F2F2Fu) != 0) ? 2 : 1;
                    if ((c11 & 0x01010101u) == 0x01010101u ||
                        (c11 & 0x02020202u) == 0x02020202u ||
                        (c11 & 0x04040404u) == 0x04040404u ||
                        (c11 & 0x08080808u) == 0x08080808u ||
                        (c11 & 0x20202020u) == 0x20202020u) {
                        res = 0;
                    }
                }
            }
            *out = (unsigned char)res;
            out++;
            x25 += sx; x26 += sx; x27 += sx; x28 += sx; x29 += sx;
            cntX -= 7;
        } while (cntX > 0);
        y25 += sy; y26 += sy; y27 += sy; y28 += sy; y29 += sy;
        cntY -= 7;
    } while (cntY > 0);
}
