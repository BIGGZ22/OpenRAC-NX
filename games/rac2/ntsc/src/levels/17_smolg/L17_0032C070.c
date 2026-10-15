#include "common.h"

extern char D_00189E20[];

extern char D_L17_001FE880[];

extern char D_L17_00296F80[];

extern char D_L17_00296FF0[];

extern void func_2EF398(void *, void *, void *);

extern void func_2EEEB8(void *, void *, void *);

extern int func_2EEB28(char *);

extern int func_2F00C0(float);

extern float func_2EF4E0(float);

extern float func_2EEDF8(float);

extern float func_2F00B0(int);

extern void func_332E48(char *);

extern void func_002A7398();

extern void func_L17_003C71B0();

INCLUDE_ASM("asm/levels", func_L17_0032C070);

INCLUDE_ASM("asm/levels", func_L17_0032E898);

INCLUDE_ASM("asm/levels", func_L17_00340DD0);

/* divides two sub-object measurements taken through the float accessor, scales the ratio by a fixed
   2*pi-scale constant, folds the result through two helpers and packs a colour byte into the object's
   high word */
void func_L17_00340EA0(char *s1)
{
    float f20;
    float t;
    char *s0;

    if (s1 == 0)
        return;
    s0 = s1 + 0x20;
    if (func_2EEB28(s1 + 0x24) != 0) {
        func_332E48(s1);
        return;
    }
    f20 = func_2F00B0(*(short *)(s0 + 4)) / func_2F00B0(*(short *)(s0 + 6));
    t = func_2EF4E0(f20 * 6.283186f);
    *(int *)(s1 + 4) = (*(int *)(s1 + 4) & 0x00FFFFFF)
                       + (func_2F00C0(func_2EEDF8(func_2F00B0(*(int *)(s0 + 0xC)) * t)) << 24);
}

INCLUDE_ASM("asm/levels", func_L17_003583F8);

INCLUDE_ASM("asm/levels", func_L17_00385DE8);

INCLUDE_ASM("asm/levels", func_L17_00390CE0);

INCLUDE_ASM("asm/levels", func_L17_003910F8);

INCLUDE_ASM("asm/levels", func_L17_003966D8);

INCLUDE_ASM("asm/levels", func_L17_003A14A0);

INCLUDE_ASM("asm/levels", func_L17_003A4740);

INCLUDE_ASM("asm/levels", func_L17_003BCBC8);

INCLUDE_ASM("asm/levels", func_L17_003BCC30);

INCLUDE_ASM("asm/levels", func_L17_003BCEC0);

INCLUDE_ASM("asm/levels", func_L17_003BD310);

/* runs seven two-stage transforms over a record reached from an id in the sub-object; the id -1
   aborts, and the last id is echoed into the shared resident word */
void func_L17_003C3250(char *a2)
{
    char *s5;
    char *s0;
    char *s2;
    char *s4;
    char *s3;
    char *base;
    int v1;
    int i;

    s5 = *(char **)(a2 + 0x68);
    v1 = *(int *)(s5 + 0xD0);
    if (v1 == -1)
        return;
    v1 = *(int *)(D_L17_001FE880 + v1 * 4);
    if (*(int *)v1 != 7)
        return;
    s3 = a2 + 0xC0;
    s4 = a2 + 0x10;
    s2 = D_L17_00296F80;
    s0 = v1 + 0x10;
    for (i = 6; i >= 0; i--) {
        func_2EF398(s0, s2, s3);
        s2 += 0x10;
        func_2EEEB8(s0, s0, s4);
        s0 += 0x10;
    }
    base = D_00189E20;
    *(unsigned short *)(base + 0x24FE) = *(unsigned short *)(s5 + 0xD0);
}

INCLUDE_ASM("asm/levels", func_L17_003C3330);

INCLUDE_ASM("asm/levels", func_L17_003C3A00);

INCLUDE_ASM("asm/levels", func_L17_003C3AF0);

INCLUDE_ASM("asm/levels", func_L17_003C3C50);

INCLUDE_ASM("asm/levels", func_L17_003C4090);

INCLUDE_ASM("asm/levels", func_L17_003C45A0);

INCLUDE_ASM("asm/levels", func_L17_003C5200);

/* raises the state word to 0x920 and runs a three-argument setup, or, when it is already 0x929,
   re-runs the 0x71B0 activation if the object is still unmarked */
void func_L17_003C5AF8(int a0)
{
    if (*(short *)(a0 + 0xAA) == 0x920) {
        if (*(unsigned char *)(a0 + 0x20) == 0) {
            *(unsigned char *)(a0 + 0x20) = 1;
            func_002A7398(a0, 1, 0);
            return;
        }
    }
    if (*(short *)(a0 + 0xAA) == 0x929) {
        if (*(unsigned char *)(a0 + 0x20) == 0)
            func_L17_003C71B0(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L17_003C5B68);

INCLUDE_ASM("asm/levels", func_L17_003C5C28);

/* true for the sub-state 0x922 when the object's stage byte sits in the 2..6 window */
int func_L17_003C5DF0(int a0)
{
    if (*(short *)(a0 + 0xAA) != 0x922)
        return 0;
    return *(unsigned char *)(a0 + 0x20) - 2U < 5;
}

INCLUDE_ASM("asm/levels", func_L17_003C5E20);

/* twin of func_L17_003C3250 on the 0x128 sub-object id and its own table */
void func_L17_003C5E78(char *a2)
{
    char *s5;
    char *s0;
    char *s2;
    char *s4;
    char *s3;
    char *base;
    int v1;
    int i;

    s5 = *(char **)(a2 + 0x68);
    v1 = *(int *)(s5 + 0x128);
    if (v1 == -1)
        return;
    v1 = *(int *)(D_L17_001FE880 + v1 * 4);
    if (*(int *)v1 != 7)
        return;
    s3 = a2 + 0xC0;
    s4 = a2 + 0x10;
    s2 = D_L17_00296FF0;
    s0 = v1 + 0x10;
    for (i = 6; i >= 0; i--) {
        func_2EF398(s0, s2, s3);
        s2 += 0x10;
        func_2EEEB8(s0, s0, s4);
        s0 += 0x10;
    }
    base = D_00189E20;
    *(unsigned short *)(base + 0x24FE) = *(unsigned short *)(s5 + 0x128);
}

INCLUDE_ASM("asm/levels", func_L17_003C5F58);

INCLUDE_ASM("asm/levels", func_L17_003C6130);

INCLUDE_ASM("asm/levels", func_L17_003C69A8);

INCLUDE_ASM("asm/levels", func_L17_003C6CE8);

INCLUDE_ASM("asm/levels", func_L17_003C7018);

INCLUDE_ASM("asm/levels", func_L17_003C70E0);

INCLUDE_ASM("asm/levels", func_L17_003C71B0);

/* Stores the second argument in the object's sub-record at 0xA4. */
void func_L17_003C7250(char *a0, int a1)
{
    *(int *)(*(char **)(a0 + 0x68) + 0xA4) = a1;
}

INCLUDE_ASM("asm/levels", func_L17_003C7260);

INCLUDE_ASM("asm/levels", func_L17_003C7498);

INCLUDE_ASM("asm/levels", func_L17_003C7680);

/* true when the sub-object word at 0xBC holds the value two */
int func_L17_003C7C58(int a0)
{
    return *(int *)(*(int *)(a0 + 0x68) + 0xBC) == 2;
}
