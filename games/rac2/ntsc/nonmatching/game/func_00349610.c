#include "include_asm.h"

/* Near miss for func_00349610 (boot 0x00349610, level 00 0x0043EF40, 27 levels).
 *
 * Retail body:
 *   sll   $a1, $a1, 2
 *   addu  $v0, $a0, $a1
 *   daddu $a0, $v0, $zero
 *   swc1  $f12, 0x0($v0)
 *   jr    $ra
 *   swc1  $f13, 0xC($a0)
 *
 * Retail keeps the plain pointer in $v0 for the first store and a 64-bit copy of it in
 * the pair ($a0,$a1) for the second one, so the two stores cannot be reordered (different
 * bases) and the second store's offset 12 is folded into its displacement. This cc1 folds
 * the 64-bit copy away (dsll/dsra into a single register) and then reorders the stores.
 */
void func_00349610(char *o, int n, float a, float b) {
    float *p = (float *)(o + n * 4);
    p[0] = a;
    p[3] = b;
}
