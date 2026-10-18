/* sceSifLoadStartModule 0x01ED87D8..0x01ED8818 (64 bytes).
 *   addiu $sp, $sp, -0x10
 *   sd    $ra, 0x0($sp)
 *   jal   func_1ED8590
 *   daddu $t0, $zero, $zero        (delay slot)
 *   ld    $ra, 0x0($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x10           (delay slot)
 * The 28 bytes above are the whole function; the interval then continues with
 * a nop plus 36 bytes of unlabelled epilogue fragments (addiu $sp,$sp,N / nop
 * pairs) that are not part of it. */
extern int func_1ED8590(char *path, int arglen, char *args, int *modres, int t0);

int sceSifLoadStartModule(char *path, int arglen, char *args, int *modres) {
    return func_1ED8590(path, arglen, args, modres, 0);
}
