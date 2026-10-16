/* Carried from the PAL decompilation's nonmatching/functional/func_0012A7E8.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern void func_001190F8(int);
extern void func_001187D0(int, int);
extern int func_0011D660(void);
extern void func_0011D6A8(void);
extern void func_00128420();
extern void func_0012BB38();
void func_0012A1B0(void *arg0, unsigned int arg1, int arg2);
/* functional: medium. libmpeg DMA of n*24 bytes (n = a1[3]*a1[4]) from the address in a1[0] to the IPU/SPR channel, split in two when over 0xFFFF; hardware polls kept as volatile reads. */
extern void *func_001187C0(int, void *, int, void *);
extern void func_00119160(int);
extern void func_0012A418(void *, int, int);
extern void func_0012A5D8();



void _csc_storeRefImage(void *arg0, int *arg1) {
    char *p = (char *)arg0;
    int ev[8];
    int loc[4];
    int n, small, r;
    unsigned int mask = 0x0FFFFFFF;
    void *h;

    ev[0] = 2;
    n = arg1[3] * arg1[4];
    func_0012BB38(*(int *)(p + 0x858), ev);
    if ((*(volatile unsigned int *)0x10002010 & 0x4000) != 0) {
        *(volatile unsigned int *)0x10002010 = 0x40000000;
    }
    small = n < 0x400;
    while (*(volatile int *)0x10002010 < 0) {
    }
    func_00128420(arg0, 0);
    loc[0] = arg1[0];
    while (*(volatile int *)0x10002010 < 0) {
    }
    loc[1] = loc[0] & mask;
    loc[0] = n * 0x18;
    if ((unsigned int)loc[0] > 0xFFFF) {
        h = func_001187C0(4, (void *)func_0012A5D8, 0, &loc[0]);
        *(volatile unsigned int *)0x1000E010 = 0x10;
        func_00119160(4);
        r = func_0011D660();
        *(volatile unsigned int *)0x1000B410 = loc[1];
        *(volatile unsigned int *)0x1000B420 = 0xFFFF;
        *(volatile unsigned int *)0x1000B400 = 0x101;
        if (r != 0) {
            func_0011D6A8();
        }
        loc[1] = (loc[1] + 0xFFFF0) & mask;
        loc[0] = loc[0] - 0xFFFF;
        if (small) {
            func_0012A1B0(arg0, *(int *)(p + 0xD8), n);
        } else {
            func_0012A418(arg0, *(int *)(p + 0xD8), n);
        }
        func_001190F8(4);
        func_001187D0(4, (int)h);
    } else {
        r = func_0011D660();
        *(volatile unsigned int *)0x1000B410 = arg1[0] & mask;
        *(volatile unsigned int *)0x1000B420 = loc[0];
        *(volatile unsigned int *)0x1000B400 = 0x101;
        if (r != 0) {
            func_0011D6A8();
        }
        loc[0] = 0;
        if (small) {
            func_0012A1B0(arg0, *(int *)(p + 0xD8), n);
        } else {
            func_0012A418(arg0, *(int *)(p + 0xD8), n);
        }
    }
    ev[0] = 3;
    func_0012BB38(*(int *)(p + 0x858), ev);
}
