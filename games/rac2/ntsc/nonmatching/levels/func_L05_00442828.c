/* Abandoned attempt - func_L05_00442828, 52 bytes, 3 places.
   Belongs in the -g8 sibling (both $gp words are reached in ordinary flow).
   Best result: 48 bytes, expected 52. Body verified correct against retail:
   the two 128-bit copies load from a1 and a0 and store to 0x1B3610 / 0x1B3620.
   Residual differences, in retail instruction order:
     1  addiu $v0,$zero,1          the constant precedes everything (mine: 2nd block)
     2  lui/a3 = 0x1B3610          materialised before the two $gp stores
     3  sw $v0,0x2B30($gp)         the constant store precedes the a2 store
     4  sw $a2,0x2B34($gp)
        lq $v0,0($a1) ; sq $v0,0($a3)      (value register $v0 in retail, $v1 here)
        lui/v1 = 0x1B3620
        lq $v0,0($a0) ; sq $v0,0($v1)
        jr $ra ; nop                 retail leaves the delay slot empty (mine fills it,
                                     which is why the body is one word short)
   Tried: natural order, reversed store order, a shared TI128 temporary, a
   TI128* local for the addresses. None reproduced the register choices or the
   empty delay slot. */
#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern int D_L05_001B1B20 __attribute__((sda));
extern int D_L05_001B1B24 __attribute__((sda));
extern TI128 D_L05_001B3610;
extern TI128 D_L05_001B3620;

void func_L05_00442828(char *a0, char *a1, int a2)
{
    D_L05_001B1B20 = 1;
    D_L05_001B1B24 = a2;
    *(TI128 *)&D_L05_001B3610 = *(TI128 *)a1;
    *(TI128 *)&D_L05_001B3620 = *(TI128 *)a0;
}
