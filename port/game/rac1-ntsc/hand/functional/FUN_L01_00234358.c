/* Carried from the PAL decompilation's src/overlays/shared/help_002274A8.c (matched C there) (games/rac1) through the version
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
extern unsigned char D_0013E533[] NOT_SDA;
f32 func_00213ED8(f32, f32, f32 *);
extern float func_L00_002339D0(float *v);
extern float func_001F9B20(void *);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern float D_0015ED6C MACRO_ADDR;
typedef struct {
    u8 pad88[0x88];
    f32 xF88;
    u8 pad234[0x1a8];
    f32 xF234;
    u8 pad270[0x38];
    s32 xI270;
    s32 xI274;
    f32 xF278;
    u8 pad298[0x1c];
    f32 xF298;
    u8 pad2c0[0x24];
    union { f32 f[2]; s32 i[2]; } a2c0;
    union { f32 f[2]; s32 i[2]; } a2c8;
    union { f32 f[2]; s32 i[2]; } a2d0;
    union { f32 F; s32 I; } u2d8;
    f32 xF2dc;
    union { f32 F; s32 I; } u2e0;
    union { f32 F; s32 I; } u2e4;
    union { f32 F; s32 I; } u2e8;
    f32 xF2ec;
    f32 xF2f0;
    f32 xF2f4;
    s32 xI2f8;
    s32 xI2fc;
    s32 xI300;
    u8 pad30a[0x6];
    s16 xH30a;
    union { s16 H; u16 U; } u30c;
    union { s16 H; u16 U; } u30e;
    u8 pada80[0x770];
    s32 xIa80;
    u8 pad12e0[0x85c];
    s16 xH12e0;
    u8 pad12ed[0xb];
    u8 xB12ed;
    u8 pad2080[0xd92];
    s32 xI2080;
    s32 xI2084;
    u8 pad208c[0x4];
    s32 xI208c;
    u8 pad20b3[0x23];
    u8 xB20b3;
} GS_c234358;
extern GS_c234358 D_0013F350;
extern void func_001F9A28(void *, void *, void *);
typedef int u128_34970_c234358 __attribute__((mode(TI)));
extern void func_L00_001FF550(void *, void *, float);
extern float func_L00_00213350(void *);
extern void func_L00_00233BA0(float *, float *, float);
extern void func_L00_00233708(void *, void *, float);



// Accelerates the hero's air velocity toward the stick direction, clamped to the step.
void FUN_L01_00234358(float step) {
    float v[4];
    float w[4];
    float u[4];
    char *g = (char *)D_0013E533 + 0xE1D;
    switch (*(unsigned char *)(g + 0x20B3)) {
    case 0: {
        float ang = *(float *)(g + 0x180);
        float spd = *(float *)(g + 0x190);
        float d, l1, l2;
        char *h;
        if (*(int *)(g + 0x208C) == 4) {
            if (*(short *)(g + 0x30A) != 0) goto heavy;
            if (*(short *)(g + 0x4A4) != 0 && *(unsigned char *)(g + 0x257) != 0)
                spd = D_0015ED6C * 2.1f;
        }
        {
            char *g2 = (char *)D_0013E533 + 0xE1D;
            if (*(short *)(g2 + 0x30A) != 0) {
                char *g2b;
            heavy:
                g2b = (char *)D_0013E533 + 0xE1D;
                if (*(int *)(g2b + 0x2084) == 0xE && D_0015ED6C < spd)
                    spd = D_0015ED6C;
            }
        }
        {
            char *g3 = (char *)D_0013E533 + 0xE1D;
            v[0] = func_001F9DC8(ang) * spd;
            v[1] = func_001F9DE0(ang) * spd;
            v[2] = *(float *)(g3 + 0xE8);
            d = -(*(float *)(g3 + 0xE0) * v[0] + *(float *)(g3 + 0xE4) * v[1]);
            l1 = func_001F9B20(v);
            l2 = func_001F9B20(g3 + 0xE0);
        }
        if (l1 == 0.0f) d = 0.0f;
        else if (l2 == 0.0f) d = 0.0f;
        else {
            d /= l1;
            d /= l2;
        }
        {
            char *g4 = (char *)D_0013E533 + 0xE1D;
            d += 1.0f;
            d *= 1.5f;
            if (*(int *)(g4 + 0x2084) == 0x81) {
                if (1.7f < d) d = 1.7f;
                if (d < 1.0f) d = 1.0f;
            } else {
                if (3.0f < d) d = 3.0f;
                if (d < 1.0f) d = 1.0f;
            }
        }
        h = (char *)&D_0013F350 + 0xE0;
        step = step * d;
        func_001F9A28(v, v, h);
        if (step < func_001F9B20(v)) func_L00_001FF550(v, v, step);
        func_001F9A10(h, h, v);
        *(u128_34970_c234358 *)w = *(u128_34970_c234358 *)(h + 0x840);
        {
            float t = func_L00_00213350(w);
            char *g5 = (char *)&D_0013F350;
            *(float *)(g5 + 0x194) = *(float *)(g5 + 0x168) - t;
        }
        break;
    }
    case 1:
    case 2: {
        char *h = (char *)&D_0013F350 + 0xE0;
        u[0] = func_L00_002339D0((float *)h);
        func_00213ED8(*(float *)(h + 0xB0), step, u);
        func_L00_00233BA0((float *)h, (float *)h, 0.0f);
        func_L00_00233708(h, h, u[0]);
        break;
    }
    }
}
