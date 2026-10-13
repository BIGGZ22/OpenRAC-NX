/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002C39D0.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L00_002c26c8 -- src/overlays/shared/vendor_002C12B0.c
 * Best so far: SIZE ours 2784 / retail 2804, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Bomb glove update (class 192). Best candidate p5.c: 2764 bytes vs 2804, first difference at +0x12C: the `t[0x2
 *   Also open: frame 416 vs 0x220 (retail keeps more locals), the float block around +0x250 (constant order), case
 */
// Bomb glove update: aims the glove, steps its state machine and fires the shot.
extern unsigned char D_0013A4E0[] NOT_SDA;
extern unsigned char D_0013E533[] NOT_SDA;
extern unsigned char D_0013E05A[] NOT_SDA;
extern char D_L00_00166C80[];
extern char D_L00_00166DC0[];
extern char *D_L00_001ABA00[];
extern float D_0015EE70_f __asm__("D_0015ED70") MACRO_ADDR;
extern int D_0015EEA4 MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern unsigned char D_0014161B[];
extern int D_L00_00173E40[];
extern void func_001FA2B8(void *, void *);
extern void func_001FA2D8(void *, void *);
extern void func_L00_00250800_2c82e8(unsigned char *, int, void *) __asm__("func_L00_0024F7C8");
extern void func_0020CCA8(char *, int, char *);
extern void func_001F9D20(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_002223F8(int, int);
extern int func_001F9740(int *);
extern int func_001F9688(int);
extern int func_L00_00233E98(int);
extern int func_L00_00216DE8(int, int);
extern int func_L00_00233DB8(int, int);
extern void func_001F9CF8(void *, void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_001FA5C8(float, float);
extern float func_001F9DE0(float);
extern void func_001F9A68(void *, void *, float);
extern float func_001F9B20(void *);
extern void func_00214DB0(void *, float, float, float);
extern float func_001F9988(float);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern unsigned char *func_L00_0025C338(int);
extern float func_001F9B80(void *, void *);
extern float func_001FA688(float, float);
extern float func_001FA6C0(int);
void func_L00_002C2488(float *a, float *b, float *out, float x, float y, float lim);
extern char *func_L00_002ACE70(int, void *);
extern void func_L00_002ACFD8(void *);
extern void func_0020C828(void *);

void FUN_L00_002c26c8(unsigned char *m) {
    unsigned char *p, *q, *t, *t2, *b1, *s, *e, *r, *x;
    unsigned char **ip;
    unsigned char *g2;
    int flag, k, quot, sh, r2;
    float F[4] __attribute__((aligned(16)));
    float Cv[4] __attribute__((aligned(16)));
    float Dv[4] __attribute__((aligned(16)));
    float Ev[4] __attribute__((aligned(16)));
    float A[4] __attribute__((aligned(16)));
    float Bv[4] __attribute__((aligned(16)));
    float W[4] __attribute__((aligned(16)));
    float Z[4] __attribute__((aligned(16)));
    float V[4] __attribute__((aligned(16)));
    float U[4] __attribute__((aligned(16)));
    float K[4] __attribute__((aligned(16)));
    float Q2[4] __attribute__((aligned(16)));
    float f0, f1, f2, f3, f4, f12, f14, f20, f21, f22, f23, f24;

    if (m == 0) return;
    p = *(unsigned char **)(m + 0x78);
    F[0] = 0.0f;
    F[1] = -0.0900000036f;
    F[2] = -0.0199999996f;
    F[3] = 0.0f;
    func_001FA2B8(A, D_L00_00166C80);
    func_001FA2D8(Bv, A);
    if (p == 0) return;
    if (*(int *)(p + 0x58) < 3) {
        *(int *)(p + 0x58) = *(int *)(p + 0x58) + 1;
        return;
    }
    func_L00_00250800_2c82e8(m, 0, Cv);
    q = p + 0x40;
    func_0020CCA8((char *)m, 0, (char *)Dv);
    func_001F9D20(Ev, F, Dv);
    func_001F9A10(q, Cv, Ev);

    t = *(unsigned char **)(p + 0x50);
    if (t != 0) *(int *)(t + 0x98) = 1;
    t = *(unsigned char **)(p + 0x50);
    if (t == 0 || t[0x20] == 0xFE || t[0x20] == 0xFD) *(unsigned char **)(p + 0x50) = 0;

    b1 = D_0013E533 + 0xE1D;
    if (*(int *)(b1 + 0x2084) == 1) func_L00_002223F8(0x1E, 1);
    if (func_001F9740((int *)(p + 0x54)) == 0) goto L3BEC;
    if (b1[0x20AC] != 0) goto L3BEC;
    if (b1[0x20A8] != 0 && func_001F9688(0x11) == *(int *)(b1 + 0x1BC)) goto L3BC8;
    if ((*(int *)(D_0013A4E0 + 0x2604) & *(int *)(b1 + 0x10A0)) == 0) goto L3BA4;
    if (*(int *)(b1 + 0x2084) != 0x1E) goto L3BA4;
    if (func_L00_00233E98(-1) != 0) goto L3BC8;
L3BA4:
    if (*(int *)(b1 + 0x2084) != 0x23) goto L3BEC;
    if (func_001F9688(0x10) != *(int *)(b1 + 0x198)) goto L3BEC;
L3BC8:
    func_L00_00216DE8(0x1A, 0);
    func_L00_00233DB8(-1, 1);
    m[0x20] = 3;
L3BEC:
    t = *(unsigned char **)(p + 0x50);
    if (t == 0) goto L42B0;

    t2 = *(unsigned char **)(t + 0x78);
    V[0] = 0.0f;
    V[1] = 0.0f;
    V[2] = 0.0f;
    V[3] = 0.0f;
    V[0] = 1.0f;
    func_001F9CF8(V, V, D_0013E533 + 0x145D);
    f20 = -0.361968011f;
    f0 = func_L00_001FF8B0(*(float *)(b1 + 0x670), *(float *)(b1 + 0x674));
    f21 = 0.859039009f;
    f0 = func_001FA580(f20, f0);
    f24 = 15.0f;
    f0 = func_001F9DC8(f0);
    f0 = f0 * f21;
    W[0] = f0;
    f0 = func_L00_001FF8B0(*(float *)(b1 + 0x670), *(float *)(b1 + 0x674));
    f0 = func_001FA580(f20, f0);
    f0 = func_001F9DE0(f0);
    f0 = f0 * f21;
    W[1] = f0;
    W[2] = 0.0f;
    func_001F9A10(W, W, D_0013E533 + 0x145D - 0x5C0);
    W[2] = W[2] + 0.490819991f;
    f23 = func_L00_001FF8B0(V[0], V[1]);

    s = D_0013A4E0 + 0x2460;
    if ((*(int *)(s + 0x1A0) & 5) == 0) {
        f12 = 8.5f;
    } else {
        f12 = func_001FA6C0(D_0013E05A[0x4D0]) * 2.5f + 8.5f;
    }
    func_001F9A68(Z, V, f12);
    func_001F9A10(Z, Z, W);

    b1 = D_0013E533 + 0xE1D;
    if ((*(int *)(s + 0x1A0) & 5) == 0) goto L4074;
    if (b1[0x20AC] != 0) goto L4074;

    U[0] = 0.0f;
    U[1] = 0.0f;
    U[2] = 1.0f;
    U[3] = 0.0f;
    flag = 0;
    func_001F9D20(U, U, Bv);
    x = *(unsigned char **)(b1 + 0x2080);
    f0 = func_L00_001FF8B0(U[0], U[1]);
    f0 = func_001FA5C8(*(float *)(x + 0x48), f0);
    f0 = func_L00_001FF8B0(U[0], U[1]);
    f23 = f0;
    f0 = func_001F9B20(U);
    f0 = func_L00_001FF8B0(f0, U[2]);
    f20 = f0;
    if (0.593411922f < f20) f20 = 0.593411922f;
    if (f20 < -1.39626336f) f20 = -1.39626336f;

    if ((*(int *)(s + 0x1A0) & 5) == 0) {
        f12 = 8.5f;
    } else {
        f12 = func_001FA6C0(D_0013E05A[0x4D0]) * 2.5f + 8.5f;
    }
    func_00214DB0(U, f12, f23, f20);
    qcopy(K, D_L00_00166DC0);
    f0 = func_001F9B20(U);
    f1 = f0;
    f0 = 0.0f;
    if (f1 == f0) {
        f22 = U[2] / 0.00999999978f;
    } else {
        f22 = U[2] / f1;
    }
    f1 = D_0015EE70_f;
    f0 = 11.0f;
    f20 = f1 * f0;
    if ((*(int *)(s + 0x1A0) & 5) == 0) {
        f12 = (f20 * 8.5f) * 0.5f;
    } else {
        f12 = (f20 * (func_001FA6C0(D_0013E05A[0x4D0]) * 2.5f + 8.5f)) * 0.5f;
    }
    f0 = func_001F9988(f12);
    f1 = f0 + f0;
    f3 = 1.0f - f22;
    f2 = D_0015EE70_f;
    f4 = 11.0f;
    f1 = f1 * f0;
    f2 = f2 * f4;
    f1 = f1 * f3;
    f21 = f1 / f2;
    f22 = f22 * f21;
    if (0.0f < f21) {
        f0 = func_001F9DC8(f23);
        f0 = f0 * f21;
        U[0] = f0;
        f0 = func_001F9DE0(f23);
        f0 = f0 * f21;
        U[2] = f22;
        U[1] = f0;
        func_001F9A10(Q2, D_L00_00166DC0, U);
        r2 = func_L00_001EFC70(D_L00_00166DC0, Q2, 2, (int)m, 0);
        if (r2 != 0) {
            if (*(int *)((unsigned char *)D_L00_00173E40 + 0x18) != *(int *)(b1 + 0x2080)) {
                qcopy(Z, (unsigned char *)D_L00_00173E40 + 0x20);
                flag = 1;
                f23 = func_L00_001FF8B0(Z[0] - W[0], Z[1] - W[1]);
            }
        }
        if (flag == 0) {
            f20 = 5.0f;
            f0 = func_001F9DC8(f23);
            f0 = f0 * f21;
            f0 = f0 * f20;
            Z[0] = f0;
            f0 = func_001F9DE0(f23);
            f0 = f0 * f21;
            f1 = f22 * f20;
            f0 = f0 * f20;
            Z[2] = f1;
            Z[1] = f0;
            func_001F9A10(Z, Z, D_L00_00166DC0);
        }
    }
    goto L423C;

L4074:
    b1 = D_0013E533 + 0xE1D;
L4078:
    *(int *)(b1 + 0xA50) = 0;
    for (ip = (unsigned char **)D_L00_001ABA00; (e = *ip) != 0; ip++) {
        r = func_L00_0025C338((int)e);
        if ((signed char)e[0x20] < 0) continue;
        if (r == 0) continue;
        if ((*(unsigned short *)(e + 0x34) & 0x1000) == 0) continue;
        qcopy(U, e + 0x10);
        U[2] = U[2] + *(float *)(r + 0x10);
        f20 = func_001F9B80(W, U);
        f22 = func_L00_001FF8B0(U[0] - W[0], U[1] - W[1]);
        f0 = func_001F9B80(W, U);
        f21 = func_L00_001FF8B0(f0, U[2] - W[2]);
        f1 = func_001FA688(f23, f22);
        if (!(f1 < 0.785398185f)) {
            if (!(f20 < 4.0f)) continue;
            if (!(f1 < 1.57079637f)) continue;
        }
        if (!(f20 < f24)) continue;
        if (!(f21 < 0.959931076f)) continue;
        if (!(U[2] - W[2] < 2.0f)) continue;
        r2 = func_L00_001EFC70(D_L00_00166DC0, U, 6, (int)m, 0);
        if (r2 != 0) continue;
        *(unsigned char **)(b1 + 0xA50) = e;
        f24 = f20;
        f23 = f22;
        qcopy(Z, U);
    }
L4238:
L423C:
    f1 = D_0015EE70_f;
    f0 = 11.0f;
    f20 = f1 * f0;
    if ((*(int *)(s + 0x1A0) & 5) == 0) {
        f14 = 8.5f;
    } else {
        f14 = func_001FA6C0(D_0013E05A[0x4D0]) * 2.5f + 8.5f;
    }
    func_L00_002C2488(W, Z, (float *)t2, f20, f14, 0.00000999999975f);

L42B0:
    switch (m[0x20]) {
    case 0:
        *(unsigned char **)(p + 0x50) = 0;
        m[0x20] = (m[0x70] & 2) ? 2 : 1;
        /* fall through */
    case 1:
        if (m[0x70] & 2) m[0x20] = 2;
        break;
    case 2:
        break;
    case 3:
        t = *(unsigned char **)(p + 0x50);
        if (t == 0) {
            *(unsigned char **)(p + 0x50) = (unsigned char *)func_L00_002ACE70((int)m, q);
            break;
        }
        func_L00_002ACFD8(W);
        g2 = D_0014161B + 0x65;
        if (!(0xFFFE < *(unsigned short *)(g2 + 0x50))) *(unsigned short *)(g2 + 0x50) = *(unsigned short *)(g2 + 0x50) + 1;
        k = 600;
        quot = func_001F9688(D_0015EEA4) / k;
        if ((int)*(unsigned short *)(g2 + 0x52) < quot) {
            *(short *)(g2 + 0x52) = func_001F9688(D_0015EEA4) / k;
        }
        sh = D_0015ED84;
        *(int *)(g2 + 0x54) = *(int *)(g2 + 0x54) | (1 << sh) | 0x80000000;
        *(unsigned char **)(p + 0x50) = 0;
        m[0x20] = 4;
        *(int *)(p + 0x54) = func_001F9688(0x14);
        break;
    case 4:
        x = D_0013E533 + 0xE1D - 0xBB0;
        if (*(int *)(x + 0x2084) == 0x23 || x[0x20A8] != 0) {
            t = *(unsigned char **)(p + 0x50);
            goto L4438;
        }
        m[0x20] = 2;
        break;
    case 5:
        m[0x20] = 6;
        break;
    case 6:
        if (*(unsigned char **)(p + 0x50) == 0) goto L4454;
        func_0020C828(*(unsigned char **)(p + 0x50));
        break;
    default:
        break;
    }
    t = *(unsigned char **)(p + 0x50);
L4438:
    if (t == 0) goto L4454;
    qcopy(t + 0x10, q);
    return;
L4454:
    if (func_L00_00233E98(-1) != 0 || D_0013E533[0xE1D + 0x20A8] != 0) {
        *(unsigned char **)(p + 0x50) = (unsigned char *)func_L00_002ACE70((int)m, q);
    }
}
