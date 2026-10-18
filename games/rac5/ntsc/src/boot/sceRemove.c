/* sceRemove 0x01ED6628..0x01ED6648 (32 bytes, body 28 + 4 zero fill).
 *   addiu $sp, $sp, -0x10
 *   sd    $ra, 0x0($sp)
 *   jal   func_1ED6478
 *   addiu $a1, $zero, 0x6       (delay slot)
 *   ld    $ra, 0x0($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x10        (delay slot)
 *   nop
 * Removes a file through the common kernel helper with command 6. */
extern int func_1ED6478(char *path, int op);

int sceRemove(char *path) {
    return func_1ED6478(path, 6);
}
