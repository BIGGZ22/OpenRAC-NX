/* NEAR MISS - 60 bytes emitted, 60 expected: the size is right, the order of the
 * first two loads is not.
 * last check.py line: "14_aranos_prison: instruction at +0x0 differs" /
 * "func_L14_00427210: 0 of 1 places match"
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
 *   00427230 (delay slot of the beql) addiu $a1,$zero,1
 *   00427234 lw    $v0,0x4($v1)
 *   00427238 beqz  $v0 -> RET
 *   0042723C nop
 *   00427240 addiu $a1,$zero,1            ; R1
 *   00427244 jr    $ra                    ; RET
 *   00427248 daddu $v0,$a1,$zero
 *
 * Wave 9 (p1) found the spelling that reaches the retail's size: the two flag
 * tests must be a goto to a shared tail, not an if/else chain - the chain gets
 * if-converted (sltu $v0,$zero,$v0) and the body grows.  With the goto the body
 * is exactly 60 bytes and 12 of the 15 words are identical.  The two words left
 * are the order of the first two loads: retail loads 0x68 before 0x20 (the state
 * pointer in $v1, the flag byte in $a0), this cc1 always loads the flag byte
 * first (in $v1) and the state pointer second (in $a0) - the source order of the
 * two loads does not change that.  The beql in the retail is likely and copies
 * the "r = 1" into its delay slot; here it is the plain beq.
 */
int func_L14_00427210(char *a0)
{
    int r = 0;
    char *st;
    unsigned char b;

    if (a0 == 0)
        goto one;
    b = *(unsigned char *)(a0 + 0x20);
    st = *(char **)(a0 + 0x68);
    if (b == 0xFE)
        goto one;
    if (b == 0xFD)
        goto ret;
    r = 1;
    if (*(int *)(st + 4) == 0)
        goto ret;
one:
    r = 1;
ret:
    return r;
}
