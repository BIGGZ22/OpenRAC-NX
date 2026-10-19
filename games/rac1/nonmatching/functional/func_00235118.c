/* functional: high. fcheck: unchecked. VIF1 interrupt handler: reads VIF1_STAT (0x10009000); when VIF1_ERR-side word 0x10009030 is nonzero and the stall/interrupt bits (STAT & 0xC) are set, it clears bits of D_00160FE0: all five low bits when STAT bits 28-30 are all set (then returns without acknowledging), otherwise bit 1, 2, 3 or 4 for the VIF interrupt codes 0x201, 0x202, 0x204 and 0x208 in STAT bits 16-25; then it writes STAT | 0x100 back to 0x10009000 to resume. Returns 0. Retail ends with sync and ei (it runs as an interrupt handler); the port drops both. Differential test: 20 cases (10 STAT values x VIF1 word 0/1) agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model), I/O pages compared. */
extern unsigned int D_00160FE0;

int func_00235118(void) {
    unsigned int stat = *(volatile unsigned int *)0x10009000;
    unsigned int code = stat & 0x3FF0000;
    if (*(volatile unsigned int *)0x10009030 == 0 || (stat & 0xC) == 0) {
        return 0;
    }
    if ((stat & 0x70000000) == 0x70000000) {
        D_00160FE0 &= ~0x1Fu;
        return 0;
    }
    if (code & 0x2000000) {
        switch (code) {
        case 0x2010000:
            D_00160FE0 &= ~0x2u;
            break;
        case 0x2020000:
            D_00160FE0 &= ~0x4u;
            break;
        case 0x2040000:
            D_00160FE0 &= ~0x8u;
            break;
        case 0x2080000:
            D_00160FE0 &= ~0x10u;
            break;
        }
    }
    *(volatile unsigned int *)0x10009000 = stat | 0x100;
    return 0;
}
