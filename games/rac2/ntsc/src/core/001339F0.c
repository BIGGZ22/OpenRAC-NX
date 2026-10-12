#include "include_asm.h"

void func_001339F0(void *dest, const void *src, int n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    int i = 0;

    if (n <= 0) {
        return;
    }

    do {
        d[i] = s[i];
        i++;
    } while (i < n);
}

INCLUDE_ASM("asm/core", func_00133A28);

INCLUDE_ASM("asm/core", func_00133A78);

INCLUDE_ASM("asm/core", func_00133AA0);
