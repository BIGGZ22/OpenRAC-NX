/* 0 of 27 places match - 76 bytes expected, candidate is right except two swapped loads.
   File: src/levels/shared/L00_003F8E08-g8.c (needs -O2 -G8 -ffunction-sections).
   Last line: func_L00_003FE8D0: 0 of 27 places match [00_aranos_tutorial: instruction
   at +0xc differs]. Candidate below; 46 shapes were tried (loop forms, declaration and
   assignment orders, pointer/struct spellings). Where it differs from retail:
     +0x08  mine lw $v0,0x0($gp)    | retail lw $v1,0x68($a0)
     +0x0C  mine lw $v1,0x68($a0)   | retail lw $v0,0x1C1C($gp)
     +0x14  mine sh $zero,0x6($v1)  | retail blez $v0,->end
     +0x18  mine beqz $v0,->end     | retail sh $zero,0x6($v1)
   Everything from +0x10 on matches, including the three filler nops before the
   backward bnez, the $v1=base / $v0=count roles and the sb/return tail. This cc1
   hoists the $gp count load above the base-pointer load (and then fills the bnez
   delay slot with it) where retail issues the base load first and fills the delay
   slot with that; the `sh` then sinks behind the blez in retail. A scheduling-only
   residue, same wall as the register-pair swaps: no C spelling reached it.
   The $gp member is D_L00_001B0C0C (gp_rel 0x1C1C measured from $gp = 0x001AEFF0,
   taken from the matched neighbour func_L00_003FE280 in the same -g8 unit). */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;

extern int D_L00_001B0C0C __attribute__((sda));

void func_L00_003FE8D0(char *a0)
{
    if (*(u8 *)(a0 + 0x20) == 0) {
        u32 *p = *(u32 **)(a0 + 0x68);
        s32 n = D_L00_001B0C0C;
        s32 i;
        p[0] = 0;
        *(u16 *)((char *)p + 6) = 0;
        p = (u32 *)((char *)p + 0x10);
        for (i = n; i != 0; i--) {
            *p = 0;
            p++;
        }
        *(u8 *)(a0 + 0x20) = 1;
    }
}
