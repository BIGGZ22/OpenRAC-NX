#include "include_asm.h"

extern int ref_L00_002FBF60_00;

extern int ref_L00_002FC008_00;

extern int ref_L00_002FC018_00;

extern int ref_L00_002FC020_00;

extern int ref_L00_002FC028_00;

void func_L00_002FBF60(void)
{
    ref_L00_002FBF60_00 = 1;
}

void func_L00_002FC008(void)
{
    ref_L00_002FC008_00 = 1;
}

int func_L00_002FC018(void)
{
    return ref_L00_002FC018_00;
}

void func_L00_002FC020(void)
{
    ref_L00_002FC020_00 = 0;
}

void func_L00_002FC028(void)
{
    ref_L00_002FC028_00 = 0;
}

INCLUDE_ASM("asm/levels", func_L00_002FC058);

INCLUDE_ASM("asm/levels", func_L00_002FC460);

INCLUDE_ASM("asm/levels", func_L00_002FC580);

extern int r1_func_0011CC38_002FC588();

/* rac1-decomp src/core/00119D88.c: func_0011CCB0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_L00_002FC588(void) {
    return r1_func_0011CC38_002FC588();
}

extern int r1_func_0011CC38_002FC5A8();

/* rac1-decomp src/core/00119D88.c: func_0011CCB0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_L00_002FC5A8(void) {
    return r1_func_0011CC38_002FC5A8();
}

extern int r1_func_0011CC38_002FC5C8();

/* rac1-decomp src/core/00119D88.c: func_0011CCB0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_L00_002FC5C8(void) {
    return r1_func_0011CC38_002FC5C8();
}

extern int r1_func_0011CC38_002FC5E8();

/* rac1-decomp src/core/00119D88.c: func_0011CCB0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_L00_002FC5E8(void) {
    return r1_func_0011CC38_002FC5E8();
}

INCLUDE_ASM("asm/levels", func_L00_002FC798);
