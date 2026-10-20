/* 0 of 2 places match - last line: 04_barlow: 116 bytes, expected 100
   Candidate is 116 bytes against retail's 100. The body, the -1 sentinel, the <<8 stride
   against the level's base word and both byte stores are right; the loop form is not:
   retail decrements the counter at the top of the block and keeps ONE induction pointer
   (`lw $v0,0x0($a0)` / `addiu $a1,$a1,-0x1` / `beq $v0,$t0,exit` / ... / `bgez $a1,loop` /
   `addiu $a0,$a0,0x4`), this cc1 emits a two-pointer loop with a second exit test
   (`bltz` + `bne`). Three source shapes tried (`for (i = 20; i >= 0; i--, p++)`,
   the pointer advanced at the end of the body, `do/while (--i >= 0)`) all give 116.
   The global at 0x001B24DC is read absolutely here, so it needs a second name: the unit's
   own declaration is the $gp view. */

#include "include_asm.h"

extern unsigned D_001B24DC_abs;

void func_L04_0040A5C0(char *a0)
{
    int *p = (int *)(*(int *)(a0 + 0x68) + 0x1E4);
    int i;

    for (i = 20; i >= 0; i--) {
        int id = *p;

        if (id == -1)
            break;
        *(unsigned char *)(*(int *)(D_001B24DC_abs + (id << 8)) + 0xA0) = 1;
        *(unsigned char *)(D_001B24DC_abs + (*p << 8) + 0x20) = 2;
        p++;
    }
}
