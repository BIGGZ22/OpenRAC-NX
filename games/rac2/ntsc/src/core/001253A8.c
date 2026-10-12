#include "common.h"
#include "structs.h"

extern int r1_D_001313EC;

extern int r1_func_0011B4C8();

extern void r1_func_00118C90(int);

extern char r1_D_00132E08[];

extern int r1_func_00121040(int);

extern int r1_D_001325C0;

extern volatile int r1_D_00131414;

extern void r1_func_0011A6C8();

extern int r1_D_001313D0;

extern char r1_D_001530F0[];

extern char r1_D_00153110[];

typedef struct {
    char b[8];
} CdClock;

extern void *r1_func_00121D08();

typedef struct {
    short sceGsInterMode;
    short sceGsOutMode;
    short sceGsFFMode;
    short sceGsVersion;
    volatile int (*sceGsVSCfunc)(int);
    int sceGsVSCid;
} sceGsGParam;

extern void *r1_func_00121D08(void);

          /* sceGsGetGParam */
extern int r1_func_00118DE0();

                /* GsPutIMR */
extern int r1_func_00118AA0();

                /* RemoveIntcHandler */
extern int r1_func_00119328();

extern void r1_func_00118A50(int, int, int);

INCLUDE_ASM("asm/core", func_001253A8);

/* rac1-decomp src/core/00121750.c: func_00121930 */
/* sceCdGetError, r1_func_00120E98's sibling: S-command 3, RPC 4, and -1
   rather than 0 as the failure result.

   The RPC failure is handled first (SignalSema, return -1), then the
   success path. That is the layout retail has, and it keeps 2.9-ee from
   cross-jumping the two `return -1` tails into one (SIZE 144/152 with
   the success path first, which 2.95.3 needed). */
int func_00125588(void) {
    int r;

    if (r1_func_00121040(3) == 0) {
        return -1;
    }
    if (r1_func_0011B4C8(r1_D_00132E08, 4, 0, 0, 0, &r1_D_001325C0, 4, 0, 0) < 0) {
        r1_func_00118C90(r1_D_001313EC);
        return -1;
    }
    r = *(int *)((unsigned int)&r1_D_001325C0 | 0x20000000);
    r1_func_00118C90(r1_D_001313EC);
    return r;
}

/* rac1-decomp src/core/00121750.c: func_001219C8 */
/* sceCdBreak, sibling of func_00125588: S-command 0x1E, RPC 0x16 on
   r1_D_00132E08 under the r1_D_001313EC semaphore, with the r1_D_00131414 busy flag
   set around it; returns the reply word read uncached. The volatile flag
   and semaphore reads stay out of the delay slots, where reorg never puts
   a volatile access (this module sees its globals as volatile, as
   cdvd000.o does).

   As in func_00125588, the RPC failure path comes first: with the success
   path first 2.9-ee merges the two `return 0` tails (SIZE 176/184). */
int func_00125620(void) {
    int r;

    if (r1_func_00121040(0x1E) == 0) {
        return 0;
    }
    r1_D_00131414 = 8;
    if (r1_func_0011B4C8(r1_D_00132E08, 0x16, 0, 0, 0, &r1_D_001325C0, 4, 0, 0) < 0) {
        r1_func_00118C90(*(volatile int *)&r1_D_001313EC);
        r1_D_00131414 = 0;
        return 0;
    }
    r1_D_00131414 = 0;
    r = *(int *)((unsigned int)&r1_D_001325C0 | 0x20000000);
    r1_func_00118C90(*(volatile int *)&r1_D_001313EC);
    return r;
}

/* rac1-decomp src/core/00121750.c: func_00121A80 */
/* sceCdReadClock: S-command 0xF, RPC 1 with a 0x10-byte reply; the
   8-byte clock after the status word is copied out uncached. Failure
   path first, as in func_00125588. */
int func_001256D8(CdClock *out) {
    int r;

    if (r1_func_00121040(0xF) == 0) {
        return 0;
    }
    if (r1_D_001313D0 > 0) {
        r1_func_0011A6C8(r1_D_001530F0);
    }
    if (r1_func_0011B4C8(r1_D_00132E08, 1, 0, 0, 0, &r1_D_001325C0, 0x10, 0, 0) < 0) {
        r1_func_00118C90(r1_D_001313EC);
        return 0;
    }
    *out = *(CdClock *)(((unsigned int)&r1_D_001325C0 + 4) | 0x20000000);
    if (r1_D_001313D0 > 0) {
        r1_func_0011A6C8(r1_D_00153110);
    }
    r = *(int *)((unsigned int)&r1_D_001325C0 | 0x20000000);
    r1_func_00118C90(r1_D_001313EC);
    return r;
}

/* rac1-decomp src/core/00121750.c: func_00121B78 */
  /* SetGsCrt */

/* sceGsResetGraph. Adapted from Lombyte (MIT) for PAL. */
void func_001257D0(short mode, short inter, short out, short ff) {
    sceGsGParam *gp;

    switch (mode) {
    case 0:
        gp = (sceGsGParam *)r1_func_00121D08();
        *(volatile unsigned long *)0x12001000 = 0x200;
        gp->sceGsInterMode = inter;
        gp->sceGsOutMode = out;
        gp->sceGsVersion = (short)((*(volatile unsigned long *)0x12001000 >> 16) & 0xFF);
        r1_func_00118DE0(0xFF00);
        gp->sceGsFFMode = (ff != 0);
        if (gp->sceGsVSCfunc != 0) {
            r1_func_00119328(2);
            r1_func_00118AA0(2, gp->sceGsVSCid);
            gp->sceGsVSCfunc = 0;
            gp->sceGsVSCid = 0;
        }
        r1_func_00118A50(inter & 1, out & 0xFF, ff & 1);
        break;
    case 1:
        *(volatile unsigned long *)0x12001000 = 0x100;
        break;
    case 5:
        gp = (sceGsGParam *)r1_func_00121D08();
        gp->sceGsFFMode = (ff != 0);
        gp->sceGsInterMode = inter;
        gp->sceGsOutMode = out;
        gp->sceGsVersion = (short)((*(volatile unsigned long *)0x12001000 >> 16) & 0xFF);
        r1_func_00118A50(inter & 1, out & 0xFF, ff & 1);
        break;
    }
}
