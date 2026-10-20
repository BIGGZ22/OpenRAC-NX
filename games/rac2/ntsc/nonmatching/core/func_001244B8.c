/* from src/core/001232F0.c: WaitGsPathsIdle, the SDK's GIF/VIF1/VU0 path-idle spin
   (symbol_addrs/eu: "polls D_xxxxx9000/A000/3C00/3020 MMIO + cfc2 vc13"). Retail reads
   VU0 control register 29 with `cfc2`, a COP2 instruction this cc1 never emits from C,
   so the 136 bytes cannot match; the polling chain around it is a do/while over four
   reads. The body below is the behaviour (for the port), not a candidate. */
int func_001244B8(void) {
    int flags;

    do {
        flags = 0;
        if (*(volatile unsigned int *)0x10009000 & 0x100)
            flags |= 1;
        if (*(volatile unsigned int *)0x1000A000 & 0x100)
            flags |= 2;
        if (*(volatile unsigned int *)0x10003C00 & 0x3)
            flags |= 4;
        if (/* cfc2 $a0, $29 */ 0 & 0x100)
            flags |= 8;
        if (*(volatile unsigned int *)0x10003020 & 0xC00)
            flags |= 0x10;
    } while (flags != 0);
    return flags;
}
