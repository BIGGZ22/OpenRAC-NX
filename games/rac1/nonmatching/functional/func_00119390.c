/* functional: high. fcheck: unchecked. Calls the kernel syscall stub func_00118AE0 with interrupts disabled: if interrupts were enabled (Status.EIE) it disables them with func_0011D960 (DI) first and re-enables them with func_0011D9A8 (EI) afterwards; returns the stub's result. Retail tests EIE with mfc0 before calling DI; calling DI unconditionally and keeping its result is the same, since DI returns that bit. The sync after the call has no host meaning. Written from the asm (COP0 access is not emulated). */
extern int func_0011D960(void);
extern int func_0011D9A8(void);
extern int func_00118AE0(int);

int func_00119390(int arg) {
    int was_on = func_0011D960();
    int ret = func_00118AE0(arg);
    if (was_on) {
        func_0011D9A8();
    }
    return ret;
}
