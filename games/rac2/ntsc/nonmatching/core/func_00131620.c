/* from src/core/001315E0.c: the catalogue row is 72 bytes, the function is 64 */
/*
 * The row starts at 0x131620, where retail has
 *
 *     +0x00  sdr  $v1, 0x0($a3)
 *     +0x04  nop
 *     +0x08  addiu $sp, $sp, -0x20      <- a normal prologue: the body starts here
 *
 * Nothing reaches those two words: no `jr $ra` precedes them (the previous row,
 * func_001315E0, 64 bytes, already returns at 0x13161C), this function never sets
 * $a3, and the sibling row func_00131668 (4 bytes) is the same kind of fragment -
 * it repeats the last instruction of this body. So the catalogue boundary, not
 * the C, is what is 8 bytes out (tools/levels.py).
 *
 * The C below is the whole body, at the unit's SDK profile (`sdk -O2 -G2
 * -ffunction-sections`, `-G0`/`-G2` both tried) it compiles to 64 bytes, one
 * instruction for instruction with retail 0x131628-0x131668 - the prologue, the
 * call, the arm order (`beqz` to the call, `lui` in its delay slot, `b` with the
 * `lbu` in its delay slot), the `lw`/`srl`/`andi` tail and the epilogue - except
 * that the global's address goes to $v0 and is reused as the load destination
 * (this cc1's habit for the two `...D6`/`...D0` siblings above), where retail
 * materialises it in $v1 and leaves the value in $v0.
 *
 * D_00138152 is a byte global at an address outside the small-data window: it
 * needs NOT_SDA, or the file's -G2 makes the load $gp-relative.
 */
extern int func_001315E0(void);

extern void func_0011ACD0(void *);

extern unsigned char D_00138152;

int func_00131620(void) {
    unsigned int local;
    int r;

    if (func_001315E0() != 0) {
        r = D_00138152;
    } else {
        func_0011ACD0(&local);
        r = (local >> 1) & 3;
    }
    return r;
}
