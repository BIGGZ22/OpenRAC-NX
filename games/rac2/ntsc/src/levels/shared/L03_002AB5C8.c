#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L03_002AB5C8);

INCLUDE_ASM("asm/levels", func_L03_002AD540);

INCLUDE_ASM("asm/levels", func_L03_002ADC00);

extern unsigned char D_L03_00189E20[];

void func_L03_002ADCA0(void) {
    unsigned char *g = D_L03_00189E20;

    if (*(short *)(g + 0x6BC) != 0 && *(int *)(g + 0x660) == 0)
        *(float *)(*(unsigned char **)(g + 0x2290) + 0xF4) = *(float *)(g + 0x6B0);
}

INCLUDE_ASM("asm/levels", func_L03_002B0958);

INCLUDE_ASM("asm/levels", func_L03_002B8D10);

INCLUDE_ASM("asm/levels", func_L03_002BC430);

#define MACRO_ADDR __attribute__((section(".sdata")))

extern int r1_D_L02_0015F04C MACRO_ADDR;

extern int r1_D_L02_00169670[];

/* rac1-decomp src/overlays/shared/camera_001FC308.c: func_L02_001FC308 */
/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/camera/001fc008.c, FUN_L02_001fc008. */
void func_L03_002D84F0(int x) {
    int i;

    if (r1_D_L02_0015F04C < 16) {
        for (i = 0; i < r1_D_L02_0015F04C; i++) {
            if (x == r1_D_L02_00169670[i]) {
                return;
            }
        }
        r1_D_L02_00169670[r1_D_L02_0015F04C++] = x;
    }
}

INCLUDE_ASM("asm/levels", func_L03_002EC508);

INCLUDE_ASM("asm/levels", func_L03_002ECF08);

INCLUDE_ASM("asm/levels", func_L03_002ED2F8);

INCLUDE_ASM("asm/levels", func_L03_002EFB60);

INCLUDE_ASM("asm/levels", func_L03_003245B0);

INCLUDE_ASM("asm/levels", func_L03_00326458);

INCLUDE_ASM("asm/levels", func_L03_003268C0);

INCLUDE_ASM("asm/levels", func_L03_00329EF0);

INCLUDE_ASM("asm/levels", func_L03_0032AB20);

INCLUDE_ASM("asm/levels", func_L03_0032AD10);

INCLUDE_ASM("asm/levels", func_L03_00339128);

extern int func_2EEC20(void *);
extern void func_331270(void *);

void func_L03_00339268(unsigned char *a0) {
    if (*(short *)(a0 + 0xA) == -1)
        return;
    if (func_2EEC20(a0 + 0xA) == 0)
        return;
    func_331270(a0);
}

INCLUDE_ASM("asm/levels", func_L03_003564C0);

INCLUDE_ASM("asm/levels", func_L03_0038A6D8);

INCLUDE_ASM("asm/levels", func_L03_0038A9D0);

INCLUDE_ASM("asm/levels", func_L03_0038AB30);

INCLUDE_ASM("asm/levels", func_L03_0038B238);

INCLUDE_ASM("asm/levels", func_L03_0038B2E8);

INCLUDE_ASM("asm/levels", func_L03_0038B390);

INCLUDE_ASM("asm/levels", func_L03_0038B408);

INCLUDE_ASM("asm/levels", func_L03_0038B4E8);

INCLUDE_ASM("asm/levels", func_L03_0038B5A8);

INCLUDE_ASM("asm/levels", func_L03_0038B5D8);

INCLUDE_ASM("asm/levels", func_L03_0038BCC0);

extern float func_2EF158(void *, void *);
extern void func_2B8300(int, int, void *);

void func_L03_00393228(unsigned char *a0) {
    unsigned char *g = D_L03_00189E20;

    if (*(unsigned char *)(g + 0x22B4) == 0)
        if (func_2EF158(g + 0x80, a0 + 0x10) < 1.7f)
            func_2B8300(1, 0x3F, a0);
}

INCLUDE_ASM("asm/levels", func_L03_003A40A0);

int func_L03_003A4978(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/levels", func_L03_003C1BF0);

INCLUDE_ASM("asm/levels", func_L03_003C3220);

extern int func_419B10(void *, void *, int, void *, void *, void *, float, float);
extern void func_32D0F0(void *, int, float);
float func_L03_003C5AB8(unsigned char *);

int func_L03_003C41B8(unsigned char *a0, int a1, int a2) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    float f = 0.0f;
    float out;
    int r;

    if (a2 != 0)
        f = func_L03_003C5AB8(a0);
    r = func_419B10(*(void **)(s0 + 0x220), a0 + 0x10, a1, &out, s0 + 0x368, s0 + 0x36C, f,
                    *(float *)(s0 + 0x110));
    func_32D0F0(a0, a1, out);
    return r;
}

INCLUDE_ASM("asm/levels", func_L03_003C4240);

INCLUDE_ASM("asm/levels", func_L03_003C4368);

INCLUDE_ASM("asm/levels", func_L03_003C4468);

INCLUDE_ASM("asm/levels", func_L03_003C4578);

INCLUDE_ASM("asm/levels", func_L03_003C4800);

INCLUDE_ASM("asm/levels", func_L03_003C4A10);

INCLUDE_ASM("asm/levels", func_L03_003C4BD8);

INCLUDE_ASM("asm/levels", func_L03_003C4D40);

INCLUDE_ASM("asm/levels", func_L03_003C4F58);

INCLUDE_ASM("asm/levels", func_L03_003C5128);

INCLUDE_ASM("asm/levels", func_L03_003C52D0);

INCLUDE_ASM("asm/levels", func_L03_003C5488);

INCLUDE_ASM("asm/levels", func_L03_003C5668);

INCLUDE_ASM("asm/levels", func_L03_003C58F0);

INCLUDE_ASM("asm/levels", func_L03_003C5AB8);
