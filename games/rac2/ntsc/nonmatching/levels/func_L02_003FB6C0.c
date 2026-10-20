/* func_L02_003FB6C0, 288 bytes, 1 placement (level 2, 02_maktar_nebula).
   NEAR MISS: 292 bytes against 288.  Everything reproduces except two words:
   the two values that cross the calls are given the callee-saved registers the
   other way round (retail keeps the copy of a1 in $s1 and the table base in
   $s2, this cc1 keeps the table base in $s1), and the tail's delay slot
   differs because of it (retail: `swc1 $f0,0x20($v1)` then `b` with
   `addiu $v0,$zero,1` in the slot; here the constant is hoisted in front of
   the store and the store goes into the slot).

   What does reproduce: the prologue, the table base materialised in a register
   (`addiu $s2,$s0,0xA8` then `addu $v0,$s2,$v0` then `lw $a0,0($v0)` - it
   needs the base in a variable, an expression folds the 0xA8 into the load),
   the two calls with the index recomputed after the first one, the switch
   tree (root 0xB22 with the 0xB23 pivot, left 0x9CA, right 0xC3B/0xC3E -
   this cc1's own balanced tree for those four cases), the four counter
   increments, the 1.0f and the shared exit.

   The register swap was chased with: the copy of a1 written before/after the
   object load, the copy used in the first call or only in the second, the
   parameter used directly everywhere (no local copy), and the local copy used
   in all three later uses.  All four give the table base $s1.  The retail
   allocation gives the value with the longest live range (the copy of a1, used
   after the second call) the lower register; this cc1 gives it the higher one.
   That is what is left to unblock it. */

#include "include_asm.h"

typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern s32 func_L02_003D77B8(void *, void *);
extern void func_L02_003D7868(void *, void *);

s32 func_L02_003FB6C0(char *a0, char *a1) {
    char *s0 = *(char **)(a0 + 0x68);
    char *s2;
    s32 v0;
    s32 v1;
    float f0;

    v0 = *(s32 *)(s0 + 0xD8);
    s2 = s0 + 0xA8;
    a0 = *(char **)(s2 + v0 * 4);
    if (a0 != 0) {
        if (func_L02_003D77B8(a0, a1) != 0) {
            a0 = *(char **)(s2 + *(s32 *)(s0 + 0xD8) * 4);
            func_L02_003D7868(a0, a1);
            v1 = *(s16 *)(a1 + 0xAA);
            switch (v1) {
            case 0x9CA:
                (*(s32 *)(s0 + 0x90))++;
                f0 = 1.0f;
                break;
            case 0xB22:
                (*(s32 *)(s0 + 0x94))++;
                f0 = 1.0f;
                break;
            case 0xC3B:
                (*(s32 *)(s0 + 0x98))++;
                f0 = 1.0f;
                break;
            case 0xC3E:
                (*(s32 *)(s0 + 0x9C))++;
                f0 = 1.0f;
                break;
            default:
                return 1;
            }
            *(float *)(*(char **)(a1 + 0x68) + 0x20) = f0;
            return 1;
        }
    }
    return 0;
}
