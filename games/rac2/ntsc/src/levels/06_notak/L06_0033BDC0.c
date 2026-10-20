#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L06_0033BDC0);

INCLUDE_ASM("asm/levels", func_L06_0035B768);

INCLUDE_ASM("asm/levels", func_L06_00361728);

/* func_L06_003660F0 is matched in L06_003660F0.c (built with -mno-split-addresses). */

INCLUDE_ASM("asm/levels", func_L06_0037E6B0);

INCLUDE_ASM("asm/levels", func_L06_00396268);

INCLUDE_ASM("asm/levels", func_L06_003C5ED8);

INCLUDE_ASM("asm/levels", func_L06_003D0AC8);

INCLUDE_ASM("asm/levels", func_L06_003D0EE0);

INCLUDE_ASM("asm/levels", func_L06_003D64C0);

INCLUDE_ASM("asm/levels", func_L06_003F3448);

extern int func_002A9640(char *a0, int a1, int a2);

extern float func_002A96A0(char *a0, int a1);

extern void func_002AB800(char *a0, char *a1);

extern void func_002A6C58(char *a0, int a1, int a2);

extern void func_002AB8C0(char *a0, char *a1);

/* runs a moby's timer down and picks the dying or the revived path from what is left */
void func_L06_003F3690(char *a0, char *a1)
{
    float t = func_002A96A0(a0, func_002A9640(a0, 0xC30000, 0));

    if (t > 0.0f) {
        float left = *(float *)(a1 + 0x20) - t;

        *(float *)(a1 + 0x20) = left;
        if (left > 0.0f) {
            *(unsigned char *)(a1 + 0x57) = 0x5A;
            func_002AB800(a0, a1 + 0x50);
        } else {
            func_002A6C58(a0, 0, 3);
            *(unsigned char *)(a0 + 0x20) = 2;
            *(unsigned char *)(a1 + 0x57) = 0x78;
            func_002AB800(a0, a1 + 0x50);
        }
    }
    *(unsigned char *)(a0 + 0xA8) = 0xFF;
    func_002AB8C0(a0, a1 + 0x50);
}

extern int func_002A8508(int a0, int a1);

/* counts the 0x75E-slot entries that are live and returns the last free one */
int func_L06_003F3760(int a0, char *a1)
{
    char *s0 = a1;
    int free_slot = -1;
    int i;

    *(unsigned char *)(s0 + 0x81) = func_002A8508(*(int *)(s0 + 0x84), -1);
    for (i = 0; i < 20; i++) {
        char *m = ((char **)(s0 + 0xA0))[i];

        if (m != 0 && *(short *)(m + 0xAA) == 0x75E
            && *(unsigned char *)(m + 0x20) != 0xFE && *(unsigned char *)(m + 0x20) != 0xFD)
            (*(unsigned char *)(s0 + 0x81))++;
        else
            free_slot = i;
    }
    return free_slot;
}

INCLUDE_ASM("asm/levels", func_L06_003F3800);

INCLUDE_ASM("asm/levels", func_L06_003F3EF0);

extern void func_0029F338(char *a0);

/* clears one word of a 0x75D moby's state block when it is neither dying nor gone */
void func_L06_003F4020(char *a0, char *a1)
{
    char *m = *(char **)(a1 + 0x10);

    if (m != 0 && *(short *)(m + 0xAA) == 0x75D
        && *(unsigned char *)(m + 0x20) != 0xFE && *(unsigned char *)(m + 0x20) != 0xFD) {
        int *p = *(int **)(m + 0x68);

        p += *(unsigned char *)(a1 + 0x1C);
        p[40] = 0;
    }
    func_0029F338(a0);
}

INCLUDE_ASM("asm/levels", func_L06_003F4080);

extern int D_L06_001B3760[];

/* zeroes the six words a finished moby left behind */
void func_L06_003F4908(char *a0)
{
    if (*(unsigned char *)(a0 + 0x20) == 0) {
        int i;

        for (i = 5; i >= 0; i--)
            D_L06_001B3760[i] = 0;
        func_0029F338(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L06_003FB050);

INCLUDE_ASM("asm/levels", func_L06_003FDA08);

INCLUDE_ASM("asm/levels", func_L06_003FDEB8);

INCLUDE_ASM("asm/levels", func_L06_003FE178);

INCLUDE_ASM("asm/levels", func_L06_00400498);

INCLUDE_ASM("asm/levels", func_L06_004012B0);

INCLUDE_ASM("asm/levels", func_L06_00401820);

INCLUDE_ASM("asm/levels", func_L06_00402230);

INCLUDE_ASM("asm/levels", func_L06_00403F60);

INCLUDE_ASM("asm/levels", func_L06_0040C270);

INCLUDE_ASM("asm/levels", func_L06_00419CA8);

/* clears one word of a 0xA58 moby's state block when it is neither dying nor gone */
void func_L06_0041CAB0(char *a0, char *a1)
{
    char *m = *(char **)(a1 + 0x10);

    if (m != 0 && *(short *)(m + 0xAA) == 0xA58
        && *(unsigned char *)(m + 0x20) != 0xFE && *(unsigned char *)(m + 0x20) != 0xFD) {
        int *p = *(int **)(m + 0x68);

        p += *(unsigned char *)(a1 + 0x1C);
        p[40] = 0;
    }
    func_0029F338(a0);
}

INCLUDE_ASM("asm/levels", func_L06_0041CB10);
