#include "include_asm.h"

typedef struct {
    float b0[4];
    float b1[4];
    float b2[4];
    float b3[4];
    int k0, k1, k2, k3;
    int z80, z84, z88;
    float f92, f96;
    int z100;
    float f104, f108;
    long long v112;
    long long v120;
    long long v128;
    long long v136;
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float v192, v196, v200, v204;
    float v208, f212, f216, v220;
} LOC_L11_458DF0;

extern void func_00282D30_2(char *dst, char *src, float scale);

extern void func_L02_00333480(char *a, char *b, char *c);

extern float func_00282C10(float a);

extern void func_00283CF0(float a);

extern int func_002A7820(int lo, int hi);

extern float func_002A7940(void);

extern void func_L11_004507B0(void *a0, unsigned int color, float f, int rand, float a, int a3,
                              int a4, float one, void *a5);

extern void ref_L11_00458DF0_00(char *p);

extern void ref_L11_00458DF0_01(char *p, float scale);

extern long long func_0027C878(int id);

extern float func_00283248(float x, float y);

extern float func_00282E80(char *a0, float *a1);

extern void func_00283410(void *dst, void *src);

extern void func_00280B90(void *dst, void *src, int flag);

extern float D_L11_001BA780[];

extern int func_002ACBC8(void *a, int b);

extern void func_002829D8(void *a);

extern unsigned char D_L11_00189EA0[];

INCLUDE_ASM("asm/levels", func_L11_002C5E80);

INCLUDE_ASM("asm/levels", func_L11_002CDE10);

void func_L11_003404E0(char *a0)
{
    func_L02_00333480(a0, a0 + 0x10, a0 + 0xC0);
}

INCLUDE_ASM("asm/levels", func_L11_00397C08);

INCLUDE_ASM("asm/levels", func_L11_00397E68);

INCLUDE_ASM("asm/levels", func_L11_003CF250);

INCLUDE_ASM("asm/levels", func_L11_003F4198);

INCLUDE_ASM("asm/levels", func_L11_00414660);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_0041E460(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

void func_L11_00439D78(float a, float b, float c, float d)
{
    float v = (d + d) * (a - b);

    func_00283CF0((c + func_00282C10(c * c + v)) / d);
}

INCLUDE_ASM("asm/levels", func_L11_0044FFA0);

/* Draws the record's two markers, the scaled radius and the two colours, for each of the
   two record kinds the byte at 0x20 selects. */
void func_L11_00450508(unsigned char *p)
{
    switch (p[0x20]) {
    case 0:
        func_L11_004507B0(p + 0x10, 0x103080, *(float *)(p + 0x2C) * 45.0f,
                         func_002A7820(95, 111), func_002A7940(), 11, 2, 1.0f, p);
        func_L11_004507B0(p + 0x10, 0x3F3F3F, *(float *)(p + 0x2C) * 22.5f,
                         func_002A7820(111, 127), func_002A7940(), 11, 2, 1.0f, p);
        break;
    case 1:
        func_L11_004507B0(p + 0x10, 0x103080, *(float *)(p + 0x2C) * 45.0f,
                         func_002A7820(95, 111), func_002A7940(), 11, 2, 1.0f, p);
        func_L11_004507B0(p + 0x10, 0x3F3F3F, *(float *)(p + 0x2C) * 22.5f,
                         func_002A7820(111, 127), func_002A7940(), 11, 2, 1.0f, p);
        break;
    }
}

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_004506C0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L11_004507B0);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_00458D00(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

/* The 224-byte local record the level's camera update fills in. */

/* The record's reset and its four-vector scaler: a different function in every
 * level that holds them, so each place resolves its own target. */

/* The level's table of floats. */

/* Fills a local record: the level's handle, the four blocks it scales, and the
 * two vectors the level's own table gives against the object's position. */
void func_L11_00458DF0(char *a0, void *a1, void *a2, int a3, int t0, float f12, float f13)
{
    LOC_L11_458DF0 local;
    float x;

    ref_L11_00458DF0_00((char *)&local);
    local.v120 = func_0027C878(a3);
    local.v128 = 0xFF9000000260LL;
    local.v136 = ((long long)t0 << 2) | 0x0000008000000040LL;
    local.k3 = (int)a1 + ((int)a2 << 24);
    local.k2 = (int)a1 + ((int)a2 << 24);
    local.k1 = (int)a1 + ((int)a2 << 24);
    local.k0 = (int)a1 + ((int)a2 << 24);
    local.v112 = 0;
    local.v220 = 0.0f;
    local.f216 = func_00283248(D_L11_001BA780[0x50] - *(float *)a0,
                                D_L11_001BA780[0x51] - *(float *)(a0 + 4));
    x = func_00283248(func_00282E80(a0, D_L11_001BA780 + 0x50),
                      D_L11_001BA780[0x52] - *(float *)(a0 + 8));
    local.f212 = -x;
    local.v208 = f13;
    func_00283410(&local.m00, &local.v208);
    local.v192 = *(float *)a0;
    local.v196 = *(float *)(a0 + 4);
    local.v200 = *(float *)(a0 + 8);
    local.v204 = 1.0f;
    ref_L11_00458DF0_01((char *)&local, f12);
    func_00280B90(&local, &local.m00, 0);
}

INCLUDE_ASM("asm/levels", func_L11_0045FEF8);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_0045FFB0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L11_00460258);

INCLUDE_ASM("asm/levels", func_L11_0046B9F0);

INCLUDE_ASM("asm/levels", func_L11_0046BC08);

INCLUDE_ASM("asm/levels", func_L11_0046BD18);

INCLUDE_ASM("asm/levels", func_L11_0046C1E0);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_00472008(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L11_00472090);

void func_L11_00474888(char *a0)
{
    if (func_002ACBC8(D_L11_00189EA0, *(int *)(*(char **)(a0 + 0x68))) == 0) return;
    if (*(int *)(D_L11_00189EA0 + 0x2214) != 0x82) return;
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
}

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_00478EF0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}
