/* sceSifSetCmdBuffer 0x01ED38F8..0x01ED3910 (24 bytes).
 *   lui   $v1, 0x1FE
 *   addiu $v1, $v1, 0x3E98
 *   lw    $v0, 0x14($v1)
 *   sw    $a1, 0x18($v1)
 *   jr    $ra
 *   sw    $a0, 0x14($v1)           (delay slot)
 * Installs the two SIF command buffers and returns the previous one.
 * Statement order matters: the store of the first argument is emitted last. */
extern int sif_cmd_buffer[];

int sceSifSetCmdBuffer(int a0, int a1) {
    int old = sif_cmd_buffer[5];

    sif_cmd_buffer[5] = a0;
    sif_cmd_buffer[6] = a1;
    return old;
}
