/* functional: medium. A break-pointer allocator (sbrk shape): with interrupts disabled around the update, new = D_0012FCF4 + n; if the value the kernel call func_00118C60 returns (called with n still in a0, as retail does) is below new, set errno (*func_001138A8()) to 12 and return -1, otherwise store new and return the old break. DI/EI go through func_0011D960/func_0011D9A8; EI only if interrupts were on at entry. Meaning of the func_00118C60 syscall result (memory limit) is inferred, not known. */
extern unsigned int D_0012FCF4;
extern int func_0011D960(void);
extern int func_0011D9A8(void);
extern unsigned int func_00118C60(int n);
extern int *func_001138A8(void);

int func_00119110(int n) {
    int was_on = func_0011D960() != 0;
    unsigned int old = D_0012FCF4;
    unsigned int new_brk = old + n;
    unsigned int limit = func_00118C60(n);
    if (limit < new_brk) {
        *func_001138A8() = 12;
        if (was_on) {
            func_0011D9A8();
        }
        return -1;
    }
    D_0012FCF4 = new_brk;
    if (was_on) {
        func_0011D9A8();
    }
    return (int)old;
}
