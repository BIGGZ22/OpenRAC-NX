/* Carried from the PAL decompilation's nonmatching/functional/func_L01_00262BC0.c (games/rac1/pal) through the version
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
/* functional: high. Copies ripple heights and halos into the scratchpad: 16 x 2 pairs of 16-byte blocks from a1 (stride 0x40) into a0+0x70 (stride 0x60), 4 x 3 blocks from a1+0x400/0x480/0x500 into a0+0x10/0x670/0x6D0, 16 x 3 words from a1+0x440/0x4C0/0x540 into a0+0x6C/0xB0/0xB4 (stride 0x60), then six words from a1+0x580 to a0+0xC/0x50/0x66C/0x6B0/0x6B4/0x710. Same reads, writes and order as the assembly; no return value. */
typedef int u128_62BC0_c261c10 __attribute__((mode(TI)));

void FUN_L01_00261c10(char *a0, char *a1)
{
    {
        char *s = a1;
        char *d = a0 + 0x70;
        int outer = 16;
        do {
            char *dd = d;
            int inner = 2;
            do {
                u128_62BC0_c261c10 p = *(u128_62BC0_c261c10 *)s;
                u128_62BC0_c261c10 q = *(u128_62BC0_c261c10 *)(s + 0x10);
                *(u128_62BC0_c261c10 *)dd = p;
                *(u128_62BC0_c261c10 *)(dd + 0x10) = q;
                s += 0x20;
                dd += 0x20;
            } while (--inner != 0);
            d += 0x60;
        } while (--outer != 0);
    }

    {
        char *s1 = a1 + 0x400;
        char *s2 = a1 + 0x480;
        char *s3 = a1 + 0x500;
        char *d1 = a0 + 0x10;
        char *d2 = a0 + 0x670;
        char *d3 = a0 + 0x6D0;
        int n = 4;
        do {
            u128_62BC0_c261c10 x = *(u128_62BC0_c261c10 *)s1;
            u128_62BC0_c261c10 y = *(u128_62BC0_c261c10 *)s2;
            u128_62BC0_c261c10 z = *(u128_62BC0_c261c10 *)s3;
            *(u128_62BC0_c261c10 *)d1 = x;
            *(u128_62BC0_c261c10 *)d2 = y;
            *(u128_62BC0_c261c10 *)d3 = z;
            s1 += 0x10;
            s2 += 0x10;
            s3 += 0x10;
            d1 += 0x10;
            d2 += 0x10;
            d3 += 0x10;
        } while (--n != 0);
    }

    {
        char *s1 = a1 + 0x440;
        char *s2 = a1 + 0x4C0;
        char *s3 = a1 + 0x540;
        char *d1 = a0 + 0x6C;
        char *d2 = a0 + 0xB0;
        char *d3 = a0 + 0xB4;
        int n = 16;
        do {
            int x = *(int *)s1;
            int y = *(int *)s2;
            int z = *(int *)s3;
            *(int *)d1 = x;
            *(int *)d2 = y;
            *(int *)d3 = z;
            s1 += 4;
            s2 += 4;
            s3 += 4;
            d1 += 0x60;
            d2 += 0x60;
            d3 += 0x60;
        } while (--n != 0);
    }

    {
        int *w = (int *)(a1 + 0x580);
        *(int *)(a0 + 0xC) = w[0];
        *(int *)(a0 + 0x50) = w[1];
        *(int *)(a0 + 0x66C) = w[2];
        *(int *)(a0 + 0x6B0) = w[3];
        *(int *)(a0 + 0x6B4) = w[4];
        *(int *)(a0 + 0x710) = w[5];
    }
}
