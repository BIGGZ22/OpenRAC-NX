#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x122AD8-0x123168. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's libgraph, graph018.o (sceGsExecStoreImage). Built with Sony's
 * 2.9-ee (Makefile.sn, EE29_CORE), like the prebuilt libgraph.a.
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

typedef int u128_si __attribute__((mode(TI)));
typedef struct {
    unsigned long SBP:14; unsigned long p14:2; unsigned long SBW:6; unsigned long p22:2;
    unsigned long SPSM:6; unsigned long p30:2; unsigned long DBP:14; unsigned long p46:2;
    unsigned long DBW:6; unsigned long p54:2; unsigned long DPSM:6; unsigned long p62:2;
} SiBitbltbuf;
typedef struct {
    unsigned long RRW:12; unsigned long p12:20; unsigned long RRH:12; unsigned long p44:20;
} SiTrxreg;
typedef struct {
    unsigned int vifcode[4];
    unsigned long giftag[2];
    SiBitbltbuf bitbltbuf; long bitbltbufaddr;
    unsigned long trxpos; long trxposaddr;
    SiTrxreg trxreg; long trxregaddr;
    unsigned long finish; long finishaddr;
    unsigned long trxdir; long trxdiraddr;
} SiStoreImage __attribute__((aligned(16)));

extern char D_00153310[];
extern char D_00153348[];
extern char D_00153378[];
extern char D_001533B8[];
extern u128_si D_00132E50;
extern unsigned long func_00118DD0(void);
extern int func_00118DE0(unsigned long);

int func_00122AD8(SiStoreImage *sp, u128_si *dstaddr)
{
    unsigned int cnt = 0;
    int nfull = 0, nrem = 0, nbyte = 0, nrest = 0;
    int w, h, psm, bytes, i, old, n;
    int h2;
    union { u128_si q; unsigned char c[16]; } buf;

    w = sp->trxreg.RRW;
    h = sp->trxreg.RRH;
    psm = sp->bitbltbuf.SPSM;
    h2 = 0;
    switch (psm) {
    case 0x00:
    case 0x30:
        bytes = w * h * 4;
        nbyte = bytes & 0xF;
        nfull = (bytes >> 4) & ~7u;
        nrem = (bytes >> 4) & 7;
        if (!nbyte) {
            h2 = h;
            nrest = 0;
        } else {
            h2 = (h + 3) & ~3;
            n = w * h2;
            nrest = (n >> 2) - nfull - nrem - 1;
        }
        break;
    case 0x01:
    case 0x31:
        bytes = w * h * 3;
        nbyte = bytes & 0xF;
        nfull = (bytes >> 4) & ~7u;
        nrem = (bytes >> 4) & 7;
        if (!nbyte) {
            h2 = h;
            nrest = 0;
        } else {
            h2 = (h + 15) & ~15;
            n = w * h2;
            n = ((n * 3) >> 4) - nfull;
            nrest = n - nrem - 1;
        }
        break;
    case 0x02:
    case 0x0A:
    case 0x32:
    case 0x3A:
        bytes = w * h * 2;
        nbyte = bytes & 0xF;
        nrem = (bytes >> 4) & 7;
        nfull = (bytes >> 4) & ~7u;
        if (!nbyte) {
            h2 = h;
            nrest = 0;
        } else {
            h2 = (h + 7) & ~7u;
            n = w * h2;
            nrest = (n >> 3) - nfull - nrem - 1;
        }
        break;
    case 0x13:
    case 0x1B:
        bytes = w * h;
        nbyte = bytes & 0xF;
        nrem = (bytes >> 4) & 7;
        nfull = (bytes >> 4) & ~7u;
        if (!nbyte) {
            h2 = h;
            nrest = 0;
        } else {
            h2 = (h + 7) & ~7u;
            n = w * h2;
            nrest = (n >> 4) - nfull - nrem - 1;
        }
        break;
    case 0x14:
    case 0x24:
    case 0x2C:
        bytes = (w * h) >> 1;
        nbyte = bytes & 0xF;
        nrem = (bytes >> 4) & 7;
        nfull = (bytes >> 4) & ~7u;
        if (!nbyte) {
            h2 = h;
            nrest = 0;
        } else {
            h2 = (h + 7) & ~7u;
            n = w * h2;
            nrest = (n >> 5) - nfull - nrem - 1;
        }
        break;
    }

    if (nbyte) {
        *(long *)((unsigned int)&sp->trxreg | 0x20000000) = (long)(int)sp->trxreg.RRW | ((long)h2 << 32);
    }

    while (*(volatile int *)0x10009000 & 0x100) {
        if (cnt++ > 0x1000000) {
            func_0011A6C8(D_00153310);
            return -1;
        }
    }

    old = func_00118DE0(func_00118DD0() | 0x200);
    *(volatile long *)0x12001000 = 2;
    *(volatile int *)0x10009020 = 7;
    if (((unsigned int)sp & 0x70000000) == 0x70000000) {
        *(volatile int *)0x10009010 = ((unsigned int)sp & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile int *)0x10009010 = (unsigned int)sp & 0x0FFFFFFF;
    }
    *(volatile int *)0x10009000 = 0x101;
    while (*(volatile int *)0x10009000 & 0x100) {
        if (cnt++ > 0x1000000) {
            func_0011A6C8(D_00153310);
            return -1;
        }
    }

    while (!(*(volatile long *)0x12001000 & 2)) {
        if (cnt++ > 0x1000000) {
            func_0011A6C8(D_00153348);
            *(volatile u128_si *)0x10005000 = D_00132E50;
            return -1;
        }
    }

    *(volatile int *)0x10003C00 = 0x800000;
    *(volatile long *)0x12001040 = 1;

    if (nfull) {
        *(volatile int *)0x10009020 = nfull;
        if (((unsigned int)dstaddr & 0x70000000) == 0x70000000) {
            *(volatile int *)0x10009010 = ((unsigned int)dstaddr & 0x0FFFFFFF) | 0x80000000;
        } else {
            *(volatile int *)0x10009010 = (unsigned int)dstaddr & 0x0FFFFFFF;
        }
        *(volatile int *)0x10009000 = 0x100;
        while (*(volatile int *)0x10009000 & 0x100) {
            if (cnt++ > 0x1000000) {
                func_0011A6C8(D_00153378);
                *(volatile long *)0x12001000 = 0x100;
                *(volatile long *)0x12001040 = 0;
                *(volatile int *)0x10003000 = 1;
                *(volatile int *)0x10003C10 = 1;
                return -1;
            }
        }
    }

    for (i = 0; i < nrem; i++) {
        while (!(*(volatile int *)0x10003C00 & 0x1F000000)) {
            if (cnt++ > 0x1000000) {
                func_0011A6C8(D_001533B8);
                *(volatile long *)0x12001000 = 0x100;
                *(volatile long *)0x12001040 = 0;
                *(volatile int *)0x10003000 = 1;
                *(volatile int *)0x10003C10 = 1;
                return -1;
            }
        }
        dstaddr[nfull + i] = *(volatile u128_si *)0x10005000;
    }

    if (nbyte) {
        while (!(*(volatile int *)0x10003C00 & 0x1F000000)) {
            if (cnt++ > 0x1000000) {
                func_0011A6C8(D_001533B8);
                *(volatile long *)0x12001000 = 0x100;
                *(volatile long *)0x12001040 = 0;
                *(volatile int *)0x10003000 = 1;
                *(volatile int *)0x10003C10 = 1;
                return -1;
            }
        }
        buf.q = *(volatile u128_si *)0x10005000;
        for (i = 0; i < nbyte; i++) {
            ((unsigned char *)&dstaddr[nfull + nrem])[i] = buf.c[i];
        }
        for (i = 0; i < nrest; i++) {
            while (!(*(volatile int *)0x10003C00 & 0x1F000000)) {
                if (cnt++ > 0x1000000) {
                    func_0011A6C8(D_001533B8);
                    *(volatile long *)0x12001000 = 0x100;
                    *(volatile long *)0x12001040 = 0;
                    *(volatile int *)0x10003000 = 1;
                    *(volatile int *)0x10003C10 = 1;
                    return -1;
                }
            }
            buf.q = *(volatile u128_si *)0x10005000;
        }
    }

    *(volatile int *)0x10003C00 = 0;
    *(volatile long *)0x12001040 = 0;
    func_00118DE0(old);
    {
        u128_si t = D_00132E50;
        *(volatile long *)0x12001000 = 2;
        *(volatile u128_si *)0x10005000 = t;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00123164);
