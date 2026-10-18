/* sceSifLoadModule 0x01ED87B8..0x01ED87D8 (32 bytes).
 *   addiu $sp, $sp, -0x20
 *   daddu $t0, $zero, $zero
 *   sd    $ra, 0x10($sp)
 *   jal   func_1ED8590
 *   daddu $a3, $sp, $zero          (delay slot)
 *   ld    $ra, 0x10($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x20           (delay slot)
 * Loads a module and discards the module result word (5th argument is 0). */
extern int func_1ED8590(char *path, int arglen, char *args, int *modres, int t0);

int sceSifLoadModule(char *path, int arglen, char *args) {
    int modres;

    return func_1ED8590(path, arglen, args, &modres, 0);
}
