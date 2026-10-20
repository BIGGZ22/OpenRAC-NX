#include "include_asm.h"

/* A -G0 unit of its own, as 002FC468.c: retail reads D_001B22A8, D_001A7340 and D_001A7344
   absolutely (lui/lw) and only the last store, in the return's delay slot, through $gp. */
extern unsigned char *D_001B22A8 __attribute__((sda));
extern int D_001A7340 __attribute__((sda));
extern int D_001A7344 __attribute__((sda));

/* Clamps a rectangle to the D_001A7340 x D_001A7344 screen and appends the packet that writes
   it to GS register 0x40 (the packet func_002FC560 builds for any register). The value is
   built one field at a time: a single four-way | expression is regrouped by the compiler and
   computes the halves in another order. */
void func_002FC620(int x0, int x1, int y0, int y1)
{
    int w = D_001A7340 - 1;
    int h = D_001A7344 - 1;
    unsigned long v;

    if (x0 < 0) x0 = 0;
    if (x1 > w) x1 = w;
    if (y0 < 0) y0 = 0;
    if (y1 > h) y1 = h;
    v = x0;
    v |= (unsigned long)x1 << 16;
    v |= (unsigned long)y0 << 32;
    v |= (unsigned long)y1 << 48;

    *(unsigned int *)(D_001B22A8 + 0) = 0x10000002;
    *(unsigned int *)(D_001B22A8 + 4) = 0;
    *(unsigned int *)(D_001B22A8 + 8) = 0;
    *(unsigned int *)(D_001B22A8 + 12) = 0x50000002;
    *(unsigned int *)(D_001B22A8 + 16) = 0x8001;
    *(unsigned int *)(D_001B22A8 + 20) = 0x10000000;
    *(unsigned int *)(D_001B22A8 + 24) = 0xE;
    *(unsigned int *)(D_001B22A8 + 28) = 0;
    *(unsigned long *)(D_001B22A8 + 32) = v;
    *(unsigned int *)(D_001B22A8 + 40) = 0x40;
    *(unsigned int *)(D_001B22A8 + 44) = 0;
    D_001B22A8 += 0x30;
}
