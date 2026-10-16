/* Carried from the PAL decompilation's src/game/menu.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it. */
extern void func_001F97E8();
extern void func_001F98D0(void *, void *, int);

void FUN_00207c28(void *dst, unsigned char *ctrl, int spans) {
    unsigned char *dst_end;
    unsigned char *end;
    unsigned char *span;
    unsigned char *cursor;
    unsigned char *next_dst;
    unsigned char *next;
    unsigned char *wrapped;
    unsigned char *r;
    unsigned char *w;
    unsigned char *from;
    unsigned char *to;
    int count;
    int n;
    int bit;

    dst_end = (unsigned char *)dst + 0x8000;
    end = (unsigned char *)0x70002000;
    span = (unsigned char *)spans;
    count = *ctrl >> 1;
    ctrl++;
    func_001F97E8((void *)0x70000000, 0, 0x2400);
    bit = 1;
    cursor = (unsigned char *)0x70000000;
    for (;;) {
        next_dst = (unsigned char *)dst + 0x400;
        do {
            cursor += *span++;
            n = *span++;
            if (n != 0) {
                do {
                    n--;
                    next = cursor + 1;
                    if (count == 0) {
                        do {
                            count = *ctrl++;
                            bit = !bit;
                        } while (count == 0);
                    }
                    *cursor = bit;
                    count--;
                    cursor = next;
                } while (n != 0);
            }
        } while (cursor < end);
        r = (unsigned char *)0x70000000;
        w = (unsigned char *)0x70000000;
        do {
            *w = *r++;
            *w |= *r++ << 1;
            *w |= *r++ << 2;
            *w |= *r++ << 3;
            *w |= *r++ << 4;
            *w |= *r++ << 5;
            *w |= *r++ << 6;
            *w |= *r++ << 7;
            w++;
        } while (r < end);
        func_001F98D0(dst, (void *)0x70000000, 0x400);
        dst = next_dst;
        if (next_dst == dst_end) {
            return;
        }
        func_001F97E8((void *)0x70000000, 0, 0x2000);
        wrapped = cursor - 0x2000;
        from = (unsigned char *)0x70002000;
        to = (unsigned char *)0x70000000;
        if (cursor > end) {
            do {
                *to++ = *from++;
            } while (from < cursor);
        }
        func_001F97E8((void *)0x70002000, 0, 0x400);
        cursor = wrapped;
    }
}
