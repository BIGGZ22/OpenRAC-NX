#include "common.h"

/* Near miss: the candidate is retail's exact 96 bytes and the sequence of
 * calls, branches and the 128-bit copy is right, but the pointer arithmetic
 * and the block order differ.
 *
 * Retail computes the destination `a0 + 0x10` in 32 bits (`addiu $s0, $a0,
 * 0x10`) and sign-extends it for the call (`daddu $a0, $s0, $zero`), and it
 * stores the quadword through that same 32-bit value (`sq $v0, 0($s0)`). This
 * cc1 sign-extends first (`daddu $s0, $a0, $zero`) and then adds in 64 bits
 * (`addiu $a0, $s0, 0x10`), and it stores through `a0 + 0x10` (`sq $v0,
 * 0x10($s0)`). Every spelling of the address tried here (an int local, an
 * unsigned local, a pointer built from a cast, a pointer local) gives the
 * same 64-bit arithmetic.
 *
 * The 128-bit copy itself comes out as retail's `lq`/`sq` pair, so a
 * `mode(TI)` local is the right shape.
 */

typedef int TI128 __attribute__((mode(TI)));

extern TI128 D_001BFFAFA0;

extern s32 func_00277258(void *, s32, void *, float);

s64 func_002ED948(s32 a0)
{
    s64 r = 0;
    s32 p = a0 + 0x10;

    if (func_00277258(p, 2, a0, 1.5f) != 0) {
        *(TI128 *)p = D_001BFFAFA0;
        r = 1;
    }
    return r;
}
