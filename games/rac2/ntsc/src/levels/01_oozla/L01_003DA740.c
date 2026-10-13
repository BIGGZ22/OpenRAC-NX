#include "common.h"

extern void func_L01_003DC8B8(u8 *, void *, void *);
extern void func_L01_003DC9E0(void *, void *);
extern void func_L01_003DC9A8(void **, s32 *);
extern s32 func_L01_00321178(void *, void *);
extern void func_L01_00318638(void *, void *, void *);

INCLUDE_ASM("asm/levels", func_L01_003DA740);

INCLUDE_ASM("asm/levels", func_L01_003DAB98);

INCLUDE_ASM("asm/levels", func_L01_003DAE58);

INCLUDE_ASM("asm/levels", func_L01_003DBB88);

INCLUDE_ASM("asm/levels", func_L01_003DBCB8);

INCLUDE_ASM("asm/levels", func_L01_003DBF38);

INCLUDE_ASM("asm/levels", func_L01_003DC078);

INCLUDE_ASM("asm/levels", func_L01_003DC838);

INCLUDE_ASM("asm/levels", func_L01_003DC8B8);

void func_L01_003DC9A8(void **a0, s32 *a1) {
    s32 i;

    for (i = 0; i < *a1; i++) {
        *(s32 *)((u8 *)*a0 + 0x9C) = 0;
        a0++;
    }
}

INCLUDE_ASM("asm/levels", func_L01_003DC9E0);

void func_L01_003DCD40(u8 *a0, void *a1) {
    u8 *o = (u8 *)*(s32 **)(a0 + 0x68);
    s32 list[0x10];
    u8 vec[0x10];
    s32 count;

    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC8B8(a0, list, &count);
    }
    qcopy(vec, a0 + 0x10);
    func_L01_00318638(a0, a1, o + 0x1C0);
    func_L01_003DC9E0(a0, vec);
    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC9A8((void **)list, &count);
    }
}

s32 func_L01_003DCDD8(u8 *a0, void *a1) {
    u8 *o = (u8 *)*(s32 **)(a0 + 0x68);
    s32 list[0x10];
    u8 vec[0x10];
    s32 count;
    s32 r;

    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC8B8(a0, list, &count);
    }
    qcopy(vec, a0 + 0x10);
    r = func_L01_00321178(a0, a1);
    func_L01_003DC9E0(a0, vec);
    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC9A8((void **)list, &count);
    }
    return r;
}

INCLUDE_ASM("asm/levels", func_L01_003DCE70);

INCLUDE_ASM("asm/levels", func_L01_003DCF40);

INCLUDE_ASM("asm/levels", func_L01_003DD3A0);

INCLUDE_ASM("asm/levels", func_L01_003DD6A8);

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

INCLUDE_ASM("asm/levels", func_L01_0042CE78);
