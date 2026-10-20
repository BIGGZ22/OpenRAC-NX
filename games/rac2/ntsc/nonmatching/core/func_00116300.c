/* from src/core/00115200.c: C that compiles to 88 bytes where retail has 96 */
/*
 * The unit saves $ra, $s0 and $s1 in TImode slots (`sq`), so it needs the
 * -mastra-quad-saves profile; with it the prologue, the two calls, the stores
 * and the epilogue are all exact.
 *
 * What differs is the layout of the first branch. Retail emits
 *
 *     +0x1C  lw   $v0, D_001341E0
 *     +0x20  bne  $v0, $s0, <block>
 *     +0x24  lui  $v0, 0x13            (delay slot)
 *     +0x28  b    <tail>
 *     +0x2C  lui  $v1, 0x13            (delay slot)
 *     +0x30  <block>: jal func_0011AC60 ; lw $a0, 0x4DB8($v0) ; lui ; sw
 *     +0x40  <tail>:  lw $v0, 0x41E4($v1) ; addiu ; sw ; restores ; jr
 *
 * so the empty arm carries a jump of its own and the block is not the
 * fall-through; this cc1 lays the block out as the fall-through and needs no
 * second branch (88 bytes). Tried: the plain if/block form, an explicit empty
 * else, and `else goto done;` with a shared `done:` label - all three compile
 * to the same 88-byte body, so this is the scheduler's block layout and not a
 * source shape. The block's `lui $v1` in the branch delay slot is part of the
 * same difference.
 */
extern int D_001341E0;

extern int D_001341E4;

extern int D_00134DB8;

extern int func_0011AB10(void);

extern void func_0011AC60(int arg0);

int func_00116300(void) {
    int id = func_0011AB10();

    if (D_001341E0 != id) {
        func_0011AC60(D_00134DB8);
        D_001341E0 = id;
    }
    return ++D_001341E4;
}
