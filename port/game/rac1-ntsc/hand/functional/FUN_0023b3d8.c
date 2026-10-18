/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_0023C7A8.c (games/rac1): func_0023C7A8, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. vblankHandler: same reads, writes and calls in the same order. Retail ends with sync + ei (re-enable interrupts) before returning 0; those two have no effect on the host and are left out. */

extern int func_00122FE0(int);
extern int func_00120558(int, int);
extern void func_001232C8(int, int);
extern int *func_0023D2D8(char *);
extern volatile int D_001611E0;
extern volatile int D_001611E4;
extern volatile int D_001611E8;
extern volatile int D_001611EC;
extern volatile int D_001611F0;
extern char *D_0016120C;
int FUN_0023b3d8(void) {
    int h = func_00122FE0(2);
    int *buf;
    D_001611EC = (int)((*(volatile unsigned long long *)0x12001000 >> 13) & 1);
    if (D_001611E0 == 0)
        return 0;
    D_001611E4 = D_001611E4 + 1;
    D_001611F0 = func_00120558(1, 0);
    if (D_001611F0 != 0)
        return 0;
    buf = func_0023D2D8(D_0016120C + 0xD9168);
    if (buf == 0)
        return 0;
    if (D_001611EC == 0) {
        if (buf[0] == 2) {
            func_00120558(0, 0);
            func_001232C8(h, (int)buf + 0x40);
            buf[0] = 1;
            return 0;
        }
    }
    if (D_001611EC == 1) {
        int st = buf[0];
        if (st == 1) {
            func_00120558(0, 0);
            func_001232C8(h, (int)buf + 0x13880);
            buf[0] = 0;
            D_001611E8 = st;
        }
    }
    return 0;
}
