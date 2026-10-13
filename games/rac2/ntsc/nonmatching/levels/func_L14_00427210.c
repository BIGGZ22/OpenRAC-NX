/* NEAR MISS — 68 bytes emitted, 60 expected.
 * last check.py line: "14_aranos_prison: 68 bytes, expected 60" / "func_L14_00427210: 0 of 1 places match"
 *
 * Retail keeps the result in $a1 and ONE shared return block at the end:
 *   00427210 daddu $a1,$zero,$zero        ; r = 0
 *   00427214 beqz  $a0 -> R1              ; a0 == 0  -> r = 1
 *   00427218 lw    $v1,0x68($a0)          ; delay slot: p
 *   0042721C lbu   $a0,0x20($a0)          ; t
 *   00427220 addiu $v0,$zero,0xFE
 *   00427224 beq   $a0,$v0 -> R1
 *   00427228 addiu $v0,$zero,0xFD         ; delay slot
 *   0042722C beql  $a0,$v0 -> RET         ; delay slot: addiu $a1,$zero,1
 *   00427230 addiu $a1,$zero,0x1
 *   00427234 lw    $v0,0x4($v1)
 *   00427238 beqz  $v0 -> RET
 *   R1: 00427240 addiu $a1,$zero,1
 *   RET: 00427244 jr $ra ; daddu $v0,$a1,$zero
 *
 * Written forms tried, all with the trailing else-if in an if/else chain:
 *   - if (a0==0) r=1; else {...}            -> bnel over the then-block, r lands in $v0
 *   - if (a0!=0) {...} else { r=1; }        -> beqz to a shared R1 (correct) but the last
 *                                             branch is if-converted to `sltu $v0,$zero,$v0`
 *                                             instead of the retail `beqz $v0 / addiu $a1,1`.
 * The if-conversion of the trailing `else if (*(int*)(p+4) != 0) r = 1;` is the residual wall
 * (see docs/COMPILER-NOTES.md "If-conversion is the small-tier wall").
 */
int func_L14_00427210(char *a0)
{
    int r;
    char *p;
    unsigned char t;

    r = 0;
    if (a0 != 0) {
        p = *(char **)(a0 + 0x68);
        t = *(unsigned char *)(a0 + 0x20);
        if (t == 0xFE) {
            r = 1;
        } else if (t == 0xFD) {
            r = 1;
        } else if (*(int *)(p + 4) != 0) {
            r = 1;
        }
    } else {
        r = 1;
    }
    return r;
}
