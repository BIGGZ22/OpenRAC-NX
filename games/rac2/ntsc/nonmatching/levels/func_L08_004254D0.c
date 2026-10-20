/* func_L08_004254D0 (72 bytes, 1 place) - best attempt, NOT MATCHED.
 *
 * Docker-free ruler (scratch/wslcmp.py): "08_tabora: instruction at +0xc differs", right
 * size after the fix below.  The whole body is the sibling func_L08_00425518's, minus its
 * live-state test, and matches instruction for instruction except the two input registers
 * of the table lookup:
 *   retail : lw $v1,0x39EC($gp) ; sll $v0,$a0,8 ; addu $v0,$v1,$v0 ; beqz $v0, ...
 *   mine   : lw $v0,0x0($gp)    ; sll $v1,$a0,8 ; addu $v0,$v0,$v1 ; [daddu $a0,$zero,$zero]
 *            then beqz
 * so the table holds $v1 and the shifted index $v0 in retail, the other way round here, and
 * this cc1 hoists `a0 = 0` above the addu where retail sinks it into the beqz delay slot.
 *
 * Measured, no effect: the sibling's exact two-local spelling (`v0 = a0 << 8; v1 = D_...;
 * v1 = v1 + v0;`), the same with the two statements swapped, one `char *p` local holding the
 * sum (`p = (char *)D_L08_001B29DC + (a0 << 8)`), and the early-return form `if (a0 == -1)
 * return 1;` (which additionally flips the guard from `beq` to `bne`).  The sibling
 * func_L08_00425518 does match with the two-local spelling, so the difference is the
 * allocator's response to the shorter body, not the spelling.  Register-allocation wall.
 */

typedef unsigned char u8;
typedef signed int s32;

extern s32 D_L08_001B29DC __attribute__((sda));

s32 func_L08_004254D0(s32 a0) {
    char *p;
    u8 state;
    if (a0 != -1) {
        p = (char *)D_L08_001B29DC + (a0 << 8);
        state = *(u8 *)(p + 0x20);
        a0 = 0;
        if (p == 0 || state == 0xFE || state == 0xFD) {
            a0 = 1;
        }
        return a0;
    }
    return 1;
}
