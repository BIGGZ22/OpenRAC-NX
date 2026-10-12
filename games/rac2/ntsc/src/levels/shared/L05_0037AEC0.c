#include "include_asm.h"

extern char *func_002ED688(char *object, int flag);

extern void func_00282C88_2(char *target);

extern void func_L05_0037BB28(char *object);

extern void func_002EE678(char *object, int value);

extern void func_L05_0037CDD8(char *object, int value);

extern void func_002EE7B8(char *object);

extern void func_L05_0037AE40(char *object);

extern int D_L05_00221D10[];

/* Initialises a spawned sub-object from the owner's fields and increments a shared counter, then runs three follow-up calls and conditionally two more. */
void func_L05_0037AEC0(char *object, int flag)
{
    char *s;
    int **slot;

    s = func_002ED688(object, flag);
    if (s == 0)
        return;

    *(short *)(object + 50) = 511;
    *(unsigned char *)(object + 48) = 255;
    *(short *)(object + 52) = *(unsigned short *)(object + 52) & 0xfffc;
    *(int *)(object + 152) = *(int *)(*(int *)(object + 36) + 16);
    *(int *)(s + 272) = 1;
    *(int *)(s + 248) = 0;
    func_00282C88_2(s + 112);
    *(int *)(s + 268) = 0;
    *(int *)(s + 280) = 0;
    *(int *)(s + 252) = 0;
    *(int *)(s + 308) = 0;
    *(int *)(s + 332) = 1;
    *(int *)(s + 348) = 0;
    slot = *(int ***)(object + 104);
    *(float *)*slot = (float)*(int *)(s + 328);
    D_L05_00221D10[4] = D_L05_00221D10[4] + 1;
    func_L05_0037BB28(object);
    func_002EE678(object, 1);
    func_L05_0037CDD8(object, 1);
    if (flag)
        func_002EE7B8(object);
    if (D_L05_00221D10[13] == 1)
        func_L05_0037AE40(object);
}

INCLUDE_ASM("asm/levels", func_L05_0037AFD0);

INCLUDE_ASM("asm/levels", func_L05_0037AFF8);

INCLUDE_ASM("asm/levels", func_L05_0037B108);

INCLUDE_ASM("asm/levels", func_L05_0037B210);

INCLUDE_ASM("asm/levels", func_L05_0037B280);

INCLUDE_ASM("asm/levels", func_L05_0037B3A0);

INCLUDE_ASM("asm/levels", func_L05_0037B438);

INCLUDE_ASM("asm/levels", func_L05_0037B440);

INCLUDE_ASM("asm/levels", func_L05_0037B538);

INCLUDE_ASM("asm/levels", func_L05_0037B6C0);
