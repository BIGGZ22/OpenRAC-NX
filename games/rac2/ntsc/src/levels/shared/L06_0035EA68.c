#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

typedef struct {
    s32 pad0;
    s32 pad1;
    s32 pad2;
    s32 fieldC;
} L06Entry;

extern L06Entry D_L06_00139A28[];

extern s32 func_L01_003246F8();

INCLUDE_ASM("asm/levels", func_L06_0035EA68);

void func_L06_0036F728(s32 a0, s32 a1) {
    s32 v1 = func_L01_003246F8(a0);
    if (v1 != -1)
        D_L06_00139A28[v1].fieldC = a1;
}

INCLUDE_ASM("asm/levels", func_L06_00382608);

extern float func_00282E48(char *, char *);
extern float func_00282C48(float);

s32 func_L06_00382850(char *a0, char *a1, float f12) {
    s32 i;
    s32 best = 0;
    float best_val = 1.0e11f;
    float base = f12;

    for (i = 0; i < *(s32 *)a1; i++) {
        float v = func_00282E48(a1 + 0x10 + i * 0x10, a0);
        if (func_00282C48(v - base) < best_val) {
            best_val = v;
            best = i;
        }
    }
    return best;
}

INCLUDE_ASM("asm/levels", func_L06_003829D8);

INCLUDE_ASM("asm/levels", func_L06_0039A2F0);

INCLUDE_ASM("asm/levels", func_L06_003E0D88);

INCLUDE_ASM("asm/levels", func_L06_003E1040);

INCLUDE_ASM("asm/levels", func_L06_003E11A0);

INCLUDE_ASM("asm/levels", func_L06_003E1498);

s32 func_L06_003E3CC8(u8 *a0) {
    if (*(s16 *)(a0 + 0xAA) != 0x330)
        return 0;
    return (u32)(a0[0x20] - 2) < 5;
}

INCLUDE_ASM("asm/levels", func_L06_003E3CF0);

INCLUDE_ASM("asm/levels", func_L06_003E3ED0);

INCLUDE_ASM("asm/levels", func_L06_003E3F28);

INCLUDE_ASM("asm/levels", func_L06_003E4BE0);

INCLUDE_ASM("asm/levels", func_L06_003E4F08);

INCLUDE_ASM("asm/levels", func_L06_003E4FA0);

INCLUDE_ASM("asm/levels", func_L06_003E5428);

INCLUDE_ASM("asm/levels", func_L06_003E57E8);

INCLUDE_ASM("asm/levels", func_L06_003E5D30);

INCLUDE_ASM("asm/levels", func_L06_004005F8);

INCLUDE_ASM("asm/levels", func_L06_004009B8);

INCLUDE_ASM("asm/levels", func_L06_00402570);

INCLUDE_ASM("asm/levels", func_L06_00403130);

INCLUDE_ASM("asm/levels", func_L06_00403710);

INCLUDE_ASM("asm/levels", func_L06_00403DA8);

INCLUDE_ASM("asm/levels", func_L06_0042EAB8);

INCLUDE_ASM("asm/levels", func_L06_00444D00);

INCLUDE_ASM("asm/levels", func_L06_004638F0);

INCLUDE_ASM("asm/levels", func_L06_00463D40);

INCLUDE_ASM("asm/levels", func_L06_00463FC0);

extern s32 D_L06_001B9890;

void func_L06_00475C10(float a0) {
    s32 v1 = D_L06_001B9890;
    if (*(s16 *)(v1 + 0x86) == 0)
        *(float *)(*(s32 *)(v1 + 0x70) + 0x24C) = a0;
}

INCLUDE_ASM("asm/levels", func_L06_0047A268);

INCLUDE_ASM("asm/levels", func_L06_0047A408);

INCLUDE_ASM("asm/levels", func_L06_0047AC68);

INCLUDE_ASM("asm/levels", func_L06_0047C1D0);

INCLUDE_ASM("asm/levels", func_L06_00491A30);
