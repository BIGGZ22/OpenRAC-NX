#include "include_asm.h"

typedef int s32;

typedef unsigned int u32;

typedef unsigned char u8;

typedef unsigned short u16;

typedef unsigned long u64;

typedef short s16;

typedef float f32;

typedef struct {
    u32 unknown;
    s32 data_offset;
    s32 length;
    s32 extra;
} PackedHeaderCountView;

INCLUDE_ASM("asm/levels", func_L04_002CC748);

typedef struct {
    u8 pad00[0x7A8];
    f32 field7A8;
    f32 field7AC;
    f32 field7B0;
    f32 field7B4;
    u8 pad7B8[0x2290 - 0x7B8];
    u32 field2290;
    u32 field2294;
} D00189E20;
extern D00189E20 D_00189E20;
extern void func_002AEE70(u32 a0, void *a1, void *a2, f32 f0, f32 f1, f32 f2);
extern void func_002AEF28(u32 a0, void *a1, void *a2, f32 f0, f32 f1, f32 f2);
void func_L04_002CE810(void) {
    if (D_00189E20.field2294 != 0x7C)
        return;
    func_002AEE70(D_00189E20.field2290, &D_00189E20.field7A8, &D_00189E20.field7AC,
                  0.3f, 0.116355299949646f, -1.0f);
}

void func_L04_002CE870(void) {
    if (D_00189E20.field2294 != 0x7C)
        return;
    func_002AEF28(D_00189E20.field2290, &D_00189E20.field7B0, &D_00189E20.field7B4,
                  0.174532949924469f, 0.02617994323372841f, 0.03490659222006798f);
}

INCLUDE_ASM("asm/levels", func_L04_002D91B8);

INCLUDE_ASM("asm/levels", func_L04_002D9780);

INCLUDE_ASM("asm/levels", func_L04_002DB970);

INCLUDE_ASM("asm/levels", func_L04_002DBE70);

INCLUDE_ASM("asm/levels", func_L04_002DCE78);

#define NOT_SDA __attribute__((section(".data")))

extern int r1_D_0012FCF0 NOT_SDA;

/* rac1-decomp src/core/00118A40.c: func_00118EC0 */
void func_L04_002FB920(void) {
    r1_D_0012FCF0 = 0;
}

INCLUDE_ASM("asm/levels", func_L04_0030C4C0);

INCLUDE_ASM("asm/levels", func_L04_0030C948);

INCLUDE_ASM("asm/levels", func_L04_0030CC70);

INCLUDE_ASM("asm/levels", func_L04_0030EDE0);

INCLUDE_ASM("asm/levels", func_L04_0030FA30);

INCLUDE_ASM("asm/levels", func_L04_0031D630);

INCLUDE_ASM("asm/levels", func_L04_0032F1D8);

INCLUDE_ASM("asm/levels", func_L04_0032F340);

INCLUDE_ASM("asm/levels", func_L04_0032F3C0);

INCLUDE_ASM("asm/levels", func_L04_0032F62C);

INCLUDE_ASM("asm/levels", func_L04_0032F994);

INCLUDE_ASM("asm/levels", func_L04_0033C420);

INCLUDE_ASM("asm/levels", func_L04_0033CB08);

/* sched1 hoists the last statement of */
void func_L04_0033CB88(char *p)
{
    *(int *)(p + 0) = 640;
    *(float *)(p + 8) = 0.5f;
    *(int *)(p + 20) = 0;
    *(float *)(p + 4) = 1.0f;
    *(float *)(p + 12) = 0.5f;
    *(int *)(p + 16) = 0;
    *(float *)(p + 36) = 2.7f;
    *(int *)(p + 24) = 0;
    *(float *)(p + 32) = 0.3f;
    *(float *)(p + 28) = 0.3f;
    *(float *)(p + 40) = 1.57f;
    *(float *)(p + 44) = 0.1f;
}

INCLUDE_ASM("asm/levels", func_L04_0033CC00);

INCLUDE_ASM("asm/levels", func_L04_0033D0C0);

INCLUDE_ASM("asm/levels", func_L04_0033D158);

INCLUDE_ASM("asm/levels", func_L04_0033D4B0);

INCLUDE_ASM("asm/levels", func_L04_003429F8);

extern void func_00282D60(void *buf, u32 value, float scale);
extern void func_00283DB8(void *buf, float value);
void func_L04_00343498(u32 value, float f) {
    u8 buf[16];
    func_00282D60(buf, value, 0.0009765625f);
    func_00283DB8(buf, f);
}

INCLUDE_ASM("asm/levels", func_L04_00344568);

INCLUDE_ASM("asm/levels", func_L04_00347AB8);

INCLUDE_ASM("asm/levels", func_L04_0034B9B0);

INCLUDE_ASM("asm/levels", func_L04_003524D0);

INCLUDE_ASM("asm/levels", func_L04_00358868);

INCLUDE_ASM("asm/levels", func_L04_00358AE8);

INCLUDE_ASM("asm/levels", func_L04_0035F3E0);

INCLUDE_ASM("asm/levels", func_L04_00362768);

INCLUDE_ASM("asm/levels", func_L04_00362940);

INCLUDE_ASM("asm/levels", func_L04_00362A50);

INCLUDE_ASM("asm/levels", func_L04_00363690);

INCLUDE_ASM("asm/levels", func_L04_00363CA0);

INCLUDE_ASM("asm/levels", func_L04_00365300);

INCLUDE_ASM("asm/levels", func_L04_00365B48);

INCLUDE_ASM("asm/levels", func_L04_00378838);

INCLUDE_ASM("asm/levels", func_L04_0039FA08);

INCLUDE_ASM("asm/levels", func_L04_0039FC28);

INCLUDE_ASM("asm/levels", func_L04_0039FEA8);

INCLUDE_ASM("asm/levels", func_L04_0039FEF0);

INCLUDE_ASM("asm/levels", func_L04_003A0008);

INCLUDE_ASM("asm/levels", func_L04_003A00E8);

extern void func_002FD1B8(u32 a0, u32 a1, void *a2);
void func_L04_003A02A0(u32 a0, u32 a1) {
    func_002FD1B8(a0, a1, 0);
}

INCLUDE_ASM("asm/levels", func_L04_003A0C40);

INCLUDE_ASM("asm/levels", func_L04_003A0CE8);

INCLUDE_ASM("asm/levels", func_L04_003A0FC8);

INCLUDE_ASM("asm/levels", func_L04_003A11F8);

INCLUDE_ASM("asm/levels", func_L04_003A1240);

INCLUDE_ASM("asm/levels", func_L04_003A14B8);

/* returns the packed header's signed rounded sixteen-byte block count in word units */
s32 func_L04_003A1540(const PackedHeaderCountView *header) {
    s32 fixed = header->extra + 16;
    s32 total = header->data_offset + fixed + header->length;
    return ((total + 15) / 16) * 4;
}

INCLUDE_ASM("asm/levels", func_L04_003A1578);

INCLUDE_ASM("asm/levels", func_L04_003A15C0);

INCLUDE_ASM("asm/levels", func_L04_003A1868);

INCLUDE_ASM("asm/levels", func_L04_003A19E8);

void func_L04_003A1A48(u8 *dst, u16 *src, s32 count) {
    if (count <= 0)
        return;
    dst += 0x1E;
    do {
        *(u16 *)dst = *src;
        src++;
        dst += 0x1190;
        count--;
    } while (count != 0);
}

INCLUDE_ASM("asm/levels", func_L04_003A1A78);

INCLUDE_ASM("asm/levels", func_L04_003A1C98);

extern u32 func_00282978(u8 *ptr);
void func_L04_003A1FA0(u8 *self) {
    if (func_00282978((u8 *)(*(u32 *)(*(u32 *)(self + 0x28) + 0x68) + 0x16)) != 0)
        *(u8 *)(*(u32 *)(self + 0x28) + 0x20) = 1;
}

INCLUDE_ASM("asm/levels", func_L04_003A1FE0);

INCLUDE_ASM("asm/levels", func_L04_003A21F8);

typedef struct {
    u32 value;
    float factor;
    u32 pad[2];
} TblEntry;
typedef struct {
    TblEntry entries[64];
    u32 count;
} Tbl;
extern Tbl D_L04_00231E00;
void func_L04_003A5C08(u32 value, float factor) {
    D_L04_00231E00.entries[D_L04_00231E00.count].value = value;
    D_L04_00231E00.entries[D_L04_00231E00.count].factor = factor;
    D_L04_00231E00.count++;
}

INCLUDE_ASM("asm/levels", func_L04_003B3168);

INCLUDE_ASM("asm/levels", func_L04_003B3580);

INCLUDE_ASM("asm/levels", func_L04_003C3BA0);

INCLUDE_ASM("asm/levels", func_L04_003C4050);

INCLUDE_ASM("asm/levels", func_L04_003C4858);

INCLUDE_ASM("asm/levels", func_L04_003C49C0);

INCLUDE_ASM("asm/levels", func_L04_003C6FE8);

extern u32 D_L04_001B2E84 __attribute__((sda));

/* adds VALUE to the running total and returns the total as it was before */
u32 func_L04_003C70C8(u32 value) {
    u32 old = D_L04_001B2E84;
    D_L04_001B2E84 = old + value;
    return old;
}
