#include "include_asm.h"

typedef unsigned char u8;

extern void func_002EE358(void *, void *, void *);
extern void func_00324530(void *, void *, void *, void *);
extern float func_002EE9C8(float, float);

extern void func_002EE5F0(void *, void *, float);
extern void func_002EE440(void *, void *, void *);

INCLUDE_ASM("asm/levels", func_L26_002EC840);

INCLUDE_ASM("asm/levels", func_L26_002FC8D8);

INCLUDE_ASM("asm/levels", func_L26_002FCCC0);

/* Samples the record's plane at its own position and returns the resident height. */
float func_L26_00324598(u8 *a0, void *a1, void *a2) {
    float buf[4];

    func_002EE358(buf, a1, a0 + 0x10);
    func_00324530(a0, buf, buf, a2);
    return func_002EE9C8(buf[0], buf[1]);
}

/* Scales the three vertices of the record's triangle, ordering the middle pair
   by the caller's flag. */
void func_L26_00324DE8(u8 *a0, int a1) {
    func_002EE5F0(a0, a0, 1.0f);
    if (a1 != 0)
        func_002EE440(a0 + 0x10, a0 + 0x20, a0);
    else
        func_002EE440(a0 + 0x10, a0, a0 + 0x20);
    func_002EE5F0(a0 + 0x10, a0 + 0x10, 1.0f);
    func_002EE440(a0 + 0x20, a0 + 0x10, a0);
}

INCLUDE_ASM("asm/levels", func_L26_003547D8);

INCLUDE_ASM("asm/levels", func_L26_0038BB28);

INCLUDE_ASM("asm/levels", func_L26_0038BF40);

INCLUDE_ASM("asm/levels", func_L26_00391520);

INCLUDE_ASM("asm/levels", func_L26_003BFF58);

INCLUDE_ASM("asm/levels", func_L26_003D4590);

INCLUDE_ASM("asm/levels", func_L26_003D4930);

INCLUDE_ASM("asm/levels", func_L26_003D4AE8);

INCLUDE_ASM("asm/levels", func_L26_003DB818);

INCLUDE_ASM("asm/levels", func_L26_003DBE18);

INCLUDE_ASM("asm/levels", func_L26_003DC048);

/* func_L26_003DC128 lives in the -g8 sibling file (it reaches a global through $gp). */

INCLUDE_ASM("asm/levels", func_L26_003DF680);

INCLUDE_ASM("asm/levels", func_L26_003DF7E8);

INCLUDE_ASM("asm/levels", func_L26_004070F8);

INCLUDE_ASM("asm/levels", func_L26_0040C8A8);

INCLUDE_ASM("asm/levels", func_L26_0041C598);
