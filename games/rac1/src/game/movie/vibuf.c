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
int getFIFOindex(ViBuf *vb, unsigned int addr) {
    unsigned int mask = 0x0FFFFFFF;
    if (addr == (((vb->count << 4) + (unsigned int)vb->tags + 16) & mask)) {
        return 0;
    }
    return (addr - vb->base) >> 11;
}
extern void func_0011D960(void);
extern void func_0011D9A8(void);

/* setD3_CHCR(unsigned int) */
void setD3_CHCR(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}
/* setD4_CHCR(unsigned int) */
void setD4_CHCR(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B400 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}
/* scTag2 */
void scTag2(unsigned long *tag, unsigned int a, unsigned int b, unsigned int c) {
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
int viBufCreate(VB *vb, int data, unsigned int tags, int count, int f50, int f54) {
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
    viBufReset(vb);
    vb->f48 = 0;
    return 1;
}
extern void func_0023CFF0(unsigned long *, unsigned int, unsigned int, unsigned int);
extern void func_0023CF80(unsigned int);

/* viBufReset(ViBuf *) */
int viBufReset(ViBuf *vb) {
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
void viBufBeginPut(ViBuf *vb, unsigned char **p1, int *n1, unsigned char **p2, int *n2) {
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
void viBufEndPut(ViBuf *vb, int size) {
    func_00118CB0(vb->sema);
    vb->f14 += size;
    *(long *)(vb->pad48) += size;
    func_00118C90(vb->sema);
}
extern char D_001E8E50[];
extern int func_0023CEC8(ViBuf *, unsigned int);

/* viBufAddDMA(ViBuf *): under the buffer's semaphore, reads the channel-4 DMA position,
 * advances fC/f10 past the consumed slots, turns whole 0x800-byte units of f14 into
 * ref tags (0x80 qwords each) and restarts channel 4. Returns 0 when f44 is clear. */
int viBufAddDMA(ViBuf *vb) {
    int ok = 0;
    unsigned int chcr;
    int idx;
    int done;
    int n;
    int i;
    int rd;
    int pos;

    func_00118CB0(vb->sema);
    if (vb->f44 == 0) {
        ErrMessage(D_001E8E50);
        return 0;
    }
    setD4_CHCR(5);
    chcr = *(volatile unsigned int *)0x1000B400;
    rd = getFIFOindex(vb, *(volatile unsigned int *)0x1000B410);
    done = (rd + vb->count - vb->fC) % vb->count;
    vb->fC = (vb->fC + done) % vb->count;
    vb->f10 -= done;
    pos = (vb->fC + vb->f10) % vb->count;

    n = vb->f14 / 0x800;
    vb->f14 -= n << 11;
    if (n > 0) {
        int last = vb->count - 1;
        int prev = (vb->fC + vb->f10 + last) % vb->count;
        scTag2((unsigned long *)(vb->tags + prev * 16), vb->base + (prev << 11), 3, 0x80);
        ok = 1;
    }
    idx = pos;
    for (i = 0; i < n; i++) {
        int t = 0;
        if (i != n - 1) {
            t = 3;
        }
        scTag2((unsigned long *)(vb->tags + idx * 16), vb->base + (idx << 11), t, 0x80);
        idx = (idx + 1) % vb->count;
    }
    vb->f10 += n;
    if (vb->f10 != 0) {
        if (ok) {
            chcr = (chcr & 0x0FFFFFFF) | 0x30000000;
        }
        setD4_CHCR(chcr | 0x100);
    }
    func_00118C90(vb->sema);
    return 1;
}
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
int viBufStopDMA(ViBuf *vb) {
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
    setD3_CHCR(0);
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
int viBufRestartDMA(ViBuf *vb) {
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
        int a = getFIFOindex(vb, s->d4_madr);
        int b = getFIFOindex(vb, madr);
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
        setD3_CHCR(s->d3_chcr | 0x100);
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
int viBufDelete(ViBuf *vb) {
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
int viBufCount(ViBuf *vb) {
    int count;
    func_00118CB0(vb->sema);
    count = (vb->f10 << 11) + vb->f14;
    func_00118C90(vb->sema);
    return count;
}
/* viBufFlush(ViBuf *) */
void viBufFlush(ViBuf *vb) {
    func_00118CB0(vb->sema);
    vb->f14 = ((vb->f14 + 0x7FF) / 0x800) * 0x800;
    func_00118C90(vb->sema);
}
/* a % b as a helper: once inlined, its zero-divisor check is tested against $0 */
static inline int ring_mod(int a, int b) { return a % b; }

/* viBufModifyPts(ViBuf *, TimeStamp *): walks the 0x18-byte entries of the table at
 * 0x50 from the oldest one (index (f5C - f58) mod ntbl) while they lie within ts's
 * byte range (pos/len, modulo the 0x800 * count ring), trims the overlapped bytes off
 * each entry, clears an entry that becomes empty and lowers f58 (not below 0). */
int viBufModifyPts(ViBuf *vb, TimeStamp *ts) {
    int i = (vb->f5C - vb->f58 + vb->ntbl) % vb->ntbl;
    int size = vb->count << 11;
    int go = 1;
    TimeStamp *e;

    if (vb->f58 > 0) {
    while (go && (e = &((TimeStamp *)vb->tbl)[i])->len != 0 && ts->len != 0) {
        if (ring_mod(e->pos + size - ts->pos, size) < ts->len) {
            int n = ts->pos + ts->len - e->pos;
            if (e->len < n) {
                n = e->len;
            }
            e->pos = (e->pos + n) % size;
            e->len -= n;
            if (e->len == 0) {
                if (e->pts >= 0) {
                    e->pts = -1;
                    e->dts = -1;
                    e->pos = 0;
                    e->len = 0;
                }
                vb->f58 = (vb->f58 - 1 < 0) ? 0 : vb->f58 - 1;
            }
        } else {
            go = 0;
        }
        i = (i + 1) % vb->ntbl;
    }
    }
    return 0;
}
extern int func_0023DA88(ViBuf *, TimeStamp *);

/* viBufPutTs(ViBuf *, TimeStamp *) */
int viBufPutTs(ViBuf *vb, TimeStamp *ts) {
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
/* viBufGetTs(ViBuf *, TimeStamp *): works out the ring position the IPU is decoding
 * (channel-4 MADR, less the qwords held in the IPU FIFO per IPU_TOP, plus the saved bit
 * position at 0x38 in bytes), then searches the f58 pending entries of the table at 0x50
 * from the oldest for one whose pos/len range covers it. On a hit copies its pts/dts to
 * *ts, marks them -1 and lowers f58 (not below 0); otherwise *ts gets -1/-1. Returns 1. */
static inline int ts_dist(int a, int size) {
    return a % size;
}

/* viBufGetTs(ViBuf *, TimeStamp *) */
int viBufGetTs(ViBuf *vb, TimeStamp *ts) {
    unsigned int madr = *(volatile unsigned int *)0x1000B410;
    unsigned int top = *(volatile unsigned int *)0x10002020;
    int bp = *(unsigned int *)((char *)vb + 0x38) & 0x7F;
    int size = vb->count << 11;
    int found = 0;
    int pos;
    int n;
    int end;
    int j;

    madr -= (((top >> 16) & 3) + ((top >> 8) & 0xF)) << 4;
    func_00118CB0(vb->sema);
    ts->pts = -1;
    ts->dts = -1;
    madr += bp >> 3;
    madr += size;
    pos = (madr - vb->base) % size;
    n = vb->f58;
    end = vb->f5C;
    for (j = 0; j < n && !found; j++) {
        int k = (end - n + vb->ntbl + j) % vb->ntbl;
        if (ts_dist(pos + size - ((TimeStamp *)vb->tbl)[k].pos, size) < ((TimeStamp *)vb->tbl)[k].len) {
            found = 1;
            ts->pts = ((TimeStamp *)vb->tbl)[k].pts;
            ts->dts = ((TimeStamp *)vb->tbl)[k].dts;
            ((TimeStamp *)vb->tbl)[k].pts = -1;
            ((TimeStamp *)vb->tbl)[k].dts = -1;
            vb->f58 -= (vb->f58 < 2) ? vb->f58 : 1;
        }
    }
    func_00118C90(vb->sema);
    return 1;
}
