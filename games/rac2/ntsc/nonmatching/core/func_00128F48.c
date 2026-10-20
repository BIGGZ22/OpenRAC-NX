/* from src/core/00128D58.c: the row is not one function body.
 *
 * The 136 bytes are three pieces:
 *   +0x00  `sb $v1,0x0($a1)`      - one word, the delay slot of a stripped
 *                                   function's last jump (in rac1-decomp's
 *                                   dead-strip model a function of size 4 mod 8
 *                                   leaves exactly its final word behind);
 *   +0x04  0xCDCDCDCD             - the linker's fill (rac1-decomp's
 *                                   src/libgcc/README.md documents the same
 *                                   word filling its two retail gaps);
 *   +0x08  the real body, 128 bytes, ending in `jr $ra; addiu $sp,$sp,0x20`.
 *
 * The body itself resists C for a second, independent reason: it builds its
 * argument out of consecutive constant masks with operations that no GCC
 * folds away -
 *     $v0 = 0xFFFFC000; and $v0,$v0,$v0;  or $v0,$v0,2;
 *     and $v0,$v0,0xFFFF3FFF; or $v0,$v0,0x8000; and $v0,$v0,0xFFFFFFFF;
 *     or $v0,$v0,3; $t0 = -1 (via dsll/ori, dsll/ori); and $v0,$v0,$t0;
 *     or $a1,$v0,0x01000000
 * - `and $v0,$v0,$v0` and `and $v0,$v0,-1` are no-ops that a compiler
 * constant-folds, and the 64-bit -1 built with two dsll/ori pairs is the
 * signature of hand-written bit-field code. It then calls func_001287A8 with
 * a1 = 0xFFFF8003 | 0x01000000 and a2 = the address of a stack slot, returns
 * the slot's word when the call returns >= 0 and the call's result otherwise.
 *
 * Two separate walls, so this is not a near miss with a residual to steer:
 * the row start is a tools/levels.py question and the body is not plain C.
 * No compile was spent.
 */
