/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). vblankHandler: same reads, writes and calls in the same order. Retail ends with sync + ei (re-enable interrupts) before returning 0; those two have no effect on the host and are left out. */
extern int func_001232E0(int);
extern int func_00120858(int, int);
extern void func_001235C8(int, int);
extern int *func_0023E6A8(char *);
extern volatile int D_001612E0;
extern volatile int D_001612E4;
extern volatile int D_001612E8;
extern volatile int D_001612EC;
extern volatile int D_001612F0;
extern char *D_0016130C;

int func_0023C7A8(void) {
    int h = func_001232E0(2);
    int *buf;

    D_001612EC = (int)((*(volatile unsigned long long *)0x12001000 >> 13) & 1);
    if (D_001612E0 == 0)
        return 0;
    D_001612E4 = D_001612E4 + 1;
    D_001612F0 = func_00120858(1, 0);
    if (D_001612F0 != 0)
        return 0;
    buf = func_0023E6A8(D_0016130C + 0xD9168);
    if (buf == 0)
        return 0;
    if (D_001612EC == 0) {
        if (buf[0] == 2) {
            func_00120858(0, 0);
            func_001235C8(h, (int)buf + 0x40);
            buf[0] = 1;
            return 0;
        }
    }
    if (D_001612EC == 1) {
        int st = buf[0];
        if (st == 1) {
            func_00120858(0, 0);
            func_001235C8(h, (int)buf + 0x13880);
            buf[0] = 0;
            D_001612E8 = st;
        }
    }
    return 0;
}
