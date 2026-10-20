#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern int func_337518(int a0, char *a1, int a2);

extern void func_42DF10(void *a0);

extern void *D_L10_001ADE90 __attribute__((sda));

extern int func_32AC10(int a0);


extern float func_309B38(float f12, float f13);

extern float func_309B80(float f12, float f13);


int func_L10_0042E590(int a0) {
    return func_337518(a0, (char *)0x230000, 0) != 0;
}

void func_L10_00425410(int a0, void *a1) {
    int i;

    if (a0 > 0) {
        i = a0;
        do {
            func_42DF10(a1);
            i--;
        } while (i != 0);
    }
}

void *func_L10_00449AD0(void) {
    return D_L10_001ADE90;
}

extern char *D_L10_001B2528;
extern void func_32AF78(void *a0);


INCLUDE_ASM("asm/levels", func_L10_00425798);

INCLUDE_ASM("asm/levels", func_L10_00424028);

INCLUDE_ASM("asm/levels", func_L10_00424290);

INCLUDE_ASM("asm/levels", func_L10_00424528);

INCLUDE_ASM("asm/levels", func_L10_00424CD0);

INCLUDE_ASM("asm/levels", func_L10_00425508);

INCLUDE_ASM("asm/levels", func_L10_00425888);

INCLUDE_ASM("asm/levels", func_L10_00424448);

INCLUDE_ASM("asm/levels", func_L10_0042B500);

INCLUDE_ASM("asm/levels", func_L10_0042BCA0);

/* func_L10_0042BE20 is defined in L10_00424028-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L10_0042BEE0);

INCLUDE_ASM("asm/levels", func_L10_0042DF10);

INCLUDE_ASM("asm/levels", func_L10_0042E100);

INCLUDE_ASM("asm/levels", func_L10_0042E4D0);

INCLUDE_ASM("asm/levels", func_L10_0042E660);
