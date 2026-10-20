/* 0 of 3 places match - 48 or 60 bytes produced, 56 expected.
   Level 18 (18_damosel) 0x0042840C etc. Retail is a plain two-case compare chain:
     lh $v1,0xAA($a0) ; addiu $v0,$zero,0xEC0 ; beq $v1,$v0,.E1
     addiu $v0,$zero,0xEE1 ; beq $v1,$v0,.E2
     addiu $v0,$zero,-1 ; b .End ; nop
     .E1: jr $ra ; addiu $v0,$zero,3
     .E2: jr $ra ; addiu $v0,$zero,0xA
     .End: jr $ra ; nop
   Every spelling of this cc1 (switch with returns, switch on a result variable with
   break, default-first, if/else-if chain, `switch (int)`) comes out either 48 bytes
   or a branchless select (`xori` + `movz`, 44-60 bytes); the retail keeps the compare
   chain and gives the two case arms their own `jr $ra`. Nothing reached the plain
   chain, so the wall is this cc1's if-conversion of a two-way select. */

#include "include_asm.h"

typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;

s32 func_L18_004283F0(u8 *a0)
{
    s32 r;

    switch (*(s16 *)(a0 + 0xAA)) {
    case 0xEC0: r = 3; break;
    case 0xEE1: r = 10; break;
    default: r = -1; break;
    }
    return r;
}
