/* Carried from the PAL decompilation's src/core/00122630.c (matched C there) (games/rac1) through the version
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
/*
 * Privileged/ordering CPU instructions the retail C emitted through the SDK's
 * own inline helpers. Kept here as the only place for them, like qcopy();
 * functions call these, never write the asm themselves.
 */
static __inline__ void CpuSync_cscegssetdefloadimage(void) { __asm__ __volatile__("sync"); }
extern void func_0011A3C8(void *);
extern char D_001530B0[];
typedef int u128_li_cscegssetdefloadimage __attribute__((mode(TI)));
typedef struct {
    unsigned long NLOOP:15; unsigned long EOP:1; unsigned long pad16:30; unsigned long PRE:1;
    unsigned long PRIM:11; unsigned long FLG:2; unsigned long NREG:4;
    unsigned long REGS0:4; unsigned long REGS1:4; unsigned long REGS2:4; unsigned long REGS3:4;
    unsigned long REGS4:4; unsigned long REGS5:4; unsigned long REGS6:4; unsigned long REGS7:4;
    unsigned long REGS8:4; unsigned long REGS9:4; unsigned long REGS10:4; unsigned long REGS11:4;
    unsigned long REGS12:4; unsigned long REGS13:4; unsigned long REGS14:4; unsigned long REGS15:4;
} LiGifTag_cscegssetdefloadimage __attribute__((aligned(16)));
typedef struct {
    LiGifTag_cscegssetdefloadimage giftag0;
    unsigned long bitbltbuf; long bitbltbufaddr;
    unsigned long trxpos; long trxposaddr;
    unsigned long trxreg; long trxregaddr;
    unsigned long trxdir; long trxdiraddr;
    LiGifTag_cscegssetdefloadimage giftag1;
} LiLoadImage_cscegssetdefloadimage __attribute__((aligned(16)));



int sceGsSetDefLoadImage(LiLoadImage_cscegssetdefloadimage *lp, short dbp, short dbw, short dpsm, short x, short y, short w, short h)
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
        func_0011A3C8(D_001530B0);
        return 0;
    }
    *(u128_li_cscegssetdefloadimage *)&lp->giftag0 = (u128_li_cscegssetdefloadimage)0;
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
    *(u128_li_cscegssetdefloadimage *)&lp->giftag1 = (u128_li_cscegssetdefloadimage)0;
    lp->giftag1.NLOOP = size;
    lp->giftag1.EOP = 1;
    lp->giftag1.FLG = 2;
    CpuSync_cscegssetdefloadimage();
    return 6;
}
