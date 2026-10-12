#include "include_asm.h"

struct A0488 { unsigned char pad[0x38]; volatile unsigned long long word; };

void func_002A0488(struct A0488 *p, unsigned int a, unsigned int b, unsigned int c)
{
    unsigned long long value = (*(volatile unsigned long long *)&p->word << 32) >> 32;
    value |= (unsigned long long)a << 32;
    value |= (unsigned long long)b << 40;
    value |= (unsigned long long)c << 48;
    p->word = value;
}
