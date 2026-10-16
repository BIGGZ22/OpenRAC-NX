/* Carried from the PAL decompilation's src/overlays/l07_umbris/vendor_002CE470.c (matched C there) (games/rac1) through the version
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
extern int func_001F9770(void *);
extern float func_001F9B80(void *, void *);
extern int func_001F9688(int);
extern int func_0022DA68(int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern float D_0015ED6C MACRO_ADDR;
extern float func_L00_001FF8B0(float, float);
extern void func_L00_001FF500(void *, void *, float);
extern float func_002132A8(float, float);
extern int func_L00_00257B90(int, int);
extern float func_00213308(void);
char *func_L07_0030BA90(char *pos, char *vec, int c, short d);
extern void func_0020C828(void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern char D_L07_00166E40[];
extern float D_0015ED70 MACRO_ADDR;
extern float D_L07_0015F580[] MACRO_ADDR;
extern int D_L07_0015F5CC MACRO_ADDR;
extern void func_001F99F8(float *);
extern void func_L00_0025A120(void *);
extern float func_001F9B48(void *, void *);
extern char *func_L00_0025A420(void *, int, int);
extern int func_L00_0025A478(void *, void *, void *, int, int *, float *, int, int);
extern void func_L00_00257470(void *, int, int);
extern void func_L00_0025D538(void *, void *);
extern void func_L00_0025FCB8(void *, void *, float);
extern void func_0020CB10(char *, int, void *);
extern void func_L00_0025D458(void *, short *);
extern void func_001F9D20(void *, void *, void *);
extern void func_001FA050(void *, void *);
extern float func_001FA580(float, float);
extern float func_001FA5C8(float, float);
extern float func_001FA688(float, float);
extern void func_L00_001FFF28(void *, int, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern float func_L00_0025ABF0(void *, void *, void *, float, float);
extern void *func_L00_00263FD8(char *, int, float *, void *, int, int, float, float *, float *, float *);



// Bombomatic update: runs its fuse near the hero, bursts and drifts toward its target, with a state machine on m[0x20].
void FUN_L07_0030d4d8(char *m) {
    char *d;
    char *s16;
    char *t;
    int ia;
    int r;

    float lim;
    float tv;
    float v20[4];
    float v30[20];
    float v80[4];
    float v90[4];
    float vA0[4];
    float vB0[16];
    float vF0[4];
    float v100[4];

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
        *(int *)t = 0;
        break;
    case 2:
        *(int *)t = 0;
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
        tv = *(float *)t;
        if (tv <= lim) {
            func_L00_00257470(m, 0, -1);
            *(float *)t = 0.0f;
            func_001F99F8(v20);
            func_L00_0025E450(m, v20, 0, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 1, 15.0f, 1, 1, -1, 0);
            func_L00_00263FD8(m, 0x621, (float *)(m + 0x10), m + 0x40, 0, 0, 0.0f, D_L07_0015F580, D_L07_0015F580, D_L07_0015F580);
            func_L00_00263FD8(m, 0x622, (float *)(m + 0x10), m + 0x40, 0, 0, 0.0f, D_L07_0015F580, D_L07_0015F580, D_L07_0015F580);
            func_0020C828(m);
            return;
        }
        tv -= lim;
        *(float *)t = tv;
        *(unsigned char *)(d + 0x67) = 0xFA;
        *(short *)(d + 0x26) = func_001F9688(60);
        func_L00_0025D458(m, (short *)(d + 0x60));
    }
eb20:
    ((unsigned char *)m)[0xA4] = 0xFF;
    func_L00_0025D538(m, d + 0x60);
    func_L00_0025FCB8(m, v30, 20.0f);
    switch (((unsigned char *)m)[0x20]) {
    case 0: {
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
        break;
    }
    case 1: {
        if (*(int *)(d + 0xB4) <= D_L07_0015F5CC) {
            int r9 = func_001F9688(0xB4);
            *(int *)(d + 0xB4) = D_L07_0015F5CC + r9;
            *(short *)(d + 0xB8) = 0;
            *(float *)(d + 0xBC) = func_00213308();
            *(short *)(d + 0xBA) = 0;
            m[0x20] = 2;
        }
        break;
    }
    case 2: {
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
                        f = func_001FA688(f, g);
                        if (f < best) {
                            qcopy(vF0, v100);
                            best = f;
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
            if (*(int *)(d + 0xB4) <= D_L07_0015F5CC) {
                int r9 = func_001F9688(0x78);
                *(int *)(d + 0xB4) = D_L07_0015F5CC + r9;
                m[0x20] = 1;
            }
        break;
    }
    }

    if (((unsigned char *)m)[0x20] != 0) {
        float f = D_0015ED6C * 17.80235863f;
        float g = func_001FA580(*(float *)(d + 0xC0), f);
        *(float *)(d + 0xC0) = g;
        func_L00_001FFF28(d + 0x80, 2, g);
    }
}
