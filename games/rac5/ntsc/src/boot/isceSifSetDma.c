/* isceSifSetDma 0x01ED3608..0x01ED3640 (56 bytes).
 *   lui   $v0, 0x1F0
 *   addiu $sp, $sp, -0x10
 *   lw    $v1, -0x10B0($v0)
 *   beqz  $v1, 0x01ED362C
 *   sd    $ra, 0x0($sp)              (delay slot)
 *   jal   func_1ED34B8
 *   addiu $a2, $zero, 0x1            (delay slot)
 *   b     0x01ED3638
 *   ld    $ra, 0x0($sp)              (delay slot)
 * 0x01ED362C:
 *   jal   _isceSifSetDma
 *   nop                              (delay slot)
 *   ld    $ra, 0x0($sp)
 * 0x01ED3638:
 *   jr    $ra
 *   addiu $sp, $sp, 0x10             (delay slot)
 * Interrupt-safe DMA submit: goes through the packet builder while DMA is
 * busy, otherwise straight to the raw submission. */
extern int sif_dma_busy;
extern int func_1ED34B8(int, int, int);
extern int _isceSifSetDma(int, int);

int isceSifSetDma(int addr, int size) {
    int r;

    if (sif_dma_busy) r = func_1ED34B8(addr, size, 1);
    else r = _isceSifSetDma(addr, size);
    return r;
}
