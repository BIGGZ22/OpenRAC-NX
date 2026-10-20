#include "include_asm.h"

extern void func_0011AC60(int value);

extern void func_00350808_1(int value);

extern void func_00350798(int value);

extern void func_0011AC40(int value);

/* snapshots five hardware registers into a structure, spins while one register's nibble stays set, snapshots five more and returns 1 */
int func_00350DC8(int *p)
{
    func_0011AC60(p[16]);
    p[17] = 0;
    func_00350808_1(5);
    p[7] = *(volatile int *)0x1000B410;
    p[8] = *(volatile int *)0x1000B430;
    p[9] = *(volatile int *)0x1000B420;
    p[10] = *(volatile int *)0x1000B400;
    if (*(volatile int *)0x10002010 & 0xF0)
        while (*(volatile int *)0x10002010 & 0xF0)
            ;
    func_00350798(0);
    p[11] = *(volatile int *)0x1000B010;
    p[12] = *(volatile int *)0x1000B020;
    p[13] = *(volatile int *)0x1000B000;
    p[14] = *(volatile int *)0x10002020;
    p[15] = *(volatile int *)0x10002010;
    func_0011AC40(p[16]);
    return 1;
}

extern int func_00350750(void *, int);

struct ipu_fifo {
    unsigned base;
    unsigned link;
    int units;
    int head;
    int count;
    int pad14;
    int pad18;
    unsigned madr;
    unsigned tadr;
    unsigned qwc;
    unsigned chcr;
    unsigned d3_madr;
    unsigned d3_qwc;
    unsigned d3_chcr;
    unsigned ipu_bp;
    unsigned ipu_ctrl;
    int intr;
    int saved;
};

/* restores the saved IPU and DMA channel 3/4 registers (the inverse of func_00350DC8) after rewinding
   the channel 4 address past the bytes the IPU still holds, wrapping inside the ring buffer; each of the
   two ring-index tests is written with its % twice because retail keeps one divide but a zero check
   per use */
int func_00350ED8(struct ipu_fifo *p)
{
    unsigned bp = p->ipu_bp;
    unsigned ifc = (bp >> 8) & 0xF;
    unsigned fp = (bp >> 16) & 3;
    unsigned fifo = fp + ifc;
    unsigned bp_pos = bp & 0x7F;
    unsigned madr = p->madr - fifo * 16;
    unsigned qwc = p->qwc + fifo;
    unsigned chcr = p->chcr | 0x100;
    unsigned tadr;

    tadr = p->tadr;
    func_0011AC60(p->intr);
    if (madr < p->base) {
        int units = p->units;
        int size = units << 11;
        int tag;

        qwc = (p->base - madr) >> 4;
        tadr = p->link & 0x0FFFFFFF;
        madr += size;
        tag = (p->madr != p->base && p->madr != p->base + size) ? 3 : 0;
        chcr = ((p->chcr & 0x0FFFFFFF) | (tag << 28)) | 0x100;
        if ((units - p->head) % units < 0 || (units - p->head) % units >= p->count) {
            p->head = units - 1;
            p->count++;
        }
    } else {
        int r1 = func_00350750(p, p->madr);
        int r2 = func_00350750(p, madr);

        if (r1 != r2) {
            int units = p->units;
            unsigned base = p->base;
            unsigned a;
            unsigned b;
            int tag;

            qwc = (base + (r1 << 11) - madr) >> 4;
            a = base + (p->madr - base) % (units << 11);
            b = base + (((p->head + p->count) % units) << 11);
            tadr = ((r1 << 4) + p->link) & 0x0FFFFFFF;
            tag = (a != b) ? 3 : 0;
            chcr = ((p->chcr & 0x0FFFFFFF) | (tag << 28)) | 0x100;
            if ((r2 + units - p->head) % units < 0 || (r2 + units - p->head) % units >= p->count) {
                p->head = r2;
                p->count++;
            }
        }
    }
    if (p->d3_madr != 0) {
        if (p->d3_qwc != 0) {
            *(volatile unsigned *)0x1000B010 = p->d3_madr;
            *(volatile unsigned *)0x1000B020 = p->d3_qwc;
            func_00350798(p->d3_chcr | 0x100);
        }
    }
    if (p->count != 0) {
        if (*(volatile int *)0x10002010 < 0)
            while (*(volatile int *)0x10002010 < 0)
                ;
        *(volatile unsigned *)0x10002000 = bp_pos;
        if (*(volatile int *)0x10002010 < 0)
            while (*(volatile int *)0x10002010 < 0)
                ;
    }
    *(volatile unsigned *)0x1000B410 = madr;
    *(volatile unsigned *)0x1000B430 = tadr;
    *(volatile unsigned *)0x1000B420 = qwc;
    if (p->count != 0)
        func_00350808_1(chcr);
    *(volatile unsigned *)0x10002010 = p->ipu_ctrl;
    p->saved = 1;
    func_0011AC40(p->intr);
    return 1;
}

extern void func_0011AC30(int value);
int func_00351210(int *p)
{
    func_00350808_1(5);
    *(volatile int *)0x1000B420 = 0;
    *(volatile int *)0x1000B410 = 0;
    *(volatile int *)0x1000B430 = 0;
    func_0011AC30(p[16]);
    return 1;
}
