/* func_L00_003ACC88, 112 bytes, 27 placements.  ABANDON - compiler-profile wall.
   The retail materialises a 128-bit zero with an MMI pair (0x700014A9 =
   pcpyld $v0,$zero,$zero) and stores the register form (`sq $v0,0x10($sp)`).
   rac2_fold_zero_ti_store folds exactly that store to `sq $zero` whenever the
   register is dead afterwards, which it is here, so the register form cannot be
   produced from C under the pinned chain.  Last line of tools/check.py:
   "00_aranos_tutorial: instruction at +0x1c differs" (sq $zero vs pcpyld+sq). */

#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern float D_L00_001AF7A4 __attribute__((sda));

extern void func_2EE5B0();

extern void func_2EE108();

void func_L00_003ACC88(char *a0, char *a1)
{
    float local[4];
    float other[4];
    TI128 zero = 0;

    *(TI128 *)local = zero;
    local[0] = D_L00_001AF7A4;
    local[3] = 1.0f;
    *(TI128 *)other = *(TI128 *)local;
    func_2EE5B0(other, other, a0 + 0xC0);
    func_2EE108(a1, other, a0 + 0x10);
}
