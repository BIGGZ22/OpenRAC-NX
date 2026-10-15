#include "include_asm.h"

typedef int s32;
typedef unsigned char u8;

extern float func_00283B98(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

extern void func_L04_003A21F8(char *, s32);

extern char D_L18_002A9C40[];

extern float D_L18_001B97C0[];

extern float func_002A7878(float, float);

extern void func_0029FCC0(void *, int, void *);

extern void func_00282CF0(float *, const float *, const float *);

extern void func_00282F20(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

extern int func_002A7820(int, int);

extern float func_002A7940(void);

extern int D_L18_001C9D40;

extern void func_L02_003FFC48(void *, int, int, int, int, float, float);

extern s32 func_002E59B0(void *, s32);

extern s32 func_002E5CF8(s32, s32, void *);

extern void *func_002A9640(void *, s32, s32);

extern float func_002A96A0(void *, void *);

extern void func_002AB800(void *, void *);

extern void func_002AB8C0(void *, void *);

extern int func_002A7750(void *, int, int);

typedef int TI128 __attribute__((mode(TI)));

void func_L18_00435CC8(void) {
    func_L04_003A21F8(D_L18_002A9C40, 0x12);
}

void func_L18_00434128(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 i;
    for (i = 0; i < 2; i++) {
        u8 *p = &state[i * 0xA0 + 0x610];
        s32 j;
        for (j = 1; j >= 0; j--) {
            float x;
            *(float *)(p + 0x40) = func_00283B98(*(float *)(p + 0x40), *(float *)(p + 0x60));
            *(float *)(p + 0x44) = func_00283B98(*(float *)(p + 0x44), *(float *)(p + 0x64));
            *(float *)(p + 0x48) = func_00283B98(*(float *)(p + 0x48), *(float *)(p + 0x68));
            x = func_00283180(*(float *)(p + 0x40));
            *(float *)(p + 0x20) = x * *(float *)(p + 0x80);
            *(float *)(p + 0x24) = func_00283198(*(float *)(p + 0x44)) * *(float *)(p + 0x84);
            *(float *)(p + 0x28) = -func_00283198(*(float *)(p + 0x48)) * *(float *)(p + 0x88);
            p += 0x10;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L18_00433460);

INCLUDE_ASM("asm/levels", func_L18_00433B70);

INCLUDE_ASM("asm/levels", func_L18_00434510);

INCLUDE_ASM("asm/levels", func_L18_00434A98);

INCLUDE_ASM("asm/levels", func_L18_00434FC0);

INCLUDE_ASM("asm/levels", func_L18_004350E0);

INCLUDE_ASM("asm/levels", func_L18_004354F8);

INCLUDE_ASM("asm/levels", func_L18_004359A8);

INCLUDE_ASM("asm/levels", func_L18_00435B10);

INCLUDE_ASM("asm/levels", func_L18_00436B80);

INCLUDE_ASM("asm/levels", func_L18_00437190);

INCLUDE_ASM("asm/levels", func_L18_00437BA8);

INCLUDE_ASM("asm/levels", func_L18_00439960);

extern int D_L18_001C9D40;

extern float func_00283B98(float, float);

extern s32 func_002E59B0(void *, s32);

extern s32 func_002E5CF8(s32, s32, void *);

extern void *func_002A9640(void *, s32, s32);

extern float func_002A96A0(void *, void *);

extern void func_002AB800(void *, void *);

extern void func_002AB8C0(void *, void *);

extern int func_002A7750(void *, int, int);

INCLUDE_ASM("asm/levels", func_L18_00439FC8);

INCLUDE_ASM("asm/levels", func_L18_0043C258);

INCLUDE_ASM("asm/levels", func_L18_0043C778);

INCLUDE_ASM("asm/levels", func_L18_0043CAF8);

INCLUDE_ASM("asm/levels", func_L18_0043F130);

INCLUDE_ASM("asm/levels", func_L18_0043F308);

INCLUDE_ASM("asm/levels", func_L18_0043FB08);

INCLUDE_ASM("asm/levels", func_L18_0043FC70);

INCLUDE_ASM("asm/levels", func_L18_00440718);

INCLUDE_ASM("asm/levels", func_L18_00441050);

INCLUDE_ASM("asm/levels", func_L18_004412F0);

INCLUDE_ASM("asm/levels", func_L18_00441438);

INCLUDE_ASM("asm/levels", func_L18_00447128);

INCLUDE_ASM("asm/levels", func_L18_0046B660);

INCLUDE_ASM("asm/levels", func_L18_0046BAD8);
