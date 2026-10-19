/* functional: high. fcheck: unchecked. EI: enables interrupts and returns 1 if they were already enabled (Status.EIE, bit 16 of COP0 Status, read before the ei), 0 if not. The port keeps the enable bit in ee_interrupts_enabled, defined with func_0011D960 (DI). Not run in the emulator (it reads COP0 Status); written from the asm. */
extern int ee_interrupts_enabled;

int func_0011D9A8(void) {
    int was = ee_interrupts_enabled != 0;
    ee_interrupts_enabled = 1;
    return was;
}
