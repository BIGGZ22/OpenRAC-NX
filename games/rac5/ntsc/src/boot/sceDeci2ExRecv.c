/* sceDeci2ExRecv 0x01EDA320..0x01EDA358 (56 bytes, body 52 + 4 zero fill).
 *   addiu $sp, $sp, -0x20
 *   daddu $v0, $a1, $zero
 *   andi  $a2, $a2, 0xFFFF
 *   sw    $a0, 0x0($sp)
 *   sd    $ra, 0x10($sp)
 *   daddu $a1, $sp, $zero
 *   sw    $v0, 0x4($sp)
 *   addiu $a0, $zero, -0x5
 *   jal   Deci2Call
 *   sw    $a2, 0x8($sp)              (delay slot)
 *   ld    $ra, 0x10($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x20             (delay slot)
 *   nop
 * Extended Deci2 receive; the length word is masked to 16 bits.
 * The two named locals and the statement order are load bearing: they are what
 * makes GCC 2.9 keep the length in $a2 and store the second word through $v0. */
extern int Deci2Call(int cmd, void *arg);

int sceDeci2ExRecv(int a0, int a1, int a2) {
    int buf[3];
    int x = a1;
    unsigned int len = a2 & 0xFFFF;

    buf[1] = x;
    buf[2] = len;
    buf[0] = a0;
    return Deci2Call(-5, buf);
}
