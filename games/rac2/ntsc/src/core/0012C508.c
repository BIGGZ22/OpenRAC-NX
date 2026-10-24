#include "common.h"
#include "structs.h"

#define IPU_CTRL   ((volatile unsigned int *)0x10002010)

#define IPU_BP     ((volatile unsigned int *)0x10002020)

#define IPU_TOP    ((unsigned long *)0x10002030)

/* func_0012C508 returns a short: its epilogue sign-extends $v0. */
extern short func_0012C508(int, int);

extern int r1_D_00132F70[];

extern void r1_func_0012BCC8(int);

typedef struct {
    unsigned char pad0[0x11C];
    int unk11C;
    unsigned char pad120[0x6F8];
    int unk818;
    unsigned char pad81C[0x1C];
    int unk838;
    int unk83C;
    unsigned char pad840[0x18];
    void *cbData;
} MpegVdec;

#define IPU_CMD  ((volatile unsigned long *)0x10002000)

extern int func_0012C680(void *, int);

extern void func_0012C788(void *, int);

extern int func_0012C878(void *, int);

/* rac1-decomp src/core/00125630.c: func_001286E8 */
/* _ipuVdec (libmpeg). Adapted from Lombyte (GPL-3.0) for PAL. */
short func_0012C508(int arg0, int tbl) {
    MpegVdec *d = (MpegVdec *)arg0;
    int i;
    int j;
    unsigned int cmd;
    long r;
    long top;
    unsigned int bp;

    i = 0;
    j = 0;
    while ((*IPU_CTRL & 0x80004000) == 0x80000000) {
        if (j++ > 5000) {
            r1_func_0012BCC8((int)d->cbData);
            j = 0;
        }
    }
    cmd = (tbl << 26) | 0x30000000;
    *(volatile unsigned int *)IPU_CMD = cmd;
    d->unk818 = r1_D_00132F70[(int)cmd >> 28];
    r = *IPU_CMD;
    while (r < 0) {
        if (i++ > 5000) {
            r1_func_0012BCC8((int)d->cbData);
            i = 0;
        }
        r = *IPU_CMD;
    }
    bp = *IPU_BP;
    top = *IPU_TOP;
    d->unk838 = top;
    if (top < 0) {
        d->unk83C = -(bp & 0x1F) & 0x1F;
    } else {
        d->unk83C = 32;
    }
    d->unk11C = (int)r == 0;
    return (short)r;
}
extern int func_0012C680(void *arg, int count);
INCLUDE_ASM("asm/core", func_0012C680);

extern void func_0012C788(void *arg0, int arg1);
extern long r1_func_00128638();




/* rac1-decomp src/core/00125630.c: func_00128968 */
/*
 * IPU command send: r1_func_00128590's wait (IPU_CTRL & 0x80004000 ==
 * 0x80000000, a r1_func_0012BCC8 kick every 5001 spins) inlined, then
 * cmd = arg1 | 0x40000000 to IPU_CMD, s+0x818 = r1_D_00132F70[cmd >> 28]
 * (r1_func_00128560 inlined), s+0x838 = the narrowed r1_func_00128638 result,
 * s+0x83C = 0x20.
 *
 * Written plainly it is exact under 2.9-ee, which rematerialises the
 * MMIO constants inside the loop and keeps only %hi(r1_D_00132F70) in $18,
 * like retail; 2.95.3 hoists them into saved registers (SIZE 252/240).
 * The stores go in source order 0x838 then 0x83C.
 */
void func_0012C788(void *arg0, int arg1) {
    int counter = 0;
    unsigned int cmd;

    while ((*(volatile int *)0x10002010 & 0x80004000) == 0x80000000) {
        if (counter++ >= 0x1389) {
            r1_func_0012BCC8(*(int *)((char *)arg0 + 0x858));
            counter = 0;
        }
    }
    cmd = arg1 | 0x40000000;
    *(volatile int *)0x10002000 = cmd;
    *(int *)((char *)arg0 + 0x818) = r1_D_00132F70[cmd >> 28];
    *(int *)((char *)arg0 + 0x838) = r1_func_00128638(arg0);
    *(int *)((char *)arg0 + 0x83C) = 0x20;
}

extern int func_0012C878(void *arg0, int n);




/* rac1-decomp src/core/00125630.c: func_00128A58 */
/* _nextBit (libmpeg.a:mpc.o), Get_Bits: waits for the IPU, refills the
 * bit cache when it is empty or short, returns the top n bits and tells
 * the IPU to consume them. The r1_D_00132F70 index needs its own local,
 * computed before the command store, or the allocator swaps $v1 and $a0.
 */
int func_0012C878(void *arg0, int n) {
    char *s = (char *)arg0;
    int counter = 0;
    unsigned int cmd;
    unsigned int idx;
    unsigned int oldBits;
    unsigned int result;

    while ((*(volatile int *)0x10002010 & 0x80004000) == 0x80000000) {
        if (counter++ >= 0x1389) {
            r1_func_0012BCC8(*(int *)(s + 0x858));
            counter = 0;
        }
    }

    if (*(int *)(s + 0x818) != 0 || *(int *)(s + 0x83C) < n) {
        *(volatile int *)0x10002000 = 0x40000000;
        *(int *)(s + 0x818) = r1_D_00132F70[4];
        *(int *)(s + 0x838) = r1_func_00128638(s);
    }

    *(int *)(s + 0x83C) = 0x20;
    oldBits = *(unsigned int *)(s + 0x838);
    cmd = n | 0x40000000;
    idx = cmd >> 28;
    *(volatile unsigned int *)0x10002000 = cmd;
    result = oldBits >> (0x20 - n);
    *(int *)(s + 0x818) = r1_D_00132F70[idx];
    *(int *)(s + 0x838) = r1_func_00128638(s);
    return result;
}

