/* functional: high. fcheck: unchecked. DMA controller reset: sets bit 9 of D_PCR (0x1000E020), writes 1 (DMA enable) to D_CTRL (0x1000E000), and zeroes the quadword count (Dn_QWC, +0x20) of channels VIF0, VIF1, GIF, fromIPU, toIPU, fromSPR and toSPR. Differential test: 3 starting D_PCR values agree with retail run in an EE emulator, I/O pages compared. */
void func_0020C268(void) {
    *(volatile unsigned int *)0x1000E020 |= 0x200;
    *(volatile unsigned int *)0x1000E000 = 1;
    *(volatile unsigned int *)0x10008020 = 0;
    *(volatile unsigned int *)0x10009020 = 0;
    *(volatile unsigned int *)0x1000A020 = 0;
    *(volatile unsigned int *)0x1000B020 = 0;
    *(volatile unsigned int *)0x1000B420 = 0;
    *(volatile unsigned int *)0x1000D020 = 0;
    *(volatile unsigned int *)0x1000D420 = 0;
}
