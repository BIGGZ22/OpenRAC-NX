#include "common.h"
#include "structs.h"

extern int func_00127040(void *);

typedef struct {
    unsigned char sts;
    unsigned char std;
    unsigned char mfd;
    unsigned char rcyc;
    unsigned short express;
    unsigned short notify;
    unsigned short sqwc;
    unsigned short tqwc;
    void *rbadr;
    unsigned int rbmsk;
} sceDmaEnv;

extern unsigned char r1_D_00153520[];

extern unsigned char r1_D_00153530[];

extern unsigned char r1_D_00153540[];

extern sceDmaEnv r1_D_00159AD8;

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern int func_001272A8(void *);

extern void r1_func_00118C90(int);

extern int r1_D_00132EAC;

extern char r1_D_00159B00[];

extern char r1_D_0015B0C0[];

extern int r1_func_0011B4C8();

extern int r1_func_00118CC0(int);

extern char r1_D_00159BB0[];

extern void *r1_func_00116B00(void *, const void *, int);

extern int r1_D_00132EA8;

extern int func_00127508(int, int, int, int);

/* rac1-decomp src/core/00123168.c: func_001233E8 */
/* sceDmaPutEnv. Adapted from Lombyte (GPL-3.0) for PAL. */
int func_00127040(void *arg0) {
    sceDmaEnv *env = arg0;
    unsigned int ctrl;
    unsigned int pcr;
    unsigned int sqwc;
    unsigned int rbor;
    unsigned int rbsr;

    ctrl = *(volatile unsigned int *)0x1000E000;
    pcr = *(volatile unsigned int *)0x1000E020;
    sqwc = *(volatile unsigned int *)0x1000E030;
    rbor = *(volatile unsigned int *)0x1000E050;
    rbsr = *(volatile unsigned int *)0x1000E040;
    if (env->sts >= 10) {
        return -1;
    }
    if (env->std >= 10) {
        return -2;
    }
    if (env->mfd >= 10) {
        return -3;
    }
    if (env->rcyc >= 7) {
        return -4;
    }
    ctrl = (ctrl & ~0x30) | (r1_D_00153520[env->sts] << 4);
    ctrl = (ctrl & ~0xC0) | (r1_D_00153530[env->std] << 6);
    ctrl = (ctrl & ~0xC) | (r1_D_00153540[env->mfd] << 2);
    if (env->rcyc != 0) {
        ctrl |= 2;
        ctrl = (ctrl & ~0x300) | ((env->rcyc - 1) << 8);
    } else {
        ctrl &= ~2;
    }
    pcr = (env->express << 16) | env->notify;
    sqwc = (env->tqwc << 16) | env->sqwc;
    rbor = (unsigned int)env->rbadr;
    rbsr = env->rbmsk;
    *(volatile unsigned int *)0x1000E000 = ctrl;
    *(volatile unsigned int *)0x1000E020 = pcr;
    *(volatile unsigned int *)0x1000E030 = sqwc;
    *(volatile unsigned int *)0x1000E050 = rbor;
    *(volatile unsigned int *)0x1000E040 = rbsr;
    r1_D_00159AD8 = *env;
    return 0;
}

INCLUDE_ASM("asm/core", func_00127218);

INCLUDE_ASM("asm/core", func_00127288);

/* rac1-decomp src/core/00123168.c: func_00123650 */
/*
 * sceDmaPause: DI (r1_func_0011D960), force D_ENABLEW.CPND (0x10000) if
 * D_ENABLER has it clear, read D_CTRL (a volatile read for its side
 * effect only), clear the channel's CHCR.STR (bit 8), restore
 * D_ENABLEW, EI (r1_func_0011D9A8) if DI said interrupts were on, and
 * return the old CHCR.
 *
 * Exact under 2.9-ee. The mask has to be an unsigned constant
 * (`old & 0xFFFFFEFF`; `~0x100` is one addiu, SIZE 148), and the channel
 * word goes through a `volatile unsigned int *`, which keeps its load
 * after the D_CTRL read and gives retail's $4-$7 allocation. Under
 * 2.95.3 the same C differs in the prologue order only (8/152).
 */
int func_001272A8(void *arg0) {
    volatile unsigned int *d = arg0;
    int en;
    unsigned int prev, old;
    en = r1_func_0011D960();
    prev = *(volatile unsigned int *)0x1000F520;
    if ((prev & 0x10000) == 0)
        *(volatile unsigned int *)0x1000F590 = prev | 0x10000;
    *(volatile unsigned int *)0x1000E000;
    old = *d;
    *d = old & 0xFFFFFEFF;
    *(volatile unsigned int *)0x1000F590 = prev;
    if (en != 0) r1_func_0011D9A8();
    return old;
}

INCLUDE_ASM("asm/core", func_00127340);

INCLUDE_ASM("asm/core", func_00127500);

/* rac1-decomp src/core/001236F0.c: func_001238B0 */
/* sceMcOpen(port, slot, fname, mode) (libmc): same RPC-wrapper shape as
   sceMcDelete (func_00128068) but with the flags/mode argument stored to
   the descriptor's +0x8 word instead of a hardcoded 0, and RPC command 2. */
int func_00127508(int port, int slot, int fname_, int mode) {
    char *cd = r1_D_00159B00;
    char *fname = (char *)fname_;
    char *fp;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    if (fname == 0 || *fname == 0) {
        r1_func_00118C90(r1_D_00132EAC);
        return -210;
    }
    r1_func_00116B00(r1_D_00159BB0 + 0x14, fname, 0x3FF);
    fp = r1_D_00159BB0;
    *(int *)(fp + 0x0) = port;
    fp[0x413] = 0;
    *(int *)(fp + 0x8) = mode;
    *(int *)(fp + 0x4) = slot;
    r = r1_func_0011B4C8(cd, 2, 1, fp, 0x414, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 2;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}
