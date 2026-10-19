/* fstat 0x01ED13E0..0x01ED13F8 (24 bytes, body 20 + 4 zero fill).
 *   addiu $v0, $zero, 0x2000
 *   sd    $zero, 0x48($a1)
 *   sw    $v0, 0x4($a1)
 *   jr    $ra
 *   daddu $v0, $zero, $zero
 * Kernel file layer stub: fills the caller's stat buffer with a fixed mode
 * and a zeroed 64-bit size field, then reports success. */
int fstat(int fd, void *buf) {
    *(long long *)((char *)buf + 0x48) = 0;
    *(int *)((char *)buf + 0x4) = 0x2000;
    return 0;
}
