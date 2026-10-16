/* Carried from the PAL decompilation's src/core/0012A2F0.c (matched C there) (games/rac1) through the version
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
extern void func_0012C468_a(void *, void *) __asm__("func_0012C328");
extern void func_0012BB38();
extern int func_00118AC0_2A558(int, void *, int, void *) __asm__("func_001187C0");
extern void func_00119460_2A558(int) __asm__("func_00119160");
extern void func_0012A418_2A558(void) __asm__("func_0012A2D8");
extern char D_001539B0[];
typedef struct {
    volatile int done; /* 0x00: chunks finished (callback counts) */
    int err;           /* 0x04 */
    int size;          /* 0x08 */
    int next;          /* 0x0C */
    int total;         /* 0x10 */
} CscRec_2A558_cdocsc2;



/* libmpeg chunked DMA copy: arm the channel-3 callback with a progress record, start the transfer, wait for all chunks. */
void _doCSC2(void *arg0, unsigned int arg1, int arg2) {
    char *p = (char *)arg0;
    int event[8];
    CscRec_2A558_cdocsc2 rec;
    CscRec_2A558_cdocsc2 *rp;
    int h;
    int r;

    rec.total = arg2 / 0x3FF + 1;
    rec.size = arg2;
    rec.next = (arg1 + 0xFFC00) & 0x0FFFFFFF;
    rec.err = 0;
    rec.done = 0;
    rp = &rec;
    while (*(volatile int *)0x10002010 < 0) {
    }
    h = func_00118AC0_2A558(3, (void *)func_0012A418_2A558, 0, rp);
    *(volatile unsigned int *)0x1000E010 = 8;
    func_00119460_2A558(3);
    r = func_0011D660();
    {
        volatile unsigned int *madr = (volatile unsigned int *)0x1000B010;
        volatile unsigned int *qwc = (volatile unsigned int *)0x1000B020;
        volatile unsigned int *chcr = (volatile unsigned int *)0x1000B000;
        *madr = arg1 & 0x0FFFFFFF;
        *qwc = 0xFFC0;
        *chcr = 0x100;
    }
    if (r != 0) {
        func_0011D6A8();
    }
    *(volatile unsigned int *)0x10002000 = 0x700003FF;
    event[0] = 4;
    func_0012BB38(*(void **)(p + 0x858), event);
    while (rec.done < rec.total) {
    }
    if (rec.err != 0) {
        func_0012C468_a(p, D_001539B0);
    }
    while (*(volatile int *)0x10002010 < 0) {
    }
    func_001190F8(3);
    func_001187D0(3, h);
}
