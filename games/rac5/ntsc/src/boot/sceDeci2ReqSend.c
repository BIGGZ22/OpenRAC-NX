/* sceDeci2ReqSend 0x01EDA2C0..0x01EDA2F0 (48 bytes).
 *   sll   $a1, $a1, 24
 *   addiu $sp, $sp, -0x20
 *   sra   $a1, $a1, 24
 *   sw    $a0, 0x0($sp)
 *   sw    $a1, 0x4($sp)
 *   addiu $a0, $zero, 0x3
 *   sd    $ra, 0x10($sp)
 *   jal   Deci2Call
 *   daddu $a1, $sp, $zero          (delay slot)
 *   ld    $ra, 0x10($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x20           (delay slot)
 * Sends a Deci2 request: the option byte is sign extended into the message. */
extern int Deci2Call(int cmd, void *arg);

int sceDeci2ReqSend(int id, int opt) {
    int buf[2];

    buf[0] = id;
    buf[1] = (signed char)opt;
    return Deci2Call(3, buf);
}
