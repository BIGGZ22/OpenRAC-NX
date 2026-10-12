#include "include_asm.h"

extern int func_002886C8(int a, char *b);

extern void func_002883E8(int a);

/* Qwen draft (branch qwen/autonomy, d969a356), checked by tools/check.py */
void func_00278EB0(void);

/* loops 56 times calling a two-argument helper and, unless the current index is zero, a second one-argument helper with that index. */
void func_00290A40(char *a0)
{
    int i;

    for (i = 0; i < 56; i++) {
        if (func_002886C8(i, a0) == 0)
            continue;
        if (i != 0)
            func_002883E8(i);
    }
}

INCLUDE_ASM("asm/game", func_00290AA8);

INCLUDE_ASM("asm/game", func_00290AB0);

INCLUDE_ASM("asm/game", func_00290B48);

INCLUDE_ASM("asm/game", func_00290C60);

void func_00291108(void)
{
  func_00278EB0();
  return;
}

INCLUDE_ASM("asm/game", func_00291128);

INCLUDE_ASM("asm/game", func_00291148);

INCLUDE_ASM("asm/game", func_00291158);

INCLUDE_ASM("asm/game", func_00291268);

INCLUDE_ASM("asm/game", func_002912E8);

void func_002912F0(void)
{
}

INCLUDE_ASM("asm/game", func_002912F8);

INCLUDE_ASM("asm/game", func_00291308);

INCLUDE_ASM("asm/game", func_00291448);

INCLUDE_ASM("asm/game", func_00291450);

INCLUDE_ASM("asm/game", func_002914B8);

INCLUDE_ASM("asm/game", func_00291640);
