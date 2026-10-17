/* functional: medium. Handwritten (ASM_FUNC); the cache-range helper. Retail reads COP0 Status and calls DIntr only when
   bit 0x10000 was set, then EIntr after the range op under the same test. The host has no COP0, so the test is taken as
   set (both calls always made); DIntr/EIntr are the port's host versions, so that is a no-op there. The two arguments are
   rounded down to 64-byte cache lines before the range op, as retail does. */
extern int func_0011D960(void);  /* DIntr: returns the previous state; retail ignores it here */
extern void func_0011D9A8(void); /* EIntr */
extern void func_001191E0(unsigned int addr, unsigned int size); /* range cache op, both masked to 64 bytes */

void func_00119288(void *a, void *b) {
    unsigned int addr = (unsigned int)a & 0xFFFFFFC0u;
    unsigned int size = (unsigned int)b & 0xFFFFFFC0u;
    int enabled = 1;

    if (enabled) {
        func_0011D960();
    }
    func_001191E0(addr, size);
    if (enabled) {
        func_0011D9A8();
    }
}
