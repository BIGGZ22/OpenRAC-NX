#include "include_asm.h"
#include "common.h"

/* Quadword copies and clears, the spelling that reaches lq/sq. */
typedef int TI128 __attribute__((mode(TI)));

INCLUDE_ASM("asm/levels", func_L07_003F4578);

extern u8 D_L07_001B31AF;

INCLUDE_ASM("asm/levels", func_L07_0040FC50);

INCLUDE_ASM("asm/levels", func_L07_003F4970);

INCLUDE_ASM("asm/levels", func_L07_003F4B08);

void func_L07_003F4C18(u8 *a0, u8 *a1, u8 *a2)
{
    u8 *s0 = *(u8 **)(a0 + 0x68);

    if (*(u8 **)(s0 + 0x20) != 0) {
        func_00283098(a1, s0, *(u8 **)(s0 + 0x20) + 0xC0);
        func_00282CC0(a1, a1, *(u8 **)(s0 + 0x20) + 0x10);
        func_00283098(a2, s0 + 0x10, *(u8 **)(s0 + 0x20) + 0xC0);
        func_00282CC0(a2, a2, *(u8 **)(s0 + 0x20) + 0x10);
        return;
    }
    qcopy(a1, s0);
    qcopy(a2, s0 + 0x10);
}

INCLUDE_ASM("asm/levels", func_L07_003F4CC0);

INCLUDE_ASM("asm/levels", func_L07_003F59C0);

INCLUDE_ASM("asm/levels", func_L07_003F5B50);

INCLUDE_ASM("asm/levels", func_L07_003F6328);

INCLUDE_ASM("asm/levels", func_L07_003F6528);

INCLUDE_ASM("asm/levels", func_L07_003F65E8);

INCLUDE_ASM("asm/levels", func_L07_003F67B8);

INCLUDE_ASM("asm/levels", func_L07_003F68F0);

INCLUDE_ASM("asm/levels", func_L07_003F6960);

INCLUDE_ASM("asm/levels", func_L07_003F6C40);

INCLUDE_ASM("asm/levels", func_L07_003F6CF0);

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

INCLUDE_ASM("asm/levels", func_L07_003F7BE0);

INCLUDE_ASM("asm/levels", func_L07_003F7BF0);

extern void func_002A93B0(u8 *a);

extern void func_002A0F48(u8 *a);

extern void func_0029FC20(u8 *a, s32 b, void *c);

extern void func_00283678(u8 *a, void *b);

void func_L07_003F8010(u8 *a0)
{
    u8 *s1 = *(u8 **)(a0 + 0x68);
    u8 *x = *(u8 **)(s1 + 0x410);
    u8 a[0x30];
    u8 b[0x10];

    if (x != 0) {
        qcopy(x + 0x10, a0 + 0x10);
        qcopy(x + 0xF0, a0 + 0xF0);
        func_002A93B0(x);
    }
    func_002A0F48(*(u8 **)(s1 + 0x410));
    if (*(u8 **)(s1 + 0x414) != 0) {
        func_0029FC20(a0, 0, a);
        func_00283678(*(u8 **)(s1 + 0x414) + 0xC0, a);
        qcopy(*(u8 **)(s1 + 0x414) + 0x10, b);
    }
    func_002A0F48(*(u8 **)(s1 + 0x414));
}

INCLUDE_ASM("asm/levels", func_L07_003F80B8);

INCLUDE_ASM("asm/levels", func_L07_003F8430);

INCLUDE_ASM("asm/levels", func_L07_003F8780);

extern void func_00282CF0(u8 *a, u8 *b, u8 *c);

extern void func_002AFDD8(u8 *a, u8 *b, u8 *c, s32 d);

void func_L07_003F8828(u8 *a0, u8 *a1)
{
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x95) == 0) {
        u8 *s0 = s1 + 0x400;

        func_00282CF0(s0, a1, *(u8 **)(s1 + 0x418) + 0x10);
        func_002AFDD8(*(u8 **)(s1 + 0x418), s0, s0, 0);
        return;
    }
    qcopy(s1 + 0x400, a1);
}

extern void func_00283098(u8 *a, u8 *b, u8 *c);

extern void func_00282CC0(u8 *a, u8 *b, u8 *c);

void func_L07_003F8898(u8 *a0, u8 *a1)
{
    u8 *s0 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x95) == 0) {
        func_00283098(a1, s0 + 0x400, *(u8 **)(s0 + 0x418) + 0xC0);
        func_00282CC0(a1, a1, *(u8 **)(s0 + 0x418) + 0x10);
        return;
    }
    qcopy(a1, s0 + 0x400);
}

INCLUDE_ASM("asm/levels", func_L07_003F8908);

INCLUDE_ASM("asm/levels", func_L07_003F8AB0);

extern void func_002E5CF8(s32 a, s32 b, u8 *c);

extern void func_0029FCC0(u8 *a, s32 b, void *c);

extern void func_L07_003F43E0(u8 *a, u8 *b, void *c, void *d, f32 e);

void func_L07_003F8BB0(u8 *a0)
{
    u8 *s2 = *(u8 **)(a0 + 0x68);
    u8 a[16];
    u8 b[16];

    func_002E5CF8(3, 0, a0);
    func_0029FCC0(a0, 0, a);
    func_L07_003F8898(a0, b);
    func_L07_003F43E0(a0, *(u8 **)(s2 + 0x418), a, b, (f32)*(u8 *)(s2 + 0x35));
}

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

INCLUDE_ASM("asm/levels", func_L07_003F9028);

INCLUDE_ASM("asm/levels", func_L07_003F9088);

extern u8 D_001A7BD0;

extern void func_L01_003C91C8(u8 *a0);

INCLUDE_ASM("asm/levels", func_L07_003FF618);

INCLUDE_ASM("asm/levels", func_L07_004080E0);

INCLUDE_ASM("asm/levels", func_L07_00408338);

INCLUDE_ASM("asm/levels", func_L07_0040E198);

INCLUDE_ASM("asm/levels", func_L07_0040E980);

INCLUDE_ASM("asm/levels", func_L07_0040EC08);

INCLUDE_ASM("asm/levels", func_L07_0040EFC0);

INCLUDE_ASM("asm/levels", func_L07_0040EFC8);

INCLUDE_ASM("asm/levels", func_L07_0040F0D0);

INCLUDE_ASM("asm/levels", func_L07_0040F468);

INCLUDE_ASM("asm/levels", func_L07_0040F598);

INCLUDE_ASM("asm/levels", func_L07_0040F6B8);

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
