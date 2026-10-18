/* sceSifExitCmd 0x01ED38C0..0x01ED38F8 (56 bytes, body 52 + 4 zero fill).
 *   addiu $sp, $sp, -0x10
 *   sd    $ra, 0x0($sp)
 *   jal   DisableDmac
 *   addiu $a0, $zero, 0x5           (delay slot)
 *   lui   $v1, 0x1FE
 *   addiu $a0, $zero, 0x5
 *   jal   RemoveDmacHandler
 *   lw    $a1, 0x3E94($v1)          (delay slot)
 *   lui   $v1, 0x1F0
 *   ld    $ra, 0x0($sp)
 *   sw    $zero, -0x10B0($v1)
 *   jr    $ra
 *   addiu $sp, $sp, 0x10            (delay slot)
 *   nop
 * Tears down the SIF DMA command handler (channel 5) and clears the DMA-busy
 * word; the return value is whatever RemoveDmacHandler left in $v0. */
extern int DisableDmac(int channel);
extern int RemoveDmacHandler(int channel, int handler);
extern int dma_handler_slot;
extern int sif_dma_busy;

int sceSifExitCmd(void) {
    DisableDmac(5);
    RemoveDmacHandler(5, dma_handler_slot);
    sif_dma_busy = 0;
}
