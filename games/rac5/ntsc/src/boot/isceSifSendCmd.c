/* isceSifSendCmd 0x01ED3B50..0x01ED3CD8 (interval 392 bytes; the function is
 * 0x01ED3B50..0x01ED3B8C, 60 bytes + 4 zero fill; the interval continues into
 * the unnamed func_1ED3B90 at 0x01ED3B90).
 *
 *   daddu $v0, $a2, $zero      a2 -> v0
 *   daddu $v1, $a3, $zero      a3 -> v1
 *   daddu $t3, $t0, $zero      t0 -> t3
 *   addiu $sp, $sp, -0x10
 *   daddu $t2, $t1, $zero      t1 -> t2
 *   daddu $a2, $a1, $zero      a1 -> a2 (arg2 of the worker)
 *   sd    $ra, 0x0($sp)
 *   daddu $a3, $v0, $zero
 *   daddu $t0, $v1, $zero
 *   daddu $t1, $t3, $zero
 *   jal   func_1ED39D8         the 7-argument packet worker
 *   addiu $a1, $zero, 0x1      mode = 1 (delay slot)
 *   ld    $ra, 0x0($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x10       (delay slot)
 *   nop
 * Interrupt-safe variant of sceSifSendCmd: identical but for the worker's
 * second argument, which is 1 here and 0 in sceSifSendCmd.
 * Source: lombyte-depot src/sdk/rpc/isce_sif_send_cmd.c. */
extern int func_1ED39D8(int, int, int, int, int, int, int);

int isceSifSendCmd(int packet, int command, int source, int source_size,
                   int destination, int destination_size)
{
    return func_1ED39D8(packet, 1, command, source, source_size,
                        destination, destination_size);
}
