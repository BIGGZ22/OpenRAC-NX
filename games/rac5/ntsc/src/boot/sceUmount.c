/* sceUmount 0x01ED7880..0x01ED78A0 (32 bytes, body 28 + 4 zero fill).
 *   addiu $sp, $sp, -0x10
 *   sd    $ra, 0x0($sp)
 *   jal   func_1ED6478
 *   addiu $a1, $zero, 0x15      (delay slot)
 *   ld    $ra, 0x0($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x10        (delay slot)
 *   nop
 * Unmounts a device through the common kernel helper with command 0x15. */
extern int func_1ED6478(char *path, int op);

int sceUmount(char *path) {
    return func_1ED6478(path, 0x15);
}
