#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L05_003008C0);

INCLUDE_ASM("asm/levels", func_L05_00302CE8);

extern int func_002FEF40(int);

extern int func_L05_00302CE8(int, int, int, int, int, int, int, int, char *);

extern char D_L05_002A2DB0[];

/* Forward the seven arguments with a fresh step and the shared table. */
void func_L05_00303000(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    int step = func_002FEF40(2);

    func_L05_00302CE8(a0, a1, a2, a3, a4, a5, a6, step, D_L05_002A2DB0);
}

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

/* func_L05_00379ED0 is defined in L05_003008C0-g8.c (this unit has no $gp access for it). */

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

/* Add an argument to the stored value and clamp it into [8, 15]. */
void func_L05_0037ADD8(float arg)
{
    float *p = (float *)D_L05_00221D10;
    float v = p[12] + arg;
    float lim = 15.0f;

    p[12] = v;
    if (v > lim) {
        p[12] = lim;
    } else {
        lim = 8.0f;
        if (v < lim)
            p[12] = lim;
    }
}

float func_L05_0037AE28(void) {
    return ((float *)D_L05_00221D10)[12];
}

INCLUDE_ASM("asm/levels", func_L05_0037AE38);

extern char *func_L05_00379CA0(char *);

extern void func_L05_0037B838(char *);

/* Hand the object over, then clear its action state and step the shared counter. */
void func_L05_0037AE40(char *arg)
{
    char *o = func_L05_00379CA0(arg);

    if (o == 0)
        return;
    func_L05_0037B838(arg);
    *(unsigned short *)(arg + 0x32) = 0;
    *(unsigned short *)(arg + 0x34) = *(unsigned short *)(arg + 0x34) | 3;
    *(unsigned char *)(arg + 0x30) = 0;
    *(int *)(arg + 0x98) = 0;
    *(int *)(o + 0x110) = 0;
    if (D_L05_00221D10[4] != 0)
        D_L05_00221D10[4] = D_L05_00221D10[4] - 1;
}
