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
/* video input buffer: 0x5C bytes as touched by the asm */
typedef struct VB {
    int f0;             /* 0x0 */
    unsigned int f4;    /* 0x4 uncached address of the tag array */
    int count;          /* 0x8 */
    char padC[0xC];
    int f18;            /* 0x18 count * 0x800 */
    char pad1C[0x24];
    int sema;           /* 0x40 */
    int f44;
    long f48;           /* 0x48 cleared with one sd */
    int f50;            /* 0x50 */
    int f54;            /* 0x54 */
} VB;

typedef struct SemaParam {
    int f0;
    int max;
    int init;
    int fC;
    int f10;
    int f14;
} SemaParam;

extern int func_00118C70(SemaParam *);
extern int func_0023D090();

/* viBufCreate: fill the header, make a semaphore (1/1), reset, clear 0x48 */
int func_0023D018(VB *vb, int data, unsigned int tags, int count, int f50, int f54) {
    SemaParam sp;

    vb->f0 = data;
    vb->f4 = (tags & 0x0FFFFFFF) | 0x20000000;
    vb->count = count;
    vb->f18 = count << 11;
    vb->f50 = f50;
    vb->f54 = f54;
    sp.init = 1;
    sp.max = 1;
    vb->sema = func_00118C70(&sp);
    func_0023D090(vb);
    vb->f48 = 0;
    return 1;
}
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
extern void func_0023CF10(unsigned int);

/* Saved DMA/IPU register words at 0x1C-0x3C of the buffer. */
typedef struct VbDmaSave {
    char pad0[0x1C];
    unsigned int d4_madr;   /* 0x1C */
    unsigned int d4_tadr;   /* 0x20 */
    unsigned int d4_qwc;    /* 0x24 */
    unsigned int d4_chcr;   /* 0x28 */
    unsigned int d3_madr;   /* 0x2C */
    unsigned int d3_qwc;    /* 0x30 */
    unsigned int d3_chcr;   /* 0x34 */
    unsigned int ipu_top;   /* 0x38 */
    unsigned int ipu_ctrl;  /* 0x3C */
} VbDmaSave;

/* viBufStopDMA(ViBuf *): clears f44, stops channel 4 and saves its registers, waits for
 * the IPU_CTRL 0xF0 bits to clear, stops channel 3 and saves its registers plus
 * IPU_TOP/IPU_CTRL. Returns 1. */
int func_0023D540(ViBuf *vb) {
    VbDmaSave *s = (VbDmaSave *)vb;

    func_00118CB0(vb->sema);
    vb->f44 = 0;
    setD4_CHCR(5);
    s->d4_madr = *(volatile unsigned int *)0x1000B410;
    s->d4_tadr = *(volatile unsigned int *)0x1000B430;
    s->d4_qwc = *(volatile unsigned int *)0x1000B420;
    s->d4_chcr = *(volatile unsigned int *)0x1000B400;
    for (;;) {
        if (!(*(volatile unsigned int *)0x10002010 & 0xF0)) {
            break;
        }
    }
    func_0023CF10(0);
    s->d3_madr = *(volatile unsigned int *)0x1000B010;
    s->d3_qwc = *(volatile unsigned int *)0x1000B020;
    s->d3_chcr = *(volatile unsigned int *)0x1000B000;
    s->ipu_top = *(volatile unsigned int *)0x10002020;
    s->ipu_ctrl = *(volatile unsigned int *)0x10002010;
    func_00118C90(vb->sema);
    return 1;
}
extern int func_0023CEC8(ViBuf *, unsigned int);


/* (i - start) taken modulo count lies in [0, len) */
#define VB_IN_RANGE(i, start, len, count) \
    ((((i) + (count) - (start)) % (count)) >= 0 && (((i) + (count) - (start)) % (count)) < (len))

/* viBufRestartDMA(ViBuf *): from the registers saved at 0x1C-0x3C, backs channel 4 up by
 * the qwords still held in the IPU FIFO (IPU_TOP bits 8-11 and 16-17), rebuilds its
 * MADR/TADR/QWC/CHCR, restarts channel 3 if it had a transfer, re-issues the IPU
 * command with the saved bit position (IPU_TOP & 0x7F) when f10 is non-zero, restores
 * IPU_CTRL and sets f44. Returns 1. */
int func_0023D650(ViBuf *vb) {
    VbDmaSave *s = (VbDmaSave *)vb;
    int bp = s->ipu_top & 0x7F;
    int fifo = ((s->ipu_top >> 16) & 3) + ((s->ipu_top >> 8) & 0xF);
    unsigned int madr = s->d4_madr - (fifo << 4);
    unsigned int qwc = s->d4_qwc + fifo;
    unsigned int chcr = s->d4_chcr | 0x100;
    unsigned int tadr = s->d4_tadr;

    func_00118CB0(vb->sema);

    if (madr < vb->base) {
        int id;
        qwc = (vb->base - madr) >> 4;
        tadr = (unsigned int)vb->tags & 0x0FFFFFFF;
        madr += vb->count << 11;
        id = (s->d4_madr != vb->base && s->d4_madr != vb->base + (vb->count << 11)) ? 3 : 0;
        chcr = (s->d4_chcr & 0x0FFFFFFF) | (id << 28) | 0x100;
        if (!VB_IN_RANGE(0, vb->fC, vb->f10, vb->count)) {
            vb->fC = vb->count - 1;
            vb->f10++;
        }
    } else {
        int a = func_0023CEC8(vb, s->d4_madr);
        int b = func_0023CEC8(vb, madr);
        if (a != b) {
            int id;
            int size = vb->count << 11;
            qwc = (vb->base + (a << 11) - madr) >> 4;
            tadr = ((a << 4) + (unsigned int)vb->tags) & 0x0FFFFFFF;
            id = (vb->base + (s->d4_madr - vb->base) % size
                  == vb->base + (((vb->fC + vb->f10) % vb->count) << 11)) ? 0 : 3;
            chcr = (s->d4_chcr & 0x0FFFFFFF) | (id << 28) | 0x100;
            if (!VB_IN_RANGE(b, vb->fC, vb->f10, vb->count)) {
                vb->fC = b;
                vb->f10++;
            }
        }
    }

    if (s->d3_madr != 0 && s->d3_qwc != 0) {
        *(volatile unsigned int *)0x1000B010 = s->d3_madr;
        *(volatile unsigned int *)0x1000B020 = s->d3_qwc;
        func_0023CF10(s->d3_chcr | 0x100);
    }
    if (vb->f10 != 0) {
        for (;;) {
            if (*(volatile int *)0x10002010 >= 0) {
                break;
            }
        }
        *(volatile unsigned int *)0x10002000 = bp;
        for (;;) {
            if (*(volatile int *)0x10002010 >= 0) {
                break;
            }
        }
    }
    *(volatile unsigned int *)0x1000B410 = madr;
    *(volatile unsigned int *)0x1000B430 = tadr;
    *(volatile unsigned int *)0x1000B420 = qwc;
    if (vb->f10 != 0) {
        setD4_CHCR(chcr);
    }
    *(volatile unsigned int *)0x10002010 = s->ipu_ctrl;
    vb->f44 = 1;
    func_00118C90(vb->sema);
    return 1;
}
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
