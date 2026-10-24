#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x121750-0x121D18. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Six Sony SDK archive members back to back (each ends 8-byte aligned, so
 * no fill separates them): libcdvd's cdvd005.o (sceCdRead), cdvd015.o
 * (sceCdGetError, func_00121930), cdvd018.o (sceCdBreak, func_001219C8)
 * and cdvd039.o (sceCdReadClock), then libgraph's graph001.o
 * (sceGsResetGraph, sceGsGetGParam = func_00121D08). Built with Sony's
 * 2.9-ee (Makefile.sn, EE29_CORE), like the prebuilt archives they match.
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
extern int func_00120E98(void);

/* sceCdRead: queue one CD/DVD read request (lba, sectors, buffer, mode)
 * in the request block at D_001314C0 and start it through the RPC
 * func_0011B4C8. Returns 1 when the RPC was accepted, 0 otherwise.
 * The volatile state clears precede the semaphore read; the verified
 * GNU 2.10-ee profile fills SignalSema's call slot with that word load.
 * Sector size by mode->datapattern: 1 -> 0x918, 2 -> 0x924, else 0x800. */
extern int func_0011B4C8(void *, int, int, void *, int, void *, int, void *, void *);
extern int D_001313F4 NOT_SDA;
extern char D_001314C0[];
extern char D_001324C0[];
extern int D_00132580 NOT_SDA;
extern char D_001530C8[];
extern char D_001530E0[];
extern volatile int D_00131414;
extern void func_0011AD70(void *, int);
extern void func_00120A88();

int func_00121750(int lba, int sectors, void *buf, unsigned char *mode) {
    char *req = D_001314C0;
    int size;
    int r;

    if ((D_001313F4 & 1) == 0) {
        if (func_00120E98() == 6) {
            return 0;
        }
    }
    if (func_00120D28(4) == 0) {
        return 0;
    }

    *(int *)(req + 0x0) = lba;
    *(int *)(req + 0x4) = sectors;
    *(void **)(req + 0x8) = buf;
    req[0xC] = mode[0];
    req[0xD] = mode[1];
    req[0xE] = mode[2];
    *(void **)(req + 0x10) = D_001324C0;
    *(void **)(req + 0x14) = &D_00132580;

    switch (mode[2]) {
    case 1:
        size = sectors * 0x918;
        break;
    case 2:
        size = sectors * 0x924;
        break;
    case 0:
    default:
        size = sectors << 11;
        break;
    }

    D_00132580 = 0;
    if ((*(volatile int *)&D_001313F4 & 2) == 0) {
        func_0011AD70(buf, size);
    }
    func_0011AD70(D_001324C0, 0x90);
    func_0011AD70(req, 0x18);
    func_0011AD70(&D_00132580, 4);
    if (D_001313D0 > 0) {
        func_0011A6C8(D_001530C8);
    }

    D_00131414 = 1;
    *(volatile int *)&D_001313F0 = 1;
    r = func_0011B4C8(D_00132590, 1, 1, req, 0x18, 0, 0,
                      (void *)func_00120A88, D_001324C0);
    if (r < 0) {
        D_00131414 = 0;
        *(volatile int *)&D_001313F0 = 0;
        func_00118C90(*(volatile int *)&D_001313E8);
        return 0;
    }
    if (D_001313D0 > 0) {
        func_0011A6C8(D_001530E0);
    }
    return 1;
}

extern int func_00121040(int);
extern int D_001325C0;

/* sceCdGetError, func_00120E98's sibling: S-command 3, RPC 4, and -1
   rather than 0 as the failure result.

   The RPC failure is handled first (SignalSema, return -1), then the
   success path. That is the layout retail has, and it keeps 2.9-ee from
   cross-jumping the two `return -1` tails into one (SIZE 144/152 with
   the success path first, which 2.95.3 needed). */
int func_00121930(void) {
    int r;

    if (func_00121040(3) == 0) {
        return -1;
    }
    if (func_0011B4C8(D_00132E08, 4, 0, 0, 0, &D_001325C0, 4, 0, 0) < 0) {
        func_00118C90(D_001313EC);
        return -1;
    }
    r = *(int *)((unsigned int)&D_001325C0 | 0x20000000);
    func_00118C90(D_001313EC);
    return r;
}

extern volatile int D_00131414;

/* sceCdBreak, sibling of func_00121930: S-command 0x1E, RPC 0x16 on
   D_00132E08 under the D_001313EC semaphore, with the D_00131414 busy flag
   set around it; returns the reply word read uncached. The volatile flag
   and semaphore reads stay out of the delay slots, where reorg never puts
   a volatile access (this module sees its globals as volatile, as
   cdvd000.o does).

   As in func_00121930, the RPC failure path comes first: with the success
   path first 2.9-ee merges the two `return 0` tails (SIZE 176/184). */
int func_001219C8(void) {
    int r;

    if (func_00121040(0x1E) == 0) {
        return 0;
    }
    D_00131414 = 8;
    if (func_0011B4C8(D_00132E08, 0x16, 0, 0, 0, &D_001325C0, 4, 0, 0) < 0) {
        func_00118C90(*(volatile int *)&D_001313EC);
        D_00131414 = 0;
        return 0;
    }
    D_00131414 = 0;
    r = *(int *)((unsigned int)&D_001325C0 | 0x20000000);
    func_00118C90(*(volatile int *)&D_001313EC);
    return r;
}

extern char D_001530F0[];
extern char D_00153110[];
typedef struct {
    char b[8];
} CdClock;

/* sceCdReadClock: S-command 0xF, RPC 1 with a 0x10-byte reply; the
   8-byte clock after the status word is copied out uncached. Failure
   path first, as in func_00121930. */
int func_00121A80(CdClock *out) {
    int r;

    if (func_00121040(0xF) == 0) {
        return 0;
    }
    if (D_001313D0 > 0) {
        func_0011A6C8(D_001530F0);
    }
    if (func_0011B4C8(D_00132E08, 1, 0, 0, 0, &D_001325C0, 0x10, 0, 0) < 0) {
        func_00118C90(D_001313EC);
        return 0;
    }
    *out = *(CdClock *)(((unsigned int)&D_001325C0 + 4) | 0x20000000);
    if (D_001313D0 > 0) {
        func_0011A6C8(D_00153110);
    }
    r = *(int *)((unsigned int)&D_001325C0 | 0x20000000);
    func_00118C90(D_001313EC);
    return r;
}

typedef struct {
    short sceGsInterMode;
    short sceGsOutMode;
    short sceGsFFMode;
    short sceGsVersion;
    volatile int (*sceGsVSCfunc)(int);
    int sceGsVSCid;
} sceGsGParam;

extern void *func_00121D08(void);          /* sceGsGetGParam */
extern int func_00118DE0();                /* GsPutIMR */
extern int func_00118AA0();                /* RemoveIntcHandler */
extern int func_00119328();
extern void func_00118A50(int, int, int);  /* SetGsCrt */

/* sceGsResetGraph. Adapted from Lombyte (GPL-3.0) for PAL. */
void func_00121B78(short mode, short inter, short out, short ff) {
    sceGsGParam *gp;

    switch (mode) {
    case 0:
        gp = (sceGsGParam *)func_00121D08();
        *(volatile unsigned long *)0x12001000 = 0x200;
        gp->sceGsInterMode = inter;
        gp->sceGsOutMode = out;
        gp->sceGsVersion = (short)((*(volatile unsigned long *)0x12001000 >> 16) & 0xFF);
        func_00118DE0(0xFF00);
        gp->sceGsFFMode = (ff != 0);
        if (gp->sceGsVSCfunc != 0) {
            func_00119328(2);
            func_00118AA0(2, gp->sceGsVSCid);
            gp->sceGsVSCfunc = 0;
            gp->sceGsVSCid = 0;
        }
        func_00118A50(inter & 1, out & 0xFF, ff & 1);
        break;
    case 1:
        *(volatile unsigned long *)0x12001000 = 0x100;
        break;
    case 5:
        gp = (sceGsGParam *)func_00121D08();
        gp->sceGsFFMode = (ff != 0);
        gp->sceGsInterMode = inter;
        gp->sceGsOutMode = out;
        gp->sceGsVersion = (short)((*(volatile unsigned long *)0x12001000 >> 16) & 0xFF);
        func_00118A50(inter & 1, out & 0xFF, ff & 1);
        break;
    }
}

extern char D_00132E40[];

void *func_00121D08(void) {
    return D_00132E40;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00121D14);
