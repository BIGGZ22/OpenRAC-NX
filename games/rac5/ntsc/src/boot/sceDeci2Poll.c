/* sceDeci2Poll 0x01EDA2F0..0x01EDA320 (48 bytes).
 *   addiu $sp, $sp, -0x20
 *   sw    $a0, 0x0($sp)
 *   daddu $a1, $sp, $zero
 *   sd    $ra, 0x10($sp)
 *   jal   Deci2Call
 *   addiu $a0, $zero, 0x4          (delay slot)
 *   ld    $ra, 0x10($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x20           (delay slot)
 * The 40 bytes above are the whole function (the interval then continues with
 * 8 bytes of unlabelled epilogue fragments that are not part of it). */
extern int Deci2Call(int cmd, void *arg);

int sceDeci2Poll(int a0) {
    int buf[1];

    buf[0] = a0;
    return Deci2Call(4, buf);
}
