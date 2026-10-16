/* Carried from the PAL decompilation's src/overlays/shared/vendor_00298BB8.c (matched C there) (games/rac1) through the version
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
extern float func_001FA580(float, float);
extern float func_001FA6C0(int);
extern void func_001F9A28(void *dst, void *a, void *b);
extern float func_001F9AF0(void *);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001F9DE0(float);
extern void func_001F4600(void (*)(void), void *);
extern float D_0015ED6C MACRO_ADDR;
extern float func_001FA5C8(float, float);
extern float func_001F9DC8(float x);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern int func_00213260(int);
extern int func_001FA6D0(float);
extern void func_001F9A68(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern float func_001F96B0(float);
extern void func_00214DB0(void *, float, float, float);
extern void func_00214890(void *dst, void *vec, void *axis, float angle);
extern void func_L00_001FF290(void *, void *, void *);
extern float func_002132A8(float, float);
extern void func_001FA050(float *, float *);
extern void func_001F9EC0_2A3DD0(void *, void *, void *) __asm__("func_001F9CF8");
typedef int u128_2A8090_c2a6e68 __attribute__((mode(TI)));
typedef struct {
    float v[2];
    float f8;
    float fC;
    unsigned char *moby;
    int flags;
    unsigned char b18;
    unsigned char b19;
    short h1A;
    float f1C;
    int i20;
    int pad24[3];
} Ray_2A8090_c2a6e68;
extern void func_001FA2B8(void *, void *);
extern void func_00214128(void *);
extern float func_001FA610(float);
extern int func_001160D8(void);
extern u8 D_L15_00174460[];
extern short D_L15_00161558;
extern short D_L15_0016155C;
extern short D_L15_00161560;
extern void func_L15_002A7628();
extern unsigned char *func_L00_00273F80_r(float *pos, char *vel, int color, unsigned char life, unsigned char b, int mode, float scale) __asm__("func_L00_002730E0");



/* Spiderbot update: steps the walk/turn state, then probes ahead and sprays sparks along the path. */
void FUN_L15_002a6e68(unsigned char *moby) {
    float old[4];
    float A[4];
    float B[4];
    float C[4];
    unsigned char *d = *(unsigned char **)(moby + 0x78);
    *(u128_2A8090_c2a6e68 *)old = *(u128_2A8090_c2a6e68 *)(moby + 0x10);
    {
    float D[4];
    float M[16];
    float T[4];
    switch (moby[0x20]) {
    case 0:
        *(u128_2A8090_c2a6e68 *)d = *(u128_2A8090_c2a6e68 *)old;
        *(float *)(moby + 0x58) = *(float *)(d + 0x30) / 1.5f;
        *(float *)(d + 0x28) = *(float *)(d + 0x28) * 0.017453292f;
        if (*(int *)(d + 0x20) == 0) {
            moby[0x20] = 1;
            *(float *)(d + 0x2C) = *(float *)(moby + 0x48);
        } else if (*(int *)(d + 0x20) >= 3) {
            moby[0x20] = 2;
            *(float *)(d + 0x2C) = *(float *)(moby + 0x48);
        } else {
            moby[0x20] = 3;
        }
        break;
    case 1: {
        *(u128_2A8090_c2a6e68 *)C = 0;
        C[2] = *(float *)(d + 0x24);
        *(u128_2A8090_c2a6e68 *)D = 0;
        *(u128_2A8090_c2a6e68 *)B = *(u128_2A8090_c2a6e68 *)C;
        *(float *)(d + 0x28) = func_001FA580(*(float *)(d + 0x28),
            360.0f / (*(float *)(d + 0x24) * 6.2831855f / *(float *)(d + 0x30)) * 0.017453292f * D_0015ED6C);
        *(float *)(moby + 0x58) = *(float *)(d + 0x30) / 1.5f;
        C[0] = func_001F9DC8(*(float *)(d + 0x2C));
        C[1] = func_001F9DE0(*(float *)(d + 0x2C));
        C[2] = 0.0f;
        func_00214890(A, B, C, *(float *)(d + 0x28));
        func_001F9A10(T, d, A);
        *(u128_2A8090_c2a6e68 *)(moby + 0x10) = *(u128_2A8090_c2a6e68 *)T;
        D[2] = func_001FA580(*(float *)(d + 0x2C), 1.5707964f);
        D[1] = func_001FA580(-*(float *)(d + 0x28), 3.1415927f);
        func_001FA050(M, D);
        func_001FA2B8(moby + 0xC0, M);
        func_00214128(moby + 0xC0);
        *(unsigned short *)(moby + 0x34) |= 0x100;
        break;
    }
    case 2: {
        float b1;
        float b2;
        *(u128_2A8090_c2a6e68 *)C = 0;
        C[2] = -*(float *)(d + 0x24);
        *(u128_2A8090_c2a6e68 *)D = 0;
        *(u128_2A8090_c2a6e68 *)B = *(u128_2A8090_c2a6e68 *)C;
        b1 = func_001FA580(func_001FA610(func_001F9DC8(*(float *)(d + 0x28)) * 1.5707964f),
                           *(int *)(d + 0x20) == 4 ? 1.5707964f : 0.0f);
        *(float *)(d + 0x28) = func_001FA580(*(float *)(d + 0x28),
            360.0f / (*(float *)(d + 0x24) * 6.2831855f / *(float *)(d + 0x30)) * 0.017453292f * D_0015ED6C);
        b2 = func_001FA580(func_001FA610(func_001F9DC8(*(float *)(d + 0x28)) * 1.5707964f),
                           *(int *)(d + 0x20) == 4 ? 1.5707964f : 0.0f);
        *(float *)(moby + 0x58) = *(float *)(d + 0x24) * func_001FA5C8(b2, b1) / (D_0015ED6C * 1.5f);
        C[0] = func_001F9DC8(*(float *)(d + 0x2C));
        C[1] = func_001F9DE0(*(float *)(d + 0x2C));
        C[2] = 0.0f;
        func_00214890(A, B, C, b2);
        func_001F9A10(T, d, A);
        *(u128_2A8090_c2a6e68 *)(moby + 0x10) = *(u128_2A8090_c2a6e68 *)T;
        D[2] = func_001FA580(*(float *)(d + 0x2C), 1.5707964f);
        D[1] = func_001FA580(-b2, 0.0f);
        func_001FA050(M, D);
        func_001FA2B8(moby + 0xC0, M);
        func_00214128(moby + 0xC0);
        *(unsigned short *)(moby + 0x34) |= 0x100;
        break;
    }
    case 3: {
        float w;
        float x;
        w = func_001F9DC8(*(float *)(d + 0x28)) * *(float *)(d + 0x24);
        *(float *)(d + 0x28) = func_001FA580(*(float *)(d + 0x28),
            360.0f / (*(float *)(d + 0x24) * 6.2831855f / *(float *)(d + 0x30)) * 0.017453292f * D_0015ED6C);
        x = func_001F9DC8(*(float *)(d + 0x28)) * *(float *)(d + 0x24);
        *(float *)(moby + 0x58) = (x - w) / (D_0015ED6C * 1.5f);
        func_00214DB0(A, x, *(int *)(d + 0x20) != 2 ? *(float *)(moby + 0x48) : 0.0f,
                      *(int *)(d + 0x20) == 2 ? 1.5707964f : 0.0f);
        func_001F9A10(B, d, A);
        *(u128_2A8090_c2a6e68 *)(moby + 0x10) = *(u128_2A8090_c2a6e68 *)B;
        break;
    }
    }
    }
    {
        Ray_2A8090_c2a6e68 R;
        float T2[4];
        int i;
        *(u128_2A8090_c2a6e68 *)B = 0;
        B[2] = 12.0f;
        func_001F9EC0_2A3DD0(B, B, moby + 0xC0);
        func_001F9A10(R.v, moby + 0x10, B);
        *(u128_2A8090_c2a6e68 *)A = *(u128_2A8090_c2a6e68 *)R.v;
        R.flags = 0x10001;
        R.moby = moby;
        R.f1C = 1.0f;
        R.i20 = 1;
        func_L00_001FF500(R.v, B, 1.0f);
        R.h1A = *(unsigned short *)(moby + 0xA6);
        R.f8 = 1.0f;
        R.fC = 5627.9248f;
        R.b18 = 1;
        R.b19 = 1;
        func_001F9A28(T2, moby + 0x10, old);
        *(u128_2A8090_c2a6e68 *)C = *(u128_2A8090_c2a6e68 *)T2;
        func_L00_001EFC70(moby + 0x10, A, 9, moby, &R);
        if (func_L00_001EFC70(moby + 0x10, A, 2, 0, 0)) {
            *(u128_2A8090_c2a6e68 *)(d + 0x10) = *(u128_2A8090_c2a6e68 *)D_L15_00174460;
        } else {
            *(u128_2A8090_c2a6e68 *)(d + 0x10) = *(u128_2A8090_c2a6e68 *)A;
        }
        for (i = 0; i < *(int *)&D_L15_00161558; i++) {
            float s;
            float len;
            int k;
            unsigned char life;
            s = -1.0f;
            if (func_001160D8() & 1) s = 1.0f;
            k = func_001FA6D0(func_001F96B0(func_002132A8(*(float *)&D_L15_00161560 * 0.5f, *(float *)&D_L15_00161560)));
            func_001F9A28(T2, d + 0x10, moby + 0x10);
            *(u128_2A8090_c2a6e68 *)B = *(u128_2A8090_c2a6e68 *)T2;
            func_001F9A68(B, B, s);
            len = func_001F9AF0(B);
            *(u128_2A8090_c2a6e68 *)A = s < 0.0f ? *(u128_2A8090_c2a6e68 *)(d + 0x10) : *(u128_2A8090_c2a6e68 *)(moby + 0x10);
            life = k;
            func_001F9A68(B, B, func_002132A8(0.0f, 0.9f));
            func_L00_001FF290(T2, A, B);
            func_L00_001FF500(B, B, func_002132A8(D_0015ED6C * 10.0f, len / func_001FA6C0(k * 4)));
            func_L00_00273F80_r(A, (char *)B, 0x604040FF, life, func_00213260(0xFF), 0, *(float *)&D_L15_0016155C);
        }
    }
    func_001F4600(func_L15_002A7628, moby);
}
