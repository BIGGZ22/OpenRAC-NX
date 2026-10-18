/* close 0x01ED1320..0x01ED1328 (8 bytes).
 *   jr    $ra
 *   addiu $v0, $zero, -1
 * Stub of the IOP/EE kernel file layer: always fails. */
int close(int fd) {
    return -1;
}
