#include "common.h"

extern void func_L01_003DC8B8(u8 *, void *, void *);
extern void func_L01_003DC9E0(void *, void *);
extern void func_L01_003DC9A8(void **, s32 *);
extern s32 func_L01_00321178(void *, void *);
extern void func_L01_00318638(void *, void *, void *);

extern s32 func_310648();

extern f32 func_310580(f32 x, f32 y);

extern s32 func_310528(s32 a, s32 b);

extern f32 func_2E3A48(f32 x);

extern f32 func_2E3A60(f32 x);

extern void func_2E3470(void *a, void *b, void *c);

extern void func_32EF38(void *buf, s32 a1, s32 a2, s32 a3, f32 x, f32 y);

INCLUDE_ASM("asm/levels", func_L01_003DA740);

INCLUDE_ASM("asm/levels", func_L01_003DAB98);

INCLUDE_ASM("asm/levels", func_L01_003DAE58);

/* Scatters the measured primitive around the object, a1 times. */
INCLUDE_ASM("asm/levels", func_L01_003DBB88);

INCLUDE_ASM("asm/levels", func_L01_003DBCB8);

INCLUDE_ASM("asm/levels", func_L01_003DBF38);

INCLUDE_ASM("asm/levels", func_L01_003DC078);

/* func_L01_003DC838 is defined in L01_003DA740-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L01_003DC8B8);

void func_L01_003DC9A8(void **a0, s32 *a1) {
    s32 i;

    for (i = 0; i < *a1; i++) {
        *(s32 *)((u8 *)*a0 + 0x9C) = 0;
        a0++;
    }
}

INCLUDE_ASM("asm/levels", func_L01_003DC9E0);

INCLUDE_ASM("asm/levels", func_L01_003DCD40);

INCLUDE_ASM("asm/levels", func_L01_003DCDD8);

INCLUDE_ASM("asm/levels", func_L01_003DCE70);

INCLUDE_ASM("asm/levels", func_L01_003DCF40);

INCLUDE_ASM("asm/levels", func_L01_003DD3A0);

/* func_L01_003DD6A8 */
extern void func_002AB8C0(void *, void *);

extern void func_00282978(void *);

extern s32 func_00282948(void *);

extern void func_002B0990(void *, void *, s32, s32, f32);

void func_L01_003DD6A8(u8 *a0)
{
    u32 v;
    u8 *p = *(u8 **)(a0 + 0x68);

    func_002AB8C0(a0, p + 0x100);
    v = *(u16 *)(a0 + 0x96);
    if (v <= 0xFFFE) {
        *(u16 *)(a0 + 0x96) = v + 1;
    }
    func_00282978(p + 0x3D8);
    func_00282948(p + 0x404);
    func_00282978(p + 0x3D6);
    v = *(s16 *)(p + 0x3DA);
    if (v != 0) {
        *(u16 *)(p + 0x3D6) = 0x168;
        *(s16 *)(p + 0x3DA) = 0;
    }
    func_002B0990(a0, p + 0x300, 1, 1, 1.8f);
}

INCLUDE_ASM("asm/levels", func_L01_003E8ED8);

INCLUDE_ASM("asm/levels", func_L01_003EC0C0);

INCLUDE_ASM("asm/levels", func_L01_003ECCF8);

INCLUDE_ASM("asm/levels", func_L01_00405300);

INCLUDE_ASM("asm/levels", func_L01_0040EED0);

INCLUDE_ASM("asm/levels", func_L01_0040F118);

void func_L01_0040F340(u8 *a0) {
    if (*(u8 *)(a0 + 0x20) == 0) {
        *(u8 *)(a0 + 0x30) = 0xFF;
        *(u8 *)(a0 + 0x20) = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L01_0040F360);

INCLUDE_ASM("asm/levels", func_L01_0040F5A8);

INCLUDE_ASM("asm/levels", func_L01_0040F958);

INCLUDE_ASM("asm/levels", func_L01_00410AD0);

INCLUDE_ASM("asm/levels", func_L01_00412E00);

INCLUDE_ASM("asm/levels", func_L01_00412F18);

INCLUDE_ASM("asm/levels", func_L01_00416440);

INCLUDE_ASM("asm/levels", func_L01_00416A50);

/* Arms the measured primitive block of the given object. */
INCLUDE_ASM("asm/levels", func_L01_00416B30);

INCLUDE_ASM("asm/levels", func_L01_00416C08);

INCLUDE_ASM("asm/levels", func_L01_00416E78);

INCLUDE_ASM("asm/levels", func_L01_004171C8);

void func_L01_0041C960(u8 *a0) {
    *(u16 *)(a0 + 0x34) |= 0x802;
}

INCLUDE_ASM("asm/levels", func_L01_0041C970);

INCLUDE_ASM("asm/levels", func_L01_0042C450);

INCLUDE_ASM("asm/levels", func_L01_0042C540);

INCLUDE_ASM("asm/levels", func_L01_0042C8D8);

/* func_L01_0042CE78 */
typedef struct L01_0042CE78_ENTRY {
    char pad0[0x38];
    u8 f38;
    u8 f39;
} L01_0042CE78_ENTRY;

typedef struct L01_0042CE78_OBJ {
    char pad0[0x84];
    s16 f84;
} L01_0042CE78_OBJ;

extern u8 *D_L01_001B1F04 __attribute__((sda));

extern s32 func_L00_004290C0(void *);

s32 func_L01_0042CE78(L01_0042CE78_OBJ *a0, u8 *a1)
{
    L01_0042CE78_ENTRY *e = *(L01_0042CE78_ENTRY **)(D_L01_001B1F04 + a0->f84 * 0x20 + 0x1C);
    s32 r;

    if (e->f39 != 0) {
        r = -1;
    } else if (e->f38 != 0) {
        r = 1;
    } else if (a1 != 0 && *(s16 *)(a1 + 0x7E) == 0) {
        r = 0;
    } else {
        if (func_L00_004290C0(a0) != 0) {
            goto one;
        }
        goto zero;
    one:
        r = 1;
        return r;
    zero:
        r = 0;
        return r;
    }
    return r;
}
