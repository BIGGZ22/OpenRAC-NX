#include "common.h"
#include "structs.h"

/*
 * movie/vibuf.cpp in the original source; text 0x23CEC8-0x23DE98.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

typedef struct TimeStamp {
    long pts;
    long dts;
    int pos;
    int len;
} TimeStamp;

typedef struct ViBuf {
    unsigned int base;      /* 0x0 */
    unsigned char *tags;    /* 0x4 */
    int count;              /* 0x8 */
    int fC;                 /* 0xC */
    int f10;                /* 0x10 */
    int f14;                /* 0x14 */
    int f18;
    char pad1C[0x24];
    int sema;               /* 0x40 */
    int f44;                /* 0x44 */
    char pad48[8];
    char *tbl;              /* 0x50, entries of 0x18 bytes */
    int ntbl;               /* 0x54 */
    int f58;                /* 0x58 */
    int f5C;                /* 0x5C */
} ViBuf;

/* getFIFOindex(ViBuf *, void *) */
int func_0023CEC8(ViBuf *vb, unsigned int addr) {
    unsigned int mask = 0x0FFFFFFF;
    if (addr == (((vb->count << 4) + (unsigned int)vb->tags + 16) & mask)) {
        return 0;
    }
    return (addr - vb->base) >> 11;
}
extern void func_0011D960(void);
extern void func_0011D9A8(void);

/* setD3_CHCR(unsigned int) */
void func_0023CF10(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}
/* setD4_CHCR(unsigned int) */
void func_0023CF80(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B400 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}
/* scTag2 */
void func_0023CFF0(unsigned long *tag, unsigned int a, unsigned int b, unsigned int c) {
    *tag = ((unsigned long)a << 32) | ((unsigned long)b << 28) | (unsigned long)c;
}
INCLUDE_ASM("asm/nonmatchings/text", func_0023D018); /* viBufCreate */
extern void func_0023CFF0(unsigned long *, unsigned int, unsigned int, unsigned int);
extern void func_0023CF80(unsigned int);

/* viBufReset(ViBuf *) */
int func_0023D090(ViBuf *vb) {
    int i;

    vb->f44 = 1;
    vb->fC = 0;
    vb->f10 = 0;
    vb->f14 = 0;
    vb->f58 = 0;
    vb->f5C = 0;

    for (i = 0; i < vb->ntbl; i++) {
        ((TimeStamp *)vb->tbl)[i].pts = -1;
        ((TimeStamp *)vb->tbl)[i].dts = -1;
        ((TimeStamp *)vb->tbl)[i].pos = 0;
        ((TimeStamp *)vb->tbl)[i].len = 0;
    }

    for (i = 0; i < vb->count; i++) {
        unsigned int mask = 0x0FFFFFFF;
        scTag2((unsigned long *)(vb->tags + i * 16), ((i << 11) + vb->base) & mask, 3, 0x80);
    }
    {
        unsigned int mask = 0x0FFFFFFF;
        scTag2((unsigned long *)(vb->tags + i * 16), (unsigned int)vb->tags & mask, 2, 0);

        *(volatile unsigned int *)0x1000B420 = 0;
        *(volatile unsigned int *)0x1000B410 = vb->base & mask;
        *(volatile unsigned int *)0x1000B430 = (unsigned int)vb->tags & mask;
    }
    setD4_CHCR(5);
    return 1;
}
/* viBufBeginPut(ViBuf *, unsigned char **, int *, unsigned char **, int *) */
void func_0023D1F0(ViBuf *vb, unsigned char **p1, int *n1, unsigned char **p2, int *n2) {
    int tagIdx;
    int pos;
    int available;

    func_00118CB0(vb->sema);

    tagIdx = vb->f10 + 2;
    pos = ((vb->fC + vb->f10) << 11) + vb->f14;
    available = ((vb->count - tagIdx) << 11) - vb->f14;
    pos = pos % vb->f18;

    if (vb->f18 - pos >= available) {
        *p1 = (unsigned char *)(vb->base + pos);
        *n1 = available;
        *p2 = 0;
        *n2 = 0;
    } else {
        *p1 = (unsigned char *)(vb->base + pos);
        *n1 = vb->f18 - pos;
        *p2 = (unsigned char *)vb->base;
        *n2 = available - (vb->f18 - pos);
    }

    func_00118C90(vb->sema);
}
/* viBufEndPut(ViBuf *, int) */
void func_0023D2E8(ViBuf *vb, int size) {
    func_00118CB0(vb->sema);
    vb->f14 += size;
    *(long *)(vb->pad48) += size;
    func_00118C90(vb->sema);
}
INCLUDE_ASM("asm/nonmatchings/text", func_0023D340); /* viBufAddDMA(ViBuf *) */
INCLUDE_ASM("asm/nonmatchings/text", func_0023D540); /* viBufStopDMA(ViBuf *) */
INCLUDE_ASM("asm/nonmatchings/text", func_0023D650); /* viBufRestartDMA(ViBuf *) */
extern void func_00118C80(int);     /* DeleteSema */

/* viBufDelete(ViBuf *) -- stops DMA channel 4 (setD4_CHCR(5)), zeroes its MADR/QWC/TADR
 * and deletes the buffer's semaphore. Returns 1. */
int func_0023D988(ViBuf *vb) {
    setD4_CHCR(5);
    *(volatile unsigned int *)0x1000B420 = 0;
    *(volatile unsigned int *)0x1000B410 = 0;
    *(volatile unsigned int *)0x1000B430 = 0;
    func_00118C80(vb->sema);
    return 1;
}
extern int func_00118CB0(int);
extern int func_00118C90(int);

/* viBufCount(ViBuf *) */
int func_0023D9E0(ViBuf *vb) {
    int count;
    func_00118CB0(vb->sema);
    count = (vb->f10 << 11) + vb->f14;
    func_00118C90(vb->sema);
    return count;
}
/* viBufFlush(ViBuf *) */
void func_0023DA30(ViBuf *vb) {
    func_00118CB0(vb->sema);
    vb->f14 = ((vb->f14 + 0x7FF) / 0x800) * 0x800;
    func_00118C90(vb->sema);
}
INCLUDE_ASM("asm/nonmatchings/text", func_0023DA88); /* viBufModifyPts(ViBuf *, TimeStamp *) */
extern int func_0023DA88(ViBuf *, TimeStamp *);

/* viBufPutTs(ViBuf *, TimeStamp *) */
int func_0023DBE0(ViBuf *vb, TimeStamp *ts) {
    int ret = 0;
    func_00118CB0(vb->sema);
    if (vb->f58 < vb->ntbl) {
        viBufModifyPts(vb, ts);
        if (ts->pts >= 0 || ts->dts >= 0) {
            ((TimeStamp *)vb->tbl)[vb->f5C].pts = ts->pts;
            ((TimeStamp *)vb->tbl)[vb->f5C].dts = ts->dts;
            ((TimeStamp *)vb->tbl)[vb->f5C].pos = ts->pos;
            ((TimeStamp *)vb->tbl)[vb->f5C].len = ts->len;
            vb->f58++;
            vb->f5C = (vb->f5C + 1) % vb->ntbl;
        }
        ret = 1;
    }
    func_00118C90(vb->sema);
    return ret;
}
INCLUDE_ASM("asm/nonmatchings/text", func_0023DCF0); /* viBufGetTs(ViBuf *, TimeStamp *) */
