#include "include_asm.h"

/* A -G0 unit of its own: retail loads D_001B22A8 absolutely (lui/lw) everywhere except the
   store in the call's delay slot, which the assembler turns into the one-instruction $gp form
   for a bare small-data symbol. 002FC3C0.c is -G8 and would load it through $gp too. */
extern unsigned char *D_001B22A8 __attribute__((sda));

/* func_00282B50 is a quadword copy: (destination, source, byte count). */
extern void func_00282B50(void *destination, void *source, int bytes);

/* Appends a four-word packet header to the buffer at D_001B22A8 (a tag with the quadword count,
   a zero word, a fixed word, and a word built from a0 and the count), then copies count
   quadwords from source behind it and advances the buffer. */
void func_002FC468(unsigned int a0, void *source, unsigned int count)
{
    *(unsigned int *)(D_001B22A8 + 0) = 0x10000000u | count;
    *(unsigned int *)(D_001B22A8 + 4) = 0;
    *(unsigned int *)(D_001B22A8 + 8) = 0x01000404u;
    *(unsigned int *)(D_001B22A8 + 12) = a0 | (count << 16) | 0x6C000000u;
    D_001B22A8 += 16;
    func_00282B50(D_001B22A8, source, count << 4);
    D_001B22A8 += count << 4;
}
