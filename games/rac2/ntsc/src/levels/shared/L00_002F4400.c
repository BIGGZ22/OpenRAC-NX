#include "include_asm.h"

void func_L00_002F4400(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002F4510);

INCLUDE_ASM("asm/levels", func_L00_002F45B8);

INCLUDE_ASM("asm/levels", func_L00_002F46C8);

extern short D_L00_0027D880[];

void func_L00_002F4720(void) {
    D_L00_0027D880[0] = 0;
}

void func_L00_002F51B0(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002F5890);

INCLUDE_ASM("asm/levels", func_L00_002F59A8);

INCLUDE_ASM("asm/levels", func_L00_002F62D8);

INCLUDE_ASM("asm/levels", func_L00_002F6B90);

INCLUDE_ASM("asm/levels", func_L00_002F6ED0);

INCLUDE_ASM("asm/levels", func_L00_002F7298);

INCLUDE_ASM("asm/levels", func_L00_002F74F0);

extern int r1_func_0011CC38();

/* rac1-decomp src/core/00119D88.c: func_0011CCB0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_L00_002F7568(void) {
    return r1_func_0011CC38();
}

int func_L00_002F7588(void) {
    return 0;
}

int func_L00_002F9858(void) {
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002FACF8);

INCLUDE_ASM("asm/levels", func_L00_002FB8F8);

INCLUDE_ASM("asm/levels", func_L00_002FBF58);
