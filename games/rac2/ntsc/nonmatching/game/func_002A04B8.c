#include "include_asm.h"

/* The reader of the three bytes func_002A0488 packs into word 7 of the command block:
   byte 4 of the word at +0x38 goes to *a, byte 5 to *b, byte 6 to *c.  Retail loads the
   64-bit word into $a0 (the parameter register it has just read), shifts it three times
   and masks the three results before it stores them; this cc1 keeps the word in $v0,
   reuses $a0 for the second result and interleaves the shifts with the stores. */
struct A04B8 { unsigned char pad[0x38]; volatile unsigned long long word; };

void func_002A04B8(struct A04B8 *p, volatile int *a, volatile int *b, volatile int *c)
{
    unsigned long long value = p->word;
    int first = (int)(unsigned char)(value >> 32);
    int second = (int)(unsigned char)(value >> 40);
    int third = (int)(unsigned char)(value >> 48);
    *a = first;
    *b = second;
    *c = third;
}
