/* 0 of 3 places match - the whole body matches except one register.
   File: src/levels/shared/L18_00427BE0-g8.c (-O2 -G8 -ffunction-sections).
   All 45 instructions are present and in retail's order, including the `beqz`/`nop`
   guard, the three `$gp` loads, both `func_00308590(s0 + 0x392/0x396)` calls, the
   `sltu $v0,0xFFFE,$v1` tail guard and the 5th float argument. The residue is the
   first counter only - 3 words at +0xA0/+0xA8/+0xB0:
     mine   lhu $v0,0x394($s0) ; addiu $v0,$v0,1 ; sh $v0,0x394($s0)
     retail lhu $v1,0x394($s0) ; addiu $v1,$v1,1 ; sh $v1,0x394($s0)
   Retail gives that temp $v1 and then reuses $v1 for the second counter; this cc1
   gives it $v0. Six spellings measured (`+= 1`, an explicit local for the counter,
   the load hoisted before the call, the store after it, the second counter also
   named): the same 3 words every time. Register-allocation wall, same family as
   the register-pair swaps. The `$gp` members work with `__attribute__((sda))`. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef float f32;

extern int D_L18_gp2264 __attribute__((sda));
extern int D_L18_gp2268 __attribute__((sda));
extern int D_L18_gp226C __attribute__((sda));
extern f32 D_L18_gp230C __attribute__((sda));

extern short func_00335778(int, int);
extern void func_00308590(void *);
extern void func_00308560(u32);
extern void func_00340BD8(char *, void *, int, int, f32);

void func_L18_00428048(char *self) {
    char *s0 = *(char **)(self + 0x68);

    if (*(u8 *)(s0 + 0x39A) != 0) {
        *(u8 *)(s0 + 0x39A) = 0;
        if (*(s16 *)(s0 + 0x390) < D_L18_gp2264) {
            *(s16 *)(s0 + 0x390) = func_00335778(D_L18_gp2268, D_L18_gp226C);
        }
    }
    func_00308590(s0 + 0x392);
    func_00308590(s0 + 0x396);
    *(u16 *)(s0 + 0x394) += 1;
    func_00308560(*(u32 *)(s0 + 0x3A8));
    if (*(u16 *)(self + 0x96) <= 0xFFFE) {
        *(u16 *)(self + 0x96) += 1;
    }
    func_00340BD8(self, s0 + 0x2C0, 1, 1, D_L18_gp230C);
}
