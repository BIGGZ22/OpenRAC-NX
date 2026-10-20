#include "include_asm.h"

extern int D_001A73B8[];
extern int **D_001B18D4;

/* Near miss for func_0028BD68 (boot 0x0028BD68, 27 levels, 72 bytes).
 *
 * Retail body:
 *   lui    $a3, 0x1B ; lw $a3, 0x18D4($a3) ; daddu $a2, $zero, $zero
 *   lui    $a0, 0x1A ; addiu $a0, $a0, 0x73B8 ; addiu $a1, $zero, 0x7
 *   lw     $v0, 0($a3) ; addiu $a1, $a1, -1 ; lw $v1, 0($a0)
 *   addu   $v0, $a2, $v0 ; lw $v0, 0x18($v0) ; beq $v1, $v0, +8
 *   addiu  $a2, $a2, 0x1C ; sw $v0, 0($a0) ; bgez $a1, -0x24
 *   addiu  $a0, $a0, 0x4 ; jr $ra ; nop
 *
 * The body, the loop and the 0x1C stride are recovered; what is left is allocation:
 * retail keeps the four values in $a0-$a3 (the table pointer, the item pointer, the
 * counter and the count), this cc1 puts them in $a0-$a3 in another order and adds a
 * load-delay nop after the value load.  A body without arguments lands them in
 * $s0-$s3 instead.
 */
void func_0028BD68(void) {
    int i = 0;
    int *p = D_001A73B8;
    int *table = D_001B18D4[0];
    int count = 7;
    do {
        int v = *(int *)((char *)(*table + i) + 0x18);
        count--;
        if (*p != v) *p = v;
        i += 0x1C;
        p++;
    } while (count >= 0);
}
