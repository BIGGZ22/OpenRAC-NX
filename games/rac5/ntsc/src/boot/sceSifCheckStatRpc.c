/* sceSifCheckStatRpc 0x01ED47B0..0x01ED47F0 (64 bytes, body 52 + 4 zero fill).
 *   lw    $a1, 0x0($a0)
 *   beqz  $a1, 0x01ED47DC
 *   nop                              (delay slot)
 *   lw    $v1, 0x4($a0)
 *   lw    $v0, 0x18($a1)
 *   bne   $v1, $v0, 0x01ED47DC
 *   nop                              (delay slot)
 *   lw    $v0, 0x10($a1)
 *   andi  $v0, $v0, 0x1
 *   bnez  $v0, 0x01ED47E4
 *   nop                              (delay slot)
 * 0x01ED47DC:
 *   jr    $ra
 *   daddu $v0, $zero, $zero          (delay slot)
 * 0x01ED47E4:
 *   jr    $ra
 *   addiu $v0, $zero, 0x1            (delay slot)
 *   nop
 * True when the RPC descriptor is present, its sequence word matches the
 * caller's and the busy bit is set. */
int sceSifCheckStatRpc(int *rpc) {
    int *sd = (int *)rpc[0];

    if (!(sd != 0 && rpc[1] == sd[6] && (sd[4] & 1))) return 0;
    return 1;
}
