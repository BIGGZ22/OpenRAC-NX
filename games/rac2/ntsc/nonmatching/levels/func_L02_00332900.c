/* 0 of 9 places match - candidate is 120 bytes, retail 112. File:
   src/levels/shared/L02_002B5F50-g8.c (-O2 -G8 -ffunction-sections).
   Retail compares a $gp word against the word at the absolute 0x001B30C8 and then
   REBUILDS that absolute address at each of the two sites (lui $v0,0x1B ; lw
   $v0,0x30C8($v0)) instead of keeping it in a callee-saved register; the candidate
   still CSEs the address (one extra $s0 save/restore and a shifted frame).
   `D_001B30C8` + `D_001B30C8_1` (two names for the one object, as in
   nonmatching/levels/func_L00_002A7958.c) took 128 -> 120 bytes but did not remove
   the last two instructions. The if/else under the guard, its result variable and the
   evaluation order of `D_L02_d / func_002F3148()` were all measured and are correct. */

#include "include_asm.h"

typedef unsigned int u32;
typedef float f32;

extern u32 D_L02_a __attribute__((sda));
extern u32 D_L02_b __attribute__((sda));
extern f32 D_L02_c __attribute__((sda));
extern f32 D_L02_d __attribute__((sda));

extern u32 D_001B30C8 __attribute__((nosda));
extern u32 D_001B30C8_1 __attribute__((nosda));

extern f32 func_002F3148(void);

void func_L02_00332900(void) {
    if (D_L02_a != D_001B30C8) {
        if (D_L02_b == 0)
            D_L02_c = 0.0f;
        else
            D_L02_c = D_L02_d / func_002F3148();
        D_L02_b = 0;
        D_L02_a = D_001B30C8_1;
        D_L02_d = 0.0f;
    }
    *(volatile u32 *)0x10000010 = 0x81;
    *(volatile u32 *)0x10000000 = 0;
}
