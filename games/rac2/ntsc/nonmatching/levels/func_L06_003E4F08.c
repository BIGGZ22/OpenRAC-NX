/*
 func_L06_003E4F08 (152 bytes, -g8) — 4 bytes left: the constant is kept
   in a callee-saved FPR ($f20 + save) whereas the retail
   re-materialises it on every call.

   Body conforming word for word (allocations $s1/$s0, call order, the 5
   floats reached through $gp with __attribute__((sda)), the literal
   0.000290888245f as lui+ori+mtc1) EXCEPT this:
     retail : lui/ori/mtc1 $f13 before call 1, then lui/ori/mtc1 $f14 before
              call 2 (the constant is used twice in call 2); no
              FPR save; frame -0x20; 38 words.
     ours   : a single materialisation in $f20 kept across call 1
              (the CSE hoists the constant_double), + swc1/lwc1 $f20, frame -0x30;
              37 words.
   Tried with no effect: literal on the left of the multiplication, argument
   order, sda attribute. Same family as the hoisting wall observed on
   func_L06_003E4FA0.

   last check.py line:
     func_L06_003E4F08: 0 of 5 places match (src/levels/shared/L06_0035EA68-g8.c)
     06_notak: 148 bytes, expected 152

#include "include_asm.h"
*/

typedef unsigned char u8;
typedef unsigned int u32;

extern float D_L06_001AF510 __attribute__((sda));
extern float D_L06_001AF514 __attribute__((sda));
extern float D_L06_001AF518 __attribute__((sda));
extern float D_L06_001AF51C __attribute__((sda));
extern float D_L06_001AF520 __attribute__((sda));

extern void func_002AEE70(u32 a0, void *a1, void *a2, float f0, float f1, float f2);
extern void func_002AEF28(u32 a0, void *a1, void *a2, float f0, float f1, float f2);

void func_L06_003E4F08(char *a0)
{
    if (*(u8 *)(a0 + 0x20) != 5 && *(u8 *)(a0 + 0x20) != 7) {
        char *s0 = *(char **)(a0 + 0x68);
        func_002AEE70((u32)a0, s0 + 0xEC, s0 + 0xE8, D_L06_001AF510,
                      D_L06_001AF514 * 0.000290888245f, *(float *)(s0 + 0x108));
        func_002AEF28((u32)a0, s0 + 0xF0, s0 + 0xF4, D_L06_001AF518,
                      D_L06_001AF51C * 0.000290888245f, D_L06_001AF520 * 0.000290888245f);
    }
}
