/* lseek 0x01ED1328..0x01ED1330 (8 bytes).
 *   jr    $ra
 *   addiu $v0, $zero, -1
 * Stub of the IOP/EE kernel file layer: always fails. */
int lseek(int fd, int offset, int whence) {
    return -1;
}
