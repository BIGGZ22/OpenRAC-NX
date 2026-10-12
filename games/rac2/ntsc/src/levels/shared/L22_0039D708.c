#include "include_asm.h"

extern void ref_L22_003A4FF8_00(void);
extern int func_L22_003A2DF8(int, float);

typedef struct Rec_003A2D58 {
    int count;
    unsigned char pad04[0x18];
    float angle;
} Rec_003A2D58;

extern Rec_003A2D58 *D_L22_001FE940[];

INCLUDE_ASM("asm/levels", func_L22_0039D708);

INCLUDE_ASM("asm/levels", func_L22_003A15D0);

INCLUDE_ASM("asm/levels", func_L22_003A1EC8);

INCLUDE_ASM("asm/levels", func_L22_003A1ED8);

INCLUDE_ASM("asm/levels", func_L22_003A2160);

INCLUDE_ASM("asm/levels", func_L22_003A2780);

INCLUDE_ASM("asm/levels", func_L22_003A2988);

INCLUDE_ASM("asm/levels", func_L22_003A2A28);

INCLUDE_ASM("asm/levels", func_L22_003A2B90);

INCLUDE_ASM("asm/levels", func_L22_003A2CB8);

int func_L22_003A2D58(char *a0, int a1, int a2) {
    char *base = *(char **)(a0 + 0x68) + 0x2A0;
    int *slot = (int *)(base + a1 * 4);
    Rec_003A2D58 *rec;
    int n;
    rec = D_L22_001FE940[*slot];
    if (func_L22_003A2DF8(a2, rec->angle) != 0) return 0;
    rec = D_L22_001FE940[*slot];
    n = rec->count - 1;
    rec = (Rec_003A2D58 *)((char *)rec + n * 16);
    func_L22_003A2DF8(a2, rec->angle);
    return n;
}

INCLUDE_ASM("asm/levels", func_L22_003A2DF8);

void func_L22_003A2E10(char *a0, unsigned short a1) {
    char *p = *(char **)(a0 + 0x68);
    unsigned short old = *(unsigned short *)(p + 0x354);
    int i = (short)*(short *)(p + 0x35E);
    char *w;
    i = (i + 1) % 3;
    *(short *)(p + 0x35E) = (short)i;
    w = p + i * 2;
    *(unsigned short *)(w + 0x358) = old;
    *(unsigned short *)(p + 0x354) = a1;
}

INCLUDE_ASM("asm/levels", func_L22_003A2E50);

INCLUDE_ASM("asm/levels", func_L22_003A3368);

INCLUDE_ASM("asm/levels", func_L22_003A3550);

INCLUDE_ASM("asm/levels", func_L22_003A36A0);

INCLUDE_ASM("asm/levels", func_L22_003A39C8);

INCLUDE_ASM("asm/levels", func_L22_003A3B50);

INCLUDE_ASM("asm/levels", func_L22_003A3CA0);

INCLUDE_ASM("asm/levels", func_L22_003A3D50);

INCLUDE_ASM("asm/levels", func_L22_003A4128);

INCLUDE_ASM("asm/levels", func_L22_003A42E8);

INCLUDE_ASM("asm/levels", func_L22_003A4490);

INCLUDE_ASM("asm/levels", func_L22_003A45F0);

INCLUDE_ASM("asm/levels", func_L22_003A47E8);

INCLUDE_ASM("asm/levels", func_L22_003A48F8);

INCLUDE_ASM("asm/levels", func_L22_003A4938);

INCLUDE_ASM("asm/levels", func_L22_003A4C68);

INCLUDE_ASM("asm/levels", func_L22_003A4CA8);

INCLUDE_ASM("asm/levels", func_L22_003A4EE8);

int func_L22_003A4FF8(void) {
    ref_L22_003A4FF8_00();
    return 1;
}
