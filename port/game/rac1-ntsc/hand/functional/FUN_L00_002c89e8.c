/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002C9DC8.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * With PAL's later corrections (nonmatching/functional/func_L00_002C9DC8.c), both in the US assembly
 * (config/us/overlays/asm/FUN_L00_002c89e8.s): the position bounds are 1021.0 (0x447F4000), not 1023.0,
 * and state 3's spiral offset scale is 0.05 (0x3D4CCCCD), not 0.1. */
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
typedef struct {
    float v[4];
} __attribute__((aligned(16))) Q4_c2c89e8;

/* NON_MATCHING FUN_L00_002c89e8 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: SIZE ours 2248 / retail 2288, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   # FUN_L00_002c89e8 (hq10 s08, bomb update, 2288 bytes)
 *   Outline: null/state checks, position bounds check (bad path calls func_0020C828), flag update at moby+0x34, th
 *   Run 1 (p0): compiles, 2224 bytes vs 2288. Run 2 (p1): base inlined instead of a local, worse. Run 3 (p2): flag
 *   Left: 40 bytes short. The prologue saves $fp and $s7 in retail (hi part of D_0013E533+0xE1D kept in $fp, G reb
 */
/* Bomb (class 230) update: steers the bomb's position toward its target and runs its state machine (idle, arm, flight, detonate). Written from the assembly. */
extern char D_0013E533[];
extern char D_L00_00173E60[];
extern char D_L00_00173E70[];
extern int D_L00_00173E40[];
extern void *D_L00_00173E58;
extern int D_L00_0015FFE4 MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
extern float func_001F9AF0(void *a);
/* NTSC-U calls FUN_L00_002603d0 here (jal 0x0C0980F4: returns the moby's word 0x78 -> +8), not
 * FUN_L00_0025c338 (the level's FUN_002141f8, word 0x78 -> +0) as the carry from PAL had it. */
extern int func_L00_002603D0(char *);
extern int func_001F9688(int);
extern int func_001F9740(int *);
extern void *func_L00_0026ED28(void *);
extern float func_001FA6C0(int);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF290(void *, void *, void *);
extern int func_L00_001F0D60(float, void *, int, void *);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_L00_001FF660(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_L00_002C8078(void *);
extern float func_00213508(void *, int, float);
extern int func_L00_00260400(int, char *, char *, void *, char *, void *);
extern int func_001F9770(void *);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern char *func_L00_002BFE08(int, void *, void *);
extern void func_0020C828(void *);

void FUN_L00_002c89e8(char *m) {
    char *s;
    char *u;
    float f0, f1, f2, f3, f20, f21, f22, f23;
    Q4_c2c89e8 v0, v1, v2, v3;
    int r;

    if (m == 0) return;
    s = *(char **)(m + 0x78);
    if (s == 0) return;
    if (*(float *)(m + 0x10) < 2.0f || 1021.0f < *(float *)(m + 0x10) ||
        *(float *)(m + 0x14) < 2.0f || 1021.0f < *(float *)(m + 0x14) ||
        *(float *)(m + 0x18) < 2.0f || 1021.0f < *(float *)(m + 0x18)) {
        func_0020C828(m);
        return;
    }
    {
        char *P = D_0013E533 + 0xE1D;
        if (*(unsigned char *)(P + 0x20A5) != 0 && *(unsigned char *)(m + 0x20) < 2) {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x41;
        } else {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xFFBE;
        }
    }
    {
        char *P = D_0013E533 + 0xE1D;
        if (*(short *)(*(char **)(s + 0x30) + 0xA6) == 0xE5 && *(short *)(s + 0x3A) == 0 &&
            func_001F9AF0(s) > 0.0f) {
            if (*(int *)(P + 0x2FC) == 0 || func_L00_002603D0(*(char **)(P + 0x2FC)) == 0) {
                func_L00_002C8440(s, m, 1);
            } else {
                if (func_L00_002C8440(s, m, 0) == 0) {
                    func_L00_002C8440(s, m, 2);
                }
            }
        }
    }
    switch (*(unsigned char *)(m + 0x20)) {
    case 0:
        m[0x20] = 1;
        *(int *)(s + 0x34) = func_001F9688(4);
        *(short *)(s + 0x38) = 0;
        *(int *)(s + 0x3C) = 0;
        break;
    case 1: {
        char *P = D_0013E533 + 0xE1D;
        *(short *)(s + 0x38) = 0;
        {
            char *p = *(char **)(m + 0x24);
            if (*(float *)(m + 0x2C) < *(float *)(p + 0x24) * 0.25f) {
                *(float *)(m + 0x2C) = *(float *)(m + 0x2C) + *(float *)(p + 0x24) * (D_0015ED60 * 0.05f);
            }
        }
        if (func_001F9740((int *)(s + 0x34)) != 0) {
            *(int *)(s + 0x34) = func_001F9688(4);
            if (P[0x20A5] == 0) {
                func_L00_0026ED28(m);
            }
        }
        if (P[0x20A4] == 0) {
            char *p2 = *(char **)(s + 0x30);
            if (p2 != 0 && (((unsigned char *)p2)[0x20] & 0xF0) != 0xF0) {
                char *q = *(char **)(P + 0x1090);
                if (q != 0 && *(short *)(q + 0xA6) == 0xE5) {
                    return;
                }
            }
        }
        func_0020C828(m);
        break;
    }
    case 2: {
        char *P = D_0013E533 + 0xE1D;
        f22 = 0.5f;
        f21 = func_001FA6C0(((unsigned char *)D_0013E533)[1]) + 1.0f;
        {
            char *p = *(char **)(m + 0x24);
            if (*(float *)(m + 0x2C) < *(float *)(p + 0x24) * f22 * f21) {
                *(float *)(m + 0x2C) = *(float *)(m + 0x2C) +
                    *(float *)(p + 0x24) * (D_0015ED60 * 0.05f) * f21;
            }
        }
        if (func_001F9740((int *)(s + 0x34)) != 0) {
            *(int *)(s + 0x34) = func_001F9688(4);
            func_L00_0026ED28(m);
        }
        u = m + 0x10;
        *(float *)(s + 0x8) = *(float *)(s + 0x8) - D_0015ED70 * 9.0f;
        func_001F9A10(&v0, u, s);
        func_L00_001FF290(&v3, u, D_0013E533 + 0xF5D);
        qcopy(&v1, u);
        f23 = 0.15f;
        {
            char *p = *(char **)(m + 0x24);
            f1 = D_0015ED60 * f23 * *(float *)(m + 0x2C);
            f2 = *(float *)(p + 0x24);
            v1.v[2] = v1.v[2] - f1 / *(float *)(p + 0x24);
            qcopy(&v2, &v0);
            v2.v[2] = v2.v[2] - f1 / f2;
        }
        qcopy(&v3, &v0);
        f20 = f21 * 0.2f;
        v3.v[2] = v3.v[2] + f20;
        r = func_L00_001F0D60(f21 * 0.3f, &v3, 4, *(void **)(P + 0x2080));
        if (r != 0) {
            qcopy(u, D_L00_00173E70);
            *(float *)(m + 0x18) = *(float *)(m + 0x18) - f20;
            f21 = 0.01f;
            func_L00_001FF660(s, s, D_L00_00173E70 + 0x10);
        } else {
            r = func_L00_001EFC70(&v1, &v2, 0, *(int *)(P + 0x2080), 0);
            if (r == 0) {
                qcopy(u, &v0);
                return;
            }
            qcopy(u, D_L00_00173E60);
            {
                char *p = *(char **)(m + 0x24);
                f0 = D_0015ED60 * f23 * *(float *)(m + 0x2C) / *(float *)(p + 0x24);
                *(float *)(m + 0x18) = *(float *)(m + 0x18) + f0;
            }
            f21 = 0.01f;
            func_L00_001FF660(s, s, D_L00_00173E60 + 0x20);
        }
        func_001F9A68(s, s, f22);
        if (func_001F9AF0(s) < D_0015ED60 * f21) {
            goto L344;
        }
        if (*(int *)(P + 0x2FC) != 0 && func_L00_002603D0(*(char **)(P + 0x2FC)) != 0) {
            float a20 = func_001F9AF0(s);
            if (a20 < D_0015ED60 * f21 + func_001F9AF0(D_0013E533 + 0xF1D)) {
                goto L344;
            }
        }
        goto L394;
    L344:
        {
            int n = *(unsigned short *)(s + 0x38) + 1;
            *(short *)(s + 0x38) = n;
            if (D_L00_00173E40[6] == 0 || D_L00_00173E40[7] > 0 ||
                func_001F9688(0xF0) < (short)n) {
                m[0x20] = 3;
                *(short *)(s + 0x38) = 0;
            }
        }
    L394:
        if (*(int *)(P + 0x2FC) != 0 && func_L00_002603D0(*(char **)(P + 0x2FC)) != 0) {
            func_001F9A10(s, s, P + 0x100);
        }
        func_001F9A10(u, u, s);
        func_L00_002C8078(m);
        return;
    }
    case 3: {
        char *P = D_0013E533 + 0xE1D;
        u = m + 0x10;
        f0 = func_00213508(u, 0, 0.5f);
        if (f0 != 0.0f && D_L00_00173E58 != 0) {
            func_L00_00260400((int)m, D_L00_00173E58, u, m + 0x40, u, m + 0x40);
        }
        if (func_001F9770(s + 0x38) != 0) {
            char *p = (char *)D_L00_0015FFE4;
            int cnt = 0;
            while (p != 0) {
                if (*(short *)(p + 0xA6) == 0xBA) {
                    if ((((unsigned char *)p)[0x20] & 0xF0) != 0xF0) cnt++;
                }
                p = *(char **)(p + 0x28);
            }
            if (cnt >= 8) {
                *(short *)(s + 0x38) = func_001F9688(0xF);
            } else {
                f20 = 0.05f;
                f21 = 6.2831855f;
                f22 = 0.25f;
                f0 = func_001F9DC8(((float)*(int *)(s + 0x3C)) * f21 * f22);
                f1 = D_0015ED60;
                v1.v[0] = f1 * f20 * f0;
                f0 = func_001F9DE0(((float)*(int *)(s + 0x3C)) * f21 * f22);
                v1.v[1] = (f1 * f20) * f0;
                v1.v[2] = f1 * 0.02f;
                if (*(int *)(P + 0x2FC) != 0 && func_L00_002603D0(*(char **)(P + 0x2FC)) != 0) {
                    func_001F9A10(&v1, &v1, P + 0x100);
                }
                func_001F9A10(&v0, &v1, u);
                func_L00_002BFE08(*(int *)(s + 0x30), &v0, &v1);
                *(short *)(s + 0x38) = func_001F9688(8);
                *(int *)(s + 0x3C) = *(int *)(s + 0x3C) + 1;
                if (*(int *)(s + 0x3C) >= 4) {
                    m[0x20] = 4;
                }
            }
        }
        if (*(int *)(s + 0x3C) < 2) {
            if (func_001F9740((int *)(s + 0x34)) != 0) {
                *(int *)(s + 0x34) = func_001F9688(4);
                func_L00_0026ED28(m);
            }
        }
        break;
    }
    case 4:
        f0 = *(float *)(m + 0x2C) * 0.92f;
        m[0x23] = m[0x23] - 4;
        *(float *)(m + 0x2C) = f0;
        f0 = func_00213508(m + 0x10, 0, 0.5f);
        if (f0 != 0.0f && D_L00_00173E58 != 0) {
            func_L00_00260400((int)m, D_L00_00173E58, m + 0x10, m + 0x40, m + 0x10, m + 0x40);
        }
        if ((unsigned char)m[0x23] == 0) {
            func_0020C828(m);
        }
        break;
    case 5:
    default:
        m[0x20] = 0;
        break;
    }
}
