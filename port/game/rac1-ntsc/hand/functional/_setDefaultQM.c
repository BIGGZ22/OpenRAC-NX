/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), src/core/0012AC80.c (games/rac1): func_0012C990, matched C there.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */

extern int func_0011D660(void);
extern void func_0011D6A8(void);
extern void func_00128450(void *);

void *func_0012BB38(void *arg0, int *arg1);
extern void func_00128420(void *, unsigned int);
extern void _setDefaultQM(void *, int, int);
void _setDefaultQM(void *arg, int cmd, int addr) {
    char *mp = arg;
    int cb[8];
    int intr;
    cb[0] = 2;
    func_0012BB38(*(void **)(mp + 0x858), cb);
    func_00128450(mp);
    *(volatile int *)0x10002000 = 0;
    func_00128450(mp);
    intr = func_0011D660();
    *(volatile int *)0x1000B410 = addr & 0x0FFFFFFF;
    *(volatile int *)0x1000B420 = 4;
    *(volatile int *)0x1000B400 = 0x101;
    if (intr != 0) {
        func_0011D6A8();
    }
    func_00128420(mp, cmd);
    func_00128450(mp);
    cb[0] = 3;
    func_0012BB38(*(void **)(mp + 0x858), cb);
}
