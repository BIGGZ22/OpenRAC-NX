#include "include_asm.h"

typedef unsigned short u16;

extern u16 *D_001B188C;

/* Near miss for func_0028AB20 (boot 0x0028AB20, 27 levels, 76 bytes).
 *
 * Retail body: a scan over a u16 table with an 8-byte stride that counts the entries
 * it walks and returns the count:
 *   lui $a1, 0x1B ; lw $a1, 0x188C($a1) ; ori $v1, $zero, 0xFFFF ; lhu $v0, 0($a1)
 *   beq $v0, $v1, END ; daddu $a2, $zero, $zero ; beq $v0, $a0, END
 *   daddu $v1, $a1, $zero ; ori $a1, $zero, 0xFFFF ; addiu $v1, $v1, 0x8
 *   L: lhu $v0, 0($v1) ; beq $v0, $a1, END ; addiu $a2, $a2, 0x1 ; nop ; nop
 *      bne $v0, $a0, L ; addiu $v1, $v1, 0x8 ; END: jr $ra ; daddu $v0, $a2, $zero
 *
 * The scan is recovered; what is left is that retail keeps the table pointer in a
 * 64-bit register (the `daddu $v1, $a1, $zero` copy, and the 8-byte stride added to
 * its low word only), while this cc1 keeps a 32-bit pointer and sign-extends it
 * (dsll/dsra) wherever it is used.
 */
int func_0028AB20(int value) {
    u16 *p = D_001B188C;
    int count = 0;
    if (p[0] == 0xFFFF) return count;
    if (p[0] == value) return count;
    p += 4;
    while (p[0] != 0xFFFF) {
        count++;
        if (p[0] == value) break;
        p += 4;
    }
    return count;
}
