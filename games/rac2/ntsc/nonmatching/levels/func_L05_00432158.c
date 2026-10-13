/* Abandoned attempt - func_L05_00432158, 68 bytes, 4 places.
   Retail body (05_feltzin_system@0x00432158):
     addiu $a2,$a1,0x10 ; addiu $v1,$a0,0x10 ; lq $v0,0($v1) ; sq $v0,0($a2)
     addiu $a0,$a0,0xC0 ; addiu $a1,$a1,0xC0 ; daddu $v1,$zero,$zero ; nop
     L: lq $v0,0($a0) ; sq $v0,0($a1) ; addiu $v1,$v1,0x1 ; addiu $a0,$a0,0x10
        slti $v0,$v1,0x3 ; bnez $v0,L ; addiu $a1,$a1,0x10   (delay)
   So: element 1 copied, then a three-element loop over elements 12..14, with an
   incrementing counter compared by slti and a nop in front of the loop (the
   assembler's short-loop padding, since the loop body is six instructions).
   Best result: 60 bytes, expected 68. Every version came out strength-reduced
   (`addiu $v1,$zero,0x2` / `daddiu $v1,$v1,-0x1` / `bgez`), i.e. the decrementing
   form, and the padding nop landed inside the loop instead of before it.
   Tried: pointer increments, indexed form a1[12+i] = a0[12+i], int and long
   counters. */
#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

void func_L05_00432158(TI128 *a0, TI128 *a1)
{
    int i;

    a1[1] = a0[1];
    for (i = 0; i < 3; i++) {
        a1[12 + i] = a0[12 + i];
    }
}
