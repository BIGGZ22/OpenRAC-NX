#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x123168-0x1236F0. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Two Sony libraries back to back, both built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE): libgraph's sceGsSyncVCallback (func_00123168,
 * the last of libgraph's modules, which end 8-byte aligned so no fill
 * separates them) and the start of libdma ("libdma: sync timeout";
 * sceDmaGetChan, sceDmaReset, sceDmaPutEnv, sceDmaPause, ...).
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);
extern int func_00114060(int, void *);
extern void func_00113AE0(void *);
extern void func_00117118(void *, void *, int, int);
extern int func_00119008();
extern int D_0012FCF0 NOT_SDA;
extern void func_00118E90(int arg0, void *arg1);
extern void *D_00154A40 NOT_SDA;
extern int D_00155080[];
extern void func_001193F8(int);
extern void func_00118AD0(int, int);
extern int D_00154F54;
extern int D_0012FD04;
extern int D_00154F64 NOT_SDA;
extern int D_00154F6C NOT_SDA;
extern void func_0011AA90(int, int, int, int, int, int, int);
extern void func_0011AA00(void);
extern int D_0012FD08 NOT_SDA;
extern int func_0011D960(void);
extern void func_0011D9A8(void);
extern int func_00118C70(void *);
extern int D_0012FDA0;
extern int D_0012FDA4;
extern char D_00157E80[];
extern int D_0012FD9C;
extern void func_0011BBF0(void);
extern int D_0012FD9C NOT_SDA;
extern char D_0012FCEC[];
extern char D_001580A8[];
extern int D_0012FDA8;
extern void func_001153FC(void *, int, int);
extern int D_0012FD94;
extern int D_0012FDAC;
extern char D_00158140[];
extern int D_00158180;
extern int D_001581C0;
extern char D_00158528[];
extern int D_0012FDB4;
extern int func_0011CE70(int arg0, int arg1, int arg2, void *arg3);
extern void func_00118EC0(void);
extern void func_0011D4E0(void);
extern void func_00118EB0(void);
extern int D_00130420;
extern int D_00130424;
extern int D_00130BD0[];
extern char D_00130428[];
extern int func_0011DC50(void);
extern void func_00118D80(int);
extern int func_00120F30(int);
extern void *D_00159840;
extern int D_001313E0;
extern int D_001313E8;
extern int D_001313EC;
extern int D_001313F0;
extern int D_001313E4;
extern int D_001313FC;
extern int func_0011B4C8();
extern int func_00120D28(int);
extern void func_00118C90(int);
extern char D_00132590[];
extern int D_00131440;
extern void func_0011A6C8();
extern int func_0011B6B8(void *);
extern char D_00153000[];
extern char D_00132E08[];
extern int D_001313D0;
extern int func_00121040(int);
extern int D_001325C0;
extern char D_00132E40[];

extern void *func_00121D08(void);
extern int func_00119328(int);
extern int func_00118AA0(int, int);
extern int func_00118A90(int, void *, int);
extern void func_00119390(int);

/*
 * libgraph's sceGsSyncVCallback: install (or, called with arg0 == 0,
 * uninstall) the V-sync handler at offset 8/0xC of D_00132E40
 * (func_00121D08's sceGsGetGParam -- the same global func_00122140 reads
 * the mode flag from), under DisableIntc/EnableIntc (func_00119328/
 * func_00119390) of INTC 2 (VBLANK start) around the AddIntcHandler/
 * RemoveIntcHandler syscalls (func_00118A90/func_00118AA0). Returns the
 * previous handler.
 *
 * Exact under 2.9-ee as written. Under 2.95.3 it was 8/0xA0: that
 * compiler set up a0 (lvl) before a1 (arg0) for the func_00118A90 call
 * whatever the spelling, where retail (and 2.9-ee) has a1 first.
 */
int func_00123168(void *arg0) {
    char *g = (char *)func_00121D08();
    int lvl = 2;
    int prev = *(int *)(g + 8);

    if (arg0 == 0) {
        func_00119328(lvl);
        func_00118AA0(lvl, *(int *)(g + 0xC));
        *(int *)(g + 0xC) = 0;
        *(int *)(g + 8) = 0;
    } else {
        if (prev != 0) {
            func_00119328(lvl);
            func_00118AA0(lvl, *(int *)(g + 0xC));
        }
        *(int *)(g + 8) = (int)arg0;
        *(int *)(g + 0xC) = func_00118A90(lvl, arg0, -1);
        func_00119390(lvl);
    }
    return prev;
}

extern int func_00123650(void *);
extern char D_001534E0[];

/*
 * Wait for a DMA channel: spin while CHCR.STR (bit 8 of *arg0) is set;
 * after 0x1000000 polls report through func_0011A6C8(D_001534E0,
 * "libdma: sync timeout") and sceDmaPause (func_00123650) the channel on
 * every further poll.
 *
 * The countdown starts at 0x1000000 and is decremented before the test:
 * retail's `lui 0x100; addiu -1` (splat's "D_FFFFFF") is that first
 * n-- folded into the constant, and reorg copies the loop's n-- into the
 * bnez slot. The old reading (0xFFFFFF, `if (n < 0)` then `n--`) gave
 * `lui 0xff; ori 0xffff` and was 24/112. The channel word is read
 * volatile, as the hardware register it is.
 */
void func_00123208(void *arg0) {
    int n = 0x1000000;

    while ((*(volatile int *)arg0 & 0x100) != 0) {
        if (--n < 0) {
            func_0011A6C8(D_001534E0);
            func_00123650(arg0);
        }
    }
}

LINKER_REMNANT("asm/remnants/core_text", func_00123278);

int func_00123280(int arg0) {
    if ((unsigned int)arg0 >> 28 == 7) {
        arg0 &= 0x0FFFFFFF;
        arg0 |= 0x80000000;
    }
    return arg0;
}

/* Zero-fill n bytes at dst.

   The plain post-decrement loop: the counter becomes a new pseudo (n - 1
   in $v0, -1 in $v1) instead of the parameter decremented in place, and
   the empty delay slot is 2.9-ee itself -- it pads the short loop with
   nops in its own output and leaves the bne unfilled, so the assembler
   puts the nop in the slot. That is the "unfilled short-loop slot" seen
   elsewhere in core_text. Under 2.95.3 the same C gets the registers
   right but reorg fills the slot (6/52); the old do-while was 2/52. */
void func_001232A8(char *dst, int n) {
    while (n-- != 0) {
        *dst++ = 0;
    }
}

extern int D_00132E70[];

/* sceDmaGetChan: the channel's register block, 0 for an id past the ten
   channels. The in-range return comes first under 2.9-ee (the
   `>= 0xA` guard-first form matched 2.95.3 and is 21/40 here). */
int func_001232E0(unsigned int arg0) {
    if (arg0 < 0xA) {
        return D_00132E70[arg0];
    }
    return 0;
}

/*
 * sceDmaReset(mode): ret = D_CTRL.DMAE; for each of the ten channels
 * whose flag (D_001534F8[i]) is set, zero chcr/tadr/madr/as1/as0/sadr of
 * the channel's registers (D_00132E70[i]); D_STAT = 0xFF1F, then
 * D_STAT &= 0xFF1F0000; zero a 0x14-byte sceDmaEnv (func_001232A8) and
 * sceDmaPutEnv it (func_001233E8); if mode == 1 set DMAE. Exact under
 * both compilers.
 *
 * Levers (build-sn/try/func_00123308/RESULT.md):
 *  - an index loop: loop.c strength-reduces it into the two pointers and
 *    the down-counter itself, in retail's init order (the old
 *    hand-written two-pointer loop swapped them);
 *  - the MMIO spelled inline as `*(volatile unsigned int *)0x1000E0x0`
 *    (a pointer local becomes absolute `$at` addressing: the old 228);
 *  - the two D_STAT stores are not volatile, only its read, so the second
 *    store can go into the jal slot as in retail (a volatile store never
 *    does);
 *  - sceDmaPutEnv returns int: the `li 1` after it is in $v1.
 */
extern int D_001534F8[];
extern int func_001233E8(void *);

int func_00123308(int arg0) {
    char buf[0x14];
    int flag;
    int i;

    flag = *(volatile unsigned int *)0x1000E000 & 1;

    for (i = 0; i < 10; i++) {
        if (D_001534F8[i] != 0) {
            int *p = (int *)D_00132E70[i];
            p[0] = 0;
            p[0xC] = 0;
            p[4] = 0;
            p[0x14] = 0;
            p[0x10] = 0;
            p[0x20] = 0;
        }
    }

    *(unsigned int *)0x1000E010 = 0xFF1F;
    *(unsigned int *)0x1000E010 = *(volatile unsigned int *)0x1000E010 & 0xFF1F0000;

    func_001232A8(buf, 0x14);
    func_001233E8(buf);

    if (arg0 == 1) {
        *(volatile unsigned int *)0x1000E000 = *(volatile unsigned int *)0x1000E000 | 1;
    }
    return flag;
}

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
extern unsigned char D_00153520[];
extern unsigned char D_00153530[];
extern unsigned char D_00153540[];
extern sceDmaEnv D_00159AD8;

/* sceDmaPutEnv. Adapted from Lombyte (GPL-3.0) for PAL. */
int func_001233E8(void *arg0) {
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
    ctrl = (ctrl & ~0x30) | (D_00153520[env->sts] << 4);
    ctrl = (ctrl & ~0xC0) | (D_00153530[env->std] << 6);
    ctrl = (ctrl & ~0xC) | (D_00153540[env->mfd] << 2);
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
    D_00159AD8 = *env;
    return 0;
}

LINKER_REMNANT("asm/remnants/core_text", func_001235C0);

extern int func_00123280(int);
extern void func_00123208(void *);

/* Reset the record at p with func_00123208, keep func_00123280(arg1) in
   p[12] unless it holds -1, clear p[8], and set flags 0x105 in p[0] with
   bits 2-3 cleared. The -1 test is unsigned, as retail builds 0xFFFFFFFF. */
void func_001235C8(int *p, int arg1) {
    int r = func_00123280(arg1);

    func_00123208(p);
    if ((unsigned int)p[12] != 0xFFFFFFFFU) {
        p[12] = r;
    }
    p[8] = 0;
    p[0] = (p[0] & ~0xC) | 0x105;
}

LINKER_REMNANT("asm/remnants/core_text", func_00123630);

/*
 * sceDmaPause: DI (func_0011D960), force D_ENABLEW.CPND (0x10000) if
 * D_ENABLER has it clear, read D_CTRL (a volatile read for its side
 * effect only), clear the channel's CHCR.STR (bit 8), restore
 * D_ENABLEW, EI (func_0011D9A8) if DI said interrupts were on, and
 * return the old CHCR.
 *
 * Exact under 2.9-ee. The mask has to be an unsigned constant
 * (`old & 0xFFFFFEFF`; `~0x100` is one addiu, SIZE 148), and the channel
 * word goes through a `volatile unsigned int *`, which keeps its load
 * after the D_CTRL read and gives retail's $4-$7 allocation. Under
 * 2.95.3 the same C differs in the prologue order only (8/152).
 */
int func_00123650(void *arg0) {
    volatile unsigned int *d = arg0;
    int en;
    unsigned int prev, old;
    en = func_0011D960();
    prev = *(volatile unsigned int *)0x1000F520;
    if ((prev & 0x10000) == 0)
        *(volatile unsigned int *)0x1000F590 = prev | 0x10000;
    *(volatile unsigned int *)0x1000E000;
    old = *d;
    *d = old & 0xFFFFFEFF;
    *(volatile unsigned int *)0x1000F590 = prev;
    if (en != 0) func_0011D9A8();
    return old;
}

LINKER_REMNANT("asm/remnants/core_text", func_001236E8);
