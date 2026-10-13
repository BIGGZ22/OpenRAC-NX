/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002C2DD8.c (games/rac1/pal) through the version
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
extern void func_001F9A10(void *, void *, void *);

/* NON_MATCHING FUN_L00_002c1ad0 -- src/overlays/shared/vendor_002C12B0.c
 * Best so far: SIZE ours 2448 / retail 2488, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped at the budget (14 runs). Best: p13.c (2436 bytes, matches the prologue through +120, first diff at the
 *   Wall: none hit; budget spent with wordings that move size but not the register or branch-form differences.
 */
extern int func_001F9688(int);
extern void func_001FA2B8(void *, void *);
extern void func_001FA2D8(void *, void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_0020CCA8(char *, int, char *);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_002223F8(int, int);
extern int func_001F9740(int *);
extern int func_L00_00233E98(int);
extern int func_L00_00216DE8(int, int);
extern int func_L00_00233DB8(int, int);
extern void func_001F9CF8(void *, void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001FA5C8(float, float);
extern void func_001F9A68(void *, void *, float);
extern float func_001F9B20(void *);
extern void func_00214DB0(void *, float, float, float);
extern float func_001F9988(float);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_L00_002AA008(void *, void *, void *);
extern void func_L00_0025F780(int, short *, short);
extern void func_0020C828(void *);
extern unsigned char *func_L00_002A9ED0(void *, void *, void *);
extern unsigned char D_0013E533[] NOT_SDA;
extern unsigned char D_0013A4E0[] NOT_SDA;
extern unsigned char D_0013E05A[] NOT_SDA;
extern unsigned char D_0014161B[] NOT_SDA;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED68 MACRO_ADDR;
extern float D_0015EE70_f __asm__("D_0015ED70") MACRO_ADDR;
extern int D_0015EEA4 MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern char D_L00_00166C80[];
extern char D_L00_00166DC0[];
extern int D_L00_00173E40[];
extern short D_L00_001B07B0[];
void func_L00_002C1928(float a, float b, float c, float *p, float *q, float *out);

typedef int u128x_c2c1ad0 __attribute__((mode(TI)));

/* update function of moby class 190 (mine_glove): aims the glove and fires its shot */
void FUN_L00_002c1ad0(char *m) {
    char *md;
    char *p;
    char *tgt;
    char *ptr;
    unsigned char *X;
    unsigned char *W;
    unsigned char *Y;
    unsigned char *Q;
    unsigned char *Z;
    float v00[4], v10[4], v20[4], v30[4];
    float v70[4], v80[4], v90[4], vc0[4];
    float v100[4], v110[4], v120[4];
    float kk, f20, f21, f23, a, a2, g, k1, len, q1;
    int flag, fire, r, ret, c88;

    if (m == 0) return;
    md = *(char **)(m + 0x78);
    if (*(int *)(md + 0x5C) < 3) {
        *(int *)(md + 0x5C) = *(int *)(md + 0x5C) + 1;
        return;
    }
    qzero(v10);
    v10[1] = -0.09f;
    v10[2] = -0.02f;
    func_001FA2B8(v90, D_L00_00166C80);
    func_001FA2D8(vc0, v90);
    if (md == 0) return;
    func_L00_0024F7C8(m, 0, v00);
    func_0020CCA8(m, 0, (char *)v30);
    func_001F9D20(v20, v10, v30);
    func_001F9A10(md + 0x40, v00, v20);

    p = *(char **)(md + 0x50);
    if (p == 0) {
        *(char **)(md + 0x50) = 0;
    } else {
        *(int *)(p + 0x98) = 1;
        p = *(char **)(md + 0x50);
        if (p == 0) {
            *(char **)(md + 0x50) = 0;
        } else if (((unsigned char *)p)[0x20] == 0xFE || ((unsigned char *)p)[0x20] == 0xFD) {
            *(char **)(md + 0x50) = 0;
        }
    }

    X = (unsigned char *)D_0013E533 + 0xE1D;
    W = (unsigned char *)D_0013A4E0 + 0x2460;
    Y = (unsigned char *)D_0013E05A + 0x4C6;
    if (*(int *)(X + 0x2084) == 1) func_L00_002223F8(0x1E, 1);
    flag = func_001F9740((int *)(md + 0x58));

    if (flag != 0 && X[0x20AC] == 0) {
        fire = 0;
        if (X[0x20A8] != 0 && *(int *)(X + 0x1BC) == func_001F9688(0x11)) fire = 1;
        if (fire == 0 && (*(int *)(D_0013A4E0 + 0x2604) & *(int *)(X + 0x10A0))) {
            if (*(int *)(X + 0x2084) == 0x1E && func_L00_00233E98(-1) != 0) fire = 1;
        }
        if (fire == 0 && *(int *)(X + 0x2084) == 0x23 && *(int *)(X + 0x198) == func_001F9688(0x10)) fire = 1;
        if (fire) {
            func_L00_00216DE8(0x1A, 0);
            func_L00_00233DB8(-1, 1);
            m[0x20] = 3;
        }
    }

    p = *(char **)(md + 0x50);
    if (p != 0 && X[0x20AC] == 0) {
        tgt = *(char **)(p + 0x78);
        if (Y[0x11] != 0) {
            kk = (*(int *)(W + 0x1A0) & 5) ? 12.0f : 4.0f;
        } else {
            kk = (*(int *)(W + 0x1A0) & 5) ? 6.0f : 4.0f;
        }
        *(u128x_c2c1ad0 *)v110 = 0;
        v110[0] = D_0015ED6C * kk;
        qcopy(v100, v110);
        func_001F9CF8(v100, v100, (char *)D_0013E533 + 0x145D);

        f20 = -0.3619680107f;
        a = func_L00_001FF8B0(*(float *)(X + 0x670), *(float *)(X + 0x674));
        v70[0] = func_001F9DC8(func_001FA580(f20, a)) * 0.8590390086f;
        v70[1] = func_001F9DE0(func_001FA580(f20, func_L00_001FF8B0(*(float *)(X + 0x670), *(float *)(X + 0x674)))) * 0.8590390086f;
        v70[2] = 0.0f;
        func_001F9A10(v70, v70, (char *)D_0013E533 + 0xE9D);
        v70[2] = v70[2] + 0.4908199906f;
        func_L00_001FF8B0(v100[0], v100[1]);
        func_001F9A68(v80, v100, D_0015ED68 * 60.0f);
        func_001F9A10(v80, v80, v70);

        if (*(int *)(W + 0x1A0) & 5) {
            flag = 0;
            *(u128x_c2c1ad0 *)v120 = 0;
            v120[2] = (Y[0x11] != 0) ? 12.0f : 6.0f;
            qcopy(v110, v120);
            func_001F9D20(v110, v110, vc0);
            a = func_L00_001FF8B0(v110[0], v110[1]);
            ptr = *(char **)(X + 0x2080);
            func_001FA5C8(*(float *)(ptr + 0x48), a);
            a2 = func_L00_001FF8B0(v110[0], v110[1]);
            len = func_001F9B20(v110);
            g = func_L00_001FF8B0(len, v110[2]);
            if (g > 0.5934119225f) g = 0.5934119225f;
            if (g < -1.3962633610f) g = -1.3962633610f;
            k1 = g;
            if (Y[0x11] != 0) {
                kk = (*(int *)(W + 0x1A0) & 5) ? 12.0f : 4.0f;
            } else {
                kk = (*(int *)(W + 0x1A0) & 5) ? 6.0f : 4.0f;
            }
            func_00214DB0(v110, kk, a2, k1);
            len = func_001F9B20(v110);
            f20 = v110[2] / ((len == 0.0f) ? 0.01f : len);
            kk = (Y[0x11] != 0) ? 12.0f : 6.0f;
            g = func_001F9988(((D_0015EE70_f * 9.8f) * kk) * 0.5f);
            f21 = (((g + g) * g) * (1.0f - f20)) / (D_0015EE70_f * 9.8f);
            if (0.0f < f21) {
                f23 = f20 * f21;
                v110[0] = func_001F9DC8(a2) * f21;
                v110[1] = func_001F9DE0(a2) * f21;
                v110[2] = f23;
                func_001F9A10(v120, D_L00_00166DC0, v110);
                if (func_L00_001EFC70(D_L00_00166DC0, v120, 2, (int)m, 0) != 0) {
                    Q = (unsigned char *)D_L00_00173E40;
                    if (*(int *)(Q + 0x18) != *(int *)(X + 0x2080)) {
                        qcopy(v80, Q + 0x20);
                        flag = 1;
                        a2 = func_L00_001FF8B0(v80[0] - v70[0], v80[1] - v70[1]);
                    }
                }
                if (flag == 0) {
                    v80[0] = (func_001F9DC8(a2) * f21) * 5.0f;
                    v80[1] = (func_001F9DE0(a2) * f21) * 5.0f;
                    v80[2] = f23 * 5.0f;
                    func_001F9A10(v80, v80, D_L00_00166DC0);
                }
            }
        }
        if (Y[0x11] != 0) {
            kk = (*(int *)(W + 0x1A0) & 5) ? 12.0f : 4.0f;
        } else {
            kk = (*(int *)(W + 0x1A0) & 5) ? 6.0f : 4.0f;
        }
        func_L00_002C1928(D_0015EE70_f * 9.8f, 1.0f, kk, v70, v80, (float *)tgt);
    }

    switch ((unsigned char)m[0x20]) {
    case 0:
        *(char **)(md + 0x50) = 0;
        m[0x20] = (m[0x70] & 2) ? 2 : 1;
        /* fall through */
    case 1:
        if (m[0x70] & 2) m[0x20] = 2;
        break;
    case 2:
        break;
    case 3:
        p = *(char **)(md + 0x50);
        if (p != 0) {
            func_L00_002AA008(v70, p, *(void **)(p + 0x78));
            Z = (unsigned char *)D_0014161B + 0x65;
            c88 = *(unsigned short *)(Z + 0x88);
            if (c88 <= 0xFFFE) *(short *)(Z + 0x88) = c88 + 1;
            q1 = func_001F9688(D_0015EEA4) / 600;
            if ((int)*(unsigned short *)(Z + 0x8A) < (int)q1) {
                *(short *)(Z + 0x8A) = func_001F9688(D_0015EEA4) / 600;
            }
            *(int *)(Z + 0x8C) = *(int *)(Z + 0x8C) | (1 << D_0015ED84) | 0x80000000;
            func_L00_0025F780((int)p, D_L00_001B07B0, 0x3F);
            *(char **)(md + 0x50) = 0;
            *(int *)(md + 0x54) = func_001F9688(10);
        }
        m[0x20] = 4;
        *(int *)(md + 0x58) = func_001F9688(0x14);
        break;
    case 4:
        if (*(int *)(X + 0x2084) == 0x23) break;
        if (X[0x20A8] != 0) break;
        m[0x20] = 2;
        break;
    case 5:
        m[0x20] = 6;
        break;
    case 6:
        p = *(char **)(md + 0x50);
        if (p != 0) {
            func_0020C828(p);
            *(char **)(md + 0x50) = 0;
        }
        break;
    default:
        break;
    }

    func_001F9740((int *)(md + 0x54));
    p = *(char **)(md + 0x50);
    if (p != 0) {
        qcopy(p + 0x10, md + 0x40);
    } else if (*(int *)(md + 0x54) == 0) {
        if (func_L00_00233E98(-1) != 0) {
            qzero(v100);
            *(char **)(md + 0x50) = (char *)func_L00_002A9ED0(m, md + 0x40, v100);
        }
    }
}
