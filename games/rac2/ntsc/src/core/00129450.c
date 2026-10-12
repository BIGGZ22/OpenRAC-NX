#include "common.h"
#include "structs.h"

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

typedef struct {
    unsigned char pad0[0x838];
    int unk838;
    int unk83C;
    unsigned char pad840[0x18];
    void *cbData;
} MpegDec;

extern char r1_D_00153888[];

extern void func_0012C3B0(void *);

extern void r1_func_0012BC78(int, void *);

extern void r1_func_0012C468(void *, void *);

#define IPU_CTRL   ((volatile unsigned int *)0x10002010)

#define IPU_BP     ((volatile unsigned int *)0x10002020)

#define IPU_TOP    ((unsigned long *)0x10002030)

#define D3_CHCR    ((volatile unsigned int *)0x1000B400)

#define D3_QWC     ((volatile unsigned int *)0x1000B420)

#define D4_CHCR    ((volatile unsigned int *)0x1000B000)

#define D4_QWC     ((volatile unsigned int *)0x1000B020)

#define D_ENABLER  ((volatile unsigned int *)0x1000F520)

#define D_ENABLEW  ((volatile unsigned int *)0x1000F590)

extern int func_0012B1C0(void *);

/* func_0012C508 returns a short: its epilogue sign-extends $v0. */
extern short func_0012C508(int, int);

INCLUDE_ASM("asm/core", func_00129450);

INCLUDE_ASM("asm/core", func_001296A0);

INCLUDE_ASM("asm/core", func_00129DA8);

INCLUDE_ASM("asm/core", func_0012A1C8);

INCLUDE_ASM("asm/core", func_0012A3E8);

INCLUDE_ASM("asm/core", func_0012A460);

INCLUDE_ASM("asm/core", func_0012A4F8);

INCLUDE_ASM("asm/core", func_0012A5B0);

INCLUDE_ASM("asm/core", func_0012A680);

INCLUDE_ASM("asm/core", func_0012A730);

INCLUDE_ASM("asm/core", func_0012A7E8);

INCLUDE_ASM("asm/core", func_0012A8E0);

INCLUDE_ASM("asm/core", func_0012A9E0);

INCLUDE_ASM("asm/core", func_0012AA80);

INCLUDE_ASM("asm/core", func_0012AB30);

INCLUDE_ASM("asm/core", func_0012AC10);

INCLUDE_ASM("asm/core", func_0012ACF8);

INCLUDE_ASM("asm/core", func_0012ADD0);

INCLUDE_ASM("asm/core", func_0012AEA0);

INCLUDE_ASM("asm/core", func_0012AFC0);

INCLUDE_ASM("asm/core", func_0012B0D8);

/* rac1-decomp src/core/00125630.c: func_00127378 */
void func_0012B198(int arg0) {
    unsigned int *p = (unsigned int *)0x10002010;
    *p = (*p & 0xFF7FFFFF) | (arg0 << 23);
}

/* rac1-decomp src/core/00125630.c: func_001273A0 */
/* _waitBdecOut (libmpeg). Adapted from Lombyte (MIT) for PAL. */
int func_0012B1C0(void *arg0) {
    MpegDec *d = arg0;
    int cb1[8];
    int cb2[8];
    int ret;
    int intr;
    unsigned int bp;
    long top;
    unsigned int rest;

    ret = 1;
    func_0012C3B0(d);
    if (*D4_QWC != 0 && !(*IPU_CTRL & 0x4000)) {
        do {
            if (*D3_QWC == 0 && !(*D3_CHCR & 0x100)) {
                cb1[0] = 1;
                r1_func_0012BC78((int)d->cbData, cb1);
            }
        } while (*D4_QWC != 0 && !(*IPU_CTRL & 0x4000));
    }
    bp = *IPU_BP;
    top = *IPU_TOP;
    d->unk838 = top;
    if (top < 0) {
        rest = bp & 0x1F;
        d->unk83C = rest ? 32 - rest : 0;
    } else {
        d->unk83C = 32;
    }
    if (*IPU_CTRL & 0x4000) {
        r1_func_0012C468(d, r1_D_00153888);
        cb2[0] = 2;
        r1_func_0012BC78((int)d->cbData, cb2);
        *(unsigned int *)0x10002010 = 0x40000000;
        cb2[0] = 3;
        r1_func_0012BC78((int)d->cbData, cb2);
        intr = r1_func_0011D960();
        *D_ENABLEW = *D_ENABLER | 0x10000;
        *D4_CHCR = 0;
        *D_ENABLEW = *D_ENABLER & ~0x10000;
        if (intr) {
            r1_func_0011D9A8();
        }
        ret = 0;
        *D4_QWC = 0;
    }
    return ret;
}

/* rac1-decomp src/core/00125630.c: func_001275A0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee turns a void one-call function into a bare tail jump (8 bytes
   against 28). */
short func_0012B3C0(int arg0) {
    return func_0012C508(arg0, 3);
}
