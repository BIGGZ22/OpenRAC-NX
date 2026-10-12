#include "include_asm.h"

extern int func_00115DA8_1(char *, char *, ...);

extern char D_L00_001AD6B0[];

extern char D_L00_001AD6C0[];

extern char D_L00_001AD6C8[];

extern void Fbbb7bee1_FUN_00282A38(int value);

extern int D_L06_001B2F94 __attribute__((sda));

extern int D_L06_001B2F90 __attribute__((sda));

extern int D_L06_001A8F10 __attribute__((sda));

extern int D_L06_001B2F80[];

extern int D_L06_001B2F88_5 __attribute__((sda));

extern int D_L06_001B2370 __attribute__((sda));

extern int D_L06_001B2374 __attribute__((sda));

/* shared native body of family bc82131e */
void func_002F9BC0(char *dst, int value)
{
    if (value > 999999)
        func_00115DA8_1(dst, D_L00_001AD6B0, value / 1000000, (value / 1000) % 1000, value % 1000);
    else if (value >= 1000)
        func_00115DA8_1(dst, D_L00_001AD6C0, value / 1000, value % 1000);
    else
        func_00115DA8_1(dst, D_L00_001AD6C8, value);
}

INCLUDE_ASM("asm/game", func_002F9C80);

INCLUDE_ASM("asm/game", func_002FA270);

INCLUDE_ASM("asm/game", func_002FA880);

INCLUDE_ASM("asm/game", func_002FAD30);

INCLUDE_ASM("asm/game", func_002FAD68);

INCLUDE_ASM("asm/game", func_002FADB0);

INCLUDE_ASM("asm/game", func_002FAE68);

INCLUDE_ASM("asm/game", func_002FB170);

INCLUDE_ASM("asm/game", func_002FB1A0);

INCLUDE_ASM("asm/game", func_002FBFB8);

INCLUDE_ASM("asm/game", func_002FC008);

INCLUDE_ASM("asm/game", func_002FC010);

void func_002FC070(int mask)
{
    while (*(volatile int *)0x10008000 & 0x100)
        Fbbb7bee1_FUN_00282A38(16);
    *(volatile int *)0x10008020 = 0;
    *(volatile int *)0x10008030 = mask & 0x0FFFFFFF;
    *(volatile int *)0x10008000 = 325;
    while (*(volatile int *)0x10008000 & 0x100)
        Fbbb7bee1_FUN_00282A38(16);
}

INCLUDE_ASM("asm/game", func_002FC138);

INCLUDE_ASM("asm/game", func_002FC190);

INCLUDE_ASM("asm/game", func_002FC1D8);

/* shared native body of family 74e6a904f3b4f71c (default profile) */
void func_002FC220(void)
{
    int a0;
    int a1;
    int a2;
    int v1;

    a0 = 1 - D_L06_001B2F94;
    a1 = D_L06_001B2F90;
    a2 = D_L06_001A8F10;
    D_L06_001B2F94 = a0;
    v1 = D_L06_001B2F80[a0];
    a1 = v1 + D_L06_001B2F90;
    D_L06_001B2F88_5 = v1;
    a1 = a1 - a2;
    D_L06_001B2370 = a1;
    D_L06_001B2374 = a1 - 8192;
}

INCLUDE_ASM("asm/game", func_002FC280);
