/* func_L09_00408058 (220 bytes, 1 place: level 09_dobbo) - best attempt, NOT MATCHED.
 *
 * tools/check.py func_L09_00408058 --diff:
 *   09_dobbo: instruction at +0x88 differs
 *   func_L09_00408058: 0 of 1 places match (src/levels/09_dobbo/L09_003FEA48.c)
 *
 * The body is recovered: 51 of the 55 instructions are byte-identical, and the size is
 * exact.  Retail:
 *
 *   beqz $a0 -> end ; lbu $v1,0x20($a0) ; beq $v1,0xFE -> end ; beq $v1,0xFD -> end
 *   ; if (a1 != 0) { p = *(char **)(a0+0x24);
 *                    if (p == 0 || *(s16*)(p+0x46) != 0x12) {   <- the disjunction is what
 *                        v = *(u16*)(a0+0x34);                  <- gives retail's branches
 *                        if (a1 > 0) v &= 0xFFFD; else v |= 2;
 *                        *(u16*)(a0+0x34) = v; } }
 *   ; if (a2 != 0) { if (a0 == 0 || p == 0 || kind != 0x12) {
 *                        if (a2 > 0) { v = *(u16*)(a0+0x34); v &= 0xFFFE;
 *                                       *(u8*)(a0+0x31) = 1; }
 *                        else        { v = *(u16*)(a0+0x34); *(u8*)(a0+0x31) = 0;
 *                                       v |= 1; }
 *                        *(u16*)(a0+0x34) = v; } }
 *   ; if (a3 != 0) { p = *(char **)(a0+0x24); if (p != 0) {
 *                        if (a3 > 0) *(void**)(a0+0x98) = *(void**)(p+0x10);
 *                        else        *(void**)(a0+0x98) = 0; } }
 *
 * The branch polarity of the third block is the one that reads oddly: retail's
 * `blezl $a3` means the a3 <= 0 arm stores 0 and the a3 > 0 arm stores p[0x10].
 *
 * What is left is four words of register allocation in the second block only:
 *
 *   retail : blez $a2 ; addiu $v1,$zero,0x1 ; lhu $v0,0x34($a0) ; sb $v1,0x31($a0)
 *            ; b ; andi $v0,$v0,0xFFFE ; lhu $v0,0x34($a0) ; sb $zero,0x31($a0)
 *            ; ori $v0,$v0,0x1 ; sh $v0,0x34($a0)
 *   here   : blez $a2 ; addiu $v0,$zero,0x1 ; lhu $v1,0x34($a0) ; sb $v0,0x31($a0)
 *            ; b ; andi $v1,$v1,0xFFFE ; lhu $v1,0x34($a0) ; sb $zero,0x31($a0)
 *            ; ori $v1,$v1,0x1 ; sh $v1,0x34($a0)
 *
 * i.e. the materialised constant 1 and the halfword value take $v1/$v0 in retail and
 * $v0/$v1 here.  The first block matches exactly once the two blocks use SEPARATE
 * variables (one variable for the whole function makes this cc1 give the value $v1 in
 * both blocks), so the value keeps $v0 in the first block; the second block is the one
 * where the constant, materialised into the `blez` delay slot, is allocated before the
 * value and takes $v0.
 *
 * Measured, no effect (all 220 bytes, same four words):
 *   - the value as u16 and as s32;
 *   - the sub-object pointer as a named local and written inline (the inline form is
 *     needed for the `a0 == 0` test of the second block, which retail keeps);
 *   - the load of the halfword before the test and inside each arm;
 *   - the two arms written with the byte store before and after the halfword update;
 *   - the arms written the other way round (`if (a2 <= 0) ... else ...`), which this
 *     cc1 turns into a `bgtz` and a different block order;
 *   - the whole halfword loaded at the top of the function (the value then becomes live
 *     across the branches and this cc1 puts it in $t0).
 *
 * The wall is the order in which this cc1 allocates the two blocks: retail's second
 * block keeps the value in $v0, which is what happens when the arm without a constant
 * is allocated first.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;

void func_L09_00408058(char *a0, s32 a1, s32 a2, s32 a3) {
    u16 v1;
    u16 v2;

    if (a0 == 0) {
        return;
    }
    if (*(u8 *)(a0 + 0x20) == 0xFE) {
        return;
    }
    if (*(u8 *)(a0 + 0x20) == 0xFD) {
        return;
    }
    if (a1 != 0) {
        if (*(char **)(a0 + 0x24) == 0 ||
            *(s16 *)(*(char **)(a0 + 0x24) + 0x46) != 0x12) {
            v1 = *(u16 *)(a0 + 0x34);
            if (a1 > 0) {
                v1 &= 0xFFFD;
            } else {
                v1 |= 2;
            }
            *(u16 *)(a0 + 0x34) = v1;
        }
    }
    if (a2 != 0) {
        if (a0 == 0 || *(char **)(a0 + 0x24) == 0 ||
            *(s16 *)(*(char **)(a0 + 0x24) + 0x46) != 0x12) {
            if (a2 > 0) {
                v2 = *(u16 *)(a0 + 0x34);
                v2 &= 0xFFFE;
                *(u8 *)(a0 + 0x31) = 1;
            } else {
                v2 = *(u16 *)(a0 + 0x34);
                *(u8 *)(a0 + 0x31) = 0;
                v2 |= 1;
            }
            *(u16 *)(a0 + 0x34) = v2;
        }
    }
    if (a3 != 0) {
        if (*(char **)(a0 + 0x24) != 0) {
            if (a3 > 0) {
                *(void **)(a0 + 0x98) = *(void **)(*(char **)(a0 + 0x24) + 0x10);
            } else {
                *(void **)(a0 + 0x98) = 0;
            }
        }
    }
}
