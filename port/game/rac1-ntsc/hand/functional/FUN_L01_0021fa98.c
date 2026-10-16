/* Carried from the PAL decompilation's nonmatching/functional/func_L01_0021F9C8.c (games/rac1) through the version
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
/* functional: medium. Builds two DMA packets at the packet cursor D_L01_001611C0 from templates (D_L01_001C44A0, D_L01_00160EC0/60/80): n entries of 8-byte (c), 4-byte (b), 12-byte (a) data in the first, n 8-byte entries (d) in the second, patches qword counts, then appends a DMA call tag; not run against real calls. */
extern u32 *D_L01_00161240_9C8 __asm__("D_L01_001611C0") MACRO_ADDR;
extern u32 D_L01_001C4720_9C8[] __asm__("D_L01_001C44A0");
extern u32 D_L01_00160F40_9C8[] __asm__("D_L01_00160EC0");
extern u32 D_L01_00160F60_9C8[] __asm__("D_L01_00160EE0");
extern u32 D_L01_00160F80_9C8[] __asm__("D_L01_00160F00");



void FUN_L01_0021fa98(s32 n, u32 *a, u32 *b, u32 *c, u32 *d, s32 flag) {
    u32 *base;
    u32 *cur;
    u32 *start2;
    u32 *end1;
    u32 *src;
    s32 k;
    s32 j;
    u32 hi;
    u32 mode;

    mode = ((flag & 1) << 1) + 0xC;
    base = D_L01_00161240_9C8;
    cur = base;
    hi = (u32)n << 16;

    for (k = 0; k < 5; k++)
        qcopy(cur + k * 4, D_L01_001C4720_9C8 + k * 4);
    cur += 20;
    cur[-16] |= n;
    qcopy(cur, D_L01_00160F40_9C8);
    cur += 4;
    cur[-1] |= hi;
    src = c;
    k = n;
    do {
        *(u64 *)cur = *(u64 *)src;
        src += 2;
        cur += 2;
        k--;
    } while (k != 0);
    while (((u32)cur & 0xF) != 0)
        *cur++ = 0;

    start2 = cur;
    qcopy(cur, D_L01_00160F60_9C8);
    cur += 4;
    cur[-1] |= hi;
    src = b;
    k = n;
    do {
        *cur++ = *src++;
        k--;
    } while (k != 0);
    while (((u32)cur & 0xF) != 0)
        *cur++ = 0;

    qcopy(cur, D_L01_00160F80_9C8);
    cur += 4;
    cur[-1] |= hi;
    src = a;
    k = n;
    do {
        cur[0] = src[0];
        cur[1] = src[1];
        cur[2] = src[2];
        src += 3;
        cur += 3;
        k--;
    } while (k != 0);
    while (((u32)cur & 0xF) != 0)
        *cur++ = 0;

    cur[0] = 0x15000000 | mode;
    cur[1] = 0x11000000;
    cur[2] = 0;
    cur[3] = 0;
    cur += 4;
    end1 = cur;
    base[0] |= (u32)(((char *)cur - (char *)base) >> 4) - 1;
    D_L01_00161240_9C8 = cur;
    base = cur;

    for (k = 0; k < 5; k++)
        qcopy(cur + k * 4, D_L01_001C4720_9C8 + k * 4);
    cur += 20;
    cur[-16] |= n;
    cur[-15] |= 0x01000000;
    cur[-10] |= 0x01000000;
    qcopy(cur, D_L01_00160F40_9C8);
    cur += 4;
    cur[-1] |= hi;
    src = d;
    k = n;
    do {
        *(u64 *)cur = *(u64 *)src;
        src += 2;
        cur += 2;
        k--;
    } while (k != 0);
    while (((u32)cur & 0xF) != 0)
        *cur++ = 0;

    base[0] |= (u32)(((char *)cur - (char *)base) >> 4) - 1;
    D_L01_00161240_9C8 = cur;
    base = cur;
    base[2] = 0;
    base[3] = 0;
    base[1] = (u32)start2;
    base[0] = 0x30000000 | (u32)(((char *)end1 - (char *)start2) >> 4);
    D_L01_00161240_9C8 = base + 4;
}
