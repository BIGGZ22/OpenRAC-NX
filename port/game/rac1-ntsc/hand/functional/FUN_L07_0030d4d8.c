/* Carried from the PAL decompilation's nonmatching/functional/func_L07_0030E8B8.c (games/rac1/pal) through the version
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
/* What the PAL file this was written in declares before it. */
extern char D_L07_00166E40[];
extern int func_001F9688(int);
extern float func_L00_001FF8B0(float, float);
extern float D_0015ED6C MACRO_ADDR;
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_0022DA68(int, int, int);

/* functional: high. The near miss with its missing store restored (+0x26 of the data gets the random value func_001F9688(60) returns, not the constant 0xFA stored to +0x67); the rest differs from retail only in compiler choices (registers, stack slots, scheduling, branch layout of the state chain and of the empty jump-table cases). */
/* Level 07 (Umbris) moby update; m is the moby, d = m+0x78 its data. When m[0x31] is set and the
 * moby is within 27 units of D_L07_00166E40 (the position used for the hero elsewhere in this file)
 * it calls func_L00_0025A120 and sets m[0x7F] = 0x15. Hits are taken through func_L00_0025A478 on
 * the health float d+0x20 (results 1 and 2 zero it); hits whose source moby has class 0x372 are
 * ignored. If health <= the hit's value at s+0x2C (s from func_L00_0025A420) it dies (func_L00_00257470, func_L00_0025E450,
 * effects 0x621 and 0x622 at its position, func_0020C828) and returns; otherwise it loses that much,
 * sets d+0x67 = 0xFA and d+0x26 = func_001F9688(60) and calls func_L00_0025D458. Then a state
 * machine on m[0x20]: 0 arms the timer d+0xB4 from random values and goes to 1; 1 waits for the
 * timer, rearms it, picks a heading d+0xBC (func_00213308) and goes to 2; 2, each time the short
 * timer d+0xB8 runs out and the hero is near (90 or 40 units, alternating on d+0xC4 bit 0), picks
 * of its three points (func_L00_0024F7C8) the one closest in angle to the heading and fires
 * func_L07_0030BA90 from it with a computed velocity, counting shots in d+0xBA; back to 1 on the
 * timer. Outside state 0 it turns d+0xC0 and applies it with func_L00_001FFF28(d+0x80, 2, ...). */
extern float D_L07_0015F580[] MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern int D_L07_0015F5CC MACRO_ADDR;
extern void func_001F99F8(float *);
extern void func_L00_0025A120(void *);
extern float func_001F9B48(void *, void *);
extern char *func_L00_0025A420(void *, int, int);
extern int func_L00_0025A478(void *, void *, void *, int, int *, float *, int, int);
extern void func_L00_00257470(void *, int, int);
extern void func_L00_0025D538(void *, void *);
extern void func_L00_0025FCB8(void *, void *, float);
extern int func_L00_00257B90(int, int);
extern void func_0020CB10(char *, int, void *);
extern void func_0020C828(void *);
extern void func_L00_0025D458(void *, short *);
extern void func_001F9D20(void *, void *, void *);
extern void func_001FA050(void *, void *);
extern float func_001FA580(float, float);
extern float func_001FA5C8(float, float);
extern float func_001FA688(float, float);
extern float func_002132A8(float, float);
extern float func_00213308(void);
extern int func_001F9770(void *);
extern float func_001F9B80(void *, void *);
extern void func_L00_001FFF28(void *, int, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern float func_L00_0025ABF0(void *, void *, void *, float, float);
extern void func_L00_0025F4A8_alt(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025E450");
extern void *func_L00_00263FD8(char *, int, float *, void *, int, int, float, float *, float *, float *);

void FUN_L07_0030d4d8(char *m) {
    char *d;
    char *s16;
    char *t;
    int ia;
    int r;
    int state;
    float lim;
    float v20[4];
    float v30[4];
    float v100[4];
    float v80[4];
    float v90[4];
    float vA0[4];
    float vB0[4];
    float vF0[4];

    if (m == 0) {
        return;
    }
    d = *(char **)(m + 0x78);
    if (((unsigned char *)m)[0x31] != 0) {
        if (func_001F9B48(m + 0x10, D_L07_00166E40) < 27.0f) {
            func_L00_0025A120(m);
            m[0x7F] = 0x15;
        }
    }
    if (d == 0) {
        return;
    }
    s16 = func_L00_0025A420(m, 0x230000, 0);
    t = d + 0x20;
    r = func_L00_0025A478(m, s16, t, 0, &ia, 0, 0, 4);
    switch (r) {
    case 0:
        break;
    case 1:
        *(int *)(d + 0x20) = 0;
        break;
    case 2:
        *(int *)(d + 0x20) = 0;
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        break;
    case 10:
        break;
    case 11:
        break;
    }

    if (ia >= 2) {
        char *o;
        if (s16 == 0) {
            lim = *(float *)(s16 + 0x2C);
        } else {
            o = *(char **)(s16 + 0x20);
            if (o == 0) {
                lim = *(float *)(s16 + 0x2C);
            } else {
                if (*(short *)(o + 0xA6) == 0x372) {
                    goto eb20;
                }
                lim = *(float *)(s16 + 0x2C);
            }
        }
        if (*(float *)t <= lim) {
            func_L00_00257470(m, 0, -1);
            *(float *)t = 0.0f;
            func_001F99F8(v20);
            func_L00_0025F4A8_alt(m, v20, 0, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 1, 15.0f, 1, 1, -1, 0);
            func_L00_00263FD8(m, 0x621, (float *)(m + 0x10), m + 0x40, 0, 0, 0.0f, D_L07_0015F580, D_L07_0015F580, D_L07_0015F580);
            func_L00_00263FD8(m, 0x622, (float *)(m + 0x10), m + 0x40, 0, 0, 0.0f, D_L07_0015F580, D_L07_0015F580, D_L07_0015F580);
            func_0020C828(m);
            return;
        }
        *(float *)t = *(float *)t - lim;
        *(unsigned char *)(d + 0x67) = 0xFA;
        *(short *)(d + 0x26) = func_001F9688(60);
        func_L00_0025D458(m, (short *)(d + 0x60));
    }
eb20:
    m[0xA4] = 0xFF;
    func_L00_0025D538(m, d + 0x60);
    func_L00_0025FCB8(m, v30, 20.0f);
    state = ((unsigned char *)m)[0x20];
    if (state == 1) {
        if (D_L07_0015F5CC >= *(int *)(d + 0xB4)) {
            int r9 = func_001F9688(0xB4);
            *(short *)(d + 0xB8) = 0;
            *(int *)(d + 0xB4) = D_L07_0015F5CC + r9;
            *(float *)(d + 0xBC) = func_00213308();
            *(short *)(d + 0xBA) = 0;
            m[0x20] = 2;
        }
    } else if (state >= 2) {
        if (state == 2) {
            int i;
            if (func_001F9770(d + 0xB8) != 0) {
                *(short *)(d + 0xB8) = func_001F9688(7);
                if (((unsigned char *)m)[0x31] == 0) {
                    if (!(func_001F9B80(m + 0x10, D_L07_00166E40) < 90.0f && (*(int *)(d + 0xC4) & 1) != 0)) {
                        if (!(func_001F9B80(m + 0x10, D_L07_00166E40) < 40.0f)) {
                            goto eeE4;
                        }
                    }
                }
                *(int *)(d + 0xC4) ^= 1;
                if (func_001F9B80(m + 0x10, v30) < 20.0f) {
                    *(float *)(d + 0xBC) = func_L00_001FF8B0(v30[0] - *(float *)(m + 0x10), v30[1] - *(float *)(m + 0x14));
                }
                v20[0] = func_002132A8(3.0f, 11.0f);
                v20[1] = func_002132A8(-2.0f, 2.0f);
                v20[2] = 0.0f;
                vF0[3] = -1.0f;
                {
                    float best = 3.1415927f;
                    float h;
                    for (i = 1; i < 4; i++) {
                        float f;
                        float g;
                        func_L00_0024F7C8(m, i, v100);
                        f = func_L00_001FF8B0(v100[0] - *(float *)(m + 0x10), v100[1] - *(float *)(m + 0x14));
                        g = func_001FA5C8(*(float *)(d + 0xBC), D_0015ED6C * 17.80235863f);
                        h = func_001FA688(f, g);
                        if (h < best) {
                            qcopy(vF0, v100);
                            best = h;
                        }
                    }
                    qcopy(vA0, vF0);
                }
                qzero(v80);
                v80[2] = *(float *)(d + 0xBC);
                func_001FA050(vB0, v80);
                func_001F9D20(v20, v20, vB0);
                func_L00_001FF500(v90, v20, D_0015ED6C * 7.0f);
                func_001F9A10(v20, v20, m + 0x10);
                v90[2] = func_L00_0025ABF0(vA0, v20, 0, D_0015ED6C * 7.0f, -(D_0015ED70 * 9.8f));
                func_001F9A10(vA0, vA0, v90);
                func_L07_0030BA90(vA0, v90, (int)m, *(short *)(d + 0xBA));
                *(short *)(d + 0xBA) = *(short *)(d + 0xBA) + 1;
            }
        eeE4:
            if (D_L07_0015F5CC >= *(int *)(d + 0xB4)) {
                int r9 = func_001F9688(0x78);
                *(int *)(d + 0xB4) = D_L07_0015F5CC + r9;
                m[0x20] = 1;
            }
        }
    } else {
        if (state == 0) {
            int r1 = func_001F9688(*(int *)(d + 0xB0));
            int r2;
            int r3;
            r2 = func_L00_00257B90(0, 10);
            r3 = func_001F9688(r2);
            *(int *)(d + 0xB4) = D_L07_0015F5CC + r1 + r3;
            func_0020CB10(m, 0, d + 0x70);
            *(int *)(d + 0xC0) = 0;
            func_0022DA68(0, 4, (int)m);
            m[0x20] = 1;
            *(int *)(d + 0xC4) = 0;
        }
    }

    if (((unsigned char *)m)[0x20] != 0) {
        float f = D_0015ED6C * 17.80235863f;
        float g = func_001FA580(*(float *)(d + 0xC0), f);
        *(float *)(d + 0xC0) = g;
        func_L00_001FFF28(d + 0x80, 2, g);
    }
}
