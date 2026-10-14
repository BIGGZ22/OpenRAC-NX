/* Carried from the PAL decompilation's nonmatching/l18_veldin2/func_L18_002EBBF0.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L18_002ea800 -- src/overlays/l18_veldin2/vendor_002A8400.c
 * Best so far: SIZE ours 1200 / retail 1236, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L18_002ea800 (level 18, 1236 bytes, vendor_002A8400.c): builds a vector A (copied from m+0x10) and vector
 *   Stopped after 4 runs. The prototype in the file is `void FUN_L18_002ea800(void *)`, so the candidate takes vo
 */
extern short D_L18_00161F68;
extern short D_L18_00161F6C;
extern short D_L18_00161F70;
extern short D_L18_00161F74;
extern short D_L18_00161F78;
extern short D_L18_00161F7C;
extern short D_L18_00161F80;
extern short D_L18_00161F84;
extern short D_L18_00161F88;
extern short D_L18_00161F8C;
extern short D_L18_00161F90;
extern short D_L18_00161F94;
extern short D_L18_00161F98;
extern short D_L18_00161FA0;
extern short D_L18_00161FA4;
extern short D_L18_00161FA8;
extern short D_L18_00161FAC;
extern short D_L18_00161FB0;
extern short D_L18_00161FB4;
extern short D_L18_00161FB8;
extern short D_L18_00161FBC;
extern short D_L18_00161FC0;
extern short D_L18_00161FC4;
extern char D_L18_00161FC8;
extern short D_L18_00161FCC;
extern short D_L18_00161FD0;
extern char D_L18_00161FD4;
extern short D_L18_00161FD8;
extern short D_L18_00161FDC;
extern char D_L18_00161FE0;
extern short D_L18_00161FE4;
extern short D_L18_00161FE8;
extern unsigned char D_L18_00161FC9;
extern unsigned char D_L18_00161FCA;
extern unsigned char D_L18_00161FCB;
extern unsigned char D_L18_00161FD5;
extern unsigned char D_L18_00161FD6;
extern unsigned char D_L18_00161FD7;
extern unsigned char D_L18_00161FE1;
extern unsigned char D_L18_00161FE2;
extern unsigned char D_L18_00161FE3;
extern int D_L18_001D9C70[];
extern int D_L18_001D9C88[];
extern float D_0015ED6C MACRO_ADDR;
extern int D_L18_0015F580 MACRO_ADDR;
extern char D_L18_00167680[];
extern int func_001F9688(int);
extern void func_L06_0030BEF8(char *src, int a1, int a2, int a3);
extern int func_L00_0028DC90(int i, int a1, int v, int k);
extern float func_002132A8(float, float);
extern float func_00213308(void);
extern void func_00214DB0(void *, float, float, float);
extern void func_001F9A68(void *, void *, float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern char *func_00218888(void *, void *, void *, int, int, int, int, int, int);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern int func_00213260(int);
extern int func_L00_00257B90(int, int);
extern void func_L00_0026A9F0(void *, void *, int, int, float, int, int, int, int, float);
extern int func_L00_002AC910(void *, void *, void *, float, int, int, int, int, int);

/* Builds two vector sets from the object at m and the level tables, then runs the per-entry updates. */
void FUN_L18_002ea800(void *mv) {
    char *m = mv;
    float A[4];
    float B[4];
    int i;
    int k;
    int r;
    int xa, xb, xc;
    int s1, s2, t18, t17, q1, q2, r16, r2;
    float f20, f21, f22, f23, t, v, g, h, kk;

    qcopy(A, m + 0x10);
    r = func_001F9688(0x1E);
    func_L06_0030BEF8((char *)A, r, 0x80806060, 0x801010);
    func_L00_0028DC90(0x10, 0, (int)m, 0x58E);
    f23 = 0.8f;
    f22 = 1.2f;
    for (i = 0; i < *(int *)&D_L18_00161F6C; i++) {
        f21 = func_002132A8(-0.34906584f, 0.34906584f);
        f20 = func_00213308();
        t = func_002132A8(*(float *)&D_L18_00161F70, *(float *)&D_L18_00161F74);
        func_00214DB0(A, t * D_0015ED6C, f20, f21);
        func_001F9A68(B, A, *(float *)&D_L18_00161F94);
        t = func_002132A8(f23, f22);
        B[2] = B[2] + *(float *)&D_L18_00161F90 * t * D_0015ED6C;
        B[3] = *(float *)&D_L18_00161F8C;
        A[3] = *(float *)&D_L18_00161F8C;
        f20 = func_002132A8(f23, f22);
        r = func_001F9688(*(int *)&D_L18_00161F80);
        xa = func_001FA898_r((float)r * f20);
        r = func_001F9688(*(int *)&D_L18_00161F84);
        xb = func_001FA898_r((float)r * f20);
        r = func_001F9688(*(int *)&D_L18_00161F88);
        xc = func_001FA898_r((float)r * f20);
        func_00218888(m + 0x10, A, B, *(int *)&D_L18_00161F78, *(int *)&D_L18_00161F7C, xa, xb, xc, *(int *)&D_L18_00161F68);
    }

    f23 = 0.0f;
    for (k = 0; k < *(int *)&D_L18_00161F98; k++) {
        t = func_002132A8(*(float *)&D_L18_00161FA0, *(float *)&D_L18_00161FA4);
        f22 = t * D_0015ED6C;
        f20 = func_002132A8(1.04719758f, 1.39626336f);
        t = func_00213308();
        func_00214DB0(A, f22, t, f20);
        f21 = func_00213308();
        g = func_001F9DC8(f21);
        f20 = g;
        h = func_002132A8(f23, *(float *)&D_L18_00161FBC);
        f20 = f20 * h;
        B[0] = f20;
        kk = func_001F9DE0(f21);
        f20 = kk;
        h = func_002132A8(f23, *(float *)&D_L18_00161FBC);
        f20 = f20 * h;
        B[2] = f23;
        B[1] = f20;
        func_001F9A10(B, B, m + 0x10);
        s1 = func_00213260(6);
        t18 = D_L18_001D9C70[s1];
        s2 = func_00213260(6);
        t17 = D_L18_001D9C88[s2];
        f20 = *(float *)&D_L18_00161FA8 * 400000.0f;
        q1 = func_L00_00257B90(*(int *)&D_L18_00161FAC, *(int *)&D_L18_00161FB0);
        r16 = func_001F9688(q1);
        q2 = func_L00_00257B90(*(int *)&D_L18_00161FB4, *(int *)&D_L18_00161FB8);
        r2 = func_001F9688(q2);
        func_L00_0026A9F0(B, A, t18, t17, f20, r16, r2, 0, 0, f22);
    }

    f20 = 0.0f;
    v = *(float *)&D_L18_00161FCC;
    if (f20 < *(float *)&D_L18_00161FC0) {
        r = func_001F9688(*(int *)&D_L18_00161FC4);
        func_L00_002AC910(m, m + 0x10, &D_L18_0015F580, *(float *)&D_L18_00161FC0, r, *(unsigned char *)&D_L18_00161FC8, D_L18_00161FC9, D_L18_00161FCA, D_L18_00161FCB);
        v = *(float *)&D_L18_00161FCC;
    }
    if (f20 < v) {
        r = func_001F9688(*(int *)&D_L18_00161FD0);
        func_L00_002AC910(m, m + 0x10, &D_L18_0015F580, *(float *)&D_L18_00161FCC, r, *(unsigned char *)&D_L18_00161FD4, D_L18_00161FD5, D_L18_00161FD6, D_L18_00161FD7);
        v = *(float *)&D_L18_00161FD8;
    }
    if (f20 < v) {
        r = func_001F9688(*(int *)&D_L18_00161FDC);
        func_L00_002AC910(m, m + 0x10, &D_L18_0015F580, *(float *)&D_L18_00161FD8, r, *(unsigned char *)&D_L18_00161FE0, D_L18_00161FE1, D_L18_00161FE2, D_L18_00161FE3);
    }
    if (f20 < *(float *)&D_L18_00161FE4) {
        *(float *)(D_L18_00167680 + 0x160) = *(float *)&D_L18_00161FE4;
        *(int *)(D_L18_00167680 + 0x168) = func_001F9688(*(int *)&D_L18_00161FE8);
    }
}
