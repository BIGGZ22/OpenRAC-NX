/* Carried from the PAL decompilation's src/overlays/shared/vendor_002C99E0.c (matched C there) (games/rac1) through the version
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
extern float func_001F9FA8_0031B8A8(float) __asm__("func_001F9DE0");
extern float func_001F9F90_0031B8A8(float) __asm__("func_001F9DC8");
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern unsigned char D_0013E05A[];
extern int D_L11_001611C0 MACRO_ADDR;



/* Builds one GIF packet for a rotated textured sprite and advances the packet pointer. */
void FUN_L11_00310cf8(float fa, float fb, float fc, float fd, float fe, int a0, int a1, long a2, int a3,
                       int a4, int a5, int a6, int a7) {
    float s[4];
    float t[4];
    float v[4];
    float w[4];
    float x[4];
    float y[4];
    float z[4];
    unsigned long c0;
    unsigned long c1;
    unsigned long c2;
    unsigned long c3;
    unsigned long pa2;
    int q;
    unsigned long hi;
    unsigned long v7;
    unsigned long v4;
    unsigned long v6;
    unsigned long v8;
    int v5;
    int u0;
    int u1;
    char *old;
    unsigned long *n;
    int *g;
    float k;

    c0 = a4 & 0xFF;
    c1 = a5 & 0xFF;
    c2 = a6 & 0xFF;
    c3 = a7 & 0xFF;
    pa2 = a2;
    q = a3;

    v[0] = fa;
    v[1] = fb;
    s[0] = fd * func_001F9FA8_0031B8A8(fe);
    s[1] = fd * func_001F9F90_0031B8A8(fe);
    t[0] = fc * func_001F9F90_0031B8A8(fe);
    t[1] = -fc * func_001F9FA8_0031B8A8(fe);
    func_001F9A10(w, v, s);
    func_001F9A28(w, w, t);
    func_001F9A10(x, v, s);
    func_001F9A28(y, v, t);
    qcopy(z, v);

    *(int *)(D_L11_001611C0 + 0) = 0x10000007;
    c1 = c1 << 8;
    c0 = c0 | c1;
    v5 = 0x50000007;
    v4 = 0x53535353106;
    *(int *)(D_L11_001611C0 + 4) = 0;
    c2 = c2 << 16;
    c0 = c0 | c2;
    c3 = c3 << 24;
    v6 = 0x154;
    v8 = 0x100010;
    v7 = 0xB400000000008001;
    c0 = c0 | c3;
    *(int *)(D_L11_001611C0 + 8) = 0;
    k = 16.0f;
    u0 = a0 << 4;
    u1 = a1 << 20;
    hi = (unsigned long)q << 32;
    *(int *)(D_L11_001611C0 + 12) = v5;
    old = (char *)D_L11_001611C0;
    n = (unsigned long *)(old + 0x10);
    D_L11_001611C0 = (int)n;
    *(unsigned long *)(old + 0x10) = v7;
    n[1] = v4;
    n[2] = pa2;
    n[3] = v6;
    n[4] = c0;
    n[5] = v8;

    {
        int a = func_001FA898_r(w[0] * k);
        int b = func_001FA898_r(w[1] * k);
        g = (int *)(D_0013E05A + 0x4A6);
        n[6] = (unsigned long)(a + g[4] - 8) | ((unsigned long)(b + g[5] - 8) << 16) | hi;
        n[7] = u0;
    }
    {
        int a = func_001FA898_r(x[0] * k);
        int b = func_001FA898_r(x[1] * k);
        n[8] = (unsigned long)(a + g[4] - 8) | ((unsigned long)(b + g[5] - 8) << 16) | hi;
        n[9] = u1;
    }
    {
        int a = func_001FA898_r(y[0] * k);
        int b = func_001FA898_r(y[1] * k);
        u1 = u1 + u0;
        n[10] = (unsigned long)(a + g[4] - 8) | ((unsigned long)(b + g[5] - 8) << 16) | hi;
        n[11] = u1;
    }
    {
        int a = func_001FA898_r(z[0] * k);
        int b = func_001FA898_r(z[1] * k);
        n[12] = (unsigned long)(a + g[4] - 8) | ((unsigned long)(b + g[5] - 8) << 16) | hi;
        n[13] = 0;
    }

    D_L11_001611C0 += 0x70;
}
