/* Carried from the PAL decompilation's src/overlays/shared/help_0020CDF0.c (matched C there) (games/rac1) through the version
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
extern char D_0013E533[];
extern short D_L00_0015F694;
extern void func_001FA2B8();
extern void func_00214128(void *);
extern void func_001F99F8(void *);
extern float func_L00_0020F0B8(float *a, float *b);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_00214598(void *, void *);
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) V4_F7F8_c20f160;



/* Advances the 16-slot history ring, scores slots against each other and averages the agreeing ones into arg+0x40. */
void FUN_L00_0020f160(char *arg) {
    V4_F7F8_c20f160 vv[4];
    float fa[16];
    int ri;
    int cnt;
    float *fp;
    char *out;
    int n, i, k, acc;
    float mx;

    {
    char *b = D_0013E533 + 0xE1D;
    func_001FA2B8(D_0013E533 + 0xE1D + 0x12F0 + *(int *)(b + 0x21B8) * 0x30);
    func_00214128(D_0013E533 + 0xE1D + 0x12F0 + *(int *)(b + 0x21B8) * 0x30);

    *(int *)(b + 0x21B8) = (*(int *)(b + 0x21B8) + 1) % 16;
    *(int *)(b + 0x21BC) = *(int *)(b + 0x21BC) + 1;
    if (*(int *)(b + 0x21BC) > 16) {
        *(int *)(b + 0x21BC) = 16;
    }
    }

    for (k = 0; k < 3; k++) {
        func_001F99F8(&vv[k]);
    }

    {
    char *b = D_0013E533 + 0xE1D;
    if (*(int *)(b + 0x208C) == 0x16) {
        *(int *)&D_L00_0015F694 = 14;
    } else if (*(int *)(b + 0x2084) == 2) {
        *(int *)&D_L00_0015F694 = 5;
    } else {
        *(int *)&D_L00_0015F694 = 11;
    }
    }

    {
    char *b = D_0013E533 + 0xE1D;
    if (*(int *)(b + 0x21C0) < *(int *)&D_L00_0015F694) {
        *(int *)(b + 0x21C0) = *(int *)(b + 0x21C0) + 1;
    } else if (*(int *)&D_L00_0015F694 < *(int *)(b + 0x21C0)) {
        *(int *)(b + 0x21C0) = *(int *)(b + 0x21C0) - 1;
    }
    }

    {
    char *b = D_0013E533 + 0xE1D;
    n = *(int *)(b + 0x21C0);
    if (n < 2) {
        n = 2;
    }
    if (*(int *)(b + 0x21BC) < n) {
        return;
    }
    }

    out = arg + 0x40;
    cnt = 0;

    for (i = 0; i < n; i++) {
        char *b = D_0013E533 + 0xE1D;
        int t;
        fp = fa;
        t = i - 15;
        ri = (*(int *)(b + 0x21B8) - t) % 16;
        for (k = 0; k < n; k++) {
            if (i != k) {
                char *b = D_0013E533 + 0xE1D;
                int u = k - 15;
                int rj = (*(int *)(b + 0x21B8) - u) % 16;
                *(float *)((char *)fp - (-(i * 4))) += func_L00_0020F0B8(
                    (float *)(D_0013E533 + 0xE1D + 0x12F0 + ri * 0x30),
                    (float *)(D_0013E533 + 0xE1D + 0x12F0 + rj * 0x30));
            }
        }
    }

    mx = 0.0f;
    for (i = 0; i < n; i++) {
        if (mx < fa[i]) {
            mx = fa[i];
        }
    }

    for (i = 0; i < n; i++) {
        if (mx * 0.2f < fa[i]) {
            cnt++;
        }
    }

    acc = 0;
    for (i = 0; i < n; i++) {
        if (mx * 0.2f < fa[i] && cnt > 0 && cnt < 3) {
            continue;
        }
        acc++;
        {
        char *b = D_0013E533 + 0xE1D;
        int t = i - 15;
        ri = (*(int *)(b + 0x21B8) - t) % 16;
        for (k = 0; k < 3; k++) {
            func_001F9A10(&vv[k], &vv[k], D_0013E533 + 0xE1D + 0x12F0 + ri * 0x30 + k * 16);
        }
        }
    }

    for (k = 0; k < 3; k++) {
        func_001F9A68(&vv[k], &vv[k], 1.0f / (float)acc);
    }
    func_00214598(vv, out);
}
