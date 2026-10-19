/* functional: high. fcheck: unchecked. DI: disables interrupts and returns 1 if they were enabled (Status.EIE, bit 16 of COP0 Status), 0 if not. Retail repeats di/sync.p until EIE reads clear (the documented EE workaround for a DI lost to an incoming interrupt). The host has no EE interrupts: the port keeps the enable bit in ee_interrupts_enabled (defined here, shared with func_0011D9A8, EI), so a DI/EI pair still nests the way callers expect. Not run in the emulator (it reads COP0 Status); written from the asm. */
int ee_interrupts_enabled = 1;

int func_0011D960(void) {
    int was = ee_interrupts_enabled != 0;
    ee_interrupts_enabled = 0;
    return was;
}
