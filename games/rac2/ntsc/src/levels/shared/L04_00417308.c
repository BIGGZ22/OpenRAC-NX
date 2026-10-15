#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef unsigned int u32;

typedef short s16;

typedef int s32;

typedef float f32;

INCLUDE_ASM("asm/levels", func_L04_00417308);

INCLUDE_ASM("asm/levels", func_L04_00418A58);

INCLUDE_ASM("asm/levels", func_L04_00418BF0);

INCLUDE_ASM("asm/levels", func_L04_004195E0);

INCLUDE_ASM("asm/levels", func_L04_00419918);

INCLUDE_ASM("asm/levels", func_L04_0041AD18);

INCLUDE_ASM("asm/levels", func_L04_00423178);

INCLUDE_ASM("asm/levels", func_L04_004256B0);

INCLUDE_ASM("asm/levels", func_L04_00432258);

INCLUDE_ASM("asm/levels", func_L04_004323F8);

INCLUDE_ASM("asm/levels", func_L04_00432530);

INCLUDE_ASM("asm/levels", func_L04_00434AD0);

INCLUDE_ASM("asm/levels", func_L04_0043BC58);

extern void func_002833F0(char *a, char *b);

extern void func_00282D30(char *a, char *b, float f);

extern int func_00283D38(int a, int b, float f);

extern void func_002A0488(void *a, u8 b, u8 c, u8 d);

extern void func_002A10E8(void *a);

void func_L04_0043BE40(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);
    char *p;
    char *r;
    char *t;
    char *u;
    float f;
    s32 v;

    p = *(char **)(s0 + 0x48);
    *(u16 *)(p + 0x34) &= 0xFFFE;
    *(u8 *)(*(char **)(s0 + 0x48) + 0x31) = 1;
    r = *(char **)(s0 + 0x48);
    *(s32 *)(r + 0x98) = *(s32 *)(*(s32 *)(r + 0x24) + 0x10);
    t = *(char **)(s0 + 0x48);
    func_002833F0(t + 0xC0, t + 0xF0);
    u = *(char **)(s0 + 0x48);
    f = *(float *)(s0 + 0x58);
    func_00282D30(u + 0xE0, u + 0xE0, f);
    f = *(float *)(s0 + 0x58);
    v = func_00283D38(0x800000FF, *(s32 *)(s0 + 0x54), f);
    func_002A0488(*(char **)(s0 + 0x48), v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF);
    func_002A10E8(*(char **)(s0 + 0x48));
}

INCLUDE_ASM("asm/levels", func_L04_0043BEE8);

INCLUDE_ASM("asm/levels", func_L04_00451150);

INCLUDE_ASM("asm/levels", func_L04_00454BF8);

INCLUDE_ASM("asm/levels", func_L04_00455418);

/* evaluates the ordered scalar cubic value squared times negative two times value plus three */
f32 func_L04_004554F8(f32 value) {
    f32 squared = value * value;
    value = value * -2.0f;
    value = value + 3.0f;
    return squared * value;
}

extern void func_33B9B0(char *a0, int a1, int a2);

extern int func_33E6B0(char *a0, int a1, int a2);

extern void func_348788(char *a0, int a1, float f12);

extern void func_382DF8(int a0, int a1, char *a2);

extern void func_345860(char *a0, int a1, float f12, float f13, float f14, float f15);

extern void func_3317E8(char *a0);

void func_L04_00456748(char *s0)
{
    char *s1 = *(char **)(s0 + 0x68);

    switch (*(unsigned char *)(s0 + 0x20)) {
    case 0:
        func_33B9B0(s0, 1, -1);
        break;
    case 1:
        if (func_33E6B0(s0, 0x4010000, 0) != 0 && *(int *)s1 == 0)
            func_33B9B0(s0, 2, -1);
        *(unsigned char *)(s0 + 0xA8) = 0xFF;
        break;
    case 2:
        func_348788(s0, 1, 0.0f);
        func_382DF8(0, 0, s0);
        func_345860(s0, 0x1E, 0.4f, 4.4f, 1.0f, 0.083333333f);
        func_3317E8(s0);
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L04_00456860);

INCLUDE_ASM("asm/levels", func_L04_00459408);

/* rac1-decomp src/game/stub.c: func_001E97C8 */
/* Takes an argument (callers pass one, e.g. r1_func_001EC210) and ignores
   it -- the unused parameter costs no codegen, still 0/8. */
int func_L04_0046B740(void *arg0) {
    return 0;
}

/* func_L04_0046CD68 is defined in L04_00417308-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L04_0046CE68);

INCLUDE_ASM("asm/levels", func_L04_0046CFB8);

INCLUDE_ASM("asm/levels", func_L04_0046D250);

INCLUDE_ASM("asm/levels", func_L04_0046D6A8);

extern s32 D_L04_001B1E84 __attribute__((sda));

extern s32 D_L04_001B24DC __attribute__((sda));

int func_L04_0046D768(u8 *a0)
{
    s32 v1 = (*(s16 *)(a0 + 0x84) << 5) + D_L04_001B1E84;
    u8 *a1 = (u8 *)(D_L04_001B24DC + (*(s32 *)(*(s32 *)(v1 + 0x1C) + 0x60) << 8));
    return (u32)(*(u8 *)(a1 + 0x20) - 2) < 2;
}

void func_L04_0047A7C8(void)
{
}

void func_L04_0047B420(void)
{
}

INCLUDE_ASM("asm/levels", func_L04_00482D50);

INCLUDE_ASM("asm/levels", func_L04_004834A0);

void func_L04_00487228(void)
{
}

void func_L04_004895B8(void)
{
}

INCLUDE_ASM("asm/levels", func_L04_0048A968);

void func_L04_0048AE50(void)
{
}
