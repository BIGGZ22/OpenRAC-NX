#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x122630-0x122818. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's libgraph, graph015.o (sceGsSetDefLoadImage). Built with Sony's
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

extern void func_0011A6C8(void *);
extern char D_001531B0[];

typedef int u128_li __attribute__((mode(TI)));
typedef struct {
    unsigned long NLOOP:15; unsigned long EOP:1; unsigned long pad16:30; unsigned long PRE:1;
    unsigned long PRIM:11; unsigned long FLG:2; unsigned long NREG:4;
    unsigned long REGS0:4; unsigned long REGS1:4; unsigned long REGS2:4; unsigned long REGS3:4;
    unsigned long REGS4:4; unsigned long REGS5:4; unsigned long REGS6:4; unsigned long REGS7:4;
    unsigned long REGS8:4; unsigned long REGS9:4; unsigned long REGS10:4; unsigned long REGS11:4;
    unsigned long REGS12:4; unsigned long REGS13:4; unsigned long REGS14:4; unsigned long REGS15:4;
} LiGifTag __attribute__((aligned(16)));
typedef struct {
    LiGifTag giftag0;
    unsigned long bitbltbuf; long bitbltbufaddr;
    unsigned long trxpos; long trxposaddr;
    unsigned long trxreg; long trxregaddr;
    unsigned long trxdir; long trxdiraddr;
    LiGifTag giftag1;
} LiLoadImage __attribute__((aligned(16)));

int func_00122630(LiLoadImage *lp, short dbp, short dbw, short dpsm, short x, short y, short w, short h)
{
    int size = 0;

    switch (dpsm) {
    case 0x00:
    case 0x30:
        size = (w * h) >> 2;
        break;
    case 0x01:
    case 0x31:
        size = (w * h * 3) >> 4;
        break;
    case 0x02:
    case 0x0A:
    case 0x32:
    case 0x3A:
        size = (w * h) >> 3;
        break;
    case 0x13:
    case 0x1B:
        size = (w * h) >> 4;
        break;
    case 0x14:
    case 0x24:
    case 0x2C:
        size = (w * h) >> 5;
        break;
    }
    if (size > 0x7FFF) {
        func_0011A6C8(D_001531B0);
        return 0;
    }
    *(u128_li *)&lp->giftag0 = (u128_li)0;
    lp->giftag0.NLOOP = 4;
    lp->giftag0.NREG = 1;
    lp->giftag0.REGS0 = 0xE;
    lp->bitbltbuf = ((unsigned long)dbp << 32) | ((unsigned long)dbw << 48) | ((unsigned long)dpsm << 56);
    lp->bitbltbufaddr = 0x50;
    lp->trxpos = ((unsigned long)x << 32) | ((unsigned long)y << 48);
    lp->trxposaddr = 0x51;
    lp->trxreg = (unsigned long)w | ((unsigned long)h << 32);
    lp->trxregaddr = 0x52;
    lp->trxdir = 0;
    lp->trxdiraddr = 0x53;
    *(u128_li *)&lp->giftag1 = (u128_li)0;
    lp->giftag1.NLOOP = size;
    lp->giftag1.EOP = 1;
    lp->giftag1.FLG = 2;
    CpuSync();
    return 6;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00122814);
