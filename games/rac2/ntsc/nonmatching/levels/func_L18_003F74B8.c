/* func_L18_003F74B8 (212 bytes, 3 places) -- NEAR MISS, not integrated.
   Clears the sub-object's +0x20 word, lowers the entry's +0x34 flag and runs down
   the +0x292 counter; while the counter is not -1, or the +0x296 word is 0xE, it
   either registers the func_L18_003F7590 callback through func_L01_003E9CF8(s0, 0,
   cb) or calls func_L18_003F7590 directly, depending on the second argument, and
   the other path does the same with func_0029F338 / a null callback.
   The candidate is exactly retail's size (212) with every call, both flag stores,
   the sign-extended counter test and the arm layout matching (the arms were
   inverted until the test was written `t != -1 || *(s16 *)(s1 + 0x296) == 0xE`).
   What is left is scheduling and one register: this cc1 puts the first call's
   `addiu $a1,zero,0x13` in the jal's delay slot (retail has it before the jal and
   the `sw $zero,0x20($s1)` after it in the slot), and it sign-extends the counter
   into $v0 where retail uses $v1.  A `s32 mode = 0x13;` spelling changes nothing. */
#include "common.h"

void func_L18_003F74B8(char *a0, s32 a1)
{
    char *s0 = a0;
    s32 s2 = a1;
    char *s1 = *(char **)(s0 + 0x68);
    s16 t;
    s32 mode = 0x13;

    func_002A7750(s0, mode, 0);
    *(s32 *)(s1 + 0x20) = 0;
    *(u16 *)(s0 + 0x34) &= 0xEFFF;
    t = *(u16 *)(s1 + 0x292) - 1;
    *(u16 *)(s1 + 0x292) = t;
    if (t != -1 || *(s16 *)(s1 + 0x296) == 0xE) {
        if (s2 != 0) {
            func_L18_003F7590(s0);
        } else {
            func_L01_003E9CF8(s0, 0, func_L18_003F7590);
        }
    } else {
        if (s2 != 0) {
            func_0029F338(s0);
        } else {
            func_L01_003E9CF8(s0, 0, 0);
        }
    }
}
