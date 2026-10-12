#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L05_003008C0);

INCLUDE_ASM("asm/levels", func_L05_00302CE8);

INCLUDE_ASM("asm/levels", func_L05_00303000);

INCLUDE_ASM("asm/levels", func_L05_00304B40);

INCLUDE_ASM("asm/levels", func_L05_0030C930);

INCLUDE_ASM("asm/levels", func_L05_00324DB8);

INCLUDE_ASM("asm/levels", func_L05_00334358);

INCLUDE_ASM("asm/levels", func_L05_00336630);

INCLUDE_ASM("asm/levels", func_L05_00339078);

INCLUDE_ASM("asm/levels", func_L05_00339EE8);

INCLUDE_ASM("asm/levels", func_L05_0033C2E0);

INCLUDE_ASM("asm/levels", func_L05_0033C5E8);

INCLUDE_ASM("asm/levels", func_L05_0033C650);

INCLUDE_ASM("asm/levels", func_L05_0033CD40);

INCLUDE_ASM("asm/levels", func_L05_0033D3A0);

INCLUDE_ASM("asm/levels", func_L05_0033DEE8);

INCLUDE_ASM("asm/levels", func_L05_0033E228);

INCLUDE_ASM("asm/levels", func_L05_0033E600);

INCLUDE_ASM("asm/levels", func_L05_0033F4A8);

INCLUDE_ASM("asm/levels", func_L05_00352B28);

INCLUDE_ASM("asm/levels", func_L05_00379968);

INCLUDE_ASM("asm/levels", func_L05_00379B60);

extern char *func_002ED688(int, int);

void func_L05_00379C68(int arg0, int value) {
    char *object = func_002ED688(arg0, value);
    if (object != 0)
        *(int *)(object + 0x150) = value;
}

extern int D_L05_00225DD0[];

int *func_L05_00379EB8(void) {
    return D_L05_00225DD0;
}

INCLUDE_ASM("asm/levels", func_L05_00379ED0);

extern int D_L05_00221D10[];

void func_L05_00379F18(int value) {
    D_L05_00221D10[13] = value;
}

void func_L05_00379F28(int value) {
    D_L05_00221D10[15] = value;
}

int func_L05_00379F38(void) {
    return D_L05_00221D10[15];
}

void func_L05_00379F48(int value) {
    D_L05_00221D10[18] = value;
}

int func_L05_00379F58(void) {
    return D_L05_00221D10[18];
}

void func_L05_00379F68(void) {
    char *base = (char *)D_L05_00221D10 + 0xC0;
    int *p = (int *)(base + 0x3FD0);
    int i = 0x7F;
    do {
        *p = 0;
        p -= 0x20;
        i--;
    } while (i >= 0);
}

INCLUDE_ASM("asm/levels", func_L05_00379FA0);

INCLUDE_ASM("asm/levels", func_L05_0037A218);

INCLUDE_ASM("asm/levels", func_L05_0037A5F8);

INCLUDE_ASM("asm/levels", func_L05_0037AA88);

INCLUDE_ASM("asm/levels", func_L05_0037AB18);

INCLUDE_ASM("asm/levels", func_L05_0037AB88);

INCLUDE_ASM("asm/levels", func_L05_0037AC00);

INCLUDE_ASM("asm/levels", func_L05_0037AC40);

INCLUDE_ASM("asm/levels", func_L05_0037AC80);

INCLUDE_ASM("asm/levels", func_L05_0037ADD8);

float func_L05_0037AE28(void) {
    return ((float *)D_L05_00221D10)[12];
}

INCLUDE_ASM("asm/levels", func_L05_0037AE38);

INCLUDE_ASM("asm/levels", func_L05_0037AE40);
