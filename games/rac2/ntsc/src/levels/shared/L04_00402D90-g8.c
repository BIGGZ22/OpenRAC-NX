#include "include_asm.h"

/* Bodies whose retail bytes reach their globals through $gp: this file is built -G8.
   Measured here: cc1 emits `sw $r,SYM` as a macro and GAS expands it. Outside a delay
   slot it always becomes `lui $at,%hi ; sw %lo($at)` (HI16/LO16) whatever the attribute;
   inside a delay slot (.set nomacro: one instruction only) it becomes the $gp-relative
   form with a GPREL16 relocation, whose symbol must therefore be named GP + the retail
   displacement. `sda` only adds `.extern SYM,size`. */

extern unsigned int D_L04_001A8ED0;

extern unsigned int D_L04_001B0908 __attribute__((sda));

void func_L04_0040AC80(void)
{
    D_L04_001A8ED0 = 2;
    D_L04_001B0908 = 1;
}

extern unsigned int D_L04_001B0910 __attribute__((sda));

void func_L04_0040AC98(void)
{
    D_L04_001B0910 = 1;
}

/* func_L04_0040AC30 reaches its globals off $gp too, but its body is not matched
   yet; its marker stays in L04_00402D90.c and the attempt is in the wave-8 drafts. */

/* 0x001A8ED0 is absolute in func_L04_0040AC80 and $gp-relative here, so it is
   reached under a second name, declared small data.  The store to 0x001A8EE4 is
   written last in the source: cc1 emits the last store of a straight-line run
   first and then the source order, which is the order retail has. */
extern unsigned int D_L04_001A8ED0_1 __attribute__((sda));

extern unsigned int D_L04_001A8EE4;

void func_L04_0040ACA8(void)
{
    D_L04_001A8EE4 = 1;
    D_L04_001A8ED0_1 = 2;
    D_L04_001B0908 = 2;
}
