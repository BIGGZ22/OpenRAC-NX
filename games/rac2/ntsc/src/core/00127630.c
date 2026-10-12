#include "common.h"
#include "structs.h"

extern int r1_D_00132EA8;

extern int func_00127508(int, int, int, int);

extern void r1_func_00118C90(int);

extern int r1_D_00132EAC;

extern char r1_D_00159B00[];

extern int r1_D_00159B80;

extern char r1_D_0015B0C0[];

extern int r1_func_0011B4C8();

extern int r1_func_00118CC0(int);

extern void func_001277F8(void *);

/* rac1-decomp src/core/001236F0.c: func_001239D8 */
int func_00127630(int arg0, int arg1, int arg2) {
    int r = func_00127508(arg0, arg1, arg2, 0x40);
    if (r == 0) {
        r1_D_00132EA8 = 0xB;
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_00123A10 */
/* RPC 3 on the client r1_D_00159B00, in func_00127630's shape: -100 with
   no server bound, -200 when the semaphore wait fails. */
int func_00127668(int arg0) {
    char *cd = r1_D_00159B00;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    r1_D_00159B80 = arg0;
    r = r1_func_0011B4C8(cd, 3, 1, &r1_D_00159B80, 0x30, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 3;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_00123AC8 */
/* RPC 4, the same shape with three request words. `buf` is assigned
   after the semaphore call, so retail builds its address in a temp. */
int func_00127720(int arg0, int arg1, int arg2) {
    char *cd = r1_D_00159B00;
    int *buf;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    buf = &r1_D_00159B80;
    buf[0] = arg0;
    buf[4] = arg1;
    buf[5] = arg2;
    r = r1_func_0011B4C8(cd, 4, 1, buf, 0x30, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 4;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_00123BA0 */
/*
 * Unpack a scratchpad-resident descriptor: arg0 is forced into the SPR
 * window (| 0x20000000), then two counted byte runs are copied out of
 * it -- the one at +0x10 to the pointer at +0x8 for the count at +0x0,
 * and the one at +0x50 to the pointer at +0xC for the count at +0x4.
 * The counts are re-read from the descriptor on every iteration, so
 * they are written out in full rather than cached.
 *
 * The explicit zero-trip guard around each do/while is what makes this
 * match, and it is worth knowing why. Written as a plain `for`, the
 * source base (p + 0x10) is loop-invariant, so GCC hoists it ABOVE the
 * guard; retail computes it BELOW, between the guard and the loop head.
 * That one position shifts the loop head by a word, which in turn costs
 * the alignment nop retail carries -- two instructions per loop, 16
 * bytes over the two loops. Spelling the guard yourself puts the
 * invariant where retail has it. (Folding the +0x10 into the load
 * offset instead of keeping a base pointer costs another 8.)
 */
void func_001277F8(void *arg0) {
    char *p = (char *)((unsigned int)arg0 | 0x20000000);
    unsigned char *src;
    char *d;
    int i;

    if (*(int *)(p + 0x0) != 0) {
        d = *(char **)(p + 0x8);
        i = 0;
        if (i < *(int *)(p + 0x0)) {
            src = (unsigned char *)(p + 0x10);
            do {
                *d++ = src[i];
                i++;
            } while (i < *(int *)(p + 0x0));
        }
    }
    if (*(int *)(p + 0x4) != 0) {
        d = *(char **)(p + 0xC);
        i = 0;
        if (i < *(int *)(p + 0x4)) {
            src = (unsigned char *)(p + 0x50);
            do {
                *d++ = src[i];
                i++;
            } while (i < *(int *)(p + 0x4));
        }
    }
}
