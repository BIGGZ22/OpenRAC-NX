#include "include_asm.h"

extern float D_L02_001B1030 __attribute__((sda));

extern int D_L02_001AD410;

extern char D_L02_001B11D0[8] __attribute__((sda));

extern char D_L02_001B11E0[8] __attribute__((sda));

extern char D_L02_001B11F0[8] __attribute__((sda));

extern char D_L02_001B1200[8] __attribute__((sda));

extern int D_L02_001B1210 __attribute__((sda));

extern void func_00282C88(void *);

extern void func_00335438(char *, char *, int, int, int);

INCLUDE_ASM("asm/levels", func_L01_0042A6F0);

void func_L02_003F56C0(void) {
    int v0 = D_L02_001AD410;
    float *p = (float *)((char *)v0 + 0x74);
    float f;

    f = p[0] + D_L02_001B1030 * 1.6666667535901069641113281250e-02f;
    p[0] = f;
    if (f > 1.0f)
        p[0] = f - 1.0f;
}

void func_L02_003FD770(void) {
    char buf[16];
    func_00282C88(buf);
    func_00335438(D_L02_001B11F0, D_L02_001B1200, 1, 0, 0);
    func_L01_0042A6F0(2, D_L02_001B1210);
    func_00335370(D_L02_001B11D0);
    func_003353C8(D_L02_001B11E0);
}

extern char *D_L02_001B1190[] __attribute__((sda));

/* Whether the pair of table entries the object names still agree with its own four words. */
INCLUDE_ASM("asm/levels", func_L02_003FCBF0);
