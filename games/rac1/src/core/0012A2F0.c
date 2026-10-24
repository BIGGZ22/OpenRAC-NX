#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x12A2F0-0x12AC80. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
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
extern void func_00123650(void *);
extern char D_001534E0[];
extern int D_00132E70[];
extern int func_001238B0(int, int, int, int);
extern int D_00132EA8;
extern int *D_00159B28;
extern int *D_00159B2C;
extern int *D_00159B30;
extern char D_0015B108[];
extern int D_0015B180;
extern void func_00124B60(void *);
extern char D_00153658[];
extern int func_00124920(int);
typedef struct {
    char unk_00[4];
    int  unk_04;
    int  unk_08;
    char *unk_0C;
    char unk_10[0x320];
} Ent330;
extern Ent330 D_0015B640[];
extern void func_00119288(void *a, void *b);
extern void func_001286E8(int, int);
extern int D_00132F70[];
extern void func_0012BCC8(int);
extern void func_00128590(void *);
extern void func_00128968(void *, int);
extern int func_00128860(void *, int);
extern int func_00128A58(void *, int);
extern void func_00129180(void *);
extern char D_00153A80[];
extern void func_00116248_4(void *, char *, int, int) __asm__("func_00116248");
extern void func_0012C468_a(void *, void *) __asm__("func_0012C468");

extern void func_00128560();
extern void func_0012BC78();

/* _doCSC (libmpeg). Stack-size probe #2: a 32-byte local (two 16-byte
   compiler stack slots) instead of one, since a single int/4-int array
   both land in one slot here (frame still 0x10 short of retail's 0x70,
   see c1-c4). */
void func_0012A2F0(void *arg0, unsigned int arg1, int arg2) {
    char *p = (char *)arg0;
    int r;
    int event[8];

    while (*(volatile int *)0x10002010 < 0) {
    }
    r = func_0011D960();
    *(volatile unsigned int *)0x1000B010 = arg1 & 0x0FFFFFFF;
    *(volatile unsigned int *)0x1000B020 = (unsigned int)arg2 << 6;
    *(volatile unsigned int *)0x1000B000 = 0x100;
    if (r != 0) {
        func_0011D9A8();
    }
    func_00128560(arg0, arg2 | 0x70000000);
    event[0] = 4;
    func_0012BC78(*(int *)(p + 0x858), event);
    while (((*(volatile unsigned int *)0x1000B000 >> 8) & 1) != 0) {
    }
    while (*(volatile int *)0x10002010 < 0) {
    }
}

ASM_FUNC("asm/handwritten/core_text", func_0012A418);

extern int func_00118AC0_2A558(int, void *, int, void *) __asm__("func_00118AC0");
extern void func_00119460_2A558(int) __asm__("func_00119460");
extern void func_0012A418_2A558(void) __asm__("func_0012A418");
extern char D_00153AB0[];

typedef struct {
    volatile int done; /* 0x00: chunks finished (callback counts) */
    int err;           /* 0x04 */
    int size;          /* 0x08 */
    int next;          /* 0x0C */
    int total;         /* 0x10 */
} CscRec_2A558;

/* libmpeg chunked DMA copy: arm the channel-3 callback with a progress record, start the transfer, wait for all chunks. */
void func_0012A558(void *arg0, unsigned int arg1, int arg2) {
    char *p = (char *)arg0;
    int event[8];
    CscRec_2A558 rec;
    CscRec_2A558 *rp;
    int h;
    int r;

    rec.total = arg2 / 0x3FF + 1;
    rec.size = arg2;
    rec.next = (arg1 + 0xFFC00) & 0x0FFFFFFF;
    rec.err = 0;
    rec.done = 0;
    rp = &rec;
    while (*(volatile int *)0x10002010 < 0) {
    }
    h = func_00118AC0_2A558(3, (void *)func_0012A418_2A558, 0, rp);
    *(volatile unsigned int *)0x1000E010 = 8;
    func_00119460_2A558(3);
    r = func_0011D960();
    {
        volatile unsigned int *madr = (volatile unsigned int *)0x1000B010;
        volatile unsigned int *qwc = (volatile unsigned int *)0x1000B020;
        volatile unsigned int *chcr = (volatile unsigned int *)0x1000B000;
        *madr = arg1 & 0x0FFFFFFF;
        *qwc = 0xFFC0;
        *chcr = 0x100;
    }
    if (r != 0) {
        func_0011D9A8();
    }
    *(volatile unsigned int *)0x10002000 = 0x700003FF;
    event[0] = 4;
    func_0012BC78(*(void **)(p + 0x858), event);
    while (rec.done < rec.total) {
    }
    if (rec.err != 0) {
        func_0012C468_a(p, D_00153AB0);
    }
    while (*(volatile int *)0x10002010 < 0) {
    }
    func_001193F8(3);
    func_00118AD0(3, h);
}

ASM_FUNC("asm/handwritten/core_text", func_0012A718);

extern void *func_0012BC78_result(void *, int *) __asm__("func_0012BC78");
/* Transfer the MPEG input through the IPU/SPR DMA channel, splitting large blocks. */
extern void *func_00118AC0(int, void *, int, void *);
extern void func_00119460(int);
extern void func_0012A558(void *, unsigned int, int);
extern void func_0012A718();

void func_0012A7E8(void *arg0, int *arg1) {
    char *p = (char *)arg0;
    int ev[8];
    int loc[4];
    int n, small, r, addr;
    unsigned int mask;
    void *h;

    ev[0] = 2;
    n = arg1[3] * arg1[4];
    func_0012BC78_result(*(void **)(p + 0x858), ev);
    if ((*(volatile unsigned int *)0x10002010 & 0x4000) != 0) {
        *(volatile unsigned int *)0x10002010 = 0x40000000;
    }
    small = n < 0x400;
    while (*(volatile int *)0x10002010 < 0) {
    }
    func_00128560(arg0, 0);
    addr = arg1[0];
    while (*(volatile int *)0x10002010 < 0) {
    }
    mask = 0x0FFFFFFF;
    loc[1] = addr & mask;
    loc[0] = n * 0x18;
    if ((unsigned int)loc[0] > 0xFFFF) {
        h = func_00118AC0(4, (void *)func_0012A718, 0, &loc[0]);
        *(volatile unsigned int *)0x1000E010 = 0x10;
        func_00119460(4);
        r = func_0011D960();
        *(volatile unsigned int *)0x1000B410 = loc[1];
        *(volatile unsigned int *)0x1000B420 = 0xFFFF;
        *(volatile unsigned int *)0x1000B400 = 0x101;
        if (r != 0) {
            func_0011D9A8();
        }
        loc[1] = (loc[1] + 0xFFFF0) & mask;
        loc[0] = loc[0] - 0xFFFF;
        if (small) {
            func_0012A2F0(arg0, *(int *)(p + 0xD8), n);
        } else {
            func_0012A558(arg0, *(int *)(p + 0xD8), n);
        }
        func_001193F8(4);
        func_00118AD0(4, (int)h);
    } else {
        r = func_0011D960();
        *(volatile unsigned int *)0x1000B410 = arg1[0] & mask;
        *(volatile unsigned int *)0x1000B420 = loc[0];
        *(volatile unsigned int *)0x1000B400 = 0x101;
        if (r != 0) {
            func_0011D9A8();
        }
        loc[0] = 0;
        if (small) {
            func_0012A2F0(arg0, *(int *)(p + 0xD8), n);
        } else {
            func_0012A558(arg0, *(int *)(p + 0xD8), n);
        }
    }
    ev[0] = 3;
    func_0012BC78_result(*(void **)(p + 0x858), ev);
}

extern long func_0012AAC8(void *, int);

/* _sysbitInit: points the bit stream at arg0 at the arg3-byte ring buffer
   at arg2, reading from arg1, and resets it. The store order is
   load-bearing (LEVERS.md lever 3): arg1's +0x8 store has to come before
   its +0xC store, after the other fields in retail's order. */
void func_0012AA70(void *arg0, int arg1, int arg2, int arg3) {
    char *p = (char *)arg0;
    *(int *)(p + 0x28) = arg3;
    *(int *)(p + 0x8) = arg1;
    *(long *)(p + 0x0) = 0;
    *(int *)(p + 0x10) = 0;
    *(long *)(p + 0x18) = 0;
    *(int *)(p + 0x20) = arg2;
    *(int *)(p + 0x24) = arg2 + arg3;
    *(int *)(p + 0xC) = arg1;
    func_0012AAC8(arg0, 0);
}

/* Peek n high bits of the 64-bit accumulator, without consuming them.
 * As a C shift, this requires 1 <= n <= 64. GNU 2.10-ee assembles the
 * final int conversion directly into the return delay slot. */
int func_0012AAA8(void *arg0, int n) {
    unsigned long acc = *(unsigned long *)arg0;
    return (int)(acc >> (64 - n));
}

/* The bitstream reader's state (func_0012AA70 sets it up). */
typedef struct {
    unsigned long acc;     /* 0x00: the next bits, top-aligned */
    unsigned char *start0; /* 0x08 */
    unsigned char *cur;    /* 0x0C: next byte to load */
    unsigned int bits;     /* 0x10: valid bits in acc */
    long total;            /* 0x18: bits consumed so far */
    unsigned char *start;  /* 0x20: the ring buffer */
    unsigned char *end;    /* 0x24 */
    int len;               /* 0x28 */
} BitStream;

/* Consume n bits, then refill the accumulator a byte at a time from the
   ring buffer until more than 56 bits are buffered; returns the new
   total (mpeg2decode's Flush_Buffer shape). The `c` local and the
   returned total are load-bearing: they bring the function to the 40
   insns that put GCSE's hash buckets, and so PRE's registers, in
   retail's order. */
long func_0012AAC8(void *arg0, int n) {
    BitStream *p = arg0;
    p->acc <<= n;
    p->bits -= n;
    if (p->bits <= 56) {
        do {
            unsigned int c = *p->cur++;
            p->acc |= (unsigned long)c << (56 - p->bits);
            if (p->cur >= p->end) p->cur = p->start;
            p->bits += 8;
        } while (p->bits <= 56);
    }
    return p->total += n;
}

/* Get n bits: peek them (func_0012AAA8), then consume them
   (func_0012AAC8). */
int func_0012AB60(void *arg0, int arg1) {
    int r = func_0012AAA8(arg0, arg1);
    func_0012AAC8(arg0, arg1);
    return r;
}


/* Get one bit, as func_0012AB60. */
int func_0012ABB0(void *arg0) {
    int r;
    r = func_0012AAA8(arg0, 1);
    func_0012AAC8(arg0, 1);
    return r;
}

/* _sysbitJump (libmpeg bit.c): skip n bytes, then refill through
   _sysbitFlush. Clearing acc and bits before the total update is the
   one statement order that matches. */
void func_0012ABF8(void *arg0, int n) {
    BitStream *p = arg0;
    p->acc = 0;
    p->bits = 0;
    p->total += n * 8;
    p->cur = p->start0 + (p->total >> 3);
    if (p->cur >= p->end) p->cur -= p->len;
    func_0012AAC8(p, 0);
}

unsigned int func_0012AC50(char *arg0, int arg1) {
    unsigned int v = *(int *)(arg0 + 0x8) + (arg1 >> 3);
    if (v >= *(unsigned int *)(arg0 + 0x24)) {
        v -= *(int *)(arg0 + 0x28);
    }
    return v;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012AC7C);
