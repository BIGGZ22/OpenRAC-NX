#include "include_asm.h"
#include "common.h"

/* Quadword copies and clears, the spelling that reaches lq/sq. */
typedef int TI128 __attribute__((mode(TI)));

INCLUDE_ASM("asm/levels", func_L07_003F4578);

extern u8 D_L07_001B31AF __attribute__((sda));

void func_L07_0040FC50(void)
{
    u8 *p;
    s32 i;

    for (i = 15, p = &D_L07_001B31AF; i >= 0; i--) {
        *p = 0;
        p--;
    }
}

/* func_L07_003F4970 is defined in L07_003F4578-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L07_003F4B08);

INCLUDE_ASM("asm/levels", func_L07_003F4C18);

INCLUDE_ASM("asm/levels", func_L07_003F4CC0);

INCLUDE_ASM("asm/levels", func_L07_003F59C0);

INCLUDE_ASM("asm/levels", func_L07_003F5B50);

INCLUDE_ASM("asm/levels", func_L07_003F6328);

extern void func_3109C8(u8 *a, s32 b, s32 c);

extern f32 func_2E49C8(f32 a, f32 b);

extern f32 func_2E5360(f32 a, f32 b);

extern f32 D_L07_00189E20[];

extern void func_L07_003F68F0(u8 *a0, f32 x);

void func_L07_003F6528(u8 *a0)
{
    u8 *s0;
    f32 r;

    func_3109C8(a0, 2, -1);
    s0 = *(u8 **)(a0 + 0x68);
    *(u8 *)(a0 + 0x7F) = 0;
    *(f32 *)(s0 + 0x20) = (f32)*(s16 *)(s0 + 0x24);
    r = func_2E5360(func_2E49C8(D_L07_00189E20[0x20] - *(f32 *)(a0 + 0x10),
                                D_L07_00189E20[0x21] - *(f32 *)(a0 + 0x14)),
                    *(f32 *)(s0 + 0x60));
    if (r > 0.0f) {
        *(f32 *)(s0 + 0x54) = -1.5707964f;
    } else {
        *(f32 *)(s0 + 0x54) = 1.5707964f;
    }
    func_L07_003F68F0(a0, 0.0f);
}

INCLUDE_ASM("asm/levels", func_L07_003F65E8);

INCLUDE_ASM("asm/levels", func_L07_003F67B8);

/* func_L07_003F68F0 is defined in L07_003F4578-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L07_003F6960);

INCLUDE_ASM("asm/levels", func_L07_003F6C40);

/* func_L07_003F6CF0 is defined in L07_003F4578-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L07_003F6DC0);

INCLUDE_ASM("asm/levels", func_L07_003F7048);

/* An entity node: the flag word at 0x34, the counter at 0x98, two children. */
typedef struct SibNode SibNode;

struct SibNode {
    u8 pad00[0x34];
    u16 flags34;
    u8 pad36[0x68 - 0x36];
    SibNode *f68;
    u8 rest68[0x98 - 0x6C];
    s32 f98;
    u8 rest9c[0x410 - 0x9C];
    SibNode *f410;
    SibNode *f414;
};

void func_L07_003F78F8(SibNode *a0)
{
    SibNode *p = a0->f68;

    a0->flags34 = (a0->flags34 | 0x41) & 0xEFFF;
    a0->f98 = 0;
    p->f410->flags34 |= 0x41;
    p->f410->flags34 &= 0xEFFF;
    p->f410->f98 = 0;
    p->f414->flags34 |= 0x41;
    p->f414->flags34 &= 0xEFFF;
    p->f414->f98 = 0;
}

INCLUDE_ASM("asm/levels", func_L07_003F7968);

u8 *func_L07_003F7BE0(u8 *a0)
{
    return *(u8 **)(*(u8 **)(a0 + 0x68) + 0x420);
}

INCLUDE_ASM("asm/levels", func_L07_003F7BF0);

extern void func_002A93B0(u8 *a);

extern void func_002A0F48(u8 *a);

extern void func_0029FC20(u8 *a, s32 b, void *c);

extern void func_00283678(u8 *a, void *b);

INCLUDE_ASM("asm/levels", func_L07_003F8010);

INCLUDE_ASM("asm/levels", func_L07_003F80B8);

INCLUDE_ASM("asm/levels", func_L07_003F8430);

/* func_L07_003F8780 is defined in L07_003F4578-g8.c (this unit has no $gp access for it). */

extern void func_00282CF0(u8 *a, u8 *b, u8 *c);

extern void func_002AFDD8(u8 *a, u8 *b, u8 *c, s32 d);

INCLUDE_ASM("asm/levels", func_L07_003F8828);

extern void func_00283098(u8 *a, u8 *b, u8 *c);

extern void func_00282CC0(u8 *a, u8 *b, u8 *c);

INCLUDE_ASM("asm/levels", func_L07_003F8898);

INCLUDE_ASM("asm/levels", func_L07_003F8908);

INCLUDE_ASM("asm/levels", func_L07_003F8AB0);

extern void func_002E5CF8(s32 a, s32 b, u8 *c);

extern void func_0029FCC0(u8 *a, s32 b, void *c);

extern void func_L07_003F43E0(u8 *a, u8 *b, void *c, void *d, f32 e);


extern u8 *func_L07_003F6B88(u8 *a);

extern void func_L07_003F6C40(u8 *a, u8 *b, void *c, f32 d);

/* One 128-byte record, its float field read by the measured index. */
typedef struct {
    u8 pad38[0x38];
    f32 f38;
    u8 rest[128 - 0x3C];
} SibRec128;

extern SibRec128 *D_L07_001B23CC;

INCLUDE_ASM("asm/levels", func_L07_003F8C38);

INCLUDE_ASM("asm/levels", func_L07_003F8CD0);

INCLUDE_ASM("asm/levels", func_L07_003F8D90);

extern void func_002A7470(u8 *a0, s32 a1, s32 a2, s32 a3);

void func_L07_003F9028(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);

    if (*(u32 *)(p + 0x420) == 0) {
        if (*(u8 *)(a0 + 0x43) != 2) {
            func_002A7470(a0, 2, 0, 10);
        }
        return;
    }
    if (*(u8 *)(a0 + 0x43) != 8) {
        func_002A7470(a0, 8, 0, 10);
    }
}

void func_L07_003F9088(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);

    if (*(u32 *)(p + 0x420) == 0) {
        if (*(u8 *)(a0 + 0x43) != 3) {
            func_002A7470(a0, 3, 0, 10);
        }
        return;
    }
    if (*(u8 *)(a0 + 0x43) != 9) {
        func_002A7470(a0, 9, 0, 10);
    }
}

extern u8 D_001A7BD0 __attribute__((sda));

extern void func_L01_003C91C8(u8 *a0);

void func_L07_003FF618(u8 *a0)
{
    if (D_001A7BD0 != 0) {
        *(u16 *)(a0 + 0x34) |= 0x2;
        return;
    }
    func_L01_003C91C8(a0);
}

INCLUDE_ASM("asm/levels", func_L07_004080E0);

INCLUDE_ASM("asm/levels", func_L07_00408338);

INCLUDE_ASM("asm/levels", func_L07_0040E198);

INCLUDE_ASM("asm/levels", func_L07_0040E980);

INCLUDE_ASM("asm/levels", func_L07_0040EC08);

INCLUDE_ASM("asm/levels", func_L07_0040EFC0);

extern void func_338D68(void *a, s32 b, void *c, void *d, s32 e, f32 f);

extern f32 func_2E4508(void *a, void *b);

void func_L07_0040EFC8(s32 *a0, f32 *a1, f32 a, f32 b)
{
    f32 p2[4];
    f32 p1[4];
    f32 tmp[4];
    f32 d = a - b;
    f32 s;

    if (d < 0.0f)
        d = d + (f32)a0[0];
    s = a + b;
    if ((f32)a0[0] < s)
        s = s - (f32)a0[0];
    func_338D68(a0, 1, p1, tmp, 0, d);
    func_338D68(a0, 1, p2, tmp, 0, s);
    a1[2] = func_2E49C8(p1[0] - p2[0], p1[1] - p2[1]);
    a1[1] = -func_2E49C8(func_2E4508(p2, p1), p1[2] - p2[2]);
}

INCLUDE_ASM("asm/levels", func_L07_0040F0D0);

INCLUDE_ASM("asm/levels", func_L07_0040F468);

INCLUDE_ASM("asm/levels", func_L07_0040F598);

/* func_L07_0040F6B8 is defined in L07_003F4578-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L07_0040F760);


INCLUDE_ASM("asm/levels", func_L07_0040F8F8);

INCLUDE_ASM("asm/levels", func_L07_0040FA90);

INCLUDE_ASM("asm/levels", func_L07_0040FC88);

INCLUDE_ASM("asm/levels", func_L07_00410588);

extern void func_002833F0(u8 *a, u8 *b);

extern void func_L00_00323958(u8 *a, u8 *b);

void func_L07_004106B8(u8 *a0)
{
    u8 *s1 = *(u8 **)(a0 + 0x68);

    func_002833F0(a0 + 0xC0, a0 + 0xF0);
    func_L00_00323958(a0, s1 + 0x1B0);
}

INCLUDE_ASM("asm/levels", func_L07_00410700);

INCLUDE_ASM("asm/levels", func_L07_00410930);

INCLUDE_ASM("asm/levels", func_L07_00410C50);
